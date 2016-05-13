#include <PWMServo.h>
#include <SoftwareSerial.h>
//#include <PID_v1.h>

#include "busCommand.h"
#include "DirectionMotor.h"
#include "Sonar.h"
#include "ServoMotor.h"


/*---------DEFINES-----------*/ 


/*---------BLUETOOTH RESPONSE COMMANDS-----------*/
String BlueSensorData = "sensors";

/*---------VARIABLES-----------*/
bool DEBUG = true;
bool DEBUG_SENSOR_DATA = true;

int isCameraRunning = false;
int cameraValue = 0;
//int SetpointDirection, InputDirection, OutputDirection;
//int SetpointThrottle, InputThrottle, OutputThrottle;

//PID DirectionPID(&InputDirection, &OutputDirection, &SetpointDirection, 2, 5, 1, DIRECT);
//PID ThrottlePID(&InputThrottle, &OutputThrottle, &SetpointThrottle, 2, 5, 1, DIRECT);

SoftwareSerial blueSerial(2, 3); // RX, TX


//int dLeft, dFront, dRight;
boolean IsAutonomous = false;

/*---------PINS-----------*/
uint8_t steeringPin = 10; //
uint8_t tractionPin = 9; //PWM Brushless

uint8_t ECHO_LEFT = 6;
uint8_t TRIGGER_LEFT = 7;

uint8_t ECHO_RIGHT = 11;
uint8_t TRIGGER_RIGHT = 12;

unsigned int DataSampleTime = 1000; //ms. SampleTime for other data
unsigned int SerialDataTime = 1000; //ms. Time period for sending data to car

busCommand bCmd;
Sonar* mysonar;


DirectionMotor* steeringMotor;
ServoMotor* tractionMotor;

//NewPing leftSonar(TRIGGER_LEFT, ECHO_LEFT, Sonar::MAX_DISTANCE);
//NewPing rightSonar(TRIGGER_RIGHT, ECHO_RIGHT, Sonar::MAX_DISTANCE);

void oneSensorCycle(unsigned int cm[]);

void setupBlueSerial() {
	blueSerial.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  blueSerial.print("$");  // Print three times individually
  blueSerial.print("$");
  blueSerial.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  blueSerial.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  blueSerial.begin(9600);  // Start bluetooth serial at 9600
}

void setup(){
  setupBlueSerial();
	Serial.begin(9600);
  
	//Set commands
	bCmd.addCommand("s", stopMotors);
	bCmd.addCommand("steer", steer);
	bCmd.addCommand("t", traction);
	bCmd.addCommand("b", brake);
	bCmd.addCommand("a", autoNom);
  bCmd.addCommand("camera", camera);
	bCmd.setDefaultHandler(unrecognized);
	
	//Comment this line to receive commands over standard serial
	bCmd.setSoftwareSerial(&blueSerial);
	//Setup sensors
	mysonar = new Sonar(oneSensorCycle);
	mysonar->addSonar(TRIGGER_LEFT, ECHO_LEFT);
	mysonar->addSonar(TRIGGER_RIGHT, ECHO_RIGHT);
	mysonar->init();
	//setup PID
	// InputDirection = 0;
	// SetpointDirection = 50;
	// DirectionPID.SetMode(AUTOMATIC);
	// DirectionPID.SetOutputLimits(-255, 255);
	//Init other things
	steeringMotor = new DirectionMotor(steeringPin);
	tractionMotor = new ServoMotor(tractionPin);

	Serial.println("Setup complete");
	//blueSerial.println("Setup complete");
}

void loop(){
  //Input = ..
  //myPid.Compute();
  //analogWrite(3, Output);
  
  //Serial.println("alive");
	bCmd.readBus();
	if(bCmd.isReadyToParse())
		bCmd.Parse();
	if(IsAutonomous) {
		mysonar->checkSonar();
	}
	//Serial.println(mysonar->cm[0]);
	//replace delay with static member millis
	//oneSensorCycle();
	delay(10);
	//updateSensorData();
	//sendSerialData();
	
	//Code for memory
	//Serial.print("freeMemory()=");
	//Serial.println(freeMemory());
	//
	isCameraRunning = false;
}

void autoNom(){
	if(IsAutonomous) {
		//Stop motors
		stopMotors();
		//tractionMotor->setSpeed(42);
	}
	else {
		tractionMotor->setSpeed(42);
	}
	IsAutonomous = !IsAutonomous;
	debug(String(F("Autonomous toggled")), DEBUG);
}

void camera() {
  char *arg;
  int angle = 0;
  arg = bCmd.getArg();
	if(arg != NULL) angle = atoi(arg);
  
  if(IsAutonomous) {
  	cameraValue = map(angle, -90, 90, -255, 255);
  	isCameraRunning = true;
	  // if(angle > 5) {
	  //   steer(-255);
	  // } else if(angle < -5) {
	  //   steer(255);
	  // } else {
	  // 	steer(0);
	  // }
	}
}

void oneSensorCycle(unsigned int cm[]) { 
	//blueSerial.print(cm[0]);
	//blueSerial.print(" ");
	//blueSerial.println(cm[1]);

	int dLeft, dRight, dFront;
	int steerValue = 0;
	if(!IsAutonomous) return;

	// Sensor ping cycle complete, do something with the results.
	dLeft = cm[0]; //index depends on sensor addition order in setup
	dRight = cm[1]; 
	// dFront = cm[2];
	// Serial.print("; LF "); Serial.print(cm[0]);
	// Serial.print("; F "); Serial.print(cm[1]);
	// Serial.print("; RF "); Serial.print(cm[2]);
	// Serial.println("}");
	
	// Speed = 120, dLeft = cm[0], dFront = cm[1], dRight = cm[2];
	// if(dFront == 0) dFront = MAX_DISTANCE;
	// if(dLeft == 0) dLeft = 39;
	// if(dRight == 0) dRight = 39;

	// if(dLeft < 50 || dRight < 50) {
	// 	InputDirection = dLeft < dRight ? dLeft : dRight;
	// 	DirectionPID.Compute();
	// 	Output = dLeft < dRight ? Output : Output * (-1);
	// 	steer(Output);
	// } else {
	// 	steer(0);
	// }
	if(dLeft < 50 || dRight < 50) {
		int sensorsValue = dLeft - dRight;
		sensorsValue = map(sensorsValue, -50, 50, -255, 255);
		steerValue = sensorsValue;
		if(isCameraRunning) {
			steerValue = cameraValue * 0.4 + sensorsValue * 0.6;
		}
		steer(steerValue); 
	} else {
		steer(steerValue);
	}
	
	Serial.print("Done ");
	Serial.print(dLeft); Serial.print(" ");
	Serial.print(dRight); Serial.print(" ");
	Serial.print(steerValue); Serial.print(" ");
	Serial.println("");
	
	//todo: algorithm here  
	
}

void updateSensorData(){
	static unsigned long lastUpdate = 0;
	unsigned long now = millis();

	if(now - lastUpdate > DataSampleTime){
		//Analog data (Temp and steering pos)
		//tractionMotorTemp = analogRead(MOTOR_TEMP_PIN);
		//steeringMotorTemp = tractionMotorTemp;
		//steeringPos = analogRead(STEERING_POS_PIN);
		//battVoltage = analogRead(BAT_VOLTAGE_PIN);
		//battVoltage = map(battVoltage, 0, 1024, 0, 10);
		//debug(String("batt read ") + battVoltage, DEBUG);
		lastUpdate = now;
	}
}

void sendSerialData(){
	static unsigned long lastSendTime = millis();
	unsigned long now = millis();

	if(now - lastSendTime > SerialDataTime){
		//Send data
		String toSend = String("");
		toSend += BlueSensorData;

		//tractionPWM, steeringPWM, steeringMotorTemp, tractionMotorTemp, steeringPos
		//motorRPM, carSpeed, distanceTraveled, battVoltage
		//toSend += String(" ") + tractionServoSignal;
		//toSend += String(" ") + steeringServoSignal;
		//toSend += String(" ") + steeringPos;
		//toSend += String(" ") + distance;

		//blueSerial.println(toSend);
		debug(String("Data to send ") + toSend, DEBUG_SENSOR_DATA);
		lastSendTime = now;
	}
}

/*---------COMMANDS-----------*/


void stopMotors(){
	steeringMotor->setDirection(0);
	tractionMotor->setSpeed(0);
	debug(String(F("Motors stopped")), DEBUG);
}

void brake(){
	//steeringMotor->stop();
}

void steer(){
	char *arg;
	int Speed = 0;

	arg = bCmd.getArg();
	if(arg != NULL) Speed = atoi(arg);

	steeringMotor->setDirection(Speed);   	
	debug(String(F("Steering signal set to ")) + Speed, DEBUG); 
}

void steer(int Speed){
	steeringMotor->setDirection(Speed);
}

void traction(){
	char *arg;
	int Speed = 0;
	arg = bCmd.getArg();
	if(arg != NULL) Speed = atoi(arg);
	tractionMotor->setSpeed(Speed);
	debug(String(F("Traction signal set to ")) + Speed, DEBUG); 
}

void unrecognized(const char *command) {
	Serial.println("What?");
}

/*---------HELPERS-----------*/


void debug(String msg, bool debugVar){
	if(debugVar)
	Serial.println(msg);
}
