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
    map <int, string> usersMap;
    map <int, string> chatRoomMap;
    vector<string> listRoom; 
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
    if (listen(masterSocket, clientLimit) < 0){  
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
            send(newSocket, greet.c_str(), strlen(greet.c_str()), 0) ;
            
            cout << "Greeting has been sent" << endl;

            char name[1024];
            int v;
            if ((v = read(newSocket,name, 1024))!=0){
                name[v] = '\0';
            } 
            
            string s(name);
            //add newSocket to clientSocket[] vector
            for (int i = 0; i < clientLimit; i++){
                if( clientSocket[i] == 0 ){
                    usersMap.insert(pair <int, string> (i,s));
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
                    buffer[valread] = '\0';
                    //cout << buffer << endl;
                    // string baseQuery = s(buffer);
                    vector<string> query; 
                    char *token = strtok(buffer, " ");
                    string s(token);
                    query.push_back(s);
                    while (token != NULL){
                        // printf("%s\n", token);
                        token = strtok(NULL, " ");
                        if(token == NULL) break;
                        string s1(token);
                        query.push_back(s1);
                    }
                    if(query[0]=="reply" && (query[2]=="udp" || query[2]=="tcp")){
                        //client want to send file
                    }else if(query[0]=="reply"){
                        //client want to send message
                        string reply ;
                        for(int j = 1 ; j < query.size() ; j++){
                            reply = reply + query[j] +" ";
                        }
                        cout << "reply will be : " << reply << endl; 
                        for(int j = 0 ; j< clientLimit ; j++){
                            int sdBrodcast = clientSocket[j];
                            if(sdBrodcast>0 && sdBrodcast!=sd){
                                send(sdBrodcast , reply.c_str() , strlen(reply.c_str()) , 0 );
                            }
                        }
                    }else if(query[0]=="lU"){
                        for (auto it : usersMap){
                            cout << " " << it.first << ":" << it.second << endl;    
                        } 
                    }else if(query[0]=="create"&& query[1]=="chatroom"){
                        listRoom.push_back(query[2]);
                        chatRoomMap.insert(pair <int, string>(i,query[2]));    
                    }else if(query[0]=="join"){
                        chatRoomMap.insert(pair <int, string>(i,query[1]));
                    }else if(query[0]=="leave"){
                        chatRoomMap.erase(i);
                    }else if(query[0]=="list" && query[1]=="chatrooms"){
                        for(int j = 0 ; j < listRoom.size();j++){
                            cout << listRoom[j] << endl;
                        }
                    }else if(query[0]=="list" && query[1]=="users"){
                        for (auto it : usersMap){
                            if(chatRoomMap[it.first]==chatRoomMap[i]){
                                cout << " " << it.first << ":" << it.second << endl;
                            }
                        }
                    }else if(query[0]=="add"){
                        for(auto it : usersMap){
                            if(it.second==query[1]){
                                chatRoomMap[it.first] = chatRoomMap[i];
                            }
                        }
                    }
                    else if(query[0]=="my" && query[1]=="chatroom"){
                        cout << chatRoomMap[i] << endl;
                    }
                    else if(query[0]=="help"){
                        cout <<"All commands : "<< endl;
                        cout <<"    "<<"create chatroom {chatRoom1}"<< endl;
                        cout <<"    "<<"list chatrooms"<< endl;
                        cout <<"    "<<"join {chatRoom1}"<< endl;
                        cout <<"    "<<"leave"<< endl;
                        cout <<"    "<<"list users"<< endl;
                        cout <<"    "<<"add {user2}"<< endl;
                        cout <<"    "<<"reply \"message content\""<< endl;
                        cout <<"    "<<"reply A.txt tcp"<< endl;
                        cout <<"    "<<"reply A.txt udp"<< endl;
                        cout <<"    "<<"my chatroom"<< endl;   
                    } 
                }
            }  
        }

    }
    
    return 0;
}