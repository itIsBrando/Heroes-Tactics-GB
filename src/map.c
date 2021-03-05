#include "defines.h"
#include "map.h"
#include "structs.h"

#include <gb/gb.h>

static map_t *activeMap;

static const uint8_t map_tile_flags[] = {
    0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

void map_draw() {
    set_bkg_tiles(0, 0, activeMap->width, activeMap->height, activeMap->data);
}

/**
 * Loads a map
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
    return activeMap->data[x + y * activeMap->width];
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