#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

constexpr int PORT = 12345;
constexpr int BUFFER_SIZE = 1024;

int main() {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE] = { 0 };

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Set server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Accept incoming connection
    if ((newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize)) < 0) {
        std::cerr << "Accept failed\n";
        return 1;
    }

    // Receive data from client
    ssize_t bytesRead;
    while ((bytesRead = recv(newSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        std::cout << "Received from client: " << buffer << std::endl;
        memset(buffer, 0, BUFFER_SIZE); // Clear buffer
    }

    if (bytesRead == 0) {
        std::cout << "Client disconnected\n";
    }
    else if (bytesRead == -1) {
        std::cerr << "Receive failed\n";
        return 1;
    }

    close(newSocket); // Close connection
    close(serverSocket); // Close server socket

    return 0;
}
