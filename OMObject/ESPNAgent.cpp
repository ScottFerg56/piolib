#include "ESPNAgent.h"
#include "FLogger.h"

void DumpMac(const char* msg, const uint8_t* mac)
{
    flogv("%s: %02X:%02X:%02X:%02X:%02X:%02X", msg, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

std::vector<ESPNAgent*> ESPNAgent::ESPNAgents;

ESPNAgent* ESPNAgent::FindAgent(const uint8_t* peerMacAddress)
{
    for (auto agent : ESPNAgents)
    {
        if (memcmp(agent->PeerInfo.peer_addr, peerMacAddress, 6) == 0)
            return agent;
    }
    DumpMac("Agent not found", peerMacAddress);
    flogf("");
    return nullptr;
}

void DataSentCb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESPNAgent::FindAgent(mac_addr)->OnDataSent(status);
}

void DataRecvCb(const uint8_t *mac_addr, const uint8_t *pData, int len)
{
    ESPNAgent::FindAgent(mac_addr)->OnDataRecv(pData, len);
}

void ESPNAgent::Setup(uint8_t peerMacAddress[])
{
    memset(&PeerInfo, 0, sizeof(PeerInfo));
    memcpy(PeerInfo.peer_addr, peerMacAddress, sizeof(PeerInfo.peer_addr));
    ESPNAgents.push_back(this);
    DumpMac("Register Agent", peerMacAddress);

    // flogi("WIFI init");
    if (!WiFi.mode(WIFI_STA))
        flogf("WIFI init FAILED");

    flogi("MAC addr: %s", WiFi.macAddress().c_str());

    // flogi("ESP_NOW init");
    if (esp_now_init() != ESP_OK)
        flogf("ESP_NOW init FAILED");

    if (esp_now_add_peer(&PeerInfo) != ESP_OK)
        flogf("ESP_NOW peer add FAILED");

    esp_now_register_send_cb(DataSentCb);
    esp_now_register_recv_cb(DataRecvCb);

    // flogi("ESP_NOW init complete");

    if (pRoot->IsDevice)
    {
        SendCmd(".");   // send heartbeat to let controller know we're alive
    }
    else
    {
        // longer timeout on the controller
        // so we're not racing with the device to respond
        Metro.PeriodMS += 1000;
    }
}

void ESPNAgent::Run(void)
{
    // check to send next file transfer packet
    if (FilePacketSend)
        SendNextFilePacket();

    if (ConnectionChange)
    {
        ConnectionChange = false;
        pRoot->ConnectionChanged(Connected);
    }

    if (Metro)
    {
        if (pRoot->IsDevice)
        {
            // flogv("device heartbeat");
            SendCmd(".");
        }
        else
        {
            flogv("device offline");
            SetConnection(false);
        }
    }

    Agent::Run();
}

void ESPNAgent::OnDataSent(esp_now_send_status_t status)
{
    DataSent = false;
    if (status != ESP_NOW_SEND_SUCCESS)
    {
        flogw("Delivery Fail");
        SetConnection(false);

        if (FilePacketCount != 0)
        {
            if (++FilePacketSendErrorCount > 2)
            {
                // retries failed
                FilePacketSend = false;
                floge("File transfer failed");
            }
            else
            {
                // retry
                FilePacketSend = true;
                floge("File transfer retry %d", FilePacketSendErrorCount);
            }
        }
    }
    else
    {
        SetConnection(true);
    }
}

void ESPNAgent::OnDataRecv(const uint8_t *pData, int len)
{
    SetConnection(true);
    if (len > 0)
    {
        String data(pData, len);
        switch (data[0])
        {
        case '.':   // heartbeat from device (actions taken above are all we need)
            break;
        case '1':
            {
                // first file transfer packet
                FilePacketHdr hdr;
                memcpy(&hdr, pData, sizeof(hdr));
                FilePacketNumber = 1;
                FilePacketCount = hdr.packetNum;
                FilePath = String((char*)pData + sizeof(hdr));
                flogv("Starting transfer: %s  #packets: %lu", FilePath.c_str(), FilePacketCount);
                pFS->remove(("/" + FilePath).c_str());
                // respond with ACK
                Send((uint8_t*)"3", 1);
            }
            break;
        case '2':
            {
                // file transfer data packet
                FilePacketHdr hdr;
                memcpy(&hdr, pData, sizeof(hdr));
                uint32_t packetNumber = hdr.packetNum;
                if (packetNumber != FilePacketNumber)
                {
                    floge("packet number %lu doesn't match expected %lu", packetNumber, FilePacketNumber);
                    FilePacketNumber = 0;
                    FilePacketCount = 0;
                    FilePath = "";
                    SendCmd("4");   // terminate transfer
                    break;
                }
                //Serial.println("chunk NUMBER = " + String(currentTransmitCurrentPosition));
                File file = pFS->open(("/" + FilePath).c_str(), FILE_APPEND);
                if (!file)
                {
                    floge("Error opening file for append");
                    FilePacketNumber = 0;
                    FilePacketCount = 0;
                    FilePath = "";
                    SendCmd("4");   // terminate transfer
                    break;
                }
                file.write(pData + sizeof(hdr), len - sizeof(hdr));
                uint32_t fileSize = file.size();
                file.close();
                // flogv("File packet %lu of %lu received, file size: %lu", FilePacketNumber, FilePacketCount, fileSize);
        
                if (FilePacketNumber == FilePacketCount)
                {
                    FilePacketNumber = 0;
                    FilePacketCount = 0;
                    flogv("File transfer complete");
                    pRoot->ReceivedFile(FilePath);
                    FilePath = "";
                }
                else
                {
                    ++FilePacketNumber;
                }
                // ACK the packet
                Send((uint8_t*)"3", 1);
            }
            break;
        case '3':
            {
                // ACK received from previous file transfer packet
                // OK the transmission of next packet
                // flogv("File transfer ACK received");
                ++FilePacketNumber;
                FilePacketSend = true;
            }
            break;
        case '4':
            {
                // termination received
                FilePacketCount = 0;
                FilePacketNumber = 0;
                FilePacketSend = false;
                FilePath = "";
                floge("File transfer terminated");
            }
            break;
        case '[':
            // flog* output from peer just for remote diagnosis
            Serial.print(data.c_str());
            break;
        default:
            {
                // assume anyting else is an input command
                // queue it up
                while (data.length() > 0)
                {
                    String cmd;
                    auto inx = data.indexOf(';');
                    if (inx == -1)
                    {
                        cmd = data;
                        data.clear();
                    }
                    else
                    {
                        cmd = data.substring(0, inx);
                        data.remove(0, inx+1);
                    }
                    inputCommands.push(cmd);
                }
            }
            break;
        }
    }
}

bool ESPNAgent::Send(const uint8_t *pData, int len)
{
    if (DataSent)
    {
        // data sent but not acknowledged; wait it out a while
        unsigned long ms = millis();
        while (DataSent)
        {
            if (millis() - ms > 100)
                DataSent = false;
        }
    }
    DataSent = true;
    esp_err_t result = esp_now_send(PeerInfo.peer_addr, pData, len);
    if (result != ESP_OK)
    {
        SetConnection(false);
        floge("Error sending data: %s", esp_err_to_name(result));
        if (result == ESP_ERR_ESPNOW_ARG)
        {
            floge("len: %d", len);
            if (len > 0)
                floge("data: %s", pData);
        }
        DataSent = false;
        return false;
    }
    return true;
}

void ESPNAgent::SetConnection(bool connect)
{
    if (connect)
        Metro.Reset();
    if (connect == Connected)
        return;
    ConnectionChange = true;
    Connected = connect;
}

void ESPNAgent::StartFileTransfer(String filePath)
{
    if (filePath.length() > 31)
    {
        floge("Filename length must be < 32");
        return;
    }
    File file = pFS->open(filePath.c_str(), FILE_READ);
    if (!file)
    {
        floge("File open failed");
        return;
    }
    uint32_t fileSize = file.size();
    file.close();
    FilePacketSendErrorCount = 0;
    // packet 0 is the start packet with filename but no data
    FilePacketNumber = 0;
    FilePacketCount = fileSize / FilePacketSize;
    if (fileSize % FilePacketSize != 0)
    FilePacketCount++;
    FilePath = filePath;
    String fileName(pathToFileName(filePath.c_str()));
    flogv("Starting transfer: %s  file size: %lu  #packets: %lu", FilePath.c_str(), fileSize, FilePacketCount);
    FilePacketHdr hdr = { '1', FilePacketCount };
    uint8_t messageArray[sizeof(hdr) + fileName.length() + 1];
    memcpy(messageArray, &hdr, sizeof(hdr));
    strcpy((char*)(messageArray + sizeof(hdr)), fileName.c_str());
    Send(messageArray, sizeof(messageArray));
}

void ESPNAgent::SendNextFilePacket()
{
    FilePacketSend = false;

    // if got to AFTER the last package
    if (FilePacketNumber > FilePacketCount)
    {
        FilePacketNumber = 0;
        FilePacketCount = 0;
        FilePath = "";
        flogv("File transfer complete");
        return;
    }

    File file = pFS->open(FilePath.c_str(), FILE_READ);
    if (!file)
    {
        FilePacketNumber = 0;
        FilePacketCount = 0;
        FilePath = "";
        floge("File open failed");
        return;
    }

    uint32_t packetDataSize = FilePacketSize;
    if (FilePacketNumber == FilePacketCount)
    {
        // last packet - adjust the size
        packetDataSize = file.size() - ((FilePacketCount - 1) * FilePacketSize);
        // flogv("Last file packet data size: %lu", packetDataSize);
    }

    FilePacketHdr hdr = { '2', FilePacketNumber };
    uint8_t messageArray[sizeof(hdr) + packetDataSize];
    memcpy(messageArray, &hdr, sizeof(hdr));

    // we'll increment the FilePacketNumber in OnDataSent when packet transmission is confirmed

    file.seek((FilePacketNumber - 1) * FilePacketSize);
    file.readBytes((char*)messageArray + sizeof(hdr), packetDataSize);
    file.close();
    Send(messageArray, sizeof(messageArray));
}
