#ifndef _MOTORBASE_H
#define _MOTORBASE_H

#include "Entity.h"

/**
 * @brief Class to sync Rovio drive motor actions across ESP_NOW interface between remote Domains
 */
class MotorBase : public Entity
{
public:
    /**
     * @brief The desired RPM for the motor [-100..100]
     */
    Property Goal = Property(PropertyID_Goal, "Goal", false, true);
    /**
     * @brief The observed RPM for the motor [-100..100]
     */
    Property RPM = Property(PropertyID_RPM, "RPM", true, false);
    /**
     * @brief The Power [-255..255] currently being applied to acheive the Goal
     */
    Property Power = Property(PropertyID_Power, "Powr", true, false);
    Property* pa[4] = { &Goal, &RPM, &Power, nullptr };
    /**
     * @brief Construct a new Motor Base object
     * 
     * @param entity The Entity ID
     * @param name The Entity Name
     */
    MotorBase(EntityID entity, const char* name) : Entity(entity, name, pa) {}
};

#endif // _MOTORBASE_H
