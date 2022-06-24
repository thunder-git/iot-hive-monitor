/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

#define SLEEP_SECS 15 
#define SEND_TIMEOUT 245  // 245 millis seconds timeout 

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xC4, 0x5B, 0xBE, 0x55, 0x14, 0xD0}; //C4:5B:BE:55:14:D0

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  String d;
  bool e;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer

volatile boolean callbackCalled;
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  delay(500);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println();
  Serial.print("Hive Monitor MAC Address:  ");
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    gotoSleep();;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    Serial.printf("send_cb, send done, status = %i\n", sendStatus);
    callbackCalled = true;
  });

  callbackCalled = false;

  uint8_t bs[sizeof(myData)];
  memcpy(bs, &myData, sizeof(myData));
  esp_now_send(NULL, bs, sizeof(myData)); // NULL means send to all peers
}
 
void loop() {
  if (callbackCalled || (millis() > SEND_TIMEOUT)) {
    gotoSleep();
  }
}

void gotoSleep() {
  // add some randomness to avoid collisions with multiple devices
  int sleepSecs = SLEEP_SECS + ((uint8_t)RANDOM_REG32/2); 
  Serial.printf("Up for %i ms, going to sleep for %i secs...\n", millis(), sleepSecs); 
  ESP.deepSleep(sleepSecs * 1000000, RF_NO_CAL);
}