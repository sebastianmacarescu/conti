#ifndef DirectionMotor_h
#define DirectionMotor_h

#include <Arduino.h>
#include <Servo.h>
class DirectionMotor{public:
  int PIN;
  Servo servo;
  int dir;

  DirectionMotor(int pin);
  //Speed [-255, 255]
  void setDirection(int _Speed);
  void update();
  void set_dir(int );
  void stop();
};

#endif //DirectionMotor_h
