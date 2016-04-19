#include "DCMotor.h"

DCMotor::DCMotor(int en, int a, int b){
	EN_PIN = en;
	A_PIN = a;
	B_PIN = b;
	pinMode(EN_PIN, OUTPUT);
	pinMode(A_PIN, OUTPUT);
	pinMode(B_PIN, OUTPUT);
	
	Speed = dir = 0;
	digitalWrite(A_PIN, dir);
	digitalWrite(B_PIN, !dir);
}

//Speed [-255, 255]
void DCMotor::setSpeed(int _Speed){
	if(_Speed == 0) {
		stop();
	}
	else if(_Speed < 0)
		set_dir(1);
	else set_dir(0);
	Speed = _Speed;
}
void DCMotor::update(){
	analogWrite(EN_PIN, abs(Speed));
}
void DCMotor::stop(){
	digitalWrite(A_PIN, 0);
	digitalWrite(B_PIN, 0);
	analogWrite(EN_PIN, 0);
	Speed = 0;
}
void DCMotor::set_dir(int _dir){
	dir = _dir;
	digitalWrite(A_PIN, dir);
	digitalWrite(B_PIN, !dir);
	Serial.print("Changed dir to ");
	Serial.println(dir);
}