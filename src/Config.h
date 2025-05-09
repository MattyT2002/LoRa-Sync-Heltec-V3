#ifndef CONFIG_H
#define CONFIG_H

// Device Information
#define NODE_number 3 
#define NODE_name  "Node 3 Home"             
//#define Chip SX1276 //E32 868
#define Chip SX1262 //helect

#define BOARD_HELTEC
//#define BOARD_NANO33BLE

#define LORA_NSS 8              // Slave Select pin
#define LORA_RST 12             // Reset pin
#define LORA_DIO0 14            // DIO0 pin 
#define LORA_DIO1 13            // DIO1 pin 




// LoRa Configuration
#define LORA_FREQUENCY 868E6   // Frequency in Hz 
#define LORA_BANDWIDTH 125   // Bandwidth in kHz (125, 250, or 500)
#define LORA_SPREADING_FACTOR 7 // Spreading factor (7-12)
#define LORA_CODING_RATE 5     // Coding rate denominator (5-8)
#define LORA_OUTPUT_POWER 14 // Output power in dBm


#define BROADCAST_INTERVAL 30000 // Interval to send hello packet in ms
#define PRINT_INTERVAL 30000 // Interval to print directory in ms
#define CLEANUP_INTERVAL 120000 // Interval to remove stale nodes in ms
#define NEIGHBOUR_TIMEOUT 120000 // Timeout for neighbor nodes in ms
#define IGNORE_NODE "999"
#define TEST_MESSAGE_DESTINATION 999 // Destination node for test messages

#endif