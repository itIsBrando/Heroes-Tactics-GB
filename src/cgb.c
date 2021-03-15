#include <gb/hardware.h>
#include <gb/cgb.h>
#include <gb/gb.h>

#include "units.h"
#include "diamond.h"
#include "game.h"
#include "map.h"
#include "cgb.h"

uint16_t palette[] = {
    RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK,
    RGB_WHITE, RGB_PINK, RGB_BLUE, RGB_PURPLE,
    RGB_WHITE, RGB_DARKGRAY, RGB_GREEN, RGB_BROWN
};

uint16_t palette_sprite[] = {
    0x0000, RGB_LIGHTFLESH, RGB_CYAN, RGB_BLACK,
    0x0000, RGB_LIGHTFLESH, RGB_RED, RGB_BLACK,
};


/**
 * Initializes CGB palette
 */
void cgb_init()
{
    if(!is_cgb())
        return;
    
    set_bkg_palette(0, 3, palette);

    set_sprite_palette(0, 2, palette_sprite);
}


#define TEAM_COLOR_BLUE 10
#define TEAM_COLOR_RED 1

uint8_t cgb_get_team_color(team_t *team)
{
    return (team == mth_get_match()->teams[0]) ? TEAM_COLOR_BLUE : TEAM_COLOR_RED; // blue : red;
}


/**
 * Sets up the palettes in cgb mode. Only does the map
 */
void cgb_map()
{
    if(!is_cgb())
        return;

    VBK_REG = 1;

    for(uint8_t y = 0; y < map_get_height(); y++)
    {
        for(uint8_t x = 0; x < map_get_width(); x++)
        {
            uint8_t pal = 0;
            switch(map_get(x, y))
            {
            case 0xF:
                pal = 1;
                break;
            case 0x5:
            case 0x10:
                pal = 2;
                break;
            default:
                continue;
            }
            fill_bkg_rect(x, y, 1, 1, pal);
        }
    }

    VBK_REG = 0;
}


/**
 * Sets the palette for CGB
 * *external checking required*
 * @param x tile x
 * @param y tile y
 */
void cgb_diamond(uint8_t x, uint8_t y)
{
    VBK_REG = 1;
    fill_bkg_rect(x, y, 1, 1, 1);
    VBK_REG = 0;
}


/**
 * Hides any actively drawn triangle
 */
void cgb_hide_diamond()
{
    uint8_t x = 0, y = 0;

    VBK_REG = 1;
    for(uint8_t i = 0; i < sizeof(tri_active_diamond); i++)
    {
        if(tri_get(x, y))
            fill_bkg_rect(x, y, 1, 1, 0);

        if(++x >= tri_get_width())
            x = 0, y++;
    }

    VBK_REG = 0;
}


/**
 * @returns true if we are running on a CGB
 */
bool is_cgb()
{
     return _cpu == CGB_TYPE;
}