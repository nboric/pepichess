//
// Created by Neven Boric on 3/7/23.
//
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "prediction_engine.h"
#include "move_logic.h"

struct scored_move
{
	struct move_pos move;
	double predicted_score;
};

void add_all_moves(struct ll_node* move_list, struct board* board, struct piece* piece)
{
	struct pos pos_from = piece->pos;
	struct coord coord_from;
	pos_to_coord(&coord_from, &pos_from);
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (piece->valid_moves[i][j])
			{
				struct pos pos_to = (struct pos){ i, j };
				struct coord coord_to;
				pos_to_coord(&coord_to, &pos_to);
				struct move_coord move_coord = (struct move_coord){ coord_from, coord_to };
				if (move_puts_own_king_in_check(board, piece->color, &move_coord))
				{
					continue;
				}
				struct move_pos* move = malloc(sizeof(struct move_pos));
				*move = (struct move_pos){ pos_from, pos_to };
				ll_add(move_list, move);
			}
		}
	}
}

struct scored_move*
predict_best_move_rec(struct board* board, struct move_pos previous_move, enum color current_player, double alpha, double beta, int max_depth)
{
	struct scored_move* result;
	if (max_depth == 0)
	{
		result = malloc(sizeof(struct scored_move));
		result->move = previous_move;
		result->predicted_score = calc_score(board, current_player);
		return result;
	}

	struct ll_node* move_list = ll_create();

	for (struct ll_node* ln = board->active_pieces[current_player]->next; ln != NULL; ln = ln->next)
	{
		struct piece* piece = ln->value;
		add_all_moves(move_list, board, piece);
	}

	enum color next_player = get_other_player(current_player);
	struct move_pos* winner = NULL;

	double best_score = INT32_MIN;

	for (struct ll_node* n = move_list->next; n != NULL; n = n->next)
	{
		struct move_pos* move = n->value;
		struct coord coord_from;
		struct coord coord_to;
		pos_to_coord(&coord_from, &move->from);
		pos_to_coord(&coord_to, &move->to);
		struct move_coord move_coord = (struct move_coord){ coord_from, coord_to };
		struct board* board2 = board_copy(board);
		move_piece(board2, &move_coord);

		update_valid_moves(board2);
		struct scored_move* next_move = predict_best_move_rec(board2, *move, next_player, -beta, -alpha, max_depth - 1);
		board_free(board2);

		double score;
		if (next_move != NULL)
		{
			score = -next_move->predicted_score;
		}
		else
		{
			score = 40;
		}
		free(next_move);
		if (score > best_score)
		{
			best_score = score;
			winner = move;
		}
		if (best_score > alpha)
		{
			alpha = best_score;
		}
		if (alpha >= beta)
		{
			break;
		}
	}
	struct scored_move* scored_winner = NULL;
	if (winner != NULL)
	{
		scored_winner = malloc(sizeof(struct scored_move));
		scored_winner->move = *winner;
		scored_winner->predicted_score = best_score;
		return scored_winner;
	}
	ll_free(move_list, free);
	return scored_winner;
}

struct move_coord predict_best_move(struct game_status* status)
{
	struct move_coord result;
	struct move_pos empty_move;
	struct scored_move* best_move = predict_best_move_rec(status->board, empty_move, status->current_player, INT32_MIN, INT32_MAX, 5);
	pos_to_coord(&result.from, &best_move->move.from);
	pos_to_coord(&result.to, &best_move->move.to);
	free(best_move);
	return result;
}
