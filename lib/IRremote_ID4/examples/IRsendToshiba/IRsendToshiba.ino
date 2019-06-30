/*
 * IRremote: ToshibaSend - demonstrates sending Toshiba heat pump
 * RAS-10PKVP-ND IR codes with IRsend.
 * An IR LED must be connected to Arduino PWM pin 3.
 *
 * Do not send the commands to real heat pump, but you can test sending
 * and reading data back by using IRrecvDumpToshiba. You can verify
 * the received signals by comparing to data in file:
 * ../IRrecvDumpToshiba/codes_from_toshiba.txt
 *
 * Version 0.1 May, 2016
 * Copyright 2016, Ilkka Tengvall
 */


#include <IRremote.h>

IRsend irsend;

void setup()
{
}

void loop() {
  IRsend::ir_toshiba_cmd_s cmd;
  cmd.values.bytes[0] = 0;
  cmd.values.bytes[1] = 0;
  cmd.values.bytes[2] = 0;
  int j, delay_ms=500;

  cmd.type               = IRsend::HEAT_PUMP_CMD;
  cmd.values.field.fan   = TOSHIBA_FAN_AUTO;
  cmd.values.field.mode  = TOSHIBA_MODE_AUTO;
  cmd.values.field.pure  = TOSHIBA_PURE_OFF;

  // sample temp settings
  for (j=30; j >= TOSHIBA_HEAT_BASE; j--) {
    cmd.values.field.temp = j - TOSHIBA_HEAT_BASE;
    irsend.sendTOSHIBA(cmd);
    delay(delay_ms);
  }

  // sample mode settings
  for (j=0; j < 4 ; j++) {
    cmd.values.field.mode = j;
    irsend.sendTOSHIBA(cmd);
    delay(delay_ms);
  }
  // sample power off
  cmd.values.field.mode  = 7;
  irsend.sendTOSHIBA(cmd);
  delay(delay_ms);

  cmd.values.field.mode = 0;

  // sample fan settings
  for (j=0; j < 7 ; j++) {
    if (j==1) continue;
    cmd.values.field.fan = j;
    irsend.sendTOSHIBA(cmd);
    delay(delay_ms);
  }
  cmd.values.field.fan = 0;

  // sample pure on
  cmd.values.field.pure = 1;
  irsend.sendTOSHIBA(cmd);
  delay(delay_ms);
  cmd.values.field.pure = 0;

  cmd.type = IRsend::SWING_CMD;
  irsend.sendTOSHIBA(cmd);
  delay(delay_ms);

  cmd.type = IRsend::HI_PWR_CMD;
  irsend.sendTOSHIBA(cmd);
  delay(delay_ms);

  cmd.type = IRsend::SLEEP_CMD;
  irsend.sendTOSHIBA(cmd);
  delay(delay_ms);

  cmd.type = IRsend::UP_DOWN_CMD;
  irsend.sendTOSHIBA(cmd);
  delay(delay_ms);

  // send out loop marker
  cmd.values.bytes[0] = 0xff;
  cmd.values.bytes[1] = 0xff;
  cmd.values.bytes[2] = 0xff;
  cmd.type = IRsend::HEAT_PUMP_CMD;
  for (j=0; j<5; j++) {
    irsend.sendTOSHIBA(cmd);
    delay(delay_ms);
  }
    
  delay(5000); //5 second delay between each signal burst
}
