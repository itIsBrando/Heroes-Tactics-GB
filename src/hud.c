#include "defines.h"
#include "main.h"
#include "hud.h"
#include "units.h"
#include "map.h"
#include "game.h"
#include "cgb.h"
#include "oam.h"

#include <gb/gb.h>

static uint8_t counter;
static bool hasVblank = false;

static char hud_strat_string[][5] = {
    "NONE",
    "NEAR",
    "ATK",
    "RUN",
    "HEAL",
    "HLER"
};


/**
 * Draws the player's units onto the window
 */
void hud_draw_hotbar(team_t *team)
{
    SHOW_WIN;
    fill_win_rect(0, 0, 20, 1, 4); // set window border
    fill_win_rect(0, 4, 20, 1, 4); // set window border
    fill_win_rect(0, 1, 19, 3, 0); // clear visible window

    for(uint8_t x = 0; x < team->size; x++)
    {
        const uint8_t tx = x * 7;
        print_window(unit_get_name(team->units[x]), tx, 1);
        hud_draw_health(team->units[x], tx, 2, true);
        fill_win_rect(tx + 5, 1, 1, 1, team->units[x]->tile);
    }

    cgb_draw_hud();
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

    // print a message if we are dead rather than a visual
    if(unit->isDead)
    {
        if(useWindow)
            print_window("DEAD", x, y);
        else
            print("DEAD", x, y);
        return;
    }

    for(int8_t i = 0; i < unit->stats.maxHealth; i++)
    {
        uint8_t tile = (i < unit->stats.health) ? 0x11 : 0x12; // heart tile
        // use either filled or unfilled heart
        if(useWindow)
            fill_win_rect(x + i, y, 1, 1, tile);
        else
            fill_bkg_rect(x + i, y, 1, 1, tile);
    }

    if(is_cgb())
    {
        VBK_REG = 1;
        if(useWindow)
            fill_win_rect(x, y, unit->stats.maxHealth, 1, CGB_BG_HEART);
        else
            fill_bkg_rect(x, y, unit->stats.maxHealth, 1, CGB_BG_HEART);
        
        VBK_REG = 0;
    }
}


/**
 * Shows informative text to the user on what they are doing
 * @param action type of text to say HUD_ACTION_...
 */
void hud_show_action(const hud_action_t action)
{
    const char ACTIONS[][5] = {
        "MOVE",
        "ATK ",
        "PEAK",
        "RNGE"
    };

    hud_force_hide_warn();
    print_window(ACTIONS[action], 0, 3);
}


/**
 * Hides any action if it is shown
 */
void hud_hide_action()
{
    if(!hasVblank)
        fill_win_rect(0, 3, 4, 1, 0);
}


static char *get_strat_string(ai_strat_t strat)
{
    return hud_strat_string[strat];
}


/**
 * Shows the details about the tile at (x, y)
 * @param x tile x
 * @param y tile y
 */
void hud_show_details(uint8_t x, uint8_t y)
{
    unit_t *unit = unit_get(mth_get_current_team(), x, y);
    const uint8_t tile = map_get_with_fog(x, y);
    const bool isYours = unit != NULL;

    hud_show_action(HUD_ACTION_PEAK);

    if(!isYours)
        unit = unit_get_any(x, y);

    if(unit && tile != TILE_FOG)
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
        // DEBUG ONLY
        print(get_strat_string(unit->strategy), 20-5, 5);
        // show tile of the unit
        fill_bkg_rect(20 - 4, 1, 1, 1, unit->tile);
    } else
    {
        print("TILE", 20-5, 0);
        fill_bkg_rect(20 - 3, 1, 1, 1, tile);
        if(map_fget(tile) & 0x1)
            print("SOLID", 20 - 5, 2);
    }
}


static void hud_confirm_end_turn_left()
{
    fill_win_rect(10, 3, 1, 1, 0x9E);
    fill_win_rect(15, 3, 1, 1, 0);
}

static void hud_confirm_end_turn_right()
{
    fill_win_rect(10, 3, 1, 1, 0);
    fill_win_rect(15, 3, 1, 1, 0x9E);
}


/**
 * Presents a dialog on the window. Allows the user to end their turn or to continue
 * @returns true if the user said yes
 */
bool hud_confirm_end_turn()
{
    uint8_t pad;
    bool cursor = true;

    hud_force_hide_warn();
    print_window("End Turn:  YES  NO", 0, 3);
    hud_confirm_end_turn_left();

    do {
        pad = joypad();

        if(pad & J_LEFT) {
            hud_confirm_end_turn_left();
            cursor = true;
        } else if(pad & J_RIGHT) {
            hud_confirm_end_turn_right();
            cursor = false;
        }

        wait_vbl_done();
    } while(pad != J_A);

    waitjoypad(J_A);
    fill_win_rect(0, 3, 20, 1, 0);

    return cursor;
}

/**
 * Hides the details of a tile
 */
void hud_hide_details()
{
    // decrease height by one when debug is fixed @todo
    fill_bkg_rect(20-5, 0, 5, 6, 0);
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

/**
 * If there is a warning displayed on the HUD, then this function 
 *  will hide it
 */
void hud_force_hide_warn()
{
    counter = 0xFF; // force comparsion to result in `true`
    if(hasVblank)
        hud_vbl_int();
}

/**
 * @param cur cursor
 */
static void hud_unit_attack_draw(uint8_t cur)
{
    print_window(" ATK END BACK", 0, 3);
    fill_win_rect(cur << 2, 3, 1, 1, 158); // '>'
    waitjoypad(J_LEFT | J_RIGHT);
}


/**
 * The menu that appears after a unit has moved
 * @returns 1 for END TURN, 2 for BACK, or 0 for ATTACK
 */
uint8_t hud_unit_attack_menu()
{
    uint8_t pad, cur = 0;

    hud_force_hide_warn();
    hud_unit_attack_draw(0);
    
    do {
        pad = joypad();

        if((pad & J_LEFT) && cur > 0)
            hud_unit_attack_draw(--cur);
        else if((pad & J_RIGHT) && cur < 2)
            hud_unit_attack_draw(++cur);
        else if(pad & J_B)
        {
            cur = 2;
            break;
        }

        wait_vbl_done();
    } while(pad != J_A);

    // clear the menu
    fill_win_rect(0, 3, 13, 1, 0);
    
    return cur;
}


/**
 * Scrolls the window upwards by `pixels` pixels
 * @param number of pixels to scroll
 */
void scroll_win_up(const uint8_t pixels)
{
    for(uint8_t i = 0; i < pixels; i++)
    {
        WY_REG--;
        __asm__("HALT");
    }
}


/**
 * Scrolls the window downwards by `pixels` pixels
 * @param number of pixels to scroll
 */
void scroll_win_down(const uint8_t pixels)
{
    for(uint8_t i = 0; i < pixels; i++)
    {
        WY_REG++;
        __asm__("HALT");
    }
}


void hud_change_turn_banner()
{
    fill_win_rect(0, 7, 20, 1, 4); // draw a horizontal line
    print_window("Team   turn.\n\x1D to continue", 1, 5);
    printInt(mth_get_team_number(), 6, 5, true);

    // create a little border
    fill_win_rect(0, 5, 1, 2, 4);
    fill_win_rect(19, 5, 1, 2, 4);

    scroll_win_up(24);
}


inline void hud_change_turn_banner_cleanup()
{
    scroll_win_down(24);
}


static uint8_t sprite_indexes[MAX_TEAM_SIZE << 2];
static uint8_t hud_global_pos = 0;

/**
 * Shows a little icon above each unit to show ownership
 */
void hud_show_unit_control_type(const team_t *team)
{
    const uint8_t tile = team->control == CONTROLLER_PLAYER ? TILE_ICON_YOU : TILE_ICON_CPU;

    for(uint8_t i = 0; i < team->size; i++)
    {
        const unit_t *unit = team->units[i];
        uint8_t s1, s2;
        s1 = sprite_indexes[hud_global_pos++] = spr_allocate();
        s2 = sprite_indexes[hud_global_pos++] = spr_allocate();

        const uint8_t x = (unit->row << 3) + 4;
        const uint8_t y = (unit->column << 3) + 8;
        move_sprite(s1, x, y);
        move_sprite(s2, x + 8, y);
        set_sprite_prop(s1, 0);
        set_sprite_prop(s2, 0);
        set_sprite_tile(s1, tile);
        set_sprite_tile(s2, tile + 1);
    }
}


void hud_hide_unit_control_type(const team_t *team)
{
    for(uint8_t i = 0; i < (team->size << 1); i++)
    {
        move_sprite(sprite_indexes[--hud_global_pos], 0, 0);
        spr_free(sprite_indexes[hud_global_pos]);
    }
}