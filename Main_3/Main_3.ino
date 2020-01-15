/* This script is used in the arduino right now with TB6600 motor drivers
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

 
//#define switchPin 4

bool read_status;
float a = 0; 
float x_a = 0;  
int switch_stats;
bool over_ride = false;
bool negative = false;
int motor_selection = 0;


class stepper //our own class for stepper, arduino library is meant to directly connect a motor with no drivers.
{
  public:
  //declare class functions 
    stepper(int _DirPin, int _PulsePin);
    void Step();
    void SetSpeed(int _interval);
  //  void GoTo(int position_target); //consider add this function in future? Probably not neccessary

  //declare variables and constants 
    float position_curr;
    float position_required;
    int step_per_rev = 200;
    float gear_ratio = 26.85;
    float output_step_per_rev = step_per_rev * gear_ratio;
    int distance_per_rev = 8; //mm
    double distance_per_step = distance_per_rev / output_step_per_rev;
    float tol = 0.01; //tolerance on travel distance, mm
    int dirPin, pulsePin;

  private:
    long time_prev;
    long interval; //This is used to control the speed of motor, DO NOT set this too high (>= loop speed of loop_motor) 
                   //it will cause the program to stuck here forever as it does not have time to excute other loops.
};

//constructor
stepper::stepper(int _DirPin, int _PulsePin) 
{
  pinMode(_DirPin, OUTPUT);
  pinMode(_PulsePin, OUTPUT);
  dirPin = _DirPin;
  pulsePin = _PulsePin;
  interval = 400; // initialize default speed
  position_curr = 0;  // Assume start at 0 coordinate, change once calibration is added.
}

void stepper::Step(){
  time_prev = micros(); //use time stamp to measure interval between pulses so the whole system will not be delayed by delay() command
  while (micros() - time_prev < interval){
    digitalWrite(pulsePin,HIGH); 
  } 
  digitalWrite(pulsePin,LOW); 
}

void stepper::SetSpeed(int _interval){
  interval = _interval;
}

//class definitaion done

//declare objects
stepper motor1(2,3);
stepper motor2(5,6);

long serialtime;

void setup() {
  // Declare pins
  //  pinMode(switchPin, INPUT);
  
  motor1.position_required = 0;
  motor2.position_required = 0;
  
  Serial.begin(9600);
}

void loop(){
    while (Serial.available() != 0) {
      a = Serial.read() - 48; // ASCII conversion to numbers
      if (a == -3){ //when negative sign is detected, ideally no negative sign should be allowed. AKA once calibration is done it should start at (0,0) and goes up
        negative = true;
      }else if (a == 49){//97 - 48 == lower case 'a'
        motor_selection = 1;
      }else if (a == 50) { //lower case 'b'
        motor_selection = 2;
      }else if (a >= 0){ 
        if (negative == true){
          x_a = -abs(x_a*10) - a;
      }else{
        x_a = x_a*10 + a;   
      }
      read_status = true;
      serialtime = micros();
    }
  }
  
  if ((read_status == true)&&(micros() - serialtime > 5000)){ //wait some time before actually sending in command in case 
                                                              //arduino reading numbers too slow and cause two commands read in in stead of one
    if (motor_selection == 1){
      motor1.position_required = x_a;
    }else if (motor_selection == 2){
      motor2.position_required = x_a;
    }
    read_status = false;
    negative = false;
    x_a = 0;
  }
}

void loop_motor(1){ //main loop
  if (over_ride == false){ //if limit switch is pressed, do not move.
    if (abs(motor1.position_curr - motor1.position_required) > motor1.tol){ //check first motor's position and target, then decide direction and move
      if (motor1.position_curr > motor1.position_required){
        digitalWrite(motor1.dirPin, LOW);
        motor1.position_curr -= motor1.distance_per_step;
      }else{
        digitalWrite(motor1.dirPin,HIGH);
        motor1.position_curr += motor1.distance_per_step;
      }
      motor1.Step();
    }else if (abs(motor2.position_curr - motor2.position_required) > motor2.tol){
      if (motor2.position_curr > motor2.position_required){
        digitalWrite(motor2.dirPin, LOW);
        motor2.position_curr -= motor2.distance_per_step;
      }else{
        digitalWrite(motor2.dirPin,HIGH);
        motor2.position_curr += motor2.distance_per_step;
      }
      motor2.Step();
    }
  }
}

void loop_print(100){ //print position, in a slower loop since we do not need it to update that fast, it will also slow down the program.
  Serial.print("Motor1: ");
  Serial.print(motor1.position_curr,5);
  Serial.print("  Motor2: ");
  Serial.println(motor2.position_curr,5);
}

//below is for limit switch
/*
void loop2(10){
  switch_stats = digitalRead(switchPin);
   // Serial.println(switch_stats);
    

  if ( switch_stats == HIGH){
    over_ride = false;
  } else {
    over_ride = HIGH;
  }
}*/
