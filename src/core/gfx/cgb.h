#ifndef CGB_H
#define CGB_H

#include "../../structs.h"


#define CGB_BG_DEFAULT  0
#define CGB_BG_WATER    1
#define CGB_BG_TREE     2
#define CGB_BG_HEART    3
#define CGB_BG_SAND     3
#define CGB_BG_BRIDGE   4

#define CGB_SPR_BLUE    0
#define CGB_SPR_RED     1
#define CGB_SPR_GRAY    2

bool is_cgb();
void cgb_init();
void cgb_map();
void cgb_diamond(uint8_t);
void cgb_hide_diamond();
uint8_t cgb_get_team_palette(team_t *team);
void cgb_write_tile(uint8_t x, uint8_t y);

void cgb_draw_hud();
void cgb_draw_battle();
void cgb_draw_heal();

void cgb_cleanup_battle();
#endif