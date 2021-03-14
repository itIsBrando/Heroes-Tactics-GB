#ifndef AI_H
#define AI_H

#include "structs.h"

typedef struct {
    unit_t *unit; // pointer to unit that has this priority
    int8_t priority; // higher numbers are less favorable
} heuristic_t;


void ai_do_turn(unit_t *);
bool ai_check_attack(unit_t *unit, unit_t *target);
bool ai_check_heal(unit_t *unit);

void ai_get_heursitic_target(unit_t *unit, heuristic_t *t, uint8_t size);
void ai_get_destination_position(position_t *, unit_t *);
unit_t *ai_get_target(unit_t *, ai_strat_t);

void ai_run_from(position_t *, unit_t *, unit_t *);

bool ai_should_run(unit_t *unit, unit_t *other);
bool ai_would_kill(unit_t *unit, unit_t *other);

#endif