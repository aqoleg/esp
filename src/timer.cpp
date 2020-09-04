#include <LittleFS.h>
#include <FS.h>
#include "serial.h"
#include "timer.h"

static const char fileName[] PROGMEM = "/config.txt";
static const unsigned long daySec = 86400;
bool run = false; // timer state
// events
int eventsN = 0; // total amount of the events
unsigned long eventsMs[20]; // time of the event in ms since 00:00:00Z utc
bool eventsIsRelay0[20]; // relay 0 or 1
bool eventsRelayState[20]; // turn relay on or off
// next event to fire
int nextEventN = 0; // if the same as eventsN, the next event is the start of a new day
unsigned long nextEventMs; // time of the next event in ms since 00:00:00Z utc
// millis() = startMs + timeMs
unsigned long startMs = 0; // timestamp millis() of the last 00:00:00Z utc in ms
unsigned long timeMs = 0; // time since the last 00:00:00Z utc in ms

// loads data from /config.txt
void load() {
  File file = LittleFS.open(FPSTR(fileName), "r");
  if (!file) {
    file.close();
    return;
  }
  String s = file.readString();
  file.close();

  eventsN = 0;
  run = s.charAt(1) == 'n'; // on/off
  int endOfLineIndex;
  int commaIndex = 0;
  do {
    endOfLineIndex = s.indexOf("\n", commaIndex);
    commaIndex = s.indexOf(",", endOfLineIndex);
    if (commaIndex < 0) {
      return;
    }
    eventsMs[eventsN] = (s.substring(endOfLineIndex + 1, commaIndex).toInt() % daySec) * 1000;
    if (eventsN > 0 && eventsMs[eventsN - 1] > eventsMs[eventsN]) {
      return;
    }
    eventsIsRelay0[eventsN] = s.charAt(commaIndex + 1) != '1';
    eventsRelayState[eventsN] = s.charAt(commaIndex + 4) == 'n'; // ,1,on
  } while (++eventsN < 20);
}

// saves data to /config.txt, returns true if successful
bool save() {
  String s = run ? "on" : "off";
  for (int i = 0; i < eventsN; i++) {
    s += "\n";
    s += eventsMs[i] / 1000;
    s += eventsIsRelay0[i] ? ",0" : ",1";
    s += eventsRelayState[i] ? ",on" : ",off";
  }
  byte bytes[s.length() + 1]; // one byte for the end of string
  s.getBytes(bytes, s.length() + 1);

  File file = LittleFS.open(FPSTR(fileName), "w");
  bool result = file.write(bytes, s.length()) == s.length();
  file.close();
  return result;
}

// synchronizes the state of the relays after any change
void set() {
  bool relay0State = false;
  bool relay1State = false;
  nextEventN = 0;
  while (nextEventN < eventsN) {
    if (timeMs < eventsMs[nextEventN]) { // this event is the next
      break;
    }
    if (eventsIsRelay0[nextEventN]) {
      relay0State = eventsRelayState[nextEventN];
    } else {
      relay1State = eventsRelayState[nextEventN];
    }
    nextEventN++;
  }
  nextEventMs = nextEventN == eventsN ? daySec * 1000 : eventsMs[nextEventN];
  if (run) {
    switchRelays(relay0State, relay1State);
  }
}

void setupTimer() {
  load();
  nextEventMs = nextEventN == eventsN ? daySec * 1000 : eventsMs[nextEventN];
}

void loopTimer() {
  timeMs = millis() - startMs; // rollover-safe
  if (timeMs >= nextEventMs) {
    if (nextEventN == eventsN) { // start a new day
      startMs += daySec * 1000;
      nextEventN = 0;
    } else {
      if (run) {
        switchRelay(eventsIsRelay0[nextEventN], eventsRelayState[nextEventN]);
      }
      nextEventN++;
    }
    nextEventMs = nextEventN == eventsN ? daySec * 1000 : eventsMs[nextEventN];
  }
}

void setTime(unsigned long sec) {
  timeMs = (sec % daySec) * 1000;
  startMs = millis() - timeMs; // rollover-safe
  set();
}

bool switchTimer(bool state) {
  run = state;
  set();
  return save();
}

bool addEvent(unsigned long sec, bool isRelay0, bool relayState) {
  if (eventsN == 20) {
    return false;
  }
  unsigned long ms = (sec % daySec) * 1000;
  // find a position for the new event
  int pos = eventsN - 1; // start from the last event
  while (pos >= 0) {
    if (eventsMs[pos] <= ms) { // this event is not later than the new one
      break;
    }
    pos--;
  }
  pos++; // the next position is for the new event
  // move later events to the next position
  for (int i = eventsN - 1; i >= pos; i--) {
    eventsMs[i + 1] = eventsMs[i];
    eventsIsRelay0[i + 1] = eventsIsRelay0[i];
    eventsRelayState[i + 1] = eventsRelayState[i];
  }
  eventsMs[pos] = ms;
  eventsIsRelay0[pos] = isRelay0;
  eventsRelayState[pos] = relayState;
  eventsN++;
  set();
  return save();
}

bool editEvent(int eventN, unsigned long sec, bool isRelay0, bool relayState) {
  if (eventN < 0 || eventN >= eventsN) {
    return false;
  }
  unsigned long ms = (sec % daySec) * 1000;
  // find a position for the edited event
  int pos = eventN;
  if (eventN > 0 && eventsMs[eventN - 1] > ms) {
    pos = eventN - 2; // pos (eventN - 1) is checked
    while (pos >= 0) {
      if (eventsMs[pos] <= ms) { // this event not later than the edited one
        break;
      }
      pos--;
    }
    pos++; // the next position is for the edited event
    // move events between current position and new position to the next position
    for (int i = eventN - 1; i >= pos; i--) {
      eventsMs[i + 1] = eventsMs[i];
      eventsIsRelay0[i + 1] = eventsIsRelay0[i];
      eventsRelayState[i + 1] = eventsRelayState[i];
    }
  } else if (eventN < eventsN - 1 && ms > eventsMs[eventN + 1]) {
    pos = eventN + 2; // pos (eventN + 1) is checked
    while (pos < eventsN) {
      if (ms <= eventsMs[pos]) { // this event not earlier than the edited one
        break;
      }
      pos++;
    }
    pos--; // the previous position is for the edited event
    // move events between current position and new position to the previous position
    for (int i = eventN + 1; i <= pos; i++) {
      eventsMs[i - 1] = eventsMs[i];
      eventsIsRelay0[i - 1] = eventsIsRelay0[i];
      eventsRelayState[i - 1] = eventsRelayState[i];
    }
  }
  eventsMs[pos] = ms;
  eventsIsRelay0[pos] = isRelay0;
  eventsRelayState[pos] = relayState;
  set();
  return save();
}

bool deleteEvent(int eventN) {
  if (eventN < 0 || eventN >= eventsN) {
    return false;
  }
  // move later events to the previous position
  for (int i = eventN + 1; i < eventsN; i++) {
    eventsMs[i - 1] = eventsMs[i];
    eventsIsRelay0[i - 1] = eventsIsRelay0[i];
    eventsRelayState[i - 1] = eventsRelayState[i];
  }
  eventsN--;
  set();
  return save();
}

String getEvents() {
  String s = F("{\"run\":\"");
  s += run ? "on" : "off";
  s += F("\",\"events\":[");
  for (int i = 0; i < eventsN; i++) {
    if (i > 0) {
      s += ",";
    }
    s += F("{\"time\":");
    s += eventsMs[i] / 1000;
    s += F(",\"relay\":");
    s += eventsIsRelay0[i] ? "0" : "1";
    s += F(",\"state\":\"");
    s += eventsRelayState[i] ? "on" : "off";
    s += "\"}";
  }
  return s + "]}";
}

String getTime() {
  String s = F("{\"time\":");
  s += timeMs / 1000;
  s += F(",\"nextEvent\":");
  if (nextEventN == eventsN) {
    s += eventsN == 0 ? "-1" : "0";
  } else {
    s += nextEventN;
  }
  return s + "}";
}
