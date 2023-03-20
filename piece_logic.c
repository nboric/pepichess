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

int is_castling_allowed(struct board* board, struct piece* king, struct piece* rook)
{
	if (king->has_moved || rook->has_moved)
	{
		return 0;
	}
	if (king->is_in_check)
	{
		return 0;
	}
	int king_dir = 1;
	if (rook->pos.p[1] < king->pos.p[1])
	{
		king_dir = -1;
	}
	int step;
	for (step = 1; step < 3; step++)
	{
		struct pos new_pos = (struct pos){ king->pos.p[0], king->pos.p[1] + step * king_dir };
		if (!is_empty(board, &new_pos))
		{
			return 0;
		}
		struct move_pos move = (struct move_pos){ king->pos.p[0], king->pos.p[1], new_pos.p[0], new_pos.p[1] };
		struct coord coord_from;
		struct coord coord_to;
		pos_to_coord(&coord_from, &move.from);
		pos_to_coord(&coord_to, &move.to);
		struct move_coord move_coord = (struct move_coord){ coord_from, coord_to };
		if (move_puts_king_in_check(board, king->color, &move_coord))
		{
			return 0;
		}
	}
	if (king_dir == -1)
	{
		struct pos new_pos = (struct pos){ rook->pos.p[0], rook->pos.p[1] + 1 };
		if (!is_empty(board, &new_pos))
		{
			return 0;
		}
	}
	return 1;
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
	// castling
	if (!piece->has_moved && !piece->is_in_check)
	{
		for (int dir = -1; dir < 2; dir += 2)
		{
			struct piece* rook;
			struct pos rook_pos = (struct pos){ piece->pos.p[0], piece->pos.p[1] + (dir == 1? 3: -4) };
			if ((rook = board->squares[rook_pos.p[0]][rook_pos.p[1]].piece) != NULL)
			{
				if (rook->type == ROOK && rook->color == piece->color)
				{
					if (is_castling_allowed(board, piece, rook))
					{
						struct pos new_pos = (struct pos){ piece->pos.p[0], piece->pos.p[1] + dir * 2 };
						set_valid(piece, &new_pos, 1);
					}
				}
			}
		}
	}
}