#include "Sonar.h"

uint8_t Sonar::currentSensor = 0;
Sonar* Sonar::singleton = NULL;

Sonar* Sonar::getInstance() {
  return singleton;
}

Sonar::Sonar(){
  singleton = this;
}

Sonar::Sonar(void (*cycle)(unsigned int*)){
  callback = cycle;
  singleton = this;
}

void Sonar::addSonar(int trigger, int echo){
  sonar[SONAR_NUM++] = NewPing(trigger, echo, MAX_DISTANCE);
}

void Sonar::init() {
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
}

void Sonar::checkSonar() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
}

void Sonar::echoCheck(){ 
  // If ping received, set the sensor distance to array.
  Sonar* sonar = Sonar::getInstance();
  int currentSensor = sonar->currentSensor;
  if (sonar->sonar[currentSensor].check_timer())
    sonar->cm[currentSensor] = sonar->sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

void Sonar::oneSensorCycle(){
  // Sensor ping cycle complete, do something with the results.
  if(FilterType == 0) {
    BasicFilter();
    callback(cm);
  } else if(Measurements <= FILTER_SIZE) {
    updateFilter();
  } else{
    sortMeasurments();
    switch(FilterType) {
      case 1:
        MedianFilter();
      break;
      case 2:
        IQRMedianFilter();
      break;
    }
    Measurements = 0;
    callback(cm);
  }
}

void Sonar::updateFilter(){
  for(int i = 0; i < SONAR_NUM; i++){
    FilterMeasurments[i][Measurements] = cm[i];
  }
  Measurements++;
}

void Sonar::sortMeasurments(){
  int tmp, i, j;
  boolean sorted = false;
  
  while(!sorted){
    sorted = true;
    for(i = 0; i < SONAR_NUM; i++)
      for(j = 1; j < FILTER_SIZE; j++)
        if(FilterMeasurments[i][j - 1] > FilterMeasurments[i][j]){
          tmp =  FilterMeasurments[i][j - 1];
          FilterMeasurments[i][j - 1] = FilterMeasurments[i][j];
          FilterMeasurments[i][j] = tmp;
          sorted = false;
        }
  }
}

void Sonar::MedianFilter() {
  for(int i = 0; i < SONAR_NUM; i++)
    cm[i] =  FilterMeasurments[i][FILTER_SIZE / 2];
}

void Sonar::IQRMedianFilter() {
  int i, j;
  int q1, q3, iqr, l1, l2;
  int tmp[FILTER_SIZE], tmpLength = 0;
  for(i = 0; i < SONAR_NUM; i++) {
    
    q1 = FilterMeasurments[i][FILTER_SIZE/4];
    q3 = FilterMeasurments[i][FILTER_SIZE*3/4];
    iqr = q3 - q1;
    l1 = q1 - 1.5 * iqr;
    l2 = q3 - 1.5 * iqr;  
    
    for(j = 0; j < FILTER_SIZE; j++) {
      
      if(FilterMeasurments[i][j] < l2 && FilterMeasurments[i][j] > l1) {
        tmp[tmpLength++] = FilterMeasurments[i][j];
      }
      
    }
    cm[i] =  tmp[tmpLength / 2];
  }
}

void Sonar::BasicFilter() {
  for(int i = 0; i < SONAR_NUM; i++){
    
    if(cm[i] >= BASIC_MAX_DISTANCE || cm[i] == 0) {
      cm[i] = BASIC_MAX_DISTANCE;
    }
  }
}