#include "LinuxSerialDevice.h"

// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/ioctl.h> // ioctl()
#include <sys/time.h>

LinuxSerialDevice::LinuxSerialDevice()
{
    // Nothing to do
}

void LinuxSerialDevice::set_device_file(const char *device_file)
{
    this->device_file = device_file;
}

bool LinuxSerialDevice::ser_connect(uint32_t baud_rate)
{
    // Reference: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

    // Open serial port device file
    this->serial_port = open(this->device_file, O_RDWR);
    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return false;
    }

    // Create new termios struct
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    // Read in existing settings
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return false;
    }

    // Control Modes
    tty.c_cflag &= ~PARENB;   // Don't check parity
    tty.c_cflag &= ~CSTOPB;   // Only use 1 stop bit
    tty.c_cflag |= CS8;       // 8 bits per byte
    tty.c_cflag &= ~CRTSCTS;  // Disable hardware flow control
    tty.c_cflag |= CLOCAL;    // Disable modem-specific signal lines
    tty.c_cflag |= CREAD;     // Enable reading data
    // Local Modes
    tty.c_lflag &= ~ICANON;   // Disable canonical mode (i.e. enable raw mode)
    tty.c_lflag &= ~ECHO;     // Disable echo
    tty.c_lflag &= ~ECHOE;    // Disable erasure
    tty.c_lflag &= ~ECHONL;   // Disable new-line echo
    tty.c_lflag &= ~ISIG;     // Disable interpretation of INTR, QUIT and SUSP
    // Input Modes
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                          // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    // Output Modes
    tty.c_oflag &= ~OPOST;    // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR;    // Prevent conversion of newline to carriage return/line feed

    // Configure for non-blocking (return immediately with available data)
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;

    // Set baud rate
    cfsetispeed(&tty, baud_rate);
    cfsetospeed(&tty, baud_rate);

    // Save tty settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return false;
    }

    return true;
}

uint32_t LinuxSerialDevice::ser_available()
{
    uint32_t bytes_avail;
    ioctl(this->serial_port, FIONREAD, &bytes_avail);
    return bytes_avail;
}

uint8_t LinuxSerialDevice::ser_read()
{
    uint8_t byte_in;
    read(this->serial_port, &byte_in, 1);
    return byte_in;
}

bool LinuxSerialDevice::ser_write(uint8_t *data, uint32_t length)
{
    return (write(this->serial_port, data, length) == length);
}

void LinuxSerialDevice::ser_disconnect()
{
    close(this->serial_port);
}

uint64_t LinuxSerialDevice::platform_millis()
{
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return (tval.tv_usec / 1000);
}