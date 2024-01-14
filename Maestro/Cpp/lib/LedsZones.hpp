#ifndef LedsZones_h
#define LedsZones_h
#pragma once
#include <stdint.h>
#include <time.h>
#include "CRGB.hpp"
#include "SongData.hpp"

/**
 * @brief Class to handle leds zones
 * 
 */
class LedsZones
{
public:
  /**
   * @brief Create a new LedsZones object
   * 
   * @param zones number of zones
   * @param leds number of leds
   */
  LedsZones(uint8_t zones, uint16_t leds);

  /**
   * @brief set the zone for a led
   * @param led led index
   * @param zone zone number
   */
  void setZone(uint16_t led, uint8_t zone);
  /**
   * @brief set the nb of different lightIDs for a zone
   * @param zone zone number
   * @param nbIDs lightIDs count
   */
  void setLightIDs(uint8_t zone, uint8_t nbIDs);
  /**
   * @brief Set animation duration
   * @param zone affected zone number
   * @param duration duration in nanoseconds
   */
  void setDuration(uint8_t zone, uint64_t duration);
  /**
   * @brief Set animation duration for a specific lightID
   * @param zone affected zone number
   * @param duration duration in nanoseconds
   * @param lightID lightIDs to affect
   * @param lightIDSize size of the lightID array
   */
  void setDuration(uint8_t zone, uint64_t duration, uint8_t *lightID, uint8_t lightIDSize);
  /**
   * @brief Set a new target color for a zone
   * @param zone affected zone number
   * @param color new target color
   */
  void setColor(uint8_t zone, CRGB color);
  /**
   * @brief Set a new target color for a specific lightID
   * @param zone affected zone number
   * @param color new target color
   * @param lightID lightIDs to affect
   * @param lightIDSize size of the lightID array
   */
  void setColor(uint8_t zone, CRGB color, uint8_t *lightID, uint8_t lightIDSize);
  /**
   * @brief Set a new target color for a zone starting from another color
   * @param zone affected zone number
   * @param from original color
   * @param target target color
   */
  void setColor(uint8_t zone, CRGB from, CRGB target);
  /**
   * @brief Set a new target color for a specific lightID starting from another color
   * @param zone affected zone number
   * @param from original color
   * @param target target color
   * @param lightID lightIDs to affect
   * @param lightIDSize size of the lightID array
   */
  void setColor(uint8_t zone, CRGB from, CRGB target, uint8_t *lightID, uint8_t lightIDSize);
  /**
   * @brief Set an animation for a zone
   * @param zone affected zone number
   * @param anim animation ID (probably 3 then)
   * @param easingID easing ID
   */
  void setAnim(uint8_t zone, uint8_t easingID);
  /**
   * @brief Set an animation for a specific lightID
   * @param zone affected zone number
   * @param anim animation ID (probably 3 then)
   * @param easingID easing ID
   * @param lightID lightIDs to affect
   * @param lightIDSize size of the lightID array
   */
  void setAnim(uint8_t zone, uint8_t easingID, uint8_t *lightID, uint8_t lightIDSize);
  /**
   * @brief get the current color for a led
   * @param led led index
   * @return CRGB current color
   */
  CRGB getColor(uint16_t led);
  /**
   * @brief Compute animations for all leds
   * @return true if at least one led has changed
   */
  bool computeAnimations();
  /**
   * @brief Get raw data and put it in data (size: leds * 3)
   */
  void getRaw(uint8_t *data);
  /**
   * @brief Get the number of leds
   * @return uint16_t number of leds
   */
  uint16_t getNbLEDs();
  /**
   * @brief Compute attributes for all leds such as zoneID
   */
  void computeAttributes();

  /**
   * @brief Register an event
   * @param event event to register
   */
  void register_event(Event event);

private:
  uint16_t m_nbLEDs;
  uint8_t m_nbZones;
  uint8_t *m_zone;     ///< zone    for this led (size: leds)
  uint8_t *m_nbLightID; ///< size: zone
  uint8_t *m_lightID;  ///< LightID for this led (size: leds)
  uint16_t *m_zoneID; ///< size: zone
  bool *m_zoneControlledByLightID;  ///< Tell wether this zone is controlled by LightID or not (size: Zones)
  uint64_t *m_durations;  ///< Animation duration in nanoseconds
  timespec *m_animEpoch; /// Begining of the animation
  CRGB *m_color;              ///< Current color
  CRGB *m_colorOriginal;      ///< Original color
  CRGB *m_colorTarget;        ///< Target color
  /**
   * @brief Easing ID applicable to this index  
   *  &nbsp;
   *  0 = linear  
   *  1 = step  
   *  40, 41 = Flash to on, flash to black  
   *  For the following : in, out, inout  
   *  &nbsp;
   *  2, 3, 4 = Sine  
   *  5, 6, 7 = Cubic  
   *  8, 9, 10 = Quint  
   *  11, 12, 13 = Circ  
   *  14, 15, 16 = Elastic  
   *  17, 18, 19 = Quad  
   *  20, 21, 22 = Quart  
   *  23, 24, 25 = Expo  
   *  26, 27, 28 = Back  
   *  29, 30, 31 = Bounce  
   *  any other value = no animation
   */
  uint8_t *m_easing;             

  CRGB easeColor(CRGB target, CRGB original, uint64_t spent, uint64_t duration, uint8_t easeID);
  bool computeAnimations(uint16_t i, timespec now);
  void setLightIDControlledZone(uint8_t zone);
  uint16_t getIndex(uint8_t zone, uint8_t lightID);
};
#endif // !LedsZones_h
