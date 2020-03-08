#include <stdio.h>
#include <unistd.h>

#include "LinuxSerialDevice.h"
#include "TestStandCommHost.h"

#define BAUD_RATE 115200

int main()
{
    printf("MessageTerminal Started\n");

    LinuxSerialDevice device("/dev/ttyS3");
    TestStandCommHost comm(&device);

    if (!device.ser_connect(BAUD_RATE)) return 1;

    while (true) {
        if (comm.ping()) {
            printf("PING Successful!\n");
        } else {
            printf("PING failed!\n");
        }
        sleep(1);
    }

    device.ser_disconnect();

    return 0;
}