
#include "Config.h"
#include "NodeDirectory.h"
#include "LoRaManager.h"

#define LED 9
#define N_NODES 2
#include "Config.h"
#include "BLE_Heltec.h"
BLE_Heltec ble;

NodeDirectory nodeDirectory;

LoRaManager meshNode(NODE_name, NODE_number, nodeDirectory, ble);


void onBLEMessageReceived(const String& msg)
{
    Serial.println("Callback: Web app sent --> " + msg);
    meshNode.sendMessage(msg);
    // Example: you could broadcast the message over LoRa
    // meshNode.sendCustomMessage(msg);

    // Example: Echo back
    ble.sendMessageToUser("Received: " + msg);
}


void setup() {
    Serial.begin(115200);
    ble.begin(NODE_name);
    ble.setMessageCallback([](const String& message) {
        onBLEMessageReceived(message);
    });
    meshNode.setupLoRa();
    nodeDirectory.setSelfId(NODE_number);

}


static unsigned long lastBroadcast = 0;
static unsigned long lastPrint = 0;
static unsigned long lastCleanup = 0;

void loop()
{
     
    if (millis() - lastBroadcast > BROADCAST_INTERVAL) {
        meshNode.sendHelloPacket();
        lastBroadcast = millis();
        
       
    }
    
    meshNode.listenForPackets();


    if (millis() - lastPrint > PRINT_INTERVAL) {
        lastPrint = millis();
        ble.sendMessageToUser(nodeDirectory.toVisJson().c_str());
        Serial.println(nodeDirectory.toVisJson().c_str());
        ble.sendMessageToUser("Node directory sent!");
       
    }
    if (millis() - lastCleanup > CLEANUP_INTERVAL) {
        nodeDirectory.removeStaleNodes(NEIGHBOUR_TIMEOUT);
        lastCleanup = millis();
    }
    delay(100);
}

