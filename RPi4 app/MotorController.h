#include "MotorMonitor.h"
#include <cmath>
#include <chrono>
#include <thread>

const int motorCW = 17;
const int motorCCW = 18;

const int maxSpeed = 120;  // equivalent 12V

const int frequency = 1000; // Hz
const float tick = 1.0 / frequency; // s

class MotorController {
private:
	int lastPowerSet;
	int lastDirection;
	int acceleration;
	
	int pwmOnTime, pwmOffTime;

public:
	int setupController();
	
	void setAcceleration(int accel);

	void calculatePWM(float);
	void powerMotorPWM(int direction);
	void accelerateMotor(int direction, float speed);
	int stopMotor();
	int turnMotor(int direction, float desiredSpeed, float mDuration);
	int turnCW(float speed, float duration);
	int turnCCW(float speed, float duration);
	
	MotorMonitor motorMonitor;

};
