#ifndef CGB_H
#define CGB_H

#include "structs.h"

bool is_cgb();
void cgb_init();
void cgb_map();
void cgb_diamond(uint8_t x, uint8_t y);
void cgb_hide_diamond();
uint8_t cgb_get_team_color(team_t *team);
void cgb_write_tile(uint8_t x, uint8_t y);

void cgb_draw_hud();
void cgb_draw_battle();

void cgb_cleanup_battle();
#endif