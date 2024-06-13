#include "MotorController.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono> 
#include <sstream>
#include <vector>
#include <string>


//  CORE    ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

constexpr int recvPort = 12345;
constexpr int speedPort = 12346;
constexpr int BUFFER_SIZE = 1024;


enum mCmd {
    none = 0,
    rotateCW,
    rotateCCW,
    SpdOn,
    SpdOff,
    Acc,
    quit
};

class MotorServer {
private:
    bool doSpeedMeasure;
    float rpm;
    bool clientMsgConnected;
	bool clientSpdConnected;
	
    std::queue<std::string> messageQueue; // Queue to store received messages
    std::queue<std::string> responseQueue; // Queue to store messages to send
    std::queue<std::string> speedQueue; // Queue to store speed to send
    std::queue<std::tuple<mCmd, float, float>> commandQueue; // Queue to store commands to execute
	
	// Mutex to synchronize access to the queues
    std::mutex messageMutex, speedMutex, responseMutex, commandMutex; 
    
    void pushMessage(std::string);
    void pushResponse(std::string);
    void pushSpeed(std::string);
    void pushCommand(std::tuple<mCmd, float, float>);
    
    std::string popMessage();
    std::string popResponse();
    std::string popSpeed();
    std::tuple<mCmd, float, float> popCommand();
    
public:
    std::string enumToString(mCmd);
    float absoluteValue(float);
    std::string to_string_with_precision(float, int);
    std::string get_string_from_bool(bool);
    std::tuple<mCmd, float, float> parseCommand(std::string);
    
    void stopMonitorSpeedMeasure();
    void startMonitorSpeedMeasure();

	int serverMsgSocket;
	int serverSpdSocket;
    int clientMsgSocket;
    int clientSpdSocket;

    void sendResponse(const std::string&);
    void sendSpeed(std::string speed);
    
    void responseLoop();
    int serverLoop();
    int speedLoop();
    void measureSpeedLoop();
    void startPushingSpeed();
    
    void quitClient();


    void commandProcessingLoop();
    void commandExecutionLoop();

    void startServer();

    MotorController motorController;
};
