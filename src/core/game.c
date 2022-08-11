#include "game.h"
#include "gfx/cgb.h"
#include "gfx/hud.h"
#include "unit/units.h"
#include "../structs.h"
#include "gfx/menu.h"
#include "../main.h"
#include "battle/cursor.h"
#include "world/map.h"
#include "unit/path.h"
#include "unit/ai.h"
#include "battle/diamond.h"
#include "link.h"

#include <gb/gb.h>

static bool isAttacking;
team_t *currentTeam;

typedef struct {
    unit_t *selectedUnit;
    bool isAttacking; // false if moving, true if attacking
    bool isSelected;
    position_t oPos; // position of the unit when it was first selected
} gme_status_t;

static gme_status_t game_state;


/**
 * Runs a match
 */
void gme_run()
{
    add_VBL(unit_vbl_int);
    add_VBL(map_vbl_int);

    // draw background
    clear_bg();

    // draws all team members
    for(uint8_t i = 0; i < currentMatch.numTeams; i++)
        mth_draw_team(currentMatch.teams[i]);

    currentTeam = currentMatch.teams[0];
    map_draw();
    cgb_map();
    hud_draw_hotbar(currentTeam);
    move_win(7, 144 - 40); // set window position


    // sneakily set the turn to team 0
    currentTeam = currentMatch.teams[1];
    mth_change_turn();
    
    
    // initialize cursor
    cur_init();
    mth_print_team();

    int8_t win;

    // main loop
    do {
        gme_do_turn();

        win = mth_finished();
    } while(win == -1);

    print("Team  has won.", 0, 10);
    printInt(win + 1, 4, 10, false);

    remove_VBL(unit_vbl_int);
    cur_destroy();

    print_window("\x1D to continue", 0, 3);

    waitPressed(0xff);
    reset();
}


/**
 * Stops selecting a unit
 */
void gme_deselect_unit()
{
    if(game_state.selectedUnit)
    {
        unit_draw_paletted(game_state.selectedUnit, mth_get_current_team());
        game_state.selectedUnit = NULL;
    }
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
    game_state.oPos.x = unit->row;
    game_state.oPos.y = unit->column;
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
    case CONTROLLER_LINK:
        lnk_do_turn();
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
            hud_show_unit_control_type(mth_get_current_team());
            hud_show_unit_control_type(mth_get_opponent());
            waitjoypad(J_SELECT);

            hud_hide_details();
            hud_hide_unit_control_type(mth_get_opponent());
            hud_hide_unit_control_type(mth_get_current_team());
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
    position_t pos;
    pos.x = cx;
    pos.y = cy;

    // performs a move
    if(!gme_is_unit_selected())
    {
        gme_select_unit(unit_get(mth_get_current_team(), cx, cy), false);
        return;
    }

    if(game_state.isAttacking)
    {
        gme_attack();
        return;
    }

    unit_t *enemy = unit_get(mth_get_opponent(), cx, cy);

    unit_hide_triangle();
    
    // check if we can attack without moving
    if(unit_in_atk_range(game_state.selectedUnit, enemy))
    {
        gme_attack();
    // check to see if we can move somewhere
    } else if(unit_move_path_find(game_state.selectedUnit, &pos))
        // show menu to determine what we do after moving
        switch (hud_unit_attack_menu())
        {
        case 1:   
            game_state.selectedUnit->hasAttacked = true;
            gme_deselect_unit();                
            break;
        case 2:
            unit_move_to(game_state.selectedUnit, game_state.oPos.x, game_state.oPos.y);
            game_state.selectedUnit->hasMoved = false;
            gme_deselect_unit();
            break;
        default:
            gme_select_unit(game_state.selectedUnit, true);
        }
    else { // if we cannot move to the tile at the cursor
        if(game_state.selectedUnit->type == UNIT_TYPE_HEALER)
            gme_attack();
        gme_deselect_unit();
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
    } else
    {
        // view attack range of unit
        unit_t *unit = unit_get_any(cx, cy);

        if(!unit)
            return;
        
        hud_show_action(HUD_ACTION_RANGE);
        uint8_t range = unit->stats.damageRadius + unit->stats.movePoints;
        tri_make(unit->row, unit->column, range);
        tri_clip();
        tri_draw(0x9);
        cgb_diamond(3);

        waitjoypad(J_B);
        tri_hide();
        hud_hide_action();
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

    map_init_spawn(t1, false);
    map_init_spawn(t2, true);

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

    // indicate that we are changing turns if we are multiplayer
    if(lnk_is_multiplayer_battle() && currentTeam->control == CONTROLLER_PLAYER)
        lnk_change_turn();

    if(mth_get_team_number() == 2)
        currentTeam = currentMatch.teams[0];
    else
        currentTeam = currentMatch.teams[1];

    // allow all units to move and attack again
    for(uint8_t i = 0; i < currentTeam->size; i++)
        currentTeam->units[i]->hasAttacked = currentTeam->units[i]->hasMoved = false;

    
    // redraw old team (necessary for CGB pal & fog)
    hud_change_turn_banner();
    mth_draw_team(prevTeam);
    map_changed_turns();

    mth_print_team();    

    // redraw window HUD
    hud_draw_hotbar(currentTeam);


    // wait until `A` button pressed
    waitPressed(J_A | J_START);

    fill_bkg_rect(0, 10, 20, 2, 0);
    hud_change_turn_banner_cleanup();
}


/**
 * Draws the team that is currently playing
 */
void mth_print_team()
{
    // draw team number
    const uint8_t w = map_get_width();
    print("TEAM", w, 0);
    printInt(mth_get_team_number(), w + 4, 0, false);

    print(
        mth_get_current_team()->control == CONTROLLER_PLAYER ? "YOU" : "OPP",
         w + 1, 1
    );

    if(is_cgb())
    {
        VBK_REG = 1;
        fill_bkg_rect(w, 0, 5, 1, cgb_get_team_palette(mth_get_current_team()));
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
    const bool hasFog = map_has_fog();

    for(uint8_t i = 0; i < team->size; i++)
        if(hasFog && map_get_with_fog(team->units[i]->row, team->units[i]->column) == 23)
            unit_hide(team->units[i]);
        else
            unit_draw_paletted(team->units[i], team);
}