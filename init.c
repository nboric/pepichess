//
// Created by Neven Boric on 3/6/23.
//

#include <stdlib.h>
#include "init.h"
#include "move_logic.h"
#include "piece_logic.h"

#define USE_UNICODE_PIECES

void add_pieces_to_board(struct board* board, struct player* player)
{
	for (int i = 0; i < 16; i++)
	{
		struct pos* pos = &player->pieces[i]->pos;
		board->squares[pos->p[0]][pos->p[1]].piece = player->pieces[i];
		ll_add(board->active_pieces[player->id], player->pieces[i]);
	}
	update_valid_moves(board);
}

struct piece* create_piece(enum color color, enum piece_id type, struct coord coord)
{
	struct piece* piece = malloc(sizeof(struct piece));
	piece->color = color;
	piece->type = type;
	piece->coord = coord;
	coord_to_pos(&piece->pos, &piece->coord);

	switch (type)
	{
	case PAWN:
		piece->name = "pawn";
#ifdef USE_UNICODE_PIECES
		piece->symbol = color == WHITE ? "♙" : "♟";
#else
		piece->symbol = color == WHITE ? "P" : "p";
#endif
		piece->valid_moves_update_func = move_validation_pawn;
		piece->capture_score = 1;
		break;
	case ROOK:
		piece->name = "rook";
#ifdef USE_UNICODE_PIECES
		piece->symbol = color == WHITE ? "♖" : "♜";
#else
		piece->symbol = color == WHITE ? "R" : "r";
#endif
		piece->valid_moves_update_func = move_validation_rook;
		piece->capture_score = 5;
		break;
	case KNIGHT:
		piece->name = "knight";
#ifdef USE_UNICODE_PIECES
		piece->symbol = color == WHITE ? "♘" : "♞";
#else
		piece->symbol = color == WHITE ? "N" : "n";
#endif
		piece->valid_moves_update_func = move_validation_knight;
		piece->capture_score = 3;
		break;
	case BISHOP:
		piece->name = "bishop";
#ifdef USE_UNICODE_PIECES
		piece->symbol = color == WHITE ? "♗" : "♝";
#else
		piece->symbol = color == WHITE ? "B" : "b";
#endif
		piece->valid_moves_update_func = move_validation_bishop;
		piece->capture_score = 3.25;
		break;
	case QUEEN:
		piece->name = "queen";
#ifdef USE_UNICODE_PIECES
		piece->symbol = color == WHITE ? "♕" : "♛";
#else
		piece->symbol = color == WHITE ? "Q" : "q";
#endif
		piece->valid_moves_update_func = move_validation_queen;
		piece->capture_score = 9;
		break;
	case KING:
		piece->name = "king";
#ifdef USE_UNICODE_PIECES
		piece->symbol = color == WHITE ? "♔" : "♚";
#else
		piece->symbol = color == WHITE ? "K" : "k";
#endif
		piece->valid_moves_update_func = move_validation_king;
		piece->capture_score = 40;
		break;
	}
	piece->has_moved = 0;

	return piece;
}

void init_player(struct player* player, int color)
{
	int n = 0;
	player->id = color;
	if (player->id == WHITE)
	{
		player->pieces[n++] = create_piece(WHITE, ROOK, (struct coord){ 'A', '1' });
		player->pieces[n++] = create_piece(WHITE, KNIGHT, (struct coord){ 'B', '1' });
		player->pieces[n++] = create_piece(WHITE, BISHOP, (struct coord){ 'C', '1' });
		player->pieces[n++] = create_piece(WHITE, QUEEN, (struct coord){ 'D', '1' });
		player->pieces[n++] = create_piece(WHITE, KING, (struct coord){ 'E', '1' });
		player->pieces[n++] = create_piece(WHITE, BISHOP, (struct coord){ 'F', '1' });
		player->pieces[n++] = create_piece(WHITE, KNIGHT, (struct coord){ 'G', '1' });
		player->pieces[n++] = create_piece(WHITE, ROOK, (struct coord){ 'H', '1' });
		for (int i = 0; i < 8; i++)
		{
			player->pieces[n++] = create_piece(WHITE, PAWN, (struct coord){ 'A' + i, '2' });
		}
	}
	else
	{
		player->pieces[n++] = create_piece(BLACK, ROOK, (struct coord){ 'A', '8' });
		player->pieces[n++] = create_piece(BLACK, KNIGHT, (struct coord){ 'B', '8' });
		player->pieces[n++] = create_piece(BLACK, BISHOP, (struct coord){ 'C', '8' });
		player->pieces[n++] = create_piece(BLACK, QUEEN, (struct coord){ 'D', '8' });
		player->pieces[n++] = create_piece(BLACK, KING, (struct coord){ 'E', '8' });
		player->pieces[n++] = create_piece(BLACK, BISHOP, (struct coord){ 'F', '8' });
		player->pieces[n++] = create_piece(BLACK, KNIGHT, (struct coord){ 'G', '8' });
		player->pieces[n++] = create_piece(BLACK, ROOK, (struct coord){ 'H', '8' });
		for (int i = 0; i < 8; i++)
		{
			player->pieces[n++] = create_piece(BLACK, PAWN, (struct coord){ 'A' + i, '7' });
		}
	}
}

struct board* board_create()
{
	struct board* b = malloc(sizeof(struct board));
	enum color color = WHITE;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			b->squares[i][j].color = color;
			b->squares[i][j].piece = NULL;
			color++;
			color %= 2;
		}
		color++;
		color %= 2;
	}
	b->active_pieces[0] = ll_create();
	b->active_pieces[1] = ll_create();
	return b;
}