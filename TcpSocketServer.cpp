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
/* 
Here we use if to know on what operating system
the server is launched
*/
#if defined(__APPLE__)
    #define USE_KQUEUE // I use MAC
    #include <sys/event.h>
    #include <time.h>
#else
    #define USE_POLL // someone may use Linux/Windows
    #include <poll.h>
#endif




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

                // make a new card for a new user
                EV_SET(&change_event, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
                //we send 1 card , and recieve NULL with its length 0, the lsat NULL is time-out

                kevent(kq, &change_event, 1, NULL, 0, NULL);
            } else if (evList[i].flags & EV_EOF) {          //EV_EOF means that client's got disconnected
                close(current_fd); // in OS our socket is written like {5, clientsokcet} so we use close() to close by number
            } else {
                char buffer[1024] = {0};
                int bytesReceived = recv(current_fd, buffer, sizeof(buffer), 0);
                if(bytesReceived <= 0){
                    std::cout <<"Client disconected";
                }
                if(bytesReceived > 0){
                    std::string rawMessage(buffer, bytesReceived);// it'll take from buffer exactly bytesReceived bytes, ignoring /0
                    std::string response = processor.execute(RespParser::parse(rawMessage));
                    size_t totalSent = 0;
                    size_t bytesLeft = response.length();
                    const char* data = response.c_str();//oldshcool func send() doesn't understand string so we convert it to char array
                    //we send data by parts
                    while (totalSent < bytesLeft) {
                        int n = send(current_fd, data + totalSent, bytesLeft - totalSent, 0);
                        
                        if (n < 0) {
                            std::cerr << "Send error: " << strerror(errno) << std::endl;
                            break; // break out of while
                        }
                        
                        totalSent += n;
                    }
                }

            }
            }
        }

        

    }
#endif
    


    

    
    












    // //while loop cause it's sarver
    // while(true){
    //     int clientSocket = accept(serverSocket, nullptr, nullptr);
    //     // int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

    //     if(clientSocket < 0){
    //         std::cerr << "Accepting failed" << strerror(errno) << std::endl;
    //         continue;
    //     }

    //     std::cout << "Client are connected" << std::endl;

    //     //use while cause always waiting for something
    //     while(true){
    //         char buffer[1024] = {0};
    //         int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);//receiving our command
    //         //if we don't get message it means something goes wrong
    //         if(bytesReceived <= 0){
    //             if(bytesReceived == 0){
    //                 std::cout <<"Client disconected";
    //             } else{
    //                 std::cerr << "Disconnect" << strerror(errno) << std::endl;
    //             }
    //             break;
    //         }
    //         //create string which size is buffer and the value is bytesReceived
    //         std::string rawMessage(buffer, bytesReceived);

    //         std::string response = processor.execute(RespParser::parse(rawMessage));//use parse method to parse ht command and give it to the CommandProcassor method

    //         size_t totalSent = 0;
    //         size_t bytesLeft = response.length();
    //         const char* data = response.c_str();//our oldshcool func send() doesn't understand string so we convert it to char array
    //         //we send data by parts
    //         while (totalSent < bytesLeft) {
    //             int n = send(clientSocket, data + totalSent, bytesLeft - totalSent, 0);
                
    //             if (n < 0) {
    //                 std::cerr << "Send error: " << strerror(errno) << std::endl;
    //                 break; // break out of while
    //             }
                
    //             totalSent += n;
    //         }
    //     }
    // close(clientSocket);//close client socket
    // std::cout << "Client socket is closed" << std::endl;        
    // }
    
