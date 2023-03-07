//
// Created by Neven Boric on 2/25/23.
//

#ifndef _PIECE_LOGIC_H_
#define _PIECE_LOGIC_H_

#include "common.h"

void move_validation_pawn(struct board *board, struct piece *piece);
void move_validation_rook(struct board *board, struct piece *piece);
void move_validation_bishop(struct board* board, struct piece* piece);
void move_validation_queen(struct board* board, struct piece* piece);
void move_validation_knight(struct board* board, struct piece* piece);
void move_validation_king(struct board* board, struct piece* piece);

#endif //_PIECE_LOGIC_H_
