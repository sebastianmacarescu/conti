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

SoftwareSerial blueSerial(2, 3); // RX, TX


int dLeft, dFront, dRight;
int angle_camera;

boolean IsAutonomous = false;

/*---------PINS-----------*/
uint8_t steeringPin = 10; //
uint8_t tractionPin = 9; //PWM Brushless

uint8_t ECHO_LEFT = 6;
uint8_t TRIGGER_LEFT = 7;

uint8_t ECHO_RIGHT = 11;
uint8_t TRIGGER_RIGHT = 12;

unsigned int DataSampleTime = 1000; //ms. SampleTime for other data
unsigned int SerialDataTime = 300; //ms. Time period for sending data to car

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
	
	//Init other things
	steeringMotor = new DirectionMotor(steeringPin);
	tractionMotor = new ServoMotor(tractionPin);

	Serial.println("Setup complete");
	//blueSerial.println("Setup complete");
}

void loop(){
  
	bCmd.readBus();
	if(bCmd.isReadyToParse())
		bCmd.Parse();
	
	mysonar->checkSonar();

	delay(10);
	//updateSensorData();
	sendSerialData();
	
	//Code for memory
	//Serial.print("freeMemory()=");
	//Serial.println(freeMemory());
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
  angle_camera = angle;
	cameraValue = map(angle, -40, 40, 255, -255);
	isCameraRunning = true;
}

void oneSensorCycle(unsigned int cm[]) { 
	int steerValue = 0;

	// Sensor ping cycle complete, do something with the results.
	dLeft = cm[0]; //index depends on sensor addition order in setup
	dRight = cm[1]; 
	// dFront = cm[2];

	if(!IsAutonomous) return;	
	


	// if(dLeft < 80 || dRight < 80) {
	// 	int sensorsValue = dLeft - dRight;
	// 	sensorsValue = map(sensorsValue, -80, 80, -255, 255);
	// 	steerValue = sensorsValue;
	// 	if(isCameraRunning) {
	// 		//Serial.println(isCameraRunning);
	// 		steerValue = cameraValue * 0.6 + sensorsValue * 0.4;
	// 	}
	// 	Serial.println("Steer_val " + steerValue);
	// 	steer(steerValue); 
	// } 
	// else {
	// 	steer(steerValue);
	// }


	int sensorsValue = dLeft - dRight;
	sensorsValue = map(sensorsValue, -80, 80, -255, 255);
	steerValue = sensorsValue;
	if(isCameraRunning) {
		steerValue = cameraValue * 0.6 + sensorsValue * 0.4;
	}
	Serial.println("Steer_val " + steerValue);
	steer(steerValue);
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

		// dLeft dRight camera traction steer
		toSend += String(" ") + dLeft;
		toSend += String(" ") + dRight;
		toSend += String(" ") + angle_camera;
		toSend += String(" ") + tractionMotor->raw_value;
		toSend += String(" ") + steeringMotor->raw_value;

		toSend += ";";
		blueSerial.println(toSend);
		//debug(String("Data to send ") + toSend, DEBUG_SENSOR_DATA);
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
