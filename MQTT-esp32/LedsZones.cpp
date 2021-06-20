#include "LedsZones.h"
#include "easing.hpp"

#include <cmath>

LedsZones::LedsZones(byte zones, int leds)
{
    m_nbZones = zones;
    m_zone = new byte[leds];
    m_anim = new byte[zones];
    m_animEpoch = new unsigned long[zones];
    m_easing = new byte[zones];
    m_durations = new float[zones];
    m_color = new CRGB[zones];
    m_colorTarget = new CRGB [zones];
    m_colorOriginal = new CRGB[zones];
}

void LedsZones::setZone(int led, byte zone)
{
    m_zone[led] = zone;
}

void LedsZones::setDuration(byte zone, float duration)
{
    /**
     * @brief Set animation duration in seconds
     * 
     * @param duration duration in seconds
    */
    m_durations[zone] = duration * 1000.;
}
void LedsZones::setColor(byte zone, CRGB color)
{
    m_colorTarget[zone] = color;
    m_colorOriginal[zone] = m_color[zone];
}
void LedsZones::setColor(byte zone, CRGB from, CRGB target)
{
    m_colorTarget[zone] = target;
    m_colorOriginal[zone] = from;
}
void LedsZones::setAnim(byte zone, byte anim)
{
    m_anim[zone] = anim;
    m_animEpoch[zone] = millis();
}
void LedsZones::setAnim(byte zone, byte anim, byte easingID)
{
    m_anim[zone] = anim;
    m_animEpoch[zone] = millis();
    m_easing[zone] = easingID;
}

CRGB LedsZones::getColor(int led)
{
    return m_color[m_zone[led]];
}

bool LedsZones::computeAnimations()
{
    // TODO Fix changes check
    bool changes = false;
    for (int i = 0; i < m_nbZones; i++)
    {
        if (m_anim[i] && m_animEpoch[i] + m_durations[i] < millis()) {
            if(m_anim[i] == 2) setColor(i, CRGB::Black);
            m_anim[i] = 0;
        } else
            changes = true;

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
    }
    return changes;
}

CRGB LedsZones::dim(CRGB colortodim)    // Dim a copy of the color to keep the original one unaltared
{
    return colortodim.nscale8(180);
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
        progress = easing::inSine(normalizedTime);
        break;
    case 2:
        progress = easing::outSine(normalizedTime);
        break;
    case 3:
        progress = easing::inOutSine(normalizedTime);
        break;
    case 4:
        progress = easing::inCubic(normalizedTime);
        break;
    case 5:
        progress = easing::outCubic(normalizedTime);
        break;
    case 6:
        progress = easing::inOutCubic(normalizedTime);
        break;
    case 7:
        progress = easing::inQuint(normalizedTime);
        break;
    case 8:
        progress = easing::outQuint(normalizedTime);
        break;
    case 9:
        progress = easing::inOutQuint(normalizedTime);
        break;
    case 10:
        progress = easing::inCirc(normalizedTime);
        break;
    case 11:
        progress = easing::outCirc(normalizedTime);
        break;
    case 12:
        progress = easing::inOutCirc(normalizedTime);
        break;
    case 13:
        progress = easing::inElastic(normalizedTime);
        break;
    case 14:
        progress = easing::outElastic(normalizedTime);
        break;
    case 15:
        progress = easing::inOutElastic(normalizedTime);
        break;
    case 16:
        progress = easing::inQuad(normalizedTime);
        break;
    case 17:
        progress = easing::outQuad(normalizedTime);
        break;
    case 18:
        progress = easing::inOutQuad(normalizedTime);
        break;
    case 19:
        progress = easing::inQuart(normalizedTime);
        break;
    case 20:
        progress = easing::outQuart(normalizedTime);
        break;
    case 21:
        progress = easing::inOutQuart(normalizedTime);
        break;
    case 22:
        progress = easing::inExpo(normalizedTime);
        break;
    case 23:
        progress = easing::outExpo(normalizedTime);
        break;
    case 24:
        progress = easing::inOutExpo(normalizedTime);
        break;
    case 25:
        progress = easing::inBack(normalizedTime);
        break;
    case 26:
        progress = easing::outBack(normalizedTime);
        break;
    case 27:
        progress = easing::inOutBack(normalizedTime);
        break;
    case 28:
        progress = easing::inBounce(normalizedTime);
        break;
    case 29:
        progress = easing::outBounce(normalizedTime);
        break;
    case 30:
        progress = easing::inOutBounce(normalizedTime);
        break;
    case 40:    // Flash then on
        progress = exp(-7. * normalizedTime + 1.) - exp(-13. * normalizedTime) * (2.718281828 + 1.) + 1.;   // 0 to 1 with a nice exponential with overshoot
        return CRGB( (uint8_t)((float)original.r * min(1.-progress, 0.) + (float)target.r * progress),
                     (uint8_t)((float)original.g * min(1.-progress, 0.) + (float)target.g * progress),
                     (uint8_t)((float)original.b * min(1.-progress, 0.) + (float)target.b * progress));
                // Starts at the original color then uses only the target when progress > 1
        break;
    case 41:    // Flash then black
        progress = exp(-7. * normalizedTime + 1.) - exp(-33. * normalizedTime + 1.);    // Same as 40 except it ends at 0
        return CRGB((uint8_t)(((float)original.r * min(1.-progress, 0.)) * (normalizedTime < 0.06) + (float)target.r * progress),
                    (uint8_t)(((float)original.g * min(1.-progress, 0.)) * (normalizedTime < 0.06) + (float)target.g * progress),
                    (uint8_t)(((float)original.b * min(1.-progress, 0.)) * (normalizedTime < 0.06) + (float)target.b * progress));
                // As progress goes back under 1 we need to ignore the original color at some point
                // "some point" is here the maximum of progress (that happen around 0.06)
        break;  // is that even usefull ?
    }

    return CRGB((uint8_t)(original.r + (target.r - original.r) * progress),
                (uint8_t)(original.g + (target.g - original.g) * progress),
                (uint8_t)(original.b + (target.b - original.b) * progress));
}