#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define SIZE_OF_MESSAGE 128

#define DEBUG 1

typedef struct descriptors
{
    int client_one;
    int client_two;
}descriptors_t;

int createSocketConnectionForServer(int* server_fd, struct sockaddr_in* address)
{
    int serverFd = 0;
    *server_fd = 0;

    socklen_t addrLength = sizeof(*address);
    int opt = 1;
    
    char message[SIZE_OF_MESSAGE];
    strcpy(message,"Hello from server\n");

    if((serverFd = socket(AF_INET,SOCK_STREAM, 0)) < 0) 
    {
        printf("Error at socket\n");
        return -1;
    }

    if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt,sizeof(opt)))
    {
        printf("Error at setsockopt\n");
        return -2;
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);

    if (bind(serverFd,(struct sockaddr*) address,addrLength) < 0) 
    {
        printf("bind failed");
        return -3;
    }

    // Maximum 2 connection with server, for rest an error is sent
    if(listen(serverFd,2) < 0)
    {
        printf("Error at listen\n");
        return -4;
    }

    *server_fd = serverFd;  

    return 0;
}

int createConnectionWithClient(int* client_fd,int server_fd,struct sockaddr_in* address)
{
    int new_socket_fd = 0;
    socklen_t addrLength = sizeof(address);

    #if DEBUG
        printf("Connecting with client.... \n");
    #endif

    if ((new_socket_fd = accept(server_fd, (struct sockaddr*)address,&addrLength)) < 0) {
        printf("accept");
        return -1;
    }
    
    *client_fd = new_socket_fd;
    #if DEBUG
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address->sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Connection established with client: %s:%d\n", client_ip, ntohs(address->sin_port));

    #endif

    return 0;
}

int closeConnectionWithClients(int client_one,int client_two)
{
    if(close(client_one) != 0 || close(client_two) != 0)
    {
        printf("Error at closing the connection with client\n");
        return -1;
    }

    return 0;
}

int sendMessageToClient(int client_fd, char* message)
{
    if(strlen(message) >= SIZE_OF_MESSAGE)
    {
        #if DEBUG
            printf("Message length greater than the SIZE_OF_MESSAGE\n");
        #endif
        return -1;
    }
    
    if(send(client_fd,message,strlen(message),0) < 0)
    {
        printf("Message not sent\n");
        return -2;
    }
    return 0;
}

int readMessageFromClient(int client_fd,char* message)
{
    int bytesRead = recv(client_fd,message,SIZE_OF_MESSAGE-1,0);
    if(bytesRead <= 0)
    {
        message[0] = 0;
        return -1;
    }

    message[bytesRead] = 0;
    
    return 0;
}

int gameIsOver()
{
    return 0;
}

int checkIfQuit(char* message)
{
    if(strcmp(message,"quit") == 0)
    {
        return 0;
    }
    return 1;
}

void initGameTurns(int firstPlayer, int secondPlayer,char* initMessage)
{
    sendMessageToClient(firstPlayer,initMessage);
    sendMessageToClient(secondPlayer,initMessage);

    usleep(500000);
}

int firstPlayerMove(int firstPlayer, char* message, char* waitMessage, char* yourTurnMessage)
{
    printf("Waiting for message from client %d....\n",firstPlayer);
    
    sendMessageToClient(firstPlayer,yourTurnMessage);
    
    if(readMessageFromClient(firstPlayer,message) == -1)
    {
        return -1;
    }

    if(strcmp(message,"quit") == 0)
    {
        return -1;
    }

    sendMessageToClient(firstPlayer,waitMessage);

    printf(" -> from player %d: %s\n",firstPlayer,message);

    return 0;
}

int secondPlayerMove(int secondPlayer, char* message, char* waitMessage, char* yourTurnMessage)
{
    printf("Waiting for message from client %d....\n",secondPlayer);
   
    sendMessageToClient(secondPlayer,yourTurnMessage);
    
    if(readMessageFromClient(secondPlayer,message) == -1)
    {
        return -1;
    }

    if(strcmp(message,"quit") == 0)
    {
        return -1;
    }
    
    sendMessageToClient(secondPlayer,waitMessage);
    
    printf(" -> from player %d: %s\n",secondPlayer,message);

    return 0;
}

void* startGame(void* descriptors)
{
    if(pthread_detach(pthread_self()) != 0)
    {
        printf("Error at detaching the thread\n");
        return NULL;
    }

    descriptors_t* client_descriptors = (descriptors_t*)descriptors;
    int firstPlayerFd = client_descriptors->client_one;
    int secondPlayerFd = client_descriptors->client_two;
    printf("   client one: %d client two: %d\n",client_descriptors->client_one,client_descriptors->client_two);


    // MESSAGE TYPES FOR CLIENT
    char receivedMessage[SIZE_OF_MESSAGE];
    char waitMessage[SIZE_OF_MESSAGE];
    char yourTurnMessage[SIZE_OF_MESSAGE];
    char initGameTurnsMessage[SIZE_OF_MESSAGE];
    char quitMessage[SIZE_OF_MESSAGE];

    strcpy(waitMessage,"wait");
    strcpy(yourTurnMessage,"yourTurn");
    strcpy(initGameTurnsMessage,"initialize");
    strcpy(quitMessage,"quit");

    // PUTING BOTH CLIENTS ON WAIT
    initGameTurns(firstPlayerFd,secondPlayerFd,initGameTurnsMessage);

    
    // ======= TO DO implement quit command ====
    while(!gameIsOver())
    {

        if(firstPlayerMove(firstPlayerFd,receivedMessage,waitMessage,yourTurnMessage) == -1)
        {
            usleep(500000);
            sendMessageToClient(secondPlayerFd,quitMessage);

            closeConnectionWithClients(firstPlayerFd,secondPlayerFd);

            printf("Game for players: %d and %d has been forcefully ended \n",firstPlayerFd,secondPlayerFd);

            break;
        }

        if(secondPlayerMove(secondPlayerFd,receivedMessage,waitMessage,yourTurnMessage) == -1)
        {
            usleep(500000);
            sendMessageToClient(firstPlayerFd,quitMessage);

            closeConnectionWithClients(firstPlayerFd,secondPlayerFd);

            printf("Game for players: %d and %d has been forcefully ended \n",firstPlayerFd,secondPlayerFd);

            break;
        }

    }

    return NULL;
}

int createGame(int client_one,int client_two)
{
    pthread_t thread;
    descriptors_t* descriptors_struct = malloc(sizeof(descriptors_t));
    if(descriptors_struct == NULL)
    {
        printf("Error at malloc\n");
        return -1;
    }
    descriptors_struct->client_one = client_one;
    descriptors_struct->client_two = client_two;

    if((pthread_create(&thread,NULL,startGame,descriptors_struct))<0)
    {
        printf("Error at creating game\n");
        return -2;
    }

    return 0;
}

// TO DO
// CLOSE CONNECTION FOR EVERY CLIENT

int main(int argc, char** argv)
{
    // ===== Variables Declaration =====
    int server_fd = 0;
    int client_one_fd = 0;
    int client_two_fd = 0;
    struct sockaddr_in address;


    // ===== Create Socket =====
    if(createSocketConnectionForServer(&server_fd,&address) != 0)
    {
        perror("Error at Socket Connection With Server\n");
        exit(1);
    }

    while(1)
    {
        // ===== Establish connection with client one =====
        #if DEBUG
            printf(" waiting for client pair...\n");
        #endif

        if(createConnectionWithClient(&client_one_fd,server_fd,&address) != 0)
        {
            printf("Error at connection with client one\n");
            return 0;
        }
        
        // ===== Establish connection with client two =====
        if(createConnectionWithClient(&client_two_fd,server_fd,&address) != 0)
        {
            printf("Error at connection with client two\n");
            return 0;
        }
        
        #if DEBUG
            printf(" Connection established with both clients\n");
        #endif
        
        if(createGame(client_one_fd,client_two_fd) != 0)
        {
            // Error at creating the game
            if(closeConnectionWithClients(client_one_fd,client_two_fd) != 0)
            {
                printf("Error at closing connections\n");
                return 0;
            }
            continue;
        }
    }

    

    return 0;
}
