#ifndef CONFIG_H
#define CONFIG_H

// Device Information
#define NODE_number 2      
#define NODE_name  "Home"             
#define Chip SX1262

// LoRa Chip Pins
#define LORA_NSS 8              // Slave Select pin
#define LORA_RST 12             // Reset pin
#define LORA_DIO0 14            // DIO0 pin 
#define LORA_DIO1 13            // DIO1 pin 

// LoRa Configuration
#define LORA_FREQUENCY 868E6   // Frequency in Hz 
#define LORA_BANDWIDTH 125     // Bandwidth in kHz (125, 250, or 500)
#define LORA_SPREADING_FACTOR 7 // Spreading factor (7-12)
#define LORA_CODING_RATE 5     // Coding rate denominator (5-8)


#endif