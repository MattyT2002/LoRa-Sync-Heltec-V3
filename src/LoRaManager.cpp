#include "LoRaManager.h"
#include "Config.h"
#include <RadioLib.h>  
#include "nodeManager.h"

SX1262 LoRaRadio = new Module(LORA_NSS, LORA_DIO0, LORA_RST, LORA_DIO1); // NSS, DIO0, RESET, DIO1 pins
NodeManager nodeManager; // NodeManager instance to manage nodes
// Global instance for callback function access
LoRaManager *instancePtr = nullptr;



// Node and Packet Management

Packet packetManager;
Packet recievedPacket;
Packet msgPacket; // Packet to send


LoRaManager::LoRaManager(String nodeId, int nodeNumber, NodeDirectory& nodeDirectory) : nodeDirectory(nodeDirectory)
{
    NODE_ID = nodeId;
    NODE_NUMBER = nodeNumber;
    lastBroadcast = 0;
    lastPrint = 0;
    instancePtr = this; // Store instance for callback use
}

void LoRaManager::setupLoRa()
{
    
    Serial.begin(115200);
    // Initialize LoRa module
    Serial.print(F("[SX1262] Initializing ... "));
    int state = LoRaRadio.begin();
    LoRaRadio.setBandwidth(LORA_BANDWIDTH);
    LoRaRadio.setFrequency(LORA_FREQUENCY);
    LoRaRadio.setCodingRate(LORA_CODING_RATE);
    LoRaRadio.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRaRadio.setOutputPower(LORA_OUTPUT_POWER); 
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
        {
            delay(10);
        }
    }

    Serial.println("LoRa Mesh Node Initialized with RadioLib!");
    
}

void LoRaManager::sendHelloPacket()
{
    String message = packetManager.helloMessage();
    Serial.print("Sending message: ");
    Serial.println(message);

    int state = LoRaRadio.transmit(message);

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println("Transmission successful!");
    }
    else
    {
        Serial.print("Transmission failed, error: ");
        Serial.println(state);
    }
}

void LoRaManager::sendMessage(String message)
{
    Serial.print("Sending message: ");
    Serial.println(message);

    msgPacket = Packet(message);
    String msg = msgPacket.messageToSend(message);
    int state = LoRaRadio.transmit(msg);

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println("Transmission successful!");
    }
    else
    {
        Serial.print("Transmission failed, error: ");
        Serial.println(state);
    }
    delay(10); // Avoid collisions 
}

void LoRaManager::listenForPackets()
{
    Serial.print(F("[SX1262] Waiting for incoming transmission ... "));

    String str;
    int state = LoRaRadio.receive(str);
    
    recievedPacket = Packet(str);
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
        Serial.print(F("[SX1262] Data:\t"));
        Serial.println(str);

        Serial.print(F("[SX1262] RSSI:\t"));
        Serial.print(LoRaRadio.getRSSI());
        Serial.println(F(" dBm"));

        Serial.print(F("[SX1262] SNR:\t"));
        Serial.print(LoRaRadio.getSNR());
        Serial.println(F(" dB"));

        Serial.print(F("[SX1262] Frequency error:\t"));
        Serial.print(LoRaRadio.getFrequencyError());
        Serial.println(F(" Hz"));

        // Process received packet
        recievedPacket = Packet(str);
        if (recievedPacket.deserialize(str))
        {
            Serial.println("Received valid packet!");
            String senderName = recievedPacket.getnodeName();
            String senderNumber = recievedPacket.getnodeNumber();
            float snr = LoRaRadio.getSNR();
            
            nodeDirectory.updateNeighbourNode(senderNumber.toInt(), snr, millis());
        }
        else
        {
            Serial.println("Received invalid packet!");
        }
    }
    else if (state == RADIOLIB_ERR_RX_TIMEOUT)
    {
        Serial.println(F("timeout!"));
    }
    else if (state == RADIOLIB_ERR_CRC_MISMATCH)
    {
        Serial.println(F("CRC error!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
    }

}


