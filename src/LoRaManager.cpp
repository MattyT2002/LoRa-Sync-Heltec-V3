#include "LoRaManager.h"
#include "Config.h"
#include <RadioLib.h>
#include "nodeManager.h"
#include "FragmentedMessageManager.h"

SX1262 LoRaRadio = new Module(LORA_NSS, LORA_DIO0, LORA_RST, LORA_DIO1);

LoRaManager *instancePtr = nullptr;

Packet packetManager;
Packet receivedPacket;
Packet msgPacket;
FragmentedMessageManager fragManager;

LoRaManager::LoRaManager(String nodeId, int nodeNumber, NodeDirectory& nodeDirectory, BLE_Heltec& ble)
    : nodeDirectory(nodeDirectory), ble(ble) {
    NODE_ID = nodeId;
    NODE_NUMBER = nodeNumber;
    lastBroadcast = 0;
    lastPrint = 0;
    instancePtr = this;
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


void LoRaManager::sendHelloPacket() {
    String message = packetManager.helloMessage();
    Serial.print("Sending message: ");
    Serial.println(message);

    auto fragments = fragManager.fragmentMessage(message, MAX_FRAGMENT_SIZE);
    for (auto& frag : fragments) {
        int state = LoRaRadio.transmit(frag);
        delay(100);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("Failed fragment, error: ");
            Serial.println(state);
        }
        LoRaRadio.startReceive();
    }
}

void LoRaManager::sendMessage(String message) {
    Serial.print("Sending message: ");
    Serial.println(message);

    msgPacket = Packet(message);
    String msg = msgPacket.messageToSend(message);
    Serial.print("Fragmenting packet: ");
    Serial.println(msg);

    auto fragments = fragManager.fragmentMessage(msg, MAX_FRAGMENT_SIZE);

    for (int i = 0; i < fragments.size(); i++) {
        Serial.print("Sending fragment ");
        Serial.print(i);
        Serial.print(": size=");
        Serial.println(fragments[i].length());

        int state = LoRaRadio.transmit(fragments[i]);
        delay(100);

        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("Failed fragment, error: ");
            Serial.println(state);
        }
        LoRaRadio.startReceive();
    }
}


void LoRaManager::sendDirectory() {
    Serial.println("Sending full directory update...");
    std::string json = nodeDirectory.toJson();
    String dirPacket = packetManager.dirUpdateMessage(String(json.c_str()));

    auto fragments = fragManager.fragmentMessage(dirPacket, MAX_FRAGMENT_SIZE);
    for (auto& frag : fragments) {
        int state = LoRaRadio.transmit(frag);
        delay(200);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("Failed fragment, error: ");
            Serial.println(state);
        }
        LoRaRadio.startReceive();
    }
}

void LoRaManager::listenForPackets() {
        String str;
        int state = LoRaRadio.readData(str);

        if (state == RADIOLIB_ERR_NONE) {

            if (fragManager.isFragment(str)) {
                fragManager.addFragment(str);
                if (fragManager.isComplete(str)) {
                    String full = fragManager.reassemble(str);
                    Serial.println("Full message reassembled:");
                    Serial.println(full);
                    processMessage(full);
                }
            } else {
                processMessage(str);
            }
        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            Serial.println(F("CRC error!"));
        } else {
            Serial.print(F("Read failed, code "));
            Serial.println(state);
        }
    
}


void LoRaManager::processMessage(const String& str) {
    if (str.startsWith("DIR_UPDATE|")) {
        int firstSep = str.indexOf('|');
        int secondSep = str.indexOf('|', firstSep + 1);

        if (firstSep != -1 && secondSep != -1) {
            String senderNodeStr = str.substring(firstSep + 1, secondSep);
            String jsonPayload = str.substring(secondSep + 1);
            uint16_t senderNodeId = senderNodeStr.toInt();

            NodeDirectory receivedDirectory;
            receivedDirectory.fromJson(jsonPayload.c_str());
            nodeDirectory.mergeDirectory(receivedDirectory, senderNodeId);
        }
    } else {
        receivedPacket = Packet(str);
        if (receivedPacket.deserialize(str)) {
            String senderNumber = receivedPacket.getnodeNumber();
            float snr = LoRaRadio.getSNR();
            nodeDirectory.updateNeighbourNode(senderNumber.toInt(), snr, millis());

            if (receivedPacket.getMessageType(str) == "MESSAGE") {
                if (receivedPacket.getDestinationNode(str) == String(NODE_number)) {
                    ble.sendMessageToUser(receivedPacket.getPayload(str));
                }
            }
        }
    }
}
