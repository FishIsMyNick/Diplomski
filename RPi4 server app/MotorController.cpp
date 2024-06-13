#include "MotorController.h"

/// Sets the pins and control variables
int MotorController::setupController() {
    // Set the motor pins as an output
    pinMode(motorCW, OUTPUT);
    pinMode(motorCCW, OUTPUT);

    lastPowerSet = 0;
    lastDirection = motorCW;
    acceleration = 10;

    motorMonitor.setupMonitor();

    return 0;
}

////////////////////////////////////////////////////////////////////////


//  MOTOR CONTROL   ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

/// 'acceleration' setter
void MotorController::setAcceleration(int accel) {
	acceleration = accel;
}

/// PWM signal on/off time calculation 
void MotorController::calculatePWM(float speed) {
    int T = (tick * 1000000);
    float t = T / maxSpeed;
    
    pwmOnTime = speed > maxSpeed ? t * maxSpeed : t * speed;
    pwmOffTime = T - pwmOnTime;
}

/// PWM based powering logic
void MotorController::powerMotorPWM(int direction) {
    std::chrono::microseconds onTime(pwmOnTime);
    std::chrono::microseconds offTime(pwmOffTime);

    digitalWrite(direction, HIGH);
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::high_resolution_clock::now() - start < onTime) {}

    digitalWrite(direction, LOW);
    start = std::chrono::high_resolution_clock::now();
    while (std::chrono::high_resolution_clock::now() - start < offTime) {}
}

/// Gradually accelerates the motor.
void MotorController::accelerateMotor(int direction, float speed) {
    motorMonitor.setMotorInMotion(true);
    calculatePWM(speed);
    unsigned long stepStartTime = millis();
    while (stepStartTime + (11 - acceleration) * 2 > millis()) {
        powerMotorPWM(direction);
    }
}

/// Gradually stops the motor. 
int MotorController::stopMotor() {
	int maxDuration = 2000;
    int direction = lastDirection;
    int speed = lastPowerSet;
    if (speed == 0) return 0;
    unsigned long startTime = millis();

    while (millis() - startTime < maxDuration) {
        if (lastPowerSet > 0)
            speed = --lastPowerSet;
        else break;

        accelerateMotor(direction, speed);
    }

    digitalWrite(direction, LOW);
    motorMonitor.setMotorInMotion(false);

    return 0;
}


/// Turns the motor in the desired direction, at the set speed for the 
/// set amount of time.
/// direction   => should pass values: motorCW or motorCCW
/// speed       => desired motor output speed, fixed to [0.0 - 12.0]
/// mDuration   => desired duration in ms
int MotorController::turnMotor(int direction, float desiredSpeed, float mDuration) {
    unsigned long startTime = millis();
    if (direction != lastDirection) {
        stopMotor();
    }
    int speed = lastPowerSet;

    while (millis() - startTime < mDuration) {
        if (lastPowerSet > desiredSpeed)
            speed = --lastPowerSet;
        else if (lastPowerSet < desiredSpeed)
            speed = ++lastPowerSet;

        accelerateMotor(direction, speed);
    }
    lastDirection = direction;

    digitalWrite(direction, LOW);

    return 0;
}

/// Turns the motor clockwise
int MotorController::turnCW(float speed, float mDuration) {
    if (speed > 12.0f) speed = 12.0f;
    return turnMotor(motorCW, std::round(speed * 10), mDuration);
}


/// Turns the motor counter clockwise
int MotorController::turnCCW(float speed, float mDuration) {
    if (speed > 12.0f) speed = 12.0f;
    return turnMotor(motorCCW, std::round(speed * 10), mDuration);
}
