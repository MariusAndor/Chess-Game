#ifndef GUI_H
#define GUI_H

#define ROW 8
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



Element_T *Initialize_Classic_Game_Matrix();
Game_T *Initialize_Classic_Game();
Game_T *Reinstate_Game(Game_T *Game);

Game_T*Apply_Move_Matrix(Game_T*Game,const char* move);
bool Game_Move(Game_T *Game,Client client,const char* move);
unsigned int convert_character_to_column(char c);

void print_Matrix(Element_T *Matrix);
void print_Game(Game_T *Game);
void Print_Game_Result(Game_T *Game);

#endif
