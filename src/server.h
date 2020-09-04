// http server and file system
// http requests, can be get or post:
//  /links - returns html with all links
//  /about - redirects to the website
//  /esp - returns json {
//   "sketchSize":377000,
//   "freeHeap":43000,
//   "heapFragmentation":2, // in percent
//   "ms":1000000, // ms since the start
//   "cycleCount":1000000000,
//   "resetReason":"Software/System restart"}
//  /restart - returns txt "restarting", then restarts esp
//  /flash - prepares to flash over wifi, returns txt "ok" or "already"
//  /relays - returns json with relays state, see serial.h
//  /switch with optional arguments "relay0" ("on" or "off", default "off"),
//   "relay1" ("on" or "off", default "off") - switches both relays,
//   returns the same as /relays
//  /on0 - switches relay0 on, returns the same as /relays
//  /off0 - switches relay0 off, returns the same as /relays
//  /on1 - switches relay1 on, returns the same as /relays
//  /off1 - switches relay1 off, returns the same as /relays
//  /scan - returns json with available networks, see wifi.h
//  /wifi - returns json with info about wifi, see wifi.h
//  /conn with optional arguments "ssid" and "password" -
//   returns txt "connecting", then connects to the wifi
//  /ap with optional argument "ap" ("on" or "off", default "on") -
//   returns txt "switching", then enables/disables the access point
//  /sync - synchronizes time with ntp server,
//   returns txt "synchronizing" or "not finished"
//  /time - returns json with current time, see timer.h
//  /events - returns json with events, see timer.h
//  /settime with optional argument "time" (in sec since 00:00, default "0") -
//   sets current time, returns the same as /time
//  /run with optional argument "run" ("on" or "off", default "on") -
//   enables/disables timer, returns the same as /events or error 500
//  /create with optional arguments "time" (in sec since 00:00, default "0"),
//   "relay" ("0" or "1", default "0"), "state" ("on" or "off", default "off") -
//   creates a new event, returns the same as /events or error 500
//  /edit with optional arguments "eventN" (default "0"),
//   "time" (in sec since 00:00, default "0"),
//   "relay" ("0" or "1", default "0"), "state" ("on" or "off", default "off") -
//   edits the existing event, returns the same as /events or error 500
//  /remove with optional argument "eventN" (default "0") -
//   deletes the event, returns the same as /events or error 500
//  /files - returns json {
//   "totalBytes":190000,
//   "usedBytes":100000,
//   "files":[{"name":"config.txt","size":100}, ... ]}
//  /upload get - returns html with uploading form
//  /upload post - uploads the file, returns txt "uploaded" or error 500
//  /delete without arguments - returns html with deleting form
//  /delete with argument "file" - deletes this file,
//   returns txt "deleted" or error 500
//  / - returns /index.html or error 404
//  any other path returns this file or error 404

// main setup
void setupServer();

// main loop
void loopServer();
