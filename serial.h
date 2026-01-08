#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32

#include <windows.h>

#else

#include <unistd.h>
#include <strings.h>

typedef int HANDLE;
typedef uint32_t DWORD;
#define INVALID_HANDLE_VALUE (-1)
#define Sleep(x) usleep(1000 * x)
#define _stricmp strcasecmp

#endif


HANDLE serial_open_port(const char* device, uint32_t baud_rate);

void serial_close_port(HANDLE port);

int serial_write_port(HANDLE port, uint8_t* buffer, DWORD size);

int serial_read_port(HANDLE port, uint8_t* buffer, DWORD size);

#endif
