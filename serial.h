#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

HANDLE serial_open_port(const char* device, uint32_t baud_rate);

void serial_close_port(HANDLE port);

int serial_write_port(HANDLE port, uint8_t* buffer, DWORD size);

int serial_read_port(HANDLE port, uint8_t* buffer, DWORD size);

#endif
