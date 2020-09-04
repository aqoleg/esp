#include <LittleFS.h> // just to fool the compiler
#include "wifi.h"
#include "serial.h"

// use serial interface to communicate with relays and buttons
// the message contains 4 bytes {byte0, byte1, state, byte3}
// the first to right bit of the state is the state of the relay1,
// the second to right bit is the state of the relay0
static const byte byte0 = 0xA0;
static const byte byte1 = 0x04;
static const byte byte3 = 0xA1;
byte state = 0;
int pushN = 0;
unsigned long previousPushMs = 0;

// reads serial, returns true if something happend
bool read() {
  if (Serial.available() >= 4) {
    if (Serial.read() == byte0 && Serial.read() == byte1) {
      byte byte2 = Serial.read() & 0b11; // cut unknown bits
      if (Serial.read() == byte3) {
        if (byte2 != state) { // the relay button was pushed
          state = byte2; // synchronizes the state
        } else { // the main button was pushed
          unsigned long ms = millis();
          if (ms - previousPushMs >= 70) { // 'debounce'
            previousPushMs = ms;
            pushN++;
          }
        }
        return true;
      }
    }
    // error, find the end of the messsage
    while (Serial.available()) {
      if (Serial.read() == byte3) {
        break;
      }
    }
  }
  return false;
}

// writes serial to switch relays
void write() {
  Serial.write(byte0);
  Serial.write(byte1);
  Serial.write(state);
  Serial.write(byte3);
  Serial.flush();
}

void setupSerial() {
  Serial.begin(19200);
  write(); // to synchronize the state after restart; cannot read if did not write before
}

void loopSerial() {
  if (!read() && pushN > 0 && millis() - previousPushMs >= 500) {
    switch (pushN) {
      case 1:
        state ^= 0b10;
        write();
        break;
      case 2:
        state ^= 0b1;
        write();
        break;
      case 3:
        switchAccessPoint(true);
        break;
      default:
        switchAccessPoint(false);
    }
    pushN = 0;
  }
}

void switchRelay(bool isRelay0, bool relayState) {
  byte mask = isRelay0 ? 0b10 : 0b1;
  if (relayState) {
    state |= mask;
  } else {
    state &= ~mask;
  }
  write();
}

void switchRelays(bool relay0State, bool relay1State) {
  state = relay0State ? 0b10 : 0;
  if (relay1State) {
    state |= 0b1;
  }
  write();
}

String getRelaysState() {
  String s = F("{\"relay0\":\"");
  s += state & 0b10 ? "on" : "off";
  s += F("\",\"relay1\":\"");
  s += state & 0b1 ? "on" : "off";
  return s + "\"}";
}
