#include "BLE.h"

#include "LoRaMesh.h"
#define RH_HAVE_SERIAL
#define LED 9
#define N_NODES 2

// Create an instance of the BLE class
BLE ble;


LoRaMesh meshNode("NODE_A", 1);


void setup()
{
  ble.begin();  // Initialize BLE service
  //node.setupLoRa();
  meshNode.setupLoRa();

}

void loop()
{
  static unsigned long lastBroadcast = 0;
  static unsigned long lastPrint = 0;

  // Send Hello Packet Every Interval
  if (millis() - lastBroadcast > LoRaMesh::BROADCAST_INTERVAL) {
      meshNode.sendHelloPacket();
      lastBroadcast = millis();
  }

  // LoRa reception is handled in the background via the callback function
  // RadioLib is handling the interrupt-driven receive automatically.

  // Print Directory Every 20 Seconds
  if (millis() - lastPrint > LoRaMesh::PRINT_INTERVAL) {
      meshNode.printDirectory();
      lastPrint = millis();
  }
}