#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <ncurses.h>
#include <ctype.h>
#include "chessLogic.h"
#include "GUI.h"

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

int sendMatrixToClient(int client_fd,Element_T* matrix)
{
    
    if(send(client_fd,matrix,sizeof(Element_T)*COLUMN*ROW,0) < 0)
    {
        printf("Matrix not sent\n");
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

int checkIfQuit(char* message)
{
    if(strcmp(message,"quit") == 0)
    {
        return 0;
    }
    return 1;
}

piece_t **convert_raul_to_ana(Element_T m[])
{
  piece_t **new = (piece_t **)malloc(sizeof(piece_t *) * ROW);
  if(!new)
    {
      perror("Error allocating matrix in convert_raul_to_ana()");
      exit(EXIT_FAILURE);
    }
  
  for(int i = 0; i < ROW; i++)
    {
      new[i] = (piece_t *)malloc(sizeof(piece_t) * COLS);
      if(!new[i])
      {
         perror("Error allocating m[i] in convert_raul_to_ana()");
         exit(EXIT_FAILURE);
      }
    }   
  
  for(int i = 0; i < ROW; i++)
    {
      for(int j = 0; j < COLS; j++)
      {
        // --- THE FIX IS HERE ---
        // We use a DIRECT mapping. 
        // i = 0 (Screen Top) reads Logic Index 0 (Black Pieces).
        // i = 7 (Screen Bottom) reads Logic Index 7 (White Pieces).
        int index = i * ROW + j;

        if((m[index].isWhite))
        {
           new[i][j].color = BLACK; // Remember: BLACK constant = Blue (White pieces)
        }
        else
        {
           new[i][j].color = RED;   // Remember: RED constant = Red (Black pieces)
        }
        
        new[i][j].x = i;
        new[i][j].y = j;
        
        switch(m[index].name)
        {
          case PAWN:   new[i][j].type = pawn;   break;
          case ROOK:   new[i][j].type = rook;   break;
          case BISHOP: new[i][j].type = bishop; break;
          case KNIGHT: new[i][j].type = knight; break;
          case QUEEN:  new[i][j].type = queen;  break;
          case KING:   new[i][j].type = king;   break;
          case Empty:  new[i][j].color = NONE;  break;
        }
      }
    }
  
  return new;
}

void initGameTurns(int firstPlayer, int secondPlayer,char* initMessage)
{
    usleep(500000);

    sendMessageToClient(firstPlayer,initMessage);
    sendMessageToClient(secondPlayer,initMessage);

    usleep(500000);
}


void printMatrixToClientsAndServer(int firstPlayer, int secondPlayer, Game_T* Game)
{
    print_Game(Game);

    usleep(500000);
    sendMessageToClient(firstPlayer,"printGame");
    sendMessageToClient(secondPlayer,"printGame");
    usleep(500000);
    
    sendMatrixToClient(firstPlayer,Game->Matrix);
    sendMatrixToClient(secondPlayer,Game->Matrix);

    usleep(500000);
}

int firstPlayerMove(int firstPlayer,int secondPlayer, char* message, char* waitMessage, char* yourTurnMessage,Game_T *Game,char *move)
{
    //printf("Waiting for message from client %d....\n",firstPlayer);
    
    printMatrixToClientsAndServer(firstPlayer,secondPlayer,Game);

    int i=0;
    do{
        printf("Trimit la client %d\n",i);
        sendMessageToClient(firstPlayer,yourTurnMessage);
        printf("astept de la client\n");
        if(readMessageFromClient(firstPlayer,message) == -1)
        {
            return -1;
        }
        printf("primit de la client");
        if(strcmp(message,"quit") == 0)
        {
            return -1;
        }
        if(strcmp(message,"draw") == 0)
        {
            char responseDraw[STRING_SIZE];
            usleep(500000);
            sendMessageToClient(secondPlayer,"draw");
            readMessageFromClient(secondPlayer,responseDraw);
            if(strcmp(responseDraw,"yes") ==0 )
             {
                Game->game_is_running = false;
		        Game->game_ended_in_draw = true;
                break;
             }
        }
        i++;
    }while(Game_Move(Game,Game->client1,message)==false);

    strcpy(move,message);
    sendMessageToClient(firstPlayer,waitMessage);


    //printf(" -> from player %d: %s\n",firstPlayer,message);

    return 0;
}

int secondPlayerMove(int firstPlayer,int secondPlayer, char* message, char* waitMessage, char* yourTurnMessage,Game_T *Game,char *move)
{
    //printf("Waiting for message from client %d....\n",secondPlayer);
    printMatrixToClientsAndServer(firstPlayer,secondPlayer,Game);


    int i=0;
    do{
        printf("Trimit la client %d\n",i);
        sendMessageToClient(secondPlayer,yourTurnMessage);
        printf("astept de la client\n");
        if(readMessageFromClient(secondPlayer,message) == -1)
        {
            return -1;
        }
        printf("primit de la client");
        if(strcmp(message,"quit") == 0)
        {
            return -1;
        }
        if(strcmp(message,"draw") == 0)
        {
            char responseDraw[STRING_SIZE];
            usleep(500000);
            sendMessageToClient(firstPlayer,"draw");
            readMessageFromClient(firstPlayer,responseDraw);
            if(strcmp(responseDraw,"yes") ==0 )
             {
                Game->game_is_running = false;
		        Game->game_ended_in_draw = true;
                break;
             }
        }
     i++;
    }while(Game_Move(Game,Game->client2,message)==false);

    strcpy(move,message);
    sendMessageToClient(secondPlayer,waitMessage);
    
    return 0;
}

bool askForRematch(int firstPlayer,int secondPlayer)
{
    usleep(500000);
    sendMessageToClient(firstPlayer,"rematch");
    sendMessageToClient(secondPlayer,"rematch");

    char firstPlayerMessage[SIZE_OF_MESSAGE];
    char secondPlayerMessage[SIZE_OF_MESSAGE];

    readMessageFromClient(firstPlayer,firstPlayerMessage);
    readMessageFromClient(secondPlayer,secondPlayerMessage);

    if(strcmp(firstPlayerMessage,"yes") == 0 && strcmp(secondPlayerMessage,"yes") == 0)
    {
        return true;
    }

    return false;
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

    // GETING CLIENTS USERNAMES
    char firstUsername[SIZE_OF_MESSAGE];
    char secondUsername[SIZE_OF_MESSAGE];

    readMessageFromClient(firstPlayerFd,firstUsername);
    readMessageFromClient(secondPlayerFd,secondUsername);

    sendMessageToClient(firstPlayerFd,secondUsername);
    sendMessageToClient(secondPlayerFd,firstUsername);

    usleep(500000);
    sendMessageToClient(firstPlayerFd,"white");
    sendMessageToClient(secondPlayerFd,"black");

    // PUTING BOTH CLIENTS ON WAIT
    initGameTurns(firstPlayerFd,secondPlayerFd,initGameTurnsMessage);
    
    Game_T* Game=Initialize_Classic_Game();
    Game->client1.isWhite=true;
    Game->client2.isWhite=false;
    strcpy(Game->client1.username,firstUsername);
    strcpy(Game->client2.username,secondUsername);
    piece_t **matrixForPrint = convert_raul_to_ana(Game->Matrix);
    bool rematch=true;
    char *move=malloc(STRING_SIZE);

    while(rematch==true)
    {
        printf("A new game started between  client one: %d client two: %d\n",client_descriptors->client_one,client_descriptors->client_two);
        //print_Game(Game);
        while(Game->game_is_running)
        {
            if(firstPlayerMove(firstPlayerFd,secondPlayerFd, receivedMessage,waitMessage,yourTurnMessage,Game,move) == -1)
            {
                usleep(500000);
                sendMessageToClient(secondPlayerFd,quitMessage);

                closeConnectionWithClients(firstPlayerFd,secondPlayerFd);

                printf("Game for players: %d and %d has been forcefully ended \n",firstPlayerFd,secondPlayerFd);

                break;
            }

            if(Game->game_is_running==false)
             {
                break;
             }

            if(Game->is_black_king_checked==true)
             {
                usleep(500000);
                
                printf("-> Black Checked\n");
                sendMessageToClient(firstPlayerFd,"blackCheck");
                sendMessageToClient(secondPlayerFd,"blackCheck");
             } 

            if(secondPlayerMove(firstPlayerFd,secondPlayerFd,receivedMessage,waitMessage,yourTurnMessage,Game,move) == -1)
            {
                usleep(500000);
                sendMessageToClient(firstPlayerFd,quitMessage);

                closeConnectionWithClients(firstPlayerFd,secondPlayerFd);

                printf("Game for players: %d and %d has been forcefully ended \n",firstPlayerFd,secondPlayerFd);

                break;
            }

            if(Game->game_is_running==false)
             {
                break;
             }

            Game->number_of_moves_played++;

            if(Game->number_of_moves_played==40)
            {
                Game->game_is_running=false;
                Game->game_ended_in_draw=true;
                Game->game_ended_in_white_won=false;
                break;
            }

            if(Game->is_white_king_checked==true)
             {
                usleep(500000);
                printf("-> White Cheked\n");
                sendMessageToClient(firstPlayerFd,"whiteCheck");
                sendMessageToClient(secondPlayerFd,"whiteCheck");
             } 

        }
        printf("Match between client one: %d client two: %d  ended\n",client_descriptors->client_one,client_descriptors->client_two);
        Print_Game_Result(Game);

        if(Game->game_ended_in_draw==true)
          {
            usleep(50000);
            sendMessageToClient(firstPlayerFd,"PrintDraw");
            sendMessageToClient(secondPlayerFd,"PrintDraw");
          }
        else
          {
            if(Game->game_ended_in_white_won==true)
             {
                usleep(500000);
                sendMessageToClient(firstPlayerFd,"PrintWin");
                usleep(500000);
                sendMessageToClient(secondPlayerFd,"PrintLose");
             }
            else
             {
                usleep(500000);
                sendMessageToClient(firstPlayerFd,"PrintLose");
                usleep(500000);
                sendMessageToClient(secondPlayerFd,"PrintWin"); 
             }
          }

        rematch = askForRematch(firstPlayerFd,secondPlayerFd);
        if(rematch == true)
        {
            Game = Reinstate_Game(Game);
        }
    }

    printf("Game ended ,removing connection between client one: %d client two: %d \n",client_descriptors->client_one,client_descriptors->client_two);

    free(move);
    free(Game->Matrix);
    free(Game);
    for(int i=0;i<ROW;i++)
      free(matrixForPrint[i]);
    free(matrixForPrint);

    closeConnectionWithClients(firstPlayerFd,secondPlayerFd);

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
