#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <locale.h>
#include "GUI.h"

#define PORT 8080
#define SIZE_OF_MESSAGE 128

#define COLUMN 8
#define STRING_SIZE 512

typedef enum chess_pieces
{
  PAWN,
  KNIGHT,
  QUEEN,
  KING,
  ROOK,
  BISHOP,
  Empty,
}Piece_Name;

typedef struct 
{
    Piece_Name name;
    bool isWhite;

}Element_T;

void new_print_Matrix(Element_T *Matrix,char* firstUsername, char* secondUsername)
{
    short unsigned int i = 0, j = 0;

    printf("                  %s-Black       \n",secondUsername);
    for(i=0;i<ROW;i++)
    {
        printf("  ");
        for(j=0;j<COLUMN;j++)
        {
            printf("+----");
        }
        printf("+\n");

        printf("%d ",ROW-i); 

        for(j=0;j<COLUMN;j++)
        {
             
             printf("|");
             switch(Matrix[i*ROW+j].name){   
                 case 0: printf(" P%d ", Matrix[i*ROW+j].isWhite); break; // Pawn
                 case 1: printf(" K%d ", Matrix[i*ROW+j].isWhite); break; // Knight
                 case 2: printf(" Q%d ", Matrix[i*ROW+j].isWhite); break; // Queen
                 case 3: printf(" &%d ", Matrix[i*ROW+j].isWhite); break; // King
                 case 4: printf(" R%d ", Matrix[i*ROW+j].isWhite); break; // Rook
                 case 5: printf(" B%d ", Matrix[i*ROW+j].isWhite); break; // Bishop
                 case 6: printf("    "); break; // Empty square
                 default: printf(" ?? "); break; // Unknown piece
             }
        }
        printf("|\n"); 
    }
    printf("  ");
    for(j=0;j<COLUMN;j++)
    {
        printf("+----");
    }
    printf("+\n");

    printf("  ");
    for(j=0;j<COLUMN;j++)
    {
        printf("  %c  ", 'A' + j);
    }
    printf("\n");
    printf("                  %s-White       \n",firstUsername);
}

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

char *receiveMessageFromServer(int client_fd)
{
    char *message = malloc(SIZE_OF_MESSAGE);
    if (!message) {
        perror("malloc");
        return NULL;
    }

    int bytes = recv(client_fd, message, SIZE_OF_MESSAGE - 1, 0);

    if (bytes == 0) {
        printf("The server ended the connection\n");
        free(message);
        return NULL;
    }

    if (bytes < 0) {
        perror("recv");
        free(message);
        return NULL;
    }

    message[bytes] = '\0';

    return message;
}

int receiveMatrixFromServer(int client_fd,Element_T* element)
{
    ssize_t received = recv(client_fd, element, ROW*COL*sizeof(Element_T), 0);
    if(received <= 0)
    {
        perror("Error at receveing matrix from server\n");
        return -1;
    }

    return 0;
}

Element_T *new_Initialize_Classic_Game_Matrix()
{
    Element_T* Matrix=(Element_T*)malloc(sizeof(Element_T)*ROW*COLUMN);
    if(Matrix==NULL)
      {
        perror("ERROR Allocation");
        exit(3);
      }
    short unsigned int i=0,j=0;

    for(i=0;i<ROW;i++)
      for(j=0;j<COLUMN;j++)
        {
            if(i<=1)
              Matrix[i*ROW+j].isWhite=false;
            else
              Matrix[i*ROW+j].isWhite=true;
        }

    for(i=0;i<ROW;i++)
      for(j=0;j<COLUMN;j++)
        {
            if(i>=2 && i<=5)
              Matrix[i*ROW+j].name=Empty;
        }

    for(j=0;j<COLUMN;j++)
        {
            Matrix[1*ROW+j].name=PAWN;
            Matrix[6*ROW+j].name=PAWN;
        }
    Matrix[0*ROW+0].name=ROOK;   Matrix[0*ROW+7].name=ROOK;   Matrix[7*ROW+0].name=ROOK;   Matrix[7*ROW+7].name=ROOK; 
    Matrix[0*ROW+1].name=KNIGHT; Matrix[0*ROW+6].name=KNIGHT; Matrix[7*ROW+1].name=KNIGHT; Matrix[7*ROW+6].name=KNIGHT;    
    Matrix[0*ROW+2].name=BISHOP; Matrix[0*ROW+5].name=BISHOP; Matrix[7*ROW+2].name=BISHOP; Matrix[7*ROW+5].name=BISHOP;  
    Matrix[0*ROW+3].name=QUEEN;  Matrix[7*ROW+3].name=QUEEN;
    Matrix[0*ROW+4].name=KING;   Matrix[7*ROW+4].name=KING;

    return Matrix;
}

int getMessageFromWindow(char *message,int ox,int oy)
{
    mvprintw(oy + 17, ox, "%s", "                                                ");
	mvprintw(oy + 18, ox, "%s", "type a move: ");
	mvprintw(oy + 18, ox + strlen("type a move: "), "%s", "                                             ");
	strcpy(message,read_from_window(stdscr, oy + 18, ox + strlen("type a move: ")));
    if(message==NULL)
      return -1;

    message[strlen(message)-1] = 0;
    return 0;
}

int getMessageFromTerminal(char* message)
{
    printf(" your next move: ");

    while(scanf("%s",message) != 1)
    {

    }

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


    printf("connecting to server...\n");


    if(connectWithServer(&client_fd,&serverAddress) != 0)
    {
        printf("Error at connecting with server\n");
        return 0;
    }

    printf("succesfully connected\n");
 
    char* message = malloc(sizeof(char)*SIZE_OF_MESSAGE);
    char* quitMessage = malloc(sizeof(char)*SIZE_OF_MESSAGE);

    if(message == NULL || quitMessage == NULL)
    {
        perror("Error at malloc\n");
        exit(1);
    }

    strcpy(quitMessage,"quit");

    char your_username[SIZE_OF_MESSAGE];
    char* oponent_username;
    bool isWhite;
    

    printf("Enter your username: \n");
    while(scanf("%s",your_username) != 1)
    {
        printf("Error, enter a username less than 128 characters\n");
        printf("Enter your username: \n");
    }


    sendMessageToServer(client_fd,your_username);
    oponent_username = receiveMessageFromServer(client_fd);

    if(strcmp("white",receiveMessageFromServer(client_fd)) == 0)
    {
        isWhite = true;
    }else
    {
        isWhite = false;
    }
    
    printf("Searching for an oponent....\n");
    usleep(500000);

    printf("Oponent found: %s\n",oponent_username);
    
    usleep(500000);
    printf("You are %s\n",isWhite ? "white" : "black");


    // INITIALIZATION FOR CHESS MATRIX
    Element_T* matrix = new_Initialize_Classic_Game_Matrix();
    
    // GETING INITIALIZE MESSAGE FROM SERVER
    message = receiveMessageFromServer(client_fd);

    if(message == NULL)
    {
        sendMessageToServer(client_fd,quitMessage);
        return 0;
    }

    // STARTING GAME
    while(1)
    {
    
        usleep(200000);

        if((message = receiveMessageFromServer(client_fd)) == NULL)
        {
            perror("Erorr at client receving messages from server\n");
            exit(1);
        }

        if(strcmp(message,"yourTurn") == 0)
        {            
            if(getMessageFromTerminal(message)!=0)
             {
               sendMessageToServer(client_fd,quitMessage);
               break; 
             }

            /*if(getMessageFromWindow(message,ox,oy) != 0)
            {
                sendMessageToServer(client_fd,quitMessage);
                break;
            }
            */
            
            if(checkIfQuitMessage(message) == 0)
            {
                sendMessageToServer(client_fd,quitMessage);
                break;
            }
            
            sendMessageToServer(client_fd,message);
        }else if(strcmp(message,"printGame") == 0)
        {
            // receive
            receiveMatrixFromServer(client_fd,matrix);

            if(isWhite)
            {
                new_print_Matrix(matrix,your_username,oponent_username);
            }else
            {
                new_print_Matrix(matrix,oponent_username,your_username);
            }
            

        }else if(strcmp(message,"wait") == 0)
        {
            
            printf(" wait for your turn\n");
        
            #if DEBUG
                printf("Message from server: %s\n",message);
            #endif

        }else if(strcmp(message,"initialize") == 0)
        {
        }else if(strcmp(message,"quit") == 0)
        {
            printf("%s quited the game... YOU WON CONGRATZZ!!!\n",oponent_username);
            break;
        }else if(strcmp(message,"rematch") == 0)
        {
            // rematch
            char respone[3];
            printf("Type yes for a rematch or no: ");
            while(scanf("%s",respone) != 1);

            if(strcmp(respone,"yes") == 0)
            {
                sendMessageToServer(client_fd,"yes");
            }
            else
            {
                sendMessageToServer(client_fd,"no");
            }

            
        }else if(strcmp(message,"draw") == 0)
        {
            // draw
            char respone[3];
            printf("%s wants to draw:Type yes for a draw or no: \n",oponent_username);
            while(scanf("%s",respone) != 1);

            if(strcmp(respone,"yes") == 0)
            {
                sendMessageToServer(client_fd,"yes");
            }
            else
            {
                sendMessageToServer(client_fd,"no");
            }

            
        }else if(strcmp(message,"PrintDraw") ==0)
        {
            printf("Game Ended in Draw \n");
        }else if(strcmp(message,"PrintWin") ==0)
        {
            printf("You Won the Game \n");
        }else if(strcmp(message,"PrintLose") ==0)
        {
            printf("You Lost the Game \n");
        }else if(strcmp(message,"whiteCheck") ==0)
        {
            printf("White King in Check\n");
        }else if(strcmp(message,"blackCheck")==0)
        {
            printf("Black King in Check\n");
        }
    
    }
    
    return 0;
}
