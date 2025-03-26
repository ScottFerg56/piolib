#include "OMObject.h"
#include <Preferences.h>

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

void OMObject::TraverseNodes(EnumPropFn fn)
{
    fn(this);
    for (auto p : Properties)
        fn(p);
    for (auto o : Objects)
        o->TraverseNodes(fn);
}

void Root::Setup()
{
    for (auto o : Objects)
        o->Setup();
}

void Root::Run()
{
    for (auto o : Objects)
        o->Run();
}

void DumpNode(OMNode* node)
{
    if (node->IsObject())
    {
        auto o = (OMObject*)node;
        flogi("object path: %s  name: %s", o->GetPath(), o->GetName());
    }
    else
    {
        auto p = (OMProperty*)node;
        String v;
        p->ToString(v);
        flogi("property path: %s  name: %s  value: %s", p->GetPath(), p->GetName(), v.c_str());
    }
}

const char* OMPrefNamespace = "rcvr OM";

void DumpNodePrefs(OMNode* node)
{
    if (node->IsObject())
    {
        auto o = (OMObject*)node;
        flogi("object path: %s  name: %s", o->GetPath(), o->GetName());
    }
    else
    {
        Preferences prefs;
        auto p = (OMProperty*)node;
        auto path = p->GetPath();
        prefs.begin(OMPrefNamespace, false);
        String v = prefs.isKey(path.c_str()) ? prefs.getString(path.c_str()) : "";
        flogi("property path: %s  name: %s  pref: [%s]", path, p->GetName(), v);
        prefs.end();
    }
}

void SaveNodePrefs(OMNode* node)
{
    if (node->IsObject())
    {
        auto o = (OMObject*)node;
        flogi("object path: %s  name: %s", o->GetPath(), o->GetName());
    }
    else
    {
        Preferences prefs;
        auto p = (OMProperty*)node;
        auto path = p->GetPath();
        String v;
        p->ToString(v);
        prefs.begin(OMPrefNamespace, false);
        flogi("property path: %s  name: %s  pref: [%s]", path, p->GetName(), v);
        auto ret = prefs.putString(path.c_str(), v);
        if (ret == 0)
            floge("preferences write error property path: %s  name: %s  pref: [%s]", path, p->GetName(), v);
        prefs.end();
    }
}

void LoadNodePrefs(OMNode* node)
{
    if (node->IsObject())
    {
        auto o = (OMObject*)node;
        flogv("object path: %s  name: %s", o->GetPath(), o->GetName());
    }
    else
    {
        Preferences prefs;
        auto p = (OMProperty*)node;
        auto path = p->GetPath();
        prefs.begin(OMPrefNamespace, false);
        String v = prefs.isKey(path.c_str()) ? prefs.getString(path.c_str()) : "";
        if (v.length() > 0)
        {
            flogv("property path: %s  name: %s  pref: [%s]", path, p->GetName(), v);
            p->FromString(v);
        }
        prefs.end();
    }
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
        break;
    case '*':
        if (node->IsObject())
            ((OMObject*)node)->TraverseNodes(DumpNode);
        else
            DumpNode((OMProperty*)node);
        break;
    case '>':
        if (node->IsObject())
            ((OMObject*)node)->TraverseNodes(SaveNodePrefs);
        else
            SaveNodePrefs((OMProperty*)node);
        break;
    case '<':
        if (node->IsObject())
            ((OMObject*)node)->TraverseNodes(LoadNodePrefs);
        else
            LoadNodePrefs((OMProperty*)node);
        break;
    case '!':
        if (node->IsObject())
            ((OMObject*)node)->TraverseNodes(DumpNodePrefs);
        else
            DumpNodePrefs((OMProperty*)node);
        break;
    default:
        floge("invalid packet operation: [%c]", operation);
        break;
    }
}
