#ifndef ServoMotor_h
#define ServoMotor_h

#include <Arduino.h>
#include <Servo.h>

class ServoMotor{public:
	static int PWM_INIT;
	static int PWM_STOP;
	static int PWM_F_MIN;
	static int PWM_F_MAX;
	static int PWM_B_MIN;
	static int PWM_B_MAX;
	
	Servo servo;
	int PIN;
	int dir = 1; // 0 Backward, 1 Forward
	int PWM = PWM_INIT;

	ServoMotor(int pin);
	//Speed [-255, 255]
	void setSpeed(int _Speed);
	void update();
	void stop();
	int limitTractionSpeed(int Speed);
};

#endif //ServoMotor_h
