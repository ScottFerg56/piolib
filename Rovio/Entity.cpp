#include "Entity.h"
#include "FLogger.h"

Property* Entity::GetProperty(PropertyID property)
{
    // scan all Properties
    for (Property** p = Properties; (*p) != nullptr; p++)
    {
        // find the matching ID
        if ((*p)->GetID() == property)
            return *p;
    }
    // no match is an error! not fatal here, but probably will be?!
    floge("property not found: %i", property);
    return Properties[0];
}
