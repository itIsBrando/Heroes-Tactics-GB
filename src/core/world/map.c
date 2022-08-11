
#include <gb/gb.h>
#include <string.h>

#include "map.h"
#include "../game.h"
#include "../battle/diamond.h"
#include "../unit/units.h"
#include "../gfx/cgb.h"
#include "../../data/map1.h"
#include "../../data/map2.h"
#include "../../data/map3.h"
#include "../../data/map4.h"
#include "../../data/map5.h"
#include "../../data/map6.h"

uint8_t *all_maps[MAPS_TOTAL] = {MAP_DATA, MAP2_DATA, MAP3_DATA, MAP4_DATA, MAP5_DATA, MAP6_DATA};
const uint8_t map_widths[] = {10, 9,  7, 7, 12, 10};
const uint8_t map_heights[]= {10, 8,  8, 6, 6, 10};
static map_t internalMap;
static map_t *activeMap;
static uint8_t internal_map_data[MAP_MAX_SIZE];
static uint8_t map_fog[MAP_MAX_SIZE];
static bool mapHasFog = false;

static const uint8_t map_tile_flags[] = {
    0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0
};


/**
 * Blits a map to the screen. Does not draw fog
 * @see map_draw
 */
void map_blit(map_t *map)
{
    set_bkg_tiles(0, 0, map->width, map->height, map->data);    
}


/**
 * Draws the map from (0, 0) to (w, h). Includes fog
 */
void map_draw()
{
    uint8_t *data = map_has_fog() ? map_fog : activeMap->data;

    set_bkg_tiles(0, 0,activeMap->width,activeMap->height, data);
}


/**
 * Applies fog to the map buffer
 */
static void map_apply_fog()
{
    uint8_t i;
    const team_t *team = mth_get_current_team();
    unit_t *unit;
    tri_clear();

    for(i = 0; i < team->size; i++)
    {
        unit = team->units[i];

        if(unit->isDead)
            continue;
        
        tri_make_no_clear(unit->row, unit->column, 2);
    }

    // now apply the mask that we've created

    for(i = 0; i < activeMap->size; i++)
    {
        if(!tri_active_diamond[i])
            map_fog[i] = 23;

    }

    map_fog_hide_units(mth_get_opponent());
}


/**
 * Hides units that are covered by the fog
 * @param team team of units to hide
 */
void map_fog_hide_units(team_t *team)
{
    for(uint8_t i = 0; i < team->size; i++)
    {
        unit_t *unit = team->units[i];

        if(!unit->isDead && !tri_get(unit->row, unit->column))
            unit_hide(unit);
        else
            unit_draw_paletted(unit, team);
    }
}


/**
 * Creates a separate buffer of the map and overlays fog on it
 * - redraws the map
 */
void map_generate_fog()
{
    // copy map to buffer
    memcpy(map_fog, internalMap.data, internalMap.size);
    
    // now apply fog
    map_apply_fog();
    map_draw();
}


/**
 * Updates the fog buffer and redraws the map
 */
void map_update_fog()
{
    team_t *team = mth_get_current_team();

    if(team->control != CONTROLLER_PLAYER)
    {
        map_fog_hide_units(team);
        return;
    }

    map_generate_fog();
    if(is_cgb())
        cgb_map();
}


/**
 * Loads a map from a pointer
 * @param data pointer to tilemap data
 * @param width width of data
 * @param height height of data
 * @param useFog true if the map should been drawn with a fog overlay
 * @returns pointer to map
 */
map_t *map_load_from_data(uint8_t *data, uint8_t w, uint8_t h, bool useFog)
{
    internalMap.width = w;
    internalMap.height = h;
    
    // stinky hack we need to do incase `data` gets freed
    memcpy(internal_map_data, data, MAP_MAX_SIZE);
    internalMap.data = internal_map_data;
    
    map_load(&internalMap, useFog);
    return &internalMap;
}


/**
 * @returns true if the loaded map is using fog
 */
inline bool map_has_fog()
{
    return mapHasFog;
}


/**
 * Loads a map
 * @param map pointer to map structure
 * @param useFog determines whether the map should have a fog overlay drawn on it
 */
void map_load(map_t *map, bool useFog)
{
    activeMap = map;
    activeMap->size = activeMap->width * activeMap->height;
    mapHasFog = useFog;
}


const uint8_t map_waters[] = {0x6, 0x7};
static uint8_t vblankCounter, water_counter = 0;

void map_animate()
{
    uint8_t x = 0, y = 0;

    if(++vblankCounter < 30)
        return;
    
    vblankCounter = 0;
    water_counter ^= 1;

    uint8_t data[16];
    get_bkg_data(map_waters[water_counter], 1, data);

    set_bkg_data(TILE_WATER, 1, data);
}


inline void map_vbl_int()
{
    map_animate();
}


/**
 * Sets the spawnpoint of each unit in this team
 * @param team team of units to setup
 * @param searchBackwards true to start at the end of map towards top
 */
void map_init_spawn(team_t *team, bool searchBackwards)
{
    int8_t x, y, i;
    const uint8_t size = activeMap->size-1;
    x = y = 0;

    if(searchBackwards)
    {
        x = map_get_width() - 1, y = map_get_height() - 1;
        for(i = size; i >= 0; i--)
        {
            // house
            if(activeMap->data[i] == TILE_HOUSE)
                break;
            
            if(--x < 0)
                x = map_get_width()-1, y--;
        }
    } else {
        for(i = 0; i < (int8_t)size; i++)
        {
            // house
            if(activeMap->data[i] == TILE_HOUSE)
                break;
            
            if(++x >= (int8_t)map_get_width())
                x = 0, y++;
        }
    }

    int8_t dx[] = {-1, 1, 0, 0};
    int8_t dy[] = {0, 0, -1, 1};

    // now check for surrounding tiles
    for(i = 0; i < (int8_t)team->size; i++)
    {
        unit_t *unit = team->units[i];
        unit->row = x;
        unit->column = y;

        for(uint8_t j = 0; j < 4; j++)
        {
            const uint8_t px = x + dx[j],
              py = y + dy[j];
            bool isValid = map_in_bounds(px, py) && !map_is_solid(px, py);
            // make sure that no other unit will occupy this space
            for(int8_t k = 0; k < i; k++)
            {
                if(team->units[k]->row == px && team->units[k]->column == py)
                    isValid = false;
            }
            if(isValid)
                unit->row = px, unit->column = py;

        }
    }
}


/**
 * Must be called whenever the turn has just changed
 */
void map_changed_turns()
{
    if(map_has_fog())
        map_update_fog();
}


/**
 * @param x row
 * @param y column
 * @returns the tile number at (x, y)
 */
uint8_t map_get(uint8_t x, uint8_t y)
{
    return activeMap->data[x + y * map_get_width()];
}


/**
 * Gets a tiles from the map. If fog is enabled, then invisible tiles will return the tile number of fog
 * @param x row
 * @param y column
 * @returns tile number at (x, y)
 * @see map_get
 */
uint8_t map_get_with_fog(uint8_t x, uint8_t y)
{
    if(map_has_fog())
        return map_fog[x + y * map_get_width()];
    else
        return activeMap->data[x + y * map_get_width()];
}


/**
 * @param position location to check
 * @returns the tile number at (x, y)
 */
inline uint8_t map_get_pos(position_t *position)
{
    return map_get(position->x, position->y);
}


/**
 * @returns true if the (x, y) position is within the bounds of the map, otherwise false
 */
inline bool map_in_bounds(uint8_t x, uint8_t y)
{
    return x < activeMap->width && y < activeMap->height;
}


/**
 * bit0 is solidness
 * @returns Gets the flags byte for a tile
 */
inline uint8_t map_fget(uint8_t tile)
{
    return map_tile_flags[tile];
}


/**
 * Checks to see if a tile is solid
 * @param x tile x coordinate
 * @param y tile y coordinate
 * @returns true if the tile is solid
 */
bool map_is_solid(uint8_t x, uint8_t y)
{
    return map_fget(map_get(x, y)) & 0x1;
}


/**
 * @returns the loaded map
 */
inline map_t *map_get_active()
{
    return activeMap;
}


/**
 * @returns gets the width of the loaded map
 */
inline uint8_t map_get_width()
{
    return activeMap->width;
}


/**
 * @returns gets the height of the loaded map
 */
inline uint8_t map_get_height()
{
    return activeMap->height;
}