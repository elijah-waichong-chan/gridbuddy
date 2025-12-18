#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "screen.h"
#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "global_vars.h"

int time_s=0;

float inputVoltage = 0;
float inputCurrent = 0;
float inputWattage = 0;

typedef struct struct_message {
  float volt_message;
  float current_message;
  float wattage_message;
} struct_message;

struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  time_s++;
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Input Voltage: ");
  Serial.println(myData.volt_message);
  Serial.print("Input Current: ");
  Serial.println(myData.current_message);
  Serial.print("Input Wattage: ");
  Serial.println(myData.wattage_message);
  Serial.println();
  inputVoltage = myData.volt_message;
  inputCurrent = myData.current_message;
  inputWattage = myData.wattage_message;
}

void setup() {
  Serial.begin(115200);
  LCD_Init();
  Lvgl_Init();
  startScreen();

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

void loop() {
  Timer_Loop();
}