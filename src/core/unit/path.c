
#include "path.h"
#include "../../main.h"
#include "../world/map.h"
#include "../game.h"

#include <string.h>
#include <gb/gb.h>

static queue_t queue[MAP_MAX_SIZE * 2];
static position_t pf_out[MAP_MAX_SIZE];
static position_t visited[MAP_MAX_SIZE];
static uint8_t queue_size, visited_size;

#define push_queue(curX, curY, lastPos) queue[queue_size].curPos.x = (curX), queue[queue_size].curPos.y = (curY); queue[queue_size++].lastEntry = lastPos
#define push_visited(xPos, yPos) visited[visited_size].x = xPos, visited[visited_size++].y = yPos;

const uint16_t MAX_ITER = 800;


/**
 * Creates a path between two positions
 * @param start position to start from
 * @param stop position to end with
 * @param size size of the array that will be returned
 * @returns an array of positions that will go from `start` to `end` **IN REVERSE ORDER**
 */
position_t *pf_find_xy(uint8_t xStart, uint8_t yStart, uint8_t xEnd, uint8_t yEnd, uint8_t *size)
{
    position_t start;
    position_t end;
    start.x = xStart;
    start.y = yStart;
    end.x = xEnd;
    end.y = yEnd;
    return pf_find(&start, &end, size);
}


/**
 * Creates a path between two positions
 * @param start position to start from
 * @param stop position to end with
 * @param size size of the array that will be returned
 * @returns an array of positions that will go from `start` to `end` **IN REVERSE ORDER**
 */
position_t *pf_find(position_t *start, position_t *end, uint8_t *size)
{
    uint8_t i;
    uint16_t iter = 0;
    visited_size = queue_size = 0;
    push_queue(start->x, start->y, NULL);

    // if the starting or ending position is a solid block, then we don't have a path
    if((map_fget(map_get_pos(start)) | map_fget(map_get_pos(end))) & 0x1)
        return NULL;

    i = 0;
    while(iter++ < MAX_ITER)
    {
        // break if we have made it to our goal
        if(pf_check_cell(&queue[i], end, &i))
            break;
        i++;
    }

    queue_t *q = queue[i].lastEntry;

    pf_out[0] = *end;
    i = 1; // reuse `i`
    while(q != NULL)
    {
        pf_out[i++] = q->curPos;
        q = (queue_t*)(q->lastEntry);
    }

    // printInt(i, 0, 12, false);

    (*size) = i-1;
    return pf_out;
}


/**
 * This is horribly written
 * @returns true if we reached the goal, otherwise false
 */
inline bool pf_check_cell(queue_t *curEntry, position_t *goal, uint8_t *index)
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
    
    int8_t dx[] = {-1, 1, 0, 0};
    int8_t dy[] = {0, 0, -1, 1};

    for(uint8_t i = 0; i < 4; i++)
    {
        position_t pos;
        pos.x = curPos->x + dx[i];
        pos.y = curPos->y + dy[i];
        // if(!pf_has_visited(&pos) && pf_can_move(&pos))
            push_queue(pos.x, pos.y, curEntry);
    }
    
    return false;
}

/**
 * Returns true if a position has already been visited
 */
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
    if(map_fget(tile) & 0x1)
        return false;
    
    if(!map_in_bounds(pos->x, pos->y))
        return false;

    return true;
}