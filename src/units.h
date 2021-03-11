#ifndef UNITS_H
#define UNITS_H

#include "structs.h"

extern unit_t UNIT_ARCHER;
extern unit_t UNIT_BRAWN;
extern unit_t UNIT_HEALER;


uint8_t min(uint8_t a, uint8_t b);
uint8_t getDistance(uint8_t *remainder, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);


unit_t *unit_new(type_of_unit type);

void unit_destroy(unit_t *unit);

unit_t *unit_get(team_t *team, uint8_t x, uint8_t y);

unit_t *unit_get_any(uint8_t x, uint8_t y);


char *unit_get_name(unit_t *unit);

void unit_draw(unit_t *unit);

void unit_draw_at(unit_t *unit, uint8_t x, uint8_t y);

void unit_hide(unit_t *unit);

bool unit_can_move_to(uint8_t x, uint8_t y);

bool unit_move_to(unit_t *unit, uint8_t x, uint8_t y);

void unit_set_pos(unit_t *unit, uint8_t x, uint8_t y);

uint8_t unit_get_distance(unit_t *u1, unit_t *u2);

unit_t *unit_find_nearest(team_t *opponent, unit_t *unit);


bool unit_attack(unit_t *attacker, unit_t *defender);

void unit_heal(unit_t *unit, uint8_t hp);


void unit_move_diamond(unit_t *unit);

void unit_atk_diamond(unit_t *unit);

void unit_hide_triangle();

bool unit_in_atk_range(unit_t *unit, unit_t *other);

#endif