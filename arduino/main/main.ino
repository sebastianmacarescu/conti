#include <Servo.h>
#include <SoftwareSerial.h>

#include "busCommand.h"
#include "DCMotor.h"
#include "Sonar.h"
#include "ServoMotor.h"


/*---------DEFINES-----------*/


/*---------BLUETOOTH RESPONSE COMMANDS-----------*/
String BlueSensorData = "sensors";

/*---------VARIABLES-----------*/
bool DEBUG = true;
bool DEBUG_SENSOR_DATA = true;

SoftwareSerial blueSerial(2, 3); // RX, TX

int dLeft, dFront, dRight;
boolean IsAutonomous = false;

/*---------PINS-----------*/
uint8_t steeringB = 7;
uint8_t steeringA = 8;
uint8_t steeringEn = 5;
uint8_t tractionPin = 6;

uint8_t ECHO_LEFT = 13;
uint8_t TRIGGER_LEFT = 12;

uint8_t ECHO_RIGHT = 11;
uint8_t TRIGGER_RIGHT = 10;

unsigned int DataSampleTime = 1000; //ms. SampleTime for other data
unsigned int SerialDataTime = 1000; //ms. Time period for sending data to car

busCommand bCmd;
DCMotor* steeringMotor;
ServoMotor* tractionMotor;
Sonar* mysonar;

NewPing leftSonar(TRIGGER_LEFT, ECHO_LEFT, Sonar::MAX_DISTANCE);
NewPing rightSonar(TRIGGER_RIGHT, ECHO_RIGHT, Sonar::MAX_DISTANCE);

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
	bCmd.setDefaultHandler(unrecognized);
	
	//Comment this line to receive commands over standard serial
	bCmd.setSoftwareSerial(&blueSerial);
	
	// mysonar = new Sonar(oneSensorCycle);
	// mysonar->addSonar(TRIGGER_LEFT, ECHO_LEFT);
	// mysonar->addSonar(TRIGGER_RIGHT, ECHO_RIGHT);
	// mysonar->init();

	//Init other things
	steeringMotor = new DCMotor(steeringEn, steeringA, steeringB);
	tractionMotor = new ServoMotor(tractionPin);

	Serial.println("Setup complete");
	blueSerial.println("Setup complete");
}

void loop(){

	bCmd.readBus();
	if(bCmd.isReadyToParse())
		bCmd.Parse();
	//mysonar->checkSonar();
	//replace delay with static member millis
	//oneSensorCycle();
	steeringMotor->update();
	tractionMotor->update();
	delay(100);
	//updateSensorData();
	//sendSerialData();
	
	//Code for memory
	//Serial.print("freeMemory()=");
	//Serial.println(freeMemory());
	//
}

void autoNom(){
	if(IsAutonomous) {
		//Stop motors
		stopMotors();
	}
	else {
			tractionMotor->setSpeed(42);
	}
	IsAutonomous = !IsAutonomous;
}

void oneSensorCycle() { 
	if(!IsAutonomous) return;
	// Sensor ping cycle complete, do something with the results.
	dLeft = leftSonar.ping_median() / US_ROUNDTRIP_CM;
	dRight = rightSonar.ping_median() / US_ROUNDTRIP_CM;
	// Serial.print("; LF "); Serial.print(cm[0]);
	// Serial.print("; F "); Serial.print(cm[1]);
	// Serial.print("; RF "); Serial.print(cm[2]);
	// Serial.println("}");
	
	// Speed = 120, dLeft = cm[0], dFront = cm[1], dRight = cm[2];
	// if(dFront == 0) dFront = MAX_DISTANCE;
	if(dLeft == 0) dLeft = 39;
	if(dRight == 0) dRight = 39;

	if(dLeft < 40)
	steer(-250);
	else if(dRight < 40)
	steer(250);
	else
	steer(0);
	
	Serial.print("Done ");
	Serial.print(dLeft); Serial.print(" ");
	Serial.print(dRight); Serial.print(" ");
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
	steeringMotor->setSpeed(0);
	tractionMotor->setSpeed(0);
	debug(String(F("Motors stopped")), DEBUG);
}

void brake(){
	steeringMotor->setSpeed(0);
}

void steer(){
	char *arg;
	int Speed = 0;

	arg = bCmd.getArg();
	if(arg != NULL) Speed = atoi(arg);

	steeringMotor->setSpeed(Speed);   
	
	//delay(500);

	//analogWrite(steeringPinE, abs(Speed));
	debug(String(F("Steering signal set to ")) + Speed, DEBUG); 
}

void steer(int Speed){
	steeringMotor->setSpeed(Speed);
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
