// timer and its configuration file /config.txt
// timer has a period of 24 hours and can contain up to 20 events
// after each change, the timer sets relays state the same as it would have been
//  if the timer had been running since 00:00:00Z utc
// example of config.txt, no white spaces, each event starts with a new line '\n':
//  on          // on or off, the state
//  3600,0,on   // events: time in seconds since 00:00:00Z utc, relay number, on or off
//  7200,1,off  // time >= previousTime

// main setup, call after setupServer()
void setupTimer();

// main loop
void loopTimer();

// sets current time
//  sec - unix time or seconds since 00:00:00Z utc
void setTime(unsigned long sec);

// enables/disables the timer, returns true if successful
bool switchTimer(bool state);

// creates a new event, returns true if successful
//  sec - unix time or seconds since 00:00:00Z utc
bool addEvent(unsigned long sec, bool isRelay0, bool relayState);

// edits existed event, returns true if successful
//  sec - unix time or seconds since 00:00:00Z utc
bool editEvent(int eventN, unsigned long sec, bool isRelay0, bool relayState);

// deletes the event, returns true if successful
bool deleteEvent(int eventN);

// returns a json object with timer state and events:
//  {
//   "run":"on", // timer state
//   "events":[{
//    "time":3600, // seconds since 00:00:00Z utc
//    "relay":0, // 0 or 1
//    "state":"on" // on or off
//   }, ... ] // time >= previousTime
//  }
String getEvents();

// returns a json object with current time:
//  {
//   "time":7200, // seconds since 00:00:00Z utc
//   "nextEvent":4 // number of the next event to fire or -1 if there is no events
//  }
String getTime();
