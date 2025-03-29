#ifndef LORAMESH_H
#define LORAMESH_H

#include <Arduino.h>
#include <RadioLib.h>
#include <vector>


extern SX1262 LoRaRadio;

#define LORA_SCK 9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_NSS 8  // Chip Select (SS)
#define LORA_RST 12
#define LORA_DIO0 14  // Interrupt (DIO0)
#define LORA_BUSY 13  

#define RF_FREQUENCY 868.0  // Adjust based on region (868E6 for EU, 915E6 for US)
#define TX_OUTPUT_POWER 14  // LoRa transmit power (dBm)
#define LORA_BANDWIDTH 0    // 0 = 125kHz, 1 = 250kHz, 2 = 500kHz
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1


// Node Entry Structure
struct NodeEntry {
    String nodeID;
    int senderID;
    float snr;
    unsigned long lastSeen;
};

class LoRaMesh {
public:
    // Constructor
    LoRaMesh(String nodeId, int nodeNumber);

    // Public Methods
    void setupLoRa();
    void sendHelloPacket();
    void listenForPackets();
    void updateDirectory(String senderID, int senderNumber, float snr);
    void printDirectory();
    void sendNodeDirectory();

    static const unsigned long BROADCAST_INTERVAL = 10000;  // 10 seconds
    static const unsigned long PRINT_INTERVAL = 20000;      // 20 seconds
private:
    String NODE_ID;
    int NODE_NUMBER;
    std::vector<NodeEntry> nodeDirectory;
    unsigned long lastBroadcast;
    unsigned long lastPrint;

    int state; // current state of the radio

};
#endif  // LORAMESH_H
