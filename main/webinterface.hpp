#pragma once

// (c) Marcel Kost

#include "ESP8266WiFi.h"
#include <FastLED.h>
#include <ESP8266WebServer.h>
#include <pgmspace.h>

// include html files as byte array via header
// run 'make html' to update the files
#include "webinterface_html.h"
#include "koenigslied_html.h"

class WebInterface
{
public:
  using callback_t = void (*)();
  using callback_color_t = void (*)(CRGB);
  using callback_value_t = void (*)(uint8_t);
  
  ESP8266WebServer server;
  
  WebInterface(callback_color_t callback_color, callback_value_t callback_value, callback_t callback_mode_next, callback_t callback_wifi_off) 
  : server(80), callback_color(callback_color), callback_value(callback_value), callback_mode_next(callback_mode_next), callback_wifi_off(callback_wifi_off) {}
  
  void setup()
  {
    
    server.on("/", std::bind(&WebInterface::show_interface, this));
    
    server.on("/koenigslied", [this] () {
      server.send(200, "text/html", String(FPSTR(koenigslied_html)));
      server.client().stop();
    });
    
    server.on("/api/mode/color", [this](){
      uint8_t r = server.arg("r").toInt();
      uint8_t g = server.arg("g").toInt();
      uint8_t b = server.arg("b").toInt();
      
      if (callback_color != nullptr)
        callback_color(CRGB(r, g, b));
      
      server.send(200);
    });
    
    server.on("/api/mode/value", [this](){
      uint8_t v = server.arg("v").toInt();
      
      if (callback_value != nullptr)
        callback_value(v);
      
      server.send(200);
    });
    
    server.on("/api/mode/next", [this](){
      if (callback_mode_next != nullptr)
        callback_mode_next();
      
      server.send(200);
    });
    
    server.on("/api/wifi/off", [this](){
      if (callback_wifi_off != nullptr)
        callback_wifi_off();
      
      server.send(200);
    });
    
    // add listeners for wifi opener portals
    // copied from WifiManager
    server.on("/", std::bind(&WebInterface::show_interface, this));
    server.on("/fwlink", std::bind(&WebInterface::show_interface, this));  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server.onNotFound(std::bind(&WebInterface::handleNotFound, this));

    server.begin();
    Serial.println("started Webserver");
  }
  
  void shutdown()
  {
    server.stop();
  }
  
  void handle()
  {
    server.handleClient();
  }
  
private:
  callback_t callback_mode_next = nullptr;
  callback_color_t callback_color = nullptr;
  callback_value_t callback_value = nullptr;
  callback_t callback_wifi_off = nullptr;
  
  void show_interface()
  {
    server.send(200, "text/html", String(FPSTR(webinterface_html)));
    server.client().stop();
  }
  
  // --- functions copied from WifiManager ---
  
  boolean isIp(String str) {
    for (int i = 0; i < str.length(); i++) {
      int c = str.charAt(i);
      if (c != '.' && (c < '0' || c > '9')) {
        return false;
      }
    }
    return true;
  }
  
  String toStringIp(IPAddress ip)
  {
    String res = "";
    
    for (int i = 0; i < 3; i++) {
      res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    
    return res;
  }
  
  void handleNotFound() {
    if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
      return;
    }
    
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for ( uint8_t i = 0; i < server.args(); i++ ) {
      message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
    }
    
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send ( 404, "text/plain", message );
  }
  
  boolean captivePortal()
  {
    if (!isIp(server.hostHeader()))
    {
      server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
      server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
      server.client().stop(); // Stop is needed because we sent no content length
      
      return true;
    }
    
    return false;
  }
};
