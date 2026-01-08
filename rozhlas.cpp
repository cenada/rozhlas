#include <stdio.h>
#include <stdlib.h>

#include "modbus.h"

void print_usage_and_exit()
{
    printf("usage: rozhlas SERIAL_PORT ON|OFF\n");
    exit(0);
}

int main(int argc, const char** argv)
{
    uint8_t state = 0;

    if (argc < 3)
        print_usage_and_exit();
     
    if (modbus_open(argv[1]) < 0)
        return 0;

    if (_stricmp(argv[2], "ON") == 0)
        state = STATE_ON;
    else if (_stricmp(argv[2], "OFF") == 0)
        state = STATE_OFF;
    else
        print_usage_and_exit();

    if (turn_all_4_channels(state) == 0)
        printf("success turning radio %s\n", (state == STATE_ON)?"on":"off");

    modbus_close();

    return 0;
}
