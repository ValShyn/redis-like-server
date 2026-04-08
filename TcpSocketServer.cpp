#include "CommandProcessor.hpp"
#include "RespParser.hpp"

#include <sstream>
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

    CommandProcessor processor;

    std::string port ="6379";// our port
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);// af_inet = ip4 , sock_stream = TCP, 0 is flags

    if (serverSocket == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    sockaddr_in serverAddress;// Struct form socket class
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6379);// not 8080 cause in Redis we usually use 6379, htons change from bid-endian to little

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
    
    //while loop cause it's sarver
    while(true){
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        // int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if(clientSocket < 0){
            std::cerr << "Accepting failed" << strerror(errno) << std::endl;
            continue;
        }

        std::cout << "Client are connected" << std::endl;

        //use while cause always waiting for something
        while(true){
            char buffer[1024] = {0};
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);//receiving our command
            //if we don't get message it means something goes wrong
            if(bytesReceived <= 0){
                if(bytesReceived == 0){
                    std::cout <<"Client disconected";
                } else{
                    std::cerr << "Disconnect" << strerror(errno) << std::endl;
                }
                break;
            }
            //create string which size is buffer and the value is bytesReceived
            std::string rawMessage(buffer, bytesReceived);

            std::string response = processor.execute(RespParser::parse(rawMessage));//use parse method to parse ht command and give it to the CommandProcassor method

            size_t totalSent = 0;
            size_t bytesLeft = response.length();
            const char* data = response.c_str();//our oldshcool func send() doesn't understand string so we convert it to char array
            //we send data by parts
            while (totalSent < bytesLeft) {
                int n = send(clientSocket, data + totalSent, bytesLeft - totalSent, 0);
                
                if (n < 0) {
                    std::cerr << "Send error: " << strerror(errno) << std::endl;
                    break; // break out of while
                }
                
                totalSent += n;
            }
        }
    close(clientSocket);//close client socket
    std::cout << "Client socket is closed" << std::endl;        
    }
    
}
