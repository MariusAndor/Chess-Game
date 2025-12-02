#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 8080
#define SIZE_OF_MESSAGE 128

// === GLOBAL DEFINES FOR BOTH client AND server
#define MAX_LENGTH_OF_A_MESSAGE 128
#define DEBUG 1



int connectWithServer(int* client_fd,struct sockaddr_in* serverAddress)
{
    int fd = 0;
    *client_fd = 0;
    int status = 0;

    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error at socket\n");
        return -1;
    }

    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress->sin_addr) <= 0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        exit(1);
    }

    if ((status = connect(fd,(struct sockaddr*)serverAddress,sizeof(*serverAddress)))< 0)
    {
        printf("\nConnection Failed\n");
        return -2;
    }

    *client_fd = fd;

    return 0;
}

int sendMessageToServer(int client_fd,char* message)
{
    if(strlen(message) >= SIZE_OF_MESSAGE)
    {
        #if DEBUG
            printf("Message length greater than the SIZE_OF_MESSAGE\n");
        #endif
        return -1;
    }

    if(send(client_fd,message,strlen(message),0) == -1)
    {
        printf("Message not sent\n");
        return -2;
    }
    return 0;
}

char* receiveMessageFromServer(int client_fd)
{
    char* message = malloc(sizeof(char)*SIZE_OF_MESSAGE);
    if(message == NULL)
    {
        perror("Error at malloc \n");
        return NULL;
    }

    if(recv(client_fd,message,SIZE_OF_MESSAGE,0) == -1)
    {
        perror("Error at receveing message from server\n");
        return NULL;
    }

    return message;
}

int getMessageFromTerminal(char* message)
{
    printf(" your next move: ");

    if(fgets(message,MAX_LENGTH_OF_A_MESSAGE-1,stdin) == NULL)
    {
        perror("Error at reading messaged from terminal\n");
        return -1;
    }

    message[strlen(message)-1] = 0;

    return 0;
}

int checkIfQuitMessage(char* message)
{
    if(strcmp(message,"quit") == 0)
    {
        return 0;
    }
    return 1;
}

int main(int argc, char** argv)
{
    int client_fd = 0;
    struct sockaddr_in serverAddress;

    printf("Welcome to Chess Game\n");

    #if DEBUG
        printf("connecting to server...\n");
    #endif

    if(connectWithServer(&client_fd,&serverAddress) != 0)
    {
        printf("Error at connecting with server\n");
        return 0;
    }

    #if DEBUG
        printf("succesfully connected\n");
    #endif
 
    char* message = malloc(sizeof(char)*SIZE_OF_MESSAGE);
    char* quitMessage = malloc(sizeof(char)*SIZE_OF_MESSAGE);

    if(message == NULL || quitMessage == NULL)
    {
        perror("Error at malloc\n");
        exit(1);
    }

    strcpy(quitMessage,"quit");
    message = receiveMessageFromServer(client_fd);

    if(message == NULL)
    {
        sendMessageToServer(client_fd,quitMessage);
        return 0;
    }

    while(1)
    {
        
        if(strcmp(message,"yourTurn") == 0)
        {            
            
            if(getMessageFromTerminal(message) != 0)
            {
                sendMessageToServer(client_fd,quitMessage);
                break;
            }
            
            if(checkIfQuitMessage(message) == 0)
            {
                sendMessageToServer(client_fd,quitMessage);
                break;
            }
            
            sendMessageToServer(client_fd,message);

            if((message = receiveMessageFromServer(client_fd)) == NULL)
            {
                perror("Erorr at client receving messages from server\n");
                exit(1);
            }

        }else if(strcmp(message,"wait") == 0 || strcmp(message,"initialize") == 0)
        {
            if(strcmp(message,"wait") == 0)
            {
                printf(" wait for other player's turn\n");
            }
            
            if((message = receiveMessageFromServer(client_fd)) == NULL)
            {
                perror("Erorr at client receving messages from server\n");
                exit(1);
            }

            #if DEBUG
                printf("Message from server: %s\n",message);
            #endif

        }

    }
    
    return 0;
}