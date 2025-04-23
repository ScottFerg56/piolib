#include "OMObject.h"
#include "Agent.h"
#include <Preferences.h>

const char* OMPrefNamespace = "OM";

void OMProperty::SavePref()
{
    if ((Flags & (OMF_RO_DEVICE | OMF_WO_DEVICE)) != 0 && ((Root*)MyRoot())->IsDevice)
        return;
    Preferences prefs;
    auto path = GetPath();
    String v = ToString();
    prefs.begin(OMPrefNamespace, false);
    flogi("save pref: %s.%s [%s]", Parent->Name, Name, v);
    auto ret = prefs.putString(path.c_str(), v);
    if (ret == 0)
        floge("preferences write error property path: %s  name: %s  pref: [%s]", path, Name, v);
    prefs.end();
}

void OMProperty::LoadPref()
{
    if ((Flags & (OMF_RO_DEVICE | OMF_WO_DEVICE)) != 0 && ((Root*)MyRoot())->IsDevice)
        return;
    Preferences prefs;
    auto path = GetPath();
    prefs.begin(OMPrefNamespace, false);
    String v = prefs.isKey(path.c_str()) ? prefs.getString(path.c_str()) : "";
    if (v.length() > 0)
    {
        flogv("load pref: %s.%s [%s]", Parent->Name, Name, v);
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
        flogi("dump pref: %s.%s [%s]", Parent->Name, Name, v);
    }
    prefs.end();
}

void OMProperty::RemovePref()
{
    Preferences prefs;
    auto path = GetPath();
    prefs.begin(OMPrefNamespace, false);
    if (prefs.isKey(path.c_str()))
    {
        prefs.remove(path.c_str());
        flogi("remove pref: %s.%s", Parent->Name, Name);
    }
    prefs.end();
}

void OMProperty::Dump()
{
    String v = ToString();
    flogi("property path: %s  name: %s  value: %s", GetPath(), Name, v.c_str());
}

void OMProperty::Send()
{
    if ((Flags & OMF_LOCAL) != 0)
        return;
    if ((Flags & OMF_WO_DEVICE) != 0 && ((Root*)MyRoot())->IsDevice)
        return;
    ((Root*)MyRoot())->SendCmd(String('=') + GetPath() + ToString());
}

void OMProperty::Pull()
{
    if ((Flags & OMF_WO_DEVICE) != 0 && ((Root*)MyRoot())->IsDevice)
        return;
    auto obj = (OMObject*)Parent;
    if (obj->Connector)
        obj->Connector->Pull(obj, this);
}

void OMProperty::Push()
{
    if ((Flags & OMF_RO_DEVICE) != 0 && ((Root*)MyRoot())->IsDevice)
        return;
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
        floge("property Id %c not found for object: %s", propertyID, obj->Name);
        return nullptr;
    }
    return prop;
}

OMProperty* OMObject::GetProperty(char propertyID)
{
    for (auto p : Properties)
    {
        if (p->Id == propertyID)
            return p;
    }
    return nullptr;
}

OMObject* OMObject::GetObject(char objectID)
{
    for (auto o : Objects)
    {
        if (o->Id == objectID)
            return o;
    }
    return nullptr;
}

void OMObject::AddProperty(OMProperty* p)
{
    // flogv("adding property %s  type: %d", p->Name, p->GetType());
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
    prop->Flags = def->Flags;
    AddProperty(prop);
}

void OMObject::AddObject(OMObject* o)
{
    Objects.push_back(o);
    o->Parent = this;
    // flogv("adding object %s : %s", o->Parent->Name, o->Name);
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
    flogi("object path: %s  name: %s", GetPath(), Name);
}

void Root::Setup(Agent* pagent)
{
    pAgent = pagent;
    if (IsDevice)
    {
        // traverse all properties to pull initial values
        // and load preferences
        TraverseProperties([](OMProperty *p) {
            p->Pull();
            p->LoadPref();
        });
    }
    else
    {
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
    if (inx < cmd.length() && cmd[inx] == Id)
    {
        rooted = true;
        inx++;
    }
    auto node = NodeFromPath(cmd, inx);
    // flogv("node: %s", node->Name);
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
                floge("assignment not valid for object: %s", node->Name);
                return;
            }
            auto p = (OMProperty*)node;
            auto v = cmd.substring(inx);
            flogv("assign %s to %s.%s", v.c_str(), p->Parent->Name, p->Name);
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
    case '-':
        if (node->IsObject())
            ((OMObject*)node)->TraverseProperties([](OMProperty* p) { p->RemovePref(); });
        else
            ((OMProperty*)node)->RemovePref();
        break;
    default:
        floge("invalid packet operation: [%c]", operation);
        break;
    }
}

void Root::SendCmd(String cmd) { pAgent->SendCmd(cmd); }

void Root::ConnectionChanged(bool connected)
{
    if (connected)
    {
        if (IsDevice)
        {

        }
        else
        {
            SendCmd("?R");  // request ALL current property values
        }
    }
    else
    {
        if (IsDevice)
        {

        }
        else
        {
            
        }
    }
}
