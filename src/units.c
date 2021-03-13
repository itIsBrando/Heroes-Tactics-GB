#include "defines.h"
#include "oam.h"
#include "units.h"
#include "game.h"
#include "main.h"
#include "map.h"
#include "hud.h"
#include "diamond.h"
#include "cursor.h"
#include "path.h"

#include <gb/gb.h>
#include <rand.h>

const stat_t ARCHER_STAT = {3, 3, 1, 2, 2};
const stat_t HEALER_STAT = {2, 2, 1, 3, 1};
const stat_t BRAWN_STAT = {5, 5, 2, 1, 1};
static uint8_t unit_vbl_counter;

static uint16_t malloc_offset = 0;
static uint8_t malloc_start[512];


// a list of strings that name each unit
static char unit_names[][6] = {
    "BRAWN",
    "ARCHR",
    "HEALR"
};


/**
 * @returns the smallest value between `a` and `b`
 */
inline uint8_t min(uint8_t a, uint8_t b) {
    if(a > b)
        return b;
    
    return a;
}


void *malloc(size_t s)
{
    void *ptr = malloc_start + malloc_offset;
    if(malloc_offset > 512)
    {
        print("END OF HEAP", 5, 5);
        waitPressed(J_SELECT);
        return NULL;
    }
    malloc_offset += s;
    
    return ptr;
}

// does absolutely nothing
inline void free(void *ptr) {
    ptr;
}


static uint8_t animations[][2] = {
    {0xB, 0x15},
    {0xC, 0xD},
    {0xA, 0x16},
};

/**
 * Animates all of the units in a team
 * @param team team to animate
 */
inline void unit_animate(team_t *team)
{
    for(uint8_t i = 0; i < team->size; i++)
    {
        uint8_t j = team->units[i]->type;
        // check each animation
        if(team->units[i]->tile == animations[j][0])
            team->units[i]->tile = animations[j][1];
        else
            team->units[i]->tile = animations[j][0];
        
        unit_upd_sprite_tile(team->units[i]);
    }

}


/**
 * Used to animate units of a team
 */
void unit_vbl_int()
{
    unit_vbl_counter++;
    if(unit_vbl_counter == 0x14)
    {
        unit_animate(mth_get_current_team());
        unit_vbl_counter = 0;
    }
}


/**
 * Creates a new unit. *MUST BE FREED*
 * @param type type of unit to create see *type_of_unit*
 */
unit_t *unit_new(type_of_unit type) {
    unit_t *unit = malloc(sizeof(unit_t));

    unit->hasMoved = unit->isDead = unit->hasAttacked = false;
    unit->spriteNumber = spr_allocate();
    unit->type = type;
    unit->strategy = AI_TARGET_NONE;

    do {
        unit->row = (rand() & 0x7) + 1;
        unit->column = (rand() & 0x7) + 1;
    } while(map_is_solid(unit->row, unit->column));

    switch (type)
    {
    case UNIT_TYPE_BRAWN:
        unit->stats = BRAWN_STAT;
        unit->tile = 0xB;
        break;
    case UNIT_TYPE_ARCHER:
        unit->stats = ARCHER_STAT;
        unit->tile = 0xc;
        break;
    case UNIT_TYPE_HEALER:
        unit->stats = HEALER_STAT;
        unit->tile = 0xa;
        break;
    }

    return unit;
}


/**
 * @param team pointer to the team to check
 * @param x row to check
 * @param y column to check
 * @returns first occurance of a unit at a specific coordinate, otherwise NULL
 */
unit_t *unit_get(team_t *team, uint8_t x, uint8_t y)
{
    for(uint8_t i = 0; i < team->size; i++)
    {
        if(!team->units[i]->isDead && x == team->units[i]->row && y == team->units[i]->column)
            return team->units[i];
    }

    return NULL;
}


/**
 * @param x row to check
 * @param y column to check
 * @returns first occurance of a unit FROM ANY TEAM
 */
unit_t *unit_get_any(uint8_t x, uint8_t y) {
    for(uint8_t i = 0; i < currentMatch.numTeams; i++)
    {
        unit_t *u = unit_get(currentMatch.teams[i], x, y);
        if(u)
            return u;
    }

    return NULL;
}


/**
 * Changes the OAM tile for this unit
 * @param unit unit to update
 */
inline void unit_upd_sprite_tile(unit_t *unit)
{
    set_sprite_tile(unit->spriteNumber, unit->tile);
}


/**
 * @param unit sets the position and tile of this unit
 */
void unit_draw(unit_t *unit)
{
    move_sprite(unit->spriteNumber, (unit->row << 3) + 8, (unit->column << 3) + 16);
    unit_upd_sprite_tile(unit);
}


/**
 * @param unit sets the position and tile of this unit
 * @param x pixel x
 * @param y pixel y
 */
void unit_draw_at(unit_t *unit, uint8_t x, uint8_t y)
{
    const uint8_t i = unit->spriteNumber;

    move_sprite(i, x + 8, y + 16);
    set_sprite_tile(i, unit->tile);
}


/**
 * @param unit hides this unit until `unit_draw` is called again
 */
void unit_hide(unit_t *unit)
{
    const uint8_t i = unit->spriteNumber;

    move_sprite(i, 0, 0);
    set_sprite_tile(i, unit->tile);
}


/**
 * Applies damage to a unit
 * @param dmg damage to apply to unit
 * @returns true if this unit dies
 */
bool unit_do_damage(unit_t *unit, uint8_t dmg)
{
    unit->stats.health -= dmg;

    const bool isDead = unit->stats.health <= 0;
    unit->isDead = isDead;
    return isDead;
}


/**
 * Call to graphically indicate that two units are engaging. Unit order is irrelevant
 * @param u1 one unit
 * @param u2 second unit
 */
void unit_engage(unit_t *u1, unit_t *u2)
{
    uint8_t i = 0;

    // flash attacker and defender
    for(; i < 4; i++)
    {
        uint8_t j;
        unit_hide(u1);
        unit_hide(u2);
        for(j = 0; j < 6;j++)
            wait_vbl_done();
        unit_draw(u1);
        unit_draw(u2);
        for(j = 0; j < 6;j++)
            wait_vbl_done();
    }

    for(i = 0; i < currentMatch.numTeams; i++)
    {
        for(uint8_t j = 0; j < currentMatch.teams[i]->size; j++)
        {
            unit_hide(currentMatch.teams[i]->units[j]);
        }
    }
}


/**
 * Attacks an enemy and allows enemy to counterattack
 * @param attacker does damage to defender. Finishes this unit's turn
 * @param defender takes damage
 * @returns true if `defender` dies, otherwise false
 */
bool unit_attack(unit_t *attacker, unit_t *defender)
{
    uint8_t i = 0;

    cur_hide(); // hide cursor

    unit_engage(attacker, defender);

    // set up battle screen
    clear_bg();
    unit_draw_at(attacker, 40, 40);
    unit_draw_at(defender, 120, 40);
    hud_draw_health(attacker, 40 / 8 - 3, 40/ 8 + 2, false);
    
    print("BATTLE", 0, 0);

    bool death = unit_do_damage(defender, attacker->stats.damagePoints);

    hud_draw_health(defender, 120 / 8 - 3, 40/ 8 + 2, false);
    print(unit_get_name(attacker), 0, 10);
    print("did", 6, 10);
    printInt(attacker->stats.damagePoints, 10, 10, false);

    attacker->hasAttacked = attacker->hasMoved = true;

    // wait for joypad input
   waitPressed(J_A | J_START);

    // only do counterattack if the other man lived
    if(!death)
    {
        // do counterattack
        uint8_t dmg = defender->stats.damagePoints;

        // if defender is out of attack range, do not do any damage
        if(unit_get_distance(attacker, defender) > defender->stats.damageRadius)
            dmg = 0;

        death |= unit_do_damage(attacker, dmg);

        hud_draw_health(attacker, 40 / 8 - 3, 40/ 8 + 2, false);

        print(unit_get_name(defender), 0, 10);

        printInt(dmg, 10, 10, false);        
    } else {
        print("Opponent knocked out", 0, 10);
    }

    // wait for joypad input
    waitPressed(J_A | J_START);

    clear_bg();
    map_draw(); // redraw map
    cur_show(); // redraw cursor

    // redraw all teams
    for(i = 0; i < currentMatch.numTeams; i++)
       mth_draw_team(currentMatch.teams[i]);
    
    return death;
}


/**
 * probably quite slow
 * @returns the square root of an unsigned 8-bit number
 */
uint8_t sqrt(uint8_t a) {
    uint8_t i = 1;
    for(; i * i <= a; ++i);
    i--;
    return i;
}


/**
 * Destroys a unit created by *unit_new()*
 * @param unit destroys this unit
 */
void unit_destroy(unit_t *unit)
{
    spr_free(unit->spriteNumber);
    free(unit);
}


/**
 * Returns the Manhattan distance
 * @param remainder pointer to a single byte that will hold the remainder, or NULL if unneeded
 */
uint8_t getDistance(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    const int8_t dx = x1 > x2 ? -1 : 1;
    const int8_t dy = y1 > y2 ? -1 : 1;
    uint8_t distance = 0;
    
    while(x1 != x2)
    {
        distance++;
        x1 += dx;
    }
    
    while(y1 != y2) {
        distance++;
        y1 += dy;
    }

    return distance;

    // uint8_t diff1, diff2;
    // if(x1 > x2)
    //     diff1 = x1 - x2;
    // else
    //     diff1 = x2 - x1;
    
    // if(y1 > y2)
    //     diff2 = y1 - y2;
    // else
    //     diff2 = y2 - y1;
   
    // diff1 = diff1 * diff1 + diff2 * diff2;

    // uint8_t out = sqrt(diff1);

    // // this value is essentially boolean. 0 = no remainder, otherwise remainder
    // if(remainder)
    //     *remainder = diff1 - out*out;
    // return out;
}


/**
 * Gets the distance between two units
 * @returns integer distance
 */
inline uint8_t unit_get_distance(unit_t *u1, unit_t *u2)
{
    return getDistance(u1->row, u1->column, u2->row, u2->column);
}

/**
 * Finds the unit closest to `unit` in `opponent`
 * @returns returns that unit
 */
unit_t *unit_find_nearest(team_t *opponent, unit_t *unit)
{
    uint8_t bestIndex = 0, min = 255;
    for(uint8_t i = 0; i < opponent->size; i++)
    {
        // skip if this unit is dead
        if(opponent->units[i]->isDead)
            continue;
        
        // otherwise compare distances
        const uint8_t dist = unit_get_distance(opponent->units[i], unit);
        if(dist < min)
        {
            bestIndex = i;
            min = dist;
        }
    }

    return opponent->units[bestIndex];
}

/**
 * Checks to see if (x, y) is a valid place to move the unit
 * @returns true if the tile at (x, y) is empty, otherwise false
 */
bool unit_can_move_to(uint8_t x, uint8_t y)
{
    return !(unit_get_any(x, y) || map_fget(map_get(x, y)));
}

/**
 * Moves a unit to a coordinate
 * - checks if a unit can move to this position too
 * - redraws this unit too
 * @param unit 
 * @param x row
 * @param y column
 * @returns true if the unit moved, false otherwise
 */
bool unit_move_to(unit_t *unit, uint8_t x, uint8_t y)
{
    if(!unit_can_move_to(x, y))
        return false;

    // check to see if another unit occupies this area
    uint8_t dist = getDistance(x, y, unit->row, unit->column);

    if(dist > unit->stats.movePoints)
        return false;

    unit_set_pos(unit, x, y);
    return true;
}


/**
 * Moves a unit to a position using path finding to achieve it.
 * @param unit unit to move
 * @param destination position to end at
 * @returns false if a path could not be generated
 */
bool unit_move_path_find(unit_t *unit, position_t *destination)
{
    uint8_t size;
    position_t unitPos;
    unitPos.x = unit->row;
    unitPos.y = unit->column;

    position_t *steps = pf_find(&unitPos, destination, &size);

    // stop if we cannot make a path
    if(!steps)
        return false;

    if(size-1 > unit->stats.movePoints)
    {
        hud_warn("Too far");
        return false;
    }
    
    uint8_t iter = 0; // keeps track of the number of steps we've moved

    for(int8_t i = size-1; i >= 0; i--)
    {
        unit_move_to(unit, steps[i].x, steps[i].y);

        for(uint8_t j = 0; j < 10; j++)
            wait_vbl_done();

        if(++iter > unit->stats.movePoints)
            break;
    }

    return true;
}


/**
 * Sets the unit's new position and redraws it. Sets the 'hasMoved' flag
 * @param unit
 * @param x must be in bounds
 * @param y must be in bounds
 */
inline void unit_set_pos(unit_t *unit, uint8_t x, uint8_t y)
{
    unit->row = x;
    unit->column = y;
    unit->hasMoved = true;
    unit_draw(unit);

}


/**
 * @returns a string representation of a unit based on its type
 */
inline char *unit_get_name(unit_t *unit)
{
    return unit_names[unit->type];
}


/**
 * Draws the movement diamond for a `unit`
 * @param unit unit_t to draw triangle for
 */
void unit_move_diamond(unit_t *unit)
{
    tri_make(unit->row, unit->column, unit->stats.movePoints);
    tri_clip();
    tri_draw(0x9);
}


/**
 * Draws the attack diamond for a `unit`
 * @param unit unit_t to draw triangle for
 */
void unit_atk_diamond(unit_t *unit)
{
    tri_make(unit->row, unit->column, unit->stats.damageRadius);
    tri_clip();
    tri_draw(0x8);
}


/**
 * Hides the attack triangle
 */
inline void unit_hide_triangle()
{
    tri_hide();
}


/**
 * Finds the healer in a team
 * @param team team to search
 * @returns NULL if no LIVING healer is in the team, otherwise returns a pointer to the healer
 */
unit_t *unit_get_healer(team_t *team)
{
    for(uint8_t i = 0; i < team->size; i++)
    {
        if(team->units[i]->type == UNIT_TYPE_HEALER && !team->units[i]->isDead)
            return team->units[i];
    }

    return NULL;
}


/**
 * Heals a unit, graphically
 * @param unit unit_t to heal
 * @param healer unit that heals
 * @returns false if we are at full health, otherwise heals unit and returns true
 */
bool unit_heal(unit_t *unit, unit_t *healer)
{
    const uint8_t x = 68/8, y = 40 / 8 + 2;

    // return if we are at full health
    if(unit->stats.health == unit->stats.maxHealth)
        return false;

    // prevent a healer from healing itself
    if(unit == healer)
        return false;

    // if healer is not in range, return false
    if(unit_get_distance(unit, healer) > healer->stats.damageRadius)
        return false;

    cur_hide();

    unit_engage(unit, healer);

    clear_bg();
    print("HEAL", 0, 0);
    
    unit_draw_at(unit, 68, 40);
    hud_draw_health(unit, x, y, false);

    print(unit_get_name(unit), 0, 10);
    print(" healed 1 HP", 5, 10);

    unit->stats.health = min(1 + unit->stats.health, unit->stats.maxHealth);
    healer->hasAttacked = true;

    waitPressed(J_A | J_START);

    hud_draw_health(unit, x, y, false);
    
    waitPressed(J_A | J_START);

    clear_bg();
    map_draw(); // redraw map
    cur_show(); // redraw cursor

    // redraw all teams
    for(uint8_t i = 0; i < currentMatch.numTeams; i++)
       mth_draw_team(currentMatch.teams[i]);

    return true;
}


/**
 * @returns true if `unit` is in the attackable range of `other`
 */
bool unit_in_atk_range(unit_t *unit, unit_t *other)
{
    uint8_t atkRadius = unit->stats.damageRadius;

    // if(!unit->hasMoved)
    //     atkRadius += unit->stats.movePoints;
        
    return unit_get_distance(unit, other) <= atkRadius;
}

