//
// Created by Neven Boric on 2/25/23.
//

#ifndef _COMMON_H_
#define _COMMON_H_

enum color
{
	WHITE,
	BLACK
};

enum piece_id
{
	PAWN,
	ROOK,
	KNIGHT,
	BISHOP,
	QUEEN,
	KING
};

struct pos
{
	int p[2];
};

struct coord
{
	char c[2];
};

struct square
{
	enum color color;
	struct piece* piece;
};

struct board
{
	struct square squares[8][8];
};

struct piece
{
	enum color color;
	enum piece_id type;
	struct pos pos;
	struct coord coord;
	char is_captured;
	char valid_moves[8][8];
	void (* valid_moves_update_func)(struct board* board, struct piece* piece);
	char* name;
	char* symbol;
	int has_moved;
	int is_in_check;
	int is_giving_check;
};

struct player
{
	enum color id;
	struct piece* pieces[16];
};

struct game_status
{
	int ended;
	enum color current_player;
	struct board *board;
	struct player players[2];
};

struct move
{
	struct coord from;
	struct coord to;
};

void coord_to_pos(struct pos* pos, struct coord* coord);
struct piece* get_piece(struct board* board, struct pos* pos);

struct board* board_copy(struct board* board);
void board_free(struct board* board);

#endif //_COMMON_H_
