//this script is used for bench test motor. change the delay in step function to adjust speed. enter single digital number commands in serial monitor.


#define dirPin 2
#define stepPin 3
const int step_per_rev = 200;
const int gear_ratio = 26.85;
const int output_step_per_rev = step_per_rev * gear_ratio;
const int distance_per_rev = 8; //mm

void setup() {
  float dist = 0;
float rev = 0;
float a = 0; //for serial read in ASCII

  // Declare pins as output
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  // Set the spinning direction 
  digitalWrite(dirPin, HIGH);

  Serial.begin(9600);
}

void loop(){
  
}

void loop1(100) {
  while (Serial.available() != 0) {
    a = Serial.read() - 48; // ASCII conversion
    if (a == -3){ //when negative sign is detected
      digitalWrite(dirPin,LOW);
    }
    if (a >= 0){ 
      dist = dist*10 + a;   
    }
  }
  Serial.println(dist);
  step(dist); //move stepper 
  //reset
  dist = 0;
  digitalWrite(dirPin,HIGH);
  delay(1000);
}

// state 
// real time control 


void step(int dist){
  rev = dist / distance_per_rev;
  //Makes 200 Pulses for making one full cycle rotation, no micro stepping considered for now
  for(int i = 0; i < rev * output_step_per_rev; i++){
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(500);  //change this and the next delay to change pulse speed.
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(500); 
  }
  
  //One second delay
  delay(1000);
}
