//
// Created by Neven Boric on 3/6/23.
//

#ifndef _INIT_H_
#define _INIT_H_

#include "common.h"

void add_pieces_to_board(struct board* board, struct player* player);
struct piece* piece_create(enum color color, enum piece_id type, struct coord coord);
void player_init(struct player* player, int color);
struct board* board_create();

#endif //_INIT_H_
