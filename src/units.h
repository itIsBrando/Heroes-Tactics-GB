#ifndef UNITS_H
#define UNITS_H

#include "structs.h"

extern unit_t UNIT_ARCHER;
extern unit_t UNIT_BRAWN;
extern unit_t UNIT_HEALER;

uint8_t min(uint8_t a, uint8_t b);

unit_t *unit_new(type_of_unit type);
unit_t *unit_get(team_t *team, uint8_t x, uint8_t y);
unit_t *unit_get_any(uint8_t x, uint8_t y);
void unit_destroy(unit_t *);

char *unit_get_name(unit_t *unit);
void unit_draw(unit_t *unit);
void unit_draw_at(unit_t *unit, uint8_t x, uint8_t y);
void unit_hide(unit_t *unit);
bool unit_move_to(unit_t *unit, uint8_t x, uint8_t y);
void unit_set_pos(unit_t *unit, uint8_t x, uint8_t y);
uint8_t unit_get_distance(unit_t *u1, unit_t *u2);

bool unit_attack(unit_t *attacker, unit_t *defender);
void unit_heal(unit_t *unit, uint8_t hp);

void unit_move_diamond(unit_t *unit);
void unit_atk_diamond(unit_t *unit);
void unit_hide_triangle();

void unit_draw_team(team_t *team);

#endif