#include <stdio.h>

#include "LinuxSerialDevice.h"
#include "TestStandComm.h"

#define BAUD_RATE 115200

int main()
{
    printf("MessageTerminal Started\n");

    LinuxSerialDevice device("/dev/ttyS3");
    TestStandComm comm(&device);

    if (!device.ser_connect(BAUD_RATE)) return 1;

    if (comm.ping()) {
        printf("PING Successful!");
    } else {
        printf("PING failed!");
    }

    device.ser_disconnect();

    return 0;
}