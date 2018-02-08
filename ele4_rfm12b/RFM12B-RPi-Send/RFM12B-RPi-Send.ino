// Simple RFM12B sender program, with ACK and optional encryption
// It initializes the RFM12B radio with optional encryption and passes through any valid messages to the serial port
// felix@lowpowerlab.com

#include "RFM12B.h"
#include <avr/sleep.h>

// You will need to initialize the radio by telling it what ID it has and what network it's on
// The NodeID takes values from 1-127, 0 is reserved for sending broadcast messages (send to all nodes)
// The Network ID takes values from 0-255
// By default the SPI-SS line used is D10 on Atmega328. You can change it by calling .SetCS(pin) where pin can be {8,9,10}
#define NODEID        1  //network ID used for this unit
#define NETWORKID    210  //the network ID we are on
#define GATEWAYID     10  //the node ID we're sending to
#define ACK_TIME     50  // # of ms to wait for an ack

//encryption is OPTIONAL
//to enable encryption you will need to:
// - provide a 16-byte encryption KEY (same on all nodes that talk encrypted)
// - to call .Encrypt(KEY) to start encrypting
// - to stop encrypting call .Encrypt(NULL)
uint8_t KEY[] = "ABCDABCDABCDABCD";

int interPacketDelay = 1000; //wait this many ms between sending packets
char input = 0;

// Need an instance of the Radio Module
RFM12B radio;
byte sendSize=0;
String inputString = "";
bool requestACK = false;
bool stringComplete = false;
unsigned int transmitNumber = 0;

void setup()
{
  Serial.begin(57600);
  radio.Initialize(NODEID, RF12_868MHZ, NETWORKID);
  //radio.Encrypt(KEY);
  radio.Sleep(); //sleep right away to save power
  Serial.println("START_SEND");
}

void loop()
{
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      break;
    }
    // add it to the inputString:
    inputString += inChar;
  }

  sendSize = inputString.length();

  if ((sendSize > 0) && (stringComplete == true)) {
    requestACK = !(transmitNumber % 3); //request ACK every 3rd xmission

    // HACKY!
    char *buf = (char *)malloc((sendSize + 1) * sizeof(char));
    memset(buf, '\0', (sendSize + 1));
    inputString.toCharArray(buf, sendSize + 1);

    radio.Wakeup();
    radio.Send(GATEWAYID, buf, sendSize, requestACK);
    if (requestACK)
    {
      waitForAck(); // if ...
    }
    radio.Sleep();

    free(buf);

    Serial.println("OK");

    inputString = "";
    stringComplete = false;
    transmitNumber++;

    delay(interPacketDelay);
  }
}

// wait a few milliseconds for proper ACK, return true if received
static bool waitForAck() {
  long now = millis();
  while (millis() - now <= ACK_TIME)
    if (radio.ACKReceived(GATEWAYID))
      return true;
  return false;
}
