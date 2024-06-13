#include "MotorServer.h"


//  HELPERS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

///	Returns the absolute value of a float
float MotorServer::absoluteValue(float num) {
    return (num >= 0) ? num : -num;
}

///	Formats a float to a string with desired decimal precision
std::string MotorServer::to_string_with_precision(float value, int precision) {
    std::string str = std::to_string(value);
    size_t dot_pos = str.find('.');
    if (dot_pos != std::string::npos) {
        str = str.substr(0, dot_pos + precision + 1);
    }
    return str;
}

///	Starts the speed measurement threads
void MotorServer::startMonitorSpeedMeasure() {
	doSpeedMeasure = true;
	std::thread([this]() { this->motorController.motorMonitor.measureSpeed(&rpm); }).detach();
	
	std::thread([this]() { this-> measureSpeedLoop(); }).detach();
}

///	Stops the speed measurement threads
void MotorServer::stopMonitorSpeedMeasure() {
	doSpeedMeasure = false;
	motorController.motorMonitor.stopMeasuringSpeed();
}

///	Returns the string representation of a bool
std::string MotorServer::get_string_from_bool(bool value) {
    return value ? "True" : "False";
}

/// Get string value from command enumerator
std::string MotorServer::enumToString(mCmd value) {
    switch (value) {
    case mCmd::none:
        return "None";
    case mCmd::rotateCW:
        return "Rotate CW";
    case mCmd::rotateCCW:
        return "Rotate CCW";
    case mCmd::SpdOn:
		return "Turn On Speed Measurement";
    case mCmd::SpdOff:
		return "Turn Off Speed Measurement";
    case mCmd::Acc:
		return "Set Acceleration";
    case mCmd::quit:
        return "Quit";
    default:
        return "Unknown";
    }
}

/// Parse the command recieved from client and return command structure
std::tuple<mCmd, float, float> MotorServer::parseCommand(std::string message) {
    if (message == "") {
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

    if (tokens.size() > 1) {
        try {
            speed = std::stof(tokens[1]);
            duration = std::stof(tokens[2]);
        }
        catch (const std::exception& e) {
            std::cout << "PARSER:\tInvalid argument value.\n";
        }
    }

    if (commandRaw == "RCW")
        cmd = mCmd::rotateCW;
    else if (commandRaw == "RCCW")
        cmd = mCmd::rotateCCW;
    else if (commandRaw == "TSI")
        cmd = mCmd::SpdOn;
    else if (commandRaw == "TSO")
        cmd = mCmd::SpdOff;
    else if (commandRaw == "ACC")
        cmd = mCmd::Acc;
    else if (commandRaw == "quit")
        cmd = mCmd::quit;

    std::tuple<mCmd, float, float> ret(cmd, speed, duration);
    std::cout << "PARSER:\tParsed command [" << enumToString(std::get<0>(ret)) << "] with speed [" << std::get<1>(ret) << "] for [" << std::get<2>(ret) << "]ms.\n";


    return ret;
}


void MotorServer::pushMessage(std::string message) {
	std::unique_lock<std::mutex> lock(messageMutex);
    messageQueue.push(message);
    lock.unlock();
}
void MotorServer::pushResponse(std::string response) {
	std::unique_lock<std::mutex> lock(responseMutex);
    responseQueue.push(response);
    lock.unlock();
}
void MotorServer::pushSpeed(std::string speed) {
	std::unique_lock<std::mutex> lock(speedMutex);
    speedQueue.push(speed);
    lock.unlock();
}
void MotorServer::pushCommand(std::tuple<mCmd, float, float> command) {
	std::unique_lock<std::mutex> lock(commandMutex);
    commandQueue.push(command);
    lock.unlock();
}

std::string MotorServer::popMessage() {
	std::unique_lock<std::mutex> lock(messageMutex);
    std::string message = messageQueue.front();
    messageQueue.pop();
    lock.unlock();
    return message;
}
std::string MotorServer::popResponse() {
	std::unique_lock<std::mutex> lock(responseMutex);
    std::string message = responseQueue.front();
    responseQueue.pop();
    lock.unlock();
    return message;
}
std::string MotorServer::popSpeed() {
	std::unique_lock<std::mutex> lock(speedMutex);
    std::string speed = speedQueue.front();
    speedQueue.pop();
    lock.unlock();
    return speed;
}
std::tuple<mCmd, float, float> MotorServer::popCommand() {
	std::unique_lock<std::mutex> lock(commandMutex);
    std::tuple<mCmd, float, float> cmd = commandQueue.front();
    commandQueue.pop();
    lock.unlock();
    return cmd;
}

////////////////////////////////////////////////////////////////////////

//  CONNECTION  ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

///	Enqueues response messages
void MotorServer::sendResponse(const std::string& response) {
	pushResponse(response);
}

/// Enqueues speed messages
void MotorServer::sendSpeed(std::string speed) {
	pushSpeed(speed);
}

///	Dequeues response and speed messages and sends them via the appropreate channels
void MotorServer::responseLoop() {
    while (true) {
        if (!responseQueue.empty()) {
            std::string response = popResponse();

            if (send(clientMsgSocket, response.c_str(), response.size(), 0) < 0) {
                std::cerr << "SERVER:\tFailed to send response to client\n";
            }
            else {
                std::cerr << "SERVER:\tResponse sent to client.\n";
            }

        }
        if (!speedQueue.empty()) {
            std::string response = popSpeed();

            if (send(clientSpdSocket, response.c_str(), response.size(), 0) < 0) {
                std::cerr << "SERVER:\tFailed to send speed to client\n";
            }
            else {
                //std::cerr << "SERVER:\tSpeed sent to client.\n";
            }

        }
        delay(1);
    }
}

///	Opens and maintains the message exchange channel
int MotorServer::serverLoop() {
    int newSocket;
    struct sockaddr_in serverAddr, speedAddr, clientAddr, clientSpdAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE] = { 0 };

    // Create socket
    if ((serverMsgSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "SERVER:\tMessage socket creation failed\n";
        return 1;
    }
    std::cerr << "SERVER:\tMessage socket created\n";

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.0.100");
    serverAddr.sin_port = htons(recvPort);


    // Get the server IP address
    char serverIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddr.sin_addr), serverIP, INET_ADDRSTRLEN);

    // Bind socket to address
    if (bind(serverMsgSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "SERVER:\tBind message socket failed\n";
        return 1;
    }
    std::cerr << "SERVER:\tMessage socket binded successfully.\n";


    bool running = true;
    bool clientMsgConnected = false;

    while (running) {
        // MESSAGES

        // Listen for incoming connections
        if (listen(serverMsgSocket, 5) < 0) {
            std::cerr << "SERVER:\tMessage listen failed\n";
            return 1;
        }

        std::cout << "SERVER:\tMessage server listening for client connection on " << serverIP << ":" << recvPort << std::endl;

        // Accept incoming connection
        if ((newSocket = accept(serverMsgSocket, (struct sockaddr*)&clientAddr, &addrSize)) < 0) {
            std::cerr << "SERVER:\tMessage accept failed\n";
            return 1;
        }

        // Get the client IP address
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        std::cerr << "SERVER:\tMessage client connected from " << clientIP << '\n';

        clientMsgSocket = newSocket;
		clientMsgConnected = true;

        // Receive data from client
        ssize_t bytesRead;
        while ((bytesRead = recv(clientMsgSocket, buffer, BUFFER_SIZE, 0)) > 0 && clientMsgConnected) {
			if(buffer == "quit") {
				quitClient();
				break;
			}
            std::cout << "SERVER:\tReceived message from client: " << buffer << std::endl;
            pushMessage(buffer);

            std::tuple<mCmd, float, float> cmd = parseCommand(buffer);
            std::string response = "<<SERVER>>\tCommand '";
            response.append(buffer).append("'\trecieved.");
            sendResponse(response);

            memset(buffer, 0, BUFFER_SIZE); // Clear buffer
        }

		if (clientMsgConnected) {
			if (bytesRead == 0) {
				std::cout << "SERVER:\tClient message channel disconnected\n";
				clientMsgConnected = false;
				clientMsgSocket = -1;
			}
			else if (bytesRead == -1) {
				std::cerr << "SERVER:\tReceive message failed\n";
				clientMsgConnected = false;
				clientMsgSocket = -2;
				return 1;
			}
		}

        close(clientMsgSocket); // Close connection
        std::cerr << "SERVER:\tClient message socket closed.\n";
    }
    close(serverMsgSocket); // Close server socket
    return 0;
}

///	Opens and maintains the speed exchange channel
int MotorServer::speedLoop() {
    int newSocket;
    struct sockaddr_in serverAddr, speedAddr, clientAddr, clientSpdAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE] = { 0 };

    // Create socket
    if ((serverSpdSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "SERVER:\tSpeed socket creation failed\n";
        return 1;
    }
    std::cerr << "SERVER:\tSpeed socket created\n";

    // Set server address    
    speedAddr.sin_family = AF_INET;
    speedAddr.sin_addr.s_addr = inet_addr("192.168.0.100");
    speedAddr.sin_port = htons(speedPort);


    // Get the server IP address
    char serverSpeedIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(speedAddr.sin_addr), serverSpeedIP, INET_ADDRSTRLEN);

    // Bind socket to address
    if (bind(serverSpdSocket, (struct sockaddr*)&speedAddr, sizeof(speedAddr)) < 0) {
        std::cerr << "SERVER:\tBind speed socket failed\n";
        return 1;
    }
    std::cerr << "SERVER:\tSpeed server socket binded successfully.\n";


    bool running = true;
    bool clientSpdConnected = false;

    while (running) {
        // SPEED

        // Listen for incoming connections
        if (listen(serverSpdSocket, 5) < 0) {
            std::cerr << "SERVER:\tSpeed listen failed\n";
            return 1;
        }

        std::cout << "SERVER:\tSpeed server listening for client connection on " << serverSpeedIP << ":" << recvPort << std::endl;

        // Accept incoming connection
        if ((newSocket = accept(serverSpdSocket, (struct sockaddr*)&clientSpdAddr, &addrSize)) < 0) {
            std::cerr << "SERVER:\tAccept speed failed\n";
            return 1;
        }

        // Get the client IP address
        char clientSpdIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientSpdAddr.sin_addr), clientSpdIP, INET_ADDRSTRLEN);
        std::cerr << "SERVER:\tSpeed client connected from " << clientSpdIP << '\n';

        clientSpdSocket = newSocket;
        clientSpdConnected = true;

        // Receive data from client
        ssize_t bytesRead;
        while ((bytesRead = recv(clientSpdSocket, buffer, BUFFER_SIZE, 0)) > 0 && clientSpdConnected) {
            std::cout << "SERVER:\tReceived speed from client: " << buffer << std::endl;
            pushMessage(buffer);

            std::string response = "<<SERVER>>\tCommand '";
            response.append(buffer).append("'\trecieved.");
            sendResponse(response);

            memset(buffer, 0, BUFFER_SIZE); // Clear buffer
        }

		if(clientSpdConnected) {
			if (bytesRead == 0) {
				std::cout << "SERVER:\tClient speed channel disconnected.\n";
				clientSpdConnected = false;
				clientSpdSocket = -1;
			}
			else if (bytesRead == -1) {
				std::cerr << "SERVER:\tReceive speed failed.\n";
				clientSpdConnected = false;
				clientSpdSocket = -2;
			}
		}

        close(clientSpdSocket); // Close connection
            std::cerr << "SERVER:\tClient speed socket closed.\n";
    }
    close(serverSpdSocket); // Close server socket
            std::cerr << "SERVER:\tServer speed socket closed.\n";
    return 0;
}

/// Sets connection variables to false and stops measuring speed
void MotorServer::quitClient() {
	std::cerr << "Client quit.\n";
	clientMsgConnected = false;
	clientSpdConnected = false;
    stopMonitorSpeedMeasure();
}
////////////////////////////////////////////////////////////////////////

//  MESSAGE PROCESSING  ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

/// Parse the commands recieved from client and push them to the command queue
void MotorServer::commandProcessingLoop() {
    while (true) {
        while (!messageQueue.empty()) {
            std::string message = popMessage();
            
            std::cerr << "CMDPRO:\t" << message << '\n';

            std::tuple<mCmd, float, float> cmd = parseCommand(message);
            std::cerr << "CMDPRO:\tCommand parsed.\n";
            
            pushCommand(cmd);
            
            std::cerr << "CMDPRO:\tCommand pushed to queue.\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/// Takes commands from the command queue and executes them
void MotorServer::commandExecutionLoop() {
    while (true) {
        while (!commandQueue.empty()) {
            std::tuple<mCmd, float, float> command = popCommand();

            mCmd cmd = std::get<0>(command);
            float speed = std::get<1>(command);
            float duration = std::get<2>(command);

            std::cerr << "CMDEXE:\tExecuting " << enumToString(cmd) << '\n';

            if (cmd != mCmd::SpdOn && cmd != mCmd::SpdOff) {
                std::string exeString = "<<SERVER>>\tExecuting '";
                exeString.append(enumToString(cmd)).append(" ")
                    .append(to_string_with_precision(speed, 2)).append("V ")
                    .append(to_string_with_precision(duration, -1)).append("ms'...");

                sendResponse(exeString);
            }

            switch (cmd) {
				case mCmd::rotateCW:
					motorController.turnCW(speed, duration);
					break;
				case mCmd::rotateCCW:
					motorController.turnCCW(speed, duration);
					break;
				case mCmd::SpdOn:
					startMonitorSpeedMeasure();
					break;
				case mCmd::SpdOff:
					stopMonitorSpeedMeasure();
					break;
				case mCmd::Acc:
					motorController.setAcceleration(speed);
					break;
				case mCmd::quit:
					quitClient();
					break;
				default:
					break;
            }
            std::cerr << "CMDEXE:\tCompleted " << enumToString(cmd) << '\n';
            if (cmd == mCmd::SpdOn || cmd == mCmd::SpdOff) {
                std::string exeString = "<<SERVER>>\tSpeed measurement set to: '";
                exeString.append(get_string_from_bool(doSpeedMeasure)).append("'.");

                sendResponse(exeString);
            }
            else {
                std::string compString = "<<SERVER>>\tCompleted '";
                compString.append(enumToString(cmd)).append(" ")
                    .append(to_string_with_precision(speed, 2)).append("V ")
                    .append(to_string_with_precision(duration, -1)).append("ms'");
                sendResponse(compString);
            }

        }
        motorController.stopMotor();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
////////////////////////////////////////////////////////////////////////

//  SPEED MEASUREMENT   ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

///	Enqueues speed data to be sent
void MotorServer::measureSpeedLoop() {
    while (doSpeedMeasure) {
        sendSpeed(to_string_with_precision(rpm, 2));
        delay(100);
    }
    std::cerr << "LOOP MEASURE STOP\n";
}

////////////////////////////////////////////////////////////////////////

///	Sets up the controller and monitor and starts the server's threads
void MotorServer::startServer() {
    motorController.setupController();
    
    //  start command processing loop
    std::thread([this]() { this->commandProcessingLoop(); }).detach();

    //  start command execution loop
    std::thread([this]() { this->commandExecutionLoop(); }).detach();

    // start response thread
    std::thread([this]() { this->responseLoop(); }).detach();

    //  start speed listening loop
    std::thread([this]() { this->speedLoop(); }).detach();

    //  start server listening loop
    std::thread([this]() { this->serverLoop(); }).join();

}

