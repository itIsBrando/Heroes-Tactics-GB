#ifndef MAP_H
#define MAP_H

#include "defines.h"
#include "structs.h"

extern uint8_t *all_maps[];

typedef struct {
    uint8_t width, height;
    uint8_t *data;
} map_t;

void map_draw();
void map_load(map_t *);
bool map_in_bounds(uint8_t x, uint8_t y);
uint8_t map_get_pos(position_t *position);
uint8_t map_get(uint8_t x, uint8_t y);
uint8_t map_fget(uint8_t tile);


#endif