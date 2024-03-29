//
// Created by Neven Boric on 3/6/23.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "move_logic.h"
#include "init.h"

int is_within_boundaries(struct pos* pos)
{
	if (pos->p[0] >= 0 && pos->p[0] <= 7 && pos->p[1] >= 0 && pos->p[1] <= 7)
	{
		return 1;
	}
	return 0;
}

void reset_valid_moves(struct piece* piece)
{
	memset(piece->valid_moves, 0, sizeof(piece->valid_moves));
	piece->is_giving_check = 0;
	piece->is_in_check = 0;
}

void update_is_giving_check(struct board* board, struct piece* piece)
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (piece->valid_moves[i][j])
			{
				struct pos pos_to = (struct pos){ i, j };
				struct piece* piece_to = get_piece(board, &pos_to);
				if (piece_to != NULL && piece_to->color != piece->color && piece_to->type == KING)
				{
					piece->is_giving_check = 1;
				}
			}
		}
	}
}

int move_did_put_king_in_check(struct board* board, enum color current_player)
{
	int result = 0;
	int other_player = get_other_player(current_player);

	for (struct ll_node* n = board->active_pieces[other_player]->next; n != NULL; n = n->next)
	{
		struct piece* piece = n->value;
		if (piece->is_giving_check)
		{
			result = 1;
			break;
		}
	}
	return result;
}

struct piece* move_is_castling(struct board* board, struct piece* piece, struct move_coord* move_coord)
{
	if (piece->type != KING)
	{
		return NULL;
	}
	struct move_pos move_pos;
	coord_to_pos(&move_pos.from, &move_coord->from);
	coord_to_pos(&move_pos.to, &move_coord->to);
	if (move_pos.from.p[0] != move_pos.to.p[0])
	{
		return NULL;
	}
	if (move_pos.to.p[1] == move_pos.from.p[1] + 2)
	{
		return board->squares[move_pos.from.p[0]][move_pos.from.p[1] + 3].piece;
	}
	if (move_pos.to.p[1] == move_pos.from.p[1] - 2)
	{
		return board->squares[move_pos.from.p[0]][move_pos.from.p[1] - 4].piece;
	}
	return NULL;
}

struct piece* move_is_promotion(struct board* board, struct piece* piece, struct move_coord* move_coord)
{
	if (piece->type != PAWN)
	{
		return NULL;
	}
	struct move_pos move_pos;
	coord_to_pos(&move_pos.from, &move_coord->from);
	coord_to_pos(&move_pos.to, &move_coord->to);
	if ((piece->color == WHITE && move_pos.to.p[0] == 0) || (piece->color == BLACK && move_pos.to.p[0] == 7))
	{
		return piece;
	}
	return NULL;
}

void move_piece(struct board* board, struct move_coord* move)
{
	struct pos pos_from, pos_to;
	coord_to_pos(&pos_from, &move->from);

	struct piece* piece = get_piece(board, &pos_from);

	coord_to_pos(&pos_to, &move->to);
	struct piece* piece_to = get_piece(board, &pos_to);

	board->squares[pos_to.p[0]][pos_to.p[1]].piece = piece;
	board->squares[pos_from.p[0]][pos_from.p[1]].piece = NULL;

	piece->pos = pos_to;
	piece->has_moved = 1;

	struct piece* rook;
	if ((rook = move_is_castling(board, piece, move)) != NULL)
	{
		struct pos rook_new_pos;
		if (rook->pos.p[1] == 0)
		{
			rook_new_pos = (struct pos){ rook->pos.p[0], rook->pos.p[1] + 3 };
		}
		else
		{
			rook_new_pos = (struct pos){ rook->pos.p[0], rook->pos.p[1] - 2 };
		}
		board->squares[rook_new_pos.p[0]][rook_new_pos.p[1]].piece = rook;
		board->squares[rook->pos.p[0]][rook->pos.p[1]].piece = NULL;

		rook->pos = rook_new_pos;
		rook->has_moved = 1;
	}
	struct piece* pawn;
	if ((pawn = move_is_promotion(board, piece, move)) != NULL)
	{
		// only allow promotion to queen for now
		struct piece* queen = piece_create(pawn->color, QUEEN, move->to);
		struct ll_node* active_pieces = board->active_pieces[pawn->color];
		struct ll_node* piece_node = ll_find(active_pieces, pawn);
		ll_remove(piece_node);
		free(pawn);
		board->squares[pos_to.p[0]][pos_to.p[1]].piece = queen;
		ll_add(active_pieces, queen);
	}

	update_valid_moves(board);

	enum color other_player = get_other_player(piece->color);

	board->kings[other_player]->is_in_check = move_did_put_king_in_check(board, piece->color);

	if (piece_to != NULL)
	{
		struct ll_node* piece_node = ll_find(board->active_pieces[piece_to->color], piece_to);
		ll_remove(piece_node);
		free(piece_to);
	}
}

void update_valid_moves_piece(void* value, void* data)
{
	struct piece* piece = value;
	struct board* board = data;
	reset_valid_moves(piece);
	if (piece->valid_moves_update_func != NULL)
	{
		piece->valid_moves_update_func(board, piece);
	}
	update_is_giving_check(board, piece);
}

void update_valid_moves(struct board* board)
{
	for (int i = 0; i < 2; i++)
	{
		ll_apply(board->active_pieces[i], update_valid_moves_piece, board);
	}
}

int move_puts_king_in_check(struct board* board, enum color current_player, struct move_coord* move)
{
	int result = 0;
	struct board* board2 = board_copy(board);

	move_piece(board2, move);

	result = move_did_put_king_in_check(board2, current_player);

	board_free(board2);
	return result;
}

int validate_move(struct game_status* status, struct board* board, struct move_coord* move)
{
	struct pos pos_from;
	struct pos pos_to;

	coord_to_pos(&pos_from, &move->from);
	if (!is_within_boundaries(&pos_from))
	{
		printf("Invalid from coord: %c%c\n", move->from.c[0], move->from.c[1]);
		return 0;
	}
	coord_to_pos(&pos_to, &move->to);
	if (!is_within_boundaries(&pos_to))
	{
		printf("Invalid to coord: %c%c\n", move->to.c[0], move->to.c[1]);
		return 0;
	}
	struct piece* piece_from = get_piece(board, &pos_from);
	if (piece_from == NULL)
	{
		printf("Invalid move, no piece at %c%c\n", move->from.c[0], move->from.c[1]);
		return 0;
	}
	if (piece_from->color != status->current_player)
	{
		printf("Piece not owned by player: %s at %c%c\n", piece_from->name, move->from.c[0], move->from.c[1]);
		return 0;
	}
	if (!piece_from->valid_moves[pos_to.p[0]][pos_to.p[1]])
	{
		printf("Can't move %s at %c%c to %c%c, invalid move\n", piece_from->name, move->from.c[0], move->from.c[1], move
			->to.c[0], move->to.c[1]);
		return 0;
	}
	struct piece* piece_to = get_piece(board, &pos_to);
	if (piece_to != NULL && piece_to->color == status->current_player)
	{
		printf("Can't move %s at %c%c to %c%c, square is occupied\n", piece_from->name, move->from.c[0], move->from
			.c[1], move->to.c[0], move->to.c[1]);
		return 0;
	}
	if (move_puts_king_in_check(board, status->current_player, move))
	{
		printf("Can't move %s at %c%c to %c%c, king would be in check\n", piece_from->name, move->from.c[0], move->from
			.c[1], move->to.c[0], move->to.c[1]);
		return 0;
	}
	return 1;
}