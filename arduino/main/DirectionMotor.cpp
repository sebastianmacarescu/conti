#include "DirectionMotor.h"

DirectionMotor::DirectionMotor(int pin){
  PIN = pin;
  servo.attach(PIN);
  dir = 90;
}

//Speed [-255, 255]
void DirectionMotor::setDirection(int _Speed){
  int _dir = map(_Speed, -255, 255, 20, 160);
  this->dir = _dir;
  servo.write(_dir);
}
void DirectionMotor::update(){
}

void DirectionMotor::stop(){
  dir = 90;
  servo.write(dir);
}
