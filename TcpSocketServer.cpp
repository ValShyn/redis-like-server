#pragma once

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>  //
#include <cstring>


// #include <arpa/inet.h> // ?????

// // ...
// serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

int main(){
    std::string port ="6379";
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    sockaddr_in serverAddress;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6379);// nie 8080 bo w Redis zazwyczaj korzystają z 6379

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << strerror(errno) << std::endl;
        exit(1);
    }

    std::cout << "Listening with protocol TCP on port" << port << ".\n";
    if(listen(serverSocket, 5) < 0){
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    std::cout << "Redis-clone is listening... " << std::endl;
    

    while(true){
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        // int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if(clientSocket < 0){
            std::cerr << "Accepting failed" << strerror(errno) << std::endl;
            continue;
        }

        std::cout << "Client are connected" << std::endl;

        char buffer[1024] = {0};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if(bytesReceived <= 0){
            std::cerr << "Disconnect" << strerror(errno) << std::endl;
            continue;
        } else{
            std::string message(buffer, bytesReceived);
            if(message.find("PING") != std::string::npos){ // npos = no position
                send(clientSocket, "+PONG\r\n", 7, 0);
            }
        }
        close(clientSocket);
        std::cout << "Client socket is closed" << std::endl;
        
        

    }
}
