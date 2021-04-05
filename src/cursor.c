#include "oam.h"
#include "cursor.h"
#include "map.h"
#include "structs.h"
#include "data/mapdata.h"

#include <gb/gb.h>


static uint8_t counter = 0;
static uint8_t cx, cy;
static uint8_t csn, ctn; // Cursor Sprite Number, Cursor Tile Number
static uint8_t lastPressed = 0;
static bool isShown = false;

void cur_init()
{
    cx = cy = 4;
    csn = spr_allocate();
    ctn = 0x14;
    cur_draw();
    cur_show();

    set_sprite_prop(csn, 2); // set palette to 2
}

/**
 * Hides and removes the cursor
 * ***CURSOR MUST BE cur_init FIRST***
 */
void cur_destroy()
{
    spr_free(csn);
    remove_VBL(cur_vbl);
}


/**
 * Moves the cursor by a certain distance
 * @param dx x displacement
 * @param dy y displacement
 */
void cur_move(int8_t dx, int8_t dy)
{
    if(lastPressed)
        return; 
    const uint8_t x = cx + dx;
    const uint8_t y = cy + dy;

    if(x < map_get_width())
        cx = x;

    if(y < map_get_height())
        cy = y;

    lastPressed = 6;
        
    cur_draw();
}

/**
 * Hides the cursor. Can reappear by calling `cur_show`
 */
void cur_hide()
{
    move_sprite(csn, 0, 0);
    if(isShown)
        remove_VBL(cur_vbl);
    
    isShown = false;
}

/**
 * Shows the cursor
 */
void cur_show()
{
    if(!isShown)
        add_VBL(cur_vbl);
    
    isShown = true;
}


void cur_draw()
{
    set_sprite_tile(csn, ctn);
    move_sprite(csn, (cx << 3) + 8, (cy << 3) + 16);
}

/**
 * Flips the cursor's tile
 */
void cur_animate() {
    if(ctn == 0x14)
        ctn = 0x13;
    else
        ctn = 0x14;

    cur_draw();
}


void cur_vbl()
{
    if(++counter > 15)
    {
        counter = 0;
        cur_animate();
    }

    if(lastPressed)
        lastPressed--;
}

inline uint8_t cur_get_x()
{
    return cx;
}

inline uint8_t cur_get_y()
{
    return cy;
}


/**
 * Both positions must be neighboring
 * @param pos1 position 1
 * @param pos2 position 2
 * @returns the direction that `pos1` needs to move to get to `pos2`
 */
direction_t cur_get_direction(position_t *pos1, position_t *pos2)
{
    const uint8_t x1 = pos1->x, x2 = pos2->x;
    const uint8_t y1 = pos1->y, y2 = pos2->y;

    if(x1 > x2)
        return DIRECTION_RIGHT;
    else if(x1 < x2)
        return DIRECTION_LEFT;
    
    if(y1 > y2)
        return DIRECTION_UP;
    else
        return DIRECTION_DOWN;

}