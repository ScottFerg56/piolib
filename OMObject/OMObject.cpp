#include "OMObject.h"
#include "Agent.h"
#include <Preferences.h>

const char* OMPrefNamespace = "rcvr OM";

void OMProperty::SavePref()
{
    Preferences prefs;
    auto path = GetPath();
    String v = ToString();
    prefs.begin(OMPrefNamespace, false);
    flogi("property path: %s  name: %s  pref: [%s]", path, GetName(), v);
    auto ret = prefs.putString(path.c_str(), v);
    if (ret == 0)
        floge("preferences write error property path: %s  name: %s  pref: [%s]", path, GetName(), v);
    prefs.end();
}

void OMProperty::LoadPref()
{
    Preferences prefs;
    auto path = GetPath();
    prefs.begin(OMPrefNamespace, false);
    String v = prefs.isKey(path.c_str()) ? prefs.getString(path.c_str()) : "";
    if (v.length() > 0)
    {
        flogv("property path: %s  name: %s  pref: [%s]", path, GetName(), v);
        FromString(v);
    }
    prefs.end();
}

void OMProperty::DumpPref()
{
    Preferences prefs;
    auto path = GetPath();
    prefs.begin(OMPrefNamespace, false);
    if (prefs.isKey(path.c_str()))
    {
        String v = prefs.getString(path.c_str());
        flogi("property path: %s  name: %s  pref: [%s]", path, GetName(), v);
    }
    prefs.end();
}

void OMProperty::Dump()
{
    String v = ToString();
    flogi("property path: %s  name: %s  value: %s", GetPath(), GetName(), v.c_str());
}

void OMProperty::Send()
{
    ((Root*)MyRoot())->SendCmd(String('=') + GetPath() + ToString());
}

void OMProperty::Pull()
{
    auto obj = (OMObject*)Parent;
    if (obj->Connector)
        obj->Connector->Pull(obj, this);
}

void OMProperty::Push()
{
    auto obj = (OMObject*)Parent;
    if (obj->Connector)
        obj->Connector->Push(obj, this);
}

OMNode* OMObject::NodeFromPath(String path, int& inx)
{
    // flogd("path: %s  inx: %d", path.c_str(), inx);
    if (inx >= path.length())
        return nullptr;
    auto id = path[inx];
    auto sub = GetObject(id);
    if (sub)
    {
        auto n = sub->NodeFromPath(path, ++inx);
        if (n)
            return n;
        return sub;
    }
    auto p = GetProperty(id);
    if (p)
        ++inx;
    return p;
}

OMObject* OMObject::ObjectFromPath(String path)
{
    int inx = 0;
    return (OMObject*)NodeFromPath(path, inx);
}

OMProperty* OMObject::PropertyFromPath(String path, char propertyID)
{
    auto obj = ObjectFromPath(path);
    if (!obj)
    {        
        floge("object not found for path: %s", path.c_str());
        return nullptr;
    }
    auto prop = obj->GetProperty(propertyID);
    if (!prop)
    {
        floge("property Id %c not found for object: %s", propertyID, obj->GetName());
        return nullptr;
    }
    return prop;
}

OMProperty* OMObject::GetProperty(char propertyID)
{
    for (auto p : Properties)
    {
        if (p->GetID() == propertyID)
            return p;
    }
    return nullptr;
}

OMObject* OMObject::GetObject(char objectID)
{
    for (auto o : Objects)
    {
        if (o->GetID() == objectID)
            return o;
    }
    return nullptr;
}

void OMObject::AddProperty(OMProperty* p)
{
    // flogv("adding property %s  type: %d", p->GetName(), p->GetType());
    Properties.push_back(p);
    p->Parent = this;
    if (Connector)
        Connector->Pull(this, p);
}

void OMObject::AddProperties(const OMPropDef* def)
{
    while (def && def->Id)
        AddProperty(def++);
}

void OMObject::AddProperty(const OMPropDef* def)
{
    OMProperty* prop;
    switch (def->Type)
    {
    case OMT_BOOL:
        prop = new OMPropertyBool(def->Id, def->Name);
        break;
    case OMT_LONG:
        prop = new OMPropertyLong(def->Id, def->Name, def->Min, def->Max, def->Base);
        break;
    case OMT_CHAR:
        prop = new OMPropertyChar(def->Id, def->Name, def->Valid);
        break;
    case OMT_STRING:
        prop = new OMPropertyString(def->Id, def->Name);
        break;
    }
    AddProperty(prop);
}

void OMObject::AddObject(OMObject* o)
{
    Objects.push_back(o);
    o->Parent = this;
    // flogv("adding object %s : %s", o->Parent->GetName(), o->GetName());
    if (o->Connector)
        o->Connector->Init(o);
}

void OMObject::AddObjects(const OMObjDef* def)
{
    while (def && def->Id)
        AddObject(def++);
}

void OMObject::AddObject(const OMObjDef* def)
{
    auto obj = new OMObject(def->Id, def->Name, def->Connector);
    AddObject(obj);
    obj->AddProperties(def->Properties);
    obj->AddObjects(def->Objects);
}

void OMObject::TraverseNodes(EnumNodeFn fn)
{
    fn(this);
    for (auto p : Properties)
        fn(p);
    for (auto o : Objects)
        o->TraverseNodes(fn);
}

void OMObject::TraverseProperties(EnumPropFn fn)
{
    for (auto p : Properties)
        fn(p);
    for (auto o : Objects)
        o->TraverseProperties(fn);
}

void OMObject::TraverseObjects(EnumObjFn fn)
{
    fn(this);
    for (auto o : Objects)
        o->TraverseObjects(fn);
}

void OMObject::Dump()
{
    flogi("object path: %s  name: %s", GetPath(), GetName());
}

void Root::Setup(Agent* pagent)
{
    pAgent = pagent;
    if (IsDevice)
    {
        // traverse all properties to pull initial values
        TraverseProperties([](OMProperty *p) {
            p->Pull();
        });
    }
    else
    {
        // request initial values from device
        SendCmd("?R");
    }
}

void Root::Run()
{
}

void Root::Command(String cmd)
{
    int inx = 0;
    if (inx >= cmd.length())
    {
        floge("null command");
        return;
    }
    char operation = cmd[inx++];
    bool rooted = false;
    if (inx < cmd.length() && cmd[inx] == GetID())
    {
        rooted = true;
        inx++;
    }
    auto node = NodeFromPath(cmd, inx);
    // flogv("node: %s", node->GetName());
    if (!node)
    {
        if (!rooted)
        {
            floge("node not found");
            return;
        }
        node = this;
    }
    switch (operation)
    {
    case '=':
        {
            if (node->IsObject())
            {
                floge("assignment not valid for object: %s", node->GetName());
                return;
            }
            auto p = (OMProperty*)node;
            auto v = cmd.substring(inx);
            flogv("assign %s to %s : %s", v, p->Parent->GetName(), p->GetName());
            // UNDONE: no need for call to mod string??
            p->FromString(v);
        }
        break;
    case '?':
        if (node->IsObject())
            ((OMObject*)node)->TraverseProperties([](OMProperty* p) { p->Send(); });
        else
            ((OMProperty*)node)->Send();
        break;
    case '*':
        if (node->IsObject())
            ((OMObject*)node)->TraverseNodes([](OMNode* n) { n->Dump(); });
        else
            node->Dump();
        break;
    case '>':
        if (node->IsObject())
            ((OMObject*)node)->TraverseProperties([](OMProperty* p) { p->SavePref(); });
        else
            ((OMProperty*)node)->SavePref();
        break;
    case '<':
        if (node->IsObject())
            ((OMObject*)node)->TraverseProperties([](OMProperty* p) { p->LoadPref(); });
        else
            ((OMProperty*)node)->LoadPref();
        break;
    case '!':
        if (node->IsObject())
            ((OMObject*)node)->TraverseProperties([](OMProperty* p) { p->DumpPref(); });
        else
            ((OMProperty*)node)->DumpPref();
        break;
    default:
        floge("invalid packet operation: [%c]", operation);
        break;
    }
}

void Root::SendCmd(String cmd) { pAgent->SendCmd(cmd); }
