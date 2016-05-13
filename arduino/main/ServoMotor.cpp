#include "ServoMotor.h"

int ServoMotor::PWM_STOP = 0;
int ServoMotor::PWM_INIT = 1450;
int ServoMotor::PWM_F_MIN = 1540;
int ServoMotor::PWM_F_MAX = 1560;
int ServoMotor::PWM_B_MIN = 1350;
int ServoMotor::PWM_B_MAX = 1300;

#define ANGLE_STOP 95
#define ANGLE_MIN_A 97
#define ANGLE_MIN_B 75


ServoMotor::ServoMotor(int pin){
	PIN = pin;
	servo.attach(PIN);
}

//Speed [-255, 255]
void ServoMotor::setSpeed(int _Speed){
	if(_Speed == 0)
		PWM = ANGLE_STOP;
	else if(_Speed == 50) 
		PWM = ANGLE_MIN_A;
	else if(_Speed == 100) 
		PWM = ANGLE_MIN_A + 1;
	else if(_Speed == 150) 
		PWM = ANGLE_MIN_A + 2;
	
	servo.write(PWM);
	return;
	//_Speed = limitTractionSpeed(_Speed);
	if(_Speed < 0) {
		PWM = map(_Speed, -255, 0, PWM_B_MIN, PWM_B_MAX);
		dir = 0;
	} else {
		PWM = map(_Speed, 0, 255, PWM_F_MIN, PWM_F_MAX);
		dir = 1;
	}
}
void ServoMotor::update(){
	//servo.writeMicroseconds(PWM);
}
void ServoMotor::stop(){
	servo.write(ANGLE_STOP);
	PWM = ANGLE_STOP;
}


int ServoMotor::limitTractionSpeed(int Speed) {
  if(Speed < PWM_F_MIN)
    Speed = PWM_F_MIN;
  if(Speed > PWM_F_MAX)
    Speed = PWM_F_MAX;
  return Speed;
}
