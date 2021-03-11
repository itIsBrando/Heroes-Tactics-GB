#ifndef MENU_H
#define MENU_H

#include "structs.h"

void mnu_choose_teams_init(match_t *);
void mnu_choose_map_init();

void mnu_cursor_init(uint8_t maxSize, void (*onchange)(void) );
void map_load_from_data(uint8_t *data, uint8_t w, uint8_t h);
void mnu_cursor_up();
void mnu_cursor_down();
#endif