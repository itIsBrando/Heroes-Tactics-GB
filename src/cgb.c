#include <gb/hardware.h>
#include <gb/cgb.h>
#include <gb/gb.h>

#include "units.h"
#include "diamond.h"
#include "game.h"
#include "map.h"
#include "cgb.h"

#define CGB_BG_DEFAULT  0
#define CGB_BG_WATER    1
#define CGB_BG_TREE     2
#define CGB_BG_HEART    3
#define CGB_BG_SAND     3

uint16_t palette[] = {
    RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK,
    RGB_DARKYELLOW, RGB_PINK, RGB_BLUE, RGB_PURPLE,
    RGB_DARKYELLOW, RGB_DARKGRAY, RGB_GREEN, RGB_BROWN,
    RGB_DARKYELLOW, RGB_WHITE, RGB_RED, RGB_DARKRED
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
    
    set_bkg_palette(0, 4, palette);

    set_sprite_palette(0, 2, palette_sprite);
}


#define TEAM_COLOR_BLUE 10
#define TEAM_COLOR_RED 1

uint8_t cgb_get_team_color(team_t *team)
{
    return (team == mth_get_match()->teams[0]) ? TEAM_COLOR_BLUE : TEAM_COLOR_RED; // blue : red;
}


/**
 * Updates the palette of the tile at (x, y)
 * @param x tile x
 * @param y tile y
 */
void cgb_write_tile(uint8_t x, uint8_t y)
{
    uint8_t pal;
    switch(map_get(x, y))
    {
    case 0x1:
        pal = CGB_BG_SAND;
        break;
    case 0xF:
        pal = CGB_BG_WATER;
        break;
    case 0x5:
    case 0x10:
        pal = CGB_BG_TREE;
        break;
    case 0x14: // bridge
    default:
        pal = CGB_BG_DEFAULT;
    }
    fill_bkg_rect(x, y, 1, 1, pal);
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
            cgb_write_tile(x, y);
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
            cgb_write_tile(x, y);

        if(++x >= tri_get_width())
            x = 0, y++;
    }

    VBK_REG = 0;
}


/**
 * Initializes the coloring for window HUD
 * @note Hooked from `hud_draw_hotbar`
 */
void cgb_draw_hud()
{
    if(!is_cgb())
        return;

    VBK_REG = 1;
    fill_win_rect(0, 0, 20, 1, 2);
    fill_win_rect(0, 1, 20, 1, 3); // change later
    fill_win_rect(0, 2, 20, 1, CGB_BG_HEART);
    fill_win_rect(0, 3, 20, 2, 2);
    VBK_REG = 0;
}


/**
 * Setups palette for a battle between two units
 * @note hooked in `unit_attack`
 */
void cgb_draw_battle()
{
    if(!is_cgb())
        return;

    VBK_REG = 1;
    // fill bg
    fill_bkg_rect(0, 0, 20, 13, CGB_BG_SAND);
    // fill row with heart palette
    fill_bkg_rect(2, 7, 16, 1, CGB_BG_HEART);
    VBK_REG = 0;
}


/**
 * Deinitializes the palettes after a battle has commenced
 * @note hooked in `unit_attack`
 */
void cgb_cleanup_battle()
{
    if(!is_cgb())
        return;


    cgb_map();
}


/**
 * @returns true if we are running on a CGB
 */
bool is_cgb()
{
     return _cpu == CGB_TYPE;
}