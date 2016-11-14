#include <RFM69.h>

// Turnon-specific settings
#define TURNON_NETWORK_ID         69
#define TURNON_CLIENT_ADDRESS     1
#define TURNON_SERVER_ADDRESS     2
#define TURNON_FREQUENCY          RF69_433MHZ
#define TURNON_SENSOR_PIN         0
#define TURNON_POT_LEFT_PIN       1
#define TURNON_POT_RIGHT_PIN      2
#define TURNON_POT_RANGE_SIZE     1024
#define TURNON_TX_LED_PIN         9
#define TURNON_SQUEEZE_THRESHOLD  100
#define TURNON_CYCLE_DURATION     250
#define TURNON_HOLD_MILLIS        3000
#define TURNON_ACK_TIMEOUT        2000

#define TURNON_CHANGE_BIT         1
#define TURNON_HOLD_BIT           2
#define TURNON_STATE_LEFT_BIT     4
#define TURNON_STATE_RIGHT_BIT    8


int   heldMillis    = 0;
byte  currentState  = 0;

RFM69 radio;

void setup() {
  pinMode(TURNON_TX_LED_PIN, OUTPUT);
  initRadio();
}

void loop() {
  int   sensorValue       = analogRead(TURNON_SENSOR_PIN);
  int   potLeftValue      = analogRead(TURNON_POT_LEFT_PIN);
  int   potRightValue     = analogRead(TURNON_POT_RIGHT_PIN);
  byte  potLeftBitValue   = thresholdPotValue(potLeftValue);
  byte  potRightBitValue  = thresholdPotValue(potRightValue);
  
  updateCurrentState(sensorValue, potLeftBitValue, potRightBitValue);
  broadcastCurrentStateIfChanged();
  delay(TURNON_CYCLE_DURATION);
}

// ----------------------------------------------------------
// Helper functions
// ----------------------------------------------------------

// Initializes the Radio
void initRadio() {
  bool radioInitialized = radio.initialize( TURNON_FREQUENCY, 
                                            TURNON_CLIENT_ADDRESS, 
                                            TURNON_NETWORK_ID);


  if(radioInitialized) {
    radio.setHighPower();
  }
}

// Given the sensor value, this function computes the current state
// of the transmitter.
void updateCurrentState(int sensorValue, int potLeftValue, int potRightValue) {
  if(sensorValue > TURNON_SQUEEZE_THRESHOLD) {
    heldMillis += TURNON_CYCLE_DURATION;
    
    if(heldMillis >= TURNON_HOLD_MILLIS) {
      heldMillis  = 0;

      // Turn on the change state bit
      currentState = currentState | TURNON_CHANGE_BIT;
      
      // Flip the hold state bit     
      currentState = currentState ^ TURNON_HOLD_BIT;

    }
  } else {
    heldMillis = 0;
  }
}

// Broadcasts the hold state bit, if our state changed.
void broadcastCurrentStateIfChanged() {

  // if the change state bit is on... 
  if(currentState & TURNON_CHANGE_BIT) {
    onTxBegin();
    
    // turn off the change state bit
    currentState = currentState ^ TURNON_CHANGE_BIT;

    // Get the current hold state
    byte holdState  = getCurrentStateBitValue(TURNON_HOLD_BIT);
    byte message[]  = { holdState };
    
    radio.sendWithRetry(TURNON_SERVER_ADDRESS, 
                        message, 
                        sizeof(message));
                        
    radio.receiveDone();
    onTxEnd();
  }
}

// Gets the value of the given bit from the current state.
byte getCurrentStateBitValue(byte stateBit) {
  return (currentState >> log2(stateBit)) & 1; 
}

int log2(int i) {
  return (log(i) / log(2));
}

void onTxBegin() {
  digitalWrite(TURNON_TX_LED_PIN, HIGH);
}

void onTxEnd() {
  digitalWrite(TURNON_TX_LED_PIN, LOW);
}

byte thresholdPotValue(int value) {
  int rangeMid = TURNON_POT_RANGE_SIZE / 2;
  return (value & rangeMid) / rangeMid;
}

