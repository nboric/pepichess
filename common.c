//
// Created by Neven Boric on 2/25/23.
//

#include <stdlib.h>
#include <string.h>
#include "common.h"

void coord_to_pos(struct pos* pos, struct coord* coord)
{
	pos->p[0] = 7 - (coord->c[1] - '1');
	pos->p[1] = coord->c[0] - 'A';
}

void pos_to_coord(struct coord* coord, struct pos* pos)
{
	coord->c[0] = 'A' + pos->p[1];
	coord->c[1] = '1' + 7 - pos->p[0];
}

struct piece* get_piece(struct board* board, struct pos* pos)
{
	return board->squares[pos->p[0]][pos->p[1]].piece;
}

enum color get_other_player(enum color current_player)
{
	return current_player == WHITE ? BLACK : WHITE;
}

void* piece_copy(void* piece)
{
	struct piece* new = malloc(sizeof(struct piece));
	memcpy(new, piece, sizeof(struct piece));
	return new;
}

void piece_add_to_board(void* value, void* data)
{
	struct piece* piece = value;
	struct board* board = data;
	board->squares[piece->pos.p[0]][piece->pos.p[1]].piece = piece;
}

struct board* board_copy(struct board* board)
{
	struct board* board2 = malloc(sizeof(struct board));
	memcpy(board2, board, sizeof(struct board));

	board2->active_pieces[0] = ll_copy(board->active_pieces[0], piece_copy);
	board2->active_pieces[1] = ll_copy(board->active_pieces[1], piece_copy);

	for (int i = 0; i < 2; i++)
	{
		ll_apply(board2->active_pieces[i], piece_add_to_board, board2);
	}
	for (int i = 0; i < 2; i++)
	{
		for (struct ll_node* n = board->active_pieces[i]->next; n != NULL; n = n->next)
		{
			struct piece* piece = n->value;
			if (piece->type == KING)
			{
				board2->kings[i] = piece;
				break;
			}
		}
	}

	return board2;
}

void board_free(struct board* board)
{
	ll_free(board->active_pieces[0], free);
	ll_free(board->active_pieces[1], free);
	free(board);
}

double calc_score(struct board* board, enum color current_player)
{
	double score = 0;
	for (int i = 0; i < 2; i++)
	{
		int sign = i == current_player ? 1 : -1;
		for (struct ll_node* n = board->active_pieces[i]->next; n != NULL; n = n->next)
		{
			struct piece* piece = n->value;
			score += sign * piece->capture_score;
		}
	}
	return score;
}