#ifndef _ENTITY_H
#define _ENTITY_H

#include <Arduino.h>
#include "Property.h"

/**
 * @brief IDs for Entities shared between Server (the Robot) and the client (the controller)
 */
enum EntityID : uint8_t
{
    EntityID_None,
    EntityID_LeftMotor,
    EntityID_RightMotor,
    EntityID_RearMotor,
    EntityID_Head,
    EntityID_NavLights,
    EntityID_Last
};

//// prefix ++
//inline EntityID& operator++(EntityID& id)
//{
//    id = (EntityID)((int)id + 1);
//    return id;
//}

/**
 * @brief postfix increment operator for EntityID
 * 
 * @param id The EntityID reference to increment
 * @return EntityID reference before increment
 */
inline EntityID operator++(EntityID& id, int)
{
    EntityID o = id;
    id = (EntityID)((int)id + 1);
    return o;
}

/**
 * @brief Class for an Entity (of a Domain) to sync across ESP_NOW interface between remote Domains
 */
class Entity
{
public:
    /**
     * @brief Array of pointers to Properties for the Entity
     */
    Property** Properties;

    /**
     * @brief Get the Entity's ID
     * 
     * @return EntityID 
     */
    inline EntityID GetID() { return ID; }

    /**
     * @brief Get the Property for the specified ID
     * 
     * @param property The ID of the Property to return
     * @return Property* A pointer to the Property with the specified ID
     * @remarks Not finding the ID returns nullptr
     */
    Property* GetProperty(PropertyID property);

    /**
     * @brief Construct a new Entity object
     * 
     * @param id The ID to assign to the Entity
     * @param name The Name to assign to the Entity
     * @param properties An array of Property pointers for the Entity
     */
    Entity(EntityID id, const char* name, Property** properties)
    {
        ID = id;
        Name = name;
        Properties = properties;
    }

    /**
     * @brief Get the Entity's Name
     * 
     * @return const char* The Name
     */
    inline const char* GetName() { return Name; }

protected:
    EntityID ID;
    const char* Name;
};

#endif // _ENTITY_H
