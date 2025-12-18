#include "wireless.h"

int messageTime = -3000;
int timeSinceLastMessage=0;

float inputVoltage = 0;
float inputCurrent = 0;
float inputWattage = 0;

float chargeWattage = 0;
float dischargeWattage = 0;

float batteryPercentage = 0;
float netCharge_Ah = 0;
float batteryTimeLeft_h = 0;

typedef struct struct_message {
  float inputVoltage_message;
  float inputCurrent_message;
  float inputWattage_message;

  float chargeWattage_message;
  float dischargeWattage_message;

  float batteryPercentage_message;
  float netCharge_Ah_message;
  float batteryTimeLeft_h_message;
} struct_message;

struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  messageTime = millis();
  memcpy(&myData, incomingData, sizeof(myData));
  inputVoltage = myData.inputVoltage_message;
  inputCurrent = myData.inputCurrent_message;
  inputWattage = myData.inputWattage_message;

  chargeWattage = myData.chargeWattage_message;
  dischargeWattage = myData.dischargeWattage_message;

  batteryPercentage = myData.batteryPercentage_message;
  netCharge_Ah = myData.netCharge_Ah_message;
  batteryTimeLeft_h = myData.batteryTimeLeft_h_message;
}

void Wireless_Init(void){
  // Initialize Wi-Fi in station mode
  WiFi.mode(WIFI_STA);
  Serial.println("Wi-Fi initialized in station mode.");
  delay(1000);
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW.");
    return;
  }
  Serial.println("ESP-NOW initialized.");

  // Register callback for receiving messages
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}