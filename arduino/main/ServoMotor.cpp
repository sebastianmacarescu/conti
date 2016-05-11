#include "ServoMotor.h"

int ServoMotor::PWM_STOP = 1450;
int ServoMotor::PWM_INIT = 1450;
int ServoMotor::PWM_F_MIN = 1540;
int ServoMotor::PWM_F_MAX = 1560;
int ServoMotor::PWM_B_MIN = 1350;
int ServoMotor::PWM_B_MAX = 1300;

ServoMotor::ServoMotor(int pin){
	PIN = pin;
	servo.attach(PIN);
}

//Speed [-255, 255]
void ServoMotor::setSpeed(int _Speed){
	if(_Speed == 0)
		PWM = PWM_STOP;
	else 
		PWM = 1550;
	return;
	_Speed = limitTractionSpeed(_Speed);
	if(_Speed < 0) {
		PWM = map(_Speed, -255, 0, PWM_B_MIN, PWM_B_MAX);
		dir = 0;
	} else {
		PWM = map(_Speed, 0, 255, PWM_F_MIN, PWM_F_MAX);
		dir = 1;
	}
}
void ServoMotor::update(){
	servo.writeMicroseconds(PWM);
}
void ServoMotor::stop(){
	servo.writeMicroseconds(PWM_STOP);
	PWM = PWM_STOP;
}


int ServoMotor::limitTractionSpeed(int Speed) {
  if(Speed < PWM_F_MIN)
    Speed = PWM_F_MIN;
  if(Speed > PWM_F_MAX)
    Speed = PWM_F_MAX;
  return Speed;
}
