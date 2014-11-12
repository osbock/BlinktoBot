/*******************************************
  Robot control example for JoeBot.
  Manage motor control, recieve commands from SerialPort.
  Kevin Osborn http://baldwisdom.com

  The Robot (which I call JoeBot) was created as a kit
  by Joe MacDermot for the Boston Robotics Meetup.
  Acrylic Base, and 4 gear motors tied in parallel to a 
  Chinese motor controller using L9110s  BLUE motor control board
  Arduino Nano v3
  IR reciever
  3 Generic Ping sensors
  
  Set the robot on a stand and send commands via Serial port.
********************************************/
#define BLINK_PIN 13  //yellow LED on Arduino  Nano 3.0
//      The L9110S has forward and backward inputs that you PWM
//      So to go forward, set backward to LOW and PWM forward
//      to go backward, set forward to LOW and PWM backward
//      note: left and right sides should be swapped (Ia,Ib)
//      because the motors are facing different directions.

int forwardPin[2] = {6, 9}; // Forward control
int backwardPin[2] = {5, 10}; //  backward control
int targetSpeed[2] = {0,0}; // internal for setting target speed of motors
int targetDir[2];
int currentDir[2];
int currentSpeed[2] = {0,0}; // default to off
// These might have to be set per motor
#define STARTSPEED 140
#define STARTPERIOD 250
//directions
#define FORWARD HIGH
#define REVERSE LOW
#define STOP  0
void setup() {
  Serial.begin (115200);   // debugging
  Serial.println("Startup!");
  // initialize the digital pin as an output.
  pinMode(BLINK_PIN, OUTPUT);
  pinMode(forwardPin[0], OUTPUT); 
  pinMode(backwardPin[0], OUTPUT);
  pinMode(forwardPin[1], OUTPUT); 
  pinMode(backwardPin[1], OUTPUT); 
  motorStop(0);
  motorStop(1);
}

void loop() {
  if (Serial.available()){
    char c = Serial.read();
    switch (c){
      case 'f':
        Serial.println("Forward!");
        targetDir[0] = FORWARD;
        targetSpeed[0] = 80;
        targetDir[1] = FORWARD;
        targetSpeed[1] = 80;
        break;
      case 'b':
        Serial.println("Backward!");
        targetDir[0] = REVERSE;
        targetSpeed[0] = 80;
        targetDir[1] = REVERSE;
        targetSpeed[1] = 80;
        break;
      case 's':
        Serial.println("STOP!");
        targetSpeed[0] = STOP;
        targetSpeed[1] = STOP;
        break;
      case 'r':
        Serial.println("Right!");
        targetDir[0] =FORWARD;
        targetSpeed[0] = 80;
        targetDir[1] = REVERSE;
        targetSpeed[1]=80;
        break;
      case 'l':
        Serial.println("Left!");
        targetDir[1] =FORWARD;
        targetSpeed[1] = 80;
        targetDir[0] = REVERSE;
        targetSpeed[0]=80;
        break;
      
    }
  }
  runMotors();
}

// More sophisticated Motor controllers will allow you 
// to turn the motors off, and also have a braking input.
// the blue board doesn't have a braking input, just turn
// motor Direction and speed pins low
//void motorStop(int motorNo, byte braking) {
void motorStop(int motorNo){
   digitalWrite(forwardPin[motorNo], LOW);
   digitalWrite(backwardPin[motorNo], LOW);
}
// set Motor Speed INTENTION. does not actually perform IO, that's
// done in runMotors
void setMotorSpeed(int motorNo,int direc,int mspeed){
  if (motorNo > 1) return;
  targetDir[motorNo] = direc;
  targetSpeed[motorNo] = mspeed;
}
// set Motor Speed and Direction, performing actual IO
// only called from runMotors();
void writeMotorSpeed(int motorNo, int direc, int mspeed) {
  if (motorNo > 1) return;
  //
  if (direc == FORWARD){
    digitalWrite(backwardPin[motorNo],LOW);
    analogWrite(forwardPin[motorNo],mspeed);
  }else
  {
    digitalWrite(forwardPin[motorNo],LOW);
    analogWrite(backwardPin[motorNo],mspeed);
  }
}
#ifdef DEBUG
void printSpeeds(){
  Serial.println("Left");
  Serial.print("currentSpeed: ");
  Serial.println(currentSpeed[0]);
  Serial.print("targetSpeed: ");
  Serial.println(targetSpeed[0]);
  Serial.print("direction: ");
  Serial.println(targetDir[0]);
  Serial.println("Right");
  Serial.print("currentSpeed: ");
  Serial.println(currentSpeed[1]);
  Serial.print("targetSpeed: ");
  Serial.println(targetSpeed[1]);
  Serial.print("direction: ");
  Serial.println(targetDir[1]);
}
#endif 
#ifdef DEBUG
long lastDebug;
#define DEBUGINTERVAL 1000
#endif
long lastUpdate;
#define MINTERVAL 200
void runMotors(){
  long current = millis();
  if ((current - lastUpdate) > MINTERVAL)
  {
    lastUpdate = current;
    for (int i = 0; i < 2; i++)
    {
      if ((currentSpeed[i] == 0)&& targetSpeed[i] != 0) //if motors are stopped
      {
        currentSpeed[i] = STARTSPEED; // give the motors a push
        currentDir[i] = targetDir[i];
        writeMotorSpeed(i,targetDir[i],STARTSPEED);
        // note might have to make MINTERVAL smaller and keep
        // track of how many cycles we've been at higher current
      }else // already started or changing direction, drop/go to targetSpeed
      if((currentSpeed[i] != targetSpeed[i]) || currentDir[i] != targetDir[i]){
        currentSpeed[i] = targetSpeed[i];
        currentDir[i] = targetDir[i];
        writeMotorSpeed(i,targetDir[i],targetSpeed[i]);
      }
    }
  }
#ifdef DEBUG
  if (current - lastDebug > DEBUGINTERVAL){
    lastDebug = current;
    printSpeeds();
  }
#endif
}
        

