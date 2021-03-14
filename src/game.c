
#include "game.h"
#include "units.h"
#include "structs.h"
#include "hud.h"
#include "menu.h"
#include "main.h"
#include "cursor.h"
#include "map.h"
#include "path.h"
#include "ai.h"

#include <gb/gb.h>

static unit_t *selectedUnit = NULL;
static bool isAttacking;
team_t *currentTeam;

/**
 * Runs a match
 */
void gme_run()
{
    add_VBL(unit_vbl_int);

    currentTeam = currentMatch.teams[0];
    hud_draw_hotbar(currentTeam);

    // draw background
    clear_bg();
    map_draw();
    
    // initialize cursor
    cur_init();

    // draws all team members
    for(uint8_t i = 0; i < currentMatch.numTeams; i++)
        mth_draw_team(currentMatch.teams[i]);

    uint8_t win;

    // main loop
    do {
        gme_do_turn();

        win = mth_finished();
        if(win != 255) {
            
            break;
        }
    } while(true);

    print("Team  has won.", 0, 0);
    printInt(win + 1, 4, 0, false);

    remove_VBL(unit_vbl_int);
    cur_destroy();
}


void gme_deselect_unit()
{
    selectedUnit = NULL;
    unit_hide_triangle();
}


/**
 * Runs the current turn for the team in `currentTeam`
 * - changes the turn
 */
void gme_do_turn()
{
    switch (currentTeam->control)
    {
    case CONTROLLER_PLAYER:
        gme_player_turn();
        break;
    case CONTROLLER_COMPUTER:
        gme_computer_turn();
        break;
    default:
        print("Unsupported control method", 0, 0);
    }

    mth_change_turn();
}


/**
 * Runs the turn for a computer
 */
void gme_computer_turn()
{
    for(uint8_t i = 0; i < currentTeam->size; i++)
        ai_do_turn(currentTeam->units[i]);
}

/**
 * Runs the turn for a human player
 */
void gme_player_turn()
{
    uint8_t pad;

    selectedUnit = NULL;
    cur_show();
begin:
    do {
        pad = joypad();

        if(pad & J_LEFT)
            cur_move(-1, 0);
        if(pad & J_RIGHT)
            cur_move(1, 0);
        if(pad & J_UP)
            cur_move(0, -1);
        if(pad & J_DOWN)
            cur_move(0, 1);


        if(pad & J_SELECT)
        {
            hud_show_details(cur_get_x(), cur_get_y());
            waitjoypad(J_SELECT);
            hud_hide_details();
        }

        // select a unit
        if(pad & J_A)
        {
            gme_select_a();
            waitjoypad(J_A);
        } else if(pad & J_B)
        {
            gme_select_b();
            waitjoypad(J_B);
        }

        wait_vbl_done();

        // break condition depends on short circuiting
        if((pad & J_START) && hud_confirm_end_turn())
            break;
    } while(true);


    if(selectedUnit)
        unit_hide_triangle();

    cur_hide();
}

/**
 * Called when the `A` button is pressed
 */
void gme_select_a()
{
    const uint8_t cx = cur_get_x(), cy = cur_get_y();

    // performs a move
    if(selectedUnit)
    {
        position_t pos;
        pos.x = cx;
        pos.y = cy;

        hud_hide_action();
        unit_hide_triangle();
        unit_move_path_find(selectedUnit, &pos);
        gme_deselect_unit();
        return; 
    } else
    {
        // if nothing is currently selected, then select a unit now
        selectedUnit = unit_get(mth_get_current_team(), cx, cy);

        if(!selectedUnit)
            return;

        // if the unit is already moved, prevent it from selecting
        if(selectedUnit->hasMoved) {
            selectedUnit = NULL;
            hud_warn("Unit has moved");
        } else {
            hud_show_action(HUD_ACTION_MOVE);
            unit_move_diamond(selectedUnit);
        }
    }
}

/**
 * Selects a unit at the cursor for attacking
 */
void gme_select_b()
{
    const uint8_t cx = cur_get_x(), cy = cur_get_y();

    if(selectedUnit)
    {
        hud_hide_action();
        // check to see if we can heal
        if(selectedUnit->type == UNIT_TYPE_HEALER)
        {
            unit_t *unit = unit_get(mth_get_current_team(), cx, cy);
            if(unit)
            {
                // if we cannot heal, then warn
                if(!unit_heal(unit, selectedUnit))
                    hud_warn("Unit cannot heal");
                gme_deselect_unit();
                return;
            }
        }

        unit_t *def = unit_get(mth_get_opponent(), cx, cy);
        if(def && unit_get_distance(selectedUnit, def) <= selectedUnit->stats.damageRadius)
        {
            unit_attack(selectedUnit, def);
            hud_draw_hotbar(currentTeam);
        } else {
            hud_warn("Cannot atk here");
        }

        gme_deselect_unit();
        return;
    }

    selectedUnit = unit_get(currentTeam, cx, cy);
    
    if(!selectedUnit)
        return;
    
    if(selectedUnit->hasAttacked)
    {
        selectedUnit = NULL;
        hud_warn("Unit has attacked");
    } else {
        hud_show_action(HUD_ACTION_ATK);
        unit_atk_diamond(selectedUnit);
    }

}

/**
 * Initializes the `currentMatch` struct
 * @param t1 first team (Must exist)
 * @param t2 second team (Must exist)
 * @param t3 third team (optional)
 */
void gme_init(team_t *t1, team_t *t2, team_t *t3)
{
    currentMatch.numTeams = t3 ? 3 : 2;
    currentMatch.teams[0] = t1;
    currentMatch.teams[1] = t2;
    currentMatch.teams[2] = t3;

    mnu_choose_teams_init(&currentMatch);
}

/**
 * @returns the team whose turn it current is
 */
inline team_t *mth_get_current_team()
{
    return currentTeam;
}

/**
 * Uses `currentTeam` to determine an opposing team
 * @returns the first occurance of an opponent team
 */
team_t *mth_get_opponent()
{
    for(uint8_t i = 0; i < currentMatch.numTeams; i++)
        if(currentTeam != currentMatch.teams[i])
           return currentMatch.teams[i];

    return NULL;
}

/**
 * @return the index of the team that has won, otherwise 0xFF
 */
uint8_t mth_finished()
{
    uint8_t index = 0xff;

    for(uint8_t i = 0; i < currentMatch.numTeams; i++)
    {
        if(mth_has_alive(currentMatch.teams[i]))
        {
            if(index == 0xFF)
                index = i;
            else
                return 0xFF;
        }
    }

    return index;
}


// does not support third teams yet
void mth_change_turn()
{
    if(currentTeam == currentMatch.teams[1])
        currentTeam = currentMatch.teams[0];
    else
        currentTeam = currentMatch.teams[1];

    // allow all units to move and attack again
    for(uint8_t i = 0; i < currentTeam->size; i++)
        currentTeam->units[i]->hasAttacked = currentTeam->units[i]->hasMoved = false;

    // make a banner
    print(currentTeam->control == CONTROLLER_PLAYER ? "PLR" : "CPU", 0, 10);
    printInt(currentTeam == currentMatch.teams[0] ? 1 : 2, 4, 10, false);
    print("turn.\n (A) to continue", 6, 10);

    // redraw window HUD
    hud_draw_hotbar(currentTeam);

    // wait until `A` button pressed
    while(joypad() != J_A)
        wait_vbl_done();

    fill_bkg_rect(0, 10, 20, 2, 0);
}


/**
 * Checks if there are any live units on this team
 * @param team team_t to check
 * @return true if at least one unit is alive, false if all units are dead
 */
bool mth_has_alive(team_t *team)
{
    for(uint8_t i = 0; i < team->size; i++)
    {
        if(!team->units[i]->isDead)
            return true;
    }

    return false;
}

inline match_t *mth_get_match()
{
    return &currentMatch;
}


/**
 * Draws all of the units in a team
 */
void mth_draw_team(team_t *team)
{
    for(uint8_t i = 0; i < team->size; i++)
    {
        if(team->units[i]->isDead) {
            unit_hide(team->units[i]);
        } else {
            unit_draw(team->units[i]);
            // draw team 1 with a different palette
            if(team == currentMatch.teams[1])
                set_sprite_prop(team->units[i]->spriteNumber, 0x10 | 1);
        }
    }
}