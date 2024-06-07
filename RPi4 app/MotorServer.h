
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
#include <fstream>
#include "MotorController.cpp"


//  CORE    ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

constexpr int recvPort = 12345;
constexpr int speedPort = 12346;
constexpr int BUFFER_SIZE = 1024;

bool doSpeedMeasure;
float rpm;

enum mCmd {
    none = 0,
    rotateCW,
    rotateCCW,
    getV,
    getI,
    getP,
    togSpd,
    quit
};

std::queue<std::string> messageQueue; // Queue to store received messages
std::queue<std::string> responseQueue; // Queue to store messages to send
std::queue<std::string> speedQueue; // Queue to store speed to send
std::queue<std::tuple<mCmd, float, float>> commandQueue;
std::mutex messageMutex, speedMutex, processingMutex, executeMutex; // Mutex to synchronize access to the message queue

////////////////////////////////////////////////////////////////////////



std::string enumToString(mCmd);
float absoluteValue(float);
std::string to_string_with_precision(float, int);
std::tuple<mCmd, float, float> parseCommand(std::string);
void toggleSpeedMeasure();

int clientSocket;
int speedSocket;

void sendResponse(const std::string&);
void responseLoop();
int serverLoop();
int speedLoop();
void quitClient();
void measureSpeedLoop();


void commandProcessingLoop();
void commandExecutionLoop();

void StartProgram();
