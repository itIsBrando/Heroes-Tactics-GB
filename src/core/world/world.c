
#include <gb/gb.h>

#include "world.h"
#include "../../structs.h"
#include "../../data/worldMapData.h"
#include "../../main.h"
#include "../world/map.h"
#include "../gfx/cgb.h"
#include "../gfx/oam.h"
#include "../unit/units.h"


#define NUM_WORLD_MAPS (sizeof(wld_map_y))

void set_position(const uint8_t i);

typedef struct {
    direction_t l, r, u, d;
    uint8_t mapNum;
} map_waypoint_t;

const map_waypoint_t wld_waypoints[] = {
    {0, 1, 0, 0}, // 0
    {0, 2, 1, 1}, // 1 @todo add down
    {1, 4, 2, 3}, // 2
    {3, 5, 2, 3}, // 3
    {2, 4, 4, 5}, // 4
    {3, 5, 4, 6}, // 5
    {7, 6, 5, 6}, // 6
    {7, 6, 8, 7}, // 7
    {9, 8, 8, 7}, // 8
    {9, 8, 9, 9}, // 9
};


const uint8_t wld_map_x[] = {
    1, 5, 9, 9, 13, 13, 13, 5, 5, 1
};

const uint8_t wld_map_y[] = {
    1, 1, 1, 5, 1, 5, 8, 8, 6, 6
};


unit_t unit = {.tile=0xB};

const map_t wld_map = {
    .width=WORLD_MAP_DATA_WIDTH,
    .height=WORLD_MAP_DATA_HEIGHT,
    .size=WORLD_MAP_DATA_SIZE,
    .data=WORLD_MAP_DATA,
};


#define J_DPAD (J_LEFT | J_RIGHT | J_UP | J_DOWN)

void wld_init()
{
    uint8_t worldIndex = 0, pad;

    map_load(&wld_map, false);
    map_draw();
    cgb_map();

    // add_VBL(unit_vbl_int);
    add_VBL(map_vbl_int);

    unit.spriteNumber = spr_allocate();
    set_position(0);
    unit_draw(&unit);

    waitjoypad(J_A);

    do {
        pad = joypad();
    
        if(pad & J_DPAD) {
            if(pad & J_LEFT)
                worldIndex = wld_waypoints[worldIndex].l;
            else if(pad & J_RIGHT)
                worldIndex = wld_waypoints[worldIndex].r;
            else if(pad & J_UP)
                worldIndex = wld_waypoints[worldIndex].u;
            else if(pad & J_DOWN)
                worldIndex = wld_waypoints[worldIndex].d;

            set_position(worldIndex);
            unit_draw(&unit);
            waitjoypad(J_DPAD);
        } 

        wait_vbl_done();
    } while(pad != J_A);

    spr_free(unit.spriteNumber);
}

inline void set_position(const uint8_t i)
{
    unit.column = wld_map_y[i];
    unit.row = wld_map_x[i];
}
