#include<Servo.h>
#include<SPI.h>
#include<Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX
#define motor_left 3 
#define motor_right 6
#define pwm_stop 1450
#define a1 7 
#define a2 8
#define pwm 5

int pwm_left=1400, pwm_right=1400, val_pwm=127;
Servo servo_left, servo_right;
boolean start;
void setup(){

Serial.begin(9600);
pinMode(motor_left,OUTPUT);
pinMode(motor_right,OUTPUT);
pinMode(a1,OUTPUT);
pinMode(a2,OUTPUT);
pinMode(pwm, OUTPUT);
pinMode(10, INPUT);
pinMode(11,OUTPUT);


servo_left.attach(motor_left);
servo_right.attach(motor_right);


//servo.writeMicroseconds(pwm_val);
  
}

void loop(){

  if (Serial.available()>0)
{
  char c=Serial.read();
  if (c=='a')
  pwm_left-=10;
    if (c=='p')
  pwm_left=pwm_stop;      //PWM DE OPRIRE!!!!!!
  if (c=='q')
  pwm_left+=10;
  if (c=='s')
  start=false;
  if (c=='w')
  start=true;
  if(c=='[')
  {
    digitalWrite(a1, HIGH);
 digitalWrite(a2, LOW);
 analogWrite(pwm,val_pwm); 
  }
  if(c==']')
  {
     digitalWrite(a1, LOW);
  digitalWrite(a2, HIGH);
  analogWrite(pwm, val_pwm);
  }
  
  pwm_right=pwm_left; 

}
  if (start==true)
  {
   servo_left.writeMicroseconds(pwm_left);
  servo_right.writeMicroseconds(pwm_right);
  }
  else if (start==false)
  {
    servo_left.writeMicroseconds(pwm_stop);
  servo_right.writeMicroseconds(pwm_stop); 
  }
  Serial.println(pwm_left);
 // Serial.println(pwm_left);
}




  
