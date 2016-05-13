#ifndef Sonar_h
#define Sonar_h

#include <Arduino.h>
#include "NewPing.h"

class Sonar{public:
	static const int MAX_SONAR_NUM = 5;
	static const int MAX_DISTANCE = 300;
	static const int PING_INTERVAL = 50;
	static const int FILTER_SIZE = 11;
	static const int BASIC_MAX_DISTANCE = 50;
	static Sonar* singleton;
	
	int SONAR_NUM = 0;
	unsigned long pingTimer[MAX_SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
	unsigned int cm[MAX_SONAR_NUM];         // Where the ping distances are stored.
	static uint8_t currentSensor;          // Keeps track of which sensor is active.
	int FilterType = 0; //0 - basic, 1 - median, 2 - IRQ median, 

	int Measurements = 0;
	unsigned int FilterMeasurments[5][15];

	NewPing sonar[MAX_SONAR_NUM];

	void (*callback)(unsigned int*);

	Sonar();
	Sonar(void (*cycle)(unsigned int*));

	void addSonar(int trigger, int echo);
	void init();
	void checkSonar();
	static Sonar* getInstance();

private:
	static void echoCheck();
	void oneSensorCycle();
	void updateFilter();
	void sortMeasurments();
	void MedianFilter();
  void IQRMedianFilter();
  void BasicFilter();
};

#endif //Sonar_h
