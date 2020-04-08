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
#include <time.h>

LinuxSerialDevice::LinuxSerialDevice()
{
    // Nothing to do
}

void LinuxSerialDevice::set_device_file(const char *device_file)
{
    this->device_file = device_file;
}

static speed_t get_termios_baud_rate(SerialBaudRate baud_rate)
{
    switch (baud_rate) {
        case BAUD_9600   : return B9600;
        case BAUD_19200  : return B19200;
        case BAUD_38400  : return B38400;
        case BAUD_57600  : return B57600;
        case BAUD_115200 : return B115200;
        case BAUD_230400 : return B230400;
        case BAUD_460800 : return B460800;
        case BAUD_500000 : return B500000;
        default          : return 0;
    }
}

bool LinuxSerialDevice::ser_connect(SerialBaudRate baud_rate)
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
    tty.c_lflag &= ~IEXTEN;   // Disable interpretation of VDISCARD=SI and VLNEXT=SYN
    // Input Modes
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                          // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    // Output Modes
    tty.c_oflag &= ~OPOST;    // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR;    // Prevent conversion of newline to carriage return/line feed

    // Configure for non-blocking (return immediately with available data)
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;

    // Baud rate
    speed_t termios_baud_rate = get_termios_baud_rate(baud_rate);
    if (termios_baud_rate == 0) {
        printf("Error: Invalid baud rate: %d\n", baud_rate);
        return false;
    }
    cfsetispeed(&tty, termios_baud_rate);
    cfsetospeed(&tty, termios_baud_rate);

    // Save tty settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return false;
    }

    return true;
}

void LinuxSerialDevice::ser_flush()
{
    // Wait for 10 ms for last bits of data
    usleep(10000);
    tcflush(this->serial_port, TCIOFLUSH);
}

uint32_t LinuxSerialDevice::ser_available()
{
    uint32_t bytes_avail;
    ioctl(this->serial_port, FIONREAD, &bytes_avail);
    return bytes_avail;
}

bool LinuxSerialDevice::ser_read(uint8_t *out)
{
    return (read(this->serial_port, out, 1) == 1);
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
    struct timespec monotime;
    // Use CLOCK_MONOTONIC (rather than CLOCK_REALTIME) since we don't care about
    // absolute time, just elapsed time
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    return ((monotime.tv_sec * 1000) + (monotime.tv_nsec / 1E6));
}