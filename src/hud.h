#ifndef HUD_H
#define HUD_H

typedef enum {
    HUD_ACTION_MOVE,
    HUD_ACTION_ATK,
} hud_action_t;

void hud_draw_hotbar(team_t *);
void hud_draw_health(unit_t *unit, uint8_t x, uint8_t y, const bool useWindow);

void hud_show_action(hud_action_t);
void hud_hide_action();

void hud_show_details(uint8_t x, uint8_t y);
void hud_hide_details();

void hud_warn(char *);
void hud_vbl_int();

#endif