#pragma once

// (c) Marcel Kost

#include "bounded.hpp"
#include <Ticker.h>


template <unsigned int freq, unsigned int pin>
class PushButton
{
public:
  using callback_t = void (*)();
  
  PushButton(callback_t callback_push) : callback_push(callback_push)
  {
    pinMode(pin, INPUT);
    
    ticker.attach_ms<PushButton*>(1000 / freq, [] (PushButton* ptr) 
      {
        ptr->tick();
      }, this);
  }
  
protected:
  Ticker ticker;
  Bounded<5> counter_push; // 0-5
  callback_t callback_push = nullptr;
  
  void tick()
  {
    if (digitalRead(pin) == LOW)
    {
      if (counter_push++ == 2) // 0-3 -> release, 4-5 -> push
      {
        if (callback_push != nullptr)
          callback_push();
      }
    }
    else
      counter_push--;
  }
};

// TODO: known bug in 'hold' feature
template <unsigned int freq, unsigned int pin, unsigned int hold_thres>
class PushHoldButton : public PushButton<freq, pin>
{
public:
  using callback_t = typename PushButton<freq, pin>::callback_t;
  
  PushHoldButton(callback_t callback_push, callback_t callback_hold)
    : PushButton<freq, pin>(callback_push), callback_hold(callback_hold) {}
  
protected:
  using base = PushButton<freq, pin>;

  Bounded<freq * hold_thres / 1000> counter_hold;
  callback_t callback_hold = nullptr;
  
  void tick()
  {
    if (digitalRead(pin) == LOW)
    {
      if (base::counter_push++ == 3) // 0-3 -> release, 4-5 -> push
      {
        if (base::callback_push != nullptr)
          base::callback_push();
      }
      
      if (counter_hold++ == (freq * hold_thres / 1000) - 1)
      {
        if (callback_hold != nullptr)
          callback_hold();
      }
    }
    else
    {
      if (base::counter_push-- == 0)
        counter_hold.clear();
    }
  }
};
