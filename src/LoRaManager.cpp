#include "LoRaManager.h"
#include "Config.h"
#include <RadioLib.h>  
#include "nodeManager.h"

SX1262 LoRaRadio = new Module(LORA_NSS, LORA_DIO0, LORA_RST, LORA_DIO1); // NSS, DIO0, RESET, DIO1 pins

LoRaManager *instancePtr = nullptr;



// Node and Packet Management

Packet packetManager;
Packet recievedPacket;
Packet msgPacket; // Packet to send


LoRaManager::LoRaManager(String nodeId, int nodeNumber, NodeDirectory& nodeDirectory, BLE_Heltec& ble) 
    : nodeDirectory(nodeDirectory), ble(ble) {
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
    Serial.print("Sending packet: ");
    Serial.println(msg);
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

void LoRaManager::sendDirectory()
{
    Serial.println("Sending full directory update...");

    // Get the current directory as JSON
    std::string json = nodeDirectory.toJson();

    // Build the DIR_UPDATE packet
    String dirPacket = packetManager.dirUpdateMessage(String(json.c_str()));

    // Transmit it
    int state = LoRaRadio.transmit(dirPacket);

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Directory update sent successfully!");
    } else {
        Serial.print("Directory update failed, error: ");
        Serial.println(state);
    }
}

void LoRaManager::listenForPackets()
{
    

    String str;
    int state = LoRaRadio.receive(str);

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
        Serial.print(F("[SX1262] Data:\t"));
        Serial.println(str);

        if (str.startsWith("DIR_UPDATE|")) {
            Serial.println("Received a directory update!");

            // Split the string: DIR_UPDATE|senderNode|{JSON}
            int firstSep = str.indexOf('|');
            int secondSep = str.indexOf('|', firstSep + 1);

            if (firstSep != -1 && secondSep != -1) {
                String senderNodeStr = str.substring(firstSep + 1, secondSep);
                String jsonPayload = str.substring(secondSep + 1);

                uint16_t senderNodeId = senderNodeStr.toInt();

                Serial.print("Sender Node ID: ");
                Serial.println(senderNodeId);

                Serial.println("Merging received directory...");

                NodeDirectory receivedDirectory;
                receivedDirectory.fromJson(jsonPayload.c_str());

                nodeDirectory.mergeDirectory(receivedDirectory, senderNodeId);

                Serial.println("Directory merge complete!");
            }
            else {
                Serial.println("Invalid DIR_UPDATE format!");
            }
        }
        else {
            recievedPacket = Packet(str);
            if (recievedPacket.deserialize(str)) {
                Serial.println("Received valid packet!");
                String senderName = recievedPacket.getnodeName();
                String senderNumber = recievedPacket.getnodeNumber();
                float snr = LoRaRadio.getSNR();
                nodeDirectory.updateNeighbourNode(senderNumber.toInt(), snr, millis());
                if(recievedPacket.getMessageType(str) ="MESSAGE") {
                    if(recievedPacket.getDestinationNode(str) = NODE_number) {
                        Serial.println("Received MESSAGE packet!");
                        // Send the message to the user via BLE
                        ble.sendMessageToUser(recievedPacket.getPayload(str));
                    } else {
                        Serial.println("Message not for this node.");

                    }
                } else if(recievedPacket.getPacketType() = "MESSAGE_Hop") {
                    
                }
            } else {
                Serial.println("Received invalid packet!");
            }
        }
    }
    else if (state == RADIOLIB_ERR_RX_TIMEOUT)
    {
        
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




