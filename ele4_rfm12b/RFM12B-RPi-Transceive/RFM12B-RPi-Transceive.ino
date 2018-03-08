// Simple RFM12B transceiver program, with ACK and optional encryption
// It initializes the RFM12B radio with optional encryption and passes through any valid messages to the serial port
// felix@lowpowerlab.com

#include "RFM12B.h"

// You will need to initialize the radio by telling it what ID it has and what network it's on
// The NodeID takes values from 1-127, 0 is reserved for sending broadcast messages (send to all nodes)
// The Network ID takes values from 0-255
// By default the SPI-SS line used is D10 on Atmega328. You can change it by calling .SetCS(pin) where pin can be {8,9,10}
#define NODEID        1  //network ID used for this unit
#define NETWORKID    210  //the network ID we are on
#define ACK_TIME     50  // # of ms to wait for an ack

//encryption is OPTIONAL
//to enable encryption you will need to:
// - provide a 16-byte encryption KEY (same on all nodes that talk encrypted)
// - to call .Encrypt(KEY) to start encrypting
// - to stop encrypting call .Encrypt(NULL)
uint8_t KEY[] = "ABCDABCDABCDABCD";

// Need an instance of the Radio Module
RFM12B radio;
byte sendSize = 0;
String inputString = "";
char *buf = NULL;

bool requestACK = false;
//bool stringComplete = false;
int sendState = 0;
unsigned int transmitNumber = 0;
int gatewayid = 0;

// wait a few milliseconds for proper ACK, return true if received
static bool waitForAck(int gwid) {
  long now = millis();
  while (millis() - now <= ACK_TIME)
    if (radio.ACKReceived(gwid))
      return true;
  return false;
}

void sendPayload(int gwid, char *payload, byte plsize) {
  requestACK = !(transmitNumber % 3); //request ACK every 3rd xmission
  
  radio.Send(gwid, payload, plsize, requestACK);
  if (requestACK)
    waitForAck(gwid); // if ...

  Serial.println("OK");

  transmitNumber++;
}

void setup()
{
  Serial.begin(57600);
  radio.Initialize(NODEID, RF12_868MHZ, NETWORKID);
  //radio.Encrypt(KEY);
  Serial.println("START_TRANSCEIVE");
}

void loop()
{
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      sendSize = inputString.length();
      if (sendSize > 0) {
        //stringComplete = true;
        
        buf = (char *)malloc((sendSize + 1) * sizeof(char));
        memset(buf, '\0', (sendSize + 1));
        inputString.toCharArray(buf, sendSize + 1);

        if (sendState == 0) {
          char *end = NULL;
          const int sl = (int)strtol(buf, &end, 10);
          if (end == buf) {
            Serial.println("ERROR");
          } else if (end == NULL) {
            Serial.println("ERROR");
          } else if (*end != '\0') {
            Serial.println("ERROR");
          } else {
            Serial.println("OK");
            gatewayid = sl;
            sendState = 1;
          }
        } else if (sendState == 1) {
          sendPayload(gatewayid, buf, sendSize);
          sendState = 0;
        }
  
        if (buf != NULL) {
          free(buf);
          buf = NULL;
        }
  
        inputString = "";
        //stringComplete = false;
      } else {
        Serial.println("ERROR");
        inputString = "";
        sendState = 0;
      }

      break;
    }
    // add it to the inputString:
    inputString += inChar;
  }

  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      Serial.print("[");
      Serial.print(radio.GetSender());
      Serial.print("] ");
      for (byte i = 0; i < radio.GetDataLen(); i++)
        Serial.print((char)radio.Data[i]);

      if (radio.ACKRequested())
      {
        radio.SendACK();
      }
    } // else: BAD CRC!
    
    Serial.println();
  }  
}

