#include <RFM69.h>
#include <SPI.h>

#define TURNON_NETWORK_ID       69
#define TURNON_CLIENT_ADDRESS   1
#define TURNON_SERVER_ADDRESS   2
#define TURNON_FREQUENCY        RF69_433MHZ
#define TURNON_STATE_LED_PIN    9
#define TURNON_CYCLE_DURATION   250

byte currentState       = 0;
const int LED_STATES[]  = { LOW, HIGH };

RFM69 radio;

void setup() {
  Serial.begin(115200);
  pinMode(TURNON_STATE_LED_PIN, OUTPUT);
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

void awaitCurrentStateMessage() {
  if (radio.receiveDone()) {
    byte state = radio.DATA[0];
    setCurrentState(state);

    if(radio.ACKRequested()) {
      radio.sendACK();
    }
  }

  radio.receiveDone();
}

void setCurrentState(byte state) {
  currentState = state;
  Serial.println(currentState);  
  Serial.flush();
  digitalWrite(TURNON_STATE_LED_PIN, LED_STATES[currentState]);
}

