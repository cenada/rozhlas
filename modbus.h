#ifndef _MODBUS_H_
#define _MODBUS_H_

#include <stdint.h>

#include "serial.h"

#define STATE_ON  1
#define STATE_OFF 2

int modbus_open(const char* port);
void modbus_close();

int turn_all_4_channels(uint8_t state);

#endif
