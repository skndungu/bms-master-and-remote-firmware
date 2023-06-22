#ifndef LORA_RA02_H_
#define LORA_RA02_H_

#include <Arduino.h>

#include <SPI.h>              // include libraries
#include <LoRa.h>

#define csPin     5       // LoRa radio chip select
#define resetPin 21      // LoRa radio reset
#define irqPin   22     // change for your board; must be a hardware interrupt pin
#define lora_DI01 4

bool receivedMessage = false;
char outgoing[100];

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xD0;     // address of this device
byte destination  = 0xDD;      // destination to send to

 String incoming = "";
 
void init_lora()
{
   LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
    if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
   Serial.println("LoRa init succeeded.");
   return;
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

 

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  receivedMessage = true;
}


void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}


#endif //LORA_RA02_H_