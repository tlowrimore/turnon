#include <RHReliableDatagram.h>
#include <RH_RF22.h>
#include <SPI.h>

#define TURNON_CLIENT_ADDRESS 1
#define TURNON_SERVER_ADDRESS 2

uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];

RH_RF22 driver;
RHReliableDatagram manager(driver, TURNON_SERVER_ADDRESS);

void setup() {
  Serial.begin(9600);
  initRF22();
}

void loop() {
  awaitCurrentStateMessage();
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

void awaitCurrentStateMessage() {
  if (manager.available()) {
    
    // Wait for a message addressed to us from the client
    uint8_t bufLen = sizeof(buf);
    uint8_t from;
    
    if (manager.recvfromAck(buf, &bufLen, &from)) {
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);

      uint8_t resp[] = { 1 };

      // Send a reply back to the originator client
      if (!manager.sendtoWait(resp, sizeof(resp), from)) {
        Serial.println("sendtoWait failed");
      }
    }
  }
}

