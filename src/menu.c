#include <gb/gb.h>

#include "menu.h"
#include "main.h"
#include "map.h"
#include "game.h"
#include "link.h"

const char control_text[][4] = {"PLR", "CPU", "LNK"};
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
        printInt(i+1, 6, y, false);
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
    print("(A) to start game", 0, 144 / 8 - 1);

    mnu_cursor_init(0, match->numTeams, NULL);

    mnu_choose_team_draw(match);

    waitjoypad(0xFF);

    uint8_t pad;

    do {
        pad = joypad();

        if(pad == J_LEFT && match->teams[cursor]->control > 0)
        {
            match->teams[cursor]->control--;
            mnu_choose_team_draw(match);
            waitjoypad(J_LEFT | J_RIGHT);
        } else if((pad == J_RIGHT) && match->teams[cursor]->control < 2)
        {
            match->teams[cursor]->control++;
            mnu_choose_team_draw(match);
            waitjoypad(J_LEFT | J_RIGHT);
        }

        if(pad & J_DOWN)
            mnu_cursor_down();

        if(pad & J_UP)
            mnu_cursor_up();
        
        wait_vbl_done();
    } while(pad != J_A);

    lnk_init(match);
}


/**
 * @param xCursor x alignment for the cursor
 * @param maxSize Number of menu items
 * @param onchange function pointer to call whenever the cursor is moved
 */
void mnu_cursor_init(uint8_t xCursor, uint8_t maxSize, void (*onchange)(void) )
{
    cur_x = xCursor;
    cursor = 1;
    maxCursor = maxSize;
    onchange_ptr = onchange;
    mnu_cursor_up();
}


/**
 * Moves and redraws cursor
 */
void mnu_cursor_down()
{

    if(cursor < maxCursor-1) {
        fill_bkg_rect(cur_x, 2, 1, 2+maxCursor, 0);
        cursor++;
    } else
        return;

    print(">", cur_x, 3 + cursor);

    if(onchange_ptr)
        (*onchange_ptr)();

    waitjoypad(J_DOWN);
}


/**
 * Moves and redraws the cursor
 */
void mnu_cursor_up() {

    if(cursor > 0) {
        fill_bkg_rect(cur_x, 2, 1, 2+maxCursor, 0);
        cursor--;
    } else
        return;
    
    print(">", cur_x, 3 + cursor);

    if(onchange_ptr)
        (*onchange_ptr)();
        
    waitjoypad(J_UP);
}


static bool useFog = false;

/**
 * Visually shows a preview of the map
 * @param useFog true to load the map with fog, otherwise false
 */
static void mnu_draw_map()
{
    fill_bkg_rect(0, 0, 12, 10, 0);
    
    map_blit(
        map_load_from_data(
        all_maps[cursor],
        map_widths[cursor],
        map_heights[cursor],
        useFog)
    );
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
    mnu_cursor_init(18, totalMaps, mnu_draw_map);
    print("MAP", 17, 0);

    mnu_cursor_up();

    for(uint8_t i = 1; i <= totalMaps; i++)
        printInt(i, 19, 2 + i, false);

    waitjoypad(0xff);

    do {
        pad = joypad();

        if(pad & J_DOWN){
            mnu_cursor_down();
        } else if(pad & J_UP)
            mnu_cursor_up();

        wait_vbl_done();

        // hacky way to enable fog
        if(pad == J_START) {
            useFog = true;
            mnu_draw_map();
            print("using fog", 0, 10);
            waitjoypad(J_START);
        }
    } while(pad != J_A);

}


/**
 * Displays the main menu
 * @returns index of cursor position when `J_A` is pressed
 */
uint8_t mnu_main_menu()
{
    uint8_t pad;

    print("HEROES TACTICS", 0, 0);
    print("Play\nCampaign\nHelp", 1, 3);
    mnu_cursor_init(0, 3, NULL);
    
    waitjoypad(J_A);

    do {
        pad = joypad();

        if(pad & J_DOWN)
            mnu_cursor_down();
        else if(pad & J_UP)
            mnu_cursor_up();

        wait_vbl_done();
    } while (pad != J_A);
    
    return cursor;
}