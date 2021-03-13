#include "oam.h"
#include "cursor.h"
#include "map.h"
#include "structs.h"
#include "data/mapdata.h"

#include <gb/gb.h>

static uint8_t cx, cy;
static uint8_t csn, ctn;
static uint8_t lastPressed = 0;
static bool isShown = false;

void cur_init()
{
    cx = cy = 4;
    csn = spr_allocate();
    ctn = 0x14;
    cur_draw();
    cur_show();
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


static uint8_t counter = 0;
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