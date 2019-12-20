int x = 800;

void setup() {
  // put your setup code here, to run once:
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
}

void loop() {
  for(int j=0; j<3000; j++){

    for(int i=0; i<5; i++){
      digitalWrite(6,HIGH);
      delayMicroseconds(x);
      digitalWrite(6,LOW);
      delayMicroseconds(x);
    }
    if(x>85){
      x--;
    }
  }
}
