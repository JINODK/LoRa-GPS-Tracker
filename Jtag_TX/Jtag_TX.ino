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
  // Global WiFi constructors are not called yet (global class instances like WiFi, Serial... are not yet initialized)..
  // No global object methods or C++ exceptions can be called in here!
  // The below is a static class method, which is similar to a function, so it's ok.
  ESP8266WiFiClass::preinitWiFiOff();
}

#include <TinyGPS++.h>
#include <LoRa.h>
#include <SPI.h>
#include <AESLib.h> // WARNING: only use version 2.1.9 of this library as it's the only one that worked

long i = 0;
char BUFFER[64];
char encodedBUFFER[128];
byte aes_key[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
byte aes_iv[N_BLOCK] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
char SthImportant[32];

TinyGPSPlus gps;
AESLib aesLib;

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Made by JINODK under MIT license");
  Serial.print("Booting... ");
  
  // Wiring config for SX1276 - NODEMCU
  // NSS   - D8
  // RESET - D4
  // DIO0  - D1
  LoRa.setPins(D8, D4, D1);
  Serial.println(LoRa.begin(868E6));
  LoRa.setSyncWord(0xF4);
  aesLib.gen_iv(aes_iv);
}

void loop() {
  while (Serial.available() != 0) if (gps.encode(Serial.read())) GPSdecode();
  if (millis() - i > 1000) {
    aesLib.encrypt((byte*)BUFFER, sizeof(BUFFER), (char*)encodedBUFFER, aes_key, sizeof(aes_key), aes_iv);
    LoRa.beginPacket();
    LoRa.print(encodedBUFFER);
    LoRa.endPacket();
    Serial.println(BUFFER);
    Serial.println(encodedBUFFER);
    i = millis();
  }
}

void GPSdecode() {
  sprintf(BUFFER, "S%04d LAT%11f LON%11f T%02d/%02d/%02d %02d:%02d:%02d", gps.satellites.value(), gps.location.lat(), gps.location.lng(), gps.date.day(), gps.date.month(), gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second());
}
