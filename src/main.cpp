
#include "Config.h"
#include "NodeDirectory.h"
#include "LoRaManager.h"

#define LED 9
#define N_NODES 2
#include "Config.h"

#if defined(BOARD_HELTEC)
    #include "BLE_Heltec.h"
    BLE_Heltec ble;
#elif defined(BOARD_NANO33BLE)
    #include "BLE_Nano33.h"
    BLE_Nano33 ble;
#endif
NodeDirectory nodeDirectory;
LoRaManager meshNode(NODE_name, NODE_number, nodeDirectory);

void setup() {
    Serial.begin(115200);
    ble.begin(NODE_name);
    ble.setMessageCallback([](const String& message) {
        Serial.println("Received: " + message);
    });
    meshNode.setupLoRa();
    nodeDirectory.setSelfId(NODE_number);

}


static unsigned long lastBroadcast = 0;
static unsigned long lastPrint = 0;


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
}

// === This function is called automatically when a message is received from Web App ===
void onBLEMessageReceived(const String& msg)
{
    Serial.println("Callback: Web app sent --> " + msg);

    // Example: you could broadcast the message over LoRa
    // meshNode.sendCustomMessage(msg);

    // Example: Echo back
    ble.sendMessageToUser("Received: " + msg);
}
