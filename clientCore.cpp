// Client side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char *argv[]){
    string Name = argv[1];
    char *serverIP = argv[2];
    int serverPort = strtol(argv[3],NULL,10);
    
    int clientSocket = 0 ;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Socket creation error \n");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serverIP);
    serv_addr.sin_port = htons(serverPort);
    
    //connect to server
    if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("\nConnection Failed \n");
        return -1;
    }
    string hello = "Hello from client";
    char buffer[1024] = {0};

    fd_set ioOrServer;
    while(true){
        FD_ZERO(&ioOrServer);//clear allFds  
        FD_SET(clientSocket, &ioOrServer);
        FD_SET(0, &ioOrServer);
        if(FD_ISSET(0,&ioOrServer) || FD_ISSET(clientSocket,&ioOrServer)){
            
            // cout << "----All commands----- :" << endl;
            // cout << "create chatroom chatRoom1" << endl;
            // cout << "list chatrooms" << endl;
            // cout << "join chatRoom1" << endl;
            // cout << "leave" << endl;
            // cout << "list users" << endl;
            // cout << "add user2" << endl;
            // cout << "reply message content" << endl;
            // cout << "reply A.txt tcp" << endl;
            // cout << "reply A.txt udp" << endl;
            cout << ">>" ;
        }

        
        if(FD_ISSET(clientSocket,&ioOrServer)){
            int valread = read(clientSocket,buffer,1024);
            cout << buffer << endl;
        }
        
        if(FD_ISSET(0,&ioOrServer)){
            read(0,buffer,1024);
            send(clientSocket,buffer,1024,0);    
        }
        // int valread = read(clientSocket,buffer,1024);
        // printf("%s\n",buffer );
        // // cin >> *hello;
        // hello = "I am fine server";
        // send(clientSocket , hello.c_str() , strlen(hello.c_str()) , 0 );
        // printf("Hello message sent\n");
    }    
    return 0;
}
