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

#include <TinyGPS++.h>
#include <LoRa.h>
#include <SPI.h>
#include <AESLib.h> // WARNING: only use version 2.1.9 of this library as it's the only one that worked

long i = 0; // timer counter
char BUFFER[64]; // this is where we store raw data before encrypt it
char encodedBUFFER[128]; // double the length of BUFFER for safety
byte aes_key[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}; // 128-bit key, USE YOUR OWN KEY
byte aes_iv[N_BLOCK] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}; // 128-bit IV, USE YOUR OWN KEY FOR SECURITY
char SthImportant[32];

TinyGPSPlus gps;
AESLib aesLib;

void setup() {
  Serial.begin(9600); // open serial port at 9600 baud for gps communication, and debugging (if needed), can use other pin with Software Serial,
  // wiring for GPS module: GPS TX to NODEMCU RX only for receiving gps data, NODEMCU TX is for debugging only
  Serial.println();
  base64_decode(SthImportant, "TWFkZSBieSBKSU5PREsgdW5kZXIgTUlUIGxpY2Vuc2U", 43);
  Serial.println(SthImportant); // just some reminder :)
  Serial.print("Booting... ");
  
  // Wiring config for SX1276 - NODEMCU
  // NSS   - D8
  // RESET - D4
  // DIO0  - D1
  // for more info about configuring LoRa module, visit https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md#setup
  LoRa.setPins(D8, D4, D1);
  Serial.println(LoRa.begin(868E6)); // 868mhz is a stable freq for Semtech SX1276, print out the init return
  LoRa.setSyncWord(0xF4); // just like preamble but can be changed for more security
  aesLib.gen_iv(aes_iv); //generate AES IV
}

void loop() {
  while (Serial.available() != 0) if (gps.encode(Serial.read())) GPSdecode(); // if there is data comming from the GPS module, decode it
  if (millis() - i > 1000) { // send data once every second or so
    aesLib.encrypt((byte*)BUFFER, sizeof(BUFFER), (char*)encodedBUFFER, aes_key, sizeof(aes_key), aes_iv); // encrypt the data with AES standard
    LoRa.beginPacket();
    LoRa.print(encodedBUFFER); // encoded data
    LoRa.endPacket();
    Serial.println(BUFFER); // human readable data
    Serial.println(encodedBUFFER); // encoded data for comparison
    i = millis();
  }
}

void GPSdecode() { // parse GPS data to more human readable one
  sprintf(BUFFER, "S%04d LAT%11f LON%11f T%02d/%02d/%02d %02d:%02d:%02d", gps.satellites.value(), gps.location.lat(), gps.location.lng(), gps.date.day(), gps.date.month(), gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second());
}
