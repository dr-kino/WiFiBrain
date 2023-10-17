#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define MY_SSID "telenet-D198A4A"
#define MY_WIFI_PSWD "rU7Nusacerft"

const char* ssid = MY_SSID;                   // Wi-Fi network SSID
const char* password = MY_WIFI_PSWD;          // Wi-Fi network password
const int ledPin = D4;                        // LED Pin
const char* serverAddress = "192.168.0.206";  // IP address of the first NodeMCU server
const int RSSI_MAX =-50;                      // define maximum straighten of signal in dBm
const int RSSI_MIN =-100;                     // define minimum strength of signal in dBm

WiFiClient client;

void setup() {
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);
  delay(10);

  // RSSI
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  int openNetworkCount=0;
  Serial.println("\n");
  Serial.println("Wifi scan started");
  int n = WiFi.scanNetworks();
  Serial.println("Wifi scan ended");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");

    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i) == MY_SSID) {
        openNetworkCount++;
        Serial.print(openNetworkCount);
        Serial.print(") ");
        Serial.print(WiFi.SSID(i));// SSID
                  
        Serial.print(WiFi.RSSI(i));//Signal strength in dBm  
        Serial.print("dBm (");
        Serial.print(dBmtoPercentage(WiFi.RSSI(i)));//Signal strength in %  
        Serial.print("%) ");
      }
      delay(10);
    }
    Serial.println("\n");
  }

  // Connect to Wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Serial.println(WiFi.localIP());

  Serial.print("802.11 variant: ");
  Serial.println(WiFi.getPhyMode());
  Serial.print("Wi-Fi channel: ");
  Serial.println(WiFi.channel());

  // Connect to the server
  if (client.connect(serverAddress, 25)) { // Use the same port as the server
    Serial.println("Connected to server");
  }

  digitalWrite(ledPin, HIGH); // Turn the LED off
}

void loop() {
  if (client.connected()) {
    // Write data to the server
    if (Serial.available() > 0) {
      String teststr = Serial.readString();
      client.println(teststr);
    }
    // Read data from the server
    while (client.available()) {
      String teststr = client.readString();
      Serial.println(teststr);
      teststr.trim();
      if (teststr == "ON") {
        digitalWrite(ledPin, LOW); // Turn the LED on
      }
      else if (teststr == "OFF") {
        digitalWrite(ledPin, HIGH); // Turn the LED off
      }
    }
  } else {
    Serial.println("Disconnected from server");
    delay(5000);  // Wait for a few seconds before attempting to reconnect
    setup();
  }
}

/*
 * Written by Ahmad Shamshiri
  * with lots of research, this sources was used:
 * https://support.randomsolutions.nl/827069-Best-dBm-Values-for-Wifi 
 * This is approximate percentage calculation of RSSI
 * Wifi Signal Strength Calculation
 * Written Aug 08, 2019 at 21:45 in Ajax, Ontario, Canada
 */
int dBmtoPercentage(int dBm)
{
  int quality;
    if(dBm <= RSSI_MIN)
    {
        quality = 0;
    }
    else if(dBm >= RSSI_MAX)
    {  
        quality = 100;
    }
    else
    {
        quality = 2 * (dBm + 100);
   }

     return quality;
}//dBmtoPercentage
