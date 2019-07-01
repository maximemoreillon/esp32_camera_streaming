void wifi_setup() {
  // Settings
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.println(F("[WIFI] Connecting..."));
}

void wifi_connection_manager(){

  static boolean wifi_connected = false;
  static long wifi_disconnected_time = 0;

   // Check for WiFi
  if(WiFi.status() != WL_CONNECTED){
    // Wifi disconnected
   
    if(wifi_connected){
      // Acknowledge disconnection
      wifi_connected = false;
      wifi_disconnected_time = millis();
      Serial.println(F("[WIFI] Disconnected"));
    }

    // Force reconnect if fail
    if(millis() - wifi_disconnected_time > WIFI_CONNECTION_TIMEOUT){
      wifi_disconnected_time = millis();
      Serial.println(F("[WIFI] Connection timeout, resetting connection..."));
      WiFi.disconnect(true);
      wifi_setup();
    }
  }
  else {
    // Wifi connected
    
    if(!wifi_connected){
      // Acknowledge wifi connection
      wifi_connected = true;
      Serial.print(F("[WIFI] connected, IP: "));
      Serial.println(WiFi.localIP());
    }
  }
}
