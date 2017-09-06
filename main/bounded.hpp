#pragma once

// (c) Marcel Kost


// counts from 0 to bound (including)
template <unsigned int bound>
class Bounded
{
public:
  using bound_t = unsigned int;
  
  Bounded() {}
  Bounded(bound_t init) : value(init) {}
  
  bound_t operator++(int)
  {
    bound_t tmp = value;
    value = min(value + 1, bound);
    return tmp;
  }
  
  bound_t operator--(int)
  {
    bound_t tmp = value;
    if (value > 0)
      value--;
    return tmp;
  }
  
  bound_t operator()() const
  {
    return value;
  }
  
  bound_t operator()(bound_t v)
  {
    value = min(v, bound);
    return value;
  }
  
  bound_t clear()
  {
    return value = 0;
  }
  
private:
  bound_t value = 0;
};


// counts modulo size
template <unsigned int size>
class Cycled
{
public:
  using cycle_t = unsigned int;
  
  Cycled() {}
  Cycled(cycle_t init) : value(init) {}
  
  cycle_t operator++(int)
  {
    cycle_t tmp = value;
    value = (value + 1) % size;
    return tmp;
  }
  
  cycle_t operator--(int)
  {
    cycle_t tmp = value;
    if (value > 0)
      value--;
    else
      value = size - 1;
    return tmp;
  }
  
  cycle_t operator()() const
  {
    return value;
  }
  
  cycle_t operator()(cycle_t v)
  {
    value = value % size;
    return value;
  }
  
  cycle_t get_with_offset(cycle_t o)
  {
    return (value + o) % size;
  }
  
  cycle_t clear()
  {
    return value = 0;
  }
  
private:
  cycle_t value = 0;
};
