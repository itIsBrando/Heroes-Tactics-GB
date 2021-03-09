#ifndef GAME_H
#define GAME_H

#include "structs.h"

extern team_t *currentTeam;

void gme_run();
void gme_init(team_t *, team_t *, team_t *);
void gme_select_b(); // select a unit
void gme_select_a(); // select a unit

void gme_do_turn();
void gme_computer_turn();
void gme_player_turn();

team_t *mth_get_opponent();
team_t *mth_get_current_team();
match_t *mth_get_match();
bool mth_has_alive(team_t *team);
uint8_t mth_finished();
void mth_change_turn();
void mth_draw_team(team_t *team);
#endif
