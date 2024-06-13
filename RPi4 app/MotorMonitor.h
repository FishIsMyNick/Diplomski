#include <iostream>
#include <chrono>
#include <wiringPi.h>
#include <atomic>

const int detectMagnet = 24;

class MotorMonitor {
private:
	bool measure;
	bool motorInMotion;
	std::atomic<bool> detectMagnetRE;
	
	void handleEdgeDetect();
	static void edgeDetectWrapper();
	static MotorMonitor* instance;

public:
	MotorMonitor();
	void setupMonitor();
	bool getDetectMagnetRE() const;

	
	void setMotorInMotion(bool value);
	void setMeasure(bool value);
	void setMagnetRE(bool value);
	
	void measureSpeed(float* rpm);
	void stopMeasuringSpeed();
};
