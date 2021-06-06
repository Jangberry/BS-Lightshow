#ifndef LedsZones_h
#define LedsZones_h
#include <Arduino.h>
#include <FastLED.h>
class LedsZones
{
public:
  LedsZones(byte zones, int leds);
  void setZone(int led, byte zone);
  void setTau(byte zone, float tau);
  void setColor(byte zone, CRGB *color);
  void setColorNoPointer(byte zone, CRGB color);
  void setAnim(byte zone, byte anim);
  void wipeFlash();
  byte getZone(int led);
  float getTau(int led);
  byte getAnim(int led);
  CRGB getColor(int led);

private:
  byte*  m_zone;
  float* m_taus;
  byte*  m_anim;    // 0 = none, 1 = flash, 2 = flash + fade to black
  CRGB* *m_color;
  CRGB*  m_colorNoPointer;
  int    m_nbZones;

  CRGB dim(CRGB colortodim);   // Doing it with a function like this allows to copy the color object therfore not affecting the original color
};
#endif // !LedsZones_h