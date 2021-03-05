
#include "path.h"
#include "map.h"
#include "units.h"
#include "game.h"
#include "main.h"

#include <string.h>
#include <gb/gb.h>

static queue_t queue[MAP_SIZE * 2];
static position_t pf_out[MAP_SIZE];
static position_t visited[MAP_SIZE];
static uint8_t queue_size, visited_size;

#define push_queue(curX, curY, lastPos) queue[queue_size].curPos.x = (curX), queue[queue_size].curPos.y = (curY); queue[queue_size++].lastEntry = lastPos

const uint16_t MAX_ITER = 800;

/**
 * Creates a path between two positions
 * @param start position to start from
 * @param stop position to end with
 * @param size size of the array that will be returned
 * @returns an array of positions that will go from `start` to `end` **IN REVERSE ORDER**
 */
position_t *pf_find(position_t *start, position_t *end, uint8_t *size)
{
    uint8_t i = 0;
    uint16_t iter = 0;
    visited_size = queue_size = 0;
    push_queue(start->x, start->y, NULL);

    if(map_fget(map_get_pos(start)) == 1 || map_fget(map_get_pos(end)) == 1)
        return NULL;

    while(iter++ < MAX_ITER)
    {
        // break if we have made it to our goal
        if(pf_check_cell(&queue[i], end, &i))
            break;
        i++;
        
    }

    queue_t *q = (queue[i].lastEntry);

    pf_out[0] = *end;
    i = 1; // reuse `i`
    while(q != NULL)
    {
        pf_out[i++] = q->curPos;
        q = (queue_t*)(q->lastEntry);
    }

    printInt(queue_size, 0, 11, false);

    (*size) = i;
    return pf_out;
}


/**
 * This is horribly written
 * @returns true if we reached the goal, otherwise false
 */
bool pf_check_cell(queue_t *curEntry, position_t *goal, uint8_t *index)
{
    position_t *curPos = &(curEntry->curPos);
    // delete position if it is irrelevant
    if(!pf_can_move(curPos) || pf_has_visited(curPos))
    {
        memcpy(queue + *index, queue + *index + 1, (queue_size - *index) * sizeof(queue[0]));
        (*index)--;
        queue_size--;
        return false;
    }
    
    if(goal->x == curPos->x && goal->y == curPos->y)
        return true;

    // declare the fact that we have landed on this tile
    visited[visited_size++] = *curPos;
    // fill_bkg_rect(curPos->x, curPos->y, 1, 1, 4);
    
    push_queue(curPos->x + 1, curPos->y, curEntry);
    push_queue(curPos->x - 1, curPos->y, curEntry);
    push_queue(curPos->x, curPos->y + 1, curEntry);
    push_queue(curPos->x, curPos->y - 1, curEntry);
    
    return false;
}


bool pf_has_visited(position_t *pos)
{
    for(uint8_t i = 0; i < visited_size; i++)
    {
        if(visited[i].x == pos->x && visited[i].y == pos->y)
            return true;
    }

    return false;
}


/**
 * @returns true if the position is a valid place to move to
 */
bool pf_can_move(position_t *pos)
{
    const uint8_t tile = map_get(pos->x, pos->y);
    if(map_fget(tile))
        return false;
    
    if(!map_in_bounds(pos->x, pos->y))
        return false;

    return true;
}