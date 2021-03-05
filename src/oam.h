#ifndef OAM_H
#define OAM_H

#include "structs.h"

uint8_t spr_allocate();
void spr_free(uint8_t i);
void spr_flush();

#endif