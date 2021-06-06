#include "LedsZones.h"

LedsZones::LedsZones(byte zones, int leds){
    m_zone  = new  byte[leds];
    m_taus  = new float[zones];
    m_anim  = new  byte[zones];
    m_color = new  CRGB*[zones];
    m_colorNoPointer = new CRGB[zones];
    m_nbZones = zones;
}

void LedsZones::setZone(int led, byte zone)
{
    m_zone[led] = zone;
}

void LedsZones::setTau(byte zone, float tau)
{
    m_taus[zone] = tau;
}
void LedsZones::setColor(byte zone, CRGB *color)
{
    m_color[zone] = color;
}
void LedsZones::setColorNoPointer(byte zone, CRGB color)
{
    m_colorNoPointer[zone] = color;
    m_color[zone] = &m_colorNoPointer[zone];
}
void LedsZones::setAnim(byte zone, byte anim)
{
    m_anim[zone] = anim;
}

void LedsZones::wipeFlash()
{
    for (int i = 0; i < m_nbZones; i++)
    {
        if (m_anim[i] == 2)
            setColorNoPointer(i, CRGB::Black);
        m_anim[i] = false;
    }
}

byte LedsZones::getZone(int led)
{
    return m_zone[led];
}
float LedsZones::getTau(int led)
{
    return getAnim(led) ? 0 : m_taus[getZone(led)];
}
byte LedsZones::getAnim(int led)
{
    return m_anim[getZone(led)];
}
CRGB LedsZones::getColor(int led)
{
    if (m_color[getZone(led)] == nullptr)
        setColorNoPointer(getZone(led), CRGB::Black);
    return getAnim(led) ? *m_color[getZone(led)] : dim(*m_color[getZone(led)]);
}

CRGB LedsZones::dim(CRGB colortodim){
    return colortodim.nscale8(180);
}