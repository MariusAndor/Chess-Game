#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<regex.h>
#include<ctype.h>
#include<time.h>
#include<ncurses.h>
#include"GUI.h"

#define ROW 8
#define COLUMN 8
#define STRING_SIZE 512

//#define DEBUG

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

typedef struct 
{
  char username[STRING_SIZE];
  unsigned int wins;
  bool isWhite;
  
}Client;


typedef struct 
{
  Element_T *Matrix;
  Client client1;
  Client client2;
  bool is_white_king_checked;
  bool is_black_king_checked;
  bool has_white_king_moved;
  bool has_black_king_moved;
  bool game_is_running;
  bool game_ended_in_draw;
  bool game_ended_in_white_won;
  unsigned int rounds_played;
  unsigned short int number_of_moves_played;
  char previous_move[STRING_SIZE];

}Game_T;

unsigned int min(unsigned int a,unsigned int b)
{
  if(a>=b)
    return b;
  else
    return a;
}

unsigned int max(unsigned int a,unsigned int b)
{
  if(a<=b)
    return b;
  else
    return a;
}

bool check_position_in_bounds(unsigned int row,unsigned int column)
{
  if(row<0 || row>=ROW || column<0 || column>=COLUMN)
     return false;
  else
     return true;
}

Element_T *Initialize_Classic_Game_Matrix()
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


Game_T *Initialize_Classic_Game()
{
  Game_T* game=(Game_T*)malloc(sizeof(Game_T));
  if(game==NULL)
    {
       perror("ERROR ALLOCATION GAME");
       exit(3);
    }

  game->Matrix=Initialize_Classic_Game_Matrix();
  game->number_of_moves_played=0;
  game->is_black_king_checked=false;
  game->is_white_king_checked=false;
  game->has_white_king_moved=false;
  game->has_black_king_moved=false;
  game->game_is_running=true;
  game->game_ended_in_draw=false;
  game->game_ended_in_white_won=false;
  game->rounds_played=0;

  strcpy(game->previous_move,"undefined");
  strcpy(game->client1.username,"bot1");
  strcpy(game->client2.username,"bot2");
  game->client1.isWhite=true;
  game->client2.isWhite=false;
  game->client1.wins=0;
  game->client2.wins=0;

  return game;
}

void print_Matrix(Element_T *Matrix)
{
    short unsigned int i = 0, j = 0;

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
}

unsigned int convert_character_to_column(char c)
{
  unsigned int collumn=0;
  switch (c)
  {
  case 'a':
    collumn=0;
    break;
  case 'b':
    collumn=1;
    break;
  case 'c':
    collumn=2;
    break;
  case 'd':
    collumn=3;
    break;
  case 'e':
    collumn=4;
    break;
  case 'f':
    collumn=5;
    break;
  case 'g':
    collumn=6;
    break;
  case 'h':
    collumn=7;
    break;
  }
  return collumn;
}
bool Validate_Semantically(const char *move)
{
    if(*move == 0)
      return false;


    regex_t regex;
    const char *pattern = "^\\([a-hA-H][1-8][a-hA-H][1-8]\\|resign\\|draw\\|[YyNn]\\)$";

    int value = regcomp(&regex,pattern,0);
    if(value!=0)
      {
        perror("REGEX COMP FAILED");
        exit(-7);
      }

    value=regexec(&regex,move,0,NULL,0);

    regfree(&regex);

    if(value==0)
      return true;

    return false;
}
bool Validate_Piece_Logic(Element_T* Matrix,const char *move)
{
    unsigned short int i=0;
    unsigned int column_start    =convert_character_to_column(tolower(move[0]));
    unsigned int column_end      =convert_character_to_column(tolower(move[2]));
    unsigned int row_start=0;
    unsigned int row_end=0;
    sscanf(move,"%*c%d%*c%d",&row_start,&row_end);
    row_start = 8 - row_start;
    row_end = 8 - row_end;   
    unsigned int pozition_start=row_start*ROW+column_start;
    unsigned int pozition_end=row_end*ROW+column_end;

    unsigned short int row_diff = abs(row_start - row_end);
    unsigned short int col_diff = abs(column_start - column_end);

     switch(Matrix[pozition_start].name)
    {
      case ROOK: 
        if (row_start!=row_end&&column_start!=column_end) 
        {
          return false;
        }

        if (column_start==column_end) 
        {
           int min_r=min(row_start, row_end);
           int max_r=max(row_start, row_end);
           
           for (i=min_r+1;i<max_r;i++) 
           {
             if (Matrix[i*ROW+column_start].name != Empty) 
             {
               return false; 
             }
           }
        }
        else 
          if (row_start == row_end) 
          { 
           int min_c = min(column_start, column_end);
           int max_c = max(column_start, column_end);
          
           for (i=min_c+1;i<max_c;i++) 
           { 
             if (Matrix[row_start*ROW+i].name != Empty) 
             {
               return false; 
             }
           }
        }
      
        if (Matrix[pozition_end].name != Empty && Matrix[pozition_end].isWhite == Matrix[pozition_start].isWhite) 
        {
            return false;
        }

        return true;
        break;

      case KNIGHT:

        if((column_end==column_start+1 &&(row_end==row_start+2 ||row_end==row_start-2)) 
        || (column_end==column_start-1 &&(row_end==row_start+2 ||row_end==row_start-2))
        || (column_end==column_start+2 &&(row_end==row_start+1 ||row_end==row_start-1)) 
        || (column_end==column_start-2 &&(row_end==row_start+1 ||row_end==row_start-1))
          )
        {
          return true;
        }
          return false;
        break;

      case KING:
          if((abs(column_start-column_end)>1) || (abs(row_start-row_end)>1))
            return false;
          if(Matrix[pozition_end].name!=Empty && Matrix[pozition_end].isWhite==Matrix[pozition_start].isWhite)
            return false;
        return true;
        break;
      
      case PAWN:
         if(Matrix[pozition_start].isWhite==0)
            {
              if(row_start==1 && row_end==3 && Matrix[(row_start+1)*ROW+column_start].name==Empty && Matrix[row_end*ROW+column_end].name==Empty)
                return true;
              if((row_end-row_start)==1 && Matrix[row_end*ROW+column_end].name==Empty && column_start==column_end)
                return true;
              if((row_end-row_start)==1 && abs(column_start-column_end)==1 && Matrix[row_end*ROW+column_end].name!=Empty && Matrix[row_end*ROW+column_end].isWhite==1)
                 return true;
              return false;
            }
         else
            {
              if(row_start==6 && row_end==4 && Matrix[(row_start-1)*ROW+column_start].name==Empty && Matrix[row_end*ROW+column_end].name==Empty)
                return true;
              if((row_start-row_end)==1 && Matrix[row_end*ROW+column_end].name==Empty && column_start==column_end)
                return true;
              if((row_start-row_end)==1 && abs(column_start-column_end)==1 && Matrix[row_end*ROW+column_end].name!=Empty && Matrix[row_end*ROW+column_end].isWhite==0)
                 return true;
              return false;              
            }
        break;

      case BISHOP:      
            //it is a diagonal move iff |row_diff|=|col_diff|
            if(row_diff!=col_diff)
              return false;
            else
               {
                 //check if there are no pieces in between positions
                 int row_dir, col_dir;
                 if (row_end > row_start) 
                   {row_dir = 1;}
                 else
                    {row_dir = -1;}

                 if (column_end > column_start) 
                   {col_dir = 1;}
                 else
                   {col_dir = -1;}
                
                int current_row = row_start + row_dir;
                int current_column = column_start + col_dir;
                while (current_row != row_end) 
                {
                    if(Matrix[current_row*ROW+current_column].name!=Empty)
                      {
                        return false;
                      }
                    current_row += row_dir;
                    current_column += col_dir;
                }

                if (Matrix[pozition_end].name != Empty && Matrix[pozition_end].isWhite == Matrix[pozition_start].isWhite) 
                {
                  return false;
                }
               }
            return true;
            break;

      case QUEEN:
        //check if it is a bishop or a rook move then check  if the path is clear
        //it is a diagonal move iff |row_diff|=|col_diff|
        if(row_diff==col_diff)
          {
              //bishop path
                 int row_dir, col_dir;
                 if (row_end > row_start) 
                   {row_dir = 1;}
                 else
                    {row_dir = -1;}

                 if (column_end > column_start) 
                   {col_dir = 1;}
                 else
                   {col_dir = -1;}
                
                int current_row = row_start + row_dir;
                int current_column = column_start + col_dir;
                while (current_row != row_end) 
                {
                    if(Matrix[current_row*ROW+current_column].name!=Empty)
                      {
                        return false;
                      }
                    current_row += row_dir;
                    current_column += col_dir;
                }
                if (Matrix[pozition_end].name != Empty && Matrix[pozition_end].isWhite == Matrix[pozition_start].isWhite) 
                {
                  return false;
                }

                return true;
          }
      else
        {
         if(row_start==row_end || column_start==column_end)
         {
           if(column_start==column_end)
            {
                for(i=min(row_start,row_end)+1;i<max(row_start,row_end);i++)
                 {
                    if(Matrix[i*ROW+column_start].name!=Empty)
                    {
                        return false; 
                    }
                 }
                if (Matrix[pozition_end].name != Empty && Matrix[pozition_end].isWhite == Matrix[pozition_start].isWhite) 
                {
                  return false;
                }
              return true;
           }
           if(row_start==row_end)
           {
            for(i=min(column_start,column_end)+1;i<max(column_end,column_start);i++)
              {
                if(Matrix[row_start*ROW+i].name!=Empty)
                  {
                    return false; 
                  }
                if (Matrix[pozition_end].name != Empty && Matrix[pozition_end].isWhite == Matrix[pozition_start].isWhite) 
                {
                  return false;
                }
              }
            return true;
           }
         }
        }
        return false;
        break;

    case Empty:
         return false;
    }
  
  return true;
    
}

bool is_black_in_check(Element_T* Matrix)
{
   bool return_checked_state=true;
   unsigned int i=0;
   unsigned int j=0;
   unsigned int index=0;
   unsigned int row_king=0;
   unsigned int column_king=0;

   for(i=0;i<ROW;i++)
     {
      for(j=0;j<COLUMN;j++)
        {
          if(Matrix[i*ROW+j].isWhite==false && Matrix[i*ROW+j].name==KING)
             {
              row_king=i;
              column_king=j;
              break;             
             }
        }
     }
   
   for(i=0;i<ROW;i++)
    {
      for(j=0;j<COLUMN;j++)
        {
          //ROOK
          if(Matrix[i*ROW+j].isWhite==true && Matrix[i*ROW+j].name==ROOK )
            {
              if(row_king==i)
                {
                  return_checked_state=true;
                  for(index=min(column_king,j)+1;index<max(column_king,j);index++)
                     {
                        if(Matrix[row_king*ROW+index].name!=Empty)
                           return_checked_state=false;
                     }
                  if(return_checked_state==true)
                    return true;
                }
              if(column_king==j)
                 {
                  return_checked_state=true;
                  for(index=min(row_king,i)+1;index<max(row_king,i);index++)
                     {
                        if(Matrix[index*ROW+column_king].name!=Empty)
                           return_checked_state=false;
                     }
                  if(return_checked_state==true)
                    return true;
                }
            }
          //PAWN
          if(Matrix[i*ROW+j].isWhite==true && Matrix[i*ROW+j].name==PAWN)
            {
                if(row_king==(i-1))
                  {
                    if(column_king==(j+1) || column_king==(j-1) )
                      return true;
                  }
            }
          //KNIGHT
          if(Matrix[i*ROW+j].isWhite==true && Matrix[i*ROW+j].name==KNIGHT)
            {
              if((column_king==j+1 &&(row_king==i+2 ||row_king==i-2)) 
              || (column_king==j-1 &&(row_king==i+2 ||row_king==i-2))
              || (column_king==j+2 &&(row_king==i+1 ||row_king==i-1)) 
              || (column_king==j-2 &&(row_king==i+1 ||row_king==i-1))
                )
                {
                  return true;
                }
            }
          //BISHOP  
          if(Matrix[i*ROW+j].isWhite==true && Matrix[i*ROW+j].name==BISHOP)
            {
              unsigned short int row_diff = abs(i-row_king);
              unsigned short int col_diff = abs(j-column_king);
              if(row_diff==col_diff)
                {
                  int row_dir, col_dir;
                  if (row_king > i) 
                     {row_dir = 1;}
                  else
                    {row_dir = -1;}

                  if (column_king > j) 
                   {col_dir = 1;}
                  else
                   {col_dir = -1;}
                
                  int current_row = i + row_dir;
                  int current_column = j + col_dir;
                  return_checked_state=true;
                  while (current_row != row_king) 
                  {
                    if(Matrix[current_row*ROW+current_column].name!=Empty)
                      {
                        return_checked_state=false;
                      }
                    current_row += row_dir;
                    current_column += col_dir;
                  }
                  if(return_checked_state==true)
                    {
                      return true;
                    }
                }
            }
          //QUEEN
          if(Matrix[i*ROW+j].isWhite==true && Matrix[i*ROW+j].name==QUEEN)
            {
              //rook path
              if(row_king==i)
                {
                  return_checked_state=true;
                  for(index=min(column_king,j)+1;index<max(column_king,j);index++)
                     {
                        if(Matrix[row_king*ROW+index].name!=Empty)
                           return_checked_state=false;
                     }
                  if(return_checked_state==true)
                    return true;
                }
              if(column_king==j)
                 {
                  return_checked_state=true;
                  for(index=min(row_king,i)+1;index<max(row_king,i);index++)
                     {
                        if(Matrix[index*ROW+column_king].name!=Empty)
                           return_checked_state=false;
                     }
                  if(return_checked_state==true)
                    return true;
                }
              //bishop path
              unsigned short int row_diff = abs(i-row_king);
              unsigned short int col_diff = abs(j-column_king);
              if(row_diff==col_diff)
                {
                  int row_dir, col_dir;
                  if (row_king > i) 
                     {row_dir = 1;}
                  else
                    {row_dir = -1;}

                  if (column_king > j) 
                   {col_dir = 1;}
                  else
                   {col_dir = -1;}
                
                  int current_row = i + row_dir;
                  int current_column = j + col_dir;
                  return_checked_state=true;
                  while (current_row != row_king) 
                  {
                    if(Matrix[current_row*ROW+current_column].name!=Empty)
                      {
                        return_checked_state=false;
                      }
                    current_row += row_dir;
                    current_column += col_dir;
                  }
                  if(return_checked_state==true)
                    {
                      return true;
                    }
                }
            }
        }
    }
  return false;
}

bool is_white_in_check(Element_T* Matrix)
{
   bool return_checked_state=true;
   unsigned int i=0;
   unsigned int j=0;
   unsigned int index=0;
   unsigned int row_king=0;
   unsigned int column_king=0;

   for(i=0;i<ROW;i++)
     {
      for(j=0;j<COLUMN;j++)
        {
          //find kings position
          if(Matrix[i*ROW+j].isWhite==true && Matrix[i*ROW+j].name==KING)
             {
              row_king=i;
              column_king=j;
              break;             
             }
        }
     }
   
   for(i=0;i<ROW;i++)
    {
      for(j=0;j<COLUMN;j++)
        {
          //ROOK
          if(Matrix[i*ROW+j].isWhite==false && Matrix[i*ROW+j].name==ROOK )
            {
              if(row_king==i)
                {
                  return_checked_state=true;
                  for(index=min(column_king,j)+1;index<max(column_king,j);index++)
                     {
                        if(Matrix[row_king*ROW+index].name!=Empty)
                           return_checked_state=false;
                     }
                  if(return_checked_state==true)
                    return true;
                }
              if(column_king==j)
                 {
                  return_checked_state=true;
                  for(index=min(row_king,i)+1;index<max(row_king,i);index++)
                     {
                        if(Matrix[index*ROW+column_king].name!=Empty)
                           return_checked_state=false;
                     }
                  if(return_checked_state==true)
                    return true;
                }
            }
          //PAWN
          if(Matrix[i*ROW+j].isWhite==false && Matrix[i*ROW+j].name==PAWN)
            {
                if(row_king==(i+1))
                  {
                    if(column_king==(j+1) || column_king==(j-1) )
                      return true;
                  }
            }
          //KNIGHT
          if(Matrix[i*ROW+j].isWhite==false && Matrix[i*ROW+j].name==KNIGHT)
            {
              if((column_king==j+1 &&(row_king==i+2 ||row_king==i-2)) 
              || (column_king==j-1 &&(row_king==i+2 ||row_king==i-2))
              || (column_king==j+2 &&(row_king==i+1 ||row_king==i-1)) 
              || (column_king==j-2 &&(row_king==i+1 ||row_king==i-1))
                )
                {
                  return true;
                }
            }
          //BISHOP  
          if(Matrix[i*ROW+j].isWhite==false && Matrix[i*ROW+j].name==BISHOP)
            {
              unsigned short int row_diff = abs(i-row_king);
              unsigned short int col_diff = abs(j-column_king);
              if(row_diff==col_diff)
                {
                  int row_dir, col_dir;
                  if (row_king > i) 
                     {row_dir = 1;}
                  else
                    {row_dir = -1;}

                  if (column_king > j) 
                   {col_dir = 1;}
                  else
                   {col_dir = -1;}
                
                  int current_row = i + row_dir;
                  int current_column = j + col_dir;
                  return_checked_state=true;
                  while (current_row != row_king) 
                  {
                    if(Matrix[current_row*ROW+current_column].name!=Empty)
                      {
                        return_checked_state=false;
                      }
                    current_row += row_dir;
                    current_column += col_dir;
                  }
                  if(return_checked_state==true)
                    {
                      return true;
                    }
                }
            }
          if(Matrix[i*ROW+j].isWhite==false && Matrix[i*ROW+j].name==QUEEN)
            {
              //rook path
              if(row_king==i)
                {
                  return_checked_state=true;
                  for(index=min(column_king,j)+1;index<max(column_king,j);index++)
                     {
                        if(Matrix[row_king*ROW+index].name!=Empty)
                           return_checked_state=false;
                     }
                  if(return_checked_state==true)
                    return true;
                }
              if(column_king==j)
                 {
                  return_checked_state=true;
                  for(index=min(row_king,i)+1;index<max(row_king,i);index++)
                     {
                        if(Matrix[index*ROW+column_king].name!=Empty)
                           return_checked_state=false;
                     }
                  if(return_checked_state==true)
                    return true;
                }
              //bishop path
              unsigned short int row_diff = abs(i-row_king);
              unsigned short int col_diff = abs(j-column_king);
              if(row_diff==col_diff)
                {
                  int row_dir, col_dir;
                  if (row_king > i) 
                     {row_dir = 1;}
                  else
                    {row_dir = -1;}

                  if (column_king > j) 
                   {col_dir = 1;}
                  else
                   {col_dir = -1;}
                
                  int current_row = i + row_dir;
                  int current_column = j + col_dir;
                  return_checked_state=true;
                  while (current_row != row_king) 
                  {
                    if(Matrix[current_row*ROW+current_column].name!=Empty)
                      {
                        return_checked_state=false;
                      }
                    current_row += row_dir;
                    current_column += col_dir;
                  }
                  if(return_checked_state==true)
                    {
                      return true;
                    }
                }
            }
        }
    }
  return false;
}

Game_T*Apply_Move_Matrix(Game_T*Game,const char* move)
{
    unsigned int column_start    =convert_character_to_column(tolower(move[0]));
    unsigned int column_end      =convert_character_to_column(tolower(move[2]));
    unsigned int row_start;
    unsigned int row_end;
    sscanf(move,"%*c%d%*c%d",&row_start,&row_end);
    row_start = 8 - row_start;
    row_end = 8 - row_end;   
    unsigned int pozition_start=row_start*ROW+column_start;
    unsigned int pozition_end=row_end*ROW+column_end;

    Game->Matrix[pozition_end].name=Game->Matrix[pozition_start].name;
    Game->Matrix[pozition_end].isWhite=Game->Matrix[pozition_start].isWhite;
    Game->Matrix[pozition_start].name=Empty;

    strcpy(Game->previous_move,move);
    return Game;
}

unsigned short int calculate_available_positions(Element_T *Matrix,unsigned int* available_positions_for_a_piece,const unsigned int row_start,const unsigned int column_start)
{
  unsigned int i=0;
  unsigned int j=0;
  unsigned short int size=0;
  switch(Matrix[row_start*ROW+column_start].name)
  {
  case KNIGHT:
      if(check_position_in_bounds(row_start+2,column_start+1) && (Matrix[(row_start+2)*ROW+column_start+1].name==Empty || Matrix[(row_start+2)*ROW+column_start+1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start+2)*ROW+column_start+1;
      if(check_position_in_bounds(row_start+2,column_start-1) && (Matrix[(row_start+2)*ROW+column_start-1].name==Empty || Matrix[(row_start+2)*ROW+column_start-1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start+2)*ROW+column_start-1;
      if(check_position_in_bounds(row_start-2,column_start+1) && (Matrix[(row_start-2)*ROW+column_start+1].name==Empty || Matrix[(row_start-2)*ROW+column_start+1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start-2)*ROW+column_start+1;
      if(check_position_in_bounds(row_start-2,column_start-1) && (Matrix[(row_start-2)*ROW+column_start-1].name==Empty || Matrix[(row_start-2)*ROW+column_start-1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start-2)*ROW+column_start-1;
      if(check_position_in_bounds(row_start+1,column_start+2) && (Matrix[(row_start+1)*ROW+column_start+2].name==Empty || Matrix[(row_start+1)*ROW+column_start+2].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start+1)*ROW+column_start+2;
      if(check_position_in_bounds(row_start+1,column_start-2) && (Matrix[(row_start+1)*ROW+column_start-2].name==Empty || Matrix[(row_start+1)*ROW+column_start-2].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start+1)*ROW+column_start-2;
      if(check_position_in_bounds(row_start-1,column_start+2) && (Matrix[(row_start-1)*ROW+column_start+2].name==Empty || Matrix[(row_start-1)*ROW+column_start+2].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start-1)*ROW+column_start+2;
      if(check_position_in_bounds(row_start-1,column_start-2) && (Matrix[(row_start-1)*ROW+column_start-2].name==Empty || Matrix[(row_start-1)*ROW+column_start-2].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start-1)*ROW+column_start-2;
    break;

  case KING:
      if(check_position_in_bounds(row_start,column_start+1) && (Matrix[(row_start)*ROW+column_start+1].name==Empty || Matrix[(row_start)*ROW+column_start+1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start)*ROW+column_start+1;
      if(check_position_in_bounds(row_start,column_start-1) && (Matrix[(row_start)*ROW+column_start-1].name==Empty || Matrix[(row_start)*ROW+column_start-1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start)*ROW+column_start-1;
      if(check_position_in_bounds(row_start+1,column_start-1) && (Matrix[(row_start+1)*ROW+column_start-1].name==Empty || Matrix[(row_start+1)*ROW+column_start-1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start+1)*ROW+column_start-1;
      if(check_position_in_bounds(row_start+1,column_start) && (Matrix[(row_start+1)*ROW+column_start].name==Empty || Matrix[(row_start+1)*ROW+column_start].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start+1)*ROW+column_start;
      if(check_position_in_bounds(row_start+1,column_start+1) && (Matrix[(row_start+1)*ROW+column_start+1].name==Empty || Matrix[(row_start+1)*ROW+column_start+1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start+1)*ROW+column_start+1;
      if(check_position_in_bounds(row_start-1,column_start-1) && (Matrix[(row_start-1)*ROW+column_start-1].name==Empty || Matrix[(row_start-1)*ROW+column_start-1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start-1)*ROW+column_start-1;
      if(check_position_in_bounds(row_start-1,column_start) && (Matrix[(row_start-1)*ROW+column_start].name==Empty || Matrix[(row_start-1)*ROW+column_start].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start-1)*ROW+column_start;
      if(check_position_in_bounds(row_start-1,column_start+1) && (Matrix[(row_start-1)*ROW+column_start+1].name==Empty || Matrix[(row_start-1)*ROW+column_start+1].isWhite!=Matrix[row_start*ROW+column_start].isWhite))
        available_positions_for_a_piece[size++]=(row_start-1)*ROW+column_start+1;
    break;

  case PAWN:
      if(Matrix[row_start*ROW+column_start].isWhite==true)
        {
            if(check_position_in_bounds(row_start-1,column_start+1) && Matrix[(row_start-1)*ROW+column_start+1].isWhite==false && Matrix[(row_start-1)*ROW+column_start+1].name!=Empty)
               available_positions_for_a_piece[size++]=(row_start-1)*ROW+column_start+1;
            if(check_position_in_bounds(row_start-1,column_start-1) && Matrix[(row_start-1)*ROW+column_start-1].isWhite==false && Matrix[(row_start-1)*ROW+column_start-1].name!=Empty)
               available_positions_for_a_piece[size++]=(row_start-1)*ROW+column_start-1;
            if(check_position_in_bounds(row_start-1,column_start) && Matrix[(row_start-1)*ROW+column_start].name==Empty)
               available_positions_for_a_piece[size++]=(row_start-1)*ROW+column_start;
            if(row_start==6 && Matrix[(row_start-1)*ROW+column_start].name==Empty && Matrix[(row_start-2)*ROW+column_start].name==Empty)
               available_positions_for_a_piece[size++]=(row_start-2)*ROW+column_start;
        }
      else
        {
            if(check_position_in_bounds(row_start+1,column_start+1) && Matrix[(row_start+1)*ROW+column_start+1].isWhite==true && Matrix[(row_start+1)*ROW+column_start+1].name!=Empty)
               available_positions_for_a_piece[size++]=(row_start+1)*ROW+column_start+1;
            if(check_position_in_bounds(row_start+1,column_start-1) && Matrix[(row_start+1)*ROW+column_start-1].isWhite==true && Matrix[(row_start+1)*ROW+column_start-1].name!=Empty)
               available_positions_for_a_piece[size++]=(row_start+1)*ROW+column_start-1;
            if(check_position_in_bounds(row_start+1,column_start)   && Matrix[(row_start+1)*ROW+column_start].name==Empty)
               available_positions_for_a_piece[size++]=(row_start+1)*ROW+column_start;
            if(row_start==1 && Matrix[(row_start+1)*ROW+column_start].name==Empty && Matrix[(row_start+2)*ROW+column_start].name==Empty)
               available_positions_for_a_piece[size++]=(row_start+2)*ROW+column_start;
        }
    break;
  case ROOK:
      for(i=row_start+1;i<ROW;i++)
        {
          if(check_position_in_bounds(i,column_start)==false)
            break;
          if(Matrix[i*ROW+column_start].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+column_start;
            }
          if(Matrix[i*ROW+column_start].name!=Empty && Matrix[i*ROW+column_start].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+column_start;
              break;
            }
          if(Matrix[i*ROW+column_start].name!=Empty && Matrix[i*ROW+column_start].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      for(i=row_start-1;i>=0;i--)
        {
          if(check_position_in_bounds(i,column_start)==false)
            break;
          if(Matrix[i*ROW+column_start].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+column_start;
            }
           if(Matrix[i*ROW+column_start].name!=Empty && Matrix[i*ROW+column_start].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+column_start;
              break;
            }
            if(Matrix[i*ROW+column_start].name!=Empty && Matrix[i*ROW+column_start].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      for(j=column_start+1;j<COLUMN;j++)
        {
          if(check_position_in_bounds(row_start,j)==false)
            break;
          if(Matrix[row_start*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=row_start*ROW+j;
            }
          if(Matrix[row_start*ROW+j].name!=Empty && Matrix[row_start*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=row_start*ROW+j;
              break;
            }
          if(Matrix[row_start*ROW+j].name!=Empty && Matrix[row_start*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }     
        }
      for(j=column_start-1;j>=0;j--)
        {
          if(check_position_in_bounds(row_start,j)==false)
            break;
          if(Matrix[row_start*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=row_start*ROW+j;
            }
          if(Matrix[row_start*ROW+j].name!=Empty && Matrix[row_start*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=row_start*ROW+j;
              break;
            }
          if(Matrix[row_start*ROW+j].name!=Empty && Matrix[row_start*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }  
    break;
  case QUEEN:
    //bishop path
      j=column_start;
      for(i=row_start+1;i<ROW;i++)
        {
          j++;
          if(check_position_in_bounds(i,j)==false)
            break;
          if(Matrix[i*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
              break;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      j=column_start;
      for(i=row_start+1;i<ROW;i++)
        {
          j--;
          if(check_position_in_bounds(i,j)==false)
            break;            
          if(Matrix[i*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
              break;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      i=row_start;
      for(j=column_start+1;j<COLUMN;j++)
        {
          i--;
          if(check_position_in_bounds(i,j)==false)
            break;
          if(Matrix[i*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
              break;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      i=row_start;
      for(j=column_start-1;j>=0;j--)
        {
          i--;
          if(check_position_in_bounds(i,j)==false)
            break;
          if(Matrix[i*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
              break;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      //rook path
      for(i=row_start+1;i<ROW;i++)
        {
          if(check_position_in_bounds(i,column_start)==false)
            break;
          if(Matrix[i*ROW+column_start].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+column_start;
            }
          if(Matrix[i*ROW+column_start].name!=Empty && Matrix[i*ROW+column_start].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+column_start;
              break;
            }
          if(Matrix[i*ROW+column_start].name!=Empty && Matrix[i*ROW+column_start].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      for(i=row_start-1;i>=0;i--)
        {
          if(check_position_in_bounds(i,column_start)==false)
            break;
          if(Matrix[i*ROW+column_start].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+column_start;
            }
           if(Matrix[i*ROW+column_start].name!=Empty && Matrix[i*ROW+column_start].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+column_start;
              break;
            }
            if(Matrix[i*ROW+column_start].name!=Empty && Matrix[i*ROW+column_start].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      for(j=column_start+1;j<COLUMN;j++)
        {
          if(check_position_in_bounds(row_start,j)==false)
            break;
          if(Matrix[row_start*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=row_start*ROW+j;
            }
          if(Matrix[row_start*ROW+j].name!=Empty && Matrix[row_start*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=row_start*ROW+j;
              break;
            }
          if(Matrix[row_start*ROW+j].name!=Empty && Matrix[row_start*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }     
        }
      for(j=column_start-1;j>=0;j--)
        {
          if(check_position_in_bounds(row_start,j)==false)
            break;
          if(Matrix[row_start*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=row_start*ROW+j;
            }
          if(Matrix[row_start*ROW+j].name!=Empty && Matrix[row_start*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=row_start*ROW+j;
              break;
            }
          if(Matrix[row_start*ROW+j].name!=Empty && Matrix[row_start*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }  
    break;
  case BISHOP:
      j=column_start;
      for(i=row_start+1;i<ROW;i++)
        {
          j++;
          if(check_position_in_bounds(i,j)==false)
            break;
          if(Matrix[i*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
              break;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      j=column_start;
      for(i=row_start+1;i<ROW;i++)
        {
          j--;
          if(check_position_in_bounds(i,j)==false)
            break;            
          if(Matrix[i*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
              break;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      i=row_start;
      for(j=column_start+1;j<COLUMN;j++)
        {
          i--;
          if(check_position_in_bounds(i,j)==false)
            break;
          if(Matrix[i*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
              break;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
      i=row_start;
      for(j=column_start-1;j>=0;j--)
        {
          i--;
          if(check_position_in_bounds(i,j)==false)
            break;
          if(Matrix[i*ROW+j].name==Empty)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite!=Matrix[row_start*ROW+column_start].isWhite)
            {
              available_positions_for_a_piece[size++]=i*ROW+j;
              break;
            }
          if(Matrix[i*ROW+j].name!=Empty && Matrix[i*ROW+j].isWhite==Matrix[row_start*ROW+column_start].isWhite)
            {
              break;
            }
        }
    break;

  default:
    break;
  }

  return size;
}

void print_posible_moves(unsigned int size,unsigned int *vector)
{
  unsigned int i=0;
  for(i=0;i<size;i++)
    printf("r:%d c:%d\n",vector[i]/ROW,vector[i]%COLUMN);
}

bool Check_Moves_Available_White(Game_T*Game)
{
  Element_T *Matrix_Temp=malloc(sizeof(Element_T)*ROW*COLUMN);
  if(Matrix_Temp==NULL)
    {
      endwin();
      perror("TEMP MATRIX ALLOCATION FAILED");
      exit(-1);
    }
  unsigned int* available_positions_for_a_piece=malloc(sizeof(unsigned int)*ROW*COLUMN);
  if(available_positions_for_a_piece==NULL)
    {
      endwin();
      perror("AVAILABLE POSTION VECTOR ALLOCATION FAILED");
      exit(-1);
    }
  unsigned short int size_positions;
  unsigned int i=0;
  unsigned int j=0;
  for(i=0;i<ROW;i++)
    {
      for(j=0;j<COLUMN;j++)
        {
          Matrix_Temp[i*ROW+j]=Game->Matrix[i*ROW+j];
        }
    }

  for(i=0;i<ROW;i++)
    {
      for(j=0;j<COLUMN;j++)
        {
          if(Matrix_Temp[i*ROW+j].isWhite==true && Matrix_Temp[i*ROW+j].name!=Empty)
            {
              unsigned int index=0;
              size_positions=calculate_available_positions(Matrix_Temp,available_positions_for_a_piece,i,j);
              Element_T to_be_captured;
              Element_T capturer;
              for(index=0;index<size_positions;index++)
                  { 
                    to_be_captured=Matrix_Temp[available_positions_for_a_piece[index]]; //save previous move
                    capturer=Matrix_Temp[i*ROW+j];

                    Matrix_Temp[available_positions_for_a_piece[index]]=Matrix_Temp[i*ROW+j]; //make move
                    Matrix_Temp[i*ROW+j].name=Empty;

                    if(is_white_in_check(Matrix_Temp)==false)  //check after move if it is in check
                      {
                        free(available_positions_for_a_piece);
                        free(Matrix_Temp);
                        return true;
                      }

                    Matrix_Temp[available_positions_for_a_piece[index]]=to_be_captured;
                    Matrix_Temp[i*ROW+j]=capturer; //rollback move
                    }
             }
        }
    }

  free(available_positions_for_a_piece);
  free(Matrix_Temp);
  return false;  
}

bool Check_Moves_Available_Black(Game_T*Game)
{
  Element_T *Matrix_Temp=malloc(sizeof(Element_T)*ROW*COLUMN);
  if(Matrix_Temp==NULL)
    {
      endwin();
      perror("TEMP MATRIX ALLOCATION FAILED");
      exit(-1);
    }
  unsigned int* available_positions_for_a_piece=malloc(sizeof(unsigned int)*ROW*COLUMN);
  if(available_positions_for_a_piece==NULL)
    {
      endwin();
      perror("AVAILABLE POSTION VECTOR ALLOCATION FAILED");
      exit(-1);
    }
  unsigned short int size_positions;
  unsigned int i=0;
  unsigned int j=0;
  for(i=0;i<ROW;i++)
    {
      for(j=0;j<COLUMN;j++)
        {
          Matrix_Temp[i*ROW+j]=Game->Matrix[i*ROW+j];
        }
    }

  for(i=0;i<ROW;i++)
    {
      for(j=0;j<COLUMN;j++)
        {
          if(Matrix_Temp[i*ROW+j].isWhite==false && Matrix_Temp[i*ROW+j].name!=Empty)
            {
              unsigned int index=0;
              size_positions=calculate_available_positions(Matrix_Temp,available_positions_for_a_piece,i,j);
              Element_T to_be_captured;
              Element_T capturer;
              for(index=0;index<size_positions;index++)
                    {
                    to_be_captured=Matrix_Temp[available_positions_for_a_piece[index]]; //save previous move
                    capturer=Matrix_Temp[i*ROW+j];

                    Matrix_Temp[available_positions_for_a_piece[index]]=Matrix_Temp[i*ROW+j];
                    Matrix_Temp[i*ROW+j].name=Empty;

                    if(is_black_in_check(Matrix_Temp)==false)  //make move than verify check state
                      {
                        free(available_positions_for_a_piece);
                        free(Matrix_Temp);
                        return true;
                      }

                    Matrix_Temp[available_positions_for_a_piece[index]]=to_be_captured; //rollback move
                    Matrix_Temp[i*ROW+j]=capturer;
                  }
            }
        }
     }

  free(available_positions_for_a_piece);
  free(Matrix_Temp);
  return false;  
}

bool Check_Sufficient_Material(Element_T *Matrix)
{
  unsigned short int types_of_pieces=6;//PAWN,KNIGHT,BISHOP,QUEEN,KING,ROOK
  unsigned short int number_of_pieces_white[types_of_pieces];
  unsigned short int number_of_pieces_black[types_of_pieces];
  memset(number_of_pieces_white,0,sizeof(number_of_pieces_white));
  memset(number_of_pieces_black,0,sizeof(number_of_pieces_black));

  unsigned short int i=0;
  unsigned short int j=0;
  for(i=0;i<ROW;i++)
    {
      for(j=0;j<COLUMN;j++)
        {
           if(Matrix[i*ROW+j].name!=Empty)
             {
              if(Matrix[i*ROW+j].isWhite==true)
                {
                  number_of_pieces_white[Matrix[i*ROW+j].name]++;
                }
              else
                {
                  number_of_pieces_black[Matrix[i*ROW+j].name]++;
                }
             }
        }
    }
  //king +bishop ;king +1knight ;king+2knight ;just king =>insuficient material ,hence a draw
  if(number_of_pieces_white[PAWN]==0 && number_of_pieces_black[PAWN]==0 && number_of_pieces_black[ROOK]==0 && number_of_pieces_white[ROOK] && number_of_pieces_black[QUEEN]==0 && number_of_pieces_white[QUEEN]==0)
    {
        if(number_of_pieces_white[BISHOP]==0 && number_of_pieces_black[BISHOP]==0)
           return false; //king +1knight ;king+2knight ;just king
        if(number_of_pieces_white[BISHOP]==1 && number_of_pieces_white[KNIGHT]==0 && number_of_pieces_black[BISHOP]==1 && number_of_pieces_black[KNIGHT]==0)
           return false; //king +bishop
    }
  
  return true;
}

void Print_Game_Result(Game_T *Game)
{
	/*
  printf("\n");
  if(Game->game_ended_in_draw==true)
     {
      printf("Game ended in a draw!\n");
     }
  else
     {
      if(Game->game_ended_in_white_won==true)
        {
          if(Game->client1.isWhite==true)
            printf("%s won the game!",(Game->client1.username));
          else
            printf("%s won the game!",(Game->client2.username));
        }
      else
        {
          if(Game->client1.isWhite==false)
            printf("%s won the game!",(Game->client1.username));
          else
            printf("%s won the game!",(Game->client2.username));
        }
     }
  printf("\n");*/
  
    clear();  // clear window
  
    // get coordinates:
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    const int W = 8 * CELL_W, H = 8 * CELL_H;
    int oy = (rows - H) / 2; 
    if(oy < 2)
      {
	  	oy = 2;
	  }
	
    int ox = (cols - W) / 2;
    if(ox < 4) 
      {
	  	ox = 4;
	  }
	  
	const char *win_msg1[] = {
				"(     _    _ _     _ _                               _ )",
				"(    | |  | | |   (_) |                             | |)",
				"(    | |  | | |__  _| |_ ___  __      _____  _ __   | |)",
				"(    | |/\| | '_ \| | __/ _ \ \ \ /\ / / _ \| '_ \  | |)",
				"(    \  /\  / | | | | ||  __/  \ V  V / (_) | | | | |_|)",
				"(     \/  \/|_| |_|_|\__\___|   \_/\_/ \___/|_| |_| (_))"
				};
	
	const char *win_msg2[] = {
        "(    ______ _            _                            _  )",
				"(    | ___ \ |          | |                          | | )",
				"(    | |_/ / | __ _  ___| | __ __      _____  _ __   | | )",
				"(    | ___ \ |/ _` |/ __| |/ / \ \ /\ / / _ \| '_ \  | | )",
				"(    | |_/ / | (_| | (__|   <   \ V  V / (_) | | | | |_| )",
				"(    \____/|_|\__,_|\___|_|\_\   \_/\_/ \___/|_| |_| (_) )"
				};

				
	const char *draw[] = {
				"(     _____                                       _          _   _                     _                       )",
				"(    |  __ \                                     | |        | | (_)                   | |                      )",
				"(    | |  \/ __ _ _ __ ___   ___    ___ _ __   __| | ___  __| |  _ _ __     __ _    __| |_ __ __ ___      __   )",
				"(    | | __ / _` | '_ ` _ \ / _ \  / _ \ '_ \ / _` |/ _ \/ _` | | | '_ \   / _` |  / _` | '__/ _` \ \ /\ / /   )",
				"(    | |_\ \ (_| | | | | | |  __/ |  __/ | | | (_| |  __/ (_| | | | | | | | (_| | | (_| | | | (_| |\ V  V /  _ )",
				"(     \____/\__,_|_| |_| |_|\___|  \___|_| |_|\__,_|\___|\__,_| |_|_| |_|  \__,_|  \__,_|_|  \__,_| \_/\_/  (_))"
				};
	
	int msg_lines, start_row;
	if(Game->game_ended_in_white_won==true)
	{
		// white won
		msg_lines = sizeof(win_msg1) / sizeof(win_msg1[0]);
		start_row = (rows - msg_lines) / 2;	
		for(int i = 0; i < msg_lines; i++)
		{
			mvprintw(start_row + i, (cols - strlen(win_msg1[i])) / 2, "%s", win_msg1[i]);
		}
	}
	else if(Game->game_ended_in_draw != true)
	{
		// black (blue) won
		msg_lines = sizeof(win_msg2) / sizeof(win_msg2[0]);
		start_row = (rows - msg_lines) / 2;	
		for(int i = 0; i < msg_lines; i++)
		{
			mvprintw(start_row + i, (cols - strlen(win_msg2[i])) / 2, "%s", win_msg2[i]);
		}
	}
	else if(Game->game_ended_in_draw)
	{
		msg_lines = sizeof(draw) / sizeof(draw[0]);
		start_row = (rows - msg_lines) / 2;	
		for(int i = 0; i < msg_lines; i++)
		{
			mvprintw(start_row + i, (cols - strlen(draw[i])) / 2, "%s", draw[i]);
		}
	}
	
	refresh();
	
	mvprintw(44, 98, "%s", "Press any key to exit.");
	
	getch();
	endwin();
}

bool Game_Move(Game_T *Game,Client client,const char* move)
{
    if(move[0] == 0)
      return false;

    Element_T *Matrix=Game->Matrix;
 
    //move must be in the form "^\\([a-hA-H][1-8][a-hA-H][1-8]\\|resign\\|draw\\)$"
    if(Validate_Semantically(move)==false)
      {
        type_text_on_window(48, 106, "Invalid move!", 1000);
        napms(300);
        mvprintw(48, 106, "%s", "             ");
        return false;
      }
    
    if(strcmp(move,"resign")==0)
      {
        //One player may resign early on and the game ends
        if(client.isWhite==true)
          {
            Game->game_ended_in_draw=false;
            Game->game_ended_in_white_won=false;
          }
        else
          {
            Game->game_ended_in_draw=false;
            Game->game_ended_in_white_won=true; 
          }
        Game->game_is_running=false;
        return true;
      }

    unsigned int column_start    =convert_character_to_column(tolower(move[0]));
    unsigned int column_end      =convert_character_to_column(tolower(move[2]));
    unsigned int row_start=0;
    unsigned int row_end=0;
    sscanf(move,"%*c%d%*c%d",&row_start,&row_end);
    row_start = 8 - row_start;
    row_end = 8 - row_end;   
    unsigned int pozition_start=row_start*ROW+column_start;
    unsigned int pozition_end=row_end*ROW+column_end;


    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    const int W = 8 * CELL_W, H = 8 * CELL_H;
    int oy = (rows - H) / 2; 
    if(oy < 2)
    {
		oy = 2;
	}
	
    int ox = (cols - W) / 2;
    if(ox < 4) 
    {
		ox = 4;
	}

    //Cant stall the game
    if(pozition_start==pozition_end)
      {
        type_text_on_window(48, 106, "Can't stall game.", 500);
        napms(300);
        mvprintw(48, 106, "%s", "                 ");
        return false;
      }

    //Cant move an Empty Space
    if(Matrix[pozition_start].name==Empty)
      {
        // printf("Move Empty\n");
        type_text_on_window(48, 106, "That's an empty space. Can't move anything!", 1000);
        napms(300);
        mvprintw(48, 106, "%s", "                                          ");
        return false;
      }
    //cant capture your own piece
    if(Matrix[pozition_end].name!=Empty && Matrix[pozition_end].isWhite==Matrix[pozition_start].isWhite)
      {
        // printf("Own Piece Capture\n");
        //mvprintw(oy + 20, ox, "%s", "Own Piece Capture!");
        type_text_on_window(48, 106, "Can't capture your own piece, can you?", 1000);
        napms(300);
        mvprintw(48, 106, "%s", "                                      ");
        return false;
      }
    //You cant move the opponent`s piece
    if((client.isWhite && !Matrix[pozition_start].isWhite) || (!client.isWhite && Matrix[pozition_start].isWhite))
      {
        // printf("Move Oponents\n");
        type_text_on_window(48, 106, "Leave your opponent's pieces alone!!!", 1000);
        napms(300);
        mvprintw(48, 106, "%s", "                                     ");
        return false;
      }


    //En Passant
    if(Matrix[pozition_start].name==PAWN && Validate_Semantically(Game->previous_move))  //previous move can be "undefined"
      {
        //need the previous move made as en passant("in passing") must be done immediatly after
        unsigned int previous_column_end      =convert_character_to_column(tolower(Game->previous_move[2]));
        unsigned int previous_row_end         =0;
        sscanf(Game->previous_move,"%*c%*d%*c%d",&previous_row_end);
        previous_row_end = 8 - previous_row_end;
        unsigned int pozition_previous        =previous_row_end*ROW+previous_column_end;
        
        if(Matrix[pozition_start].isWhite==true && row_start==previous_row_end && row_start==3 && abs(column_start-previous_column_end)==1 && column_end==previous_column_end && row_end==previous_row_end-1)
          {
             Game->Matrix[pozition_start].name=Empty;
             Game->Matrix[pozition_end].name=PAWN;
             Game->Matrix[pozition_end].isWhite=true;
             Game->Matrix[pozition_previous].name=Empty;
             if(is_white_in_check(Game->Matrix)==true)
               {
                  //printf("Discovery Check\n");
                  Game->Matrix[pozition_start].name=PAWN;
                  Game->Matrix[pozition_start].isWhite=true;
                  Game->Matrix[pozition_end+ROW].name=PAWN;
                  Game->Matrix[pozition_end+ROW].isWhite=false;
                  return false;
               }
             return true;
          }
        if(Matrix[pozition_start].isWhite==false && row_start==previous_row_end && row_start==4 && abs(column_start-previous_column_end)==1 && column_end==previous_column_end && row_end==previous_row_end+1)
          {
             Game->Matrix[pozition_start].name=Empty;
             Game->Matrix[pozition_end].name=PAWN;
             Game->Matrix[pozition_end].isWhite=false;
             Game->Matrix[pozition_previous].name=Empty;
             if(is_black_in_check(Game->Matrix)==true)
               {
                  // printf("Discovery Check\n");
                  Game->Matrix[pozition_start].name=PAWN;
                  Game->Matrix[pozition_start].isWhite=false;
                  Game->Matrix[pozition_end-ROW].name=PAWN;
                  Game->Matrix[pozition_end-ROW].isWhite=true;
                  return false;
               }
             return true;
          }
      }

    //Pawn Promotes to Queen
    if(Matrix[pozition_start].name==PAWN)
      {
        if(Matrix[pozition_start].isWhite==false && row_end==7 && abs(column_end-column_start)==1)
          {
            Game->Matrix[pozition_start].name=Empty;
            Game->Matrix[pozition_end].name=QUEEN;
            Game->Matrix[pozition_end].isWhite=0;
            Game->is_white_king_checked=is_white_in_check(Game->Matrix);
            return true;
          }
        if(Matrix[pozition_start].isWhite==true && row_end==0 && abs(column_start-column_end)==1)
          {
            Game->Matrix[pozition_start].name=Empty;
            Game->Matrix[pozition_end].name=QUEEN;
            Game->Matrix[pozition_end].isWhite=1;
            Game->is_black_king_checked=is_black_in_check(Game->Matrix);
            return true;
          }
      }

    if(Matrix[pozition_start].name==KING)
      { 
           //rocada mica negru
         if(  Matrix[pozition_start].isWhite==false && Game->has_black_king_moved==false && (row_end==0 && column_end==6)
           && Matrix[row_start*ROW+column_start+1].name==Empty && Matrix[row_start*ROW+column_start+2].name==Empty
           && Matrix[row_start*ROW+column_start+3].name==ROOK  &&  Matrix[row_start*ROW+column_start+3].isWhite==false 
           && Game->is_black_king_checked==false)
           {
             Game->Matrix[pozition_start].name=Empty;
             Game->Matrix[pozition_end].isWhite=false; 
             Game->Matrix[pozition_end].name=KING;
             Game->Matrix[row_start*ROW+column_start+1].name=ROOK;
             Game->Matrix[row_start*ROW+column_start+1].isWhite=false;
             Game->Matrix[row_start*ROW+column_start+3].name=Empty;

             if(Game->is_black_king_checked==true)
              { 
              //rollback ,king is in check after move 
              Game->Matrix[pozition_start].name=KING;
              Game->Matrix[pozition_start].isWhite=false;
              Game->Matrix[row_start*ROW+column_start+3].name=ROOK;
              Game->Matrix[row_start*ROW+column_start+3].isWhite=false;
              Game->Matrix[row_start*ROW+column_start+1].name=Empty; 
              Game->Matrix[row_start*ROW+column_start+2].name=Empty;
              //printf("Still in Check\n");
              return false;
              }

             Game->has_black_king_moved=true;
             Game->is_white_king_checked=is_white_in_check(Game->Matrix);
             return true;

           }
          //rocada mare negru
          if( Matrix[pozition_start].isWhite==false && Game->has_black_king_moved==false && (row_end==0 && column_end==2)
           && Matrix[0*ROW+0].name==ROOK && Matrix[0*ROW+0].isWhite==false && Matrix[0*ROW+1].name==Empty 
           && Matrix[0*ROW+2].name==Empty && Matrix[0*ROW+3].name==Empty   && Game->is_black_king_checked==false)
           {
             Game->Matrix[pozition_start].name=Empty;
             Game->Matrix[pozition_end].isWhite=false; 
             Game->Matrix[pozition_end].name=KING;
             Game->Matrix[0*ROW+3].name=ROOK;
             Game->Matrix[0*ROW+3].isWhite=false;
             Game->Matrix[0*ROW+0].name=Empty;

             if(Game->is_black_king_checked==true)
              { 
              //rollback ,king is in check after move 
              Game->Matrix[pozition_start].name=KING;
              Game->Matrix[pozition_start].isWhite=false;
              Game->Matrix[0*ROW+0].name=ROOK;
              Game->Matrix[0*ROW+0].isWhite=false;
              Game->Matrix[0*ROW+1].name=Empty;
              Game->Matrix[0*ROW+2].name=Empty;
              Game->Matrix[0*ROW+3].name=Empty;
              //printf("Still in Check\n");
              return false;
              } 

             Game->has_black_king_moved=true;
             Game->is_white_king_checked=is_white_in_check(Game->Matrix);
             return true;
           }
          //rocada mica alb
          if(Matrix[pozition_start].isWhite==true && Game->has_white_king_moved==false && (row_end==7 && column_end==6)
           && Matrix[7*ROW+column_start+1].name==Empty && Matrix[7*ROW+column_start+2].name==Empty
           && Matrix[7*ROW+column_start+3].name==ROOK && Game->is_white_king_checked==false)
           {
             Game->Matrix[pozition_start].name=Empty;
             Game->Matrix[pozition_end].isWhite=true; 
             Game->Matrix[pozition_end].name=KING;
             Game->Matrix[7*ROW+column_start+1].name=ROOK;
             Game->Matrix[7*ROW+column_start+1].isWhite=true;
             Game->Matrix[7*ROW+column_start+3].name=Empty;

             if(Game->is_white_king_checked==true)
              { 
              //rollback ,king is in check after move 
              Game->Matrix[pozition_start].name=KING;
              Game->Matrix[pozition_start].isWhite=true;
              Game->Matrix[7*ROW+column_start+3].name=ROOK;
              Game->Matrix[7*ROW+column_start+3].isWhite=true;
              Game->Matrix[7*ROW+column_start+1].name=Empty; 
              Game->Matrix[7*ROW+column_start+2].name=Empty;
              //printf("Still in Check\n");
              return false;
              }

             Game->has_white_king_moved=true;
             Game->is_black_king_checked=is_black_in_check(Game->Matrix);
             return true;

           }
          //rocada mare alb
          if(Matrix[pozition_start].isWhite==true && Game->has_white_king_moved==false && (row_end==7 && column_end==2)
           && Matrix[7*ROW+0].name==ROOK   && Matrix[7*ROW+0].isWhite==true && Matrix[7*ROW+1].name==Empty && Matrix[7*ROW+2].name==Empty 
           && Matrix[7*ROW+3].name==Empty  && Game->is_black_king_checked==false)
           {
             Game->Matrix[pozition_start].name=Empty;
             Game->Matrix[pozition_end].isWhite=true; 
             Game->Matrix[pozition_end].name=KING;
             Game->Matrix[7*ROW+3].name=ROOK;
             Game->Matrix[7*ROW+3].isWhite=true;
             Game->Matrix[7*ROW+0].name=Empty;

             if(Game->is_white_king_checked==true)
              { 
              //rollback ,king is in check after move 
              Game->Matrix[pozition_start].name=KING;
              Game->Matrix[pozition_start].isWhite=true;
              Game->Matrix[7*ROW+0].name=ROOK;
              Game->Matrix[7*ROW+0].isWhite=true;
              Game->Matrix[7*ROW+1].name=Empty;
              Game->Matrix[7*ROW+2].name=Empty;
              Game->Matrix[7*ROW+3].name=Empty;
              //printf("Still in Check\n");
              return false;
              } 

             Game->has_white_king_moved=true;
             Game->is_black_king_checked=is_black_in_check(Game->Matrix);
             return true;
           }
      }

    if(Validate_Piece_Logic(Game->Matrix,move)==false)
      {
        // printf("Piece Logic");
        type_text_on_window(48, 106, "Invalid move!", 1000);
        napms(300);
        mvprintw(48, 106, "%s", "             ");
        return false;
      }

    Element_T capturer=Game->Matrix[row_start*ROW+column_start];
    Element_T to_be_captured=Game->Matrix[row_end*ROW+column_end];
    Game=Apply_Move_Matrix(Game,move);                                 //apply move to the game

    if(client.isWhite==true && Game->is_white_king_checked==true) //from previous move state
      {
        Game->is_white_king_checked=is_white_in_check(Game->Matrix);//update check value
        if(Game->is_white_king_checked==true)
          {
            //rollback ,king is still in check after move 
            Game->Matrix[row_start*ROW+column_start]=capturer;
            Game->Matrix[row_end*ROW+column_end]=to_be_captured;
            // printf("Still in Check\n");
            type_text_on_window(48, 106, "...still in check", 500);
			napms(300);
			mvprintw(48, 106, "%s", "                 ");
            return false;
          }
      }
     if(client.isWhite==false && Game->is_black_king_checked==true) //from previous move state
      {
        Game->is_black_king_checked=is_black_in_check(Game->Matrix);//update check value
        if(Game->is_white_king_checked==true)
          {
            //rollback ,king is still in check after move
            Game->Matrix[row_start*ROW+column_start]=capturer;
            Game->Matrix[row_end*ROW+column_end]=to_be_captured;
            // printf("Still in Check\n");
            type_text_on_window(48, 106, "...still in check", 500);
			napms(300);
			mvprintw(48, 106, "%s", "                 ");
            return false;
          }
      }

    if(client.isWhite==true)
      {
          if(is_white_in_check(Game->Matrix)==true)
            {
            //after your move cant be in a discovery check
            Game->Matrix[row_start*ROW+column_start]=capturer;
            Game->Matrix[row_end*ROW+column_end]=to_be_captured;
            // printf("Discovery Check\n");
            return false;
            }
      }
    if(client.isWhite==false)
      {
          if(is_black_in_check(Game->Matrix)==true)
            {
            //after your move cant be in a discovery check
            Game->Matrix[row_start*ROW+column_start]=capturer;
            Game->Matrix[row_end*ROW+column_end]=to_be_captured;
            // printf("Discovery Check\n");
            return false;
            }
      }
      
    //king can't castle if he was moved
    if(Matrix[pozition_start].name==KING)
    {
      if(Matrix[pozition_start].isWhite==true)
        {
          Game->has_white_king_moved=true;
        }
      else
        {
          Game->has_black_king_moved=true;
        }
    }

    //check if the game ends in a draw by insufficient material
    if(Check_Sufficient_Material(Game->Matrix)==false)
      {
        Game->game_ended_in_draw=true;
        Game->game_ended_in_white_won=false;
        Game->game_is_running=false;
        return true;
      }

    //check if kings are in check after move
    if(client.isWhite==true)
      {
        Game->is_black_king_checked=is_black_in_check(Game->Matrix);
      }
    else
      {
        Game->is_white_king_checked=is_white_in_check(Game->Matrix);
      }
    
    //check if the game should end by checkmate or stalemate
    if(client.isWhite==true)
      {
        bool check_moves_black=Check_Moves_Available_Black(Game);
        if(check_moves_black==false && Game->is_black_king_checked==true)
          {
            //checkmate
            Game->game_ended_in_draw=false;
            Game->game_ended_in_white_won=true;
            Game->game_is_running=false;
            return true;
          }
        if(check_moves_black==false && Game->is_black_king_checked==false)
          {
            //stalemate
            Game->game_ended_in_draw=true;
            Game->game_ended_in_white_won=false;
            Game->game_is_running=false;
            return true;  
          }
      }
    else
      {
        bool check_moves_white=Check_Moves_Available_White(Game);
        if(check_moves_white==false && Game->is_white_king_checked==true)
          {
            //checkmate
            Game->game_ended_in_draw=false;
            Game->game_ended_in_white_won=false;
            Game->game_is_running=false;
            return true;
          }
        if(check_moves_white==false && Game->is_white_king_checked==false)
          {
            //stalemate
            Game->game_ended_in_draw=true;
            Game->game_ended_in_white_won=false;
            Game->game_is_running=false;
            return true;  
          }
      }

    strcpy(Game->previous_move,move);
    return true;
}

Game_T *Reinstate_Game(Game_T *Game)
{
  Game->number_of_moves_played=0;
  Game->is_black_king_checked=false;
  Game->is_white_king_checked=false;
  Game->has_white_king_moved=false;
  Game->has_black_king_moved=false;
  Game->game_ended_in_draw=false;
  Game->game_ended_in_white_won=false;
  Game->game_is_running=true;
  strcpy(Game->previous_move,"undefined");

  short unsigned int i=0,j=0;
    for(i=0;i<ROW;i++)
      for(j=0;j<COLUMN;j++)
        {
            if(i<=1)
              Game->Matrix[i*ROW+j].isWhite=false;
            else
              Game->Matrix[i*ROW+j].isWhite=true;
        }

    for(i=0;i<ROW;i++)
      for(j=0;j<COLUMN;j++)
        {
            if(i>=2 && i<=5)
              Game->Matrix[i*ROW+j].name=Empty;
        }

    for(j=0;j<COLUMN;j++)
        {
            Game->Matrix[1*ROW+j].name=PAWN;
            Game->Matrix[6*ROW+j].name=PAWN;
        }
    Game->Matrix[0*ROW+0].name=ROOK;   Game->Matrix[0*ROW+7].name=ROOK;   Game->Matrix[7*ROW+0].name=ROOK;   Game->Matrix[7*ROW+7].name=ROOK; 
    Game->Matrix[0*ROW+1].name=KNIGHT; Game->Matrix[0*ROW+6].name=KNIGHT; Game->Matrix[7*ROW+1].name=KNIGHT; Game->Matrix[7*ROW+6].name=KNIGHT;    
    Game->Matrix[0*ROW+2].name=BISHOP; Game->Matrix[0*ROW+5].name=BISHOP; Game->Matrix[7*ROW+2].name=BISHOP; Game->Matrix[7*ROW+5].name=BISHOP;  
    Game->Matrix[0*ROW+3].name=QUEEN;  Game->Matrix[7*ROW+3].name=QUEEN;
    Game->Matrix[0*ROW+4].name=KING;   Game->Matrix[7*ROW+4].name=KING;

  return Game;
}

void print_Game(Game_T *Game)
{
  //TODO Implement Ana's API
   printf("                  %s-Black       \n",Game->client2.username);
   print_Matrix(Game->Matrix);
   printf("                  %s-White       \n",Game->client1.username);
}

//DRIVER FOR TESTS
#ifdef DEBUG
int main()
{
    /*Semantic Check
    printf("%d\n",Validate_Semantically("D2d5"));
    printf("%d\n",Validate_Semantically("d2d5"));
    printf("%d\n",Validate_Semantically("draw"));
    printf("%d\n",Validate_Semantically("resign"));
    printf("%d\n",Validate_Semantically("Raul"));
    printf("%d\n",Validate_Semantically(""));
    printf("%d\n",Validate_Semantically("d10s"));
    */

     /*Special Checks
      PAWN PROMOTE CHECKED

     */

    /*is_black_in_check
     PAWN,ROOK,KNIGHT,BISHOP,QUEEN

    */
      
    /*Logic Check
    ROOK CHECKED
    BISHOP CHECKED
    PAWN CHECKED 
    KING CHECKED
    KNIGHT CHECKED
    QUEEN CHECKED
    */
    Game_T *Game=Initialize_Classic_Game();
    //unsigned int pos[ROW*COLUMN];
    Game=Apply_Move_Matrix(Game,"d1f7");
    Game=Apply_Move_Matrix(Game,"f1c4");
    Game=Apply_Move_Matrix(Game,"e7e5");
    //Game=Apply_Move_Matrix(Game,"d8e7");
    //printf("%d",Validate_Piece_Logic(Game->Matrix,"d7d5"));
    //unsigned int size=calculate_available_positions(Game->Matrix,pos,3,3);
    //printf("%d\n",size);
    printf("\nState Check White %d \n",is_white_in_check(Game->Matrix));
    printf("State Check Black %d \n",is_black_in_check(Game->Matrix));
    printf("BLACK %d\n",Check_Moves_Available_Black(Game));
    printf("WHITE %d\n",Check_Moves_Available_White(Game));


    print_Game(Game);
    //print_posible_moves(size,pos);

    /*Reinstate check
    Game=Reinstate_Game(Game);
    print_Game(Game);
    */

    free(Game->Matrix);
    free(Game);
    return 0;
}
#endif
