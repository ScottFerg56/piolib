#pragma once

#include <Arduino.h>
#include <vector>
#include "FLogger.h"

class OMNode
{
public:
    OMNode() {}
    OMNode*             Parent;
    virtual bool        IsObject() = 0;
    virtual char        GetID() = 0;
    virtual const char* GetName() = 0;
    String              GetPath()
    {
        if (Parent)
            return Parent->GetPath() + GetID();
        return "";
    }
    virtual void        Dump() = 0;
    OMNode*             MyRoot() { return Parent ? Parent->MyRoot() : this; }
};

class OMObject;

enum OMT
{
    OMT_LONG,
    OMT_BOOL,
    OMT_CHAR,
    OMT_STRING,
};

class OMProperty : public OMNode
{
public:
    OMProperty() {}
    bool                IsObject() { return false; }
    virtual OMT         GetType() = 0;
    virtual void        ToString(String& s) = 0;
    virtual bool        FromString(String& s) = 0;
    void                SavePref();
    void                LoadPref();
    void                DumpPref();
    void                Dump();
    void                Fetch();

    // virtual bool IsOutput() = 0;
    // virtual bool IsInput() = 0;

    inline bool         CheckResetChanged()
    {
        bool changed = Changed;
        Changed = false;
        return changed;
    }

    bool Changed = false;
};

class OMObject : public OMNode
{
public:
    OMObject() {}
    bool                IsObject() { return true; }
	virtual void	    Setup() = 0;
	virtual void	    Run() = 0;
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
    void                AddObject(OMObject* o)
    {
        Objects.push_back(o);
        o->Parent = this;
    }
    void                AddProperty(OMProperty* p)
    {
        Properties.push_back(p);
        p->Parent = this;
    }
protected:
    std::vector<OMProperty*> Properties;
    std::vector<OMObject*> Objects;
};

class OMPropertyLong : public OMProperty
{
public:
    OMPropertyLong() { }

    OMT GetType() { return OMT_LONG; }

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

    void ToString(String& s)
    {
        s.concat(String(Get(), GetBase()));
    }

    bool FromString(String& s)
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

    virtual uint8_t GetBase() { return 10; }

    virtual void Set(long value) = 0;

    virtual long Get() = 0; // { return Value; }
};

class OMPropertyBool : public OMProperty
{
public:
    OMPropertyBool() { }

    OMT GetType() { return OMT_BOOL; }

    void ToString(String& s)
    {
        s.concat(Get() ? '1' : '0');
    }

    bool FromString(String& s)
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

    virtual void Set(bool value) = 0;

    virtual bool Get() = 0;
};

class Root : public OMObject
{
public:
    using SendFn = void (*)(String cmd);
	Root() { }
	void            SetSend(SendFn send) { Send = send; }
    char            GetID() { return 'R'; }
    const char*     GetName() { return "Root"; }
	virtual void	Setup();
	virtual void	Run();
    void            Command(String cmd);
    void            SendPacket(String cmd) { if (Send) Send(cmd); }
    void            AddPacket(String cmd);
private:
    String          Packet;
    SendFn          Send;
};
