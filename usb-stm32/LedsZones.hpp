#ifndef LedsZones_h
#define LedsZones_h
#pragma once
#include <Arduino.h>
#include <FastLED.h>

class LedsZones
{
public:
  LedsZones(byte zones, u_int16_t leds);
  void setZone(u_int16_t led, byte zone);
  void setLightIDs(byte zone, byte nbIDs);
  void setDuration(byte zone, float duration);
  void setDuration(byte zone, float duration, byte *lightID, byte lightIDSize);
  void setColor(byte zone, CRGB color);
  void setColor(byte zone, CRGB color, byte *lightID, byte lightIDSize);
  void setColor(byte zone, CRGB from, CRGB target);
  void setColor(byte zone, CRGB from, CRGB target, byte *lightID, byte lightIDSize);
  void setAnim(byte zone, byte anim);
  void setAnim(byte zone, byte anim, byte *lightID, byte lightIDSize);
  void setAnim(byte zone, byte anim, byte easingID);
  void setAnim(byte zone, byte anim, byte easingID, byte *lightID, byte lightIDSize);
  CRGB getColor(u_int16_t led);
  bool computeAnimations();
  void computeAttributes();

private:
  uint m_nbLEDs;
  byte m_nbZones;
  byte *m_zone;     ///< zone    for this led (size: leds)
  byte *m_nbLightID; ///< size: zone
  byte *m_lightID;  ///< LightID for this led (size: leds)
  uint *m_zoneID; ///< size: zone
  bool *m_zoneControlledByLightID;  ///< Tell wether this zone is controlled by LightID or not (size: Zones)
  float *m_durations;  ///< Animation duration in milliseconds
  unsigned long *m_animEpoch; ///< Begining of the animation in milliseconds
  byte *m_anim;               ///< 0 = none, 1 = flash, 2 = flash + fade to black, 3 = chroma gradient
  CRGB *m_color;              ///< Current color
  CRGB *m_colorOriginal;      ///< Original color
  CRGB *m_colorTarget;        ///< Target color
  byte *m_easing;             /**< Easing ID applicable to this index
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


  CRGB dim(CRGB colortodim); // Doing it with a function like this allows to copy the color object therfore not affecting the original color
  CRGB easeColor(CRGB target, CRGB original, float spent, float duration, byte easeID);
  bool computeAnimations(u_int16_t i, unsigned long now);
  void setLightIDControlledZone(byte zone);
  uint getIndex(byte zone, byte lightID);
};
#endif // !LedsZones_h
