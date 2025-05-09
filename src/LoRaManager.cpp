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

LoRaManager::LoRaManager(String nodeId, int nodeNumber, NodeDirectory &nodeDirectory, BLE_Heltec &ble)
    : nodeDirectory(nodeDirectory), ble(ble)
{
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

void LoRaManager::sendHelloPacket()
{
    String message = packetManager.helloMessage();
    Serial.print("Sending hello message: ");
    Serial.println(message);

    int state = LoRaRadio.transmit(message);
    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.print("Failed to send hello message, error: ");
        Serial.println(state);
    }
    LoRaRadio.startReceive();

    if (state != RADIOLIB_ERR_NONE)
    {
        Serial.print("Failed fragment, error: ");
        Serial.println(state);
    }
    LoRaRadio.startReceive();
}

void LoRaManager::sendMessage(String message, int finalDestNode)
{
    int nextNode = nodeDirectory.getNextHopTo(finalDestNode);

    // Constructing the message with multi-hop info
    String packetStr = "MESSAGE|" + String(NODE_NUMBER) + "|" + String(nextNode) + "|" + String(finalDestNode) + "|" + message;

    auto fragments = fragManager.fragmentMessage(packetStr, MAX_FRAGMENT_SIZE);

    // Send each fragment with retry on failure
    for (int i = 0; i < fragments.size(); i++)
    {
        Serial.print("Sending fragment ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(fragments[i]);

        int state = LoRaRadio.transmit(fragments[i]);
        delay(100); // Slight delay between fragments

        if (state != RADIOLIB_ERR_NONE)
        {
            Serial.print("Failed to send fragment, error: ");
            Serial.println(state);
            // Retry sending the fragment, or handle failure
        }
        LoRaRadio.startReceive();
    }
}

void LoRaManager::sendDirectory()
{
    Serial.println("Sending full directory update...");
    std::string json = nodeDirectory.toJson();
    String jsonStr = String(json.c_str()); // convert before use
    String dirPacket = packetManager.dirUpdateMessage(jsonStr);

    auto fragments = fragManager.fragmentMessage(dirPacket, MAX_FRAGMENT_SIZE);
    for (auto &frag : fragments)
    {
        int state = LoRaRadio.transmit(frag);
        delay(200);
        if (state != RADIOLIB_ERR_NONE)
        {
            Serial.print("Failed fragment, error: ");
            Serial.println(state);
        }
        LoRaRadio.startReceive();
    }
}

void LoRaManager::listenForPackets()
{
    String str;
    int state = LoRaRadio.readData(str);
    if (state == RADIOLIB_ERR_NONE && str.length() == 0)
    {
        return; // No data received
    }
    if (state == RADIOLIB_ERR_NONE)
    {

        if (fragManager.isFragment(str))
        {
            fragManager.addFragment(str);
            if (fragManager.isComplete(str))
            {
                String full = fragManager.reassemble(str);
                Serial.println("Full message reassembled:");
                Serial.println(full);
                processMessage(full);
                return;
            }
        }
        else
        {
            processMessage(str);
        }
    }
    else if (state == RADIOLIB_ERR_CRC_MISMATCH)
    {
        Serial.println(F("CRC error!"));
    }
    else
    {
        Serial.print(F("Read failed, code "));
        Serial.println(state);
    }
}

void LoRaManager::processMessage(const String &str)
{
    if (str.startsWith("DIR_UPDATE|"))
    {
        // Handle directory update messages
        int firstSep = str.indexOf('|');
        int secondSep = str.indexOf('|', firstSep + 1);

        if (firstSep != -1 && secondSep != -1)
        {
            String senderNodeStr = str.substring(firstSep + 1, secondSep);
            String jsonPayload = str.substring(secondSep + 1);
            uint16_t senderNodeId = senderNodeStr.toInt();

            NodeDirectory receivedDirectory;
            receivedDirectory.fromJson(jsonPayload.c_str());
            nodeDirectory.mergeDirectory(receivedDirectory, senderNodeId);
        }
    }
    else
    {
        // Process regular message
        receivedPacket = Packet(str);
        if (receivedPacket.deserialize(str))
        {
            String senderNumber = receivedPacket.getnodeNumber();
            float snr = LoRaRadio.getSNR();

            if (senderNumber != IGNORE_NODE)
            {
                nodeDirectory.updateNeighbourNode(senderNumber.toInt(), snr, millis());
            }

            // Check if the message is for this node
            if (str.startsWith("MESSAGE|"))
            {
                Serial.println("Received message: " + str);
                int destinationNode = receivedPacket.getDestinationNode(str).toInt();
                int nextHop = receivedPacket.getNextHop(str).toInt();

                if (destinationNode == NODE_NUMBER)
                {
                    Serial.println("Message for me, sending to BLE: " + receivedPacket.getPayload(str));
                    ble.sendMessageToUser(receivedPacket.getPayload(str));
                }
                else if (nextHop == NODE_NUMBER)
                {
                    Serial.println("Forwarding message to next hop: " + receivedPacket.getPayload(str));
                    sendMessageNextHope(receivedPacket.getPayload(str), destinationNode);
                }
                else
                {
                    Serial.println("Message not for me, ignoring.");
                }
            }
        }
    }
}

void LoRaManager::sendMessageNextHope(const String& msg, int destinationNode)
{
    int nextNode = nodeDirectory.getNextHopTo(destinationNode);
    String originalSenderNumber = receivedPacket.getOriginalSender(msg);
    String finalDestination = receivedPacket.getFinalDestination(msg);
    String PayloadOnly = receivedPacket.getPayloadOnly(msg);
    String newMsg = "MESSAGE|" + originalSenderNumber + "|" + String(nextNode) + "|" + finalDestination + "|" + PayloadOnly;
    Serial.println("Forwarding message: " + newMsg);
    {
        auto fragments = fragManager.fragmentMessage(newMsg, MAX_FRAGMENT_SIZE);
        for (auto &frag : fragments)
        {
            int state = LoRaRadio.transmit(frag);
            delay(100);
            if (state != RADIOLIB_ERR_NONE)
            {
                Serial.print("Forwarding failed, error: ");
                Serial.println(state);
            }
            
        }
    }
}