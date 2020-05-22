/** @author Kodiak North
 * @date 05/21/2020
 * test project with NRF24l01 module in PRX mode, to 
 * communicate with another NRF24l01 module in PTX mode
 */

#include <Arduino.h>
#include "ConfigPRX.h"
#include "..\lib\DataLog\DataLog.h"
#include "..\lib\arduino-printf\src\printf.h"
#include "stdio.h"
#include "nRF24L01.h"
#include "RF24.h"

RX_TO_TX rtt;
RF24 radio(RF_CE_PIN, RF_CSN_PIN); // Create a Radio

void setup() {
  Serial.begin(115200);

  rtt.SwitchStatus = 0x9F;
  rtt.SolenoidStatus = 0x5E;

  pinMode(RF_IRQ_PIN, INPUT);

  if (!radio.begin())
    Serial.println("PRX failed to initialize");
  else {
    // RF24 library begin() function modified to enable PRX mode
    radio.setAddressWidth(5); // set address size to 5 bytes
    radio.setChannel(RF_CHANNEL); // set communication channel
    radio.setPayloadSize(NUM_RTT_BYTES); // set max transmission payload size to sizeof RTT data struct
    radio.enableAckPayload(); // enable ability to add payload sent with auto ACK
    radio.enableDynamicAck();
    radio.setPALevel(RF24_PA_LOW); // set power amplifier level. Using LOW for tests on bench. Should use HIGH on PL/Truck
    radio.setDataRate(RF24_1MBPS); // set data rate to most reliable speed
    radio.openReadingPipe(0, RF_PRX_READ_ADDR);
    radio.startListening(); // PRX now needs to start listening for packets
    radio.writeAckPayload(0, &rtt, NUM_RTT_BYTES);
    Serial.println("PRX initialization successful");
  }
}

unsigned long curTime = 0;
static unsigned long preTime = 0;
void loop() {
  curTime = millis();
  if (curTime - preTime >= 10) { // run at 100Hz frequency
    radio.writeAckPayload(0, &rtt, NUM_RTT_BYTES);
    preTime = curTime;
  }

  if (LOW == digitalRead(RF_IRQ_PIN)) {
    bool tx_ok, tx_fail, rx_ready;
    radio.whatHappened(tx_ok, tx_fail, rx_ready);
    if (rx_ready || radio.available()) {
      static TX_TO_RX ttr;
      static int i = 0;
      radio.read(&ttr, NUM_TTR_BYTES);
      LogInfo(F("phase 0x%X, ledControl 0x%X, encoder 0x%X, count %d\n"), ttr.Phase, ttr.LEDControl, ttr.FrontEncoder, ++i);
    }
  }
}