#include "LinuxSerialDevice.h"
#include "TestStandCommHost.h"

#include "SerialResult.h"

#include "Gantry.h"

#include "shared_defs.h"
#include "TestStandMessages.h"

#include <unistd.h>

#include <iostream>
#include <sstream>

using namespace std;

/*****************************************************************************/
/*                                  DEFINES                                  */
/*****************************************************************************/

#define BASIC_CMD(_name)                     \
bool _name(istringstream& iss)               \
{                                            \
    SerialResult res = comm._name();         \
    if (res == SERIAL_OK) {                  \
        cout << "OK" << endl;                \
    }                                        \
    else {                                   \
        printf("ERROR: %d\n", res);          \
    }                                        \
    return true;                             \
}

#define ITEM_COUNT(_a) (sizeof(_a) / sizeof(_a[0]))

/*****************************************************************************/
/*                                  TYPEDEFS                                 */
/*****************************************************************************/

typedef bool (*cmd_handler)(istringstream& iss);

typedef struct {
    const char *name;
    const char *description;
    const char *usage;
    cmd_handler handler;
} Command;

typedef enum {
    // Placeholder for an invalid command
    CMD_ID_INVALID = -1,
    // Test stand commands
    CMD_ID_PING,
    CMD_ID_GET_STATUS,
    CMD_ID_HOME,
    CMD_ID_MOVE,
    CMD_ID_STOP,
    CMD_ID_GET_POSITION,
    CMD_ID_GET_TEMP,
    CMD_ID_GET_AXIS_STATE,
    // General commands
    CMD_ID_LINK_CHECK,
    CMD_ID_RESET,
    CMD_ID_HELP,
    CMD_ID_EXIT
} CommandID;

/*****************************************************************************/
/*                                  GLOBALS                                  */
/*****************************************************************************/

LinuxSerialDevice device;
TestStandCommHost comm(device);

/*****************************************************************************/
/*                            FORWARD DECLARATIONS                           */
/*****************************************************************************/

bool help(istringstream& iss);
void print_cmd_usage(CommandID id);

/*****************************************************************************/
/*                             COMMAND FUNCTIONS                             */
/*****************************************************************************/

BASIC_CMD(ping);
BASIC_CMD(home);
BASIC_CMD(stop);

bool move(istringstream& iss)
{
    AxisId axis;
    AxisDirection dir;
    uint32_t vel_hold, dist;

    do {
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
        if (word == "pos") dir = AXIS_DIR_POSITIVE;
        else if (word == "neg") dir = AXIS_DIR_NEGATIVE;
        else break;

        // vel_hold, dist
        if (!iss.good()) break;
        iss >> vel_hold;
        if (!iss.good()) break;
        iss >> dist;

        AxisResult axis_res;
        SerialResult res = comm.move(axis, dir, vel_hold, dist, &axis_res, MSG_RECEIVE_TIMEOUT_MS);
        if (res == SERIAL_OK) {
            switch (axis_res) {
                case AXIS_OK:                 puts("AXIS_OK"); break;
                case AXIS_ERR_ALREADY_MOVING: puts("AXIS_ERR_ALREADY_MOVING"); break;
                case AXIS_ERR_LS_HOME:        puts("AXIS_ERR_LS_HOME"); break;
                case AXIS_ERR_LS_FAR:         puts("AXIS_ERR_LS_FAR"); break;
                case AXIS_ERR_INVALID:        puts("AXIS_ERR_INVALID"); break;
                default:                      puts("ERR: Invalid AxisResult"); break;
            }
        }
        else {
            printf("ERROR: %d\n", res);
            return true;
        }

        return true;
    } while(0);

    print_cmd_usage(CMD_ID_MOVE);
    return true;
}

bool get_status(istringstream& iss)
{
    Status status;
    SerialResult res = comm.get_status(&status, MSG_RECEIVE_TIMEOUT_MS);
    if (res == SERIAL_OK) {
        switch (status) {
            case STATUS_IDLE:   puts("Status: IDLE"); break;
            case STATUS_MOVING: puts("Status: MOVING"); break;
            case STATUS_HOMING: puts("Status: HOMING"); break;
            case STATUS_FAULT:  puts("Status: FAULT"); break;
            default:            puts("ERR: Invalid status"); break;
        }
    }
    else {
        printf("ERROR: %d\n", res);
    }
    return true;
}

bool get_position(istringstream& iss)
{
    PositionMsgData position;
    SerialResult res = comm.get_position(&position, MSG_RECEIVE_TIMEOUT_MS);
    if (res == SERIAL_OK) {
        printf("Position (counts): (%d, %d)\n", position.x_counts, position.y_counts);
    }
    else {
        printf("ERROR: %d\n", res);
    }
    return true;
}

bool get_temp(istringstream& iss)
{
    TempData temp_data;
    SerialResult res = comm.get_temp(&temp_data, MSG_RECEIVE_TIMEOUT_MS);
    if (res == SERIAL_OK) {
        printf("Ambient : %12f deg C\n", temp_data.temp_ambient);
        printf("Motor X : %12f deg C\n", temp_data.temp_motor_x);
        printf("Motor Y : %12f deg C\n", temp_data.temp_motor_y);
        printf("mPMT    : %12f deg C\n", temp_data.temp_mpmt);
        printf("Optical : %12f deg C\n", temp_data.temp_optical);
    }
    else {
        printf("ERROR: %d\n", res);
    }
    return true;
}

bool get_axis_state(istringstream& iss)
{
    StateMsgData status_data;
    SerialResult res = comm.get_axis_state(&status_data, MSG_RECEIVE_TIMEOUT_MS);
    if (res == SERIAL_OK) {
        printf("X axis moving : %i \n", status_data.x_motion);
        printf("Y axis moving : %i\n", status_data.y_motion);
        printf("X limit switch far : %i\n", status_data.x_ls_far);
        printf("Y limit switch far : %i\n", status_data.y_ls_far);
        printf("X limit switch home : %i\n", status_data.x_ls_home);
        printf("Y limit switch home : %i\n", status_data.y_ls_home);

    }
    else {
        printf("ERROR: %d\n", res);
    }
    return true;
}

bool link_check(istringstream& iss)
{
    SerialResult res = comm.link_check(MSG_RECEIVE_TIMEOUT_MS);
    if (res == SERIAL_OK) {
        printf("OK\n");
        return true;
    }

    printf("ERROR: %d\n", res);
    return true;
}

bool connect_to_arduino()
{
    if (!device.ser_connect(SERIAL_BAUD_RATE)) return false;
    device.ser_flush();

    cout << "Waiting for Arduino..." << flush;
    while ((comm.check_for_message() != SERIAL_OK) || (comm.received_message().id != MSG_ID_PING));

    // There might be more ping messages sitting in the buffer, so flush them all out
    device.ser_flush();

    cout << "Connected!" << endl;
    return true;
}

bool reset(istringstream& iss)
{
    device.ser_disconnect();
    return connect_to_arduino();
}

bool exit(istringstream& iss)
{
    return false;
}

Command commands[] = {
    // Note: these must be in the same order as declared in the enum

    //                        Name, Description, Usage, Handler
    [CMD_ID_PING]         = { "ping", "Send a ping to the Arduino to check if it's still alive", "ping", ping },
    [CMD_ID_GET_STATUS]   = { "get_status", "Retrieve current status of Arduino", "get_status", get_status },
    [CMD_ID_HOME]         = { "home", "Execute the homing routing", "home", home },
    [CMD_ID_MOVE]         = { "move", "Move the gantry to a new position", "move <x|y> <pos|neg> <hold_vel> <dist>", move },
    [CMD_ID_STOP]         = { "stop", "Freeze all motor functions", "stop", stop },
    [CMD_ID_GET_POSITION] = { "get_position", "Retrieve the current position of the gantry", "get_position", get_position },
    [CMD_ID_GET_TEMP]     = { "get_temp", "Retrieve temperature readings", "get_temp", get_temp },
    [CMD_ID_GET_AXIS_STATE]     = { "get_axis_state", "Retrieve axis state (moving + limits)", "get_axis_state", get_axis_state },
    [CMD_ID_LINK_CHECK]   = { "link_check", "Verify the serial communication link is working", "link_check", link_check },
    [CMD_ID_RESET]        = { "reset", "Reset the Arduino", "reset", reset },
    [CMD_ID_HELP]         = { "help", "Display the help message", "help or help <command>", help },
    [CMD_ID_EXIT]         = { "exit", "Exit the program", "exit", exit }
};

/*****************************************************************************/
/*                                   HELP                                    */
/*****************************************************************************/

void print_cmd_usage(CommandID id)
{
    printf("usage: %s\n", commands[id].usage);
}

void help_general()
{
    printf("\nAvailable Commands:\n");
    for (size_t i = 0; i < ITEM_COUNT(commands); i++) {
        printf("%15s - %s\n", commands[i].name, commands[i].description);
    }

    printf("\nType `help <command>` to see help information about a specific command\n\n");
}

void help_specific(string cmd_name)
{
    CommandID cmd_id = CMD_ID_INVALID;
    Command *cmd = nullptr;

    for (size_t i = 0; i < ITEM_COUNT(commands); i++) {
        if (commands[i].name == cmd_name) {
            cmd_id = (CommandID)i;
            cmd = &commands[i];
            break;
        }
    }

    if (cmd == nullptr) {
        printf("%s is not a valid command", cmd->name);
    }
    else {
        printf("%s - %s\n\n", cmd->name, cmd->description);
        print_cmd_usage(cmd_id);
    }
}

bool help(istringstream& iss)
{
    string word;

    if (iss.good()) {
        iss >> word;
        help_specific(word);
    }
    else {
        help_general();
    }

    return true;
}

/*****************************************************************************/
/*                                   HELP                                    */
/*****************************************************************************/

cmd_handler get_cmd_handler(const string& cmd_name)
{
    for (size_t i = 0; i < ITEM_COUNT(commands); i++) {
        if (commands[i].name == cmd_name) {
            return commands[i].handler;
        }
    }
    return nullptr;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("\nusage: %s <serial device file>\n\nexample:\n    %s /dev/ttyS3\n\n", argv[0], argv[0]);
        return 0;
    }

    device.set_device_file(argv[1]);

    if (!connect_to_arduino()) return 1;

    printf("Type 'help' to see the list of available commands.\n");
    printf("Type 'exit' to exit.\n");

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

        // Handle command
        cmd_handler handler = get_cmd_handler(word);
        if (handler != nullptr) {
            // Handler will return true if we should exit the program
            if (!handler(iss)) exit = true;
        }
        else {
            cout << "Invalid command" << endl;
        }
    }

    device.ser_disconnect();

    return 0;
}
