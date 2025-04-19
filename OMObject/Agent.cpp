#include "Agent.h"
#include "FLogger.h"

void Agent::Run()
{
    if (!inputCommands.empty())
    {
        // process an input command
        // NOTE: Disabling interrupts during the critical section removing
        // an input command from the queue
        // We just do one cmd at a time here to let the receive interrupt do its thing
        noInterrupts();
        auto cmd = inputCommands.front();
        inputCommands.pop();
        interrupts();
        flogv("Input command: [%s]", cmd.c_str());
        pRoot->Command(cmd);
        // prioritize input commands over output commands
        // doing another in the next iteration of the Loop()
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
