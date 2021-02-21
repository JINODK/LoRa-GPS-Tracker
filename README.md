# LoRa-GPS-Tracker

ESP8266 based LoRa GPS tracker using Semtech SX1276 LoRa module, Ublox NEO 6M GPS module and Nokia 5110 LCD for displaying the infomation

# Setup

## Library

Download these library:

- TinyGPS++: http://arduiniana.org/libraries/tinygpsplus
- LoRa: by Sandeep Mistry on Arduino library manager
- AESlib: by Matej Sychra on Arduino library manager
- PCD8544: by Adafruit for Nokia 5110 lcd display on Arduino library manager

*Do note that I use Adafruit display library so that anyone can replace that with SSD1306 OLED display library with minimal change to codebase*

## Wiring

### TX module

```
GPS TX -- ESP8266 RX

SX1276 module
MISO -- D6 - GPIO12 - HMISO
MOSI -- D7 - GPIO13 - HMOSI
CLK  -- D5 - GPIO14 - HSCLK
NSS  -- D8 - GPIO15 - HCS
RST  -- D4 - GPIO2
DIO0 -- D1 - GPIO5
```

### RX module

```
Nokia 5110 display module
CLK  -- D5 - GPIO14 - HSCLK
DIN  -- D7 - GPIO13 - HMOSI
D/C  -- D3 - GPIO0
CS   -- D4 - GPIO2
RST  -- NC - Not connected

SX1276 module
MISO -- D6 - GPIO12 - HMISO
MOSI -- D7 - GPIO13 - HMOSI
CLK  -- D5 - GPIO14 - HSCLK
NSS  -- D8 - GPIO15 - HCS
RST  -- D0 - GPIO16
DIO0 -- D1 - GPIO5
```
