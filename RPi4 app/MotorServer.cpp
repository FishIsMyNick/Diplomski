#include "MotorServer.h"


//  HELPERS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

float absoluteValue(float num) {
    return (num >= 0) ? num : -num;
}

std::string to_string_with_precision(float value, int precision) {
    std::string str = std::to_string(value);
    size_t dot_pos = str.find('.');
    if(dot_pos != std::string::npos) {
        str = str.substr(0, dot_pos + precision + 1);
    }
    return str;
}

void toggleSpeedMeasure() {
    doSpeedMeasure = !doSpeedMeasure;
    if(doSpeedMeasure) {
        std::thread speedMeasureThread(measureSpeedLoop);
        speedMeasureThread.detach();
    }
    else {
        stopMeasuringSpeed();
    }
}

std::string get_string_from_bool(bool value) {
    return value ? "True" : "False";
}

/// Get string value from command enumerator
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
        case mCmd::togSpd:
            return "Toggle Speed Measurement";
        case mCmd::quit:
            return "Quit";
        default:
            return "Unknown";
    }
}

/// Parse the command recieved from client and return command structure
std::tuple<mCmd, float, float> parseCommand(std::string message) {
    if(message == "") {
        std::cout << "PARSER:\tRecieved empty command.\n";
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
            std::cout << "PARSER:\tInvalid argument value.\n";
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
    else if(commandRaw == "TS")
        cmd = mCmd::togSpd;
    else if(commandRaw == "quit") 
        cmd = mCmd::quit;
            
    std::tuple<mCmd, float, float> ret(cmd, speed, duration);
    std::cout << "PARSER:\tParsed command [" << enumToString(std::get<0>(ret)) << "] with speed [" << std::get<1>(ret) << "] for [" << std::get<2>(ret) << "]ms.\n";
        
                        
    return ret;
}

////////////////////////////////////////////////////////////////////////

//  CONNECTION  ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void sendResponse(const std::string &response) {
    responseQueue.push(response);
}
void sendSpeed(float speed) {
    speedQueue.push(to_string_with_precision(speed, 2));
}

void responseLoop() {
    while(true) {
        if(!responseQueue.empty()) {
            std::string response = responseQueue.front();
            
            if (send(clientSocket, response.c_str(), response.size(), 0) < 0) {
                std::cerr << "SERVER:\tFailed to send response to client\n";
            } else {
                std::cerr << "SERVER:\tResponse sent to client.\n";
            }
            
            responseQueue.pop();
        }
        if(!speedQueue.empty()) {
            std::string response = speedQueue.front();
            
            if (send(speedSocket, response.c_str(), response.size(), 0) < 0) {
                std::cerr << "SERVER:\tFailed to send speed to client\n";
            } else {
                //std::cerr << "SERVER:\tSpeed sent to client.\n";
            }
            
            speedQueue.pop();
        }
        delay(1);
    }
}

int serverLoop() {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr, speedAddr, clientAddr, clientSpdAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE] = { 0 };

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "SERVER:\tSocket creation failed\n";
        return 1;
    }
    std::cerr << "SERVER:\tSocket created\n";

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.0.100");
    serverAddr.sin_port = htons(recvPort);
        
    
    // Get the server IP address
    char serverIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddr.sin_addr), serverIP, INET_ADDRSTRLEN);

    // Bind socket to address
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "SERVER:\tBind srv failed\n";
        return 1;
    }
    std::cerr << "SERVER:\tServer socket binded successfully.\n";
    

    bool running = true;
    bool clientConnected = false;

    while(running) {
        // MESSAGES
        
        // Listen for incoming connections
        if (listen(serverSocket, 5) < 0) {
            std::cerr << "SERVER:\tListen failed\n";
            return 1;
        }

        std::cout << "SERVER:\tServer listening for client connection on " << serverIP << ":" << recvPort << std::endl;

        // Accept incoming connection
        if ((newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize)) < 0) {
            std::cerr << "SERVER:\tAccept failed\n";
            return 1;
        }
        
        // Get the client IP address
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        std::cerr << "SERVER:\tClient message connected from " << clientIP << '\n';
        
        clientSocket = newSocket;
        

        // Receive data from client
        ssize_t bytesRead;
        while ((bytesRead = recv(newSocket, buffer, BUFFER_SIZE, 0)) > 0) {
            std::cout << "SERVER:\tReceived from client: " << buffer << std::endl;
            std::lock_guard<std::mutex> lock(messageMutex);
            messageQueue.push(buffer);
            messageMutex.unlock();
            
            std::tuple<mCmd, float, float> cmd = parseCommand(buffer);
            std::string response = "<<SERVER>>\tCommand '";
            response.append(buffer).append("'\trecieved.");
            sendResponse(response);
                        
            memset(buffer, 0, BUFFER_SIZE); // Clear buffer
        }

        if (bytesRead == 0) {
            std::cout << "SERVER:\tClient disconnected\n";
            clientSocket = -1;
        }
        else if (bytesRead == -1) {
            std::cerr << "SERVER:\tReceive failed\n";
            clientSocket = -2;
            return 1;
        }

        close(newSocket); // Close connection
    }
    close(serverSocket); // Close server socket
    return 0;
}

int speedLoop() {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr, speedAddr, clientAddr, clientSpdAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE] = { 0 };

    // Create socket
    if ((speedSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "SERVER:\tSocket creation failed\n";
        return 1;
    }
    std::cerr << "SERVER:\tSocket created\n";

    // Set server address    
    speedAddr.sin_family = AF_INET;
    speedAddr.sin_addr.s_addr = inet_addr("192.168.0.100");
    speedAddr.sin_port = htons(speedPort);
    
    
    // Get the server IP address
    char serverSpeedIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(speedAddr.sin_addr), serverSpeedIP, INET_ADDRSTRLEN);

    // Bind socket to address
    if (bind(speedSocket, (struct sockaddr*)&speedAddr, sizeof(speedAddr)) < 0) {
        std::cerr << "SERVER:\tBind spd failed\n";
        return 1;
    }
    std::cerr << "SERVER:\tServer socket binded successfully.\n";
    

    bool running = true;
    bool clientConnected = false;

    while(running) {
        // SPEED
        
        // Listen for incoming connections
        if (listen(speedSocket, 5) < 0) {
            std::cerr << "SERVER:\tListen failed\n";
            return 1;
        }

        std::cout << "SERVER:\tServer listening for client connection on " << serverSpeedIP << ":" << recvPort << std::endl;

        // Accept incoming connection
        if ((newSocket = accept(speedSocket, (struct sockaddr*)&clientSpdAddr, &addrSize)) < 0) {
            std::cerr << "SERVER:\tAccept failed\n";
            return 1;
        }
        
        // Get the client IP address
        char clientSpdIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientSpdAddr.sin_addr), clientSpdIP, INET_ADDRSTRLEN);
        std::cerr << "SERVER:\tClient speed connected from " << clientSpdIP << '\n';
        
        speedSocket = newSocket;

        // Receive data from client
        ssize_t bytesRead;
        while ((bytesRead = recv(newSocket, buffer, BUFFER_SIZE, 0)) > 0) {
            std::cout << "SERVER:\tReceived from client: " << buffer << std::endl;
            std::lock_guard<std::mutex> lock(messageMutex);
            messageQueue.push(buffer);
            messageMutex.unlock();
            
            std::string response = "<<SERVER>>\tCommand '";
            response.append(buffer).append("'\trecieved.");
            sendResponse(response);
                        
            memset(buffer, 0, BUFFER_SIZE); // Clear buffer
        }

        if (bytesRead == 0) {
            std::cout << "SERVER:\tClient disconnected\n";
            speedSocket = -1;
        }
        else if (bytesRead == -1) {
            std::cerr << "SERVER:\tReceive failed\n";
            speedSocket = -2;
            return 1;
        }

        close(newSocket); // Close connection
    }
    close(speedSocket); // Close server socket
    return 0;
}

void quitClient() {
    close(clientSocket);
    close(speedSocket);
    doSpeedMeasure = false;
    setMeasure(false);
}
////////////////////////////////////////////////////////////////////////

//  MESSAGE PROCESSING  ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

/// Parse the commands recieved from client and push them to the command queue
void commandProcessingLoop() {
    while (true) {
        while(!messageQueue.empty()) {
            std::lock_guard<std::mutex> lock(processingMutex);
            std::string message = messageQueue.front();
            std::cerr << "CMDPRO:\t" << message << '\n';
            
            std::tuple<mCmd, float, float> cmd = parseCommand(message);
            commandQueue.push(cmd);
            processingMutex.unlock();
            messageQueue.pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/// Takes commands from the command queue and executes them
void commandExecutionLoop() {
    while (true) {
        while(!commandQueue.empty()) {
            std::lock_guard<std::mutex> lock(executeMutex);
            std::tuple<mCmd, float, float> cmdValPair = commandQueue.front();
            
            mCmd cmd = std::get<0>(cmdValPair);
            float speed = std::get<1>(cmdValPair);
            float duration = std::get<2>(cmdValPair);
            
            std::cerr << "CMDEXE:\tExecuting " << enumToString(cmd) << '\n';
            
            if(cmd != mCmd::togSpd) {
                std::string exeString = "<<SERVER>>\tExecuting '";
                exeString.append(enumToString(cmd)).append(" ")
                .append(to_string_with_precision(speed, 2)).append("V ")
                .append(to_string_with_precision(duration, -1)).append("ms'...");
                
                sendResponse(exeString);
            }
            
            switch(cmd) {
                case mCmd::rotateCW:
                    turnCW(speed, duration);
                    break;
                case mCmd::rotateCCW:
                    turnCCW(speed, duration);
                    break;
                case mCmd::togSpd:
                    toggleSpeedMeasure();
                    break;
                case mCmd::quit:
                    quitClient();
                    break;
                default:
                    break;
            }
            std::cerr << "CMDEXE:\tCompleted " << enumToString(cmd) << '\n';
            if(cmd != mCmd::togSpd) {
                std::string compString = "<<SERVER>>\tCompleted '";
                compString.append(enumToString(cmd)).append(" ")
                .append(to_string_with_precision(speed, 2)).append("V ")
                .append(to_string_with_precision(duration, -1)).append("ms'");
                sendResponse(compString);
            }
            else {
                std::string exeString = "<<SERVER>>\tSpeed measurement set to: '";
                exeString.append(get_string_from_bool(doSpeedMeasure)).append("'.");
                
                sendResponse(exeString);
            }
            
            commandQueue.pop();
            executeMutex.unlock();
        }
        stopMotor();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
////////////////////////////////////////////////////////////////////////

//  SPEED MEASUREMENT   ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void measureSpeedLoop() {
    int pollRate = 50;
        
    std::thread speedMeasureThread(measureSpeed, &rpm, pollRate);
    speedMeasureThread.detach();
    
    while(doSpeedMeasure) {
        std::lock_guard<std::mutex> lock(messageMutex);
        sendSpeed(rpm);
        messageMutex.unlock();
        
        delay(100);
    }
    std::cerr<<"LOOP MEASURE STOP\n";
}

////////////////////////////////////////////////////////////////////////

void startProgram() {
    //  set the GPIO pins
    //setupGPIO();
    
    //////////////////////////////////////////////////////////////////// set up controller module
    setupController();
    
    // set up motor monitoring module
    setupMonitor();
    
    //  start command processing loop
    std::thread commandProcessingThread(commandProcessingLoop);
    commandProcessingThread.detach();
    
    //  start command execution loop
    std::thread commandExecutionThread(commandExecutionLoop);
    commandExecutionThread.detach();
    
    // start response thread
    std::thread responseThread(responseLoop);
    responseThread.detach();
    
    //  start speed listening loop
    std::thread speedListenThread(speedLoop);
    speedListenThread.detach();
    
    //  start server listening loop
    std::thread serverThread(serverLoop);
    serverThread.join();
}

int main() {
    startProgram();
}
