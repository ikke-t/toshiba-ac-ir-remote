/*
 * Toshiba AC remote for Arduino
 * Works with my Toshiba Heat Pump RAS-10PKVP-ND, emulates remote WH-H07JE
 *
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 1, June, 2019
 * Author: 2019, Ilkka Tengvall
 * License: GPLv3
 *
 * Program waits for JSON input with command data. Possible values are:
 *   cmd:  on | off | swing | hipwr | sleep | vertical | stats
 *   temp: xx °celcius, where xx=17-30°C
 *   mode: auto | cool | dry | heat
 *   fan:  auto | 1 | 2 | 3 | 4 | 5
 *   pure: on | off
 *
 * For example:
 *   { "cmd":"on", "temp":"23", "mode":"auto", "fan": "auto" }
 *   { "cmd":"off" }
 *   { "cmd":"swing" }
 *   { "cmd":"hipwr" }
 *   { "cmd":"sleep" }
 *   { "cmd":"vertical" }
 *   { "cmd": "stats"}
 *
 * Program will acknowledge the command by sending it back as received,
 * and will report status in JSON, e.g:
 *   { "cmd": "swing", "status": "ok"}
 *   {"status": "fail", "error":"unknown command"}
 */


#include <IRremote.h>
#include <ir_Toshiba.h>
#include <ArduinoJson.h>

const size_t JSON_MAX_LEN = 200;
const size_t RESULT_MAX_LEN = 200;

IRsend irsend;
struct state {
  int count = 0;
  byte cmd = 0;
  byte temp = 0;
  byte mode = 0;
  int hp = 0;
  int off = 0;
  int swing = 0;
  int hipwr = 0;
  int fan = 0;
  int pure = 0;
  int sleep = 0;
  int vertical = 0;
  int unknown = 0;
} in_state;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial) delay(50);
}

void loop() {
  IRsend::ir_toshiba_cmd_s cmd;
  char result[RESULT_MAX_LEN];

  DynamicJsonDocument doc(JSON_MAX_LEN);
  DynamicJsonDocument stats(RESULT_MAX_LEN);

  auto error = deserializeJson(doc, Serial);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  const char* in_cmd = doc["cmd"];
  const char* in_temp = doc["temp"];
  const char* in_mode = doc["mode"];
  const char* in_fan = doc["fan"];
  const char* in_pure = doc["pure"];

  serializeJson(doc, Serial);

  Serial.println("");

  // Clear out command
  cmd.values.bytes[0] = 0;
  cmd.values.bytes[1] = 0;
  cmd.values.bytes[2] = 0;

  // Send ON command with temperature, AC mode and fan speed settings
  if (!strcmp(in_cmd, "on")) {

    in_state.cmd = IRsend::HEAT_PUMP_CMD;
    cmd.type = IRsend::HEAT_PUMP_CMD;

    in_state.temp = atoi(in_temp);
    cmd.values.field.temp = in_state.temp - TOSHIBA_HEAT_BASE;
    if (cmd.values.field.temp < 0 || cmd.values.field.temp > 13) {
      Serial.println("{\"status\":\"fail\",\"error\":\"temperature out of range\"}");
      return;
    }

    // Set AC mode
    if (!strcmp(in_mode, "auto"))      cmd.values.field.mode = TOSHIBA_MODE_AUTO;
    else if (!strcmp(in_mode, "cool")) cmd.values.field.mode = TOSHIBA_MODE_COOL;
    else if (!strcmp(in_mode, "heat")) cmd.values.field.mode = TOSHIBA_MODE_HEAT;
    else if (!strcmp(in_mode, "dry"))  cmd.values.field.mode = TOSHIBA_MODE_DRY;
    else {
      Serial.println("{\"status\":\"fail\",\"error\":\"invalid mode\"}");
      return;
    }
    in_state.mode = cmd.values.field.mode;

    // Set fan speed
    if (!strcmp(in_fan, "auto"))   cmd.values.field.fan = TOSHIBA_FAN_AUTO;
    else if (!strcmp(in_fan, "1")) cmd.values.field.fan = TOSHIBA_FAN_1;
    else if (!strcmp(in_fan, "2")) cmd.values.field.fan = TOSHIBA_FAN_2;
    else if (!strcmp(in_fan, "3")) cmd.values.field.fan = TOSHIBA_FAN_3;
    else if (!strcmp(in_fan, "4")) cmd.values.field.fan = TOSHIBA_FAN_4;
    else if (!strcmp(in_fan, "5")) cmd.values.field.fan = TOSHIBA_FAN_5;
    else {
      Serial.println("{\"status\":\"fail\",\"error\":\"invalid fan speed\"}");
      return;
    }
    in_state.fan = cmd.values.field.fan;

    irsend.sendTOSHIBA(cmd);

    in_state.hp++;
    snprintf( result, sizeof(result),
      "{\"cmd\":\"on\",\"temp\":\"%i\",\"mode\":\"%s\",\"fan\":\"%s\",\"status\":\"ok\"}",
      cmd.values.field.temp, in_mode, in_fan);
    Serial.println(result);

  } // Send OFF command to turn off the AC
  else if (!strcmp(in_cmd, "off")) {

    cmd.type = IRsend::HEAT_PUMP_CMD;
    cmd.values.field.mode = TOSHIBA_MODE_OFF;

    irsend.sendTOSHIBA(cmd);

    in_state.off++;
    Serial.println("{\"cmd\":\"off\",\"status\":\"ok\"}");

  } // Send change swing horizontal direction command
  else if (!strcmp(in_cmd, "swing")) {
    in_state.cmd = IRsend::SWING_CMD;
    cmd.type = IRsend::SWING_CMD;

    irsend.sendTOSHIBA(cmd);

    in_state.swing++;
    Serial.println("{\"cmd\":\"swing\",\"status\":\"ok\"}");

  } // Send sleep command
  else if (!strcmp(in_cmd, "sleep")) {
    in_state.cmd = IRsend::SLEEP_CMD;
    cmd.type = IRsend::SLEEP_CMD;

    irsend.sendTOSHIBA(cmd);

    in_state.sleep++;
    Serial.println("{\"cmd\":\"sleep\",\"status\":\"ok\"}");

  } // Send high power command
  else if (!strcmp(in_cmd, "hipwr")) {
    in_state.cmd = IRsend::HI_PWR_CMD;
    cmd.type = IRsend::HI_PWR_CMD;

    irsend.sendTOSHIBA(cmd);

    in_state.hipwr++;
    Serial.println("{\"cmd\":\"hipwr\",\"status\":\"ok\"}");

  } // Send change vertical direction command
  else if (!strcmp(in_cmd, "vertical")) {
    in_state.cmd = IRsend::UP_DOWN_CMD;
    cmd.type = IRsend::UP_DOWN_CMD;

    irsend.sendTOSHIBA(cmd);

    in_state.vertical++;
    Serial.println("{\"cmd\":\"vertical\",\"status\":\"ok\"}");

  } // Send change vertical direction command
  else if (!strcmp(in_cmd, "stats")) {
    stats["count"] = in_state.count;
    stats["cmd"] = (int)in_state.cmd;
    stats["temp"] = (int)in_state.temp;
    stats["mode"] = (int)in_state.mode;
    stats["hp"] = in_state.hp;
    stats["off"] = in_state.off;
    stats["swing"] = in_state.swing;
    stats["hipwr"] = in_state.hipwr;
    stats["fan"] = in_state.fan;
    stats["pure"] = in_state.pure;
    stats["sleep"] = in_state.sleep;
    stats["vertical"] = in_state.vertical;
    serializeJson(stats, Serial);
    Serial.println("");
 } // Did we receive some garbage?
  else {
    in_state.unknown++;
    Serial.println("{\"status\":\"fail\",\"error\":\"unknown command\"}");
    return;
  }
  in_state.count++;
}
