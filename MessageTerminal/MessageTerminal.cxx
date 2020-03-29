#include <unistd.h>

#include <iostream>
#include <sstream>

#include "LinuxSerialDevice.h"
#include "TestStandCommHost.h"

#include "shared_defs.h"

#include "macros.h"

#define BAUD_RATE 115200

#define RECV_MSG_TIMEOUT 5000

#define BASIC_CMD(_name)                     \
void _name(istringstream& iss)               \
{                                            \
    if (comm._name()) {                      \
        cout << "OK" << endl;                \
    }                                        \
    else {                                   \
        printf("ERR: Received MSG ID: %d\n", \
                comm.received_message().id); \
    }                                        \
}

using namespace std;

LinuxSerialDevice device;
TestStandCommHost comm(device);

typedef void (*cmd_handler)(istringstream& iss);

BASIC_CMD(ping);
BASIC_CMD(home);
BASIC_CMD(stop);

void move(istringstream& iss)
{
    uint32_t accel, hold_vel, dist;
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

        if (comm.move(accel, hold_vel, dist, axis, dir)) {
            cout << "OK" << endl;
        }
        else {
            printf("ERR: Received MSG ID: %d\n", comm.received_message().id);
            return;
        }

        return;
    } while(0);

    cout << "usage: move <accel> <hold_vel> <dist> <x|y> <pos|neg>" << endl;
}

void get_status(istringstream& iss)
{
    if (comm.get_status()) {
        cout << "OK" << endl;
    }
    else {
        printf("ERR: Received MSG ID: %d\n", comm.received_message().id);
        return;
    }

    if (comm.recv_message(RECV_MSG_TIMEOUT)) {
        if (comm.received_message().id == MSG_ID_STATUS) {
            Status status = (Status)((comm.received_message().data)[0]);
            switch (status) {
                case STATUS_IDLE:   puts("Status: IDLE"); break;
                case STATUS_MOVING: puts("Status: MOVING"); break;
                case STATUS_HOMING: puts("Status: HOMING"); break;
                default:            puts("ERR: Invalid status"); break;
            }
        }
        else {
            printf("ERR: Received MSG ID: %d\n", comm.received_message().id);
        }
    }
    else {
        printf("ERR: No response\n");
    }
}

void get_data(istringstream& iss)
{
    string data_word;
    DataId data_id;

    do {
        if (!iss.good()) break;
        iss >> data_word;
        if (data_word == "MOTOR") data_id = DATA_MOTOR;
        else if (data_word == "TEMP") data_id = DATA_TEMP;
        else break;

        if (comm.get_data(data_id)) {
            cout << "OK" << endl;
        }
        else {
            printf("ERR: Received MSG ID: %d\n", comm.received_message().id);
            return;
        }

        if (comm.recv_message(RECV_MSG_TIMEOUT) && comm.received_message().id == MSG_ID_DATA) {
            switch (data_id) {
                case DATA_MOTOR:
                {
                    uint8_t *data = comm.received_message().data;
                    uint32_t motor_x = NTOHL(data);
                    uint32_t motor_y = NTOHL(data + 4);
                    printf("Motor Position: (%u, %u)\n", motor_x, motor_y);
                    break;
                }
                case DATA_TEMP:
                    // TODO
                    break;
            }
        }
        else {
            printf("ERR: Received MSG ID: %d\n", comm.received_message().id);
        }

        return;
    } while (0);

    cout << "usage: data <MOTOR|TEMP>" << endl;
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

bool connect_to_arduino()
{
    if (!device.ser_connect(BAUD_RATE)) return false;
    device.ser_flush();

    cout << "Waiting for Arduino..." << flush;
    while (!(comm.check_for_message() && comm.received_message().id == MSG_ID_PING)) {
        // Wait for ping
    }
    // There might be more ping messages sitting in the buffer, so flush them all out
    device.ser_flush();

    cout << "Connected!" << endl;
    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("\nusage: %s <serial device file>\n\nexample:\n    %s /dev/ttyS3\n\n", argv[0], argv[0]);
        return 0;
    }

    device.set_device_file(argv[1]);

    if (!connect_to_arduino()) return 1;

    bool exit = false;
    while (!exit) {
        // Output prompt
        cout << "> ";

        // Read line
        string line;
        getline(cin, line);

        if (line.empty()) {
            continue;
        }

        // Split line into words
        istringstream iss(line);
        string word;

        // Get first word
        iss >> word;

        // Check for special commands first
        if (word == "exit") {
            exit = true;
        }
        else if (word == "reset") {
            device.ser_disconnect();
            if (!connect_to_arduino()) return 1;
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