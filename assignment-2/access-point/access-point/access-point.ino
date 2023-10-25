#include <ESP8266WiFi.h>

// Access point configuration
const char *ssid = "AP-ESP8266";
const char *password = "987654321";

// Access point definitions (local IP, gateway, subnet)
IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

// Control variable and initialization
boolean waitingDHCP=false;
char last_mac[18];
int selectedChannel = 1; // Default channel

// Manage incoming device connection on ESP access point
void onNewStation(WiFiEventSoftAPModeStationConnected sta_info) {
  Serial.println("New Station :");
  sprintf(last_mac,"%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(sta_info.mac));
  Serial.printf("MAC address = %s\n",last_mac);
  Serial.printf("Id = %d\n", sta_info.aid);
  waitingDHCP=true;
}

// Setup
void setup()
{
  // Control variable for event subscription
  static WiFiEventHandler e1;

  // Serial baudrate and initialization
  Serial.begin(115200);
  delay(1000);
  Serial.println();

  // Configure Wi-Fi channel
  Serial.println("Enter the desired Wi-Fi channel (1-13):");
  while (!Serial.available()) {
    delay(10);
  }

  // Setting access point parameters
  selectedChannel = Serial.parseInt();
  Serial.print("Setting Wi-Fi channel to ");
  Serial.println(selectedChannel);

  WiFi.mode(WIFI_AP_STA);
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid,password, selectedChannel) ? "Ready" : "Failed!");
  
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  // Printing the channel and SSID
  Serial.println();
  Serial.print("AP Channel: ");
  Serial.println(WiFi.channel());
  Serial.print("AP SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Event subscription
  e1 = WiFi.onSoftAPModeStationConnected(onNewStation);
}

// Main loop
void loop() {
  // DHCP handling
  if (waitingDHCP) {
    String cb;
    // Control if there is valid device connected
    if (deviceIP(last_mac,cb)) {
      Serial.println("Ip address = " + cb);
    } else {
      Serial.println("Problem during ip address request = " + cb);
    }
  }

  delay(2000);
}

// Function to print new devices connected in the AP
boolean deviceIP(char* mac_device, String &cb) {
  struct station_info *station_list = wifi_softap_get_station_info();

  // Control loop to print new device connections
  while (station_list != NULL) {
    char station_mac[18] = {0}; sprintf(station_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));
    String station_ip = IPAddress((&station_list->ip)->addr).toString();

    // Cotrolling if the device list is empty
    if (strcmp(mac_device,station_mac)==0) {
      waitingDHCP=false;
      cb = station_ip;
      return true;
    } 
    station_list = STAILQ_NEXT(station_list, next);
  }

  // Messaging indicating that DHCP not ready or bad MAC address
  wifi_softap_free_station_info();
  cb = "DHCP not ready or bad MAC address";
  return false;
}
