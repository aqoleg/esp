#include <ESP8266WiFi.h>
#include "ntp.h"
#include "wifi.h"

static const char host[] PROGMEM = "ktoolu";
bool light; // the state of the led
unsigned long lastBlinkMs = 0;

void turnLight(bool state) {
  light = state;
  digitalWrite(LED_BUILTIN, light ? LOW : HIGH);
}

void setupWifi() {
  pinMode(LED_BUILTIN, OUTPUT);
  turnLight(false);
  WiFi.begin();
  WiFi.hostname(FPSTR(host)); // each time
}

void loopWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!light) {
      turnLight(true);
    }
    loopNtp(); // synchronize time
  } else {
    unsigned long ms = millis();
    if (ms - lastBlinkMs > 700) {
      lastBlinkMs = ms;
      turnLight(!light);
    }
  }
}

void connectWifi(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);
}

void switchAccessPoint(bool state) {
  if (state) {
    // password has minimum 8 symbols
    WiFi.softAP(FPSTR(host), F("password"));
  } else {
    // not softAPdisconnect() because no need to delete password
    WiFi.enableAP(false);
  }
}

String scanWifi() {
  String s = "[";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    if (i > 0) {
      s += ",";
    }
    s += F("{\"ssid\":\"");
    s += WiFi.SSID(i);
    s += F("\",\"bssid\":\"");
    s += WiFi.BSSIDstr(i);
    s += F("\",\"rssi\":");
    s += WiFi.RSSI(i);
    s += F(",\"channel\":");
    s += WiFi.channel(i);
    s += F(",\"encription\":\"");
    switch (WiFi.encryptionType(i)) {
      case ENC_TYPE_NONE:
        s += F("open\"}");
        break;
      case ENC_TYPE_WEP:
        s += F("wep\"}");
        break;
      case ENC_TYPE_TKIP:
        s += F("wpa/psk\"}");
        break;
      case ENC_TYPE_CCMP:
        s += F("wpa2/psk\"}");
        break;
      case ENC_TYPE_AUTO:
        s += F("auto\"}");
        break;
      default:
        s += F("unknown\"}");
    }
  }
  WiFi.scanDelete();
  return s + "]";
}

String getWifiInfo() {
  String s = F("{\"hostname\":\"");
  s += WiFi.hostname();
  s += F("\",\"phy\":\"");
  switch (WiFi.getPhyMode()) {
    case WIFI_PHY_MODE_11B:
      s += "11b";
      break;
    case WIFI_PHY_MODE_11G:
      s += "11g";
      break;
    case WIFI_PHY_MODE_11N:
      s += "11n";
      break;
    default:
      s += WiFi.getPhyMode();
  }
  s += F("\",\"sleep\":\"");
  switch (WiFi.getSleepMode()) {
    case WIFI_NONE_SLEEP:
      s += F("none");
      break;
    case WIFI_LIGHT_SLEEP:
      s += F("light");
      break;
    case WIFI_MODEM_SLEEP:
      s += F("modem");
      break;
    default:
      s += WiFi.getSleepMode();
  }
  s += F("\",\"mode\":\"");
  switch (WiFi.getMode()) {
    case WIFI_OFF:
      s += "off";
      break;
    case WIFI_STA:
      s += "sta";
      break;
    case WIFI_AP:
      s += "ap";
      break;
    case WIFI_AP_STA:
      s += "ap/sta";
      break;
    default:
      s += WiFi.getMode();
  }
  s += F("\",\"status\":\"");
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
      s += F("idle");
      break;
    case WL_NO_SSID_AVAIL:
      s += F("no ssid");
      break;
    case WL_SCAN_COMPLETED:
      s += F("scan completed");
      break;
    case WL_CONNECTED:
      s += F("connected");
      break;
    case WL_CONNECT_FAILED:
      s += F("failed");
      break;
    case WL_CONNECTION_LOST:
      s += F("lost");
      break;
    case WL_DISCONNECTED:
      s += F("disconnected");
      break;
    default:
      s += WiFi.status();
  }
  s += F("\",\"mac\":\"");
  s += WiFi.macAddress();
  s += F("\",\"ssid\":\"");
  s += WiFi.SSID();
  s += F("\",\"bssid\":\"");
  s += WiFi.BSSIDstr();
  s += F("\",\"rssi\":");
  s += WiFi.RSSI();
  s += F(",\"channel\":");
  s += WiFi.channel();
  s += F(",\"ip\":\"");
  s += WiFi.localIP().toString();
  s += F("\",\"subnet\":\"");
  s += WiFi.subnetMask().toString();
  s += F("\",\"dns0\":\"");
  s += WiFi.dnsIP(0).toString();
  s += F("\",\"dns1\":\"");
  s += WiFi.dnsIP(1).toString();
  s += F("\",\"apMac\":\"");
  s += WiFi.softAPmacAddress();
  s += F("\",\"apSsid\":\"");
  s += WiFi.softAPSSID();
  s += F("\",\"apIp\":\"");
  s += WiFi.softAPIP().toString();
  s += F("\",\"apClients\":");
  s += WiFi.softAPgetStationNum();
  return s + "}";
}
