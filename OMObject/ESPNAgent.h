#pragma once

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <queue>
#include "FS.h"
#include "Agent.h"

class ESPNAgent : public Agent
{
public:
    ESPNAgent(FS* pfs, Root* proot) : Agent(pfs, proot) { };
    void    Setup(uint8_t peerMacAddress[]);
    void    Run() override;
    bool    Send(const uint8_t *pData, int len) override;
    void    StartFileTransfer(String filePath) override;
    void    OnDataSent(esp_now_send_status_t status);
    void    OnDataRecv(const uint8_t *pData, int len);

    static ESPNAgent* FindAgent(const uint8_t* peerMacAddress);
    static ESPNAgent* PrimaryAgent() { return ESPNAgents[0]; }
private:
    esp_now_peer_info_t PeerInfo;
    static std::vector<ESPNAgent*> ESPNAgents;
    bool DataSent = false;
    bool DataConnected = false;
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
};
