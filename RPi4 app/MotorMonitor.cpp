#include <iostream>

const int detectMagnet = 24;

bool detectMagnetRE;
bool measure;
bool motorInMotion;


//	SETUP	////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void risingEdgeCallback() {
    detectMagnetRE = true;
    std::cerr << "MAGNET\n";
}

int setupMonitor() {
    pinMode(detectMagnet, INPUT);
    wiringPiISR(detectMagnet, INT_EDGE_RISING, &risingEdgeCallback);
    
    detectMagnetRE = false;
    measure = false;
    motorInMotion = false;
    
	return 0;
}

////////////////////////////////////////////////////////////////////////


void setMotorInMotion(bool value) {
	motorInMotion = value;
}
void setMeasure(bool value) {
	measure = value;
}

/// Use this one
void measureSpeed(float* rpm, int pollRate) {
	unsigned long lastHit = millis();
	unsigned long hitTime;
	unsigned long t = millis();
	measure = true;
	int timeout = 2000;
	while(measure) {
		if(detectMagnetRE) {
			detectMagnetRE = false;
			hitTime = millis();
			*rpm = (1000.0f / (hitTime - lastHit)) * 60.0f;
			lastHit = hitTime;
		}
		if(!motorInMotion) {
			*rpm = 0.0f;
		}
		if(t + pollRate < millis()) {
			//if (allowDebugRPM) std::cerr << "RPM: " << *rpm << std::endl;
			t = millis();
		}
		
		delay(1);
	}
	std::cerr<<"MONITOR STOP MEASURE\n";
}

void stopMeasuringSpeed() {
	measure = false;
}
