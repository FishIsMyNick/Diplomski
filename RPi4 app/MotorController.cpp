#include <wiringPi.h>
#include <cmath>
#include <chrono>
#include <thread>
#include "MotorMonitor.cpp"

const int motorCW = 17;
const int motorCCW = 18;

const int maxSpeed = 120;  // equivalent 12V

int lastPowerSet;
int lastDirection;

const int frequency = 1000; // Hz
const float tick = 1.0/frequency; // s

int setupController(){
	// Initialize WiringPi library
    if (wiringPiSetupGpio() == -1) {
        // Initialization failed
        return 1;
    }
    
    // Set the motor pins as an output
    pinMode(motorCW, OUTPUT);
    pinMode(motorCCW, OUTPUT);    
            
    lastPowerSet = 0;
    lastDirection = motorCW;

    return 0;
}

////////////////////////////////////////////////////////////////////////


//  MOTOR CONTROL   ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void powerMotorPWM(int direction, float speed) {
    int T = maxSpeed;
    int on = speed > T ? T : speed;
    int off = T - on;
    std::chrono::microseconds onTime(on);
    std::chrono::microseconds offTime(off);
    
    
    digitalWrite(direction, HIGH);
    auto start = std::chrono::high_resolution_clock::now();
    while(std::chrono::high_resolution_clock::now() - start < onTime) {}
    
    digitalWrite(direction, LOW);
    start = std::chrono::high_resolution_clock::now();
    while(std::chrono::high_resolution_clock::now() - start < offTime) {}
}

void accelerateMotor(int direction, float speed, int acceleration = 10) {
    setMotorInMotion(true);
    unsigned long stepStartTime = millis();
    while(stepStartTime + (11 - acceleration) * 2 > millis()) {
        powerMotorPWM(direction, speed);
    }
}

/// Stops the motor gradually. 
int stopMotor(int maxDuration = 2000, int acceleration = 10) {
    int direction = lastDirection;
    int speed = lastPowerSet;
    if(speed == 0) return 0;
    unsigned long startTime = millis();
    
    while(millis() - startTime < maxDuration) {
        if(lastPowerSet > 0) 
            speed = --lastPowerSet;
        else break;
        
        accelerateMotor(direction, speed, acceleration);
    }
    
    digitalWrite(direction, LOW);
    setMotorInMotion(false);
    
    return 0;
}


/// Turns the motor in the desired direction, at the set speed for the 
/// set amount of time.
/// direction   => should pass values: motorCW or motorCCW
/// speed       => desired motor output speed, fixed to [0.0 - 12.0]
/// mDuration   => desired duration in ms
int turnMotor(int direction, int desiredSpeed, float mDuration, int acceleration = 10, bool quickChange = false) {
    unsigned long startTime = millis();
    if (!quickChange && direction != lastDirection) {
        stopMotor();
    }
    int speed = lastPowerSet;
    
    while(millis() - startTime < mDuration) {
        if(lastPowerSet > desiredSpeed) 
            speed = --lastPowerSet;
        else if (lastPowerSet < desiredSpeed) 
            speed = ++lastPowerSet;
        
        accelerateMotor(direction, speed, acceleration);
    }
    lastDirection = direction;
    
    digitalWrite(direction, LOW);
    
    return 0;
}

/// Turns the motor clockwise
int turnCW(float speed, float mDuration, int acceleration = 10, bool quickChange = false) {
    if (speed > 12.0f) speed = 12.0f;
    return turnMotor(motorCW, std::round(speed * 10), mDuration, acceleration, quickChange);
}


/// Turns the motor counter clockwise
int turnCCW(float speed, float mDuration, int acceleration = 10, bool quickChange = false) {
    if (speed > 12.0f) speed = 12.0f;
    return turnMotor(motorCCW, std::round(speed * 10), mDuration, acceleration, quickChange);
}


////////////////////////////////////////////////////////////////////////
