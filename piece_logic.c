//
// Created by Neven Boric on 2/25/23.
//

#include <string.h>
#include "piece_logic.h"
#include "common.h"
#include "move_logic.h"

int is_empty(struct board* board, struct pos* pos)
{
	return get_piece(board, pos) == NULL;
}

int is_empty_or_can_capture(struct board* board, struct pos* pos, struct piece* piece_from)
{
	if (is_within_boundaries(pos))
	{
		if (is_empty(board, pos))
		{
			return 1;
		}
		else
		{
			struct piece* piece_to = get_piece(board, pos);
			if (piece_to != NULL && piece_to->color != piece_from->color)
			{
				return 1;
			}
		}
	}
	return 0;
}

void set_valid(struct piece* piece, struct pos* pos, int valid)
{
	piece->valid_moves[pos->p[0]][pos->p[1]] = valid;
}

void move_validation_pawn(struct board* board, struct piece* piece)
{
	int step = piece->color == WHITE ? -1 : 1;
	struct pos new_pos = (struct pos){ piece->pos.p[0] + step, piece->pos.p[1] };
	if (is_within_boundaries(&new_pos) && is_empty(board, &new_pos))
	{
		set_valid(piece, &new_pos, 1);
		if (!piece->has_moved)
		{
			step = piece->color == WHITE ? -2 : 2;
			new_pos = (struct pos){ piece->pos.p[0] + step, piece->pos.p[1] };
			if (is_within_boundaries(&new_pos) && is_empty(board, &new_pos))
			{
				set_valid(piece, &new_pos, 1);
			}
		}
	}

	step = piece->color == WHITE ? -1 : 1;
	for (int i = 0; i < 2; i++)
	{
		new_pos = (struct pos){ piece->pos.p[0] + step, piece->pos.p[1] - 1 + i * 2 };
		if (is_within_boundaries(&new_pos) && !is_empty(board, &new_pos))
		{
			struct piece* piece_to = get_piece(board, &new_pos);
			if (piece_to != NULL && piece_to->color != piece->color)
			{
				set_valid(piece, &new_pos, 1);
			}
		}
	}
}

void move_validation_rook(struct board* board, struct piece* piece)
{
	for (int hor_vert = 0; hor_vert < 2; hor_vert++)
	{
		for (int dir = -1; dir < 2; dir += 2)
		{
			struct pos new_pos;
			int i = 1;
			while (1)
			{
				if (hor_vert == 0)
				{
					new_pos = (struct pos){ piece->pos.p[0] + dir * i, piece->pos.p[1] };
				}
				else
				{
					new_pos = (struct pos){ piece->pos.p[0], piece->pos.p[1] + dir * i };
				}
				if (!is_within_boundaries(&new_pos))
				{
					break;
				}
				if (is_empty_or_can_capture(board, &new_pos, piece))
				{
					set_valid(piece, &new_pos, 1);
				}
				if (!is_empty(board, &new_pos))
				{
					break;
				}
				i++;
			}
		}
	}
}

void move_validation_bishop(struct board* board, struct piece* piece)
{
	for (int diag = 0; diag < 2; diag++)
	{
		for (int dir = -1; dir < 2; dir += 2)
		{
			struct pos new_pos;
			int i = 1;
			while (1)
			{
				if (diag == 0)
				{
					new_pos = (struct pos){ piece->pos.p[0] + dir * i, piece->pos.p[1] + dir * i };
				}
				else
				{
					new_pos = (struct pos){ piece->pos.p[0] + dir * i, piece->pos.p[1] - dir * i };
				}
				if (!is_within_boundaries(&new_pos))
				{
					break;
				}
				if (is_empty_or_can_capture(board, &new_pos, piece))
				{
					set_valid(piece, &new_pos, 1);
				}
				if (!is_empty(board, &new_pos))
				{
					break;
				}
				i++;
			}
		}
	}
}

void move_validation_queen(struct board* board, struct piece* piece)
{
	move_validation_rook(board, piece);
	move_validation_bishop(board, piece);
}

void move_validation_knight(struct board* board, struct piece* piece)
{
	for (int dir0 = -1; dir0 < 2; dir0 += 2)
	{
		for (int dir1 = -1; dir1 < 2; dir1 += 2)
		{
			for (int long_dir = 0; long_dir < 2; long_dir++)
			{
				struct pos new_pos;
				if (long_dir == 0)
				{
					new_pos = (struct pos){ piece->pos.p[0] + dir0 * 2, piece->pos.p[1] + dir1 };
				}
				else
				{
					new_pos = (struct pos){ piece->pos.p[0] + dir0, piece->pos.p[1] + dir1 * 2 };
				}
				if (is_empty_or_can_capture(board, &new_pos, piece))
				{
					set_valid(piece, &new_pos, 1);
				}
			}
		}
	}
}

void move_validation_king(struct board* board, struct piece* piece)
{
	for (int dir0 = -1; dir0 < 2; dir0++)
	{
		for (int dir1 = -1; dir1 < 2; dir1++)
		{
			if (dir0 == 0 && dir1 == 0)
			{
				continue;
			}
			struct pos new_pos = (struct pos){ piece->pos.p[0] + dir0, piece->pos.p[1] + dir1 };
			if (is_empty_or_can_capture(board, &new_pos, piece))
			{
				set_valid(piece, &new_pos, 1);
			}
		}
	}
}