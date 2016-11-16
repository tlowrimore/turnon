#include <RFM69.h>

#define TURNON_NETWORK_ID       69
#define TURNON_CLIENT_ADDRESS   1
#define TURNON_SERVER_ADDRESS   2
#define TURNON_FREQUENCY        RF69_433MHZ
#define TURNON_RX_LED_PIN       9
#define TURNON_RELAY_RED_PIN    3
#define TURNON_RELAY_GREEN_PIN  4
#define TURNON_RELAY_BLUE_PIN   5
#define TURNON_CYCLE_DURATION   250

#define TURNON_STATE_RED_BIT    0
#define TURNON_STATE_GREEN_BIT  1
#define TURNON_STATE_BLUE_BIT   2

byte currentState       = 0;
const int LED_STATES[]  = { LOW, HIGH };

RFM69 radio;

void setup() {
  pinMode(TURNON_RX_LED_PIN,      OUTPUT);
  pinMode(TURNON_RELAY_RED_PIN,   OUTPUT);
  pinMode(TURNON_RELAY_GREEN_PIN, OUTPUT);
  pinMode(TURNON_RELAY_BLUE_PIN,  OUTPUT);
  
  initRadio();
}

void loop() {
  awaitCurrentStateMessage();
  delay(TURNON_CYCLE_DURATION);
}

// ----------------------------------------------------------
// Helper functions
// ----------------------------------------------------------

// Initializes the Radio
void initRadio() {
  bool radioInitialized = radio.initialize( TURNON_FREQUENCY, 
                                            TURNON_SERVER_ADDRESS, 
                                            TURNON_NETWORK_ID);

  if(radioInitialized) {
    radio.setHighPower();
  }
}

// Listens for radio transmissions from the transmitter.
void awaitCurrentStateMessage() {
  if (radio.receiveDone()) {
    onRxBegin();
    
    currentState = radio.DATA[0];

    updateRelays();

    if(radio.ACKRequested()) {
      radio.sendACK();
    }

    onRxEnd();
  }

  radio.receiveDone();
}

// Updates the relays with the current state
void updateRelays() {
  digitalWrite( TURNON_RELAY_RED_PIN,
                bitRead(currentState, TURNON_STATE_RED_BIT));
                
  digitalWrite( TURNON_RELAY_GREEN_PIN,
                bitRead(currentState, TURNON_STATE_GREEN_BIT));
                
  digitalWrite( TURNON_RELAY_BLUE_PIN,
                bitRead(currentState, TURNON_STATE_BLUE_BIT));
}

void onRxBegin() {
  digitalWrite(TURNON_RX_LED_PIN, HIGH);
}

void onRxEnd() {
  digitalWrite(TURNON_RX_LED_PIN, LOW);
}
