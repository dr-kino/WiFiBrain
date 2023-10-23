#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "AC-ESP8266";       // SSID of the Access Point you want to connect to
const char* password = "987654321"; // Password of the Access Point
const int webServerPort = 80;

ESP8266WebServer server(webServerPort);

float temperature = 0.0; // Initialize temperature
float humidity = 0.0;    // Initialize humidity

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to the Access Point
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

  // Set up the web server route
  server.on("/", HTTP_GET, []() {
    String response = "Temperature: " + String(temperature) + char(176) + "C<br>Humidity: " + String(humidity) + " %";
    server.send(200, "text/html", response);
  });

  server.begin();
}

void loop() {
  server.handleClient();

  // Check for new data on the serial interface
  while (Serial.available() > 0) {
    char receivedChar = Serial.read();
    static String inputString = "";
    if (receivedChar == '\n') {
      // Split the received data into temperature and humidity values using a delimiter
      int delimiterPos = inputString.indexOf(',');
      if (delimiterPos != -1) {
        temperature = inputString.substring(0, delimiterPos).toFloat();
        humidity = inputString.substring(delimiterPos + 1).toFloat();
      }
      inputString = "";
    } else {
      inputString += receivedChar;
    }
  }
}