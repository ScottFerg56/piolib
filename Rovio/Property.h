#ifndef _PROPERTY_H
#define _PROPERTY_H

#include <Arduino.h>

enum PropertyID : uint8_t
{
    PropertyID_None,
    PropertyID_Goal,
    PropertyID_RPM,
    PropertyID_Power,
    PropertyID_DirectDrive,
    PropertyID_Position,
    // controller-only properties:
    PropertyID_ControlMode,
    PropertyID_Last
};

//// prefix ++
//inline PropertyID& operator++(PropertyID& k)
//{
//    k = (PropertyID)((int)k + 1);
//    return k;
//}
//
//// postfix ++
//inline PropertyID operator++(PropertyID& k, int)
//{
//    PropertyID o = k;
//    k = (PropertyID)((int)k + 1);
//    return o;
//}

/**
 * @brief Class for an Property (of an Entity) to sync across ESP_NOW interface between remote Domains
 */
class Property
{
public:
    /**
     * @brief Get The Property's ID
     * 
     * @return PropertyID 
     */
    inline PropertyID GetID() { return ID; }

    /**
     * @brief Set a Value for the Property
     * 
     * @param value The Value to set
     */
    inline void Set(int16_t value)
    {
        // NOTE: Changes are accumulated until GetChanged is called,
        // so it's possible that the Value may alternate thru different values
        // settling on the original value and Changed will be reported as set
        // even though the value appears the same!
        Changed |= Value != value;
        Value = value;
    }

    /**
     * @brief Get the Value of a Property
     * 
     * @return int16_t The Property Value
     */
    inline int16_t Get() { return Value; }
    
    /**
     * @brief Get the Changed state of the Property
     * 
     * @return true if a change has occurred that has not yet processed
     * @return false if no change is pending processing
     */
    inline bool IsChanged() { return Changed; }

    /**
     * @brief Get the Changed state of the Property, and clear it
     * 
     * @return true if a change has occurred that has not yet processed
     * @return false if no change is pending processing
     */
    inline bool GetChanged()
    {
        bool changed = Changed;
        Changed = false;
        return changed;
    }

    /**
     * @brief Get the Output state of the Property
     * 
     * @return true if the Server should send the Property (and the client should receive it)
     * @return false if the Server should not send the Property
     */
    inline bool IsOutput() { return Output; }

    /**
     * @brief Get the Input state of the Property
     * 
     * @return true if the client should send the Property (and the Server should receive it)
     * @return false if the client should not send the Property
     */
    inline bool IsInput() { return Input; }

    /**
     * @brief Construct a new Property object
     * 
     * @param id The ID to assign to the Property
     * @param name The Name to assign to the Property
     * @param output true if the Server should send the Property
     * @param input true if the client should send the Property
     */
    Property(PropertyID id, const char* name, bool output, bool input)
    {
        ID = id;
        Name = name;
        Output = output;
        Input = input;
    }

    /**
     * @brief Get the Property's Name
     * 
     * @return const char* The Name
     */
    const char* GetName() { return Name; }

protected:
    PropertyID ID;
    int16_t Value;
    const char* Name;
    bool Changed;
    bool Output;
    bool Input;
};

#endif // _PROPERTY_H
