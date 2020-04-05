#include "PseudoAxis.h"

void set_up_encoder(PseudoEncoder *encoder, void (*isr_motor_pulse)(void))
{
    pinMode(encoder->motor_pulse_pin, INPUT_PULLUP);
    pinMode(encoder->channel_a_pin, OUTPUT);
    digitalWrite(encoder->channel_a_pin, LOW);
    attachInterrupt(digitalPinToInterrupt(encoder->motor_pulse_pin), isr_motor_pulse, CHANGE);
}

bool toggle_encoder_output(PseudoAxis *pseudo_axis)
{   
    if (pseudo_axis->changes_to_skip != 0) pseudo_axis->change_counter++;
    // skip steps in the beginning
    if (pseudo_axis->change_counter <= pseudo_axis->changes_to_skip) return false;
    // else start toggling until the next set of STEPS
    else if ((pseudo_axis->change_counter <= (2 * pseudo_axis->steps_for_ratio)) || (pseudo_axis->changes_to_skip == 0))
    {   
        int motor_pin_status = digitalRead(pseudo_axis->encoder.motor_pulse_pin);

        digitalWrite((pseudo_axis->encoder).channel_a_pin, motor_pin_status);
        // check if encoder count needs to be incremented
        // if HIGH it means triggered on rising
        return (motor_pin_status == HIGH);
    }
    else
    {
        // this is assigned 1 and not 0 because at this point our step_count_x is ((2 * STEPS) + 1)
        // which means we are onto the next set of 8, and 17 would be the beginnig of that set
        pseudo_axis->change_counter = 1;
        return false;
    }
}

void isr_motor_pulse(PseudoAxis *pseudo_axis)
{   
    if (!toggle_encoder_output(pseudo_axis)) return;

    bool direction = digitalRead(pseudo_axis->motor_dir_pin);

    if (direction)
    {
        if (pseudo_axis->motor_position_current < pseudo_axis->axis_length_counts)
        {
            pseudo_axis->motor_position_current++;
            // check if limit switch was pressed before this move
            // checks whether the homing routine is being run
            if (pseudo_axis->ls_home.status == PRESSED)
            {
                // sets limit switch to unpressed
                pseudo_axis->ls_home.status = UNPRESSED;
                // using digital write so it can work on Due or Uno, execution time ~ 3 microseconds
                digitalWrite(pseudo_axis->ls_home.output_pin, UNPRESSED);

                // resets currrent position to 0
                pseudo_axis->motor_position_current = 0;
            }
        }
        else
        {
            // cant exceed max length - press limit switch
            pseudo_axis->ls_far.status = PRESSED;

            digitalWrite(pseudo_axis->ls_far.output_pin, PRESSED);
        }
    }
    else if (!direction)
    {
        if (pseudo_axis->motor_position_current > 0)
        {
            pseudo_axis->motor_position_current--;

            if (pseudo_axis->ls_far.status == PRESSED)
            {
                // sets limit switch to unpressed
                pseudo_axis->ls_far.status = UNPRESSED;

                digitalWrite(pseudo_axis->ls_far.output_pin, UNPRESSED);

                // resets currrent position to max length of gentry
                pseudo_axis->motor_position_current = pseudo_axis->axis_length_counts;
            }
        }
        else
        {
            pseudo_axis->ls_home.status = PRESSED;
            digitalWrite(pseudo_axis->ls_home.output_pin, PRESSED);
        }
    }
}

void reset_pseudo_axis(PseudoAxis *pseudo_axis)
{
    pseudo_axis->change_counter = 0;
    pseudo_axis->motor_position_current = pseudo_axis->motor_position_default;
    pseudo_axis->ls_far.status = UNPRESSED;
    pseudo_axis->ls_home.status = UNPRESSED;
}

void dump_data(PseudoAxis *pseudo_axis)
{
    if (Serial) {
        char *home_status;
        char *far_status;
        Serial.print("Axis:              "); Serial.println(pseudo_axis->axis_name);
        Serial.print("Axis length:       "); Serial.print(pseudo_axis->axis_length_counts); Serial.println(" counts");
        Serial.print("Motor position:    "); Serial.print(pseudo_axis->motor_position_current); Serial.println(" counts");
        if (pseudo_axis->ls_home.status == PRESSED) home_status = "PRESSED";
        else home_status = "UNPRESSED";
        Serial.print("LS home status:    "); Serial.println(home_status);
        if (pseudo_axis->ls_far.status == PRESSED) far_status = "PRESSED";
        else far_status = "UNPRESSED";
        Serial.print("LS far status:     "); Serial.println(far_status);
        Serial.println("--------------------------------------------");
    }
    else {
        return;
    }
}