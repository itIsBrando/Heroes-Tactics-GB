#ifndef AI_H
#define AI_H

#include "structs.h"

typedef enum {
    AI_TARGET_NEAR,
    AI_TARGET_ATK,
    AI_TARGET_HEAL
} ai_strat_t;

void ai_do_turn(unit_t *);
bool ai_check_attack(unit_t *unit, unit_t *target);
unit_t *ai_get_target(unit_t *, ai_strat_t);

#endif