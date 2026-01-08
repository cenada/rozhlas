#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "../serial.h"

// NOTE ABOUT TYPES (serial.h):
// On Windows, serial.h uses HANDLE and DWORD from windows.h.
// For Linux you will typically want something like:
//
//   #ifndef _WIN32
//   typedef int HANDLE;          // file descriptor returned by open()
//   typedef uint32_t DWORD;
//   #define INVALID_HANDLE_VALUE (-1)
//   #endif
//
// and remove the unconditional #include <windows.h>.
//
// This file assumes HANDLE is an int file descriptor and INVALID_HANDLE_VALUE is -1.

static void print_error(const char* context)
{
    fprintf(stderr, "%s: %s\n", context, strerror(errno));
}

static speed_t baud_to_speed(uint32_t baud_rate)
{
    switch (baud_rate)
    {
        case 0: return B0;
        case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
#ifdef B57600
        case 57600: return B57600;
#endif
#ifdef B115200
        case 115200: return B115200;
#endif
#ifdef B230400
        case 230400: return B230400;
#endif
#ifdef B460800
        case 460800: return B460800;
#endif
#ifdef B500000
        case 500000: return B500000;
#endif
#ifdef B576000
        case 576000: return B576000;
#endif
#ifdef B921600
        case 921600: return B921600;
#endif
#ifdef B1000000
        case 1000000: return B1000000;
#endif
#ifdef B1152000
        case 1152000: return B1152000;
#endif
#ifdef B1500000
        case 1500000: return B1500000;
#endif
#ifdef B2000000
        case 2000000: return B2000000;
#endif
#ifdef B2500000
        case 2500000: return B2500000;
#endif
#ifdef B3000000
        case 3000000: return B3000000;
#endif
#ifdef B3500000
        case 3500000: return B3500000;
#endif
#ifdef B4000000
        case 4000000: return B4000000;
#endif
        default:
            // Many systems only support the standard rates above without termios2/BOTHER.
            // If you need arbitrary baud rates, consider using Linux termios2 (ioctl TCGETS2/TCSETS2).
            errno = EINVAL;
            return (speed_t)0;
    }
}

HANDLE serial_open_port(const char* device, uint32_t baud_rate)
{
    if (!device)
    {
        errno = EINVAL;
        print_error("serial_open_port: device is null");
        return INVALID_HANDLE_VALUE;
    }

    // Open the serial device.
    // O_NOCTTY: do not become controlling terminal
    // O_SYNC:   write calls block until data has been transmitted (best-effort)
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        print_error(device);
        return INVALID_HANDLE_VALUE;
    }

    // Flush away any bytes previously read or written (similar to FlushFileBuffers on Windows).
    if (tcflush(fd, TCIOFLUSH) != 0)
    {
        print_error("Failed to flush serial port");
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

    struct termios tio;
    if (tcgetattr(fd, &tio) != 0)
    {
        print_error("Failed to get serial settings");
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

    // Put the port into "raw" mode (8N1, no echo, no canonical processing).
    cfmakeraw(&tio);

    // 8 data bits
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;

    // No parity
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~PARODD;

    // 1 stop bit
    tio.c_cflag &= ~CSTOPB;

    // Disable hardware flow control (RTS/CTS)
#ifdef CRTSCTS
    tio.c_cflag &= ~CRTSCTS;
#endif

    // Enable receiver, ignore modem control lines
    tio.c_cflag |= (CLOCAL | CREAD);

    // Disable software flow control
    tio.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Configure read timeout ~100 ms (like the Windows code's 100 ms ReadTotalTimeoutConstant).
    // VMIN=0: return as soon as any data is available (or 0 on timeout)
    // VTIME=1: 0.1 seconds timeout
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;

    speed_t spd = baud_to_speed(baud_rate);
    if (spd == (speed_t)0 && baud_rate != 0)
    {
        print_error("Unsupported baud rate");
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

    if (cfsetispeed(&tio, spd) != 0 || cfsetospeed(&tio, spd) != 0)
    {
        print_error("Failed to set baud rate");
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

    if (tcsetattr(fd, TCSANOW, &tio) != 0)
    {
        print_error("Failed to set serial settings");
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

    // Ensure settings take effect immediately.
    if (tcflush(fd, TCIOFLUSH) != 0)
    {
        print_error("Failed to flush serial port after configuration");
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

    return fd;
}

int serial_write_port(HANDLE port, uint8_t* buffer, DWORD size)
{
    if (port == INVALID_HANDLE_VALUE)
    {
        errno = EBADF;
        print_error("Failed to write to port");
        return -1;
    }
    if (!buffer && size != 0)
    {
        errno = EINVAL;
        print_error("Failed to write to port");
        return -1;
    }

    const int timeout_ms = 100; // match Windows code's WriteTotalTimeoutConstant

    size_t total = 0;
    while (total < (size_t)size)
    {
        struct pollfd pfd;
        pfd.fd = port;
        pfd.events = POLLOUT;
        pfd.revents = 0;

        int pr = poll(&pfd, 1, timeout_ms);
        if (pr < 0)
        {
            if (errno == EINTR) continue;
            print_error("Failed to write to port");
            return -1;
        }
        if (pr == 0)
        {
            errno = ETIMEDOUT;
            print_error("Failed to write all bytes to port");
            return -1;
        }

        ssize_t n = write(port, buffer + total, (size_t)size - total);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            print_error("Failed to write to port");
            return -1;
        }
        if (n == 0)
        {
            errno = EIO;
            print_error("Failed to write all bytes to port");
            return -1;
        }
        total += (size_t)n;
    }

    return 0;
}

int serial_read_port(HANDLE port, uint8_t* buffer, DWORD size)
{
    if (port == INVALID_HANDLE_VALUE)
    {
        errno = EBADF;
        print_error("Failed to read from port");
        return -1;
    }
    if (!buffer && size != 0)
    {
        errno = EINVAL;
        print_error("Failed to read from port");
        return -1;
    }

    // With VMIN=0, VTIME=1 configured in serial_open_port(), read() will block up to ~100 ms.
    // It may return fewer than 'size' bytes (including 0 on timeout), mirroring the Windows code
    // that performs a single ReadFile call.
    ssize_t n = read(port, buffer, (size_t)size);
    if (n < 0)
    {
        if (errno == EINTR) return 0; // treat interrupt as "no data" for a synchronous polling style
        print_error("Failed to read from port");
        return -1;
    }
    return (int)n;
}

void serial_close_port(HANDLE port)
{
    if (port == INVALID_HANDLE_VALUE) return;
    close(port);
    printf("closed port\n");
}
