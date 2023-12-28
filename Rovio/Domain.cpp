#include <WiFi.h>
#include "Domain.h"
#include "FLogger.h"

// CONSIDER:
// Using an unenforced Singleton Domain here because the ESP_NOW callbacks
// need to be static and we need to connect to the associated Domain.
// If we need multiple multiple Domains later we must track them and associate
// using the MAC addr passed to the callback.
Domain* Singleton;

Entity* Domain::GetEntity(EntityID entity)
{
    //flogd("entity: %i, list: %i", entity, Entities);
    // search all Entities
    for (Entity** ppe = Entities; (*ppe) != nullptr; ppe++)
    {
        // find the matching ID
        if ((*ppe)->GetID() == entity)
        {
            //flogd("-> %s", (*ppe)->GetName());
            return *ppe;
        }
    }
    // no match is an error! not fatal here, but probably will be?!
    floge("entity not found: %i", entity);
    return nullptr;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Singleton->DataSent = false;
    if (status != ESP_NOW_SEND_SUCCESS)
        flogw("Delivery Fail");
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *pData, int len)
{
    // data received is a set of Packets we examine one at a time
    Packet packet;
    // calc count of Packets
    int cnt = len / sizeof(Packet);
    //flogi("OnDataRecv: %i %i %i", len, sizeof(Packet), cnt);
    // process each Packet
    while (cnt > 0)
    {
        // copy Packet (could maybe just coerce to a packet pointer?!)
        memcpy(&packet, pData, sizeof(Packet));
        // advance to next
        pData += sizeof(Packet);
        cnt--;
        //flogd("%s.%s -> %i", Singleton->GetEntity(packet.entity)->GetName(), Singleton->GetEntity(packet.entity)->GetProperty(packet.property)->GetName(), packet.value);
        Singleton->SetEntityPropertyValue(packet.entity, packet.property, packet.value);
    }
}

void Domain::Init(uint8_t addr[])
{
    Singleton = this;

    flogi("WIFI init");
    if (!WiFi.mode(WIFI_STA))
        flogf("%s FAILED", "WIFI init");

    flogi("MAC addr: %s", WiFi.macAddress().c_str());

    flogi("ESP_NOW init");
    if (esp_now_init() != ESP_OK)
        flogf("%s FAILED", "ESP_NOW init");

    //flogi("ESP_NOW peer add");
    memset(&PeerInfo, 0, sizeof(PeerInfo));
    memcpy(PeerInfo.peer_addr, addr, sizeof(PeerInfo.peer_addr));
    //PeerInfo.channel = 0;
    //PeerInfo.encrypt = false;
    if (esp_now_add_peer(&PeerInfo) != ESP_OK)
        flogf("%s FAILED", "ESP_NOW peer add");
    
    //flogi("ESP_NOW send cb");
    esp_now_register_send_cb(OnDataSent);

    //flogi("ESP_NOW recv cb");
    esp_now_register_recv_cb(OnDataRecv);

    flogi("ESP_NOW init complete");
}

void Domain::ProcessChanges(propChange_cb cb)
{
    // data is transmitted as a set of Packets
    const int len = 10;
    Packet packets[len];     // enough for a few properties
    // track the count
    uint8_t cnt = 0;
    // scan all Entities
    for (Entity** ppe = Entities; (*ppe) != nullptr; ppe++)
    {
        // scan all Properties
        for (Property** ppp = (*ppe)->Properties; (*ppp) != nullptr; ppp++)
        {
            // ignore if not Changed
            if (!(*ppp)->GetChanged())
                continue;
            // notify the caller of a changed Property
            if (cb)
                (*cb)(*ppe, *ppp);
            // if Server, only transmit if Property is marked for Output 
            if (Server && !(*ppp)->IsOutput())
                continue;
            // if !Server, only transmit if Property is marked for Input (for the Server)
            if (!Server && !(*ppp)->IsInput())
                continue;
            // make sure we don't exceed out fixed array
            if (cnt >= len)
            {
                floge("packet buffer too small");
                break;
            }
            //flogd("%s.%s -> %i", (*ppe)->GetName(), (*ppp)->GetName(), (*ppp)->Get());
            // fill a Packet
            packets[cnt].entity = (*ppe)->GetID();
            packets[cnt].property = (*ppp)->GetID();
            packets[cnt].value = (*ppp)->Get();
            // move on to the next
            cnt++;
        }
    }

    // if there are any Packets to send
    if (cnt > 0)
    {
        if (DataSent)
        {
            unsigned long ms = millis();
            while (DataSent)
            {
                if (millis() - ms > 100)
                    DataSent = false;
            }
        }

        DataSent = true;
        esp_err_t result = esp_now_send(PeerInfo.peer_addr, (uint8_t*)&packets, cnt * sizeof(Packet));
        if (result != ESP_OK)
        {
            floge("Error sending data: %s", esp_err_to_name(result));
            DataSent = false;
        }
    }
}
