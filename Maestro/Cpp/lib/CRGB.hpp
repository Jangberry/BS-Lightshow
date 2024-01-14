#ifndef CRGB_HPP
#define CRGB_HPP
#pragma once

#include <cstdint>

union CRGB
{
  struct
  {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };
  uint32_t raw;

  CRGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
  CRGB() : r(0), g(0), b(0) {}

  CRGB operator+(const CRGB &c) const
  {
    return CRGB(r + c.r, g + c.g, b + c.b);
  }

  CRGB operator-(const CRGB &c) const
  {
    return CRGB(r - c.r, g - c.g, b - c.b);
  }

  CRGB operator*(const float &f) const
  {
    return CRGB(r * f, g * f, b * f);
  }

  CRGB operator/(const float &f) const
  {
    return CRGB(r / f, g / f, b / f);
  }

  CRGB operator=(const CRGB &c)
  {
    r = c.r;
    g = c.g;
    b = c.b;
    return *this;
  }

  CRGB operator=(const int &c)
  {
    raw = c;
    return *this;
  }

  CRGB operator+=(const CRGB &c)
  {
    r += c.r;
    g += c.g;
    b += c.b;
    return *this;
  }

  CRGB operator-=(const CRGB &c)
  {
    r -= c.r;
    g -= c.g;
    b -= c.b;
    return *this;
  }

  CRGB operator*=(const float &f)
  {
    r *= f;
    g *= f;
    b *= f;
    return *this;
  }

  CRGB operator/=(const float &f)
  {
    r /= f;
    g /= f;
    b /= f;
    return *this;
  }

  bool operator==(const CRGB &c) const
  {
    return (r == c.r && g == c.g && b == c.b);
  }

  bool operator!=(const CRGB &c) const
  {
    return (r != c.r || g != c.g || b != c.b);
  }
};

#endif
