#include "CommandProcessor.hpp"
#include "RespParser.hpp"
#include <sstream>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>


#ifndef _WIN32
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> 
    #include <fcntl.h>  
#endif


#if defined(__APPLE__)
    #define USE_KQUEUE // I use MAC
    #include <sys/event.h>
    #include <time.h>

#elif defined(__linux__) 
    #include <sys/epoll.h>

#elif defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib") // Automatyczne linkowanie na Windowsie
    
    // Hack: Zmuszamy Windowsa, by rozumiał unixowe "close()"
    #define close closesocket 
#endif


bool handleClientData(int clientSocket, CommandProcessor& processor){
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if(bytesReceived <= 0){
        std::cout << "Client disconected. ID :" << clientSocket << std::endl;
        close(clientSocket);
        return false;
    }
    
    std::string rawMessage(buffer, bytesReceived);// it'll take from buffer exactly bytesReceived bytes, ignoring /0
    std::string response = processor.execute(RespParser::parse(rawMessage));
    size_t totalSent = 0;
    size_t bytesLeft = response.length();
    const char* data = response.c_str();//oldshcool func send() doesn't understand string so we convert it to char array
    //we send data by parts
    while (totalSent < bytesLeft) {
        int n = send(clientSocket, data + totalSent, bytesLeft - totalSent, 0);
        
        if (n < 0) {
            std::cerr << "Send error: " << strerror(errno) << std::endl;
            close(clientSocket); // break out of while
            return false;
        }
        
        totalSent += n;
    }

    return true;

}    

void runEventLoop(int serverSocket, CommandProcessor& processor) {
#ifdef _WIN32
    // Switch on windows socket tool
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }
#endif


/*

This is where code splitting begins

*/


#ifdef USE_KQUEUE 
    // the part of code for MAC users
    std::cout << "Redis-clone listening... [Mode: Native macOS kqueue]\n";

    int kq = kqueue(); // create a poppy scoreboard

    struct kevent change_event;
    EV_SET(&change_event, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE , 0, 0, 0); // create a new blank for server to write data and then switch the client to status ENABLE

    kevent(kq, &change_event, 1, NULL, 0, NULL);//Send our blank to kernel and take NULL as a respond cause it's setting

    struct kevent evList[1024]; // create the array of the cards of kevent for info 

    while(true){
        // ask how many users requested something we send to kernel NULL and 0 what make server know that we just want to take the number of changes, -1 == smth wrong
        int num_events = kevent(kq, NULL, 0, evList, 1024, NULL);

        for (int i = 0; i < num_events; i++){
            //.ident is the object of the struct kevent
            int current_fd = evList[i].ident;//file_descriptor


            if(current_fd == serverSocket){

                //make a new connection. nullptr means 0, that poles usually are used for IP address of client , but now we don't need it
                int clientSocket = accept(serverSocket, nullptr, nullptr);
                std::cout << "New client!" << std::endl;

                // make a new card for a new user
                EV_SET(&change_event, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
                //we send 1 card , and recieve NULL with its length 0, the lsat NULL is time-out

                kevent(kq, &change_event, 1, NULL, 0, NULL);
            } else if (evList[i].flags & EV_EOF) {          //EV_EOF means that client's got disconnected
                close(current_fd); // in OS our socket is written like {5, clientsokcet} so we use close() to close by number
            } else {
                handleClientData(current_fd, processor);

            }
            }
        }

        

    
#endif
#ifdef _WIN32
    // Używamy struktur i flag specyficznych dla Windowsa
    std::vector<WSAPOLLFD> fds;
    fds.push_back({(SOCKET)serverSocket, POLLRDNORM, 0});

    while(true){
        // WSAPoll zamiast unixowego poll
        WSAPoll(fds.data(), fds.size(), -1);

        // Obsługa Głównego Szwajcara (nowe połączenia)
        if(fds[0].revents & POLLRDNORM){
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            std::cout << "New client! ID:" << clientSocket << std::endl;
            fds.push_back({(SOCKET)clientSocket, POLLRDNORM, 0});
        }
        
        // ZACZYNAMY OD i = 1 (omijamy Szwajcara pod fds[0])
        for(size_t i = 1; i < fds.size(); ){
            if(fds[i].revents & POLLRDNORM) {
                
                // Przekazujemy poprawny numer gniazda i procesor
                if(!handleClientData(fds[i].fd, processor)){
                    fds.erase(fds.begin() + i); // Usuwamy klienta, jest średnik!
                } else {
                    i++; // Klient zostaje, idziemy do następnego
                }

            } else {
                i++; // Brak zdarzeń, idziemy dalej
            }
        }
    }
#endif

#ifdef __linux__
    int epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD,  serverSocket, &event);
    struct epoll_event activeEvents[1024];
    while(true){
        int num_events = epoll_wait(epoll_fd, activeEvents, 1024, -1);
            for(int i = 0; i < num_events; i++){
                int current_fd = activeEvents[i].data.fd;

                if(current_fd == serverSocket){
                    int clientSocket = accept(serverSocket, nullptr, nullptr);
                    struct epoll_event clientEvent;
                    clientEvent.events = EPOLLIN;
                    clientEvent.data.fd = clientSocket;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &clientEvent);

                } else {
                    handleClientData(current_fd, processor);
            
                }
            }


    }
#endif
}
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
