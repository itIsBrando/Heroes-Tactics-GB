
#include "defines.h"
#include "oam.h"
#include "main.h"

#include <string.h>

static bool spr_candidates[40];

/**
 * Frees all sprites
 */
void spr_flush() {
    memset(spr_candidates, 0, 40);
}

/**
 * Grabs a free index in oam
 * - call spr_free() to free this sprite
 * @returns a free sprite index in OAM
 */
uint8_t spr_allocate() {
    char i = -1;
    while(spr_candidates[++i])
        if(i > 39)
            print("Out of sprite indexes", 2, 2);

    spr_candidates[i] = true;

    return i;
}

/**
 * Frees a sprite that is allocated from spr_allocate()
 * - no effect if the sprite is already freed
 */
inline void spr_free(uint8_t i) {
    spr_candidates[i] = false;
}