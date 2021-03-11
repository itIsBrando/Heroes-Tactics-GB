#include "ai.h"
#include "game.h"
#include "units.h"
#include "diamond.h"
#include "path.h"
#include "structs.h"
#include "main.h"

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

    uint8_t size;
    position_t targetPos, aiPos;

    targetPos.x = targetPos.y = 0;

    ai_get_destination_position(&targetPos, unit);

    aiPos.x = unit->row;
    aiPos.y = unit->column;

    position_t *steps = pf_find(&aiPos, &targetPos, &size);

    // stop if we cannot make a path
    if(!steps)
        return;
    
    // now move the enemy
    uint8_t iter = 0; // keeps track of the number of steps we've moved
    unit_t *target = unit_find_nearest(mth_get_opponent(), unit);

    for(int8_t i = size-1; i > 0; i--)
    {
        unit_move_to(unit, steps[i].x, steps[i].y);

        for(uint8_t j = 0; j < 15; j++)
            wait_vbl_done();
        if(++iter > unit->stats.movePoints)
            break;
        
        // if we can attack from a distance
        // but if we are running, do not focus on enemy targets
        if(unit->strategy != AI_TARGET_RUN
         && unit_get_distance(unit, target) <= unit->stats.damageRadius)
            break;
    }

    ai_check_attack(unit, target);
}


/**
 * Updates the strategy property for an AI unit
 */
void ai_set_strategy(unit_t *unit)
{
    const team_t *opponent = mth_get_opponent();
    ai_strat_t strat = AI_TARGET_NONE;

    // find the closest enemy
    unit_t *target = unit_find_nearest(opponent, unit);
        
    if(ai_would_kill(unit, target))
        strat = AI_TARGET_ATK;
    else if(ai_should_run(unit, target))
        strat = AI_TARGET_RUN;
    else
        strat = AI_TARGET_NEAR;

    unit->strategy = strat;
}



/**
 * Gets position that an AI unit should move towards
 * @param position position_t structure that is modified.
 * @param unit AI unit
 * @param strat strategy to employ
 */
void ai_get_destination_position(position_t *position, unit_t *unit)
{
    ai_set_strategy(unit);

    if(unit->strategy == AI_TARGET_RUN)
    {
        ai_run_from(position, unit, unit_find_nearest(mth_get_opponent(), unit));
    } else {
        unit_t *target = ai_get_target(unit, unit->strategy);
        
        if(target)
        {
            position->x = target->row;
            position->y = target->column;
        }
    }
}


/**
 * Gets a unit that the AI unit should move towards
 * @param unit AI unit to move
 * @param strategy strategy that the AI unit should take to attack
 * @returns an opponent unit to attack or NULL
 */
unit_t *ai_get_target(unit_t *unit, ai_strat_t strategy)
{
    team_t *opponent = mth_get_opponent();
    uint8_t i = 0;
    uint8_t min = 255, bestIndex = 0;

    switch (strategy)
    {
    case AI_TARGET_NONE:
        debug("AI has no STRAT setup");
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

        return opponent->units[bestIndex];

    // target the unit that can be killed the easiest
    case AI_TARGET_ATK:
        // find the first unit that we should attack
        for(; i < opponent->size; i++)
        {
            if(ai_would_kill(unit, opponent->units[i]))
                return opponent->units[i];
        }
        break;
    }

    return NULL;
}


/**
 * A point that the AI unit should approach
 * @param position structure that is modified. Upon returning, this position will be the furthest from `other`
 * @param unit AI unit
 * @param other unit to run from
 * @param strategy the goal
 */
void ai_run_from(position_t *position, unit_t *unit, unit_t *other)
{
    uint8_t xGoal, yGoal;
    uint8_t bestDist = 0;
    uint8_t bestIndex = 0;

    // this will hopefully never trigger
    if(!other)
        return;

    const int8_t dx[] = {-1, 1, 0, 0};
    const int8_t dy[] = {0, 0, -1, 1};

    for(uint8_t i = 0; i < 4; i++)
    {
        uint8_t curDist;
        xGoal = unit->row + (int8_t)unit->stats.movePoints * dx[i];
        yGoal = unit->column + (int8_t)unit->stats.movePoints * dy[i];

        curDist = getDistance(xGoal, yGoal, other->row, other->column);

        if(curDist > bestDist && unit_can_move_to(xGoal, yGoal))
        {
            bestDist = curDist;
            bestIndex = i;
        }
    }

    xGoal = unit->row + unit->stats.movePoints * dx[bestIndex];
    yGoal = unit->column + unit->stats.movePoints * dy[bestIndex];

    printInt(xGoal, 5, 10, false);
    printInt(yGoal, 5, 11, false);
    waitjoypad(J_B);

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

    // if AI will kill target, then proceed with the kill
    if(ai_would_kill(unit, target))
    {
        unit_attack(unit, target);
        return true;
    }

    // if the target is in range and would kill AI, do not attack
    if(ai_should_run(unit, target) || ai_would_kill(target, unit))
        return false;
    
    // if(target->stats.damagePoints >= unit->stats.health)
    //     return false;

    if(unit_in_atk_range(unit, target))
       unit_attack(unit, target);

    return true;
}


/**
 * the AI unit should only attack if it can kill opponent, or it can damage opponent, but won't die from counter attack
 * @returns true if the AI unit could attack AND kill `other`
 */
bool ai_would_kill(unit_t *unit, unit_t *other)
{
    // if our attack can kill enemy and we are in range
    if(unit->stats.damagePoints >= other->stats.health
     && unit_in_atk_range(unit, other) && !other->isDead)
        return true;
    
    return false;
}


/**
 * The AI unit should run when the opponent can kill the opponent 
 * @returns true if the AI unit is weak relative to `other` and should run
 */
bool ai_should_run(unit_t *unit, unit_t *other)
{
    // run away if `other` will kill AI
    if(other->stats.damagePoints >= unit->stats.health
     && unit_in_atk_range(other, unit) && !other->isDead)
        return true;

    return false;
}