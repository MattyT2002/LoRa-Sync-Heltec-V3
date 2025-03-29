#include "LoRaMesh.h"

SX1262 LoRaRadio = new Module(8, 14, 12, 13); // NSS, DIO0, RESET, BUSY
// Global instance for callback function access
LoRaMesh *instancePtr = nullptr;

LoRaMesh::LoRaMesh(String nodeId, int nodeNumber)
{
    NODE_ID = nodeId;
    NODE_NUMBER = nodeNumber;
    lastBroadcast = 0;
    lastPrint = 0;
    instancePtr = this; // Store instance for callback use
}

void LoRaMesh::setupLoRa()
{
    Serial.begin(115200);
    // Initialize LoRa module
    Serial.print(F("[SX1262] Initializing ... "));
    int state = LoRaRadio.begin(868.0);
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
    updateDirectory(NODE_ID, NODE_NUMBER, 0); // Add self to directory
}

// Sends a "HELLO" message to other nodes
// Sends a "LoRaMeshNode" message to other nodes
void LoRaMesh::sendHelloPacket()
{
    String message = "LoRaMeshNode|" + NODE_ID + "|" + String(NODE_NUMBER);
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

void LoRaMesh::listenForPackets()
{
    Serial.print(F("[SX1262] Waiting for incoming transmission ... "));

    String str;
    int state = LoRaRadio.receive(str);

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

        // Check if message starts with "LoRaMeshNode|"
        if (str.startsWith("LoRaMeshNode|"))
        {
            Serial.println("Received LoRaMeshNode packet!");
            Serial.println("Raw received message: [" + str + "]");

            int firstSeparator = str.indexOf('|');
            int secondSeparator = str.indexOf('|', firstSeparator + 1);

            Serial.print("First Separator Position: ");
            Serial.println(firstSeparator);
            Serial.print("Second Separator Position: ");
            Serial.println(secondSeparator);

            if (firstSeparator != -1 && secondSeparator != -1)
            {
                String senderName = str.substring(firstSeparator + 1, secondSeparator);
                int senderNumber = str.substring(secondSeparator + 1).toInt();
                float snr = LoRaRadio.getSNR();
                Serial.println("Extracted Sender Name: [" + senderName + "]");
                Serial.println("Extracted Sender Number: [" + String(senderNumber) + "]");
                updateDirectory(senderName, senderNumber, snr);
                sendNodeDirectory();
            }
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

void LoRaMesh::sendNodeDirectory()
{
    Serial.println("Sending node directory");
    for (const auto &entry : nodeDirectory)
    {
        String message = "DIR_UPDATE|" + entry.nodeID + "|" + String(entry.senderID) + "|" + String(entry.snr);
        LoRaRadio.transmit(message);
        delay(50); // Avoid collisions
    }
}

// Updates the node directory when a new node is discovered
void LoRaMesh::updateDirectory(String senderID, int senderNumber, float snr)
{
    Serial.println("updateDirectory() called with: " + senderID + ", ID: " + String(senderNumber) + ", SNR: " + String(snr));

    bool found = false;

    for (auto &entry : nodeDirectory)
    {
        if (entry.nodeID == senderID)
        {
            Serial.println("Node already in directory, updating SNR...");
            entry.snr = snr;
            entry.lastSeen = millis();
            found = true;
            break;
        }
    }

    if (!found)
    {
        Serial.println("Adding new node to directory...");
        nodeDirectory.push_back({senderID, senderNumber, snr, millis()});
        Serial.println("New node added: " + senderID + " | ID: " + String(senderNumber) + " | SNR: " + String(snr));
    }

    Serial.println("Directory now contains " + String(nodeDirectory.size()) + " nodes.");
}

// Prints the directory of discovered nodes
void LoRaMesh::printDirectory()
{
    Serial.println("\nNode Directory:");

    if (nodeDirectory.empty())
    {
        Serial.println("No nodes in directory.");
        return;
    }

    for (const auto &entry : nodeDirectory)
    {
        Serial.println("Node: " + entry.nodeID +
                       " | ID: " + String(entry.senderID) +
                       " | SNR: " + String(entry.snr) +
                       " | Last Seen: " + String((millis() - entry.lastSeen) / 1000) + " sec ago");
    }
}