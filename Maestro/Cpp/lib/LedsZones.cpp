#include <stdint.h>
#include <cmath>
#include <time.h>

#include "LedsZones.hpp"
#include "easing.hpp"
#include "SongData.hpp"
#include <stdexcept>

LedsZones::LedsZones(uint8_t zones, uint16_t leds)
{
    m_nbZones = zones;
    m_nbLEDs = leds;
    m_zone = new uint8_t[leds];
    m_animEpoch = new timespec[leds];
    m_easing = new uint8_t[leds];
    m_durations = new uint64_t[leds];
    m_color = new CRGB[leds];
    m_colorTarget = new CRGB[leds];
    m_colorOriginal = new CRGB[leds];
    m_nbLightID = new uint8_t[zones];
    m_lightID = new uint8_t[leds];
    m_zoneID = new uint16_t[zones];
    m_zoneControlledByLightID = new bool[zones];
}

void LedsZones::setZone(uint16_t led, uint8_t zone)
{
    m_zone[led] = zone;
}

void LedsZones::getRaw(uint8_t *data)
{
    for (uint16_t i = 0; i < m_nbLEDs; i++)
    {
        data[i * 3] = m_color[i].r;
        data[i * 3 + 1] = m_color[i].g;
        data[i * 3 + 2] = m_color[i].b;
    }
}

uint16_t LedsZones::getNbLEDs()
{
    return m_nbLEDs;
}

void LedsZones::computeAttributes()
{
    uint16_t nb_LEDsInZone[m_nbZones];
    for (uint16_t i = 0; i < m_nbZones; i++)
        nb_LEDsInZone[i] = 0; // Init to 0, may cause segfaults if not done
    for (uint16_t i = 0; i < m_nbLEDs; i++)
        nb_LEDsInZone[m_zone[i]] = nb_LEDsInZone[m_zone[i]] + 1;
    for (uint16_t i = 0; i < m_nbZones; i++)
    {
        m_zoneID[i] = i == 0 ? 0 : m_zoneID[i - 1] + nb_LEDsInZone[i - 1];
        m_colorOriginal[m_zoneID[i]] = 0;
        m_zoneControlledByLightID[i] = false;
    }
}

void LedsZones::register_event(Event event)
{
    CRGB color;
    uint64_t duration;
    ChromaEvent::Easing easing;

    switch (event.value)
    {
    case Event::Value::OFF:
        color = CRGB(0, 0, 0);
        duration = 0;
        easing = ChromaEvent::Easing::STEP;
        break;
    case Event::Value::ON1:
        color = CRGB(0, 0, 255);
        duration = 0;
        easing = ChromaEvent::Easing::STEP;
        break;
    case Event::Value::ON2:
        color = CRGB(255, 0, 0);
        duration = 0;
        easing = ChromaEvent::Easing::STEP;
        break;
    case Event::Value::ON3:
        color = CRGB(255, 255, 255);
        duration = 0;
        easing = ChromaEvent::Easing::STEP;
        break;
    case Event::Value::FLASH1:
        color = CRGB(0, 0, 255);
        duration = 600000000;
        easing = ChromaEvent::Easing::FLASH;
        break;
    case Event::Value::FLASH2:
        color = CRGB(255, 0, 0);
        duration = 600000000;
        easing = ChromaEvent::Easing::FLASH;
        break;
    case Event::Value::FLASH3:
        color = CRGB(255, 255, 255);
        duration = 600000000;
        easing = ChromaEvent::Easing::FLASH;
        break;
    case Event::Value::FADE1:
        color = CRGB(0, 0, 255);
        duration = 2000000000;
        easing = ChromaEvent::Easing::FADE;
        break;
    case Event::Value::FADE2:
        color = CRGB(255, 0, 0);
        duration = 2000000000;
        easing = ChromaEvent::Easing::FADE;
        break;
    case Event::Value::FADE3:
        color = CRGB(255, 255, 255);
        duration = 2000000000;
        easing = ChromaEvent::Easing::FADE;
        break;
        // TODO case Event::Value::TRANSITION: // (V3) no need to reimplement color change, original color is taken from the previous state by setColor
    }
    setAnim(event.type, easing);
    setColor(event.type, color);
    setDuration(event.type, duration);
}

void LedsZones::setLightIDs(uint8_t zone, uint8_t nbIDs)
{
    if (zone >= m_nbZones)
        return;
    int nb_LEDs = zone == m_nbZones - 1 ? m_nbLEDs - m_zoneID[zone] : m_zoneID[zone + 1] - m_zoneID[zone];
    if (nbIDs > nb_LEDs)
    {
        m_nbLightID[zone] = 0;
        for (int i = 0; i < m_nbLEDs; i++)
            if (m_zone[i] == zone)
                m_lightID[i] = 0;
    }
    else
    {
        m_nbLightID[zone] = nbIDs;
        int nb_associated = 0;
        for (int i = 0; i < m_nbLEDs; i++)
            if (m_zone[i] == zone)
            {
                m_lightID[i] = nb_associated++ * nbIDs / nb_LEDs;
            }
    }
}

void LedsZones::setDuration(uint8_t zone, uint64_t duration)
{
    m_durations[getIndex(zone, 0)] = duration;
}
void LedsZones::setDuration(uint8_t zone, uint64_t duration, uint8_t *lightID, uint8_t lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_durations[getIndex(zone, lightID[i])] = duration;
    }
}
void LedsZones::setColor(uint8_t zone, CRGB color)
{
    m_zoneControlledByLightID[zone] = false;
    m_colorTarget[getIndex(zone, 0)] = color;
    m_colorOriginal[getIndex(zone, 0)] = m_color[zone];
}
void LedsZones::setColor(uint8_t zone, CRGB color, uint8_t *lightID, uint8_t lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_colorTarget[getIndex(zone, lightID[i])] = color;
        m_colorOriginal[getIndex(zone, lightID[i])] = m_color[getIndex(zone, lightID[i])];
    }
}
void LedsZones::setColor(uint8_t zone, CRGB from, CRGB target)
{
    m_zoneControlledByLightID[zone] = false;
    m_colorTarget[getIndex(zone, 0)] = target;
    m_colorOriginal[getIndex(zone, 0)] = from;
}
void LedsZones::setColor(uint8_t zone, CRGB from, CRGB target, uint8_t *lightID, uint8_t lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_colorTarget[getIndex(zone, lightID[i])] = target;
        m_colorOriginal[getIndex(zone, lightID[i])] = from;
    }
}
void LedsZones::setAnim(uint8_t zone, uint8_t easingID)
{
    m_zoneControlledByLightID[zone] = false;
    clock_gettime(CLOCK_MONOTONIC, &m_animEpoch[getIndex(zone, 0)]);
    m_easing[getIndex(zone, 0)] = easingID;
}
void LedsZones::setAnim(uint8_t zone, uint8_t easingID, uint8_t *lightID, uint8_t lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        clock_gettime(CLOCK_MONOTONIC, &m_animEpoch[getIndex(zone, lightID[i])]);
        m_easing[getIndex(zone, lightID[i])] = easingID;
    }
}

CRGB LedsZones::getColor(uint16_t led)
{
    return m_color[getIndex(m_zone[led], m_lightID[led])];
}

bool LedsZones::computeAnimations()
{
    bool changes = false;
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    for (uint16_t i = 0; i < m_nbZones; i++)
    {
        if (m_zoneControlledByLightID[i])
            for (uint16_t j = 0; j < m_nbLightID[i]; j++)
                changes |= computeAnimations(getIndex(i, j), now);
        else
            changes |= computeAnimations(getIndex(i, 0), now);
    }
    return changes;
}
bool LedsZones::computeAnimations(uint16_t i, timespec now)
{
    if (m_color[i] == m_colorTarget[i] * 0.709 &&  (m_animEpoch[i].tv_nsec + m_animEpoch[i].tv_sec * 1000000000) + m_durations[i] < (now.tv_nsec + now.tv_sec * 1000000000))
    {
        return false;
    }

    m_color[i] = easeColor(m_colorTarget[i], m_colorOriginal[i], (now.tv_nsec + now.tv_sec * 1000000000) - (m_animEpoch[i].tv_nsec + m_animEpoch[i].tv_sec * 1000000000), m_durations[i], m_easing[i]);

    return true;
}

uint16_t LedsZones::getIndex(uint8_t zone, uint8_t lightID)
{
    return m_zoneID[zone] + lightID * m_zoneControlledByLightID[zone];
}

void LedsZones::setLightIDControlledZone(uint8_t zone)
{
    for (uint16_t i = m_zoneID[zone] + 1; i < (m_zoneID[zone] + m_nbLightID[zone]); i++)
    {
        m_color[i] = m_color[m_zoneID[zone]];
        m_colorOriginal[i] = m_colorOriginal[m_zoneID[zone]];
        m_colorTarget[i] = m_colorTarget[m_zoneID[zone]];
        m_durations[i] = m_durations[m_zoneID[zone]];
        m_animEpoch[i] = m_animEpoch[m_zoneID[zone]];
        m_easing[i] = m_easing[m_zoneID[zone]];
    }
    m_zoneControlledByLightID[zone] = true;
}

CRGB LedsZones::easeColor(CRGB target, CRGB original, uint64_t spent, uint64_t duration, uint8_t easeID)
{
    double progress;
    double normalizedTime = static_cast<double>(spent) / duration;

    if (normalizedTime >= 1) // If the animation is over
        return target * 0.709;
    
    switch (easeID)
    {
    case 0: // Linear
        progress = normalizedTime;
        break;
    case 1:
        progress = normalizedTime > 0.5;
        break;
    case 2:
        progress = easing::inSine(normalizedTime);
        break;
    case 3:
        progress = easing::outSine(normalizedTime);
        break;
    case 4:
        progress = easing::inOutSine(normalizedTime);
        break;
    case 5:
        progress = easing::inCubic(normalizedTime);
        break;
    case 6:
        progress = easing::outCubic(normalizedTime);
        break;
    case 7:
        progress = easing::inOutCubic(normalizedTime);
        break;
    case 8:
        progress = easing::inQuint(normalizedTime);
        break;
    case 9:
        progress = easing::outQuint(normalizedTime);
        break;
    case 10:
        progress = easing::inOutQuint(normalizedTime);
        break;
    case 11:
        progress = easing::inCirc(normalizedTime);
        break;
    case 12:
        progress = easing::outCirc(normalizedTime);
        break;
    case 13:
        progress = easing::inOutCirc(normalizedTime);
        break;
    case 14:
        progress = easing::inElastic(normalizedTime);
        break;
    case 15:
        progress = easing::outElastic(normalizedTime);
        break;
    case 16:
        progress = easing::inOutElastic(normalizedTime);
        break;
    case 17:
        progress = easing::inQuad(normalizedTime);
        break;
    case 18:
        progress = easing::outQuad(normalizedTime);
        break;
    case 19:
        progress = easing::inOutQuad(normalizedTime);
        break;
    case 20:
        progress = easing::inQuart(normalizedTime);
        break;
    case 21:
        progress = easing::outQuart(normalizedTime);
        break;
    case 22:
        progress = easing::inOutQuart(normalizedTime);
        break;
    case 23:
        progress = easing::inExpo(normalizedTime);
        break;
    case 24:
        progress = easing::outExpo(normalizedTime);
        break;
    case 25:
        progress = easing::inOutExpo(normalizedTime);
        break;
    case 26:
        progress = easing::inBack(normalizedTime);
        break;
    case 27:
        progress = easing::outBack(normalizedTime);
        break;
    case 28:
        progress = easing::inOutBack(normalizedTime);
        break;
    case 29:
        progress = easing::inBounce(normalizedTime);
        break;
    case 30:
        progress = easing::outBounce(normalizedTime);
        break;
    case 31:
        progress = easing::inOutBounce(normalizedTime);
        break;
    case 40: // Flash then on
        progress = (1. - normalizedTime) * 0.41 + 1.;
        // Start from 1.41 (to overcome the dim()), then goes back to 1
        return CRGB((uint8_t)(target.r * progress),
                    (uint8_t)(target.g * progress),
                    (uint8_t)(target.b * progress));
        break;
    case 41: // Flash then black
        progress = (1. - normalizedTime) * 1.41;
        // Same as 40 except it ends at 0

        return CRGB((uint8_t)(target.r * progress),
                    (uint8_t)(target.g * progress),
                    (uint8_t)(target.b * progress));
        break;
    default:
        progress = 1;
        break;
    }

    return CRGB((uint8_t)(original.r + (target.r - original.r) * progress), // Imitate the LerpUnclamped function from Unity
                (uint8_t)(original.g + (target.g - original.g) * progress),
                (uint8_t)(original.b + (target.b - original.b) * progress)) *
           0.709;
    // Maybe it could be quicker if the slope (target.r - original.r) was calculated only once (when affecting the colors) and stored in an array
}
