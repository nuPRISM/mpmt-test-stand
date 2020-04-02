#include "PseudoAxis.h"

void set_up_encoder(PseudoEncoder *encoder, void (*isr_motor_pulse)(void))
{
    pinMode(encoder->motor_pulse_pin, INPUT_PULLUP);
    pinMode(encoder->channel_a_out, OUTPUT);
    digitalWrite(encoder->channel_a_out, LOW);
    attachInterrupt(digitalPinToInterrupt(encoder->motor_pulse_pin), isr_motor_pulse, CHANGE);
}

void steps_to_counts(PseudoAxis *pseudo_axis)
{
    pseudo_axis->skip_counter++;
    // skip steps in the beginning
    if (pseudo_axis->skip_counter <= pseudo_axis->changes_to_skip)
        return;
    // else start toggling until the next set of STEPS
    else if ((pseudo_axis->skip_counter <= (2 * pseudo_axis->steps_for_ratio)) || (pseudo_axis->changes_to_skip == 0))
    {
        bool motor_pin_status = digitalRead((pseudo_axis->encoder).motor_pulse_pin);
        digitalWrite((pseudo_axis->encoder).channel_a_out, motor_pin_status ^ HIGH);
    }
    else
    {
        // this is assigned 1 and not 0 because at this point our step_count_x is ((2 * STEPS) + 1)
        // which means we are onto the next set of 8, and 17 would be the beginnig of that set
        pseudo_axis->skip_counter = 1;
        return;
    }
}

void isr_motor_pulse(PseudoAxis *pseudo_axis)
{
    steps_to_counts(pseudo_axis);

    bool direction = digitalRead(pseudo_axis->motor_dir_pin);

    if (direction)
    {
        if (pseudo_axis->motor_position_current < pseudo_axis->axis_length_counts)
        {
            pseudo_axis->motor_position_current++;
            // check if limit switch was pressed before this move
            // checks whether the homing routine is being run
            if (!pseudo_axis->ls_home.status)
            { // if !PRESSED where PRESSED = 0
                // sets limit switch to unpressed
                pseudo_axis->ls_home.status = UNRESSED;
                // using digital write so it can work on Due ot Uno, execution time ~ 3 microseconds
                digitalWrite(pseudo_axis->ls_home.output_pin, UNRESSED);

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

            if (!pseudo_axis->ls_far.status)
            { // if !PRESSED where PRESSED = 0
                // sets limit switch to unpressed
                pseudo_axis->ls_far.status = UNRESSED;

                digitalWrite(pseudo_axis->ls_far.output_pin, UNRESSED);

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
    pseudo_axis->skip_counter=0;
    pseudo_axis->motor_position_current = pseudo_axis->motor_position_default;
}