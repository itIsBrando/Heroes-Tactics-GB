#include <gb/hardware.h>
#include <gb/cgb.h>
#include <gb/gb.h>

#include "map.h"

#define is_cgb() (_cpu == CGB_TYPE)
uint16_t palette[] = {
    RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK,
    RGB_WHITE, RGB_WHITE, RGB_AQUA, RGB_WHITE,
    RGB_WHITE, RGB_DARKGRAY, RGB_GREEN, RGB_BROWN
};

uint16_t palette_sprite[] = {
    0x0000, 0x9BBD, RGB_BLACK, RGB_BLUE,
    0x0000, 0x9BBD, RGB_BLACK, RGB_RED,
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


/**
 * Sets up the palettes in cgb
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