#include <gb/gb.h>

#include "menu.h"
#include "main.h"
#include "map.h"
#include "game.h"

const char control_text[][5] = {"PLR", "CPU", "LINK"};
static uint8_t cursor, maxCursor, cur_x = 0;
static void (*onchange_ptr)(void);

static inline const char *mnu_getControllerType(control_t t)
{
    return control_text[t];
}

static void mnu_choose_team_draw(match_t *match)
{
    for(uint8_t i = 0; i < match->numTeams; i++)
    {
        const uint8_t y = i + 3;
        print("team", 1, y);
        printInt(i, 6, y, false);
        print("<   >", 8, y);
        print(mnu_getControllerType(match->teams[i]->control), 9, y);
    }
}


/**
 * Allows the user to choose controller type of each team
 * @param match pointer to the match. `numTeams` must be initialized
 */
void mnu_choose_teams_init(match_t *match)
{
    clear_bg();
    print("MATCH SET UP:", 0, 0);
    print("(A) to start game", 0, 144/8-1);

    cur_x = 0;
    mnu_cursor_init(match->numTeams, NULL);
    

    mnu_choose_team_draw(match);

    waitjoypad(0xFF);

    uint8_t pad;

    do {
        pad = joypad();

        if((pad & J_LEFT) || (pad & J_RIGHT))
        {
            match->teams[cursor]->control--;
            match->teams[cursor]->control &= 0x1;
            mnu_choose_team_draw(match);
            waitjoypad(J_LEFT | J_RIGHT);
        }

        if(pad & J_DOWN)
        {
            mnu_cursor_down();
        }

        if(pad & J_UP)
        {
            mnu_cursor_up();
        }
        
        wait_vbl_done();
    } while(pad != J_A);
}


/**
 * @param maxSize Number of menu items
 * @param onchange function pointer to call whenever the cursor is moved
 */
void mnu_cursor_init(uint8_t maxSize, void (*onchange)(void) )
{
    cursor = 0;
    maxCursor = maxSize;
    onchange_ptr = onchange;
    mnu_cursor_up();
}


/**
 * Moves and redraws cursor
 */
void mnu_cursor_down()
{
    fill_bkg_rect(cur_x, 2, 1, 2+maxCursor, 0);

    if(cursor < maxCursor-1)
        cursor++;

    print(">", cur_x, 3 + cursor);

    if(onchange_ptr)
        (*onchange_ptr)();

    waitjoypad(J_DOWN);
}


/**
 * Moves and redraws the cursor
 */
void mnu_cursor_up() {
    fill_bkg_rect(cur_x, 2, 1, 2+maxCursor, 0);

    if(cursor > 0)
        cursor--;
    
    print(">", cur_x, 3 + cursor);

    if(onchange_ptr)
        (*onchange_ptr)();
        
    waitjoypad(J_UP);
}


static void mnu_draw_map()
{
    fill_bkg_rect(0, 0, 10, 10, 0);
    map_load_from_data(
        all_maps[cursor],
        map_widths[cursor],
        map_heights[cursor],
        false);
    map_blit();
}


/**
 * Allows the user to choose a map to play on
 * @returns index of map selected
 */
void mnu_choose_map_init()
{
    const uint8_t totalMaps = MAPS_TOTAL;
    uint8_t pad;

    clear_bg();
    cur_x = 10;
    mnu_cursor_init(totalMaps, mnu_draw_map);
    print("MAP SELECT", 10, 0);

    mnu_cursor_up();

    for(uint8_t i = 0; i < totalMaps; i++)
    {
        printInt(i, 11, 3 + i, false);
    }

    waitjoypad(0xff);

    do {
        pad = joypad();

        if(pad & J_DOWN){
            mnu_cursor_down();
        } else if(pad & J_UP)
            mnu_cursor_up();

        wait_vbl_done();
    } while(pad != J_A);

}