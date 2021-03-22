#ifndef CURSOR_H
#define CURSOR_H

#include "structs.h"

void cur_move(int8_t, int8_t);
void cur_draw();
void cur_hide();
void cur_show();
void cur_init();
void cur_destroy();
void cur_animate();

void cur_vbl();

uint8_t cur_get_x();
uint8_t cur_get_y();

direction_t cur_get_direction(position_t *pos1, position_t *pos2);
#endif