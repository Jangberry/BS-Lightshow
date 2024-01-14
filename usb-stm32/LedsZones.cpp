#include "LedsZones.hpp"
#include "easing.hpp"

LedsZones::LedsZones(byte zones, u_int16_t leds)
{
    m_nbZones = zones;
    m_nbLEDs = leds;
    m_zone = new byte[leds];
    m_anim = new byte[leds];
    m_animEpoch = new unsigned long[leds];
    m_easing = new byte[leds];
    m_durations = new float[leds];
    m_color = new CRGB[leds];
    m_colorTarget = new CRGB[leds];
    m_colorOriginal = new CRGB[leds];
    m_nbLightID = new byte[zones];
    m_lightID = new byte[leds];
    m_zoneID = new uint[zones];
    m_zoneControlledByLightID = new bool[zones];
}

void LedsZones::setZone(u_int16_t led, byte zone)
{
    m_zone[led] = zone;
}

void LedsZones::computeAttributes()
{
    uint nb_LEDsInZone[m_nbZones] = {};
    for (uint i = 0; i < m_nbLEDs; i++)
        nb_LEDsInZone[m_zone[i]] = nb_LEDsInZone[m_zone[i]] + 1;
    for (uint i = 0; i < m_nbZones; i++)
    {
        m_zoneID[i] = i == 0 ? 0 : m_zoneID[i - 1] + nb_LEDsInZone[i - 1];
        m_colorOriginal[m_zoneID[i]] = CRGB::Black;
        m_zoneControlledByLightID[i] = false;
    }
}

void LedsZones::setLightIDs(byte zone, byte nbIDs)
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

void LedsZones::setDuration(byte zone, float duration)
{
    /**
     * @brief Set animation duration in seconds
     * 
     * @param duration duration in seconds
    */
    m_durations[getIndex(zone, 0)] = duration * 1000.;
}
void LedsZones::setDuration(byte zone, float duration, byte *lightID, byte lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_durations[getIndex(zone, lightID[i])] = duration * 1000.;
    }
}
void LedsZones::setColor(byte zone, CRGB color)
{
    m_zoneControlledByLightID[zone] = false;
    m_colorTarget[getIndex(zone, 0)] = color;
    m_colorOriginal[getIndex(zone, 0)] = m_color[zone];
}
void LedsZones::setColor(byte zone, CRGB color, byte *lightID, byte lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_colorTarget[getIndex(zone, lightID[i])] = color;
        m_colorOriginal[getIndex(zone, lightID[i])] = m_color[getIndex(zone, lightID[i])];
    }
}
void LedsZones::setColor(byte zone, CRGB from, CRGB target)
{
    m_zoneControlledByLightID[zone] = false;
    m_colorTarget[getIndex(zone, 0)] = target;
    m_colorOriginal[getIndex(zone, 0)] = from;
}
void LedsZones::setColor(byte zone, CRGB from, CRGB target, byte *lightID, byte lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_colorTarget[getIndex(zone, lightID[i])] = target;
        m_colorOriginal[getIndex(zone, lightID[i])] = from;
    }
}
void LedsZones::setAnim(byte zone, byte anim)
{
    m_zoneControlledByLightID[zone] = false;
    m_anim[getIndex(zone, 0)] = anim;
    m_animEpoch[getIndex(zone, 0)] = millis();
}
void LedsZones::setAnim(byte zone, byte anim, byte *lightID, byte lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_anim[getIndex(zone, lightID[i])] = anim;
        m_animEpoch[getIndex(zone, lightID[i])] = millis();
    }
}
void LedsZones::setAnim(byte zone, byte anim, byte easingID)
{
    m_zoneControlledByLightID[zone] = false;
    m_anim[getIndex(zone, 0)] = anim;
    m_animEpoch[getIndex(zone, 0)] = millis();
    m_easing[getIndex(zone, 0)] = easingID;
}
void LedsZones::setAnim(byte zone, byte anim, byte easingID, byte *lightID, byte lightIDSize)
{
    if (!m_zoneControlledByLightID[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_anim[getIndex(zone, lightID[i])] = anim;
        m_animEpoch[getIndex(zone, lightID[i])] = millis();
        m_easing[getIndex(zone, lightID[i])] = easingID;
    }
}

CRGB LedsZones::getColor(u_int16_t led)
{
    return m_color[getIndex(m_zone[led], m_lightID[led])];
}

bool LedsZones::computeAnimations()
{
    bool changes = false;
    unsigned long now = millis();
    for (uint i = 0; i < m_nbZones; i++)
    {
        if (m_zoneControlledByLightID[i])
            for (uint j = 0; j < m_nbLightID[i]; j++)
                changes |= computeAnimations(getIndex(i, j), now);
        else
            changes |= computeAnimations(getIndex(i, 0), now);
    }
    return changes;
}
bool LedsZones::computeAnimations(u_int16_t i, unsigned long now)
{
    if(m_color[i] == dim(m_colorTarget[i])){return false;}

    if (m_anim[i] && (m_durations[i] < now - m_animEpoch[i]))
    {
        if (m_anim[i] == 2){m_colorTarget[i] = CRGB::Black;}
        m_anim[i] = 0;
    }

    switch (m_anim[i])
    {
    case 0:
        m_color[i] = dim(m_colorTarget[i]);
        break;
    case 1:
        m_color[i] = easeColor(dim(m_colorTarget[i]), m_colorOriginal[i], now - m_animEpoch[i], m_durations[i], 40);
        break;
    case 2:
        m_color[i] = easeColor(dim(m_colorTarget[i]), m_colorOriginal[i], now - m_animEpoch[i], m_durations[i], 41);
        break;
    case 3:
        m_color[i] = easeColor(dim(m_colorTarget[i]), m_colorOriginal[i], now - m_animEpoch[i], m_durations[i], m_easing[i]);
        break;
    };
    return true;
}

CRGB LedsZones::dim(CRGB colortodim) // Dim a copy of the color to keep the original one unaltared
{
    return colortodim.nscale8(180); // Maybe usefull to compute it once then store...
}

uint LedsZones::getIndex(byte zone, byte lightID)
{
    return m_zoneID[zone] + lightID * m_zoneControlledByLightID[zone];
}

void LedsZones::setLightIDControlledZone(byte zone)
{
    for (uint i = m_zoneID[zone] + 1; i < (m_zoneID[zone] + m_nbLightID[zone]); i++)
    {
        m_anim[i] = m_anim[m_zoneID[zone]];
        m_animEpoch[i] = m_animEpoch[m_zoneID[zone]];
        m_color[i] = m_color[m_zoneID[zone]];
        m_colorOriginal[i] = m_colorOriginal[m_zoneID[zone]];
        m_colorTarget[i] = m_colorTarget[m_zoneID[zone]];
        m_durations[i] = m_durations[m_zoneID[zone]];
        m_easing[i] = m_easing[m_zoneID[zone]];
    }
    m_zoneControlledByLightID[zone] = true;
}

CRGB LedsZones::easeColor(CRGB target, CRGB original, float spent, float duration, byte easeID)
{
    double progress;
    double normalizedTime = spent / duration;
    switch (easeID)
    {
    case 0: // Linear
        progress = normalizedTime;
        break;
    case 1: // Step
        progress = floor(normalizedTime);
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
        progress = (1 - normalizedTime) * 0.41 + 1;
        // Start from 1.41 (to overcome the dim()), then goes back to 1
        return CRGB((uint8_t)(target.r * progress),
                    (uint8_t)(target.g * progress),
                    (uint8_t)(target.b * progress));
        break;
    case 41: // Flash then black
        progress = (1 - normalizedTime) * 1.41;
        // Same as 40 except it ends at 0

        return CRGB((uint8_t)(target.r * progress),
                    (uint8_t)(target.g * progress),
                    (uint8_t)(target.b * progress));

        break; // is that even usefull ?
    }

    return CRGB((uint8_t)(original.r + (target.r - original.r) * progress), // Imitate the LerpUnclamped function from Unity
                (uint8_t)(original.g + (target.g - original.g) * progress),
                (uint8_t)(original.b + (target.b - original.b) * progress));
    // Maybe it could be quicker if the slope (target.r - original.r) was calculated only once (when affecting the colors) and stored in an array
}
