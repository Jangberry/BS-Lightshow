#include "LedsZones.hpp"
#include "easing.hpp"

LedsZones::LedsZones(byte zones, unsigned short int leds)
{
    m_nbZones = zones;
    m_nbLEDs = leds;
    m_nbLEDsInZone = new unsigned short int[zones];
    m_nbLightIDs = new unsigned short int[zones];
    m_zone = new byte[leds];
    m_zoneID = new unsigned short int[zones];
    m_lightIDControledZone = new bool[zones];
    m_ledByLightID = new unsigned short int[zones];
    m_anim = new byte[leds];
    m_animEpoch = new unsigned long[leds];
    m_easing = new byte[leds];
    m_durations = new unsigned short int[leds];
    m_color = new CRGB[leds];
    m_colorTarget = new CRGB[leds];
    m_colorOriginal = new CRGB[leds];
}

void LedsZones::setZone(unsigned short int led, byte zone)
{
    m_zone[led] = zone;
}
void LedsZones::computeAttributes()
{
    // Count LEDs by zone
    for (unsigned short int zone = 0; zone < m_nbZones; zone++){
        m_nbLEDsInZone[zone] = (unsigned short int)0;
        m_lightIDControledZone[zone] = false;
        for (int led = 0; led < m_nbLEDs; led++)
        {
            if (m_zone[led] == zone)
            {
                m_zoneID[zone] = (unsigned short int)led;
                break;  // search for the index of the first led in this zone
            }
        }
    }
    for (int i = 0; i < m_nbLEDs; i++)
        m_nbLEDsInZone[m_zone[i]]++;
}
void LedsZones::setLightIDs(byte zone, byte nbIDs)
{
    if (nbIDs > m_nbLEDsInZone[zone]){
        m_nbLightIDs[zone] = (unsigned short int)1;
        m_ledByLightID[zone] = (unsigned short int)0;
    } else {
        m_nbLightIDs[zone] = (unsigned short int)nbIDs;
        m_ledByLightID[zone] = (unsigned short int)(m_nbLEDsInZone[zone] / nbIDs);
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
    m_lightIDControledZone[zone] = false;
}
void LedsZones::setDuration(byte zone, float duration, byte *lightID, byte lightIDSize)
{
    /**
     * @brief Set animation duration in seconds
     * 
     * @param duration duration in seconds
    */
    if (!m_lightIDControledZone[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
        m_durations[getIndex(zone, lightID[i])] = duration * 1000.;
}
void LedsZones::setColor(byte zone, CRGB color)
{
    m_colorTarget[m_zoneID[zone]] = color;
    m_colorOriginal[m_zoneID[zone]] = m_color[m_zoneID[zone]];
    m_lightIDControledZone[zone] = false;
}
void LedsZones::setColor(byte zone, CRGB color, byte *lightID, byte lightIDSize)
{
    if (!m_lightIDControledZone[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_colorTarget[getIndex(zone, lightID[i])] = color;
        m_colorOriginal[getIndex(zone, lightID[i])] = m_color[getIndex(zone, lightID[i])];
    }
}
void LedsZones::setColor(byte zone, CRGB from, CRGB target)
{
    m_colorTarget[m_zoneID[zone]] = target;
    m_colorOriginal[m_zoneID[zone]] = from;
    m_lightIDControledZone[zone] = false;
}
void LedsZones::setColor(byte zone, CRGB from, CRGB target, byte *lightID, byte lightIDSize)
{
    if (!m_lightIDControledZone[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_colorTarget[getIndex(zone, lightID[i])] = target;
        m_colorOriginal[getIndex(zone, lightID[i])] = from;
    }
}
void LedsZones::setAnim(byte zone, byte anim)
{
    m_anim[m_zoneID[zone]] = anim;
    m_animEpoch[m_zoneID[zone]] = millis();
    m_lightIDControledZone[zone] = false;
}
void LedsZones::setAnim(byte zone, byte anim, byte *lightID, byte lightIDSize)
{
    if (!m_lightIDControledZone[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_anim[getIndex(zone, lightID[i])] = anim;
        m_animEpoch[getIndex(zone, lightID[i])] = millis();
    }
}
void LedsZones::setAnim(byte zone, byte anim, byte easingID)
{
    m_anim[m_zoneID[zone]] = anim;
    m_animEpoch[m_zoneID[zone]] = millis();
    m_easing[m_zoneID[zone]] = easingID;
    m_lightIDControledZone[zone] = false;
}
void LedsZones::setAnim(byte zone, byte anim, byte easingID, byte *lightID, byte lightIDSize)
{
    if (!m_lightIDControledZone[zone])
        setLightIDControlledZone(zone);
    for (int i = 0; i < lightIDSize; i++)
    {
        m_anim[getIndex(zone, lightID[i])] = anim;
        m_animEpoch[getIndex(zone, lightID[i])] = millis();
        m_easing[getIndex(zone, lightID[i])] = easingID;
    }
}

CRGB LedsZones::getColor(int led)
{
    return m_color[getIndex(led)];
}

bool LedsZones::computeAnimations()
{
    bool changes = false;
    for (int i = 0; i < m_nbZones; i++)
    {
        if (m_lightIDControledZone[i])
        {
            for (int j = 0; j < m_nbLightIDs[i]; j++)
                changes |= computeAnimations(m_zoneID[i] + j);
        }
        else
            changes |= computeAnimations(m_zoneID[i]);
    }
    return changes;
}

bool LedsZones::computeAnimations(unsigned short int i)
{
    bool changes = m_anim[i];
    if (m_anim[i] && (m_animEpoch[i] < millis() - m_durations[i]))
    {
        if (m_anim[i] == 2)
            setColor(i, CRGB::Black);
        m_anim[i] = 0;
    }

    switch (m_anim[i])
    {
    case 0:
        m_color[i] = dim(m_colorTarget[i]);
        break;
    case 1:
        m_color[i] = easeColor(dim(m_colorTarget[i]), m_colorOriginal[i], millis() - m_animEpoch[i], m_durations[i], 40);
        break;
    case 2:
        m_color[i] = easeColor(dim(m_colorTarget[i]), m_colorOriginal[i], millis() - m_animEpoch[i], m_durations[i], 41);
        break;
    case 3:
        m_color[i] = easeColor(dim(m_colorTarget[i]), m_colorOriginal[i], millis() - m_animEpoch[i], m_durations[i], m_easing[i]);
        break;
    };
    return changes;
}

unsigned short int LedsZones::getIndex(unsigned short int led){
    if(m_lightIDControledZone[m_zone[led]]){
        byte zone = m_zone[led];
        return m_zoneID[zone] + (((led - m_zoneID[zone])/m_ledByLightID[zone]) % m_nbLEDsInZone[zone]);
    } else {
        return m_zoneID[m_zone[led]];
    }
}
unsigned short int LedsZones::getIndex(byte zone, byte lightID){
    if(m_lightIDControledZone[zone]){
        return m_zoneID[zone] + ((lightID) % m_nbLightIDs[zone]);
    } else {
        return m_zoneID[zone];
    }
}

void LedsZones::setLightIDControlledZone(byte zone)
{
    int j;
    for (int i = 1; i < m_nbLightIDs[zone]; i++)    // Starts at 1 as 0 is the "template"
    {
        m_durations[getIndex(zone, i)] = m_durations[m_zoneID[zone]];
        m_anim[getIndex(zone, i)] = m_anim[m_zoneID[zone]];
        m_easing[getIndex(zone, i)] = m_easing[m_zoneID[zone]];
        m_animEpoch[getIndex(zone, i)] = m_animEpoch[m_zoneID[zone]];
        m_color[getIndex(zone, i)] = m_color[m_zoneID[zone]];
        m_colorOriginal[getIndex(zone, i)] = m_colorOriginal[m_zoneID[zone]];
        m_colorTarget[getIndex(zone, i)] = m_colorTarget[m_zoneID[zone]];
    }
    m_lightIDControledZone[zone] = true;
}

CRGB LedsZones::dim(CRGB colortodim) // Dim a copy of the color to keep the original one unaltared
{
    return colortodim.nscale8(180); // Maybe usefull to compute it once then store...
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
    case 1:
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
        // Simpliest model. Hope that's the right one
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

        break;
    }

    return CRGB((uint8_t)(original.r + (target.r - original.r) * progress), // Imitate the LerpUnclamped function from Unity
                (uint8_t)(original.g + (target.g - original.g) * progress),
                (uint8_t)(original.b + (target.b - original.b) * progress));
}
