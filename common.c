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

struct piece* get_piece(struct board* board, struct pos* pos)
{
	return board->squares[pos->p[0]][pos->p[1]].piece;
}

struct board* board_copy(struct board* board)
{
	struct board* board2 = malloc(sizeof(struct board));
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			memcpy(&board2->squares[i][j], &board->squares[i][j], sizeof(struct square));
			if (board->squares[i][j].piece != NULL)
			{
				board2->squares[i][j].piece = malloc(sizeof(struct piece));
				memcpy(board2->squares[i][j].piece, board->squares[i][j].piece, sizeof(struct piece));
			}
		}
	}
	return board2;
}

void board_free(struct board* board)
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (board->squares[i][j].piece != NULL)
			{
				free(board->squares[i][j].piece);
			}
		}
	}
	free(board);
}