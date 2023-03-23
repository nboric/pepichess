//
// Created by Neven Boric on 3/7/23.
//
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "prediction_engine.h"
#include "move_logic.h"

#define MAX_DEPTH 4
// #define DEBUG

#ifdef DEBUG
#define DEBUG_ALL_LEVELS 1
#define PRINT_DEBUG(x) if (DEBUG_ALL_LEVELS || max_depth == MAX_DEPTH) {for (int k=0;k<(MAX_DEPTH-max_depth);k++)printf("  ");x;}
#else
#define PRINT_DEBUG(x) ;
#endif

struct scored_move
{
	struct move_pos move;
	double predicted_score;
};

struct move_status
{
	struct board* board;
	struct move_pos move;
	double score;
};

int add_all_moves(struct ll_node* move_list, struct board* board, struct piece* piece)
{
	struct pos pos_from = piece->pos;
	struct coord coord_from;
	pos_to_coord(&coord_from, &pos_from);
	int n_moves = 0;
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
				if (move_puts_king_in_check(board, piece->color, &move_coord))
				{
					continue;
				}
				struct move_status* move_status = malloc(sizeof(struct move_status));
				move_status->board = board_copy(board);
				move_status->move = (struct move_pos){ pos_from, pos_to };
				move_piece(move_status->board, &move_coord);

				// first we store shallow score, for sorting only
				move_status->score = calc_score(move_status->board, piece->color);
				ll_add(move_list, move_status);
				n_moves++;
			}
		}
	}
	return n_moves;
}

int move_shallow_compare(const void* m1, const void* m2)
{
	const struct move_status* move1 = *(struct move_status**)m1;
	const struct move_status* move2 = *(struct move_status**)m2;
	if (move1->score == move2->score)
	{
		return 0;
	}
	if (move1->score > move2->score)
	{
		return -1;
	}
	return 1;
}

struct scored_move*
predict_best_move_rec(struct board* board, enum color current_player, double alpha, double beta, int max_depth)
{
	PRINT_DEBUG(printf("max_depth=%d,current_player=%d,alpha=%f,beta=%f\n", max_depth, current_player, alpha, beta))
	struct scored_move* result;
	if (max_depth == 0)
	{
		result = malloc(sizeof(struct scored_move));
		result->predicted_score = calc_score(board, current_player);
		PRINT_DEBUG(printf("final score=%f\n", result->predicted_score))
		return result;
	}

	struct ll_node* move_list = ll_create();
	int n_moves = 0;
	int n_evaluated_moves = 0;
	for (struct ll_node* ln = board->active_pieces[current_player]->next; ln != NULL; ln = ln->next)
	{
		struct piece* piece = ln->value;
		n_moves += add_all_moves(move_list, board, piece);
	}

	// sort according to shallow score to improve likelihood of alpha-beta pruning
	// move from linked list to array to sort
	struct move_status** all_valid_moves = malloc(n_moves * sizeof(struct move_status*));
	struct ll_node* n = move_list->next;

	PRINT_DEBUG(printf("n_moves=%d\n", n_moves))

	for (int i = 0; i < n_moves; i++)
	{
		struct move_status* move_status = n->value;
		all_valid_moves[i] = move_status;
#ifdef DEBUG
		struct move_coord move;
		pos_to_coord(&move.from, &move_status->move.from);
		pos_to_coord(&move.to, &move_status->move.to);
		PRINT_DEBUG(printf("move (%c%c->%c%c), shallow score=%f\n", move.from.c[0], move.from.c[1], move.to.c[0], move
			.to.c[1], move_status->score))
#endif
		n = n->next;
	}
	ll_free(move_list, NULL);

	qsort(all_valid_moves, n_moves, sizeof(struct move_status*), move_shallow_compare);

	enum color next_player = get_other_player(current_player);
	struct move_pos* winner = NULL;

	double best_score = INT32_MIN;

	for (int i = 0; i < n_moves; i++)
	{
		struct move_status* move_status = all_valid_moves[i];

#ifdef DEBUG
		struct move_coord move;
		pos_to_coord(&move.from, &move_status->move.from);
		pos_to_coord(&move.to, &move_status->move.to);
		PRINT_DEBUG(printf("move (%c%c->%c%c)\n", move.from.c[0], move.from.c[1], move.to.c[0], move.to.c[1]))
#endif

		struct scored_move* next_move = predict_best_move_rec(move_status->board, next_player, -beta, -alpha,
			max_depth - 1);

		double score;
		if (next_move != NULL)
		{
			score = -next_move->predicted_score;
			free(next_move);
		}
		else
		{
			score = 40;
		}
		move_status->score = score;

		PRINT_DEBUG(printf("move (%c%c->%c%c), deep score=%f\n", move.from.c[0], move.from.c[1], move.to.c[0], move.to
			.c[1], move_status->score))

		// now we store the actual (deep) score, so we can randomize between ties
		move_status->score = score;
		n_evaluated_moves++;
		if (score > best_score)
		{
			best_score = score;
			winner = &move_status->move;
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
	PRINT_DEBUG(printf("n_evaluated_moves=%d\n", n_evaluated_moves))
	struct scored_move* scored_winner = NULL;
	if (winner != NULL)
	{
		int n_winners = 0;
		for (int i = 0; i < n_evaluated_moves; i++)
		{
			if (all_valid_moves[i]->score == best_score)
			{
				n_winners++;
			}
		}
		PRINT_DEBUG(printf("best_score=%f, n_winners=%d\n", best_score, n_winners))
		if (n_winners > 1)
		{
			int which_match = round((double)rand() / RAND_MAX * (n_winners - 1));
			PRINT_DEBUG(printf("which_match=%d\n", which_match))
			for (int i = 0, match = 0; i < n_evaluated_moves; i++)
			{
				if (all_valid_moves[i]->score == best_score)
				{
					if (which_match == match)
					{
						if (all_valid_moves[i]->score != best_score)
						{
							printf("error, match %f, max %f\n", all_valid_moves[i]->score, best_score);
						}
						winner = &all_valid_moves[i]->move;
						break;
					}
					match++;
				}
			}
		}
		scored_winner = malloc(sizeof(struct scored_move));
		scored_winner->move = *winner;
		scored_winner->predicted_score = best_score;
	}
	for (int i = 0; i < n_moves; i++)
	{
		board_free(all_valid_moves[i]->board);
		free(all_valid_moves[i]);
	}
	free(all_valid_moves);
	return scored_winner;
}

struct move_coord predict_best_move(struct game_status* status)
{
	struct move_coord result;
	struct scored_move* best_move = predict_best_move_rec(status->board, status
		->current_player, INT32_MIN, INT32_MAX, MAX_DEPTH);
	pos_to_coord(&result.from, &best_move->move.from);
	pos_to_coord(&result.to, &best_move->move.to);
	free(best_move);
	return result;
}
