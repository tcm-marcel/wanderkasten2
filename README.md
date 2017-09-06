Wanderkasten 2.0
================

This is the Arduino-based software for the Wanderkasten 2.0, a robust weather-proof beer case with illumination and Wifi using the ESP8266 chip on a ESP-01 board. 

More about the project on Hackaday: https://hackaday.io/project/27007-wanderkastern-20

## Prerequisites

* ESP8266 core for Arduino: https://github.com/esp8266/Arduino
* FastLED library: https://github.com/FastLED/FastLED
* makeEspArduino Makefile: https://github.com/plerup/makeEspArduino

Alternatively the Arduino IDE can be installed with the ESP8266 core and the FastLED library. The html file conversion however is only possible with the Makefile. 

## Setup

* clone this repository
* adapt Makefile:

```makefile
LIBS = path to esp8266/x.x.x/libraries and ../FastLED
MAKEESPARDUINO_PATH = path to makeEspArduino.mk
UPLOAD_PORT = change if not /dev/ttyUSB0
```

## Build and Flash

```bash
$ cd wanderkasten2
$ make html
$ make
$ make flash
```

## Missing content

The file koenigslied.htm has been modified before publishing to Github. Building the system is still possible. Refer to Marcel for the original file. 

## Copyright

The code contains small parts of WifiManager and Fire2012 by Mark Kriegsman (FastLED). The corresponding code lines should be annotated with copyright notices. 
