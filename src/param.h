#ifndef _PARAM_H
#define _PARAM_H

#include "global.h"

#define PARAM_MODE_TRANSFER     0
#define PARAM_MODE_PING_PLIP    1
#define PARAM_MODE_PING_SLIP    2
#define PARAM_MODE_TOTAL_NUMBER 3

typedef struct {
  u08 mode;
} param_t;
  
extern param_t param;  

// param result
#define PARAM_OK                  0
#define PARAM_EEPROM_NOT_READY    1
#define PARAM_EEPROM_CRC_MISMATCH 2

// init parameters. try to load from eeprom or use default
void param_init(void);
// save param to eeprom (returns param result) 
u08 param_save(void);
// load param from eeprom (returns param result)
u08 param_load(void);
// reset param
void param_reset(void);
// show params
void param_dump(void);

#endif
