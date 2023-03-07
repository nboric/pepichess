//
// Created by Neven Boric on 3/6/23.
//

#ifndef _MOVE_LOGIC_H_
#define _MOVE_LOGIC_H_

#include "common.h"

int is_within_boundaries(struct pos *pos);
void reset_valid_moves(struct piece* piece);
void update_is_giving_check(struct board* board, struct piece* piece);
int validate_move(struct game_status* status, struct board* board, struct move* move);
void move_piece(struct board* board, struct move* move);
void update_valid_moves(struct board* board);

#endif //_MOVE_LOGIC_H_
