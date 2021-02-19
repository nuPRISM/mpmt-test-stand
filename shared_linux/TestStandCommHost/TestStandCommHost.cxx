#include "TestStandCommHost.h"

#include <stdio.h>
#include <string.h>

TestStandCommHost::TestStandCommHost(SerialDevice& device) : TestStandComm(device)
{
    // Nothing else to do
}

SerialResult TestStandCommHost::get_status(Status *status_out, uint32_t timeout_ms)
{
    SerialResult res = this->send_basic_msg(MSG_ID_GET_STATUS);
    if (res != SERIAL_OK) return res;

    res = this->recv_message(MSG_ID_STATUS, 1, timeout_ms);
    if (res != SERIAL_OK) return res;

    *status_out = (Status)((this->received_message().data)[0]);
    return SERIAL_OK;
}

SerialResult TestStandCommHost::home()
{
    return this->send_basic_msg(MSG_ID_HOME);
}

SerialResult TestStandCommHost::move(AxisId axis, AxisDirection dir, uint32_t vel_hold, uint32_t dist_counts, AxisResult *res_out, uint32_t timeout_ms)
{
    // Send move message
    MoveMsgData data = {
        .vel_hold = (uint32_t)htonl(vel_hold),
        .dist_counts = (uint32_t)htonl(dist_counts),
        .axis = (uint8_t)axis,
        .dir = (uint8_t)dir
    };

    Message msg = {
        .id = MSG_ID_MOVE,
        .length = sizeof(data),
        .data = (uint8_t *)&data
    };
    
    SerialResult res = this->session.send_message(msg);
    if (res != SERIAL_OK) return res;

    // Get result
    res = this->recv_message(MSG_ID_AXIS_RESULT, 1, timeout_ms);
    if (res != SERIAL_OK) return res;

    *res_out = (AxisResult)((this->received_message().data)[0]);
    return SERIAL_OK;
}

SerialResult TestStandCommHost::stop()
{
    return this->send_basic_msg(MSG_ID_STOP);
}

SerialResult TestStandCommHost::get_position(PositionMsgData *position_out, uint32_t timeout_ms)
{
    SerialResult res = this->send_basic_msg(MSG_ID_GET_POSITION);
    if (res != SERIAL_OK) return res;

    res = this->recv_message(MSG_ID_POSITION, sizeof(PositionMsgData), timeout_ms);
    if (res != SERIAL_OK) return res;

    // Copy message data into output struct
    memcpy(position_out, this->received_message().data, sizeof(PositionMsgData));
    // Fixup byte order
    position_out->x_counts = ntohl(position_out->x_counts);
    position_out->y_counts = ntohl(position_out->y_counts);

    return SERIAL_OK;
}

SerialResult TestStandCommHost::get_temp(TempData *temp_out, uint32_t timeout_ms)
{
    SerialResult res = this->send_basic_msg(MSG_ID_GET_TEMP);
    if (res != SERIAL_OK) return res;

    res = this->recv_message(MSG_ID_TEMP, sizeof(TempMsgData), timeout_ms);
    if (res != SERIAL_OK) return res;

    // Copy message data into output struct
    TempMsgData msg_data;
    memcpy(&msg_data, this->received_message().data, sizeof(TempMsgData));
    // Fixup byte order and re-scale
    temp_out->temp_ambient = (double)ntohl(msg_data.temp_ambient) / temp_data_scaler;
    temp_out->temp_motor_x = (double)ntohl(msg_data.temp_motor_x) / temp_data_scaler;
    temp_out->temp_motor_y = (double)ntohl(msg_data.temp_motor_y) / temp_data_scaler;
    temp_out->temp_mpmt    = (double)ntohl(msg_data.temp_mpmt) / temp_data_scaler;
    temp_out->temp_optical = (double)ntohl(msg_data.temp_optical) / temp_data_scaler;

    return SERIAL_OK;
}

SerialResult TestStandCommHost::get_axis_state(StateMsgData *status_out, uint32_t timeout_ms)
{
  printf("Got into get_axis_state\n");

    SerialResult res = this->send_basic_msg(MSG_ID_GET_AXIS_STATE);
    if (res != SERIAL_OK) return res;

    res = this->recv_message(MSG_ID_AXIS_STATE, sizeof(StateMsgData), timeout_ms);
    if (res != SERIAL_OK) return res;

    // Copy message data into output struct
    //    StateMsgData msg_data;
    memcpy(status_out, this->received_message().data, sizeof(StateMsgData));

    //    printf("%i %i %i %i %i %i \n",msg_data.x_motion,msg_data.y_motion,msg_data.x_ls_far, msg_data.y_ls_far
    //	   ,msg_data.x_ls_home, msg_data.y_ls_home);
    // Fixup byte order and re-scale
    //    temp_out.x_motion = 

    //    temp_out->temp_ambient = (double)ntohl(msg_data.temp_ambient) / temp_data_scaler;
    //temp_out->temp_motor_x = (double)ntohl(msg_data.temp_motor_x) / temp_data_scaler;
    //temp_out->temp_motor_y = (double)ntohl(msg_data.temp_motor_y) / temp_data_scaler;
    //temp_out->temp_mpmt    = (double)ntohl(msg_data.temp_mpmt) / temp_data_scaler;
    //temp_out->temp_optical = (double)ntohl(msg_data.temp_optical) / temp_data_scaler;

    return SERIAL_OK;
}

SerialResult TestStandCommHost::calibrate(CalibrationKey key, void *value)
{
    this->send_buf[0] = (uint8_t)key;

    uint8_t value_size;
    switch (key) {
        case CAL_GANTRY_ACCEL:
        case CAL_GANTRY_VEL_START:
        case CAL_GANTRY_VEL_HOME:
        {
            uint32_t value_conv = htonl(*(uint32_t *)value);
            value_size = sizeof(value_conv);
            memcpy(&this->send_buf[1], &value_conv, value_size);
            break;
        }
        case CAL_TEMP_ALL_C1:
        case CAL_TEMP_ALL_C2:
        case CAL_TEMP_ALL_C3:
        case CAL_TEMP_ALL_RESISTOR:
        {
            double value_conv = htond(*(double *)value);
            value_size = sizeof(value_conv);
            memcpy(&this->send_buf[1], &value_conv, value_size);
            break;
        }
        default:
            value_size = 0;
    }

    Message msg = {
        .id = MSG_ID_CALIBRATE,
        .length = (uint8_t)(value_size + 1),
        .data = this->send_buf
    };

    return this->session.send_message(msg);
}
