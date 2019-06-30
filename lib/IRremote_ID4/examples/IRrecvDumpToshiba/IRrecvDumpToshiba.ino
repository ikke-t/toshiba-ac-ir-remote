//------------------------------------------------------------------------------
// Include the IRremote library header
//
#include <IRremote.h>

#define TOSHIBA_BITS_1        72
#define TOSHIBA_BYTES_1        9
#define TOSHIBA_BITS_2        56
#define TOSHIBA_BYTES_2        7
#define TOSHIBA_BITS_3        80
#define TOSHIBA_BYTES_3       10
#define TOSHIBA_HDR_MARK    4400
#define TOSHIBA_HDR_SPACE   4400
#define TOSHIBA_BIT_MARK     550
#define TOSHIBA_ONE_SPACE   1600
#define TOSHIBA_ZERO_SPACE   550

//------------------------------------------------------------------------------
// Tell IRremote which Arduino pin is connected to the IR Receiver (TSOP4838)
//
#define CORE_LED0_PIN 13;
int recvPin = 11;
IRrecv irrecv(recvPin);

//+=============================================================================
// Configure the Arduino
//
void  setup ( )
{
  Serial.begin(9600);   // Status message will be sent to PC at 9600 baud
  irrecv.enableIRIn();  // Start the receiver
}

//+=============================================================================
// Display IR code
//
void  ircode (decode_results *results)
{
  // Print Code
  Serial.print(results->value, HEX);
}

//+=============================================================================
// Display encoding type
//
void  encoding (decode_results *results)
{
  switch (results->decode_type) {
    default:           
    case TOSHIBA:      Serial.print("TOSHIBA");       break ;
    case UNKNOWN:      Serial.print("UNKNOWN");       break ;
    case NEC:          Serial.print("NEC");           break ;
    case SONY:         Serial.print("SONY");          break ;
    case RC5:          Serial.print("RC5");           break ;
    case RC6:          Serial.print("RC6");           break ;
    case DISH:         Serial.print("DISH");          break ;
    case SHARP:        Serial.print("SHARP");         break ;
    case JVC:          Serial.print("JVC");           break ;
    case SANYO:        Serial.print("SANYO");         break ;
    case MITSUBISHI:   Serial.print("MITSUBISHI");    break ;
    case SAMSUNG:      Serial.print("SAMSUNG");       break ;
    case LG:           Serial.print("LG");            break ;
    case WHYNTER:      Serial.print("WHYNTER");       break ;
    case AIWA_RC_T501: Serial.print("AIWA_RC_T501");  break ;
    case PANASONIC:    Serial.print("PANASONIC");     break ;
    case DENON:        Serial.print("Denon");         break ;
  }
}

//+=============================================================================
// Dump out the decode_results structure.
//
void  dumpInfo (decode_results *results)
{
  // Check if the buffer overflowed
  if (results->overflow) {
    Serial.println("IR code too long. Edit IRremoteInt.h and increase RAWLEN");
    return;
  }

  // Show Encoding standard
  Serial.print("Encoding  : ");
  encoding(results);
  Serial.println("");

  // Show Code & length
  Serial.print("Code      : ");
  ircode(results);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
}

//+=============================================================================
// Dump out the decode_results structure.
//
void  dumpRaw (decode_results *results)
{
  // Print Raw data
  Serial.print("Timing[");
  Serial.print(results->rawlen-1, DEC);
  Serial.println("]: ");

  for (int i = 1;  i < results->rawlen;  i++) {
    unsigned long  x = results->rawbuf[i] * USECPERTICK;
    if (!(i & 1)) {  // even
      Serial.print("-");
      if (x < 1000)  Serial.print(" ") ;
      if (x < 100)   Serial.print(" ") ;
      Serial.print(x, DEC);
    } else {  // odd
      Serial.print("     ");
      Serial.print("+");
      if (x < 1000)  Serial.print(" ") ;
      if (x < 100)   Serial.print(" ") ;
      Serial.print(x, DEC);
      if (i < results->rawlen-1) Serial.print(", "); //',' not needed for last one
    }
    if (!(i % 8))  Serial.println("");
  }
  Serial.println("");                    // Newline
}

void printByteInBits(byte b)
{
  for (int i=7; i>=0; i--)
    Serial.print(b>>i & 1, BIN);
}

//+=============================================================================
// Dump out the decode_results structure.
//
typedef union cmd {
  byte bytes[TOSHIBA_BYTES_1];
  struct bits {
    byte byte0;        // always F2
    byte byte1;        // always 0D
    byte byte2;        // always 03
    byte byte3;        // always FC
    byte byte4;        // always 01
    // byte 5
    byte temp   :4;    // 17-30 degrees, value as: celsius - 17 = 0-13
    byte byte5  :4;    // unused
    // byte 6
    byte fan    :3;    // 0=auto, 2=*, 3=**, 4=***, 5=****, 6=*****  
    byte mode   :2;    // 0=auto, 1=cool, 2=dry, 3=heat
    byte power  :3;    // 0=on 7=off
    // byte 7
    byte byte71 :3;    // unused
    byte pure   :1;    // 1=on, 0=off  
    byte byte72 :4;    // unused
    // byte 8
    byte crc    :8;    // some sort of CRC
  };
};

void  dumpCode (decode_results *results)
{
  int offset = 3; // skip first two markers
  int i, j;
  byte cmd_bytes[TOSHIBA_BYTES_3] = {0};
  int bytes = 0;
  int bits = 0;
  
  // Start declaration
  Serial.print("unsigned int  ");          // variable type
  Serial.print("rawData[");                // array name
  Serial.print(results->rawlen - 1, DEC);  // array size
  Serial.print("] = {");                   // Start declaration

  // Dump data
  for (int i = 1;  i < results->rawlen;  i++) {
    Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
    if ( i < results->rawlen-1 ) Serial.print(","); // ',' not needed on last one
    if (!(i & 1))  Serial.print(" ");
  }
  // End declaration
  Serial.print("};");  // 

  Serial.println("");
  
  bits = (results->rawlen - 3)/2;
  bytes = bits/8;
  if (bits == TOSHIBA_BITS_1 ||
      bits == TOSHIBA_BITS_2 ||
      bits == TOSHIBA_BITS_3) {
    Serial.print("nibbles as bits: ");
    offset = 3; // skip first two markers
    for (i = 0;  i < bits;   i++) {
      if (i%4 == 0 ) Serial.print(" ");
      if (!MATCH_MARK(results->rawbuf[offset++], TOSHIBA_BIT_MARK))       { Serial.println("bug"); break; }
      if      (MATCH_SPACE(results->rawbuf[offset], TOSHIBA_ONE_SPACE))   Serial.print("1");
      else if (MATCH_SPACE(results->rawbuf[offset], TOSHIBA_ZERO_SPACE))  Serial.print("0");
      else                                                                { Serial.println("bug"); break;}
      offset++;
    }
    offset = 3; // skip first two markers
    for (i = 0;  i < bits;   i++) {
      if (!MATCH_MARK(results->rawbuf[offset++], TOSHIBA_BIT_MARK))       { Serial.println("bug"); break; }
      if      (MATCH_SPACE(results->rawbuf[offset], TOSHIBA_ONE_SPACE))   cmd_bytes[i/8] = (cmd_bytes[i/8] << 1) | 1 ;
      else if (MATCH_SPACE(results->rawbuf[offset], TOSHIBA_ZERO_SPACE))  cmd_bytes[i/8] = (cmd_bytes[i/8] << 1) | 0 ;
      else                                                                { Serial.println("bug"); break; }
	offset++;
    }
    
    Serial.println("");
    Serial.print("bytes:            ");
    for (i=0; i<bytes; i++){
      if (cmd_bytes[i] < 0x10){
        // Serial print doesn't print leading zeroes, we want to see them
        Serial.print("0");
      }
      Serial.print(cmd_bytes[i], HEX);
      Serial.print(" ");
    }
    
    Serial.println("");
    Serial.print("bytes as bits:   ");
    for (i=0; i<bytes; i++) {
      Serial.print(" ");
      printByteInBits(cmd_bytes[i]);
    }
    Serial.println("");
  } else {
    Serial.println("not toshiba?");
    Serial.print("(rawlen-3)/2: ");
    Serial.println((results->rawlen - 3)/2, DEC);
  }

  // Comment
  Serial.print("  // ");
  encoding(results);
  Serial.print(" ");
  ircode(results);

  // Newline
  Serial.println("");

  // All protocols have data
  Serial.print("unsigned int  data = 0x");
  Serial.print(results->value, HEX);
  Serial.println(";");

}

//+=============================================================================
// The repeating section of the code
//
void  loop ( )
{
  decode_results  results;        // Somewhere to store the results

  if (irrecv.decode(&results)) {  // Grab an IR code
    dumpInfo(&results);           // Output the results
    dumpRaw(&results);            // Output the results in RAW format
    dumpCode(&results);           // Output the results as source code
    Serial.println("");           // Blank line between entries
    irrecv.resume();              // Prepare for the next value
  }
}
