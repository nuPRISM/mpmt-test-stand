/* **************************** Local Includes ***************************** */
#include "mPMTTestStand.h"
#include "conf.h"
#include "Debug.h"

/* ************************ Shared Project Includes ************************ */
#include "shared_defs.h"
#include "DefaultCalibration.h"

/* **************************** System Includes **************************** */
#include <Arduino.h>

mPMTTestStand test_stand(conf, default_calibration);

uint32_t blink_start;
bool blink_state;

void setup()
{
    DEBUG_INIT;
    pinMode(LED_BUILTIN, OUTPUT);

    test_stand.setup();

    blink_start = millis();
    blink_state = false;
}

void loop()
{
    test_stand.execute();

    // Blink an LED
    if ((millis() - blink_start) >= 500) {
        blink_state = !blink_state;
        digitalWrite(LED_BUILTIN, blink_state);
        blink_start = millis();
    }
}
