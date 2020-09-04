#include <ArduinoOTA.h>
#include "updater.h"

bool started = false;

void loopUpdater() {
  if (started) {
    ArduinoOTA.handle();
  }
}

bool startUpdater() {
  if (started) {
    return false;
  }
  started = true;
  ArduinoOTA.setPassword("password"); // enter this password in arduino
  ArduinoOTA.begin(); // does not work if call before establishing a connection!
  return true;
}
