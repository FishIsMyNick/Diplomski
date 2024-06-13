#include "MotorServer.h"

int main() {
	if (wiringPiSetupGpio() == -1) {
        // Initialization failed
		std::cerr<<"GPIO init failed.\n";
		return 1;
    }
    
	MotorServer server;
	server.startServer();
	
	while(true) {
		delay(1000);
		std::cerr << "Server finished.\n";
	}
	
	return 0;
}
