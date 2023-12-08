#ifndef _DOMAIN_H
#define _DOMAIN_H

#include <Arduino.h>
#include <esp_now.h>
#include "Entity.h"
#include "Packet.h"

/**
  * @brief     Callback function for processing Property changes
  * @param     pe pointer to the Entity
  * @param     pp pointer to the changed Property
  */
typedef void (*propChange_cb)(Entity* pe, Property* pp);

/**
 * @brief   Class to sync Entity Properties across ESP_NOW interface
*/
class Domain
{
public:
    /**
     * @brief Construct a new Domain object
     * 
     * @param server ProcessChanges will transmit properties flagged as Output for Server == true,
     *      and flagged as Input for Server == false
     * @param entities An array of Entity pointers for the Domain
     */
    Domain(bool server, Entity** entities) { Entities = entities; Server = server; }

    /**
     * @brief Initialize the ESP_NOW peer communications for the Domain
     * 
     * @param addr The MAC address of the peer to communicate with
     */
    void Init(uint8_t addr[]);

    /**
     * @brief Get the Entity for the specified ID
     * 
     * @param entity The ID of the Entity to return
     * @return Entity* A pointer to the Entity with the specified ID
     * @remarks Not finding the ID is reported as an error and the first Entity in the list is returned
     */
    Entity* GetEntity(EntityID entity);

    /**
     * @brief Process any existing changes to Properties in the Domain
     * 
     * @param cb A callback notification so the caller may further process Properties identified as changed
     */
    void ProcessChanges(propChange_cb cb);

    /**
     * @brief Get the Property for the specified EntityID and PropertyID
     * 
     * @param entity The ID of the Entity
     * @param property The ID of the Property
     * @return Property* A pointer to the Property
     * @remarks Not finding the Entity or Property is reported as an error and the first Entity/Property in the list is accessed
     */
    inline Property* GetEntityProperty(EntityID entity, PropertyID property)
    {
        return GetEntity(entity)->GetProperty(property);
    }

    /**
     * @brief Set the Value for the specified EntityID and PropertyID
     * 
     * @param entity The ID of the Entity
     * @param property The ID of the Property
     * @param value The Value to be set for the Property
     * @remarks Not finding the Entity or Property is reported as an error and the first Entity/Property in the list is accessed
     */
    inline void SetEntityPropertyValue(EntityID entity, PropertyID property, int16_t value)
    {
        GetEntityProperty(entity, property)->Set(value);
    }

    /**
     * @brief Get the Value for the specified EntityID and PropertyID
     * 
     * @param entity The ID of the Entity
     * @param property The ID of the Property
     * @param value The Value to get for the Property
     * @remarks Not finding the Entity or Property is reported as an error and the first Entity/Property in the list is accessed
     */
    inline int16_t GetEntityPropertyValue(EntityID entity, PropertyID property)
    {
        return GetEntityProperty(entity, property)->Get();
    }

protected:
    /**
     * @brief Array of pointers to Entities for the Domain
     */
    Entity** Entities;
    bool Server;
    esp_now_peer_info_t PeerInfo;
};

#endif // _DOMAIN_H
