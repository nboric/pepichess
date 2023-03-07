#include <stdio.h>
#include <ctype.h>
#include "common.h"
#include "move_logic.h"
#include "init.h"

void print_square(struct square* square)
{
	if (square->color == WHITE)
	{
		printf("\x1b[47m");
	}
	else
	{
		printf("\x1b[45m");
	}
	if (square->piece == NULL)
	{
		printf("   ");
	}
	else
	{
		printf(" %s ", square->piece->symbol);
	}
}

void print_board(struct board* board)
{
	printf("  ");
	for (int i = 0; i < 8; i++)
	{
		printf(" %c ", 'A' + i);
	}
	printf("\n");
	for (int i = 0; i < 8; i++)
	{
		printf("%d ", 8 - i);
		for (int j = 0; j < 8; j++)
		{
			print_square(&board->squares[i][j]);
		}
		printf("\x1b[m");
		printf(" %d\n", 8 - i);
	}
	printf("  ");
	for (int i = 0; i < 8; i++)
	{
		printf(" %c ", 'A' + i);
	}
	printf("\n");
}

struct move get_input()
{
	struct move result;

	char c1, c2;
	char i1, i2;
	scanf(" %c%c%c%c", &c1, &i1, &c2, &i2);

	result.from = (struct coord){ toupper(c1), i1 };
	result.to = (struct coord){ toupper(c2), i2 };

	return result;
}

int main()
{
	struct board* board = create_board();
	struct game_status game_status = { .ended = 0, .current_player = WHITE, .board = board };
	init_player(&game_status.players[0], WHITE);
	add_pieces_to_board(board, &game_status.players[0]);
	init_player(&game_status.players[1], BLACK);
	add_pieces_to_board(board, &game_status.players[1]);

	while (!game_status.ended)
	{
		int move_valid = 0;
		struct move move;
		while (!move_valid)
		{
			print_board(board);
			printf("Player %s, enter move:\n", game_status.current_player == WHITE ? "white" : "black");
			move = get_input();
			move_valid = validate_move(&game_status, board, &move);
		}
		move_piece(board, &move);
		update_valid_moves(board);
		game_status.current_player = game_status.current_player == WHITE ? BLACK : WHITE;
	}
	return 0;
}
