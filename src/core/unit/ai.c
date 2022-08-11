#include "ai.h"
#include "path.h"
#include "../game.h"
#include "../unit/units.h"
#include "../world/map.h"
#include "../battle/diamond.h"
#include "../../main.h"

#include <limits.h>
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

    for(int8_t i = size-1; i >= 0; i--)
    {
        // if we can attack from a distance
        // but if we are running, do not focus on enemy targets
        if(unit->strategy != AI_TARGET_RUN
         && unit_get_distance(unit, target) <= unit->stats.damageRadius)
            break;
        
        unit_move_to(unit, steps[i].x, steps[i].y);

        for(uint8_t j = 0; j < 15; j++)
            wait_vbl_done();
        if(++iter >= unit->stats.movePoints)
            break;
    }

    if(!ai_check_heal(unit))
        ai_check_attack(unit, target);
}


/**
 * Updates the strategy property for an AI unit
 */
void ai_set_strategy(unit_t *unit)
{
    team_t *opponent = mth_get_opponent();
    ai_strat_t strat = AI_TARGET_NONE;

    // find the closest enemy
    unit_t *target = unit_find_nearest(opponent, unit);
        
    if(ai_would_kill(unit, target))
        strat = AI_TARGET_ATK;
    else if(unit->type == UNIT_TYPE_HEALER)
    {
        strat = AI_TARGET_HEAL;
    } else if(ai_should_run(unit, target))
        // attempt to heal
        // if(unit->stats.health < unit->stats.maxHealth
        //  && unit_get_healer(mth_get_current_team()))
        //     strat = AI_TARGET_HEALER;
        // else
            strat = AI_TARGET_RUN;
    else
        strat = AI_TARGET_NEAR;

    unit->strategy = strat;
}


/**
 * Factors that increase heuristic value:
 *  - distance
 *  - health of other unit
 *  - if other unit is brawn (because they are strong)
 * *Factors that decrease heurisitc value:
 *  - if unit could kill other unit
 * A higher heurisitic value is less favorable
 * @param unit AI unit that relates to the heuristic value
 * @param t array of heuristic_t. Should have at least have `opponent's team size` indexes
 * @param size number of elements in `t` array
 * @returns `t` is the modified array. Sorted in ascending priority order
 */
void ai_get_heursitic_target(unit_t *unit, heuristic_t *t, uint8_t size)
{
    team_t *opponents = mth_get_opponent();
    // ensure that we do not overflow
    size = min(size, opponents->size);

    for(uint8_t i = 0; i < size; i++)
    {
        int8_t priority = 0;
        unit_t *enemy = opponents->units[i];

        if(enemy->isDead)
        {
            t[i].unit = NULL;
            t[i].priority = SCHAR_MAX;
            continue;
        }
        
        uint8_t dist = unit_get_distance(unit, enemy);
        priority += dist >> 1; // don't really want distance being a HUGE influence in value
        priority += enemy->stats.health;
        priority += enemy->type == UNIT_TYPE_BRAWN;
        if(ai_would_kill(unit, enemy))
            priority -= 2;

        t[i].unit = enemy;
        t[i].priority = priority;
    }
    
    // sort array from lowest priority value to highest
    uint8_t i, j;
    for(i = 0; i < size; i++)
    {
        for(j = i + 1; j < size; j++)
        {
            if(t[i].priority > t[j].priority)
            {
                heuristic_t temp;
                temp = t[i];
                t[i] = t[j];
                t[j] = temp;
            }
        }
    }
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
    team_t *curTeam = mth_get_current_team();
    uint8_t i = 0;
    int8_t min = SCHAR_MAX, bestIndex = 0;
    heuristic_t priorities[4];

    switch (strategy)
    {
    case AI_TARGET_NONE:
        debug("AI has no STRAT setup");
    case AI_TARGET_HEALER:
        // move towards our healer
        return unit_get_healer(curTeam);
    case AI_TARGET_HEAL:
        // move towards weakest unit on same team

        for(; i < curTeam->size; i++)
        {
            const int8_t hp = curTeam->units[i]->stats.health;
            bestIndex = -1;
            if(hp < min && curTeam->units[i]->stats.maxHealth != hp)
            {
                bestIndex = i;
                min = hp;
            }
        }

        // if all units are at full health, do AI_TARGET_NEAR
        if(bestIndex != -1)
            return curTeam->units[bestIndex];
    case AI_TARGET_NEAR:
        // find the most favorable unit
        ai_get_heursitic_target(unit, priorities, sizeof(priorities) / sizeof(priorities[0]));
        
        // if there is a unit that we can kill, then choose that one to target
        for(; i < opponent->size; i++)
        {
            if(ai_would_kill(unit, priorities[i].unit))
                return priorities[i].unit;
        }

        // first index is the best unit to attack
        return priorities[0].unit;

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
    uint8_t xGoal = unit->row, yGoal = unit->column, x, y;
    uint8_t bestDist = 0;
    uint8_t bestIndex = 0;
    const uint8_t xOther = other->row, yOther = other->column;

    // this will hopefully never trigger
    if(!other)
        return;

    debug("AI RUN");

    tri_make(unit->row, unit->column, unit->stats.movePoints);

    for(y = 0; y < map_get_width(); y++)
    {
        for(x = 0; x < map_get_width(); x++)
        {
            // only check distance if we can move to this point
            if(!tri_get(x, y) || map_is_solid(x, y))
                continue;
            
            uint8_t dist = getDistance(x, y, xOther, yOther);
            if(dist > bestDist)
            {
                xGoal = x, yGoal = y;
                bestDist = dist;
            }

        }
    }

    position->x = xGoal;
    position->y = yGoal;
}


/**
 * Checks to see if `unit` can heal nearby units
 * @returns true if healing could and did happen, otherwise false
 */
bool ai_check_heal(unit_t *unit)
{
    if(unit->type != UNIT_TYPE_HEALER)
        return false;

    team_t *team = mth_get_current_team();

    for(uint8_t i = 0; i < team->size; i++)
    {
        unit_t *target = team->units[i];

        if(target->isDead || target == unit || target->stats.health == target->stats.maxHealth)
            continue;

        // proceed with healing
        if(unit_heal(target, unit))
            return true;

    }

    return false;
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