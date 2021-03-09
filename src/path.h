#ifndef PATH_H
#define PATH_H

#include "defines.h"
#include "structs.h"

position_t *pf_find(position_t *, position_t *, uint8_t *);
position_t *pf_find_xy(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t *);
bool pf_can_move(position_t *pos);
bool pf_has_visited(position_t *pos);
bool pf_check_cell(queue_t *, position_t *, uint8_t *);

#endif