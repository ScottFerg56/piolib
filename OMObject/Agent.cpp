#include "Agent.h"
#include "FLogger.h"

void Agent::Run()
{
    if (!inputCommands.empty() && !lockInput)
    {
        // process input commnads
        auto cmd = inputCommands.front();
        inputCommands.pop();
        flogv("Input command: [%s]", cmd.c_str());
        pRoot->Command(cmd);
        // prioritize input commands over output commands
        return;
    }

    if (!outputCommands.empty())
    {
        // process output commands to peer
        uint8_t data[250];
        uint8_t len = 0;
        while (!outputCommands.empty())
        {
            auto cmd = outputCommands.front();
            auto cmdLen = cmd.length();
            if (len + cmdLen > sizeof(data))
                break;
            outputCommands.pop();
            if (len > 0)
                data[len++] = ';';
            strcpy((char*)&data[len], cmd.c_str());
            len += cmdLen;
        }
        flogv("Send commands: [%s]", data);
        Send(data, len);
    }
}
