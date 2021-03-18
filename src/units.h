#ifndef UNITS_H
#define UNITS_H

#include "structs.h"

extern unit_t UNIT_ARCHER;
extern unit_t UNIT_BRAWN;
extern unit_t UNIT_HEALER;


/**
 * @param a first number
 * @param b other number
 * @returns the smallest value between `a` and `b`
 */
uint8_t min(uint8_t a, uint8_t b);

/**
 * Returns the Manhattan distance
 * @param remainder pointer to a single byte that will hold the remainder, or NULL if unneeded
 */
uint8_t getDistance(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);


/**
 * Creates a new unit. *MUST BE FREED*
 * @param type type of unit to create see *type_of_unit*
 */
unit_t *unit_new(type_of_unit type);

/**
 * Destroys a unit created by *unit_new()*
 * @param unit destroys this unit
 */
void unit_destroy(unit_t *unit);

/**
 * @param team pointer to the team to check
 * @param x row to check
 * @param y column to check
 * @returns first occurance of a unit at a specific coordinate, otherwise NULL
 */
unit_t *unit_get(team_t *team, uint8_t x, uint8_t y);

/**
 * @param x row to check
 * @param y column to check
 * @returns first occurance of a unit FROM ANY TEAM
 */
unit_t *unit_get_any(uint8_t x, uint8_t y);

unit_t *unit_get_healer(team_t *team);

char *unit_get_name(unit_t *unit);

/**
 * Changes the OAM tile for this unit
 * @param unit unit to update
 */
void unit_upd_sprite_tile(unit_t *unit);

/**
 * @param unit sets the position and tile of this unit
 */
void unit_draw(unit_t *unit);

/**
 * @param unit sets the position and tile of this unit
 * @param x pixel x
 * @param y pixel y
 */
void unit_draw_at(unit_t *unit, uint8_t x, uint8_t y);

/**
 * @param unit hides this unit until `unit_draw` is called again
 */
void unit_hide(unit_t *unit);

/**
 * Checks to see if (x, y) is a valid place to move the unit
 * @returns true if the tile at (x, y) is empty, otherwise false
 */
bool unit_can_move_to(uint8_t x, uint8_t y);

/**
 * Moves a unit to a coordinate
 * - checks if a unit can move to this position too
 * - redraws this unit too
 * @param unit 
 * @param x row
 * @param y column
 * @returns true if the unit moved, false otherwise
 */
bool unit_move_to(unit_t *unit, uint8_t x, uint8_t y);


/**
 * Moves a unit to a position using path finding to achieve it.
 * @param unit unit to move
 * @param destination position to end at
 * @returns false if a path could not be generated
 */
bool unit_move_path_find(unit_t *unit, position_t *destination);

void unit_set_pos(unit_t *unit, uint8_t x, uint8_t y);

/**
 * Gets the distance between two units
 * @returns integer distance
 */
uint8_t unit_get_distance(unit_t *u1, unit_t *u2);

/**
 * Finds the unit closest to `unit` in `opponent`
 * @returns returns that unit
 */
unit_t *unit_find_nearest(team_t *opponent, unit_t *unit);

/**
 * Used to animate units of a team
 */
void unit_vbl_int();

/**
 * Animates all of the units in a team
 * @param team team to animate
 */
void unit_animate(team_t *team);


/**
 * Call to graphically indicate that two units are engaging. Unit order is irrelevant
 * @param u1 one unit
 * @param u2 second unit
 */
void unit_engage(unit_t *u1, unit_t *u2);

/**
 * Attacks an enemy and allows enemy to counterattack
 * @param attacker does damage to defender. Finishes this unit's turn
 * @param defender takes damage
 * @returns true if `defender` dies, otherwise false
 */
bool unit_attack(unit_t *attacker, unit_t *defender);

bool unit_heal(unit_t *unit, unit_t *healer);


void unit_move_diamond(unit_t *unit);

void unit_atk_diamond(unit_t *unit);

void unit_hide_triangle();

bool unit_in_atk_range(unit_t *unit, unit_t *other);

void unit_draw_paletted(unit_t *unit, team_t *team);
#endif