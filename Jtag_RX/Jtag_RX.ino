/*MIT License

Copyright (c) 2021 JINODK

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include <ESP8266WiFi.h>
void preinit() {
  // Global WiFi constructors are not called yet
  // (global class instances like WiFi, Serial... are not yet initialized)..
  // No global object methods or C++ exceptions can be called in here!
  // The below is a static class method, which is similar to a function, so it's ok.
  ESP8266WiFiClass::preinitWiFiOff(); // turn off wifi AP as we don't need that
}
#include <LoRa.h>
#include <SPI.h>
#include <AESLib.h> // WARNING: only use version 2.1.9 of this library as it's the only one that worked
#include <Adafruit_PCD8544.h>

byte i = 0; // timer counter
char rawBUFFER[128]; // double the length of decodedBUFFER for safety
char decodedBUFFER[64]; // this is where we store raw data after encrypt it
String BUFFER; // data for parsing onto the display
byte aes_key[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}; // 128-bit key, USE YOUR OWN KEY
byte aes_iv[N_BLOCK] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}; // 128-bit IV, USE YOUR OWN KEY FOR SECURITY
char SthImportant[32];

// data variables for the LCD display
byte SAT;
char LAT[12], LON[12];
char DATE[11], TIME[9];

AESLib aesLib;
// Hardware SPI:
// CLK - CLK
// DIN - MOSI
// D/C - D3
// CS  - D4
// RST - NC
Adafruit_PCD8544 display = Adafruit_PCD8544(D3, D4, -1);


void setup() {
  Serial.begin(9600); // 9600 for the sake of consistency with the TX
  Serial.println();
  base64_decode(SthImportant, "TWFkZSBieSBKSU5PREsgdW5kZXIgTUlUIGxpY2Vuc2U", 43);
  Serial.println(SthImportant); // just some reminder :)
  Serial.print("booting...");
  
  // Wiring config for SX1276 - NODEMCU
  // NSS   - D8
  // RESET - D4
  // DIO0  - D1
  // for more info about configuring LoRa module, visit https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md#setup
  LoRa.setPins(D8, D0, D1); // NSS RESET DIO0
  Serial.println(LoRa.begin(868E6)); // 868mhz is a stable freq for Semtech SX1276, print out the init return
  LoRa.setSyncWord(0xF4); // just like preamble but can be changed for more security
  LoRa.onReceive(onReceive); // register the data receive interrupt
  aesLib.gen_iv(aes_iv); //generate AES IV

  // LCD init procedure
  display.begin();
  display.clearDisplay();
  display.setContrast(50);
  display.setRotation(2);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("JtagRX v0.1");
  display.println(SthImportant); // just some reminder :)
  display.display();
  delay(1000); // just to make it more like human speed - no one can read that fast
  display.clearDisplay();
  display.display();
  LoRa.receive(); // start listening on LoRa
  
}
void loop() {
  // nothing here
}

void onReceive(int packetSize) {
  Serial.print("Packet: ");
  for (int i = 0; i < packetSize; i++) { // reading and saving the packet
    rawBUFFER[i] = LoRa.read();
    Serial.print(rawBUFFER[i]);
  }
  Serial.println();
  Serial.print("with RSSI ");
  Serial.println(LoRa.packetRssi()); // signal strengh (the lower, the worse)
  // good:        0 - -80
  // ok:        -80 - -100
  // bad:      -100 - -130
  // chase it: -130 - -140
  // F: lower than -140

  // decode packet
  aesLib.decrypt((byte*)rawBUFFER, sizeof(rawBUFFER), (char*)decodedBUFFER, aes_key, sizeof(aes_key), aes_iv);
  
  // packet structure: S0000 LAT   0.000000 LON   0.000000 T00/00/2000 00:00:00
  Serial.println(decodedBUFFER);
  // parse packet
  BUFFER = decodedBUFFER;
  BUFFER.remove(0, 1);
  SAT = BUFFER.toInt();
  BUFFER.remove(0, 8);
  BUFFER.toCharArray(LAT, 12);
  BUFFER.remove(0, 15);
  BUFFER.toCharArray(LON, 12);
  BUFFER.remove(0, 13);
  BUFFER.toCharArray(DATE, 11);
  BUFFER.remove(0, 11);
  BUFFER.toCharArray(TIME, 9);

  // print to serial
  Serial.println(SAT);
  Serial.println(LAT);
  Serial.println(LON);
  Serial.println(DATE);
  Serial.println(TIME);

  // print to display
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("SAT   ");
  display.print(SAT);
  display.setCursor(0,8);
  display.print("LAT");
  display.print(LAT);
  display.setCursor(0,16);
  display.print("LON");
  display.print(LON);
  display.setCursor(0,24);
  display.print("DATE");
  display.print(DATE);
  display.setCursor(0,32);
  display.print("TIME  ");
  display.print(TIME);
  display.setCursor(0,40);
  display.print("RSSI ");
  display.print(LoRa.packetRssi());
  display.display();
}
