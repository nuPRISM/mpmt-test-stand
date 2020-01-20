/* This header file defines a StepperMotor class
 * Pin2 connects to dir+ of the two drivers that control two motors simotineously.
 * Pin3 connects to pul+ of the two drivers that control two motors simotineously.
 * ENA+, ENA- are not connected.
 * dir- and pul- are connected to common ground of arduino.
 * VCC and GND for driver is comnnected to power supply seperate from arduion.
 * A+, A- is one pair of wires from motor; B+, B- is the other pair.
 * 
 * Pin5 and 6 are connected to the other motor driver with same schematic.
 * 
 * Limit switch is meant to connect to pin4, directly from ground with no resistor is fine, current is divided 
 * so no load is nessaccary. This part of code is commented out but can be added just by uncommenting.
 */

// =======================================
// include guard
#ifdef __STEPPERMOTOR_H_INCLUDED__
#define __STEPPERMOTOR_H_INCLUDED__

// =======================================
// forward included dependencies


// =======================================
// included dependencies


// =======================================
// class definition

class StepperMotor
{
private:
    int home;                   // counts at the expexted 0,0 limit switch position
    int dir_plus;
    int pul_plus;
    int speed;                  // speed is defined in counts per second
    int location_current;       // current location is specified in counts
    int location_final;         // final location is specified in counts

    int get_time_from_speed(int s);

public:
    // constructor
    StepperMotor(int dp, int pp, int s);

    // constructor with speed as variable
    StepperMotor(int dp, int pp);

    void set_speed(int s);
    void set_direction(int dir);
    void 

    ~StepperMotor();
};

#endif // __STEPPERMOTOR_H_INCLUDED__