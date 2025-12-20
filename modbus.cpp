#include "serial.h"

#define MODB_SLAVE_ID  1

uint8_t slave_id = MODB_SLAVE_ID;
uint8_t packet[8];
uint8_t response[21];

HANDLE hSerialPort;

int modbus_open(const char* port)
{
    hSerialPort = serial_open_port(port, 9600);
    if (hSerialPort == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Could not open modbus serial port %s\n", port);
        return -1;
    }
    return 0;
}

void modbus_close()
{
    serial_close_port(hSerialPort);
}

/* CRC-16/MODBUS (Modbus RTU)
 * init 0xFFFF, poly 0xA001, LSB-first, output little-endian in frame.
 */
uint16_t modbus_crc16(const uint8_t* data, size_t len)
{
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
    }
    return crc;
}

int turn_all_4_channels(uint8_t state)
{  
    packet[0] = slave_id;
    packet[1] = 6;
    packet[2] = 0;
    
    packet[4] = state;
    packet[5] = 0;    
    for (uint8_t channel = 1; channel <= 4; channel++)
    {
        packet[3] = channel; 
        uint16_t crc = modbus_crc16(packet, 6);
        packet[6] = crc & 255;
        packet[7] = crc >> 8;

        if (serial_write_port(hSerialPort, packet, 8) < 0)
        {
            fprintf(stderr, "could not write to modbus port\n");
            return -1;
        }

        int bytes_read = 0;
        if ((bytes_read = serial_read_port(hSerialPort, response, 21)) < 0)
        {
            fprintf(stderr, "could not read from modbus port\n");
            return -1;
        }

        if ((response[0] != slave_id) ||
            (response[1] != 6) ||
            (response[2] != 0))
        {
            fprintf(stderr, "unexpected response read from modbus port\n");
            return -1;
        }
        Sleep(10);
    }
    return 0;
}
