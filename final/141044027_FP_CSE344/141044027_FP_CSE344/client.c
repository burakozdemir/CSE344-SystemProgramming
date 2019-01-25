
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int execute(const char*name,const char*priority,int degree,const char* adress,int port);

int main(int argc , char *argv[]){
    if (argc != 6) {
        perror("Usage: $./a.out <clientname> <priority> <degree> <server_address> <port>\n");
        return -1;
    }
    srand(time(NULL));
    char name[20];
    char priority[2];
    int degree=atoi(argv[3]);
    char adress[100];
    int port=atoi(argv[5]);

    strcpy(name,argv[1]);
    strcpy(priority,argv[2]);
    strcpy(adress,argv[4]);

    execute(name,priority,degree,adress,port);
 
}


int execute(const char*name,const char*priority,int degree,const char* adress,int port){
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    srand(time(NULL));

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    //puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr(adress);
    server.sin_family = AF_INET;
    server.sin_port = htons( port);
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        
    }
     
    //puts("Connected\n");

        char sender[100]="";
        sprintf(sender,"%s %s %d",name,priority,degree);

        if( send(sock , sender , strlen(sender) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        sprintf(message,"Client %s is requesting %s %d from server %s:%d\n",name,priority,degree,adress,port);
        printf("%s",message);

        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
        }
         
        printf("%s",server_reply);
     
    close(sock);
    return 0;
}
