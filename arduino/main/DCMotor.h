#ifndef DCMotor_h
#define DCMotor_h

#include <Arduino.h>

class DCMotor{public:
	int EN_PIN, A_PIN, B_PIN;
	int Speed, dir;

	DCMotor(int en, int a, int b);
	//Speed [-255, 255]
	void setSpeed(int _Speed);
	void update();
	void set_dir(int );
	void stop();
};

#endif //DCMotor_h