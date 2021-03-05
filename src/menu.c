#include <gb/gb.h>

#include "menu.h"
#include "main.h"
#include "game.h"

const char control_text[][5] = {"PLR", "CPU", "LINK"};


static inline char *mnu_getControllerType(control_t t)
{
    return control_text[t];
}

static void mnu_choose_team_draw(match_t *match)
{
    for(uint8_t i = 0; i < match->numTeams; i++)
    {
        print("team", 1, i + 3);
        printInt(i, 6, i + 3, false);
        print(mnu_getControllerType(match->teams[i]->control), 9, i + 3);
    }
}


/**
 * Allows the user to choose controller type of each team
 * @param match pointer to the match. `numTeams` must be initialized
 */
void mnu_choose_teams(match_t *match)
{
    uint8_t y = 0;
    clear_bg();
    print("MATCH SET UP:", 0, 0);
    print("(A) to start game", 0, 144/8-1);

    mnu_choose_team_draw(match);

    waitjoypad(0xFF);

    uint8_t pad;

    do {
        pad = joypad();

        if((pad & J_LEFT) || (pad & J_RIGHT))
        {
            match->teams[y]->control--;
            match->teams[y]->control &= 0x1;
            mnu_choose_team_draw(match);
            waitjoypad(J_LEFT | J_RIGHT);
        }

        if((pad & J_DOWN) && y < match->numTeams)
        {
            fill_bkg_rect(0, 3, 1, 3, 0);
            print(">", 0, 3 + ++y);
            waitjoypad(J_DOWN);
        }

        if((pad & J_UP) && y > 0)
        {
            fill_bkg_rect(0, 3, 1, 3, 0);
            print(">", 0, 3 + --y);
            waitjoypad(J_UP);
        }
        
        wait_vbl_done();
    } while(pad != J_A);
}

