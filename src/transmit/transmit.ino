#include <RHReliableDatagram.h>
#include <RH_RF22.h>
#include <SPI.h>

// Turnon-specific settings
#define TURNON_CLIENT_ADDRESS     1
#define TURNON_SERVER_ADDRESS     2
#define TURNON_SENSOR_PIN         0
#define TURNON_SQUEEZE_THRESHOLD  100
#define TURNON_CYCLE_DURATION     500
#define TURNON_HOLD_MILLIS        3000
#define TURNON_ACK_TIMEOUT        2000
#define TURNON_CHANGE_STATE       1
#define TURNON_HOLD_STATE         2

uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];

int sensorValue   = 0;
int heldMillis    = 0;
int currentState  = 0;

RH_RF22 driver;
RHReliableDatagram manager(driver, TURNON_CLIENT_ADDRESS);

void setup() {
  Serial.begin(9600);
  initRF22();
}

void loop() {
  sensorValue = analogRead(TURNON_SENSOR_PIN);
  Serial.println(sensorValue);

  updateCurrentState(sensorValue);
  broadcastCurrentStateIfChanged();
  delay(TURNON_CYCLE_DURATION);
}

// ----------------------------------------------------------
// Helper functions
// ----------------------------------------------------------

// Initializes the RF22
void initRF22() {
  if(manager.init()) {
    Serial.println("RF22 initialized.");   
  } else {
    Serial.println("Error: RF22 failed to initialize.");
  }
}

// Given the sensor value, this function computes the current state
// of the transmitter.
void updateCurrentState(int sensorValue) {
  if(sensorValue > TURNON_SQUEEZE_THRESHOLD) {
    heldMillis += TURNON_CYCLE_DURATION;
    
    if(heldMillis >= TURNON_HOLD_MILLIS) {
      heldMillis  = 0;

      // Turn on the change state bit
      currentState = currentState | TURNON_CHANGE_STATE;
      
      // Flip the hold state bit     
      currentState = currentState ^ TURNON_HOLD_STATE;

      Serial.print("Current State changed to: ");
      Serial.println(currentState);
    }
  } else {
    heldMillis = 0;
  }
}

// Broadcasts the hold state bit, if our state changed.
void broadcastCurrentStateIfChanged() {

  // if the change state bit is on... 
  if(currentState & TURNON_CHANGE_STATE) {
    Serial.println("STATE CHANGED!");
    // turn off the change state bit
    currentState = currentState ^ TURNON_CHANGE_STATE;

    // Get the current hold state
    uint8_t holdState = getCurrentStateBitValue(TURNON_HOLD_STATE);
    uint8_t message[] = { holdState };

    Serial.print("HOLD STATE: ");
    Serial.println(holdState);

    bool msgSent = manager.sendtoWait(  message, 
                                        sizeof(message), 
                                        TURNON_SERVER_ADDRESS);
    Serial.println("Sent Msg!");
    
    if(msgSent) {
      
      uint8_t bufLen = sizeof(buf);
      uint8_t from;

      if(manager.recvfromAckTimeout(buf, &bufLen, TURNON_ACK_TIMEOUT, &from)) {
        Serial.print("got reply from : 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.println((char*)buf);
      } else {
        Serial.println("No reply, is rf22_reliable_datagram_server running?");
      }
    } else {
      Serial.println("sendToWait failed.");
    }
  }
}

// Gets the value of the given bit from the current state.
int getCurrentStateBitValue(uint8_t stateBit) {
  return (currentState >> log2(stateBit)) & 1; 
}

int log2(int i) {
  return (log(i) / log(2));
}

