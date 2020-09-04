#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <FS.h>
#include "updater.h"
#include "serial.h"
#include "wifi.h"
#include "ntp.h"
#include "timer.h"
#include "server.h"

using namespace mime;

static const char linksPage[] PROGMEM =
"<!DOCTYPE HTML>\n"
"<html lang=\"en\">\n"
" <head>\n"
"  <title>links</title>\n"
"  <meta charset=\"utf-8\" />\n"
" </head>\n"
" <body>\n"
"  <p><a href=\"/links\">/links</a></p>\n"
"  <p><a href=\"/about\">/about</a></p>\n"
"  <p><a href=\"/esp\">/esp</a></p>\n"
"  <p><a href=\"/restart\">/restart</a></p>\n"
"  <p><a href=\"/flash\">/flash</a></p>\n"
"  <p><a href=\"/relays\">/relays</a></p>\n"
"  <p><a href=\"/switch?relay0=on&relay1=on\">/switch?relay0=on&relay1=on</a></p>\n"
"  <p><a href=\"/on0\">/on0</a></p>\n"
"  <p><a href=\"/off0\">/off0</a></p>\n"
"  <p><a href=\"/on1\">/on1</a></p>\n"
"  <p><a href=\"/off1\">/off1</a></p>\n"
"  <p><a href=\"/scan\">/scan</a></p>\n"
"  <p><a href=\"/wifi\">/wifi</a></p>\n"
"  <p>/conn?ssid=ssid&password=password</p>\n"
"  <p><a href=\"/ap?ap=off\">/ap?ap=off</a></p>\n"
"  <p><a href=\"/sync\">/sync</a></p>\n"
"  <p><a href=\"/time\">/time</a></p>\n"
"  <p><a href=\"/events\">/events</a></p>\n"
"  <p><a href=\"/settime?time=3600\">/settime?time=3600</a></p>\n"
"  <p><a href=\"/run?run=off\">/run?run=off</a></p>\n"
"  <p><a href=\"/create?time=3600&relay=1&state=on\">/create?time=3600&relay=1&state=on</a></p>\n"
"  <p><a href=\"/edit?eventN=1&time=3600&relay=1&state=on\">/edit?eventN=1&time=3600&relay=1&state=on</a></p>\n"
"  <p><a href=\"/remove?eventN=1\">/remove?eventN=1</a></p>\n"
"  <p><a href=\"/files\">/files</a></p>\n"
"  <p><a href=\"/upload\">/upload</a></p>\n"
"  <p><a href=\"/delete\">/delete</a></p>\n"
"  <p><a href=\"/config.txt\">/config.txt</a></p>\n"
"  <p><a href=\"/index.html\">/index.html</a></p>\n"
"  <p><a href=\"/timer.html\">/timer.html</a></p>\n"
"  <p><a href=\"/wifi.html\">/wifi.html</a></p>\n"
"  <p><a href=\"/ktoolu.svg\">/ktoolu.svg</a></p>\n"
" </body>\n"
"</html>\n";
static const char aboutPage[] PROGMEM =
"<!DOCTYPE HTML>\n"
"<html lang=\"en\">\n"
" <head>\n"
"  <title>about</title>\n"
"  <meta charset=\"utf-8\" />\n"
" </head>\n"
" <body>\n"
"  <a href=\"https://aqoleg.github.io/ktoolu\">aqoleg.github.io/ktoolu</a>\n"
"  <script>\n"
"   window.location.replace('https://aqoleg.github.io/ktoolu');\n"
"  </script>\n"
" </body>\n"
"</html>\n";
static const char deletePage[] PROGMEM =
"<!DOCTYPE HTML>\n"
"<html lang=\"en\">\n"
" <head>\n"
"  <title>delete file</title>\n"
"  <meta charset=\"utf-8\" />\n"
" </head>\n"
" <body>\n"
"  <form method=\"post\">\n"
"   <input type=\"text\" name=\"file\" />\n"
"   <button type=\"submit\">delete</button>\n"
"  </form>\n"
" </body>\n"
"</html>\n";
static const char uploadPage[] PROGMEM =
"<!DOCTYPE HTML>\n"
"<html lang=\"en\">\n"
" <head>\n"
"  <title>upload file</title>\n"
"  <meta charset=\"utf-8\" />\n"
" </head>\n"
" <body>\n"
"  <form method=\"post\" enctype=\"multipart/form-data\">\n"
"   <input type=\"file\" name=\"file\" />\n"
"   <button type=\"submit\">upload</button>\n"
"  </form>\n"
" </body>\n"
"</html>\n";

ESP8266WebServer server(80);
File uploadingFile;

String getEspInfo() {
  String s = F("{\"sketchSize\":");
  s += ESP.getSketchSize();
  s += F(",\"freeHeap\":");
  s += ESP.getFreeHeap();
  s += F(",\"heapFragmentation\":");
  s += ESP.getHeapFragmentation();
  s += F(",\"ms\":");
  s += millis();
  s += F(",\"cycleCount\":");
  s += ESP.getCycleCount();
  s += F(",\"resetReason\":\"");
  s += ESP.getResetReason();
  return s + "\"}";
}

String getFilesInfo() {
  FSInfo fsInfo;
  LittleFS.info(fsInfo);
  String s = F("{\"totalBytes\":");
  s += fsInfo.totalBytes;
  s += F(",\"usedBytes\":");
  s += fsInfo.usedBytes;
  s += F(",\"files\":[");
  Dir dir = LittleFS.openDir("/");
  bool first = true;
  while (dir.next()) {
    if (first) {
      first = false;
    } else {
      s += ',';
    }
    s += F("{\"name\":\"");
    s += dir.fileName();
    s += F("\",\"size\":");
    s += dir.fileSize();
    s += "}";
  }
  return s + "]}";
}

void sendError() {
  server.send(500, FPSTR(mimeTable[txt].mimeType), F("error"));
}

void uploadFile() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String path = upload.filename;
    if (!path.startsWith("/")) {
      path = "/" + path;
    }
    uploadingFile = LittleFS.open(path, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE && uploadingFile) {
    uploadingFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadingFile && uploadingFile.size() == upload.totalSize) {
      server.send(200, FPSTR(mimeTable[txt].mimeType), F("uploaded"));
    } else {
      LittleFS.remove(upload.filename);
      sendError();
    }
    uploadingFile.close();
  }
}

void deleteFile() {
  String path = server.arg("file");
  if (path.length() == 0) {
    server.send(200, FPSTR(mimeTable[html].mimeType), FPSTR(deletePage));
    return;
  }
  if (!path.startsWith("/")) {
    path = "/" + path;
  }
  if (LittleFS.remove(path)) {
    server.send(200, FPSTR(mimeTable[txt].mimeType), F("deleted"));
  } else {
    sendError();
  }
}

void readFile() {
  String path = server.uri();
  if (path.equals("/")) {
    path = "/index.html";
  } else if (path.endsWith("/")) {
    path = path.substring(0, path.length() - 1);
  }
  File file = LittleFS.open(path, "r");
  if (file) {
    server.streamFile(file, getContentType(path));
  } else {
    server.send(404, FPSTR(mimeTable[txt].mimeType), F("file not found"));
  }
  file.close();
}

void setupServer() {
  LittleFS.begin();
  server.on("/links", []() {
    server.send(200, FPSTR(mimeTable[html].mimeType), FPSTR(linksPage));
  });
  server.on("/about", []() {
    server.send(200, FPSTR(mimeTable[html].mimeType), FPSTR(aboutPage));
  });
  server.on("/esp", []() {
    server.send(200, FPSTR(mimeTable[json].mimeType), getEspInfo());
  });
  server.on("/restart", []() {
    server.send(200, FPSTR(mimeTable[txt].mimeType), F("restarting"));
    delay(300);
    ESP.restart();
  });
  server.on("/flash", []() {
    server.send(200, FPSTR(mimeTable[txt].mimeType), startUpdater() ? "ok" : "already");
  });
  server.on("/relays", []() {
    server.send(200, FPSTR(mimeTable[json].mimeType), getRelaysState());
  });
  server.on("/switch", []() {
    bool relay0State = server.arg("relay0").equals("on");
    bool relay1State = server.arg("relay1").equals("on");
    switchRelays(relay0State, relay1State);
    server.send(200, FPSTR(mimeTable[json].mimeType), getRelaysState());
  });
  server.on("/on0", []() {
    switchRelay(true, true);
    server.send(200, FPSTR(mimeTable[json].mimeType), getRelaysState());
  });
  server.on("/off0", []() {
    switchRelay(true, false);
    server.send(200, FPSTR(mimeTable[json].mimeType), getRelaysState());
  });
  server.on("/on1", []() {
    switchRelay(false, true);
    server.send(200, FPSTR(mimeTable[json].mimeType), getRelaysState());
  });
  server.on("/off1", []() {
    switchRelay(false, false);
    server.send(200, FPSTR(mimeTable[json].mimeType), getRelaysState());
  });
  server.on("/scan", []() {
    server.send(200, FPSTR(mimeTable[json].mimeType), scanWifi());
  });
  server.on("/wifi", []() {
    server.send(200, FPSTR(mimeTable[json].mimeType), getWifiInfo());
  });
  server.on("/conn", []() {
    const char *ssid = server.arg("ssid").c_str();
    const char *password = server.arg("password").c_str();
    server.send(200, FPSTR(mimeTable[txt].mimeType), F("connecting"));
    delay(300);
    connectWifi(ssid, password);
  });
  server.on("/ap", []() {
    bool state = !server.arg("ap").equals("off");
    server.send(200, FPSTR(mimeTable[txt].mimeType), F("switching"));
    delay(300);
    switchAccessPoint(state);
  });
  server.on("/sync", []() {
    String s = synchronizeTime() ? F("synchronizing") : F("not finished");
    server.send(200, FPSTR(mimeTable[txt].mimeType), s);
  });
  server.on("/time", []() {
    server.send(200, FPSTR(mimeTable[json].mimeType), getTime());
  });
  server.on("/events", []() {
    server.send(200, FPSTR(mimeTable[json].mimeType), getEvents());
  });
  server.on("/settime", []() {
    setTime(server.arg("time").toInt());
    server.send(200, FPSTR(mimeTable[json].mimeType), getTime());
  });
  server.on("/run", []() {
    if (switchTimer(!server.arg("run").equals("off"))) {
      server.send(200, FPSTR(mimeTable[json].mimeType), getEvents());
    } else {
      sendError();
    }
  });
  server.on("/create", []() {
    unsigned long sec = server.arg("time").toInt();
    bool isRelay0 = !server.arg("relay").equals("1");
    bool relayState = server.arg("state").equals("on");
    if (addEvent(sec, isRelay0, relayState)) {
      server.send(200, FPSTR(mimeTable[json].mimeType), getEvents());
    } else {
      sendError();
    }
  });
  server.on("/edit", []() {
    int eventN = server.arg("eventN").toInt();
    unsigned long sec = server.arg("time").toInt();
    bool isRelay0 = !server.arg("relay").equals("1");
    bool relayState = server.arg("state").equals("on");
    if (editEvent(eventN, sec, isRelay0, relayState)) {
      server.send(200, FPSTR(mimeTable[json].mimeType), getEvents());
    } else {
      sendError();
    }
  });
  server.on("/remove", []() {
    if (deleteEvent(server.arg("eventN").toInt())) {
      server.send(200, FPSTR(mimeTable[json].mimeType), getEvents());
    } else {
      sendError();
    }
  });
  server.on("/files", []() {
    server.send(200, FPSTR(mimeTable[json].mimeType), getFilesInfo());
  });
  server.on("/upload", HTTP_GET, []() {
    server.send(200, FPSTR(mimeTable[html].mimeType), FPSTR(uploadPage));
  });
  server.on("/upload", HTTP_POST, []() {
    server.send(200, FPSTR(mimeTable[txt].mimeType), F("uploading"));
  }, uploadFile);
  server.on("/delete", deleteFile);
  server.onNotFound(readFile);
  server.begin();
}

void loopServer() {
  server.handleClient();
}
