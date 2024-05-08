/// PINS USED
/// 01 => INA219 vcc pin ( 3.3V )
/// 03 => INA219 SDA pin 
/// 05 => INA219 SCL pin
/// 02 => motor driver power ( 5V )
/// 11 => motor pin CW ( yellow )
/// 13 => motor pin CCW ( green )
/// 

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
#include "MotorController.hpp"
//#include "MotorMonitor.hpp"


//  CORE    ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
enum mCmd {
    none = 0,
    rotateCW,
    rotateCCW,
    getV,
    getI,
    getP,
    quit
};

constexpr int PORT = 12345;
constexpr int BUFFER_SIZE = 1024;

std::queue<std::string> messageQueue; // Queue to store received messages
std::queue<std::tuple<mCmd, float, float>> commandQueue;
std::mutex messageMutex, processingMutex, executeMutex; // Mutex to synchronize access to the message queue

//  DEBUG   //
bool serverPrint = false;
bool inaPrint = true;

////////////////////////////////////////////////////////////////////////




//  CONNECTION  ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


int serverLoop() {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE] = { 0 };

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        if (serverPrint) std::cerr << "SERVER:\tSocket creation failed\n";
        return 1;
    }
    if (serverPrint) std::cerr << "SERVER:\tSocket created\n";

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.0.100");
    serverAddr.sin_port = htons(PORT);
    
    // Get the server IP address
    char serverIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddr.sin_addr), serverIP, INET_ADDRSTRLEN);

    // Bind socket to address
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        if (serverPrint) std::cerr << "SERVER:\tBind failed\n";
        return 1;
    }
    if (serverPrint) std::cerr << "SERVER:\tServer socket binded successfully.\n";
    

    bool running = true;
    bool clientConnected = false;

    while(running) {
        // Listen for incoming connections
        if (listen(serverSocket, 5) < 0) {
            if (serverPrint) std::cerr << "SERVER:\tListen failed\n";
            return 1;
        }

        if (serverPrint) std::cout << "SERVER:\tServer listening for client connection on " << serverIP << ":" << PORT << std::endl;

        // Accept incoming connection
        if ((newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize)) < 0) {
            if (serverPrint) std::cerr << "SERVER:\tAccept failed\n";
            return 1;
        }
        // Get the client IP address
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        if (serverPrint) std::cerr << "SERVER:\tClient connected from " << clientIP << '\n';



        // Receive data from client
        
        ssize_t bytesRead;
        while ((bytesRead = recv(newSocket, buffer, BUFFER_SIZE, 0)) > 0) {
            if (serverPrint) std::cout << "SERVER:\tReceived from client: " << buffer << std::endl;
            std::lock_guard<std::mutex> lock(messageMutex);
            messageQueue.push(buffer);
            memset(buffer, 0, BUFFER_SIZE); // Clear buffer
        }

        if (bytesRead == 0) {
            if (serverPrint) std::cout << "SERVER:\tClient disconnected\n";
        }
        else if (bytesRead == -1) {
            if (serverPrint) std::cerr << "SERVER:\tReceive failed\n";
            return 1;
        }

        close(newSocket); // Close connection
    }
    close(serverSocket); // Close server socket
    return 0;
}
////////////////////////////////////////////////////////////////////////

//  MESSAGE PROCESSING  ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

std::string enumToString(mCmd value) {
    switch(value) {
        case mCmd::none:
            return "None";
        case mCmd::rotateCW:
            return "Rotate CW";
        case mCmd::rotateCCW:
            return "Rotate CCW";
        case mCmd::getV:
            return "Get Voltage";
        case mCmd::getI:
            return "Get Current";
        case mCmd::getP:
            return "Get Power";
        case mCmd::quit:
            return "Quit";
        default:
            return "Unknown";
    }
}
std::tuple<mCmd, float, float> parseCommand(std::string message) {
    if(message == "") {
        if (serverPrint) std::cout << "PARSER:\tRecieved empty command.\n";
        return std::tuple<mCmd, float, float>(mCmd::none, 0, 0);
    }
    
    std::istringstream iss(message);
    std::vector<std::string> tokens;
    
    std::string token;
    while (std::getline(iss, token, ' ')) {
        tokens.push_back(token);
    }
        
    std::string commandRaw = tokens[0];
    
    float speed = 0;
    float duration = 0.0;
    mCmd cmd = mCmd::none;
    
    if(tokens.size() > 1) {
        try {
            speed = std::stof(tokens[1]);
            duration = std::stof(tokens[2]);
            }
        catch(const std::exception& e) {
            if (serverPrint) std::cout << "PARSER:\tInvalid argument value.\n";
        }
    }
    
    if(commandRaw == "RCW") 
        cmd = mCmd::rotateCW;
    else if(commandRaw == "RCCW") 
        cmd = mCmd::rotateCCW;
    else if(commandRaw == "V")
        cmd = mCmd::getV;
    else if(commandRaw == "I")
        cmd = mCmd::getI;
    else if(commandRaw == "P")
        cmd = mCmd::getP;
    else if(commandRaw == "quit") 
        cmd = mCmd::quit;
            
    std::tuple<mCmd, float, float> ret(cmd, speed, duration);
    if (serverPrint) std::cout << "PARSER:\tParsed command [" << enumToString(std::get<0>(ret)) << "] with speed [" << std::get<1>(ret) << "] for [" << std::get<2>(ret) << "]ms.\n";
        
                        
    return ret;
}

void commandProcessingLoop() {
    while (true) {
        while(!messageQueue.empty()) {
            std::lock_guard<std::mutex> lock(processingMutex);
            std::string message = messageQueue.front();
            if (serverPrint) std::cerr << "CMDPRO:\t" << message << '\n';
            
            std::tuple<mCmd, float, float> cmd = parseCommand(message);
            commandQueue.push(cmd);
            
            messageQueue.pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void commandExecutionLoop() {
    while (true) {
        while(!commandQueue.empty()) {
            std::lock_guard<std::mutex> lock(executeMutex);
            std::tuple<mCmd, float, float> cmdValPair = commandQueue.front();
            
            mCmd cmd = std::get<0>(cmdValPair);
            int speed = std::get<1>(cmdValPair);
            float duration = std::get<2>(cmdValPair);
            if (serverPrint) std::cerr << "CMDEXE:\tExecuting " << enumToString(cmd) << '\n';
            
            switch(cmd) {
                case 1:
                    turnCW(speed, duration);
                    break;
                case 2:
                    turnCCW(speed, duration);
                    break;
                default:
                    break;
            }
            if (serverPrint) std::cerr << "CMDEXE:\tCompleted " << enumToString(cmd) << '\n';
            
            commandQueue.pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
////////////////////////////////////////////////////////////////////////


void startProgram() {
    //  set the GPIO pins
    setupGPIO();
    
    ////////////////////////////////////////////////////////////////////
    
    // set up motor monitoring module
    setupMonitor(100);
    
    //  start command processing loop
    std::thread commandProcessingThread(commandProcessingLoop);
    commandProcessingThread.detach();
    
    //  start command execution loop
    std::thread commandExecutionThread(commandExecutionLoop);
    commandExecutionThread.detach();
    
    
    //  start server listening loop
    std::thread serverThread(serverLoop);
    serverThread.detach();
    
    //  get i2c info
    while(true) {
        float V;
        float I;
        for(int i = 0; i < 100; i++){
            if (inaPrint) {
                V = getLoadVoltage();
                I = getLoadCurrent();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        //std::cerr << "Vlotage : " << V << "V;\tCurrent: " << I << "\n";
    }
}

void testMotor() {
    setupGPIO();
    
    int testEntries = 30;
    int cal = 2800;
    float idleV[testEntries] = {0.0f};
    float idleI[testEntries] = {0.0f};
    float cwV[testEntries] = {0.0f};
    float cwI[testEntries] = {0.0f};
    float ccwV[testEntries] = {0.0f};
    float ccwI[testEntries] = {0.0f};
    
    
    setupMonitor(1585);
    
    std::ofstream outFile("motorValues1585_5V.txt");
    
    if(!outFile.is_open()) {
        std::cerr<< "error opening file"<<std::endl;
        return;
    }
    
    float V, I;
    
    for (int i = 0; i < testEntries; i++) {
        std::cerr << "Iteration: " << i << "\n";
        outFile << "Iteration: " << i << "\n";
        
        float refV;
        float refI;
        
        for(int j = 0; j < 1000; j++){
            getLoadVoltage();
            getLoadCurrent();
        }
        refV = getAvgVoltage();
        refI = getAvgCurrent();
        std::cerr << "No load:\tVoltage: " << refV << " V\tCurrent: " << refI << " mA\n";
        outFile << "No load:\tVoltage: " <<  refV << " V\tCurrent: " << refI << " mA\n";
        idleV[i] = refV;
        idleI[i] = refI;
        
        
        turnCW(12, 5000);
        V = getAvgVoltage();
        I = getAvgCurrent();
        cwV[i] = V;
        cwI[i] = I;
        
        std::cerr << "CW:\t\tVoltage: " << V << " V\tCurrent: " << I << " mA\n";
        outFile << "CW:\t\tVoltage: " << V << " V\tCurrent: " << I << " mA\n";
        
        for(int j = 0; j < 1000; j++){
            getLoadVoltage();
            getLoadCurrent();
        }
        
        
        turnCCW(12, 5000);
        V = getAvgVoltage();
        I = getAvgCurrent();
        
        std::cerr << "CCW:\t\tVoltage: " << V << " V\tCurrent: " << I << " mA\n\n";
        outFile << "CCW:\t\tVoltage: " << V << " V\tCurrent: " << I << " mA\n\n";
        
        ccwV[i] = V;
        ccwI[i] = I;
        
    }
    outFile << "\n\n///////////////////////////////////////////////////////\n\nAVERAGES\n\n";
    
    float sumVcw = 0.0f, sumVccw = 0.0f, sumIcw = 0.0f, sumIccw = 0.0f, sumVidle = 0.0f, sumIidle = 0.0f;
    for(int i = 0; i < testEntries; i++) {
        sumVcw += cwV[i];
        sumVccw += ccwV[i];
        sumIcw += cwI[i];
        sumIccw += ccwI[i];
        sumVidle += idleV[i];
        sumIidle += idleI[i];
    }
    outFile << "IDLE:\tVoltage: " << sumVidle / testEntries << " V\tCurrent: " << sumIidle / testEntries << " mA\n";
    outFile << "CW:\tVoltage: " << sumVcw / testEntries << " V\tCurrent: " << sumIcw / testEntries << " mA\n";
    outFile << "CCW:\tVoltage: " << sumVccw / testEntries << " V\tCurrent: " << sumIccw / testEntries << " mA\n\n";
    
    outFile << "\n\n///////////////////////////////////////////////////////\n\nSort By Best CW Voltage\n";

    int indexes[testEntries];
    float tempV;
    int tempI;
    
    for (int i = 0; i < testEntries; i++) indexes[i] = i;
    
    for (int i = 0; i < testEntries; i++) {
        
        for (int j = i + 1; j < testEntries; j++) {
            if(cwV[j] < cwV[i]) {
                tempV = cwV[j];                
                cwV[j] = cwV[i];
                cwV[i] = tempV;
                
                tempV = ccwV[j];
                ccwV[j] = ccwV[i];
                ccwV[i] = tempV;
                
                tempV = cwI[j];
                cwI[j] = cwI[i];
                cwI[i] = tempV;
                
                tempV = ccwI[j];
                ccwI[j] = ccwI[i];
                ccwI[i] = tempV;
                
                tempI = indexes[j];
                indexes[j] = indexes[i];
                indexes[i] = tempI;
            }
        }
    }
    
    for (int i = 0; i < testEntries; i++) {
        outFile << "calibration value: " << indexes[i] << "\n";
        outFile << "CW\t\tVoltage: " << cwV[i] << " V\tCurrent: " << cwI[i] << " mA\n";
        outFile << "CCW\t\tVoltage: " << ccwV[i] << " V\tCurrent: " << ccwI[i] << " mA\n\n";
    }
    
    outFile << "\n\n///////////////////////////////////////////////////////\n\nSort By Best Voltage Difference\n";
    
    
    //for (int i = 0; i < testEntries; i++) indexes[i] = i;
    
    for (int i = 0; i < testEntries; i++) {
        for (int j = i + 1; j < testEntries; j++) {
            if((cwV[j] + ccwV[j]) > (cwV[i] + ccwV[i])) {
                tempV = cwV[j];                
                cwV[j] = cwV[i];
                cwV[i] = tempV;
                
                tempV = ccwV[j];
                ccwV[j] = ccwV[i];
                ccwV[i] = tempV;
                
                tempV = cwI[j];
                cwI[j] = cwI[i];
                cwI[i] = tempV;
                
                tempV = ccwI[j];
                ccwI[j] = ccwI[i];
                ccwI[i] = tempV;
                
                tempI = indexes[j];
                indexes[j] = indexes[i];
                indexes[i] = tempI;
            }
        }
    }
    
    for (int i = 0; i < testEntries; i++) {
        outFile << "calibration value: " << indexes[i] << "\n";
        outFile << "CW\t\tVoltage: " << cwV[i] << " V\tCurrent: " << cwI[i] << " mA\n";
        outFile << "CCW\t\tVoltage: " << ccwV[i] << " V\tCurrent: " << ccwI[i] << " mA\n\n";
    }
    
    outFile << "\n\n///////////////////////////////////////////////////////\n\nSort By Best CW Current\n";
    
    
    //for (int i = 0; i < testEntries; i++) indexes[i] = i;
    
    for (int i = 0; i < testEntries; i++) {
        for (int j = i + 1; j < testEntries; j++) {
            if(cwI[j] < cwI[i]) {
                tempV = cwV[j];                
                cwV[j] = cwV[i];
                cwV[i] = tempV;
                
                tempV = ccwV[j];
                ccwV[j] = ccwV[i];
                ccwV[i] = tempV;
                
                tempV = cwI[j];
                cwI[j] = cwI[i];
                cwI[i] = tempV;
                
                tempV = ccwI[j];
                ccwI[j] = ccwI[i];
                ccwI[i] = tempV;
                
                tempI = indexes[j];
                indexes[j] = indexes[i];
                indexes[i] = tempI;
            }
        }
    }
    
    for (int i = 0; i < testEntries; i++) {
        outFile << "calibration value: " << indexes[i] << "\n";
        outFile << "CW\t\tVoltage: " << cwV[i] << " V\tCurrent: " << cwI[i] << " mA\n";
        outFile << "CCW\t\tVoltage: " << ccwV[i] << " V\tCurrent: " << ccwI[i] << " mA\n\n";
    }
    
    outFile << "\n\n///////////////////////////////////////////////////////\n\nSort By Best CCW Current\n";
    
    
    //for (int i = 0; i < testEntries; i++) indexes[i] = i;
    
    for (int i = 0; i < testEntries; i++) {
        for (int j = i + 1; j < testEntries; j++) {
            if(ccwI[j] > ccwI[i]) {
                tempV = cwV[j];                
                cwV[j] = cwV[i];
                cwV[i] = tempV;
                
                tempV = ccwV[j];
                ccwV[j] = ccwV[i];
                ccwV[i] = tempV;
                
                tempV = cwI[j];
                cwI[j] = cwI[i];
                cwI[i] = tempV;
                
                tempV = ccwI[j];
                ccwI[j] = ccwI[i];
                ccwI[i] = tempV;
                
                tempI = indexes[j];
                indexes[j] = indexes[i];
                indexes[i] = tempI;
            }
        }
    }
    
    for (int i = 0; i < testEntries; i++) {
        outFile << "calibration value: " << indexes[i] << "\n";
        outFile << "CW\t\tVoltage: " << cwV[i] << " V\tCurrent: " << cwI[i] << " mA\n";
        outFile << "CCW\t\tVoltage: " << ccwV[i] << " V\tCurrent: " << ccwI[i] << " mA\n\n";
    }
    
    outFile.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    
}

void testMotor2() {
    setupGPIO();
    setupMonitor(1585);
    
    turnCCW(12, 2000);
    
    printSortedVoltage();
    printSortedCurrent();
    
    std::cerr << "AVG V: " << getAvgVoltage() << "\tAVG I: " << getAvgCurrent() << std::endl;
    
}

void testMotor3() {
    setupGPIO();
    //setupMonitor(1581);
    
    int samples = 4096;
    int indexes[samples];
    float avgCurr[samples] = {0.0f};
    float avgVolt[samples] = {0.0f};
    for(int i = 0; i < samples; i++) indexes[i] = i + 1;
    
    int pin = motorCCW;
    
    std::ofstream outFile("Resistor_22-3ohm_tests.txt");
    
    if(!outFile.is_open()) {
        std::cerr<< "error opening file"<<std::endl;
        return;
    }
    
    int startCalValue = 3000;
    int lastSuccesfullSetup = startCalValue;
    bool i2cSetupFailed = false;
    for(int i = startCalValue; i <= samples; i+=1){
        int setup = 0;
        int m;
        do {
            m = setupMonitor(i);
        }while(m != 0 && ++setup < 10);
        if(setup == 10) {
            i2cSetupFailed = true;
            if(m == 1){
                outFile << "Failed to open I2C device. CAL: " << i << "\n\n";
                std::cerr << "Failed to open I2C device. CAL: " << i << "\n\n";
            }
            else if(m == 2) {
                outFile << "Failed to set I2C slave address. CAL: " << i << "\n\n";
                std::cerr << "Failed to set I2C slave address. CAL: " << i << "\n\n";
            }
            else {
                outFile << "Unknown error. CAL: " << i << "\n";
                std::cerr << "Unknown error. CAL: " << i << "\n";
            }
            outFile << "Retry last succesfull value: " << lastSuccesfullSetup << "\n";
            std::cerr << "Retry last succesfull value: " << lastSuccesfullSetup << "\n";
        }
        if (!i2cSetupFailed) lastSuccesfullSetup = i;
        i2cSetupFailed = false;
        digitalWrite(pin, HIGH);
        
        float Isum = 0.0f;
        float Vsum = 0.0f;
        
        int testLoops = 100;
        for(int j = 0; j < testLoops; j++) {
            Isum += getLoadCurrent();
            Vsum += getLoadVoltage();
        }
        digitalWrite(pin, LOW);
        
        avgCurr[i-1] = Isum / testLoops;
        avgVolt[i-1] = Vsum / testLoops;
        
        std::cerr << "CAL: " << i << "\nCURR: " << avgCurr[i-1] << "\nVOLT: " << avgVolt[i-1] << "\n\n";
        outFile << "CAL: " << i << "\nCURR: " << avgCurr[i-1] << "\nVOLT: " << avgVolt[i-1] << "\n\n";
        delay(4000);
    }
    
    float tempF;
    int tempI;
    
    // sort by current
    for(int i = 0; i < samples; i++) {
        for(int j = i; j < samples; j++) {
            if(avgCurr[j] > avgCurr[i]) {
                tempF = avgCurr[j];
                avgCurr[j] = avgCurr[i];
                avgCurr[i] = tempF;
                
                tempF = avgVolt[j];
                avgVolt[j] = avgVolt[i];
                avgVolt[i] = tempF;
                
                tempI = indexes[j];
                indexes[j] = indexes[i];
                indexes[i] = tempI;
            }
        }
    }
    outFile << "////////////////////////////////////////////////////////\nSORT BY CURRENT:\n\n";
    for(int i = 1; i <= samples; i++)
        outFile << "#" << i <<std::endl
        << "CAL: " << indexes[i] 
        << "\nCURR: " << avgCurr[i-1] 
        << "\nVOLT: " << avgVolt[i-1] << "\n\n";
    
    
    // sort by voltage
    for(int i = 0; i < samples; i++) {
        for(int j = i; j < samples; j++) {
            if(avgVolt[j] > avgVolt[i]) {
                tempF = avgCurr[j];
                avgCurr[j] = avgCurr[i];
                avgCurr[i] = tempF;
                
                tempF = avgVolt[j];
                avgVolt[j] = avgVolt[i];
                avgVolt[i] = tempF;
                
                tempI = indexes[j];
                indexes[j] = indexes[i];
                indexes[i] = tempI;
            }
        }
    }
    outFile << "////////////////////////////////////////////////////////\nSORT BY VOLTAGE:\n\n";
    for(int i = 1; i <= samples; i++)
        outFile << "#" << i <<std::endl
        << "CAL: " << indexes[i] 
        << "\nCURR: " << avgCurr[i-1] 
        << "\nVOLT: " << avgVolt[i-1] << "\n\n";
        
    outFile.close();
    /*
    float t = millis();
        digitalWrite(pin, HIGH);
    while (t + 1000 > millis()) {
        
        /*for(int i = 0; i < power; i++)
            delay(tick);
            
        digitalWrite(motorCCW, LOW);
        //std::cerr << "VOLT: " << getLoadVoltage() << " V\tCURR: " << getLoadCurrent() << " mA\n";
        std::cerr << "CURR: " << getLoadCurrent() << " mA\n";
        /*
        for(int i = 12; i < 12 -power; i++)
            delay(tick);
        
    }
    digitalWrite(pin, LOW);
    */
}

float absoluteValue(float num) {
    return (num >= 0) ? num : -num;
}

void testMotor4() {
    int calibrationValue = 1858;
    setupGPIO();
    setupMonitor(calibrationValue);
    int pin = motorCCW;
    int cooldownTime = 1000;
    
    
    std::ofstream outFile("Resistor_22-3ohm_1825.txt");
    
    if(!outFile.is_open()) {
        std::cerr<< "error opening file"<<std::endl;
        return;
    }
    
    
    int samples = 20;
    float averageVs[samples] = {0.0f};
    float averageIs[samples] = {0.0f};
    float maxDeviationsV[samples] = {0.0f};
    float maxDeviationsI[samples] = {0.0f};
    
    float sumAllV = 0.0f, sumAllI = 0.0f, sumAllDevV = 0.0f, sumAllDevI = 0.0f;
    
    for (int i = 0; i < samples; i++) {
        digitalWrite(pin, HIGH);
        int testSize = 2000;
        float readV[testSize] = {0.0f};
        float readI[testSize] = {0.0f};
        
        float maxDevV = 0.0f, maxDevI = 0.0f;
        float sumV = 0.0f, sumI = 0.0f;
        
        for(int j = 0; j < testSize; j++) {
            readV[j] = getLoadVoltage();
            readI[j] = getLoadCurrent();
            
            sumV += readV[j];
            sumI += readI[j];
        }
        digitalWrite(pin, LOW);
        
        averageVs[i] = sumV / testSize;
        averageIs[i] = sumI / testSize;
        
        for(int j = 0; j < testSize; j++) {
            if(absoluteValue(readV[j] - averageVs[i]) > maxDevV) maxDevV = absoluteValue(readV[j] - averageVs[i]);
            if(absoluteValue(readI[j] - averageIs[i]) > maxDevI) maxDevI = absoluteValue(readI[j] - averageIs[i]);
        }
        
        maxDeviationsV[i] = maxDevV;
        maxDeviationsI[i] = maxDevI;
        
        sumAllV += averageVs[i];
        sumAllI += averageIs[i];
        sumAllDevV += maxDevV;
        sumAllDevI += maxDevI;
        
        std::cerr << "#" << i << "\n==>\tAvg Voltage: " << averageVs[i] << "\tMax V deviation: " << maxDevV 
            << "\n==>\tAvg Current: " << averageIs[i] << "\tMaxI deviation: " << maxDevI << "\n\n";
        outFile << "#" << i << "\n==>\tAvg Voltage: " << averageVs[i] << "\tMax V deviation: " << maxDevV 
            << "\n==>\tAvg Current: " << averageIs[i] << "\tMaxI deviation: " << maxDevI << "\n\n";
        
        delay(cooldownTime);
    }
    
    float avgV = sumAllV / samples;
    float avgI = sumAllI / samples;
    float avgDevV = sumAllDevV / samples;
    float avgDevI = sumAllDevI / samples;
    
    std::cerr << "\n====================================================\n==\tCONCLUSION\t==\n"
        << "\n==>\tOverall average Voltage: " << avgV 
        << "\n==>\tOverall average Current: " << avgI
        << "\n==>\tOverall average Voltage Deviation: " << avgDevV
        << "\n==>\tOverall average Current Deviation: " << avgDevI 
        << std::endl;
    outFile << "\n====================================================\n==\tCONCLUSION\t==\n"
        << "\n==>\tOverall average Voltage: " << avgV 
        << "\n==>\tOverall average Current: " << avgI
        << "\n==>\tOverall average Voltage Deviation: " << avgDevV
        << "\n==>\tOverall average Current Deviation: " << avgDevI 
        << std::endl;
        
    outFile.close();
}


void testMotor5() {
    setupGPIO();
    setupMonitor(1858);
    
    float rpm = 0;
    
    std::thread speedMeasureThread(measureSpeed2, &rpm, 100);
    speedMeasureThread.detach();
    
    turnCCW(24, 2000);
    turnCW(24, 2000);
    //turnCCW(4, 2000);
    //turnCW(14, 2000);
    //turnCCW(22, 2000);
    //turnCW(2, 2000);
    stopMotor();
}


int main() {
    testMotor5();
    //startProgram();
}
