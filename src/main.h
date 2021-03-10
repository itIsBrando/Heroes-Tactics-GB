#ifndef MAIN_H
#define MAIN_H

#include "defines.h"
#include "data/sprite.h"
#include "structs.h"

extern match_t currentMatch;
void initGame();

void empty_oam();
void clear_bg();
void waitjoypad(const uint8_t);

void print(unsigned char *, uint8_t, uint8_t);
void print_window(unsigned char *, uint8_t, uint8_t);
void printInt(uint16_t, uint8_t, uint8_t, const bool);
void debug(char *);

#endif