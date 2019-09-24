#include "esp_camera.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/*
 * BOARD SETTINGS
 * 
 * Board type: ESP32 Wrover Module
 * 
 */

// Getting WIFI_SSID, WIFI_PASSWORD, HOSTNAME, and OTA_PASSWORD from external credentials file
#include "credentials.h";


// Wifi settings
#define WIFI_CONNECTION_TIMEOUT 5000

// Camera pinout
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_init();
  wifi_setup();
  startCameraServer();
  ota_setup();
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("Error setting up MDNS responder!");
    while(1) {
        delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  MDNS.addService("http", "tcp", 80);
  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1); // flip vertically
  //s->set_brightness(s, 1); //+ve to increase, -ve to decrease
  //s->set_saturation(s, -2); //+ve to increase, -ve to lower
  s->set_hmirror(s, 1); // mirror horizontally
}

void loop() {
  wifi_connection_manager();
  ArduinoOTA.handle();
}
