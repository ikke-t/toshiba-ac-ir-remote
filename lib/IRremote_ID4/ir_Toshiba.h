//******************************************************************************
// ir_Toshiba.h
//
// Toshiba A/C RAS-10PKVP-ND by Ilkka Tengvall
//
//******************************************************************************

typedef enum TOSHIBA_CMD :byte {
    HEAT_PUMP_CMD = 1,
    SWING_CMD,
    HI_PWR_CMD,
    SLEEP_CMD,
    UP_DOWN_CMD
} toshiba_cmd_e;

#define TOSHIBA_HEAT_BASE 17
#define TOSHIBA_FAN_AUTO   0
#define TOSHIBA_FAN_1      2
#define TOSHIBA_FAN_2      3
#define TOSHIBA_FAN_3      4
#define TOSHIBA_FAN_4      5
#define TOSHIBA_FAN_5      6
#define TOSHIBA_MODE_AUTO  0
#define TOSHIBA_MODE_COOL  1
#define TOSHIBA_MODE_DRY   2
#define TOSHIBA_MODE_HEAT  3
#define TOSHIBA_MODE_OFF   7
#define TOSHIBA_PURE_ON    1
#define TOSHIBA_PURE_OFF   0

typedef union {
  byte bytes[3];       // clear all values -> set to zero
  struct {
    // byte 1
    byte byte5  :4;    // unused, always 0
    byte temp   :4;    // 17-30 degrees, value as: celsius - 17 = 0-13
    // byte 2
    byte mode   :4;    // 0=auto, 1=cool, 2=dry, 3=heat 7=power off
    byte byte6  :1;    // unused, always 0
    byte fan    :3;    // 0=auto, 2=*, 3=**, 4=***, 5=****, 6=*****
    // byte 3
    byte byte72 :4;    // unused, always 0
    byte pure   :1;    // 1=on, 0=off
    byte byte71 :3;    // unused, always 0
  } field;
} toshiba_cmd_values_u;

// four byte infrared command string.
// * type   - selects the correct IR signal lenght for the command
// * values - describe the IR commands values
typedef struct {
    toshiba_cmd_e        type;
    toshiba_cmd_values_u values;
} ir_toshiba_cmd_s;
