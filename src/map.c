#include "defines.h"
#include "map.h"
#include "structs.h"

#include <gb/gb.h>

#include "data/mapdata.h"
#include "data/map2.h"
#include "data/map3.h"

uint8_t *all_maps[MAPS_TOTAL] = {MAP_DATA, MAP2_DATA, MAP3_DATA};
uint8_t map_widths[] = {10, 9,  7};
uint8_t map_heights[]= {10, 8,  8};


static map_t internalMap;
static map_t *activeMap;

static const uint8_t map_tile_flags[] = {
    0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

/**
 * Draws the map from (0, 0) to (w, h)
 */
void map_draw() {
    set_bkg_tiles(0, 0, activeMap->width, activeMap->height, activeMap->data);
}


/**
 * Loads a map from a pointer
 * @param data pointer to tilemap data
 * @param width width of data
 * @param height height of data
 */
void map_load_from_data(uint8_t *data, uint8_t w, uint8_t h)
{
    internalMap.width = w;
    internalMap.height = h;
    internalMap.data = data;
    map_load(&internalMap);
}


/**
 * Loads a map
 * @param map pointer to map structure
 */
void map_load(map_t *map)
{
    activeMap = map;
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