#include "DirectionMotor.h"

DirectionMotor::DirectionMotor(int pin){
  PIN = pin;
  servo.attach(PIN);
  //Speed = dir = 0;
  //digitalWrite(A_PIN, dir);
  //digitalWrite(B_PIN, !dir);
}

//Speed [-255, 255]
void DirectionMotor::setDirection(int _Speed){
  int dir = map(_Speed, -255, 255, 10, 150);
  set_dir(dir);
}
void DirectionMotor::update(){
  servo.write(this->dir);
}

void DirectionMotor::stop(){
  servo.detach();
  //Serial.println("detached - !!!!!!");
  servo.attach(this->PIN);
}
void DirectionMotor::set_dir(int _dir){
  //dir = _dir;
  /*int pos;
  for (pos = 0; pos <= dir; pos = dir > pos ? pos + 1 : pos - 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }*/
  this->dir = _dir;
  servo.write(this->dir);
}
