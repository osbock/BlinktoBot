/*******************************************
  Robot control example for JoeBot.
  Manage motor control, recieve commands from IR remote.
  Kevin Osborn http://baldwisdom.com

  The Robot (which I call JoeBot) was created as a kit
  by Joe MacDermot for the Boston Robotics Meetup.
  Acrylic Base, and 4 gear motors tied in parallel to a 
  Chinese motor controller using L9110s  BLUE motor control board
  Arduino Nano v3
  IR reciever
  3 Generic Ping sensors
  
  use IR remote library &  TSOP1738 IR receiver
  http://learn.adafruit.com/ir-sensor
  Ken Shirriff's blog:  A Multi-Protocol Infrared Remote Library for the Arduino
  http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html
   The Shirriff library uses timers and interrupts, so you have to be
   careful about pin assignments but there is less latency
  ********************************************/
  //  IR commands are sent by Adadfruit NEC remote
/*
FD00FF  Vol-
FD807F  >||  (play/pause)
FD40BF  Vol+                 FD30CF    0
FD20DF  Setup                FD08F7    1
FDA05F    ^ (up)             FD8877    2
FDB04F    v (down)           FD48B7    3
FD10EF    < (left)           FD28D7    4
FD50AF    > (right)          FDA857    5
FD708F    <==|   (loop)      FD6897    6
FD609F  Stop/Mode            FD18E7    7
FD906F  Enter/Save           FD9867    8
                             FD58A7    9
*/
//#include <IRremote.h>  //you can try other remote controls with the complete LIB
#include <IRremoteNec.h>  //stripped of unnecessary protocols
#define RECV_PIN  A5  //decode pin for TSOP1738  IR detector
IRrecv irrecv(RECV_PIN);
decode_results irData;
//Note  IR decoder library uses Timer1
//      Motor PWM on pins 5&6 use Timer0
//      PWM  on 9&10 (Timer1) not longer available

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
  irrecv.enableIRIn(); // Start the receiver

}
void loop() {
  unsigned long IRcmd;
  if (irrecv.decode(&irData)) {
    IRcmd = decodeIRcommand(&irData);
    irrecv.resume(); // Receive the next value
 }
  if (IRcmd != 0)
    processIRcommand(IRcmd);
  runMotors();
}
//use remote library to detect remote control signals
unsigned long decodeIRcommand(decode_results *irdata) {
  if (irdata->decode_type != NEC)
    return 0L;
  //output raw codes.  Other remotes will work here 
  //BUT you will need to update codes in 'processIRcommand()'
  //
  Serial.print("IR command: ");
  Serial.println(irdata->value, HEX);
  return irdata->value; 
}
//raw IR codes are converted to actual robot commands
//My values come from Adafruit remote.  Other remotes will generate different codes.
void processIRcommand(unsigned long IRcmd) {
  //cmd = 0xFD48B7;  //for debugging
  int mspd = 0;
  switch (IRcmd) {
  case 0xFDA05F: // key=up
    Serial.println("Motors Forward!");
    targetDir[0] = FORWARD;
    targetSpeed[0] = 80;
    targetDir[1] = FORWARD;
    targetSpeed[1] = 80;
    break; 
  case 0xFDB04F: // key=dwn  adafruit - DOWN ARROW
    Serial.println("Motors Reverse!");
    targetDir[0] = REVERSE;
    targetSpeed[0] = 80;
    targetDir[1] = REVERSE;
    targetSpeed[1] = 80;
    break;   
  case 0xFD609F: //stop  adafruit - STOP MODE
    Serial.println("Motors Stop!");
    targetSpeed[0] = STOP;
    targetSpeed[1] = STOP;
    break; 
  case 0xFD50AF: //right  adafruit - RIGHT ARROW
    Serial.println("Right!");
    targetDir[0] =FORWARD;
    targetSpeed[0] = 80;
    targetDir[1] = REVERSE;
    targetSpeed[1]=80;
    break; 
  case 0xFD10EF: //left  //adafruit - LEFT ARROW
    Serial.println("Left!");
    targetDir[1] =FORWARD;
    targetSpeed[1] = 80;
    targetDir[0] = REVERSE;
    targetSpeed[0]=80;
    break; 
  }
  return; 
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
        

