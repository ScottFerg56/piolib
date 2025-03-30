#include "OMObject.h"
#include <Preferences.h>

const char* OMPrefNamespace = "rcvr OM";

void OMProperty::SavePref()
{
    Preferences prefs;
    auto path = GetPath();
    String v;
    ToString(v);
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
    String v;
    ToString(v);
    flogi("property path: %s  name: %s  value: %s", GetPath(), GetName(), v.c_str());
}

void OMProperty::Fetch()
{
    if (!Changed)
        return;
        Changed = false;
    String cmd;
    cmd.concat('=');
    cmd.concat(GetPath());
    ToString(cmd);
    ((Root*)MyRoot())->AddPacket(cmd);
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

void Root::Setup()
{
    for (auto o : Objects)
        o->Setup();
    TraverseProperties([](OMProperty* p) { p->Changed = false; });
}

void Root::Run()
{
    for (auto o : Objects)
        o->Run();

    TraverseProperties([](OMProperty* p) { p->Fetch(); });
    if (Packet.length() > 0)
    {
        SendPacket(Packet);
        Packet.clear();
    }
}

void Root::AddPacket(String cmd)
{
    if (Packet.length() + cmd.length() > 200)
    {
        SendPacket(Packet);
        Packet.clear();
    }
    Packet.concat(cmd);
    Packet.concat(';');
}

void Root::Command(String cmd)
{
    flogv("command packet: [%s]", cmd);

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
            flogv("assign %s to %s", v, p->GetName());
            // UNDONE: no need for call to mod string??
            p->FromString(v);
        }
        break;
    case '?':
        if (node->IsObject())
            ((OMObject*)node)->TraverseProperties([](OMProperty* p) { p->Changed = true; });
        else
            ((OMProperty*)node)->Changed = true;
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
