#include <wiringPi.h>
#include <cmath>
#include "MotorMonitor.hpp"

int motorCW = 17;
int motorCCW = 27;
int detectMagnet = 5;
int testPin = 26;

const int maxSpeed = 24;  // equivalent 12V
int lastPowerSet;
int lastDirection;

int acceleration = 1; // of 10


int frequency = 1000; // Hz
float tick = 1.0/frequency; // s

bool powerSignal[maxSpeed];

bool allowDebugSpeed;

//  SETUP   ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


void risingEdgeCallback() {
    detectMagnetRE = true;
}

int setupGPIO(){
	// Initialize WiringPi library
    if (wiringPiSetupGpio() == -1) {
        // Initialization failed
        return 1;
    }
    
    // Set the motor pins as an output
    pinMode(motorCW, OUTPUT);
    pinMode(motorCCW, OUTPUT);
    pinMode(detectMagnet, INPUT);
    pinMode(testPin, OUTPUT);
    
    wiringPiISR(detectMagnet, INT_EDGE_RISING, &risingEdgeCallback);
    
    // Initialise power bits
    for(int i = 0; i < 12; i++)
        powerSignal[i] = false;
        
    lastPowerSet = 0;
    lastDirection = motorCW;
    
    allowDebugSpeed = true;

    return 0;
}

////////////////////////////////////////////////////////////////////////


//  MOTOR CONTROL   ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


/// Generates an evenly distributed power signal to supply the desired 
/// output motor speed.
/// speed => desired output motor speed, fixed to [0.0 - 12.0]
void generatePowerSignal(int speed) {
    //if(speed == lastPowerSet) return;
    
    for(int p = 0; p < maxSpeed; p++) powerSignal[p] = false;
    
    if(speed <= 0) return;
    if(speed > maxSpeed) speed = maxSpeed;
    
    float s = 0;
    float step = maxSpeed / (speed * 1.0f);
    int i;
    while(s < maxSpeed) {
        i = std::round(s);
        powerSignal[i] = true;
        s += step;
    }
}

void powerMotorPWM(int direction, int speed) {
    digitalWrite(direction, HIGH);
    delay(tick * speed);
    digitalWrite(direction, LOW);
    delay(tick * (maxSpeed - speed));
}

void powerMotorSignal(int direction, int speed) {
    generatePowerSignal(speed);
    for(int i = 0; i < maxSpeed; i++){
        if(powerSignal[i]) {
            digitalWrite(direction, HIGH);
        }
        else { 
            digitalWrite(direction, LOW);
        }
        delay(tick);
    }
}

void accelerateMotorSignal(int direction, int speed) {
    unsigned long stepStartTime = millis();
    while(stepStartTime + acceleration * 10 > millis()) {
        powerMotorSignal(direction, speed);
    }
}
void accelerateMotorPWM(int direction, int speed) {
    unsigned long stepStartTime = millis();
    while(stepStartTime + acceleration * 100 > millis()) {
        powerMotorPWM(direction, speed);
    }
}
void accelerateMotor(int direction, int speed) {
    accelerateMotorSignal(direction, speed);
}

/// Stops the motor gradually. 
int stopMotor(int maxDuration = 2000) {
    int direction = lastDirection;
    int speed = lastPowerSet;
    if(speed == 0) return 0;
    unsigned long startTime = millis();
    
    while(millis() - startTime < maxDuration) {
        if(lastPowerSet > 0) 
            speed = --lastPowerSet;
        else break;
        
        if(allowDebugSpeed) std::cerr << "speed: " << speed << "\n";
        accelerateMotor(direction, speed);
    }
    
    digitalWrite(direction, LOW);
    
    return 0;
}


/// Turns the motor in the desired direction, at the set speed for the 
/// set amount of time.
/// direction   => should pass values: motorCW or motorCCW
/// speed       => desired motor output speed, fixed to [0.0 - 12.0]
/// mDuration   => desired duration in ms
int turnMotor(int direction, int desiredSpeed, float mDuration, bool quickChange = false) {
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
            
        if(allowDebugSpeed) std::cerr << "speed: " << speed << "\n";
        accelerateMotor(direction, speed);
    }
    lastDirection = direction;
    
    digitalWrite(direction, LOW);
    
    return 0;
}

/// Turns the motor clockwise
int turnCW(int speed, float mDuration, bool quickChange = false) {
    return turnMotor(motorCW, speed, mDuration, quickChange);
}
int turnCW(float speed, float mDuration, bool quickChange = false) {
    return turnCW(static_cast<int>(speed), mDuration, quickChange);
}

/// Turns the motor counter clockwise
int turnCCW(int speed, float mDuration, bool quickChange = false) {
    return turnMotor(motorCCW, speed, mDuration, quickChange);
}
int turnCCW(float speed, float mDuration, bool quickChange = false) {
    return turnCCW(static_cast<int>(speed), mDuration, quickChange);
}


////////////////////////////////////////////////////////////////////////
