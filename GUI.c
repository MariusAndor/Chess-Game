#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <locale.h>
#include <ctype.h>
#include "GUI.h"
#include "chessLogic.h"

#define MAX 512
#define ROW 8
#define COL 8

void enable_utf8_locale(void)
{
  // use the environment's default locale
  setlocale(LC_ALL, "");
}

int get_square_color(int row, int column)
{
  // light square: the sum is even
  // dark square: the sum is odd
  if (!((row + column) % 2))
  {
    return 2; // light
  }

  return 1; // dark
}

void init_colors(void)
{
  // check whether terminal supports colors
  if (!(has_colors()))
  {
    return;
  }

  start_color();
  use_default_colors();

  // board squares
  init_pair(1, -1, COLOR_BLACK);
  init_pair(2, -1, COLOR_WHITE);

  // pieces
  int BLACK = COLOR_BLUE;
  int RED = COLOR_RED;

  init_pair(3, RED, -1);
  init_pair(4, BLACK, -1);
  init_pair(5, NONE, -1);

  short tmp_fg, bg_light, bg_dark;
  pair_content(1, &tmp_fg, &bg_dark);
  pair_content(2, &tmp_fg, &bg_light);

  // (black / red) piece with (light / dark) bg
  init_pair(6, BLACK, bg_light);
  init_pair(7, BLACK, bg_dark);
  init_pair(8, RED, bg_light);
  init_pair(9, RED, bg_dark);
}

void put_piece(int oy, int ox, int r, int c, const char *glyph, int color)
{
  if (color == NONE)
  {
    return;
  }

  int square = get_square_color(r, c);

  // choose appropriate color-pair for the given piece
  int use_pair;

  /*
    init_pair(6, BLACK, bg_light);
    init_pair(7, BLACK, bg_dark);
    init_pair(8, RED, bg_light);
    init_pair(9, RED, bg_dark);
  */

  if (color == BLACK)
  {
    if (square == 1) // dark bg
    {
      use_pair = 7;
    }
    else
    {
      use_pair = 6;
    }
  }
  else // RED piece
  {
    if (square == 1)
    {
      use_pair = 9;
    }
    else
    {
      use_pair = 8;
    }
  }

  int y = oy + r * CELL_H + CELL_H / 2;
  int x = ox + c * CELL_W + CELL_W / 2;

  attron(COLOR_PAIR(use_pair) | A_BOLD);
  mvaddstr(y, x, glyph);
  attroff(COLOR_PAIR(use_pair) | A_BOLD);
}

void draw_board(int oy, int ox)
{
  init_colors();

  // fill squares
  bool hc = has_colors();

  for (int r = 0; r < ROW; r++)
  {
    for (int c = 0; c < COL; c++)
    {
      int pair = get_square_color(r, c);
      if (hc)
      {
        attron(COLOR_PAIR(pair));
      }
      for (int dy = 0; dy < CELL_H; ++dy)
      {
        for (int dx = 0; dx < CELL_W; ++dx)
        {
          mvaddch(oy + r * CELL_H + dy, ox + c * CELL_W + dx, ' ');
        }
      }
      if (hc)
      {
        attroff(COLOR_PAIR(pair));
      }
    }
  }

  // labels
  for (int c = 0; c < 8; ++c)
  {
    char file = 'a' + c;
    int cx = ox + c * CELL_W + CELL_W / 2;
    mvaddch(oy - 1, cx, file);
  }

  for (int r = 0; r < 8; r++)
  {
    char rank = '8' - r;
    // char rank = '1' + r;
    int ry = oy + r * CELL_H + CELL_H / 2;
    mvaddch(ry, ox - 2, rank);
  }
}

void type_text_on_window(int oy, int ox, const char *text, int seconds)
{
  int length = (int)strlen(text);
  int letter_time = seconds / strlen(text);

  for (int i = 0; i < length; i++)
  {
    mvprintw(oy, ox + i, "%c", text[i]);
    refresh();
    napms(letter_time);
  }

  napms(200);
}

char *read_from_window(WINDOW *w, int oy, int ox)
{
  char *buf = (char *)malloc(sizeof(char) * MAX);
  if (!buf)
  {
    perror("Error using malloc in read_from_window()");
    exit(EXIT_FAILURE);
  }

  int count = 0;
  int c;

  buf[0] = '\0'; // start empty
  wmove(w, oy, ox);
  wrefresh(w);

  while (1)
  {
    c = wgetch(w);

    // ENTER -> finish input
    if (c == '\n' || c == '\r')
    {
      buf[count] = '\0';
      break;
    }

    // BACKSPACE handling
    if (c == KEY_BACKSPACE || c == 8 || c == 127)
    {
      if (count > 0)
      {
        count--;
        buf[count] = '\0';
        mvwaddch(w, oy, ox + count, ' '); // erase char
        wmove(w, oy, ox + count);
        wrefresh(w);
      }
      continue;
    }

    // Printable character
    if (isprint(c))
    {
      if (count < MAX - 1)
      {
        buf[count++] = (char)c;
        buf[count] = '\0';
        mvwaddch(w, oy, ox + count - 1, (chtype)c);
        wmove(w, oy, ox + count);
        wrefresh(w);
      }
      continue;
    }
  }

  return buf;
}

void draw_pieces(int oy, int ox, piece_t **m)
{
  const char *p[] = {" ", "♟", "♜", "♝", "♞", "♛", "♚"};

  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      /*
        empty,  // position is empty - 0
        pawn,  // pion - 1
        rook,  // tura - 2
        bishop,  //nebun - 3
        knight,  // cal - 4
        queen,  // 5
        king  // 6
      */
      if (!m[i][j].type)
      {
        // empty
        continue;
      }

      put_piece(oy, ox, i, j, p[m[i][j].type], m[i][j].color);
    }
  }
}
void from_raul_to_ana(piece_t **ana, char *move)
{
  int from_col = convert_character_to_column(move[0]);
  int to_col = convert_character_to_column(move[2]);

  int from_row = 8 - (move[1] - '0');
  int to_row = 8 - (move[3] - '0');

  piece_t moving = ana[from_row][from_col];

  // destination occupied? → capture
  if (ana[to_row][to_col].type != empty)
  {
    // overwrite the captured piece
    ana[to_row][to_col] = moving;
  }
  else
  {
    // normal move
    ana[to_row][to_col] = moving;
  }

  // clear origin square
  ana[from_row][from_col].type = empty;
  ana[from_row][from_col].color = NONE;
}
