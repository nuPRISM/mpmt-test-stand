//Need Arte arduino library to run multi task in different loops excuting the same time.
//More instructions/explanation will be added in next update with class and x-y differenticiate control

#define dirPin 2
#define stepPin 3
#define switchPin 4
const int step_per_rev = 200;
const float gear_ratio = 26.85;
const float output_step_per_rev = step_per_rev * gear_ratio;
const int distance_per_rev = 8; //mm
const double distance_per_step = distance_per_rev / output_step_per_rev;
const float tol = 0.01; //tolerance on travel distance, mm
double x_required;
bool read_stats;
float a = 0; 
float x_a = 0;  
bool over_ride = false;
//for serial read in ASCII
int switch_stats;
double x_curr;
void step();

void setup() {
  float dist = 0;
  float rev = 0;


  // Declare pins
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(switchPin, INPUT);
  // Assume start at 0 coordinate, change once calibration is added.
  x_curr = 0;
  x_required = 0;
  
  Serial.begin(9600);
}

void loop(){
  
    while (Serial.available() != 0) {
    a = Serial.read() - 48; // ASCII conversion
    if (a == -3){ //when negative sign is detected, ideally no negative sign should be allowed.
      digitalWrite(dirPin,LOW);
    }
    if (a >= 0){ // in case of extra 10 in ASCII for single digit (resulting in negative a)
      x_a = x_a*10 + a;   
      read_stats = true;
    }
  }
  if (read_stats == true){
    x_required = x_a;
    read_stats = false;
  }
  x_a = 0; 

}

void loop_motor(1){
  if (over_ride == false){  
    if (abs(x_curr - x_required) > tol){
      if (x_curr > x_required){
        digitalWrite(dirPin, HIGH);
        step();
        x_curr -= distance_per_step;
      }else if (x_curr < x_required){
        digitalWrite(dirPin,LOW);
        step();
        x_curr += distance_per_step;
      }
    }
  }
}

void loop2(10){
  switch_stats = digitalRead(switchPin);
   // Serial.println(switch_stats);
    Serial.println(x_curr,5);

  if ( switch_stats == HIGH){
    over_ride = false;
  } else {
    over_ride = HIGH;
  }
}

void step(){ // does a single step of interval lengthen time. change interval to change speed.
  //long time_curr;
  long time_prev;
  long interval = 400; //This is used to control the speed of motor, DO NOT set this too high (>= loop speed of loop_motor) 
                       //it will cause the program to stuck here forever as it does not have time to excute other loops.
  time_prev = micros();
  while (micros() - time_prev < interval){
    digitalWrite(stepPin,HIGH); 
  } 
  digitalWrite(stepPin,LOW); 
}
