#include <RHReliableDatagram.h>
#include <RH_RF22.h>
#include <SPI.h>

// Turnon-specific settings
#define TURNON_CLIENT_ADDRESS   1
#define TURNON_SERVER_ADDRESS   2
#define TURNON_SENSOR_PIN       0
#define TURNON_SQEEZE_THRESHOLD 100
#define TURNON_CYCLE_DURATION   250
#define TURNON_HOLD_MILLIS      3000
#define TURNON_CHANGE_STATE     1
#define TURNON_HOLD_STATE       2

uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];

int   sensorValue   = 0;
int   heldMillis    = 0;
bool  currentState  = 0;

RH_RF22 driver;
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup() {
  Serial.begin(9600);
  initRF22();
}

void loop() {
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);

  updateCurrentState(sensorValue);
  broadcastHoldStateIfChanged();
  delay(TURNON_CYCLE_DURATION);
}

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
void updateCurrentState(sensorValue) {
  if(sensorValue >= sqeezeThreshold) {
    heldMillis += cycle;

    if(heldMillis >= holdMillis) {
      heldMillis    = 0;

      // Turn on the change state bit
      currentState = currentState | TURNON_CHANGE_STATE
      
      // Flip the hold state bit     
      currentState = currentState ^ TURNON_HOLD_STATE
      
    } else {
      heldMillis = 0;
    }
  }
}

// Broadcasts the hold state bit, if our state changed.
void broadcastHoldStateIfChanged() {

  // if the change state bit is on... 
  if(currentState & TURNON_CHANGE_STATE) {

    // turn off the change state bit
    currentState = currentState ^ TURNON_CHANGE_STATE

    // Get the current hold state
    uint8_t holdState = (currentState >> log2(TURNON_HOLD_STATE)) & 1

    
  }
}

int log2(i) {
  return (log(i) / log(2))
}

