#pragma once

const OMPropDef   LightProps[] =
{
    { 'a', "Anim",    OMT_LONG, OMF_NONE,    0, 100 },   // UNDONE: backfill this?
    { 'o', "On",      OMT_BOOL, OMF_NONE, },
    { 'c', "Color1",  OMT_LONG, OMF_NONE,    0, 0xFFFFFF, 16 },
    { 'd', "Color2",  OMT_LONG, OMF_NONE,    0, 0xFFFFFF, 16 },
    { 's', "Speed",   OMT_LONG, OMF_NONE,    0, 60000 },
    { 'r', "Reverse", OMT_BOOL, OMF_NONE, },
    { }
};

const OMPropDef   GroupProps[] =
{
    { 'o', "On",      OMT_BOOL, OMF_NONE },
    { }
};

const OMObjDef    TubesObjs[] =
{
    { 's', "Sconce",    nullptr,    LightProps, &LightConn },
    { 'f', "Floor",     nullptr,    LightProps, &LightConn },
    { }
};

const OMObjDef    HoldObjs[] =
{
    { 'y', "Bay",       nullptr,    LightProps, &LightConn },
    { 'b', "Bed",       nullptr,    LightProps, &LightConn },
    { 'g', "Grates",    nullptr,    LightProps, &LightConn },
    { 'm', "Monitor",   nullptr,    LightProps, &LightConn },
    { '0', "Red",       nullptr,    LightProps, &LightConn },
    { '1', "Green",     nullptr,    LightProps, &LightConn },
    { '2', "Blue",      nullptr,    LightProps, &LightConn },
    { '3', "Yellow",    nullptr,    LightProps, &LightConn },
    { }
};

const OMObjDef    CockpitObjs[] =
{
    { 'm', "Monitor",   nullptr,    LightProps, &LightConn },
    { '0', "Red",       nullptr,    LightProps, &LightConn },
    { '1', "Green",     nullptr,    LightProps, &LightConn },
    { '2', "Blue",      nullptr,    LightProps, &LightConn },
    { '3', "Yellow",    nullptr,    LightProps, &LightConn },
    { '4', "Wall UL",   nullptr,    LightProps, &LightConn },
    { '5', "Wall UR",   nullptr,    LightProps, &LightConn },
    { '6', "Wall LL",   nullptr,    LightProps, &LightConn },
    { '7', "Wall LR",   nullptr,    LightProps, &LightConn },
    { }
};

const OMObjDef    LightObjs[] =
{
    { 'e', "Engine",    nullptr,    LightProps, &LightConn },
    { 'l', "Landing",   nullptr,    LightProps, &LightConn },
    { 'w', "Warning",   nullptr,    LightProps, &LightConn },
    { 'd', "Headlight", nullptr,    LightProps, &LightConn },
    { 'g', "Gunwell",   nullptr,    LightProps, &LightConn },
    { 'r', "Ramp",      nullptr,    LightProps, &LightConn },
    { 't', "Tubes",     TubesObjs,  GroupProps, &GroupConn },
    { 'h', "Hold",      HoldObjs,   GroupProps, &GroupConn },
    { 'c', "Cockpit",   CockpitObjs,GroupProps, &GroupConn },
    { 'z', "Test Neo",  nullptr,    LightProps, &LightConn },
    { }
};

const OMPropDef   RectProps[] =
{
    { 's', "Sweep",    OMT_BOOL, OMF_NONE },
    { 'v', "Speed",    OMT_LONG, OMF_NONE, 0, 100 },
    { 'p', "Position", OMT_LONG, OMF_NONE, 0, 100 },
    { }
};

const OMPropDef   RampProps[] =
{
    { 's', "State",    OMT_CHAR, OMF_NONE, 0, 0, 0, "RrSeE" },
    { 'v', "Speed",    OMT_LONG, OMF_NONE, 0, 100 },
    { }
};

const OMPropDef   SoundProps[] =
{
    { 'p', "Play",    OMT_LONG,   OMF_WO_DEVICE, 1, 100 },
    { 'v', "Volume",  OMT_LONG,   OMF_NONE,      0,  21 },
    { 'x', "Delete",  OMT_LONG,   OMF_WO_DEVICE, 1, 100 },
    { 'l', "List",    OMT_STRING, OMF_RO_DEVICE },     // list must be last to set Max for Play and Delete
    { }
};

const OMPropDef   DebugProps[] =
{
    { 'l', "LogLevel", OMT_CHAR, OMF_LOCAL, 0, 0, 0, "NFEWIDV" },
    { }
};

const OMObjDef    Objects[] =
{
    { 'l', "Lights",    LightObjs, nullptr },
    { 'a', "Rectenna",  nullptr,   RectProps,  &RectennaConn },
    { 'r', "Ramp",      nullptr,   RampProps,  &RampConn },
    { 's', "Sound",     nullptr,   SoundProps, &SoundConn },
    { 'd', "Debug",     nullptr,   DebugProps, &DebugConn },
    { }
};

const OMPropDef   RootProps[] =
{
    { 'x', "Restart",   OMT_LONG, OMF_WO_DEVICE, 1234, 1234  },
    { 'f', "FreeSpace", OMT_LONG, OMF_RO_DEVICE, 0, LONG_MAX },
    { }
};
