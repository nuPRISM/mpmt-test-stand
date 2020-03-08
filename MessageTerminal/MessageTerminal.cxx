#include <unistd.h>

#include <iostream>
#include <sstream>

#include "LinuxSerialDevice.h"
#include "TestStandCommHost.h"

#define BAUD_RATE 115200

#define BASIC_CMD(_name)                    \
void _name(istringstream& iss)        \
{                                           \
    if (comm._name()) cout << "OK" << endl; \
    else cout << "ERROR" << endl;           \
}

using namespace std;

LinuxSerialDevice device("/dev/ttyS3");
TestStandCommHost comm(&device);

typedef void (*cmd_handler)(istringstream& iss);

BASIC_CMD(ping);
BASIC_CMD(get_status);
BASIC_CMD(home);
BASIC_CMD(stop);

void move(istringstream& iss)
{
    uint16_t accel, hold_vel, dist;
    AxisId axis;
    Direction dir;

    do {
        // accel, hold_vel, dist
        if (!iss.good()) break;
        iss >> accel;
        if (!iss.good()) break;
        iss >> hold_vel;
        if (!iss.good()) break;
        iss >> dist;

        string word;

        // axis
        if (!iss.good()) break;
        iss >> word;
        if (word == "x") axis = AXIS_X;
        else if (word == "y") axis = AXIS_Y;
        else break;

        // dir
        if (!iss.good()) break;
        iss >> word;
        if (word == "pos") dir = DIR_POSITIVE;
        else if (word == "neg") dir = DIR_NEGATIVE;
        else break;

        printf("accel: %d\n", accel);
        printf("hold_vel: %d\n", hold_vel);
        printf("dist: %d\n", dist);
        printf("axis: %d\n", axis);
        printf("dir: %d\n", dir);

        if (comm.move(accel, hold_vel, dist, axis, dir)) {
            cout << "OK" << endl;
        }
        else {
            cout << "ERROR" << endl;
        }
        return;
    } while(0);

    cout << "usage: move <accel> <hold_vel> <dist> <x|y> <pos|neg>" << endl;
}

void get_data(istringstream& iss)
{
    // TODO parse arguments

    if (comm.ping()) {
        cout << "OK" << endl;
    }
    else {
        cout << "ERROR" << endl;
    }
}

cmd_handler get_cmd_handler(const string& cmd_name)
{
    if (cmd_name == "ping")       return ping;
    if (cmd_name == "get_status") return get_status;
    if (cmd_name == "home")       return home;
    if (cmd_name == "move")       return move;
    if (cmd_name == "stop")       return stop;
    if (cmd_name == "get_data")   return get_data;

    return nullptr;
}

int main()
{
    if (!device.ser_connect(BAUD_RATE)) return 1;

    bool exit = false;
    while (!exit) {
        // Output prompt
        cout << "> ";

        // Read line
        string line;
        getline(cin, line);

        // Split line into words
        istringstream iss(line);
        string word;

        // Get first word
        iss >> word;

        // Check for exit
        if (word == "exit") {
            exit = true;
        }
        else {
            // Handle command
            cmd_handler handler = get_cmd_handler(word);
            if (handler != nullptr) {
                handler(iss);
            }
            else {
                cout << "Invalid command" << endl;
            }
        }
    }

    device.ser_disconnect();

    return 0;
}