#include "IRremote.h"
#include "IRremoteInt.h"
#include "ir_Toshiba.h"

//==============================================================================
//
//                           T O S H I B A
//
//  Toshiba Heat Pump RAS-10PKVP-ND has a remote WH-H07JE. It sends three
//  different lenghts signals. Most of the buttons go within 9 byte sequence.
//  The last byte is parity. 
//
//  Real signal data is readable from raw data using the example bit structures.
//
//  Ilkka Tengvall
//
//==============================================================================

#define TOSHIBA_72_BITS       72
#define TOSHIBA_9_BYTES        9

#define TOSHIBA_56_BITS       56
#define TOSHIBA_7_BYTES        7

#define TOSHIBA_80_BITS       80
#define TOSHIBA_10_BYTES      10

#define TOSHIBA_HDR_MARK    4400
#define TOSHIBA_HDR_SPACE   4400
#define TOSHIBA_BIT_MARK     550
#define TOSHIBA_ONE_SPACE   1600
#define TOSHIBA_ZERO_SPACE   550

//+=============================================================================
#if SEND_TOSHIBA
void  IRsend::sendTOSHIBA (ir_toshiba_cmd_s cmd)
{
    byte data[TOSHIBA_10_BYTES]; // must fit the longest data
    int bytes=0;
    int i;

    // Set IR carrier frequency
	enableIROut(38);

    switch (cmd.type)
    {
      case HEAT_PUMP_CMD:
        data[0] = 0xf2;
        data[1] = 0x0d;
        data[2] = 0x03;
        data[3] = 0xfc;
        data[4] = 0x01;
        data[5] = cmd.values.bytes[0];
        data[6] = cmd.values.bytes[1];
        data[7] = cmd.values.bytes[2];
        // count parity by using all bytes but the parity byte itself.
        data[8] = 0;
        for (i=0; i < 8; i++)
            data[8] = data[8]^data[i];
        bytes = TOSHIBA_9_BYTES;
        break;
      case HI_PWR_CMD:
        data[0] = 0xf2;
        data[1] = 0x0d;
        data[2] = 0x04;
        data[3] = 0xfb;
        data[4] = 0x09;
        data[5] = 0;
        data[6] = 0;
        data[7] = 0;
        data[8] = 0x01;
        data[9] = 0x08;
        bytes = TOSHIBA_10_BYTES;
        break;
      case SLEEP_CMD:
        data[0] = 0xf2;
        data[1] = 0x0d;
        data[2] = 0x04;
        data[3] = 0xfb;
        data[4] = 0x09;
        data[5] = 0;
        data[6] = 0;
        data[7] = 0;
        data[8] = 0x03;
        data[9] = 0x0a;
        bytes = TOSHIBA_10_BYTES;
        break;
      case SWING_CMD:
        data[0] = 0xf2;
        data[1] = 0x0d;
        data[2] = 0x01;
        data[3] = 0xfe;
        data[4] = 0x21;
        data[5] = 0x04;
        data[6] = 0x25;
        bytes = TOSHIBA_7_BYTES;
        break;
      case UP_DOWN_CMD:
        data[0] = 0xf2;
        data[1] = 0x0d;
        data[2] = 0x01;
        data[3] = 0xfe;
        data[4] = 0x21;
        data[5] = 0;
        data[6] = 0x21;
        bytes = TOSHIBA_7_BYTES;
        break;
    }
	// Header
	mark(TOSHIBA_HDR_MARK);
	space(TOSHIBA_HDR_SPACE);

    // Send out all bytes
    for (i=0; i < bytes; i++) {
        for (byte mask = 0x80; mask; mask >>=1) {
            if (data[i] & mask) {
                mark(TOSHIBA_BIT_MARK);
                space(TOSHIBA_ONE_SPACE);
            } else {
                mark(TOSHIBA_BIT_MARK);
                space(TOSHIBA_ZERO_SPACE);
            }

        }
    }

	// Footer
	mark(TOSHIBA_BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

#if DECODE_TOSHIBA
bool  IRrecv::decodeTOSHIBA (decode_results *results)
{
	long  data   = 0;
	int   offset = 1;  // Skip first space
	int   bits   = 0;
	int   bytes  = 0;

	// Initial mark
	if (!MATCH_MARK(results->rawbuf[offset++], TOSHIBA_HDR_MARK))   return false ;

	bits = (results->rawlen - 3)/2;
	if (!(bits == TOSHIBA_56_BITS ||
		bits == TOSHIBA_72_BITS ||
		bits == TOSHIBA_80_BITS)) {
		return false ;
	}
	bytes = bits/8;

	// Initial space
	if (!MATCH_SPACE(results->rawbuf[offset++], TOSHIBA_HDR_SPACE))  return false ;

    // we only dump 32 bits as it all won't fit into such small variable
	for (int i = 0;  i < 32;   i++) {
		if (!MATCH_MARK(results->rawbuf[offset++], TOSHIBA_BIT_MARK))  return false ;

		if      (MATCH_SPACE(results->rawbuf[offset], TOSHIBA_ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], TOSHIBA_ZERO_SPACE))  data = (data << 1) | 0 ;
		else                                                                return false ;
		offset++;
	}

	// Success
	results->bits        = bits;
	results->value       = data;
	results->decode_type = TOSHIBA;
	return true;
}
#endif

