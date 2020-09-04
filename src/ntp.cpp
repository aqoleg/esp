#include <LittleFS.h> // just to fool the compiler
#include <WiFiUdp.h>
#include "timer.h"
#include "ntp.h"

WiFiUDP udp;
int connectionsLeft = 10; // 0 if synchronized
bool listening = false; // connection established, listening for incoming message
unsigned long lastConnectMs = 0;

bool synchronizeTime() {
  if (connectionsLeft || listening) {
    return false;
  }
  connectionsLeft = 10;
  return true;
}

void loopNtp() {
  if (listening) {
    if (udp.parsePacket()) {
      // bytes 40-47 of the message contain the transmit timestamp
      // the first 4 bytes of the timestamp are seconds, the last 4 bytes are fraction part
      byte message[48];
      if (udp.read(message, 48) == 48) {
        unsigned long highWord = word(message[40], message[41]);
        unsigned long lowWord = word(message[42], message[43]);
        setTime(highWord << 16 | lowWord);
        listening = false;
        connectionsLeft = 0;
      }
      udp.stop();
    } else if (millis() - lastConnectMs > 20000) {
      udp.stop();
      listening = false;
    }
  } else if (connectionsLeft) {
    if (lastConnectMs > 0 && millis() - lastConnectMs < 10000) {
      return;
    }
    connectionsLeft--;
    lastConnectMs = millis();
    // the message contains 48 bytes, all bytes except the first one are zero
    // bits 0-1 - leap indicator, 0 - no warning
    // bits 2-4 - version, 3
    // bits 5-7 - mode, 3 - client
    byte message[48];
    memset(message, 0, 48);
    message[0] = 0b00011011;
    if (
      udp.begin(2390) &&
      udp.beginPacket("pool.ntp.org", 123) &&
      udp.write(message, 48) == 48 &&
      udp.endPacket()
    ) {
      listening = true;
    } else {
      udp.stop();
    }
  }
}
