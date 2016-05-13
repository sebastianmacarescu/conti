#include "PWMServo.h"
uint8_t steeringPin = 10; //
PWMServo servo;
char c;
int angle = 90;
void setup() {
	Serial.begin(9600);
	servo.attach(steeringPin);
}
void loop() {
	if(Serial.available()) {
		char c = Serial.read();
		if(c == 'A')
			angle -= 1;
		if(c == 'D')
			angle += 1;
		Serial.println(angle);
	}
	servo.write(angle);
}
