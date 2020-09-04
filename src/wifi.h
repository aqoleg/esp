// wifi and led
// wifi is always on, the access point can be turned on/off
// ip of the access point is 192.168.4.1
// the led blinks if esp is not connected as a client

// main setup
void setupWifi();

// main loop
void loopWifi();

// connects to the wifi network
void connectWifi(const char *ssid, const char *password);

// enables/disables the access point
void switchAccessPoint(bool state);

// returns a json array with available networks:
//  [{
//   "ssid":"wifi name",
//   "bssid":"11:11:11:11:11:11", // mac address
//   "rssi":-60, // signal strength, dbm
//   "channel":1,
//   "encription":"wpa2/psk" // open, wep, wpa/psk, wpa2/psk or auto
//  }, ... ]
String scanWifi();

// returns a json object with info about wifi:
//  {
//   "hostname":"hostname",
//   "phy":"11n", // wifi mode: 11b, 11g or 11n
//   "sleep":"modem", // sleep mode: none, light or modem
//   "mode":"sta", // station and access point: off, sta, ap or ap/sta
//   "status":"connected", // idle, no ssid, scan completed, connected, failed, lost or disconnected
//   "mac":"22:22:22:22:22:22", // station mac
//   "ssid":"wifi name",
//   "bssid":"11:11:11:11:11:11", // mac address
//   "rssi":-60, // signal strength, dbm
//   "channel":1,
//   "ip":"192.168.1.2",
//   "subnet":"255.255.255.0",
//   "dns0":"192.168.1.1",
//   "dns1":"(IP unset)",
//   "apMac":"33:33:33:33:33:33", // mac of the access point
//   "apSsid":"access point name",
//   "apIp":"192.168.4.1", // ip of the access point
//   "apClients":0 // number of the clients of the access point
//  }
String getWifiInfo();
