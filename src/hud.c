#include "defines.h"
#include "main.h"
#include "hud.h"
#include "units.h"
#include "map.h"
#include "game.h"

#include <gb/gb.h>

static uint8_t counter;
static bool hasVblank = false;

/**
 * Draws the player's units onto the window
 */
void hud_draw_hotbar(team_t *team)
{
    SHOW_WIN;
    fill_win_rect(0, 0, 20, 1, 4); // set window border
    fill_win_rect(0, 4, 20, 1, 4); // set window border
    fill_win_rect(0, 1, 19, 3, 0); // clear visible window
    move_win(7, 144 - 40); // set window position

    for(uint8_t x = 0; x < team->size; x++)
    {
        print_window(unit_get_name(team->units[x]), x * 7, 1);
        hud_draw_health(team->units[x], x * 7, 2, true);
    }
}

/**
 * Draws the UI for the health bar of a unit. Width of tiles drawn is variable
 * @param unit
 * @param x tile x
 * @param y tile y
 * @param useWindow true to draw on window, otherwise draws on background
 */
void hud_draw_health(unit_t *unit, uint8_t x, uint8_t y, const bool useWindow)
{
    fill_win_rect(x, y, 5, 1, 0);
    for(int8_t i = 0; i < unit->stats.maxHealth; i++)
        {
            uint8_t tile = (i < unit->stats.health) ? 0x11 : 0x12; // heart tile
            // use either filled or unfilled heart
            if(useWindow)
                set_win_tiles(x + i, y, 1, 1, &tile);
            else
                set_bkg_tiles(x + i, y, 1, 1, &tile);
        }
}

/**
 * Shows informative text to the user on what they are doing
 * @param action type of text to say HUD_ACTION_...
 */
void hud_show_action(const hud_action_t action)
{
    char ACTIONS[][5] = {
        "MOVE",
        "ATK",
        "PEAK"
    };

    print_window(ACTIONS[action], 0, 3);
}

/**
 * Hides any action if it is shown
 */
void hud_hide_action()
{
    fill_win_rect(0, 3, 4, 1, 0);
}


/**
 * Shows the details about the tile at (x, y)
 * @param x tile x
 * @param y tile y
 */
void hud_show_details(uint8_t x, uint8_t y)
{
    unit_t *unit = unit_get(mth_get_current_team(), x, y);
    bool isYours = unit != NULL;

    hud_show_action(HUD_ACTION_PEAK);

    if(!isYours)
        unit = unit_get_any(x, y);

    if(unit)
    {
        // print unit name
        print(unit_get_name(unit), 20-5, 0);
        // indicate whether this is your unit or not
        print(isYours ? "YOU" : "CPU", 20-3, 1);
        // print health
        hud_draw_health(unit, 20 - unit->stats.maxHealth, 2, false);
        // if has moved, then show that
        if(unit->hasMoved)
            print("MOVED", 20-5, 3);

        if(unit->hasAttacked)
            print("ATKED", 20-5, 4);
        // show tile of the unit
        fill_bkg_rect(20 - 4, 1, 1, 1, unit->tile);
    } else
    {
        const uint8_t tile = map_get(x, y);
        print("TILE", 20-5, 0);
        fill_bkg_rect(20 - 3, 1, 1, 1, tile);
        if(map_fget(tile) & 0x1)
            print("SOLID", 20 - 5, 2);
    }
}


/**
 * Hides the details of a tile
 */
void hud_hide_details()
{
    fill_bkg_rect(20-5, 0, 5, 5, 0);
    hud_hide_action();
}


void hud_vbl_int()
{
    if(counter++ > 60)
    {
        remove_VBL(hud_vbl_int);
        fill_win_rect(0, 3, 20, 1, 0);
        hasVblank = false;
    }
}

/**
 * Displays text on the window for a period of time
 * @param str string to write, should be <=20
 */
void hud_warn(char *str)
{
    print_window(str, 0, 3);
    counter = 0;
    if(!hasVblank)
    {
        add_VBL(hud_vbl_int);
        hasVblank = true;
    }
    
}