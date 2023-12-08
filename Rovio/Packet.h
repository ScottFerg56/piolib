#ifndef _PACKET_H
#define _PACKET_H

#include <Arduino.h>

#include "Entity.h"

/**
 * @brief Data Packet for communication between Domains;
 *      1 or more Packets per transmission
 */
struct Packet
{
    /**
     * @brief The ID of the Entity changing
     */
    EntityID entity;
    /**
     * @brief The ID of the Property changing
     */
    PropertyID property;
    /**
     * @brief The changed Value of the Property changing
     */
    int16_t value;
};

#endif  // _PACKET_H
