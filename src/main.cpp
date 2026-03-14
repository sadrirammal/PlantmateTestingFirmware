#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

// Sensor and relay pins
const int rlyPins[] = {4, 16, 17, 18, 19};  //confirmed
const int ledPins[] = {2, 15, 13}; // {white, blue, red}


const String api = "https://api.plantup.io";

void performOTA()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Checking for firmware update...");
        HTTPClient http;
        http.begin(api + "/device/growmate/firmware");
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            int contentLength = http.getSize();
            bool canBegin = Update.begin(contentLength);

            if (canBegin)
            {
                WiFiClient *client = http.getStreamPtr();
                int written = 0;
                uint8_t buff[128] = { 0 };

                while (http.connected() && written < contentLength) {
                    Serial.println("Updating");

                    size_t available = client->available();
                    if (available) {
                        size_t toRead = available;
                        if (toRead > sizeof(buff)) toRead = sizeof(buff);
                        int bytesRead = client->read(buff, toRead);
                        if (bytesRead > 0) {
                            Update.write(buff, bytesRead);
                            written += bytesRead;
                            vTaskDelay(1); // 1 Let watchdog breathe
                        }
                    }
                }

                if (written == contentLength)
                {
                    Serial.println("Update written successfully.");
                    if (Update.end())
                    { 
                        Serial.println("Update completed. Rebooting...");
                        ESP.restart();
                    }
                    else
                    {
                        Serial.printf("Update failed. Error #: %d\n", Update.getError());
                    }
                }
                else
                {
                    Serial.println("Update failed: Written bytes mismatch.");
                }
            }
            else
            {
                Serial.println("Update failed: Not enough space for OTA.");
            }
        }
        else
        {
            Serial.printf("Failed to download firmware. HTTP response code: %d\n", httpCode);
        }

        // Close the HTTP connection
        http.end();
    }
    else
    {
        Serial.println("Could not do OTA since we weren't connected to wifi.");
    }
}

bool setupWiFi(const char *inSsid, const char *inPassword)
{
    // Validate input parameters
    if (inSsid == nullptr || inPassword == nullptr || strlen(inSsid) == 0)
    {
        Serial.println("Error: SSID is null or empty");
        return false;
    }
    
    Serial.printf("Attempting to connect to SSID: %s\n", inSsid);
    
    // Disconnect if already connected to a different network
    if (WiFi.status() == WL_CONNECTED)
    {
        String currentSSID = WiFi.SSID();
        if (currentSSID.equals(inSsid))
        {
            Serial.println("Already connected to this WiFi network");
            return true;
        }
        
        Serial.println("Disconnecting from current network to connect to new one");
        WiFi.disconnect();
        delay(1000); // Give it time to disconnect
    }
    
    // Set station mode and begin connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(inSsid, inPassword);
    
    // Wait for connection with timeout
    const unsigned long timeout = 15000; // 15 seconds timeout
    unsigned long startTime = millis();
    int dots = 0;
    
    while (WiFi.status() != WL_CONNECTED)
    {
        // Check for timeout
        if (millis() - startTime >= timeout)
        {
            Serial.println("\nWiFi Connection timed out");
            WiFi.disconnect();
            return false;
        }
        
        // Check for connection failure
        if (WiFi.status() == WL_CONNECT_FAILED)
        {
            Serial.println("\nWiFi connection failed - likely incorrect password");
            WiFi.disconnect();
            return false;
        }
        
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    
    // Connection successful - print network details
    Serial.println("\nConnected to WiFi successfully!");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signal Strength (RSSI): %d dBm\n", WiFi.RSSI());
    
    return true;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  for (int pin : rlyPins)
    pinMode(pin, OUTPUT);
  for (int pin : ledPins)
    pinMode(pin, OUTPUT);

  for (int pin : rlyPins)
    digitalWrite(pin, LOW);
  for (int pin : ledPins)
    digitalWrite(pin, HIGH); // LED pins need to be set high to be off
}

void loop()
{
  setupWiFi("wifi name", "passwort");

  const int numberOfTestRuns = 1;
  const int pumpRunTime = 3000; 

  for(int i = 0; i < numberOfTestRuns; i++){
    for(int rly : rlyPins){
      digitalWrite(rly, HIGH);
      delay(pumpRunTime);
      digitalWrite(rly, LOW);
      delay(100);
    }
  }

  performOTA();
}
