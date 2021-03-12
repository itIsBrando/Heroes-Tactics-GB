#ifndef DIAMOND_H
#define DIAMOND_H

#include "defines.h"
#include "structs.h"

extern uint8_t tri_active_diamond[MAP_MAX_SIZE];

void tri_make(uint8_t, uint8_t, uint8_t);

uint8_t tri_get_width();
uint8_t tri_get_height();
uint8_t tri_get(uint8_t x, uint8_t y);
void tri_set(uint8_t x, uint8_t y, uint8_t v);

void tri_clip();
void tri_draw(const uint8_t);
void tri_hide();

#endif