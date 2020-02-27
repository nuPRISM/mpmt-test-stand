#include <Arduino.h>

#include "ArduinoSerialDevice.h"
#include "SerialTransport.h"
#include "Messages.h"

#define BAUD_RATE 115200

ArduinoSerialDevice serial_device(&Serial);
SerialTransport serial_transport(&serial_device);
uint8_t msg_data[MSG_DATA_LENGTH_MAX];
Message msg;

void setup()
{
    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);

    msg.data = msg_data;

    serial_transport.start(BAUD_RATE);
    Serial.println("mPMT Test Stand");
}

void loop()
{
    if (serial_transport.check_for_message(&msg)) {
        Serial.println("Message Received!");
        Serial.print("ID: ");
        Serial.println(msg.id);
        Serial.print("Length: ");
        Serial.println(msg.length);

        uint8_t data = 0x4E;
        Message msg_to_send = {
            .id = 0x40,
            .length = 1,
            .data = &data
        };
        if (serial_transport.send_message(&msg_to_send)) {
            Serial.println("\nSent response");
        } else {
            Serial.println("\nFailed to send response");
        }
    }

    // put your main code here, to run repeatedly:
    // delay(1000);
    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(1000);
    // digitalWrite(LED_BUILTIN, LOW);
}