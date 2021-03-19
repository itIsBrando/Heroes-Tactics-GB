#include "defines.h"
#include "map.h"
#include "game.h"
#include "structs.h"

#include <gb/gb.h>
#include <string.h>

#include "data/mapdata.h"
#include "data/map2.h"
#include "data/map3.h"
#include "diamond.h"
#include "units.h"
#include "cgb.h"

uint8_t *all_maps[MAPS_TOTAL] = {MAP_DATA, MAP2_DATA, MAP3_DATA};
uint8_t map_widths[] = {10, 9,  7};
uint8_t map_heights[]= {10, 8,  8};

static map_t internalMap;
static map_t *activeMap;
static uint8_t map_fog[MAP_MAX_SIZE];
static bool mapHasFog = false;

static const uint8_t map_tile_flags[] = {
    0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
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
    uint8_t *data;

    if(map_has_fog())
        data = map_fog;
    else
        data = activeMap->data;

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

    if(!map_has_fog())
        return;

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
    internalMap.data = data;
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