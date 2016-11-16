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
#define TURNON_FLOWER_RED_PIN     3
#define TURNON_FLOWER_GREEN_PIN   4
#define TURNON_FLOWER_BLUE_PIN    5
#define TURNON_SQUEEZE_THRESHOLD  75
#define TURNON_CYCLE_DURATION     250
#define TURNON_HOLD_MILLIS        3000
#define TURNON_ACK_TIMEOUT        2000

#define TURNON_CHANGE_BIT         0
#define TURNON_STATE_RED_BIT      1
#define TURNON_STATE_GREEN_BIT    2
#define TURNON_STATE_BLUE_BIT     3


int   heldMillis        = 0;
byte  currentState      = 0;

RFM69 radio;

void setup() {
  pinMode(TURNON_TX_LED_PIN,        OUTPUT);
  pinMode(TURNON_FLOWER_RED_PIN,    OUTPUT);
  pinMode(TURNON_FLOWER_GREEN_PIN,  OUTPUT);
  pinMode(TURNON_FLOWER_BLUE_PIN,   OUTPUT);
  
  initRadio();
}

void loop() {
  int   sensorValue       = analogRead(TURNON_SENSOR_PIN);
  int   potLeftValue      = analogRead(TURNON_POT_LEFT_PIN);
  int   potRightValue     = analogRead(TURNON_POT_RIGHT_PIN);
  byte  colorBit          = potValuesToColorBit(potLeftValue, potRightValue);
  
  updateCurrentState(sensorValue, colorBit);
  updateFlowerColor();
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
void updateCurrentState(int sensorValue, byte colorBit) {

  // We've met the squeeze threshold.
  if(sensorValue > TURNON_SQUEEZE_THRESHOLD) {
    heldMillis += TURNON_CYCLE_DURATION;

    // We've squeezed for a duration of at least TURNON_HOLD_MILLIS
    if(heldMillis >= TURNON_HOLD_MILLIS) {
      heldMillis  = 0;

      // Special case: if the color bit is 0,
      // reset the current state
      if(colorBit == 0) {

        // Turn-off all color bits       
        currentState = bit(TURNON_CHANGE_BIT);
      } else {
      
        // Turn on the change state bit
        bitSet(currentState, TURNON_CHANGE_BIT);
      
        // Flip the color state bit     
        currentState ^= bit(colorBit);
      }
    }
  } else {
    heldMillis = 0;
  }
}

// Update the flower LED colors
void updateFlowerColor() {
  digitalWrite( TURNON_FLOWER_RED_PIN, 
                bitRead(currentState, TURNON_STATE_RED_BIT));
                
  digitalWrite( TURNON_FLOWER_GREEN_PIN, 
                bitRead(currentState, TURNON_STATE_GREEN_BIT));
                
  digitalWrite( TURNON_FLOWER_BLUE_PIN,  
                bitRead(currentState, TURNON_STATE_BLUE_BIT));
}

// Broadcasts the hold state bit, if our state changed.
void broadcastCurrentStateIfChanged() {

  // if the change state bit is on... 
  if(currentState & bit(TURNON_CHANGE_BIT)) {
    onTxBegin();
    
    // turn off the change state bit
    bitClear(currentState, TURNON_CHANGE_BIT);

    // Shift-off the Change State Bit, leaving just the
    // color bits.
    byte message[]  = { getColorBits() };
    radio.sendWithRetry(TURNON_SERVER_ADDRESS, 
                        message, 
                        sizeof(message));
                        
    radio.receiveDone();
    onTxEnd();
  }
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

byte potValuesToColorBit(int potLeftValue, int potRightValue) {
  byte  colorBit        = 0;
  byte  potLeftState    = thresholdPotValue(potLeftValue);
  byte  potRightState   = thresholdPotValue(potRightValue);
  
  bitWrite(colorBit, 0, potRightState);
  bitWrite(colorBit, 1, potLeftState);

  return colorBit;
}

byte getColorBits() {
  return currentState >> 1;
}

