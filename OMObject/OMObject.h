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
    virtual void        ToString(String& s) = 0;
    virtual bool        FromString(String& s) = 0;
    void                Pull(bool change = false);
    void                SavePref();
    void                LoadPref();
    void                DumpPref();
    void                Fetch();

    // virtual bool IsOutput() = 0;
    // virtual bool IsInput() = 0;

    inline bool         CheckResetChanged()
    {
        bool changed = Changed;
        Changed = false;
        return changed;
    }

    bool                Changed = false;
};

class OMObject : public OMNode
{
public:
    OMObject(char id, const char* name, OMConnector* connector) : OMNode(id, name), Connector(connector) {}
    bool                IsObject() override { return true; }
    OMProperty*         GetProperty(char propertyID);
    OMObject*           GetObject(char objectID);
    OMNode*             NodeFromPath(String path, int& inx);
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
};

class OMPropertyLong : public OMProperty
{
public:
    OMPropertyLong(char id, const char* name, long min, long max, uint8_t base) : OMProperty(id, name), Min(min), Max(max), Base(base == 0 ? 10 : 16) { }
    long Value;

    OMT GetType() override { return OMT_LONG; }

    virtual long GetMin() { return LONG_MIN; }
    virtual long GetMax() { return LONG_MAX; }
    bool TestRange(long value)
    {
        if (value < GetMin() || value > GetMax())
        {
            floge("long value out of range: [%d]", value);
            return false;
        }
        return true;
    }

    void ToString(String& s) override
    {
        s.concat(String(Get(), GetBase()));
    }

    bool FromString(String& s) override
    {
        const char* p = s.c_str();
        char *pend;
        long val = strtol(p, &pend, GetBase());
        if (pend == p || val == LONG_MAX || val == LONG_MIN)
        {
            floge("invalid long value: [%s]", p);
            return false;
        }
        s.remove(0, pend - p);
        Set(val);
        return true;
    }

    virtual uint8_t GetBase() { return Base; }

    virtual void Set(long value);

    virtual long Get() { return Value; }
    long Min = LONG_MIN;
    long Max = LONG_MAX;
    uint8_t Base = 10;
};

class OMPropertyBool : public OMProperty
{
public:
    OMPropertyBool(char id, const char* name) : OMProperty(id, name) { }
    bool Value;

    OMT GetType() override { return OMT_BOOL; }

    void ToString(String& s) override
    {
        s.concat(Get() ? '1' : '0');
    }

    bool FromString(String& s) override
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
            return false;
        }
        s.remove(0, 1);
        return true;
    }

    virtual void Set(bool value);

    virtual bool Get() { return Value; }
};

class OMPropertyChar : public OMProperty
{
public:
    OMPropertyChar(char id, const char* name, const char* valid) : OMProperty(id, name), Valid(valid) { }
    char Value;

    OMT GetType() override { return OMT_CHAR; }

    void ToString(String& s) override
    {
        s.concat((char)Value);
    }

    bool FromString(String& s) override;

    int Index()
    {
        auto p = strchr(Valid, Value);
        if (!p)
            return -1;
        return p - Valid;
    }

    virtual void Set(char value);

    virtual char Get() { return Value; }
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
