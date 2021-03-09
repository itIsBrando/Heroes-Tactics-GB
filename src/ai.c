#include "ai.h"
#include "game.h"
#include "units.h"
#include "diamond.h"
#include "path.h"
#include "structs.h"

#include <gb/gb.h>

/**
 * Does a turn for the AI's unit
 * @param unit
 */
void ai_do_turn(unit_t *unit)
{
    // don't do anything if we are dead
    if(unit->isDead)
        return;

    unit_t *target = NULL;
    uint8_t size;
    position_t targetPos, aiPos;

    // if we are running, then choose a POINT to move to rather than a UNIT
    if(unit->strategy == AI_TARGET_RUN) {
        ai_get_target_position(&targetPos, unit);
    } else {
        target = ai_get_target(unit, unit->strategy);
        targetPos.x = target->row;
        targetPos.y = target->column;
    }
    aiPos.x = unit->row;
    aiPos.y = unit->column;

    position_t *steps = pf_find(&aiPos, &targetPos, &size);

    // stop if we cannot make a path
    if(!steps)
        return;
    
    // now move the enemy
    uint8_t iter = 0; // keeps track of the number of steps we've moved

    for(int8_t i = size-1; i > 0; i--)
    {
        unit_move_to(unit, steps[i].x, steps[i].y);

        for(uint8_t j = 0; j < 15; j++)
            wait_vbl_done();
        if(++iter > unit->stats.movePoints)
            break;
        
        // if we can attack from a distance
        if(unit_get_distance(unit, target) <= unit->stats.damageRadius)
            break;
    }

    ai_check_attack(unit, target);
}

/**
 * @param unit AI unit to move
 * @param strategy strategy that the AI unit should take to attack
 * @returns an opponent unit to attack or NULL
 */
unit_t *ai_get_target(unit_t *unit, ai_strat_t strategy)
{
    team_t *opponent = mth_get_opponent();
    uint8_t i = 0;
    uint8_t min = 255, bestIndex = 0;
    team_t *team;

    switch (strategy)
    {
    case AI_TARGET_RUN:
        // target our healer
        team = mth_get_current_team();
        
        for(uint8_t i = 0; i < team->size; i++)
        {
            // find a healer
            if(team->units[i]->type == UNIT_TYPE_HEALER && team->units[i] != unit)
            {
                return team->units[i];
            }
        }
        
        for(uint8_t i = 0; i < team->size; i++)
        {
            // find a healer
            if(team->units[i] != unit)
                return team->units[i];
        }

    case AI_TARGET_NONE:
        unit->strategy = AI_TARGET_NEAR;
    case AI_TARGET_HEAL: // @todo
    case AI_TARGET_NEAR:
        for(; i < opponent->size; i++)
        {
            // skip if this unit is dead
            if(opponent->units[i]->isDead)
                continue;
            
            // otherwise compare distances
            const uint8_t dist = unit_get_distance(opponent->units[i], unit);
            if(dist < min) {
                bestIndex = i;
                min = dist;
            }
        }

        // if the target wil kill the AI unit, then run from it
        unit_t *target = opponent->units[bestIndex];
        if(target->stats.damagePoints >= unit->stats.health)
        {
            unit->strategy = AI_TARGET_RUN;
            return ai_get_target(unit, unit->strategy);
        } else 
            return target;
            
    // target the unit that can be killed the easiest
    case AI_TARGET_ATK:
        for(; i < opponent->size; i++)
        {
            const uint8_t hp = opponent->units[i]->stats.health;
            if(hp < min && !opponent->units[i]->isDead) {
                bestIndex = i;
                min = hp;
            }
            
        }
        return opponent->units[bestIndex];
    }

    return NULL;
}


/**
 * A point that the AI unit should approach
 * @param position updated position
 * @param unit AI unit
 * @param strategy the goal
 */
void ai_get_target_position(position_t *position, unit_t *unit)
{
    unit_t *target = ai_get_target(unit, AI_TARGET_NEAR);
    uint8_t xGoal, yGoal;
    uint8_t bestDist = 0;
    uint8_t bestIndex = 0;

    // this will hopefully never trigger
    if(!target)
        return;

    const int8_t dx[] = {-1, 1, 0, 0};
    const int8_t dy[] = {0, 0, -1, 1};

    for(uint8_t i = 0; i < 4; i++)
    {
        uint8_t curDist;
        xGoal = unit->row + unit->stats.movePoints * dx[i];
        yGoal = unit->column + unit->stats.movePoints * dy[i];

        curDist = getDistance(NULL, xGoal, yGoal, target->row, target->column);

        if(curDist > bestDist)
        {
            bestDist = curDist;
            bestIndex = i;
        }
    }

    xGoal = unit->row + unit->stats.movePoints * dx[bestIndex];
    yGoal = unit->column + unit->stats.movePoints * dy[bestIndex];

    position->x = xGoal;
    position->y = yGoal;

}


/**
 * Allows an AI unit to attack if it can
 * @param unit AI unit
 * @param target can be `NULL`
 * @returns false if we could not attack, otherwise true
 */
bool ai_check_attack(unit_t *unit, unit_t *target)
{
    if(!target)
        return false;

    if(target->stats.damagePoints >= unit->stats.health)
        return false;

    // if AI will kill target, then proceed with the kill
    if(!(unit->stats.damagePoints >= target->stats.health))
        // if the target is in range and would kill AI, do not attack
        if(!(target->stats.health - unit->stats.damagePoints <= 0) && unit_get_distance(unit, target) > unit->stats.damageRadius)
            return false;
    
    unit_attack(unit, target);

    return true;
}


/**
 * @returns true if the AI unit should attack `other`
 */
bool ai_should_attack(unit_t *unit, unit_t *other)
{
    // if our attack can kill enemy and we are in range
    if(unit->stats.damagePoints >= other->stats.health
     && unit_in_atk_range(unit, other))
        return true;
    
    return false;
}

/**
 * @returns true if the AI unit is weak relative to `other` and should run
 */
bool ai_should_run(unit_t *unit, unit_t *other)
{
    // run away if `other` will kill AI
    if(other->stats.damagePoints > unit->stats.health && unit_in_atk_range(other, unit))
        return true;

    return false;
}