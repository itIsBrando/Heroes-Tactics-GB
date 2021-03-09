#ifndef AI_H
#define AI_H

#include "structs.h"

void ai_do_turn(unit_t *);
bool ai_check_attack(unit_t *unit, unit_t *target);
unit_t *ai_get_target(unit_t *, ai_strat_t);
void ai_get_target_position(position_t *, unit_t *);

bool ai_should_run(unit_t *unit, unit_t *other);
bool ai_should_attack(unit_t *unit, unit_t *other);

#endif