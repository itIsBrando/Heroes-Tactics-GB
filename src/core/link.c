#include <gb/gb.h>
#include <string.h>

#include "link.h"
#include "world/map.h"
#include "../main.h"
#include "unit/units.h"
#include "game.h"
#include "battle/cursor.h"
#include "gfx/hud.h"

static bool isMulti = false;
static bool isMaster;

/**
 * @returns true if we in a multiplayer battle
 */
inline bool lnk_is_multiplayer_battle()
{
    return isMulti;
}



void lnk_init(match_t *match)
{
    isMaster = match->teams[1]->control == CONTROLLER_LINK;
    isMulti = isMaster || match->teams[0]->control == CONTROLLER_LINK;

    if(!lnk_is_multiplayer_battle())
        return;

    set_interrupts(VBL_IFLAG | SIO_IFLAG);

    waitjoypad(0xff);

    print("Waiting...", 0, 12);

    // do handshake
    if(isMaster) {
        while(!lnk_send(0x5));
    }
    else {
        if(lnk_receive() != 0x5)
            lnk_report_error();
    }

    if(isMaster) {
        print("Sending map...", 0, 10);
        lnk_send_map(map_get_active());
    } else {
        packet_t packet;
        uint8_t buffer[sizeof(packet_map_t)];
        packet.data = buffer;
        print("Getting map...", 0, 10);
        lnk_wait_packet(&packet, sizeof(buffer));

        if(packet.type != LNK_SEND_MAP)
            lnk_report_error();
        
        lnk_decode_map(&packet);
    }

    print("Cable connection successful", 0, 12);
    waitPressed(J_A);
}


/**
 * Sends a byte over the link cable. Waits until the byte is sent
 * @param byte data to send over
 * @returns true if successful, otherwise false
 */
bool lnk_send(uint8_t byte)
{
    _io_out = byte;
    send_byte();
    while(_io_status == IO_SENDING)
        __asm__("halt"); // wait_vbl_done();

    delay(10);//wait_vbl_done(); // added delay for sending
    return _io_status != IO_ERROR;
}


/**
 * Gets a byte from the link cable. Waits until the byte is received
 * @returns the byte received
 */
uint8_t lnk_receive()
{
    receive_byte();
    while(_io_status == IO_RECEIVING)
        __asm__("halt"); //wait_vbl_done();

    return _io_in;
}


/**
 * Interrupts the program flow and informs the user of a critical error
 */
void lnk_report_error()
{
    // clear_bg();
    hud_warn("Error with link cable. Try reconnecting the cable and trying again");
    waitPressed(J_A);
}


/**
 * Does a turn for a player who is on another gameboy
 */
void lnk_do_turn()
{
    packet_t packet;
    uint8_t buffer[sizeof(unit_t)];
    const team_t *team = mth_get_current_team();

    cur_hide();
    packet.data = buffer;

    while(true)
    {
        lnk_wait_packet(&packet, sizeof(buffer));

        if(!lnk_validate_checksum(&packet))
            lnk_report_error();

        switch (packet.type)
        {
        case LNK_SEND_UNIT:
            memcpy(team->units[packet.index], packet.data, sizeof(unit_t));
            unit_draw_paletted(team->units[packet.index], team);
            break;
        case LNK_SEND_ATTACK:
            lnk_decode_attack(&packet);
            break;
        case LNK_SEND_HEAL:
            lnk_decode_heal(&packet);
            break;
        case LNK_END_TURN:
            cur_show();
            return;
        default:
            hud_warn("Unknown packet type");
            printInt(packet.type, 0, 10, false);
            break;
        }
        
    }

}


/**
 * Sends a packet indicating the transition of a turn
 */
void lnk_change_turn()
{
    packet_t packet;
    packet.size = 0;
    packet.type = LNK_END_TURN;
    lnk_send_packet(&packet);
}


/**
 * Sends a packet over to another gameboy
 */
void lnk_send_packet(const packet_t *packet)
{
    uint8_t *ptr = packet->data;
    lnk_send(packet->size);
    lnk_send(packet->type);

    delay(10); // wait for a frame just to be safe

    if(packet->size > 0)
    {
        for(uint8_t i = 0; i < packet->size; i++)
        {
            lnk_send(*ptr);
            ptr++;
        }
        
    }

    lnk_send(packet->index);
    // delay(50);
    lnk_send(packet->checksum);
    // hud_warn("Sent packet");
}


/**
 * Waits until a packet is available for receiving
 * @param packet variable to fill with data
 * @param bufferSize max size of the buffer in packet->data
 * @returns false if we could not receive our packet
 */
bool lnk_wait_packet(packet_t *packet, const uint8_t bufferSize)
{
    uint8_t *ptr = packet->data;
    packet->size = lnk_receive();
    packet->type = lnk_receive();

    if(packet->size > bufferSize)
        lnk_report_error();
    
    // read all data
    if(packet->size > 0)
    {
        for(uint8_t i = 0; i < packet->size; i++)
        {
            *ptr = lnk_receive();
            ptr++;
        }
    }

    packet->index = lnk_receive();
    packet->checksum = lnk_receive();

    // hud_warn("Received packet");
    return true;
}


/**
 * Checks to see if the checksum on a received packet matches its expected value
 * @returns true if no data is in packet or if checksums match
 */
bool lnk_validate_checksum(const packet_t *packet)
{
    if(packet->size == 0)
        return true;

    uint8_t checksum = 0;
    
    for(uint8_t i = 0; i < packet->size; i++)
    {
        checksum += ((uint8_t*)packet->data)[i];
    }

    return true;
    // return checksum == packet->checksum;
}


/**
 * Creates a checksum for the data in this packet
 */
void lnk_gen_checksum(packet_t *packet)
{
    packet->checksum = 0;

    for(uint8_t i = 0; i < packet->size; i++)
        packet->checksum += ((uint8_t*)packet->data)[i];

}


/**
 * @param unit unit to send
 */
void lnk_send_unit(unit_t *unit)
{
    packet_t packet;
    packet.size = sizeof(unit_t);
    packet.type = LNK_SEND_UNIT;
    packet.data = unit;
    packet.index = unit_get_index(unit, mth_get_current_team());
    lnk_gen_checksum(&packet);

    lnk_send_packet(&packet);
}


/**
 * Sends over the complete data for a map
 * @param map data to transmit
 */
void lnk_send_map(map_t *map)
{
    packet_t packet;
    packet_map_t data;

    data.hasFog = map_has_fog();
    data.width = map->width;
    data.height = map->height;
    memcpy(data.data, map->data, map->size);

    packet.size = sizeof(packet_map_t);
    packet.type = LNK_SEND_MAP;
    packet.data = &data;

    lnk_gen_checksum(&packet);
    lnk_send_packet(&packet);
}


/**
 * Decodes a map
 */
void lnk_decode_map(packet_t *packet)
{
    packet_map_t *data = packet->data;

    map_load_from_data(data->data, data->width, data->height, data->hasFog);

    match_t *match = mth_get_match();
    // this beautiful forloop was never needed :'(
    // for(team_t *team = match->teams; team < &(match->teams[match->numTeams]); team += sizeof(team_t))
    map_init_spawn(match->teams[0], false);
    map_init_spawn(match->teams[1], true);
}


/**
 * @param packet incoming packet
 */
void lnk_decode_attack(packet_t *packet)
{
    packet_attack_t *data = packet->data;
    unit_t *attacker = mth_get_current_team()->units[data->attackerIndex];
    unit_t *defender = mth_get_opponent()->units[data->defenderIndex];

    unit_attack(attacker, defender);
    cur_hide();
}


/**
 * @param attacker must be part of the current team
 * @param defender must be part of the opposing team
 */
void lnk_send_attack(unit_t *attacker, unit_t *defender)
{
    packet_t packet;
    packet_attack_t packetData;

    packetData.attackerIndex = unit_get_index(attacker, mth_get_current_team());
    packetData.defenderIndex = unit_get_index(defender, mth_get_opponent());
    packet.type = LNK_SEND_ATTACK;
    packet.size = sizeof(packet_attack_t);
    packet.data = &packetData;

    lnk_gen_checksum(&packet);
    lnk_send_packet(&packet);
}


/**
 * Sends over the position of a unit
 * @param unit unit position MUST BE PART OF CURRENT TEAM
 */
void lnk_send_unit_position(unit_t *unit)
{
    packet_t packet;
    packet_position_t packetData;

    packetData.x = unit->row;
    packetData.y = unit->column;
    packetData.unitIndex = unit_get_index(unit, mth_get_current_team());

    packet.type = LNK_SEND_POSITION;
    packet.size = sizeof(packet_position_t);
    packet.data = &packetData;

    lnk_gen_checksum(&packet);
    lnk_send_packet(&packet);
}


/**
 * @param healer must be part of the current team
 * @param other must be part of the current team. Unit to be healed
 */
void lnk_send_heal(unit_t *healer, unit_t *other)
{
    packet_t packet;
    packet_attack_t packetData;

    packetData.attackerIndex = unit_get_index(healer, mth_get_current_team());
    packetData.defenderIndex = unit_get_index(other, mth_get_current_team());
    packet.type = LNK_SEND_HEAL;
    packet.size = sizeof(packet_attack_t);
    packet.data = &packetData;

    lnk_gen_checksum(&packet);
    lnk_send_packet(&packet);
}


/**
 * @param packet incoming packet
 */
void lnk_decode_heal(packet_t *packet)
{
    packet_attack_t *data = packet->data;
    unit_t *healer = mth_get_current_team()->units[data->attackerIndex];
    unit_t *other = mth_get_current_team()->units[data->defenderIndex];

    if(!unit_heal(other, healer))
        lnk_report_error();

    cur_hide();
}


/**
 * @param packet incoming packet
 */
void lnk_decode_position(packet_t *packet)
{
    packet_position_t *data = packet->data;

    unit_t *unit = mth_get_current_team()->units[data->unitIndex];
    
    unit->row = data->x;
    unit->column = data->y;
    unit_draw(unit);
}


/**
 * Waits until both players have finished watching an attack
 */
void lnk_wait_attack_complete()
{
    print_window("Waiting for opponent", 0, 3);
    if(isMaster)
        while(!lnk_send(0x6));
    else
        while(lnk_receive() != 0x6);

    fill_win_rect(0, 3, 20, 1, 0);
}


/**
 * Checks if THIS gameboy is one whose controlling the turn
 */
bool lnk_is_leading()
{
    return lnk_is_multiplayer_battle() && mth_get_current_team()->control == CONTROLLER_PLAYER;
}