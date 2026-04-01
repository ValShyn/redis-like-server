#pragma once

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;


// #include <arpa/inet.h> // ?????

// // ...
// serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

int main(){
    string port ="0.0.0.0";
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6379);// nie 8080 bo w Redis zazwyczaj korzystają z 6379
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        std::cout << "Failed to bind socket!\n";
        return 1;
    }
    std::cout << "Listening with protocol TCP on port" << port << ".\n";
}


