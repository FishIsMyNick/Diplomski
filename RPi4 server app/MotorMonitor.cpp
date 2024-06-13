#include "MotorMonitor.h"

MotorMonitor* MotorMonitor::instance = nullptr;

MotorMonitor::MotorMonitor() : detectMagnetRE(false) {
	instance = this;
	setupMonitor();
}

//	SETUP	////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

/// Set up pins and control variables
void MotorMonitor::setupMonitor() {	
	pinMode(detectMagnet, INPUT);
	wiringPiISR(detectMagnet, INT_EDGE_RISING, &MotorMonitor::edgeDetectWrapper);

	detectMagnetRE = false;
	measure = false;
	motorInMotion = false;
}

/// Rising edge handle function
void MotorMonitor::handleEdgeDetect() {
	detectMagnetRE.store(true);
}

///	Rising edge wrapper function
void MotorMonitor::edgeDetectWrapper() {
	if(instance) {
		instance -> handleEdgeDetect();
	}
}

////////////////////////////////////////////////////////////////////////

///	'motorInMotion' setter
void MotorMonitor::setMotorInMotion(bool value) {
	motorInMotion = value;
}
///	'measure' setter
void MotorMonitor::setMeasure(bool value) {
	measure = value;
}

/// Measures the time between 2 rising edges on the detection pin
/// Sets the 'rpm' variable accordingly
void MotorMonitor::measureSpeed(float* rpm) {
	unsigned long lastHit = millis();
	unsigned long hitTime;
	measure = true;
	
	while (measure) {
		if (!motorInMotion) {
			*rpm = 0.0f;
		}
		else if (detectMagnetRE) {
			detectMagnetRE = false;
			hitTime = millis();
			*rpm = (1000.0f / (hitTime - lastHit)) * 60.0f;
			lastHit = hitTime;
		}

		delay(1);
	}
	std::cerr << "MONITOR STOP MEASURE\n";
}

///	Stops measuring speed
void MotorMonitor::stopMeasuringSpeed() {
	measure = false;
}
