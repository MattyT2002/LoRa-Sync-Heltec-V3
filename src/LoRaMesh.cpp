#include "LoRaMesh.h"

SX1262 LoRaRadio = new Module(8, 14, 12);  // NSS, DIO0, RESET, BUSY
// Global instance for callback function access
LoRaMesh* instancePtr = nullptr;

LoRaMesh::LoRaMesh(String nodeId, int nodeNumber) {
    NODE_ID = nodeId;
    NODE_NUMBER = nodeNumber;
    lastBroadcast = 0;
    lastPrint = 0;
    instancePtr = this;  // Store instance for callback use
}

void LoRaMesh::setupLoRa() {
    Serial.begin(115200);
    LoRaRadio.begin(RF_FREQUENCY);  // Initialize RadioLib's radio module

    // Set the transmission power and spreading factor
    LoRaRadio.setOutputPower(TX_OUTPUT_POWER);
    LoRaRadio.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRaRadio.setCodingRate(LORA_CODINGRATE);
    LoRaRadio.setBandwidth(LORA_BANDWIDTH);

    // Start continuous reception
    LoRaRadio.startReceive();

    Serial.println("LoRa Mesh Node Initialized with RadioLib!");
}

// Sends a "HELLO" message to other nodes
void LoRaMesh::sendHelloPacket() {
    String message = "HELLO|" + NODE_ID + "|" + String(NODE_NUMBER);
    Serial.print("Sending message: ");
    Serial.println(message);

    int state = LoRaRadio.transmit(message);

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Transmission successful!");
    } else {
        Serial.print("Transmission failed, error: ");
        Serial.println(state);
    }
}


// Custom function triggered when a message is received
void LoRaMesh::myCustomReceiveFunction() {
    uint8_t buffer[64];
    int packetSize = LoRaRadio.receive(buffer, sizeof(buffer));
    Serial.print("interrupt done ");
    if (packetSize > 0) {
        // Ensure buffer is null-terminated
        char message[packetSize + 1];
        memcpy(message, buffer, packetSize);
        message[packetSize] = '\0';  // Null terminate

        String receivedMessage = String(message);
        float snr = LoRaRadio.getSNR();  // Get the SNR

        Serial.print("Received: ");
        Serial.println(receivedMessage);

        if (receivedMessage.startsWith("HELLO|")) {
            Serial.println("Received HELLO packet!");
            int firstSeparator = receivedMessage.indexOf('|', 6);
            int secondSeparator = receivedMessage.indexOf('|', firstSeparator + 1);
            if (firstSeparator != -1 && secondSeparator != -1) {
                String senderName = receivedMessage.substring(6, firstSeparator);
                int senderID = receivedMessage.substring(firstSeparator + 1, secondSeparator).toInt();
                instancePtr->updateDirectory(senderName, senderID, snr);
            }
        }
    }

    // Resume listening for new messages
    LoRaRadio.startReceive();
}

// Updates the node directory when a new node is discovered
void LoRaMesh::updateDirectory(String senderID, int senderNumber, float snr) {
    bool found = false;
    for (auto &entry : nodeDirectory) {
        if (entry.nodeID == senderID) {
            entry.snr = snr;
            entry.lastSeen = millis();
            found = true;
            break;
        }
    }

    if (!found) {
        nodeDirectory.push_back({senderID, senderNumber, snr, millis()});
        Serial.println("New node discovered: " + senderID + " | ID: " + String(senderNumber) + " | SNR: " + String(snr));
    }
}

// Prints the directory of discovered nodes
void LoRaMesh::printDirectory() {
    Serial.println("\nNode Directory:");
    for (const auto &entry : nodeDirectory) {
        Serial.println("Node: " + entry.nodeID + " | ID: " + String(entry.senderID) + " | SNR: " + String(entry.snr) +
                       " | Last Seen: " + String((millis() - entry.lastSeen) / 1000) + " sec ago");
    }
}

