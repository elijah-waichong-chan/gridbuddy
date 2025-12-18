#include "wireless.h"

int messageTime=-5000;


// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x24, 0xD7, 0xEB, 0x49, 0x07, 0xE8};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float inputVoltage_message;
  float inputCurrent_message;
  float inputWattage_message;

  float chargeWattage_message;
  float dischargeWattage_message;

  float batteryPercentage_message;
  float netChange_Ah_message;
  float batteryTimeLeft_h_message;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if(status == ESP_NOW_SEND_SUCCESS){
    messageTime = millis();
  }
}

void wireless_Init(void) {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void SendMessage(void) 
{
  // Set values to send
  myData.inputVoltage_message = inputVoltage;
  myData.inputCurrent_message = inputCurrent;
  myData.inputWattage_message = inputWattage;

  myData.chargeWattage_message = chargeWattage;
  myData.dischargeWattage_message = dischargeWattage;

  myData.batteryPercentage_message = batteryPercentage;
  myData.netChange_Ah_message = netChange_Ah;
  myData.batteryTimeLeft_h_message = batteryTimeLeft_h;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

}