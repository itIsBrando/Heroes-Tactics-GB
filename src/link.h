#ifndef LINK_H
#define LINK_H

#include "structs.h"


typedef enum
{
    LNK_NON,
    LNK_SEND_UNIT,
    LNK_END_TURN,
    LNK_SEND_ATTACK,
    LNK_SEND_MAP,
    LNK_SEND_HEAL,
    LNK_SEND_POSITION,
} lnk_command_types;


typedef struct {
    lnk_command_types type;
    uint8_t size;       // can be zero for no data
    void *data;         // pointer to data to use/fill
    uint8_t index;      // a single byte of extranous data
    uint8_t checksum;   // ensures that everything is sent properly
} packet_t;


typedef struct {
    uint8_t attackerIndex;
    uint8_t defenderIndex;
} packet_attack_t;

typedef struct {
    uint8_t x, y;
    uint8_t unitIndex;
} packet_position_t;

typedef struct {
    uint8_t width, height;
    bool hasFog;
    uint8_t data[MAP_MAX_SIZE];
} packet_map_t;


bool lnk_is_multiplayer_battle();
bool lnk_is_leading();
void lnk_init(match_t *);

void lnk_do_turn();
void lnk_change_turn();

void lnk_send_unit(unit_t *unit);
void lnk_send_unit_position(unit_t *unit);
void lnk_send_attack(unit_t *attacker, unit_t *defender);
void lnk_send_heal(unit_t *healer, unit_t *other);
void lnk_send_map(map_t *map);


void lnk_wait_attack_complete();

uint8_t lnk_receive();
bool lnk_send(uint8_t);

void lnk_report_error();

void lnk_send_packet(const packet_t *packet);
bool lnk_wait_packet(packet_t *packet, const uint8_t);

bool lnk_validate_checksum(const packet_t *);
void lnk_gen_checksum(packet_t *);

void lnk_decode_attack(packet_t *packet);
void lnk_decode_heal(packet_t *packet);
void lnk_decode_position(packet_t *packet);
void lnk_decode_map(packet_t *packet);

#endif