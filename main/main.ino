// (c) Marcel Kost

//#define DEBUG

#ifdef DEBUG
  #pragma message DEBUG enabled!
  
  //#include <GDBStub.h>
#endif

#include "config.hpp"
#include "button.hpp"
#include "modes.hpp"
#include "webinterface.hpp"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include <DNSServer.h>
#include <Ticker.h>

extern "C" {
  #include "user_interface.h"
}

#include <memory>
#include <cinttypes>

void setup_leds();
void setup_wifi_landscape();
void shutdown_wifi_landscape();
void refresh();
void auto_rainbow_change();
void mode_next();
void mode_change(unsigned int id);

array_t led_array;
uint8_t wifi_connected_devices = 0;
bool wifi_landscape_active = false;
bool auto_rainbow_active = false;

std::unique_ptr<Mode> active_mode;
Cycled<modes_num> counter_modes;

Ticker refresh_ticker;
Ticker shutdown_ticker;
Ticker auto_rainbow_ticker;
PushButton<40, 2> button(
  mode_next
);
DNSServer dnsserver;
WebInterface webinterface(
  [] (CRGB c) { active_mode->color =  c; }, 
  [] (uint8_t v) { active_mode->value =  v; }, 
  mode_next, 
  shutdown_wifi_landscape
);


void setup()
{
  #ifdef DEBUG
    Serial.begin(115200);
  #endif  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  setup_leds();
  setup_wifi_landscape();
  
  // change to random state
  uint32_t time = system_get_time();
  random16_set_seed((uint16_t)time);
  mode_change(random8(modes_random_num));
}

inline void setup_leds()
{
  FastLED.addLeds<NEOPIXEL, 0>(led_array, led_array.size()).setCorrection(TypicalLEDStrip);
  refresh_ticker.attach_ms(1000 / refresh_freq, refresh);
  
  auto_rainbow_ticker.attach(60, auto_rainbow_change);
}

inline void setup_wifi_landscape()
{
  // start AP
  WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(wifi_ssid);
  
  // start dns server
  dnsserver.setErrorReplyCode(DNSReplyCode::NoError);
  dnsserver.start(dns_port, "*", WiFi.softAPIP());
  
  // start webserver
  webinterface.setup();
  
  // setup shutdown ticker
  shutdown_ticker.once(30, [] () {
    if (wifi_connected_devices == 0)
      shutdown_wifi_landscape();
  });
  
  wifi_landscape_active = true;
}

inline void shutdown_wifi_landscape()
{
  // stop webserver
  webinterface.shutdown();
  
  // stop dns server
  dnsserver.stop();
  
  // stop AP
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin(0); // WiFi sleep forever
  
  wifi_landscape_active = false;
  
  // activate automatic rainbow change
  auto_rainbow_active = true;
}

void refresh()
{
  if (active_mode != nullptr)
    active_mode->refresh(led_array);
  else
    led_array.fill_solid(CRGB::Black);
  
  FastLED.show();
}

void auto_rainbow_change()
{
  if (auto_rainbow_active && active_mode != nullptr)
  {
    CHSV c = rgb2hsv_approximate(active_mode->color);
    c.hue = c.hue + 10;
    active_mode->color = c;
    
  }
}

void mode_next()
{
  counter_modes++;
  mode_change(counter_modes());
}

void mode_change(unsigned int id)
{
  std::unique_ptr<Mode> new_mode;
  
  switch (id)
  {
    case 0:
    default:
      new_mode.reset(static_cast<Mode*>(new ClownRainbowMode(128)));
      break;
    
    case 1:
      new_mode.reset(static_cast<Mode*>(new ColorFadeMode(color_t::Yellow, 128)));
      break;
    
    case 2:
      new_mode.reset(static_cast<Mode*>(new SlowSpinMode(color_t::Yellow, 0)));
      break;
    
    case 3:
      new_mode.reset(static_cast<Mode*>(new FastSpinMode(color_t::Yellow, 96)));
      break;
    
    case 4:
      new_mode.reset(static_cast<Mode*>(new KastenFireMode(140)));
      break;
    
    case 5:
      new_mode.reset(static_cast<Mode*>(new StroboMode(64)));
      break;
  }
  
  active_mode = TransitionMode::create(std::move(new_mode), active_mode);
}

void loop()
{
  if (wifi_landscape_active)
  {
    wifi_connected_devices = std::max(wifi_connected_devices, WiFi.softAPgetStationNum());
    
    dnsserver.processNextRequest();
    webinterface.handle();
  }
}
