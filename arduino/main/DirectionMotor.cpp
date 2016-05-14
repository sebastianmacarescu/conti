#include "DirectionMotor.h"

DirectionMotor::DirectionMotor(int pin){
  PIN = pin;
  servo.attach(PIN);
  this->dir = MIDDLE_ANGLE;
  servo.write(this->dir);
  raw_value = 0;
}

//Speed [-255, 255]
void DirectionMotor::setDirection(int _Speed){
  if(_Speed == 0) {
    this->dir = MIDDLE_ANGLE;
  } else {
    this->dir = map(_Speed, -255, 255, 63, 107);
  }
  servo.write(this->dir);
  raw_value = _Speed;
}
void DirectionMotor::update(){
}

void DirectionMotor::stop(){
  dir = 90;
  raw_value = 0;
  servo.write(dir);
}
