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
#include <LoRa.h>
#include <SPI.h>
#include <AESLib.h>
#include <Adafruit_PCD8544.h>

byte i = 0;
char rawBUFFER[128]; // double the length of decodedBUFFER for safety
char decodedBUFFER[64];
String BUFFER;
byte aes_key[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
byte aes_iv[N_BLOCK] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
char SthImportant[32];
byte SAT;
char LAT[11], LON[11];
char DATETIME[20];

AESLib aesLib;
// Hardware SPI:
// CLK - CLK
// DIN - MOSI
// D/C - D3
// CS  - D4
// RST - NC
Adafruit_PCD8544 display = Adafruit_PCD8544(D3, D4, -1);


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Made by JINODK under MIT license");
  Serial.println("booting...");
  LoRa.setPins(D8, D0, D1); // NSS RESET DIO0
  Serial.println(LoRa.begin(868E6));
  LoRa.setSyncWord(0xF4);
  LoRa.onReceive(onReceive);
  aesLib.gen_iv(aes_iv);
  display.begin();
  display.clearDisplay();
  display.setContrast(50);
  display.setRotation(2);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("JtagRX v0.1");
  display.println(SthImportant);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();
  LoRa.receive(); // start listening
  
}
void loop() {
  // nothing here
}

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received packet '");
  // read packet
  for (int i = 0; i < packetSize; i++) {
    rawBUFFER[i] = LoRa.read();
    Serial.print(rawBUFFER[i]);
  }
  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());

  //prase packet
  aesLib.decrypt((byte*)rawBUFFER, sizeof(rawBUFFER), (char*)decodedBUFFER, aes_key, sizeof(aes_key), aes_iv);
  //S000x LAT  xx.xxxxxx LON xxx.xxxxxx Txx/xx/2021 xx:xx:xx
  Serial.println(decodedBUFFER);
  BUFFER = decodedBUFFER;
  BUFFER.remove(0, 1);
  SAT = BUFFER.toInt();
  BUFFER.remove(0, 8);
  BUFFER.toCharArray(LAT, 11);
  BUFFER.remove(0, 15);
  BUFFER.toCharArray(LON, 11);
  BUFFER.remove(0, 13);
  BUFFER.toCharArray(DATETIME, 20);
  Serial.println(SAT);
  Serial.println(LAT);
  Serial.println(LON);
  Serial.println(DATETIME);
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(SAT);
  display.setCursor(0,8);
  display.print(LAT);
  display.setCursor(0,16);
  display.print(LON);
  display.setCursor(0,24);
  display.print(DATETIME);
  display.setCursor(0,40);
  display.print(LoRa.packetRssi());
  display.display();
}
