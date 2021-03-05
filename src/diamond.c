#include <gb/gb.h>
#include <string.h>
#include "diamond.h"
#include "units.h"
#include "map.h"


#define tri_get(x, y) tri_active_diamond[(x) + MAP_WIDTH * (y)]
#define tri_set(x, y, v) tri_active_diamond[(x) + MAP_WIDTH * (y)] = v


uint8_t tri_active_diamond[MAP_SIZE];

/**
 * Creates a new diamond
 * @param row x center
 * @param column y center
 * @param radius self-explainatory
 */
void tri_make(uint8_t row, uint8_t column, uint8_t radius)
{
    int8_t startx = row;
    uint8_t stopx = startx;
    uint8_t starty = column - radius;

    memset(tri_active_diamond, 0, sizeof(tri_active_diamond));

    for(uint8_t y = 0; y <= radius << 1; y++)
    {
        column = starty + y;
        // check for underflow & overflow
        if(column < MAP_HEIGHT)
        {
            for(uint8_t j = 0; j < stopx - startx+1; j++)
            {
                row = startx + j;
                if(row < MAP_WIDTH)
                    tri_set(row, column, 1);
            }
        }

        // check to see if we are on the lower half of the diamond
        if(y >= radius)
        {
            stopx--;
            startx++;
        } else {
            stopx++;
            startx--;
        }
    }
}

/**
 * Applies clipping to the active triangle
 */
void tri_clip()
{
    uint8_t x = 0, y = 0;

    for(uint8_t i = 0; i < sizeof(tri_active_diamond); i++)
    {
        if(unit_get_any(x, y) || map_fget(map_get(x, y)))
            tri_active_diamond[i] = false;

        if(++x >= MAP_WIDTH)
            x = 0, y++;
    }
}

/**
 * Draws the active triangle
 */
void tri_draw(const uint8_t tile)
{
    uint8_t x = 0, y = 0;

    for(uint8_t i = 0; i < sizeof(tri_active_diamond); i++)
    {
        if(tri_active_diamond[i])
            fill_bkg_rect(x, y, 1, 1, tile);

        if(++x >= MAP_WIDTH)
            x = 0, y++;
    }
}