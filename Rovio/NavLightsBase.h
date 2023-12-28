#ifndef _NAVLIGHTSBASE_H
#define _NAVLIGHTSBASE_H

#include "Entity.h"

enum Animations
{
    Animation_None,
    Animation_Green,
    Animation_Cylon,
    Animation_Fwd,
};

/**
 * @brief Class to sync Rovio NavLights actions across ESP_NOW interface between remote Domains
 */
class NavLightsBase : public Entity
{
public:
    /**
     * @brief The Animation to running on the NavLights [Animations]
     */
    Property Animation = Property(PropertyID_Animation, "Animation", true, true);
    /**
     * @brief The number of iterations animations should run
     */
    Property Iterations = Property(PropertyID_Iterations, "Iterations", false, true);
    /**
     * @brief The NavLights maximum brightness
     */
    Property Brightness = Property(PropertyID_Brightness, "Brightness", false, true);
    Property* pa[4] = { &Animation, &Iterations, &Brightness, nullptr };

    /**
     * @brief Construct a new NavLights Base object
     * 
     * @param entity The Entity ID
     * @param name The Entity Name
     */
    NavLightsBase(EntityID entity, const char* name) : Entity(entity, name, pa) {}
};

#endif // _NAVLIGHTSBASE_H
