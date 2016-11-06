#include <RF22Datagram.h>
#include <SPI.h>
#include <RF22.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

int sensorPin       = 0;
int sqeezeThreshold = 100; // higher number == sqeeze harder
int cycle           = 250;
int holdMillis      = 3000;
int heldMillis      = 0;
bool isHolding      = false;
bool didChange      = false;

RF22Datagram rf22(SERVER_ADDRESS);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (rf22.init()) {
    Serial.println("RF22 init!");
  } else {
    Serial.println("RF22 init failed");
  }
}

void loop() {
  int sensorValue = analogRead(sensorPin);

  Serial.println(sensorValue);

  // Measure Sensor and set state
  if (sensorValue > sqeezeThreshold) {
    heldMillis += cycle;

    if (heldMillis >= holdMillis) {
      heldMillis  = 0;
      didChange   = true;
      
      if (!isHolding) {
        isHolding = true;
      } else {
        isHolding = false;
      }
    }

  } else {
    heldMillis = 0;
  }

  // State changed!
  if (didChange) {

    // Reset state change indicator
    didChange = false;
    bool accepted;

    if (isHolding) {
      Serial.println("Sending: ON");
      // Send a message to rf22_server
      uint8_t data[] = "1";
      accepted = rf22.sendto(data, sizeof(data), CLIENT_ADDRESS);
    } else {
      Serial.println("Sending: OFF");

      // Send a message to rf22_server
      uint8_t data[] = "0";
      accepted = rf22.sendto(data, sizeof(data), CLIENT_ADDRESS);
    }

    Serial.print("Message accepted: ");
    Serial.println(accepted);

    // Wait for transmission to complete.
    rf22.waitPacketSent();
    Serial.println("after WPS");
  }
  delay(cycle); // Pause for sensorValue

}
