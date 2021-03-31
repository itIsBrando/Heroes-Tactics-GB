#ifndef MAP_H
#define MAP_H

#include "defines.h"
#include "structs.h"

#define MAPS_TOTAL 5

extern uint8_t *all_maps[];
extern const uint8_t map_widths[];
extern const uint8_t map_heights[];


void map_draw();
bool map_has_fog();
map_t *map_load_from_data(uint8_t *data, uint8_t w, uint8_t h, bool useFog);
void map_load(map_t *, bool);
void map_blit(map_t *);
bool map_in_bounds(uint8_t x, uint8_t y);
uint8_t map_get_pos(position_t *position);
uint8_t map_get(uint8_t x, uint8_t y);
uint8_t map_get_with_fog(uint8_t x, uint8_t y);
uint8_t map_fget(uint8_t tile);
bool map_is_solid(uint8_t x, uint8_t y);

void map_update_fog();
void map_generate_fog();
void map_fog_hide_units(team_t *);
void map_changed_turns();
void map_init_spawn(team_t *, bool);

map_t *map_get_active();
uint8_t map_get_width();
uint8_t map_get_height();

#endif