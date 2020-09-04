// 2 relays, the main button and 2 relay buttons (pins on the board)
// 1 push on the button to switch the relay0 (out 1)
// 2 pushes on the button to switch the relay1 (out 2)
// 3 pushes on the button to enable the access point
// 4 and more pushes on the button to disable the access point

// main setup
void setupSerial();

// main loop
void loopSerial();

// switches one relay on/off
void switchRelay(bool isRelay0, bool relayState);

// switches both relays on/off
void switchRelays(bool relay0State, bool relay1State);

// returns a json object with relays state:
//  {"relay0":"on","relay1":"off"}
String getRelaysState();
