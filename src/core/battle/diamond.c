#include <gb/gb.h>
#include <string.h>

#include "diamond.h"
#include "../game.h"
#include "../unit/units.h"
#include "../world/map.h"
#include "../gfx/cgb.h"

uint8_t tri_active_diamond[MAP_MAX_SIZE];
static bool tri_is_shown;


void tri_make_no_clear(uint8_t row, uint8_t column, uint8_t radius)
{
    int8_t startx = row;
    uint8_t stopx = startx;
    uint8_t starty = column - radius;

    for(uint8_t y = 0; y <= radius << 1; y++)
    {
        column = starty + y;
        // check for underflow & overflow
        if(column < map_get_height())
        {
            for(uint8_t j = 0; j < stopx - startx+1; j++)
            {
                row = startx + j;
                if(row < map_get_width())
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


void tri_make(uint8_t row, uint8_t column, uint8_t radius)
{
    tri_clear();
    tri_make_no_clear(row, column, radius);
}


void tri_clear()
{
    memset(tri_active_diamond, 0, sizeof(tri_active_diamond));
}


inline uint8_t tri_get_width()
{
    return map_get_width();
}

inline uint8_t tri_get_height()
{
    return map_get_height();
}

inline uint8_t tri_get(uint8_t x, uint8_t y)
{
    return tri_active_diamond[(x) + tri_get_width() * (y)];
}

inline void tri_set(uint8_t x, uint8_t y, uint8_t v)
{
    tri_active_diamond[(x) + tri_get_width() * (y)] = v;
}


/**
 * Hides the graphically drawn diamond, only if it is drawn
 */
inline void tri_hide()
{
    if(tri_is_shown)
    {
        map_draw();
        if(is_cgb())
            cgb_hide_diamond();
    }

    tri_is_shown = false;
}

/**
 * Applies clipping to the active triangle.
 *  Anything tile that has a solid block or a unit is set to `false`
 */
void tri_clip()
{
    uint8_t x = 0, y = 0;

    for(uint8_t i = 0; i < sizeof(tri_active_diamond); i++)
    {
        if(unit_get(mth_get_current_team(), x, y) || map_fget(map_get(x, y)))
            tri_active_diamond[i] = false;

        if(++x >= tri_get_width())
            x = 0, y++;
    }
}

/**
 * Draws the active triangle. Includes CGB coloration
 */
void tri_draw(const uint8_t tile)
{
    uint8_t x = 0, y = 0;
    const bool isCGB = is_cgb();

    tri_is_shown = true;

    for(uint8_t i = 0; i < sizeof(tri_active_diamond); i++)
    {
        if(tri_active_diamond[i])
            fill_bkg_rect(x, y, 1, 1, tile);

        if(++x >= map_get_width())
            x = 0, y++;
    }

    cgb_diamond(1);
}