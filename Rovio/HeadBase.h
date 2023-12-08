#ifndef _HEADBASE_H
#define _HEADBASE_H

#include "Entity.h"

/**
 * @brief Class to sync Rovio Head actions across ESP_NOW interface between remote Domains
 */
class HeadBase : public Entity
{
public:
    /**
     * @brief The Power applied to the Head motor, [-255..255],
     *      positive values moving the Head UP and negative DOWN
     */
    Property Power = Property(PropertyID_Power, "Power", true, true);
    /**
     * @brief The Position of the head, [0..100], but perhaps with lower resolution
     */
    Property Position = Property(PropertyID_Position, "Pos", true, false);
    Property* pa[3] = { &Power, &Position, nullptr };
    /**
     * @brief Construct a new Head Base object
     * 
     * @param entity The Entity ID
     * @param name The Entity Name
     */
    HeadBase(EntityID entity, const char* name) : Entity(entity, name, pa) {}
};

#endif // _HEADBASE_H
