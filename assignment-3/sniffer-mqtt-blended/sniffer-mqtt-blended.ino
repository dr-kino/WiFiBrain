#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "./esppl_functions.h"

#define MY_STATION_MODE 1
#define MY_SNIFFER_MODE 0
uint8_t operation_mode = MY_STATION_MODE; 

#define PHONE_DEVICE 0
#define RANDO_DEVICE 1
uint8_t device = RANDO_DEVICE;
bool data_received = false;
bool device_detected = false;
bool send_data = false;

/**
  MQTT Code and Functions
 */
// Update these with values suitable for your network.
const char* ssid = "my-ssid";
const char* password = "ssi-pswd";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') { // MAC 1 - Phone
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    //digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    Serial.print("Sniffer will search for MAC 1 - Phone MAC Address...");
    device = PHONE_DEVICE;
    data_received = true;
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else if ((char)payload[0] == '2') { // MAC 2 - Randomized
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    //digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    Serial.print("Sniffer will search for MAC 2 - Randomized MAC Address...");
    device = RANDO_DEVICE;
    data_received = true;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println(" connected");
      // Once connected, publish an announcement...
      client.publish("scmdevice/mac", "MQTT Server is Connected");
      // ... and resubscribe
      client.subscribe("device/led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/**
  Sniffer Code/Functions
 */
/*  Define you friend's list size here
 How many MAC addresses are you tracking?
 */
#define LIST_SIZE 2
/*
 * This is your friend's MAC address list
 Format it by taking the mac address aa:bb:cc:dd:ee:ff 
 and converting it to 0xaa,0xbb,0xcc,0xdd,0xee,0xff
 */
uint8_t friendmac[LIST_SIZE][ESPPL_MAC_LEN] = {
   {0x80, 0x54, 0x9c, 0x5d, 0x59, 0x01}
  ,{0x32, 0x98, 0x32, 0x1e, 0xef, 0xe3}
};
/*
 * This is your friend's name list
 * put them in the same order as the MAC addresses
 */
String friendname[LIST_SIZE] = {
   "Phone MAC address"
  ,"Randomized MAC address"
};

/* You cannot use a time delay here to keep the LED on, so will need to use ratio of 
detected packets to overall packets to keep LED on for longer. If you try to use a 
delay to keep the light on for long enough to be useful, the watchdog timer kills the 
process and it dies */
int cooldown = 0; /* This variable will be a cooldown timer to keep the LED on for longer, we'll set it to 1000 if we
detect a packet from a device with a MAC address on the list, and then keep the LED on till we get 1000 packets that 
are NOT from any device on the list. */

bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

void cb(esppl_frame_info *info) {
  //for (int i=0; i<LIST_SIZE; i++) {
    if (maccmp(info->sourceaddr, friendmac[device]) || maccmp(info->receiveraddr, friendmac[device])) {
      Serial.printf("\n%s is here! :)", friendname[device].c_str());
      device_detected = true;
      digitalWrite(BUILTIN_LED, LOW); // (RC) Adapted
      cooldown = 100; // here we set it to 1000 if we detect a packet that matches our list
    }
    else { // this is for if the packet does not match any we are tracking
      if (cooldown > 0) {
        cooldown--; } //subtract one from the cooldown timer if the value of "cooldown" is more than one
      else { // If the timer is at zero, then run the turnoff function to turn off any LED's that are on.
        digitalWrite(BUILTIN_LED, HIGH); // (RC) Adapted
      } 
    } 
  //} 
}

/**
  Overall Functions (Reset Wi-Fi)
 */
void reset_wifi(uint8_t current_mode) { // (current_mode == 0) -> MQTT / (current_mode == 1) -> Sniffer
  Serial.println("Reseting WiFi Interface...");

  if (current_mode == MY_STATION_MODE) { // Reset for STATION mode (WiFi) 
    WiFi.disconnect(true);
  }
  else if (current_mode == MY_SNIFFER_MODE) { // Reset for SNIFFER mode
    wifi_promiscuous_enable(0);
    wifi_station_disconnect();
    WiFi.disconnect(true);
  }

  // Wait a moment for the disconnect to complete
  delay(1000);
}

/**
  Overall Functions (Intial Setup and Main Loop)
 */
void setup() {
  delay(500);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  //setup_wifi();
  //client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);
}

void loop() {

  if (operation_mode == MY_STATION_MODE) {
    Serial.println("WiFi in Station Mode...");
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    while (true) {
      if (!client.connected()) {
        reconnect();
      }
      client.loop();

      unsigned long now = millis();
      if (now - lastMsg > 2000) {
        //lastMsg = now;
        //value = analogRead(A0)*0.32 + 3;
        //snprintf (msg, MSG_BUFFER_SIZE, "Temperature is: %ld", value);
        //Serial.print("Publish message: ");
        //Serial.println(msg);
        //client.publish("device/temp", msg);
      }
      if (send_data == true) {
        send_data = false;
        Serial.print("Publish message: MAC Address");
        if (device == PHONE_DEVICE) {
          client.publish("scmdevice/mac", "PHONE MAC Detected");
        }
        else if (device == RANDO_DEVICE) {
          client.publish("scmdevice/mac", "RANDOM MAC Detected");
        }
        
      }

      if (data_received == true) {
        data_received = false;
        break;
      }
    }
    Serial.println("\nLeaving Station Mode...");
    operation_mode = MY_SNIFFER_MODE;
    reset_wifi(MY_STATION_MODE);
  }
  else if (operation_mode == MY_SNIFFER_MODE) {
    Serial.println("WiFi in Sniffer Mode...");
    esppl_init(cb);
    delay(100);
    esppl_sniffing_start();
    while(true) {
      for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
        esppl_set_channel(i);
        while (esppl_process_frames()) {
          //
        }
      }
      if (device_detected == true) {
        device_detected = false;
        send_data = true;
        break;
      }
    }
    Serial.println("\nLeaving Sniffer Mode...");
    operation_mode = MY_STATION_MODE;
    reset_wifi(MY_SNIFFER_MODE);   
  }
}
