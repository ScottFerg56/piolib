#pragma once

#include <Arduino.h>
#include <vector>
#include "FLogger.h"

class OMNode
{
public:
    OMNode(char id, const char* name) : Id(id), Name(name) {}
    OMNode*             Parent;
    virtual bool        IsObject() = 0;
    char                GetID() { return Id; }
    const char*         GetName() { return Name; }
    String              GetPath()
    {
        if (Parent)
            return Parent->GetPath() + GetID();
        return "";
    }
    virtual void        Dump() = 0;
    OMNode*             MyRoot() { return Parent ? Parent->MyRoot() : this; }
    char                Id;
    const char*         Name;
};

class OMObject;
class OMProperty;

enum OMT
{
    OMT_LONG,
    OMT_BOOL,
    OMT_CHAR,
    OMT_STRING,
};

class OMConnector
{
public:
    virtual void Init(OMObject* obj) = 0;
    virtual void Push(OMObject* obj, OMProperty* prop) = 0;
    virtual void Pull(OMObject* obj, OMProperty* prop) = 0;
};

struct OMPropDef
{
    char        Id;
    const char* Name;
    OMT         Type;
    long        Min;
    long        Max;
    long        Base;
    const char* Valid;  // valid chars for OMT_CHAR
};

struct OMObjDef
{
    char        Id;
    const char* Name;
    OMObjDef*   Objects;
    OMPropDef*  Properties;
    OMConnector* Connector;
};

class OMProperty : public OMNode
{
public:
    OMProperty(char id, const char* name) : OMNode(id, name) {}
        
    bool                IsObject() override { return false; }
    void                Dump() override;
    virtual OMT         GetType() = 0;
    virtual String      ToString() = 0;
    virtual void        FromString(String s) = 0;
    void                Pull();
    void                Push();
    void                Send();
    void                SavePref();
    void                LoadPref();
    void                DumpPref();
};

template <typename T> class OMPropertyType : public OMProperty
{
public:
    OMPropertyType(char id, const char* name) : OMProperty(id, name), Value(T()) {}
    T Value;
        
    void                SetSend(T value) { Value = value; Send(); }

    T Get() { return Value; }
    virtual void Set(T value)
    {
        if (!Test(value))
        {
            floge("invalid value");
            return;
        }
        if (Value == value)
            return;
        Value = value;
        Push();
    }
    virtual bool Test(T value) = 0;
};

class OMObject : public OMNode
{
public:
    OMObject(char id, const char* name, OMConnector* connector) : OMNode(id, name), Connector(connector) {}
    bool                IsObject() override { return true; }
    OMProperty*         GetProperty(char propertyID);
    OMObject*           GetObject(char objectID);
    OMObject*           ObjectFromPath(String path);
    OMProperty*         PropertyFromPath(String path, char propertyID);
    using EnumNodeFn = void (*)(OMNode* p);
    void                TraverseNodes(EnumNodeFn fn);
    using EnumPropFn = void (*)(OMProperty* p);
    void                TraverseProperties(EnumPropFn fn);
    using EnumObjFn = void (*)(OMObject* p);
    void                TraverseObjects(EnumObjFn fn);
    void                Dump();
    void                AddObjects(OMObjDef* def);
    void                AddObject(OMObjDef* def);
    void                AddProperties(OMPropDef* def);
    void                AddProperty(OMPropDef* def);
    void                AddObject(OMObject* o);
    void                AddProperty(OMProperty* p);

    OMConnector*        Connector = nullptr;
    void*               Data = nullptr;
    std::vector<OMProperty*> Properties;
    std::vector<OMObject*> Objects;
protected:
    OMNode*             NodeFromPath(String path, int& inx);
};

class OMPropertyLong : public OMPropertyType<long>
{
public:
    OMPropertyLong(char id, const char* name, long min, long max, uint8_t base) : OMPropertyType<long>(id, name), Min(min), Max(max), Base(base == 0 ? 10 : 16) {}

    OMT GetType() override { return OMT_LONG; }

    bool Test(long value) override
    {
        return value >= Min && value <= Max;
    }

    String ToString() override
    {
        return String(Get(), Base);
    }

    void FromString(String s) override
    {
        char *pend;
        Set(strtol(s.c_str(), &pend, Base));
    }
private:
    long Min;
    long Max;
    uint8_t Base;
};

class OMPropertyBool : public OMPropertyType<bool>  // Updated to inherit from OMPropertyType<bool>
{
public:
    OMPropertyBool(char id, const char* name) : OMPropertyType<bool>(id, name) {}

    OMT GetType() override { return OMT_BOOL; }

    String ToString() override
    {
        return String(Value ? '1' : '0');
    }

    void FromString(String s) override
    {
        char c = s[0];
        switch (c)
        {
        case '0':
        case 'f':
            Set(false);
            break;
        case '1':
        case 't':
            Set(true);
            break;
            
        default:
            floge("invalid boolean value: [%c]", c);
            break;
        }
    }

    bool Test(bool value) override
    {
        return true; // always valid
    }
};

class OMPropertyChar : public OMPropertyType<char>
{
public:
    OMPropertyChar(char id, const char* name, const char* valid) : OMPropertyType<char>(id, name), Valid(valid) {}

    OMT GetType() override { return OMT_CHAR; }

    String ToString() override
    {
        return String(Value);
    }

    void FromString(String s) override
    {
        if (s.length() == 0)
        {
            floge("invalid char value");
            return;
        }
        Set(s[0]);
    }
    
    int Index()
    {
        auto p = strchr(Valid, Value);
        if (!p)
            return -1;
        return p - Valid;
    }

    char FromIndex(int inx)
    {
        if (inx < 0 || inx >= strlen(Valid))
            return 0;
        return Valid[inx];
    }

    bool Test(char value) override
    {
        auto p = strchr(Valid, value);
        if (!p)
        {
            floge("invalid char value: [%c]", value);
            return false;
        }
        return true;
    }
protected:
    const char* Valid;
};

class Agent;

class Root : public OMObject
{
public:
	Root(bool isDevice, char id, const char* name, OMConnector* connector = nullptr) : OMObject(id, name, connector), IsDevice(isDevice) { }
	virtual void	Setup(Agent* pagent);
	virtual void	Run();
    virtual void    Command(String cmd);    // UNDONE: virtual temporary?
    void            SendCmd(String cmd);
    virtual void    ReceivedFile(String fileName) {}
private:
    Agent*          pAgent;
    bool            IsDevice = false;
};
