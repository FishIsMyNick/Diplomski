#include <iostream>
#include "INA219.hpp"
//#include "Adafruit_INA219_1.cpp"

//	CORE	////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

//	BUFFERS	////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

int V_index;
const int V_size = 10000;
float V_buffer[V_size] = {0.0f};

int I_index;
const int I_size = 10000;
float I_buffer[I_size] = {0.0f};

bool detectMagnetRE = false;
bool allowDebugRPM = false;

////////////////////////////////////////////////////////////////////////


//	SETUP	////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

int setupMonitor(int cal) {
	return setupINA(cal);
}

////////////////////////////////////////////////////////////////////////

//	VALUE GETTERS	////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//	Current 

float getLoadVoltage() {
	float V = ina219.getBusVoltage();
	
	if(V_index >= V_size) V_index = 0;
	
	V_buffer[V_index++] = V;
	
	return V;
}

float getAvgVoltage() {
	float Vsum = 0;
	for (int i = 0; i < V_size; i++) {
		Vsum += V_buffer[i];
	}
	return Vsum / V_size;
}


float getLoadCurrent() {
	float I = ina219.getCurrent();
	
	if(I_index >= I_size) I_index = 0;
	
	I_buffer[I_index++] = I;
	
	return I;
}


float getAvgCurrent() {
	float Isum = 0;
	for (int i = 0; i < I_size; i++) {
		Isum += I_buffer[i];
	}
	return Isum / I_size;
}

void printSortedVoltage() {
	float sorted[V_size];
	for(int i = 0; i < V_size; i++) sorted[i] = V_buffer[i];
	
	float temp;
	int zeros = 0;
	for(int i = 0; i < V_size; i++) if(sorted[i] == 0.0f) zeros++;
	
	for(int i = 0; i < V_size; i++) {
		for(int j = i; j < V_size; j++) {
			if(sorted[j] > sorted[i]) {
				temp = sorted[i];
				sorted[i] = sorted[j];
				sorted[j] = temp;
			}
		}
	}
	std::cerr << "\nSORTED TOP 3 VOLTAGE\n";
	std::cerr << "zeros found: " << zeros << "\n\n";
	for(int i = 3; i >= 0; i--) {
		std::cerr << sorted[i] << std::endl;
	}
}
void printSortedCurrent() {
	float sorted[I_size];
	for(int i = 0; i < I_size; i++) sorted[i] = I_buffer[i];
	
	float temp;
	int zeros = 0;
	for(int i = 0; i < V_size; i++) if(sorted[i] == 0.0f) zeros++;
	
	for(int i = 0; i < I_size; i++) {
		for(int j = i; j < I_size; j++) {
			if(sorted[j] > sorted[i]) {
				temp = sorted[i];
				sorted[i] = sorted[j];
				sorted[j] = temp;
			}
		}
	}
	std::cerr << "\nSORTED TOP 3 CURRENT\n";
	std::cerr << "zeros found: " << zeros << "\n\n";
	for(int i = 3; i >= 0; i--) {
		std::cerr << sorted[i] << std::endl;
	}
}

// Speed

void measureSpeed(float* rpm, int pollRate) {
	while(true) {
		int hitCount = 0;
		int t = millis();
		
		while(t + pollRate > millis()) {
			if(detectMagnetRE) {
				hitCount++;
				detectMagnetRE = false;
				//std::cerr<< "hit\n";
			}
		}
		
		*rpm = (hitCount * (1000.0f / pollRate)) * 60;
		if (allowDebugRPM) std::cerr << "RPM: " << *rpm << std::endl;
	}
}

void measureSpeed2(float* rpm, int pollRate) {
	unsigned long lastHit = millis();
	unsigned long hitTime;
	unsigned long t = millis();
	while(true) {
		if(detectMagnetRE) {
			detectMagnetRE = false;
			hitTime = millis();
			*rpm = (1000.0f / (hitTime - lastHit)) * 60.0f;
			lastHit = hitTime;
		}
		if(t + pollRate < millis()) {
			if (allowDebugRPM) std::cerr << "RPM: " << *rpm << std::endl;
			t = millis();
		}
		delay(1);
	}
}

////////////////////////////////////////////////////////////////////////
