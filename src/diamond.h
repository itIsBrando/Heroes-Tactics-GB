#ifndef DIAMOND_H
#define DIAMOND_H

#include "defines.h"
#include "structs.h"

extern uint8_t tri_active_diamond[MAP_SIZE];

void tri_make(uint8_t, uint8_t, uint8_t);

void tri_clip();
void tri_draw(const uint8_t);

#endif