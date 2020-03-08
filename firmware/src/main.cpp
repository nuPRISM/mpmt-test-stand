#include <Arduino.h>

#include "ArduinoSerialDevice.h"
#include "SerialTransport.h"
#include "SerialSession.h"
#include "Messages.h"

#define BAUD_RATE 115200

ArduinoSerialDevice serial_device(&Serial);
SerialTransport serial_transport(&serial_device);
SerialSession serial_session(&serial_transport);

void setup()
{
    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);

    serial_device.ser_connect(BAUD_RATE);
    Serial.println("mPMT Test Stand");
}

void loop()
{
    if (serial_session.check_for_message()) {
        Serial.println("\nMessage Received!");
        Serial.print("ID: ");
        Serial.println(serial_session.received_msg.id);
        Serial.print("Length: ");
        Serial.println(serial_session.received_msg.length);

        // uint8_t data = 0x4E;
        // Message msg_to_send = {
        //     .id = 0x40,
        //     .length = 1,
        //     .data = &data
        // };
        // if (serial_session.send_message(&msg_to_send)) {
        //     Serial.println("\nSent response");
        // } else {
        //     Serial.println("\nFailed to send response");
        // }
    }

    // put your main code here, to run repeatedly:
    // delay(1000);
    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(1000);
    // digitalWrite(LED_BUILTIN, LOW);
}