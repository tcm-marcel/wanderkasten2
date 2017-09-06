#pragma once

#include <FastLED.h>


const unsigned int led_size = 54; // 2*13 + 2*14
const unsigned int refresh_freq = 60;
const unsigned int modes_num = 6;
const unsigned int modes_random_num = 3;

const char* wifi_ssid = "Wanderkasten";

using color_t = CRGB;
using array_t = CRGBArray<led_size>;

const uint8_t dns_port = 53;
