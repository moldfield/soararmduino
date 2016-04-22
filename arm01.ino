#include <Servo.h>
#include <SPI.h>
#include "RF24.h"
/**
 * Start all initial constants and such
 *
 */

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 10 & 53 */
RF24 radio(10,53);

/**********************************************************/

byte addresses[][6] = {"1Node","2Node"};

// Used to control whether this node is sending or receiving
bool role = 1;//sending

Servo shoulderRoll;   // create servo 1 object 
Servo shoulderPitch;  // create servo 2 object (bigDaddyServo)
Servo elbow;          // create servo 3 object
Servo wristPitch;     // create servo 4 object
Servo wristRoll;      // create servo 5 object
Servo grabber;        // create servo 6 object

int linearActuator = 11; // pin 11

//sending character
char charToClose = 'D';
char charToOpen = 'O';

//local status variables    
bool success = 0;
bool done = 0;
char start = 'L';

// initial position
int initShoulderRoll = 180;     // Determined by calculation
int initShoulderPitch = 0;
int initElbow = 40;
int initPitch = 73;
int initRoll = 90;
//int initGrabber = 0;

// Final Position
int finalShoulderRoll = 0;     // Determined by calculation
int finalShoulderPitch = 0;
int finalElbow = 40;
int finalPitch = 73;
int finalRoll = 90;
//int finalGrabber = 0;
/**
 * Start setup
 *
 */
void setup() {
    
  

  //for coms
  Serial.begin(115200);
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses

  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);

  // Start the radio listening for data
  radio.startListening();
  
  
  
  //for servo
  // Pins are set to servo number + 1
  shoulderRoll.attach(2);
  shoulderPitch.attach(3);
  elbow.attach(4);
  wristPitch.attach(5);
  wristRoll.attach(6);
  grabber.attach(7);
  
  // Set up the linear actuator
  pinMode(linearActuator,OUTPUT);

} // end setup

/**
 * Start loop
 *
 */
void loop(){
  //DO not start until started (key press)  
  /*while(start != 'S'){
    if(Serial.available()){
      Serial.println("Serial Is Available");
      start = Serial.read(); 
    }
    Serial.println("Not Started");
    delay(1000);
  }  */

  //Ensure that the accuator is open
  /*while(!done){
    done = sendChar(charToOpen);
  }
  Serial.println("Actuator Should be Opening");
  delay(15000);//allow the actuator enough time to close*/


  //ALL ARM MOVEMENTS
  //fullSweep();
  movePositionOne();
  delay(1000);
  movePositionTwo();
  delay(2000);


  //ensure the actuator is closed  
  /*while(!success){  
      success = sendChar(charToSend);
  }
  
  
  //sendChar(charToClose); //If you just need to close the actuator
  Serial.println("Actuator Should be Closing");*/
} // end main loop

/**
 * Start function declarations
 *
 */
bool sendChar(char charToSend){
  /****************** Ping Out Role ***************************/  
  radio.stopListening();                                    // First, stop listening so we can talk.
  
  Serial.println(F("Now sending"));

   if (!radio.write( &charToSend, sizeof(char) )){
     Serial.println(F("failed"));
   }
      
  radio.startListening();                                    // Now, continue listening
  
  unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
  boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
  
  while ( ! radio.available() ){                             // While nothing is received
    if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
        timeout = true;
        break;
    }      
  }
      
  if ( timeout ){                                             // Describe the results
      Serial.println(F("Failed, response timed out."));
      return 0;
  }else{
      unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
      radio.read( &got_time, sizeof(unsigned long) );
      unsigned long end_time = micros();
      
      // Spew it
      Serial.print(F("Sent "));
      Serial.print(charToSend);
      Serial.print(F(", Got response "));
      Serial.print(got_time);
      Serial.println(F(" microseconds"));
      return 1;
  }

  // Try again 1s later
  delay(1000);    
    
} // end sendChar


void serialEvent() {
  Serial.println(F("Serial Event has been called "));
  char escape = 'K';
  while(escape != 'S'){
    if (Serial.available()) {
      escape = Serial.read();
      Serial.println(F("Character has been read"));
    }
  }
  if(escape == 'S'){
    Serial.println(F("System should resume"));
  }
}

void fullSweep() {
  Serial.println(F("Running full sweep..."));
  Serial.print(F("\n"));
  sweep(shoulderRoll,"shoulderRoll");
  delay(200);
  sweep(shoulderPitch,"shoulderPitch");
  delay(200);
  sweep(elbow,"elbow");
  delay(200);
  sweep(wristPitch,"wristPitch");
  delay(200);
  sweep(wristRoll,"wristRoll");
  delay(200);
  sweep(grabber,"grabber");
  delay(200);
}


void sweep(Servo servo,String servoName) {
  Serial.print(F("Running sweep on "));
  Serial.print(servoName);
  Serial.print(F("\n"));
  int delayNum;
  //Check if bidDaddyServo, if so run slow
  if (servoName == "shoulderPitch") {
    delayNum = 100;
  } else{
    delayNum = 40;
  }

  int pos = 90;
  
  for(pos=90;pos<=130;pos+=1) {
    writePos(servo,pos,delayNum);
  }
  for(pos=130;pos>=45;pos-=1) {
    writePos(servo,pos,delayNum);
  }
  for(pos=45;pos<=90;pos+=1) {
    writePos(servo,pos,delayNum);
  }
}

void moveToAngle(Servo servo, String servoName, int angle) {
  Serial.print(F("Running sweep on "));
  Serial.print(servoName);
  Serial.print(F("\n"));
  int delayNum;
  int currentAngle = getCurrentAngle();
  int pos = 90;
  //Check if bidDaddyServo, if so run slow
  if (servoName == "shoulderPitch") {
    delayNum = 100;
  } else{
    delayNum = 40;
  }
  // To avoid jerking, know where you're starting
  if(currentAngle) {
    pos = currentAngle;
  } 
  // Move to needed position from currentAngle
  for(pos;pos<=angle;pos+=1) {
    writePos(servo,pos,delayNum);
  }
  
  for(pos=45;pos<=90;pos+=1) {
    writePos(servo,pos,delayNum);
  }
}

/**
 * Get current angle (last value passed)
 */
int getCurrentAngle(Servo servo){
  return servo.read();
}

void openGrabber() {
  grabber.write(180);
}
void closeGrabber() {
  grabber.write(0);
}
void writePos(Servo servo,int pos,int delayNum) {
  servo.write(pos);
  Serial.print(F("Position: "));
  Serial.print(pos);
  Serial.print(F("\n"));
  delay(delayNum);
}

void movePositionOne() {
  

  shoulderRoll.write(shoulderRollAngle);
  delay(300);
  shoulderPitch.write(shoulderPitchAngle);
  delay(300);
  wristPitch.write(wristPitchAngle);
  delay(300);
  elbow.write(elbowAngle);
  delay(300);
  wristRoll.write(wristRollAngle);
  delay(300);
  grabber.write(grabberAngle);
  delay(300);
}*/
//Values for Linear actuator open 252, close 127




