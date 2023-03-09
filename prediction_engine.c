//
// Created by Neven Boric on 3/7/23.
//
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "prediction_engine.h"
#include "move_logic.h"

struct move_list
{
	struct board* board;
	struct move_pos move;
	double predicted_score;
	struct move_list* next;
};

struct move_list* move_list_node_create(struct board* board)
{
	struct move_list* node = malloc(sizeof(struct move_list));
	node->next = NULL;
	node->board = board_copy(board);
	return node;
}

void move_list_node_free(struct move_list* node)
{
	board_free(node->board);
	if (node->next != NULL)
	{
		move_list_node_free(node->next);
	}
	free(node);
}

struct move_list* add_all_moves(struct move_list* node, struct piece* piece)
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
				if (move_puts_own_king_in_check(node->board, piece->color, &move_coord))
				{
					continue;
				}
				struct move_list* new_node = move_list_node_create(node->board);
				new_node->move = (struct move_pos){ pos_from, pos_to };
				node->next = new_node;
				node = new_node;
			}
		}
	}
	return node;
}

struct move_list*
predict_best_move_rec(struct move_list* node, enum color current_player, int max_depth)
{
	struct move_list* n = node;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			struct piece* piece;
			if ((piece = node->board->squares[i][j].piece) != NULL)
			{
				if (piece->color == current_player)
				{
					n = add_all_moves(n, piece);
				}
			}
		}
	}

	enum color next_player = get_other_player(current_player);
	double max_score = INT32_MIN;
	struct move_list* winner = NULL;

	// printf("S%c%d,", current_player == WHITE ? 'w' : 'b', max_depth);

	for (n = node->next; n != NULL; n = n->next)
	{
		struct coord coord_from;
		struct coord coord_to;
		pos_to_coord(&coord_from, &n->move.from);
		pos_to_coord(&coord_to, &n->move.to);
		struct move_coord move_coord = (struct move_coord){ coord_from, coord_to };
		n->predicted_score = move_piece(n->board, &move_coord);

		// printf("%c%d,%c%c->%c%c,%.3f,", current_player == WHITE ? 'w' : 'b', max_depth, move_coord.from.c[0], move_coord.from.c[1], move_coord.to.c[0], move_coord.to.c[1], n->predicted_score);

		update_valid_moves(n->board);
		if (max_depth > 0)
		{
			struct move_list* next_turn_move_list = move_list_node_create(n->board);
			// printf(",%c%c->%c%c:\n", move_coord.from.c[0], move_coord.from.c[1], move_coord.to.c[0], move_coord.to.c[1]);
			struct move_list* next_move = predict_best_move_rec(next_turn_move_list, next_player, max_depth - 1);
			if (next_move != NULL)
			{
				n->predicted_score -= next_move->predicted_score;
			}
			else
			{
				n->predicted_score = 40;
			}
			move_list_node_free(next_turn_move_list);
			// printf("%c%d,%c%c->%c%c,%.3f,", current_player == WHITE ? 'w' : 'b', max_depth, move_coord.from.c[0], move_coord.from.c[1], move_coord.to.c[0], move_coord.to.c[1], n->predicted_score);
		}
		if (n->predicted_score > max_score)
		{
			// printf("%c%d,%c%c->%c%c,%.3f,", current_player == WHITE ? 'w' : 'b', max_depth, move_coord.from.c[0], move_coord.from.c[1], move_coord.to.c[0], move_coord.to.c[1], n->predicted_score);
			max_score = n->predicted_score;
			winner = n;
		}
	}
	if (winner == NULL) // no valid moves
	{
		return NULL;
	}
	// printf("E%c%d ", current_player == WHITE ? 'w' : 'b', max_depth);
	struct coord coord_from;
	struct coord coord_to;
	pos_to_coord(&coord_from, &winner->move.from);
	pos_to_coord(&coord_to, &winner->move.to);
	// printf("winner: %c%c->%c%c,%.3f\n", coord_from.c[0], coord_from.c[1], coord_to.c[0], coord_to.c[1], winner->predicted_score);
	return winner;
}

struct move_coord predict_best_move(struct game_status* status)
{
	struct move_coord result;
	struct move_list* move_list = move_list_node_create(status->board);
	struct move_list* best_move = predict_best_move_rec(move_list, status->current_player, 3);
	pos_to_coord(&result.from, &best_move->move.from);
	pos_to_coord(&result.to, &best_move->move.to);
	move_list_node_free(move_list);
	return result;
}
