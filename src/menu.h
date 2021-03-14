#ifndef MENU_H
#define MENU_H

#include "structs.h"

void mnu_choose_teams_init(match_t *);
void mnu_choose_map_init();

void mnu_cursor_init(uint8_t maxSize, void (*onchange)(void) );
void mnu_cursor_up();
void mnu_cursor_down();
#endif