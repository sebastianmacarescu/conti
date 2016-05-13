#ifndef DirectionMotor_h
#define DirectionMotor_h

#include <Arduino.h>
#include <PWMServo.h>
class DirectionMotor{public:
  int PIN;
  PWMServo servo;
  int dir;
  int raw_value;

  DirectionMotor(int pin);
  //Speed [-255, 255]
  void setDirection(int _Speed);
  void update();
  void stop();
};

#endif //DirectionMotor_h
