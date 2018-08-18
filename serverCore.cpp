#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <bits/stdc++.h>
using namespace std;
int main(int argc ,char *argv[]){
    //-------------intitalize------------
    int clientLimit = strtol(argv[1],NULL,10);
    char *serverIP = argv[2];
    int serverPort = strtol(argv[3],NULL,10);
    cout<< "bug" << endl;
    vector<int> clientSocket(clientLimit,0);//initialising client scoket
    
    int masterSocket = 0 ;//creating master node
    if((masterSocket = socket(AF_INET,SOCK_STREAM,0))==0){
        perror("scoket was failed");
        exit(EXIT_FAILURE);
    }
    
    //master node can have multiple sockets
    int opt = 1;
    if( setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }

    //type of socket created
    struct sockaddr_in address;  
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = inet_addr(serverIP);  
    address.sin_port = htons(serverPort);
    
    //bind the socket to given serverIP and serverPort 
    if (bind(masterSocket, (struct sockaddr *)&address, sizeof(address))<0)  {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }

    //check if server is listeneing on serverIP and serverPort
    cout << "server is listening at "<< serverIP << " " << serverPort <<endl;

    //master should have at max of 3 pending connections
    if (listen(masterSocket, 3) < 0){  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }

    //now we will wait for client to connect
    int addrlen = sizeof(address);
    cout << "Waiting for client to connect ............."<< endl;

    fd_set allFds; //contains socket info of all the sockets(one we are pushing)
    
    while(1){
        
        FD_ZERO(&allFds);//clear allFds  
        FD_SET(masterSocket, &allFds);//add master socket to allFds  
        int maxSd = masterSocket; //this contain value of max value of file discriptor Note:file discriptor value start from 1  
        
        //push client sockets into allFds 
        for ( int i = 0 ; i < clientLimit ; i++){     
            int sd = clientSocket[i];  
            if(sd > 0) //if sd is valed then puch in allFds  
                FD_SET( sd , &allFds);      
            if(sd > maxSd)//updating maxSd with latest sd
                maxSd = sd;  
        }

        //cheching if all activity are going fine with any errors
        int activity = select( maxSd + 1 , &allFds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR)){  
            printf("select error");  
        }

        //some change has been occered to server i,e: new client might have been added or removed
        if (FD_ISSET(masterSocket, &allFds)){  
            int newSocket;
            if((newSocket = accept(masterSocket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0){  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , newSocket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            
            //Greet new client
            string greet = "new client how are U?";
            send(newSocket, greet.c_str(), strlen(greet.c_str()), 0){  
            
            cout << "Greeting has been sent" << endl;

            //add newSocket to clientSocket[] vector
            for (int i = 0; i < clientLimit; i++){
                if( clientSocket[i] == 0 ){  
                    clientSocket[i] = newSocket;  
                    printf("Adding to list of sockets as %d\n" , i);
                    break;  
                }
            }
        }

        //some changes have occered in client i,e: client is dead or it has sent message
        for (int i = 0; i < clientLimit; i++){  
            int sd = clientSocket[i];  
            if (FD_ISSET( sd , &allFds)){  
                int valread;
                char buffer[1024]; 
                if ((valread = read( sd , buffer, 1024)) == 0){//Client has been disconnected 
                    getpeername(sd , (struct sockaddr*)&address,(socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                    close( sd );  
                    clientSocket[i] = 0;  
                }     
                //Client has not been disconnected and it has sent some message 
                else{  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buffer[valread] = '\0'; 
                    cout <<"asdkjf"<<endl;
                    cout << buffer << endl; 
                    send(sd , buffer , strlen(buffer) , 0 );  
                }
            }  
        }

    }
    
    return 0;
}