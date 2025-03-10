#include "BLE.h"

// Create an instance of the BLE class
BLE ble;

void setup() {
    ble.begin();  // Initialize BLE service
}

void loop() {
    ble.loop();  // Run the BLE service
}
