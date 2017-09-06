#pragma once

// (c) Marcel Kost

#include "config.hpp"
#include "bounded.hpp"

#include "ESP8266WiFi.h"
#include <FastLED.h>


class Mode
{
public:
  color_t color;
  uint8_t value;
  
  Mode() {}
  Mode(color_t color) : color(color) {}
  Mode(uint8_t value) : value(value) {}
  Mode(color_t color, uint8_t value) : color(color), value(value) {}
  Mode(color_t::HTMLColorCode color) : color(CRGB(color)) {}
  virtual void refresh(array_t& array) = 0;
};


class StaticColorMode : public Mode
{
public:
  using Mode::Mode;
  
  void refresh(array_t& array)
  {
    array.fill_solid(color);
  }
};


class StroboMode : public Mode
{
public:
  using Mode::Mode;
  
  void refresh(array_t& array)
  {
    state = (state + 1) % (value/4 + 1);
    
    if (state == 0)
      array.fill_solid(color_t::White);
    else
      array.fill_solid(color_t::Black);
  }

private:
  uint8_t state = 0;
};


class FastSpinMode : public Mode
{
public:
  using Mode::Mode;
  
  void refresh(array_t& array)
  {
    array.fill_solid(CRGB::Black);
    
    for (int i = 0; i < size; i++)
    {
      array[(state() + i) % led_size] = color;
    }
    
    unsigned int speed = (value / 32) + 1;
    
    for (int i = 0; i < speed; i++)
      state++;
  }

private:
  static const unsigned int size = 3;
  Cycled<led_size-size> state = 0;
};


class SlowSpinMode : public Mode
{
public:
  using Mode::Mode;
  
  void refresh(array_t& array)
  {
    array_t sub;
    sub(0, (sub_size/2)-1).fill_gradient_RGB(CRGB::Black, color);
    //sub(sub_size/2, sub_size).fill_solid(color);
    sub(sub_size/2, sub_size-1).fill_solid(CRGB::Black);
    
    for (int i = 0; i < led_size; i++)
    {
      unsigned int offset = ((state() / 4) + i) % sub_size;
      array[i] = sub[offset];
    }
    
    unsigned int speed = (value / 32) + 1;
    
    for (int i = 0; i < speed; i++)
      state++;
  }

private:
  static const unsigned int size = 3;
  static const unsigned int sub_size = (led_size / 4) + 1;
  Cycled<led_size> state = 0;
};


class NoiseMode : public Mode
{
public:
  NoiseMode(CRGBPalette16 palette) : Mode(), current_palette(palette) {};

  void refresh(array_t& array)
  {
    for (int i = 0; i < led_size; i++)
    {
      uint8_t index = inoise8(i * scale, state + i * scale) % 255;
      array[i] = ColorFromPalette(current_palette, index, 255, LINEARBLEND);
    }
    
    state += beatsin8(1, 1, 4);
  }

private:
  static const unsigned int scale = 5;
  
  CRGBPalette16 current_palette;
  uint16_t state;
};


class ClownRainbowMode : public Mode
{
public:
  using Mode::Mode;

  void refresh(array_t& array)
  {
    for (int i = 0; i < led_size; i++)
    {
      uint8_t index = inoise8(i * scale, noise_state + i * scale) % 255;
      array[i] = ColorFromPalette(current_palette, index, 255, LINEARBLEND);
    }
    
    uint8_t beatsin_high = value / 32;
    
    noise_state += beatsin8(1, 1, beatsin_high);
    nblendPaletteTowardPalette(current_palette, target_palette, max_changes);
    
    if (palette_state() == 0)
    {
      target_palette = CRGBPalette16(
        CHSV(random8(), 255, random8(128, 255)), 
        CHSV(random8(), 255, random8(128, 255)), 
        CHSV(random8(), 192, random8(128, 255)), 
        CHSV(random8(), 255, random8(128, 255))
      );
    }
    
    palette_state++;
  }

private:
  static const unsigned int scale = 16;
  static const unsigned int max_changes = 12;
  
  CRGBPalette16 current_palette;
  CRGBPalette16 target_palette;
  uint16_t noise_state;
  Cycled<600> palette_state;
};


class ColorFadeMode : public Mode
{
public:
  using Mode::Mode;
  
  void refresh(array_t& array)
  {
    for (int i = 0; i < led_size; i++)
    {
      uint8_t index = inoise8(i * scale, noise_state + i * scale) % 255;
      array[i] = ColorFromPalette(palette, index, 255, LINEARBLEND);
    }
    
    uint8_t beatsin_high = value / 32;
    noise_state += beatsin8(1, 1, beatsin_high);
    
    CHSV c = rgb2hsv_approximate(color);
    
    palette = CRGBPalette16(
      CHSV(c.hue + 20, 255, 255), 
      CHSV(c.hue - 20, 255, 255), 
      CHSV(c.hue, 255, 3)
    );
  }

private:
  static const unsigned int scale = 16;
  
  CRGBPalette16 palette;
  uint16_t noise_state;
};


class TransitionMode : public Mode
{
public:
  TransitionMode(std::unique_ptr<Mode> old_mode, std::unique_ptr<Mode> new_mode, 
    std::unique_ptr<Mode>& dest_ptr)
    : old_mode(std::move(old_mode)), new_mode(std::move(new_mode)), 
      dest_ptr(dest_ptr) {}
  
  void refresh(array_t& dest_array)
  {
    array_t overlay;
    
    if (old_mode != nullptr)
      old_mode->refresh(dest_array);
    else
      dest_array.fill_solid(CRGB::Black);
    
    if (new_mode != nullptr)
      new_mode->refresh(overlay);
    else
      overlay.fill_solid(CRGB::Black);
    
    dest_array.nblend(overlay, fract);
    
    fract = std::min(fract + (256 / 60), 255);
    
    if (fract == 255)
      // swap TransitionMode to new_mode
      dest_ptr = std::move(new_mode);
  }
  
  static std::unique_ptr<Mode> create(std::unique_ptr<Mode> new_mode, std::unique_ptr<Mode>& dest_ptr)
  {
    return std::unique_ptr<Mode>(static_cast<Mode*>(new TransitionMode(std::move(dest_ptr), std::move(new_mode), dest_ptr)));
  }
  
private:
  std::unique_ptr<Mode> old_mode;
  std::unique_ptr<Mode> new_mode;
  std::unique_ptr<Mode>& dest_ptr;
  uint8_t fract = 0;
};


// originally Fire2012 by Mark Kriegsman, July 2012
// Kasten version that calculates one fire and distributes it to all 4 sides
class KastenFireMode : public Mode
{
public:
  using Mode::Mode;
  
  void refresh(array_t& array)
  {
    // SPARKING: What chance (out of 255) is there that a new spark will be lit?
    // Higher chance = more roaring fire.  Lower chance = more flickery fire.
    // Default 120, suggested range 50-200.
    unsigned int sparking = map(value, 0, 255, 50, 200);
    
    // Step 1.  Cool down every cell a little
    for( int i = 0; i < fire_size; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((cooling * 10) / fire_size) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= fire_size - 1; k >= fire_size/2 + 1; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    for( int k= 0; k < fire_size/2 - 1; k++) {
      heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < sparking ) {
      int y = (fire_size/2) - 2 + random8(4);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for(int i = 0; i <= 14; i++) {
      int j = 0 + i;
      array[j] = HeatColor(heat[i]);
    }
    for(int i = 0; i <= 13; i++) {
      int j = 14 + i;
      array[j] = HeatColor(heat[i]);
    }
    for(int i = 0; i < 14; i++) {
      int j = 27 + i;
      array[j] = HeatColor(heat[i]);
    }
    for(int i = 0; i < 13; i++) {
      int j = 41 + i;
      array[j] = HeatColor(heat[i]);
    }
  }
  
private:
  static const unsigned int fire_size = 14;
  
  // There are two main parameters you can play with to control the look and
  // feel of your fire: COOLING (used in step 1 above), and SPARKING (used
  // in step 3 above).
  uint8_t heat[fire_size];
  
  // COOLING: How much does the air cool as it rises?
  // Less cooling = taller flames.  More cooling = shorter flames.
  // Default 50, suggested range 20-100 
  const unsigned int cooling = 85;
};
