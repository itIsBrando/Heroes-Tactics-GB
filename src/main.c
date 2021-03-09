#include <gb/gb.h>
#include <gb/console.h>
#include <gb/font.h>

#include <stdio.h>
#include <string.h>
#include <rand.h>

#include "structs.h"
#include "main.h"
#include "map.h"
#include "units.h"
#include "hud.h"
#include "game.h"
#include "oam.h"
#include "data/mapdata.h"


match_t currentMatch;

void main(void) {
    uint8_t x = 0, y = 0;

    // load the font
    set_bkg_1bit_data(0x80, 102, font_ibm + 130, 3);

    OBP1_REG = 0b01011000;
    set_sprite_data(1, 35, SPRITE_DATA);
    set_bkg_data(1, 35, SPRITE_DATA);

    SHOW_BKG; SHOW_SPRITES;

    // set vblank interrupt
    set_interrupts(VBL_IFLAG);
    enable_interrupts();

    spr_flush();

    clear_bg();
    
    print("Press button to start", 0, 0x10);
    while(!joypad())
        wait_vbl_done();
    initrand(DIV_REG);

    initGame();
}


/**
 * Starts the game
 */
void initGame() {
    // init player's team
    team_t enemyTeam;
    team_t player_team;
    unit_t *units[3];
    
    // creates a map
    map_t m;
    m.width = 10;
    m.height = 10;
    m.data = all_maps[0];
    map_load(&m); // loads the map

    units[0] = unit_new(UNIT_TYPE_BRAWN);
    units[1] = unit_new(UNIT_TYPE_ARCHER);
    units[2] = unit_new(UNIT_TYPE_HEALER);
    player_team.control = CONTROLLER_PLAYER;
    player_team.size = 3;
    player_team.units = units;

    unit_t *enemyUnits[2];
    enemyUnits[0] = unit_new(rand() & 1);
    enemyUnits[1] = unit_new(rand() & 1);
    enemyTeam.control = CONTROLLER_COMPUTER;
    enemyTeam.size = 2;
    enemyTeam.units = enemyUnits;
    
    gme_init(&player_team, &enemyTeam, NULL);

    gme_run();
}

/**
 * Better alternative of `waitpad`. Power efficient
 */
void waitjoypad(const uint8_t mask)
{
    while((joypad() & mask) != 0)
        wait_vbl_done();
}


/**
 * Prints a string at (x, y). Checks for newlines
 * @param tx x coordinate of string
 * @param ty y coordinate of string
 */
void print(unsigned char *str, uint8_t tx, uint8_t ty)
{
    const uint8_t len = strlen(str);
    char c;

    for(uint8_t i = 0; i < len; i++)
    {
        c = str[i];
        if(c >= 'A' && c <= 'Z')
            c = c - 'A' + 0xA1;
        else if(c >= 'a' && c <= 'z')
            c = c - 'a' + 0xC1;
        else if(c >= '!' && c <= '@')
            c = c - '!' + 0x81;
        else if(c == ' ')
            c = 0;
        
        if(c == '\n' || tx >= 20)
            tx = 0, ty++;

        set_bkg_tiles(tx++, ty, 1, 1, &c);

    }

}


/**
 * Prints a string at (x, y). Checks for newlines
 * @param tx x coordinate of string
 * @param ty y coordinate of string
 */
void print_window(unsigned char *str, uint8_t tx, uint8_t ty)
{
    const uint8_t len = strlen(str);
    char c;

    for(uint8_t i = 0; i < len; i++)
    {
        c = str[i];
        if(c >= 'A' && c <= 'Z')
            c = c - 'A' + 0xA1;
        else if(c >= 'a' && c <= 'z')
            c = c - 'a' + 0xC1;
        else if(c >= '!' && c <= '9')
            c = c - '!' + 0x81;
        else if(c == ' ')
            c = 0;
        else if(c == '\n') {
            tx = 0, ty++; continue;
        }

        if(tx >= 20)
            tx = 0, ty++;

        set_win_tiles(tx++, ty, 1, 1, &c);
    }

}


/**
 * Prints a 16-bit integer
 * @param num 16-bit unsigned integer
 * @param tx x tile to draw the number at
 * @param ty y tile to draw the number at
 * @param onWindow true to draw on window, or false to draw on bg
 */
void printInt(uint16_t num, uint8_t tx, uint8_t ty, const bool onWindow) {
    uint8_t digits[5];
    int8_t i = 0;
    
    // put all digits into a buffer
    do {
        digits[i++] = (num % 10) + 0x90;
        num /= 10;
    } while(num > 0);

    // work backwards in the buffer to place them onto the screen
    do {
        if(onWindow)
            set_win_tiles(tx++, ty, 1, 1, &digits[--i]);
        else
            set_bkg_tiles(tx++, ty, 1, 1, &digits[--i]);
    } while(i > 0);
}

/**
 * Empties all of the sprites in the shadow OAM
 */
void empty_oam() {
    memset(shadow_OAM, 0, 40 * 4);
}

void clear_bg() {
    fill_bkg_rect(0, 0, 20, 18, 0);
    print("\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0\xe0", 0, 144/8-3);

}