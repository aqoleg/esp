// simple network time protocol, see rfc1361
// the average accuracy of about a second
// device will perform 10 attempts to synchronize the time
//  after power on, restart, or manual command

// loop when wifi is connected
void loopNtp();

// synchronizes time with ntp server, returns false if already started
bool synchronizeTime();
