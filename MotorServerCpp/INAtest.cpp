#include <iostream>
#include "MotorController.hpp"

float getNoLoadVoltage() {
	float Vsum = 0.0f;
	for (int i = 0; i < 1000; i++) {
		Vsum += getLoadVoltage();
	}
	return Vsum / 1000;
}

int main() {
	setupGPIO();
	setupMonitor();
	
	int testPin = 16;
	pinMode(testPin, OUTPUT);
	digitalWrite(testPin, LOW);
	
	float refV = getNoLoadVoltage();
	
	float V, I;
	uint16_t Ix;
	unsigned int m;
	float Vsum, Isum;
	int i;
	
	for(int j = 0; j < 4; j++) {
		m = millis();
		m += 2000;
		Vsum = 0.0f;
		Isum = 0.0f;
		Ix = 0x0;
		i = 0;
		while(m > millis()){
			V = getLoadVoltage();
			I = getLoadCurrent();
			Vsum += V;
			Isum += I;
			i++;
			//std::cout << "NO LOAD: Voltage: " << V << "\tCurrent: " << I << "\n";
			delay(10);
		}
		std::cout << "NO LOAD: Voltage: " << Vsum / i - refV << "\tCurrent: " << Isum / i << "\n";
		
		digitalWrite(testPin, HIGH);
		m = millis();
		m += 2000;
		Vsum = 0.0f;
		Isum = 0.0f;
		Ix = 0x0;
		i = 0;
		while(m > millis()){
			V = getLoadVoltage();
			I = getLoadCurrent();
			Vsum += V;
			Isum += I;
			i++;
			//std::cout << "LOAD: Voltage: " << V << "\tCurrent: " << I << "\n";
			delay(10);
		}
		std::cout << "LOAD: Voltage: " << Vsum / i - refV << "\tCurrent: " << Isum / i << "\n";
	
		digitalWrite(testPin, LOW);
	}
	return 0;
}
