#include <iostream>
#include "TcpSocketServer.hpp"


int main(){
    CommandProcessor processor;

    std::string port ="6379";// our port
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);// af_inet = ip4 , sock_stream = TCP, 0 is flags

    if (serverSocket == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    int opt = 1;

    // Przygotowujemy odpowiedni wskaźnik zależnie od systemu
#ifdef _WIN32
    const char* opt_ptr = (const char*)&opt;
#else
    const void* opt_ptr = &opt;
#endif

    // Wywołujemy funkcję tylko raz, używając przygotowanego wskaźnika
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, opt_ptr, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
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

    runEventLoop(serverSocket, processor);
}
