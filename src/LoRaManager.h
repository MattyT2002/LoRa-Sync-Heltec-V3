#ifndef LORAMESH_H
#define LORAMESH_H


#include "LoRaManager.h"
#include <vector>
#include <String.h>
#include "Packet.h"
#include "NodeDirectory.h"

// Node structure to hold information about each node
struct Node {
    String nodeID;
    int senderID;
    float snr;
    unsigned long lastSeen;
};



// LoRaMesh class to manage LoRa communication and BLE interactions
class LoRaManager {
public:
    LoRaManager(String nodeId, int nodeNumber, NodeDirectory& nodeDirectory); // Constructor
    
    void setupLoRa(); // Initialize LoRa communication
    void sendHelloPacket(); // Send "Hello" packet to network
    void sendMessage(String message); // Send a generic message
    void listenForPackets(); // Listen for incoming packets
    void sendNodeDirectory(); // Send the node directory to network
    void sendNodeDirectoryToGUI(); // Send the node directory to GUI over BLE
    void updateDirectory(String senderID, int senderNumber, float snr); // Update the node directory
    
    
    
private:
    String NODE_ID;
    int NODE_NUMBER;
    unsigned long lastBroadcast;
    unsigned long lastPrint;
    NodeDirectory& nodeDirectory; // Reference to the node directory
   
    Packet packetManager; // Handles packet management
};

#endif // LORAMESH_H
