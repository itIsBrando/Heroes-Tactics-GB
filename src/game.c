#include "cgb.h"
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

static bool isAttacking;
team_t *currentTeam;

typedef struct {
    unit_t *selectedUnit;
    bool isAttacking; // false if moving, true if attacking
    bool isSelected;
} gme_status_t;

static gme_status_t game_state;


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
    
    // initialize cursor
    cur_init();
    mth_print_team();

    // draws all team members
    for(uint8_t i = 0; i < currentMatch.numTeams; i++)
        mth_draw_team(currentMatch.teams[i]);

    map_draw();
    cgb_map();

    int8_t win;

    // main loop
    do {
        gme_do_turn();

        win = mth_finished();
    } while(win == -1);

    print("Team  has won.", 0, 0);
    printInt(win + 1, 4, 0, false);

    remove_VBL(unit_vbl_int);
    cur_destroy();
}


/**
 * Stops selecting a unit
 */
void gme_deselect_unit()
{
    game_state.selectedUnit = NULL;
    hud_hide_action();
    unit_hide_triangle();
}

/**
 * @returns true if a unit is selected
 */
inline bool gme_is_unit_selected()
{
    return game_state.selectedUnit != NULL;
}


/**
 * Selects a unit
 * @param unit unit to select. If NULL, then does nothing
 * @param shouldAttack true if the unit is being selected to attack, otherwise it will move
 */
void gme_select_unit(unit_t *unit, bool shouldAttack)
{
    game_state.selectedUnit = NULL;
    
    if(unit == NULL)
        return;
    
    // if the unit is already moved, prevent it from selecting
    if(!shouldAttack && unit->hasMoved) {
        hud_warn("Unit has moved");
        return;
    } else if(shouldAttack && unit->hasAttacked) {
        hud_warn("Unit has attacked");
        return;
    }

    if(shouldAttack) {
        hud_show_action(HUD_ACTION_ATK);
        unit_atk_diamond(unit);
    } else {
        hud_show_action(HUD_ACTION_MOVE);
        unit_move_diamond(unit);
    }

    game_state.selectedUnit = unit;
    game_state.isAttacking = shouldAttack;
    game_state.isSelected = true;
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

    game_state.selectedUnit = NULL;
    cur_show();
    
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


    if(gme_is_unit_selected())
        unit_hide_triangle();

    cur_hide();
}


/**
 * Attacks or heals using the selected unit
 * @note called when the attack triangle is visible and player has hit the `A` button
 */
static void gme_attack()
{
    const uint8_t cx = cur_get_x(), cy = cur_get_y();

    // if we are highlighting our own character, then deselect
    if(unit_get(mth_get_current_team(), cx, cy) == game_state.selectedUnit)
    {
        game_state.selectedUnit->hasAttacked = true;
        unit_draw_paletted(game_state.selectedUnit, mth_get_current_team());
        gme_deselect_unit();
    }

    if(game_state.selectedUnit->type == UNIT_TYPE_HEALER)
    {
        unit_t *ally = unit_get(mth_get_current_team(), cx, cy);
        if(ally)
        {
            // if we cannot heal, then warn
            if(!unit_heal(ally, game_state.selectedUnit))
                hud_warn("Unit cannot heal");
            else
                gme_deselect_unit();
            return;
        }
    }

    unit_t *def = unit_get(mth_get_opponent(), cx, cy);
    if(def
      && unit_get_distance(game_state.selectedUnit, def) <= game_state.selectedUnit->stats.damageRadius)
    {
        unit_attack(game_state.selectedUnit, def);
        gme_deselect_unit();
    } else {
        hud_warn("Cannot atk here");
    }
}


/**
 * Called when the `A` button is pressed
 */
void gme_select_a()
{
    const uint8_t cx = cur_get_x(), cy = cur_get_y();

    // performs a move
    if(gme_is_unit_selected())
    {
        position_t pos;
        pos.x = cx;
        pos.y = cy;

        if(game_state.isAttacking)
        {
            gme_attack();
        } else {
            unit_t *enemy = unit_get(mth_get_opponent(), cx, cy);

            unit_hide_triangle();
            
            // check if we can attack without moving
            if(unit_in_atk_range(game_state.selectedUnit, enemy))
            {
                gme_attack();
            // check to see if we can move somewhere
            } else if(unit_move_path_find(game_state.selectedUnit, &pos))
                gme_select_unit(game_state.selectedUnit, true);
            else
                gme_deselect_unit();
        }
    } else
    {
        gme_select_unit(unit_get(mth_get_current_team(), cx, cy), false);
    }
}

/**
 * Selects a unit at the cursor for attacking
 */
void gme_select_b()
{
    const uint8_t cx = cur_get_x(), cy = cur_get_y();

    if(gme_is_unit_selected())
    {
        if(game_state.isAttacking)
        {
            game_state.selectedUnit->hasAttacked = true;
            unit_draw_paletted(game_state.selectedUnit, currentTeam);
        }
        hud_hide_action();
        gme_deselect_unit();
        return;
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
 * @return the index of the team that has won, otherwise -1
 */
int8_t mth_finished()
{
    int8_t index = -1;

    for(uint8_t i = 0; i < currentMatch.numTeams; i++)
    {
        if(mth_has_alive(currentMatch.teams[i]))
        {
            if(index == -1)
                index = i;
            else
                return -1;
        }
    }

    return index;
}


/*
 * does not support third teams yet
 */
void mth_change_turn()
{
    team_t *prevTeam = currentTeam;

    if(mth_get_team_number() == 2)
        currentTeam = currentMatch.teams[0];
    else
        currentTeam = currentMatch.teams[1];

    // allow all units to move and attack again
    for(uint8_t i = 0; i < currentTeam->size; i++)
        currentTeam->units[i]->hasAttacked = currentTeam->units[i]->hasMoved = false;

    // make a banner
    //@todo improve!!!
    print(currentTeam->control == CONTROLLER_PLAYER ? "PLR" : "CPU", 0, 10);
    printInt(currentTeam == currentMatch.teams[0] ? 1 : 2, 4, 10, false);
    print("turn.\n (A) to continue", 6, 10);

    // redraw old team (necessary for CGB pal & fog)
    mth_draw_team(prevTeam);

    mth_print_team();    

    // redraw window HUD
    hud_draw_hotbar(currentTeam);


    // wait until `A` button pressed
    while(joypad() != J_A)
        wait_vbl_done();

    fill_bkg_rect(0, 10, 20, 2, 0);
}


/**
 * Draws the team that is currently playing
 */
void mth_print_team()
{
    // draw team number
    uint8_t teamNum = mth_get_team_number();
    print("TEAM", map_get_width(), 0);
    printInt(teamNum, map_get_width() + 4, 0, false);

    if(is_cgb())
    {
        VBK_REG = 1;
        fill_bkg_rect(map_get_width(), 0, 5, 1, cgb_get_team_palette(mth_get_current_team()));
        VBK_REG = 0;
    }
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
 * @returns the team number that is active (1 or 2)
 */
inline uint8_t mth_get_team_number()
{
    return (currentTeam == currentMatch.teams[0]) ? 1 : 2;
}


/**
 * Draws all of the units in a team
 */
void mth_draw_team(team_t *team)
{
    for(uint8_t i = 0; i < team->size; i++)
        unit_draw_paletted(team->units[i], team);
}