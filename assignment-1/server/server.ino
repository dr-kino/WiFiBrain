#include <ESP8266WiFi.h>

#define MY_SSID "my-ssid"
#define MY_WIFI_PSWD "my-pswd"

const char* ssid = MY_SSID;             // Wi-Fi network SSID
const char* password = MY_WIFI_PSWD;    // Wi-Fi network password
const int ledPin = D4;                  // LED Pin
const int RSSI_MAX =-50;                // define maximum straighten of signal in dBm
const int RSSI_MIN =-100;               // define minimum strength of signal in dBm

WiFiServer server(25);                  // Create a server on port 25

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
    Serial.println(" networks found in total");

    Serial.println("Printing interested RSSI:");

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

  // Connect to WiFi
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

  // Start the server
  server.begin();

  digitalWrite(ledPin, HIGH); // Turn the LED off
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected");
    client.println("Hello, client! This is the server.");

    while (client.connected()) {
      // Send data to the client while it's connected
      if (Serial.available() > 0) {
        String teststr = Serial.readString();
        client.println(teststr);
        //delay(1000); // You can adjust the sending interval
      }
      // Read data from the client
      while (client.available() > 0) {
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
    }

    // The client has disconnected
    Serial.println("Client disconnected");
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
