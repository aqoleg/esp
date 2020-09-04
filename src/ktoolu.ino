#include "updater.h"
#include "serial.h"
#include "wifi.h"
#include "server.h"
#include "timer.h"

void setup() {
  setupSerial();
  setupWifi();
  setupServer();
  setupTimer();
}

void loop() {
  loopUpdater();
  loopSerial();
  loopWifi();
  loopServer();
  loopTimer();
}
