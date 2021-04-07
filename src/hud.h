#ifndef HUD_H
#define HUD_H

typedef enum {
    HUD_ACTION_MOVE,
    HUD_ACTION_ATK,
    HUD_ACTION_PEAK,
    HUD_ACTION_RANGE
} hud_action_t;

void hud_draw_hotbar(team_t *);
void hud_draw_health(unit_t *unit, uint8_t x, uint8_t y, const bool useWindow);

bool hud_confirm_end_turn();

void hud_show_action(const hud_action_t);
void hud_hide_action();

void hud_show_details(uint8_t x, uint8_t y);
void hud_hide_details();

void hud_force_hide_warn();
void hud_warn(char *);
void hud_vbl_int();

uint8_t hud_unit_attack_menu();

void hud_change_turn_banner();
void hud_change_turn_banner_cleanup();

void hud_show_unit_control_type(const team_t *team);
void hud_hide_unit_control_type(const team_t *team);
#endif