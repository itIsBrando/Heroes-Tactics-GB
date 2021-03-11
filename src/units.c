#include "defines.h"
#include "oam.h"
#include "units.h"
#include "game.h"
#include "main.h"
#include "map.h"
#include "hud.h"
#include "diamond.h"
#include "cursor.h"

#include <gb/gb.h>
#include <rand.h>

const stat_t ARCHER_STAT = {3, 3, 1, 2, 2};
const stat_t HEALER_STAT = {2, 2, 1, 3, 1};
const stat_t BRAWN_STAT = {5, 5, 2, 1, 1};

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
        return NULL;
    }
    malloc_offset += s;
    
    return ptr;
}

// does absolutely nothing
inline void free(void *ptr) {
    ptr;
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
    } while(map_fget(map_get(unit->row, unit->column)));

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
 * @param unit sets the position and tile of this unit
 */
void unit_draw(unit_t *unit)
{
    const uint8_t i = unit->spriteNumber;

    move_sprite(i, (unit->row << 3) + 8, (unit->column << 3) + 16);
    set_sprite_tile(i, unit->tile);
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
 * Attacks an enemy and allows enemy to counterattack
 * @param attacker does damage to defender. Finishes this unit's turn
 * @param defender takes damage
 * @returns true if `defender` dies, otherwise false
 */
bool unit_attack(unit_t *attacker, unit_t *defender)
{
    uint8_t i = 0;

    cur_hide(); // hide cursor

    // flash attacker and defender
    for(; i < 4; i++)
    {
        unit_hide(attacker);
        unit_hide(defender);
        for(uint8_t j = 0; j < 10;j++)
            wait_vbl_done();
        unit_draw(attacker);
        unit_draw(defender);
        for(uint8_t j = 0; j < 10;j++)
            wait_vbl_done();
    }

    for(i = 0; i < currentMatch.numTeams; i++)
    {
        for(uint8_t j = 0; j < currentMatch.teams[i]->size; j++)
        {
            unit_hide(currentMatch.teams[i]->units[j]);
        }
    }

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
    waitjoypad(J_A);
    while(joypad() != J_A)
        wait_vbl_done();

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
    waitjoypad(J_A);
    while(joypad() != J_A)
        wait_vbl_done();

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
uint8_t getDistance(uint8_t *remainder, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
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

    if(remainder) *remainder = 0;

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
    return getDistance(NULL, u1->row, u1->column, u2->row, u2->column);
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
    if(unit_get_any(x, y) || map_fget(map_get(x, y)))
        return false;

    // check to see if another unit occupies this area
    uint8_t remainder;
    uint8_t dist = getDistance(&remainder, x, y, unit->row, unit->column);
    // printInt(dist, 0, 13, false);
    // this is necessary to round up
    if(remainder)
        dist++;

    if(dist > unit->stats.movePoints)
        return false;

    unit_set_pos(unit, x, y);
    return true;
}


/**
 * Sets the unit's new position and redraws it. Sets the 'hasMoved' flag
 * @param unit
 * @param x
 * @param y
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
    map_draw();
}


/**
 * @param unit unit_t to heal
 * @param hp number health points to heal
 */
void unit_heal(unit_t *unit, uint8_t hp)
{
    unit->stats.health = min(hp + unit->stats.health, unit->stats.maxHealth);
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

