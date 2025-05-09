
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
    String destinationNodeString = msg.substring(0, msg.indexOf('|'));
    int destinationNode = destinationNodeString.toInt();
    Serial.println("Destination Node: " + String(destinationNode));
    meshNode.sendMessage(msg, destinationNode);
    


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
    meshNode.setToReceive(); 
    if (millis() - lastBroadcast > BROADCAST_INTERVAL) {
        meshNode.sendHelloPacket();
        lastBroadcast = millis();
        
       
    }
    delay(2000);
    meshNode.listenForPackets();
    


    if (millis() - lastPrint > PRINT_INTERVAL) {
        lastPrint = millis();
        ble.sendMessageToUser(nodeDirectory.toVisJson().c_str());
        Serial.println(nodeDirectory.toVisJson().c_str());
        meshNode.sendDirectory();
       
    }
    if (millis() - lastCleanup > CLEANUP_INTERVAL) {
        nodeDirectory.removeStaleNodes(NEIGHBOUR_TIMEOUT);
        lastCleanup = millis();
        meshNode.sendMessage("Test",TEST_MESSAGE_DESTINATION);

    }
    
}