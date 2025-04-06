#pragma once

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <queue>
#include "FS.h"

class Agent
{
public:
    using   CommandFn = void (*)(String cmd);
    void    Setup(FS* pfs, uint8_t peerMacAddress[], CommandFn cmdFn);
    void    Loop();
    bool    SendData(const uint8_t *pData, int len);
    void    SendCmd(String cmd);
    void    StartFileTransfer(String filePath);
    static  Agent& GetInstance() { return agent; }
    void    OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    void    OnDataRecv(const uint8_t *mac_addr, const uint8_t *pData, int len);
    String  FileReceived;
    // Delete copy constructor and assignment operator to prevent copying singleton
    Agent(const Agent&) = delete;
    Agent& operator=(const Agent&) = delete;
protected:
    FS*     pFS;
    esp_now_peer_info_t PeerInfo;
    bool DataSent = false;
    bool DataConnected = false;
    bool lockInput = false;
    std::queue<String> inputCommands;
    std::queue<String> outputCommands;
    CommandFn CmdFn;

    struct FilePacketHdr
    {
        char        tag;
        uint32_t    packetNum;
    };

    const uint16_t  FilePacketSize = 240;           // amount of file data in packet
    uint32_t        FilePacketCount = 0;            // packets in current file transfer
    uint32_t        FilePacketNumber = 0;           // current packet number
    String          FilePath;                       // path to transfered file
    bool            FilePacketSend = false;         // flag to allow next packet to be sent
    uint8_t         FilePacketSendErrorCount = 0;   // error count for sent packets

    void    SendNextFilePacket();
private:
    // Static member variable to hold the single instance
    static Agent    agent;
    // private constructor for singleton
    Agent() { };
};
