#ifndef LedsZones_h
#define LedsZones_h
#pragma once
#include <Arduino.h>
#include <FastLED.h>
class LedsZones
{
public:
  LedsZones(byte zones, int leds);
  void setZone(int led, byte zone);
  void setDuration(byte zone, float duration);
  void setColor(byte zone, CRGB color);
  void setColor(byte zone, CRGB from, CRGB target);
  void setAnim(byte zone, byte anim);
  void setAnim(byte zone, byte anim, byte easingID);
  CRGB getColor(int led);
  bool computeAnimations();

private:
  byte*  m_zone;
  float* m_durations; ///< Animation duration in milliseconds
  unsigned long* m_animEpoch; ///< Begining of the animation in milliseconds
  byte*  m_anim;    ///< 0 = none, 1 = flash, 2 = flash + fade to black, 3 = chroma gradient
  byte*  m_easing;  /**< Easing ID
                         0 = linear
                         40, 41 = Flash to on, flash to black
                         For the following : in, out, inout
                         1, 2, 3 = Sine
                         4, 5, 6 = Cubic
                         7, 8, 9 = Quint
                         10, 11, 12 = Circ
                         13, 14, 15 = Elastic
                         16, 17, 18 = Quad
                         19, 20, 21 = Quart
                         22, 23, 24 = Expo
                         25, 26, 27 = Back
                         28, 29, 30 = Bounce
                         */
  CRGB*  m_color;   ///< Current color
  CRGB*  m_colorOriginal; ///< Original color
  CRGB*  m_colorTarget; ///< Pointer to the target color
  int    m_nbZones;

  CRGB dim(CRGB colortodim);   // Doing it with a function like this allows to copy the color object therfore not affecting the original color
  CRGB easeColor(CRGB target, CRGB original, float spent, float duration, byte easeID);
};
#endif // !LedsZones_h