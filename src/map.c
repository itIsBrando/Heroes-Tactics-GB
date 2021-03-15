#include "defines.h"
#include "map.h"
#include "game.h"
#include "structs.h"

#include <gb/gb.h>

#include "data/mapdata.h"
#include "data/map2.h"
#include "data/map3.h"
#include "diamond.h"
#include "units.h"

uint8_t *all_maps[MAPS_TOTAL] = {MAP_DATA, MAP2_DATA, MAP3_DATA};
uint8_t map_widths[] = {10, 9,  7};
uint8_t map_heights[]= {10, 8,  8};


static map_t internalMap;
static map_t *activeMap;
static bool mapHasFog = false;

static const uint8_t map_tile_flags[] = {
    0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};


/**
 * Blits the `activeMap` to the screen. Excludes fog
 * @see map_draw
 */
void map_blit()
{
    set_bkg_tiles(0, 0, activeMap->width, activeMap->height, activeMap->data);
}


/**
 * Draws the map from (0, 0) to (w, h). Includes fog
 * @see map_blit()
 */
void map_draw()
{
    map_blit();
    if(mapHasFog)
        map_draw_fog();
}


/**
 * Adds a fog overlay to the drawn map
 * @todo PERFORMANCE MUST INCREASE
 */
void map_draw_fog()
{
    team_t *team = mth_get_current_team();
    team_t *opponents = mth_get_opponent();

    tri_clear(); // clears triangles

    // create a triangle for each unit
    for(uint8_t i = 0; i < team->size; i++)
    {
        unit_t *unit = team->units[i];
        tri_make_no_clear(unit->row, unit->column, 2);
    }

    // Hide tiles and units covered by a fog
    for(uint8_t y = 0; y < tri_get_height(); y++)
    {
        for(uint8_t x = 0; x < tri_get_width(); x++)
        {
            if(!tri_get(x, y) && map_get(x, y) != 2)
            {
                unit_t *unit = unit_get(opponents, x, y);

                fill_bkg_rect(x, y, 1, 1, 0x17);
                // hide an opposing unit if present
                if(unit)
                    unit_hide(unit);
            }
        }
    }
}


/**
 * Loads a map from a pointer
 * @param data pointer to tilemap data
 * @param width width of data
 * @param height height of data
 * @param useFog true if the map should been drawn with a fog overlay
 */
void map_load_from_data(uint8_t *data, uint8_t w, uint8_t h, bool useFog)
{
    internalMap.width = w;
    internalMap.height = h;
    internalMap.data = data;
    map_load(&internalMap, useFog);
}


/**
 * Loads a map
 * @param map pointer to map structure
 * @param useFog determines whether the map should have a fog overlay drawn on it
 */
void map_load(map_t *map, bool useFog)
{
    activeMap = map;
    mapHasFog = useFog;
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