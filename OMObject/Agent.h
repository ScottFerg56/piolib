#pragma once

#include <Arduino.h>
#include <queue>
#include "FS.h"
#include "OMObject.h"

class Agent
{
public:
    Agent(FS* pfs, Root* proot) : pFS(pfs), pRoot(proot) { }
    virtual void    Run();
    virtual bool    Send(const uint8_t *pData, int len) = 0;
    virtual void    StartFileTransfer(String filePath) = 0;
    void            SendCmd(String cmd) { outputCommands.push(cmd); }
protected:
    FS*     pFS;
    std::queue<String> inputCommands;
    std::queue<String> outputCommands;
    Root* pRoot;
};
