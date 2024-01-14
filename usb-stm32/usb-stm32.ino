#include <FastLED.h>
#include "LedsZones.hpp"

#define NUM_LEDS 297    // Number of LEDs on the strip
#define NUM_LEDSTRIPS 1 // Number of unadressable LED strips
#define NUM_ZONE 5

#define ZONE (buf[0] & RECV_ZONE) >> 3
#define SIGN(x) (x > 0) - (x < 0)

#pragma region Masks
#define STATE_SWITCH 0b00000001
#define STATE_MODE 0b00000010
#define STATE_BEHAVIOR 0b00000100
#define RECV_MODE 0b00000001
#define RECV_BEHAVIOR 0b00000010
#define RECV_CHANGE_COLOR 0b00000100
#define RECV_ZONE 0b00111000
#define RECV_SLOT 0b11000000
#pragma endregion Masks

union rawFloat
{
  float value;
  uint8_t raw[4];
};

struct Strip
{
  int r;
  int g;
  int b;
};

CRGB leds[NUM_LEDS + NUM_LEDSTRIPS]; // Therefore the NUM_LEDSTRIPS lasts LED objects are to be used as stored value
Strip strips[NUM_LEDSTRIPS];
LedsZones ledsZones(NUM_ZONE, NUM_LEDS + NUM_LEDSTRIPS);
bool leds_diff = true;
bool leds_refresh = true;
unsigned long timerLed = 0;
unsigned long timerStrobe = 0;
bool strobe = false;
unsigned int strobeFreq = 0;
CRGB color1 = CRGB::Blue;
CRGB color2 = CRGB::Red;
CRGB color1value = color1;
CRGB color2value = color2;
CRGB color1valuebis = color1;
CRGB color2valuebis = color2;
bool bisColor = false;
bool sceneMode = false;
bool inGame = false;
unsigned long timerLastInfo = 0;

void setup()
{
  Serial.begin(115200);

  analogReadResolution(10);
  pinMode(A0, INPUT);

  strips[0].b = PA8;
  strips[0].r = PA9;
  strips[0].g = PA10;

  FastLED.addLeds<WS2812B, PB1, GRB>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);
  for (int i = 0; i < NUM_LEDS; i++)
  { // Setting zones
    if (i <= 99)
      ledsZones.setZone(i, 2); // Left lasers
    else if (i > 99 && i <= 147)
      ledsZones.setZone(i, 1); // Ring Light
    else if (i > 147 && i <= 248)
      ledsZones.setZone(i, 3); // Right Lasers
    else if (i > 248 && i < NUM_LEDS)
      ledsZones.setZone(i, 0); // Back Lasers
  }
  ledsZones.setZone(NUM_LEDS, 4); // Center Light
  ledsZones.computeAttributes();

  for (int i = 0; i < NUM_ZONE; i++)
  {
    ledsZones.setColor(i, CRGB::Black, CRGB::White);
    ledsZones.setAnim(i, 3, 4);
    ledsZones.setDuration(i, i+1);
  }
}

void loop()
{
  while (Serial.available() > 0)
  {
    byte len = Serial.read();
    char buf[len];
    Serial.readBytes(buf, len);
    messageReceived(buf, len);
    timerLastInfo = millis();
    leds_diff = true;
  }

  if (leds_refresh)
  { // Apply (no longer in the same loop as update so it pools USB more often)
    FastLED.show();
    for (unsigned short int i = 0; i < NUM_LEDSTRIPS; i++)
    {
      analogWrite(strips[i].r, leds[NUM_LEDS + i].r);
      analogWrite(strips[i].g, leds[NUM_LEDS + i].g);
      analogWrite(strips[i].b, leds[NUM_LEDS + i].b);
    }
    leds_refresh = false;
  }
  else if (leds_diff && (millis() > timerLed + 10))
  {
    timerLed = millis();
    leds_diff = ledsZones.computeAnimations();

    for (u_int16_t i = 0; i < NUM_LEDS + NUM_LEDSTRIPS; i++)
      leds[i] = ledsZones.getColor(i);
    leds_refresh = true;
  }
  else if (sceneMode && ((millis() - timerLastInfo) > 300000))
  { // 5 minutes after the last info, disable BS mode if still enabled
    sceneMode = false;
    leds_diff = true;
    for (byte i = 0; i < NUM_ZONE; i++)
      ledsZones.setAnim(i, 0);
  }
  else if (!sceneMode)
  {
    u_int16_t t = analogRead(A0);

    if (t > 15)
    {
      if (millis() - timerStrobe > ((t - 15) >> 1))
      {
        strobe = !strobe;
        timerStrobe = millis();
        timerLed = 0;
        for (byte i = 0; i < NUM_ZONE; i++)
          ledsZones.setColor(i, CRGB::White * strobe);
        leds_diff = true;
      }
    }
    else if (!strobe)
    {
      strobe = true;
      for (byte i = 0; i < NUM_ZONE; i++)
        ledsZones.setColor(i, CRGB::White * strobe);
      leds_diff = true;
    }
  }
}

/* Description of the recieved message:
    1st byte:
     Bitfield
      +---------+----------------------------------------+---------------------+-----------------------------+----------------+-------------+
      | Bit     | 6,7 Depends on bit 2                   | 3,4,5 affected zone | 2 change color/lightIDs ?   | 1 behavior     | 0 mode      |
      +---------+----------------------------------------+---------------------+-----------------------------+----------------+-------------+
      |         | In case # => slot | Else => Chroma     | 0: Back Lasers      |         Always true         | 0: Uniform     |/ 0: Normal  |
      |         | 0: Color 1        | 0: No Chroma event | 1: Ring Light       |<---------------------------<| 1: LED by LED  |\            |
      | Meaning | 1: Color 2        | 1: RGB             | 2: Left Lasers      |-----------------------------+----------------+-------------+
      |         | 2: Color 1 bis    | 2: Gradient        | 3: Right Lasers     |# 0: Change Color (vanilla)  |/ 0: Out-game   |/ 1: BS mode |
      |         | 3: Color 2 bis    | 3: LightIDs update | 4: Center Light     |< 1: Don't (or chroma change)|\ 1: In-game    |\            |
      +---------+----------------------------------------+ 5: Color bis        +-----------------------------+----------------+-------------+
                                                         | 6: Interscope left  |
                                                         | 7: Interscope right |
                                                         +---------------------+
    For now normal mode implies changing color

    If it changes a color (0bXXXXXX0 or 0b00XX011):
      2nd, 3rd and 4th bytes are R, G and B
    Else if LightsIDs update:
      1st byte is the number of lightIDs for the affected zone
        ⇒ they won't be numbered as in beatsaber, the drinving script must take care of that
          this software must number them from back to front then from left to right
    Else:
      2nd byte is the value of the change (only available in BS mode):
        0: Turns the light group off.
        1: Changes the lights to 'color 1', and turns the lights on.
        2: Changes the lights to 'color 1', and flashes brightly before returning to normal.    duration ≅ 0.2s
        3: Changes the lights to 'color 1', and flashes brightly before fading to black.        duration ≅ 1s
        4: Unused.
        5: Changes the lights to 'color 2', and turns the lights on.
        6: Changes the lights to 'color 2', and flashes brightly before returning to normal.
        7: Changes the lights to 'color 2', and flashes brightly before fading to black.
       When the change group is Color bis:
        0: Uses main colors
        1: Uses secondary colors
      if chroma event:
        RGB:
          3rd, 4th and 5th bytes are R, G and B
        Gradient:
          3rd is easing ID    please read `LedsZones.h` for ID <=> name correspondance
          4th, 5th, 6th and 7th are duration (float (4 bits), sign bit tailing = littleEndian)
          8th, 9th, 10th are the starting color
          11th, 12th, 13rd are the ending color
        
        Next:
          +1 st byte is the number of lightIDs affected   if it's 0 then all lights of this goup are affected
          +n th bytes are the respectives lightIDs affected
        
        
*/

byte bsMode(const char *buf)
{
  byte bytesRead = 2;
  byte chroma = buf[0] >> 6;
  if (!(buf[0] & RECV_BEHAVIOR) != !inGame)
  {
    inGame = buf[0] & RECV_BEHAVIOR;
    for (int i = 0; i < NUM_ZONE; i++)
    {
      ledsZones.setColor(i, inGame ? CRGB::Black : i % 2 ? color1
                                                         : color2);
      ledsZones.setAnim(i, 3, 4); // cubic is cool
      ledsZones.setDuration(i, inGame ? 1 : 5);
    }
  }

  if ((buf[0] & (0b11000101)) == 1)
  { // If we're changing a color, vanilla way
    CRGB color = colorParsing(buf + 1);
    switch ((buf[0] & RECV_SLOT) >> 6)
    {
    case 0:
      color1value = color;
      color1 = bisColor ? color1valuebis : color1value;
      break;
    case 1:
      color2value = color;
      color2 = bisColor ? color2valuebis : color2value;
      break;
    case 2:
      color1valuebis = color;
      color1 = bisColor ? color1valuebis : color1value;
      break;
    case 3:
      color2valuebis = color;
      color2 = bisColor ? color2valuebis : color2value;
      break;
    }
    if (!inGame)
    {
      for (int i = 0; i < NUM_ZONE; i++)
      {
        ledsZones.setColor(i, i % 2 ? color1 : color2);
        ledsZones.setAnim(i, 3, 4); // cubic is cool
        ledsZones.setDuration(i, 3);
      }
    }
    bytesRead = 4;
  }
  else if ((buf[0] & 0b11000101) == 0b11000101) // We're updating lightIDs
  {
    ledsZones.setLightIDs(ZONE, buf[1]);
  }
  else
  {
    if (inGame)
    {
      if (ZONE == 5)
      {
        color1 = buf[1] ? color1valuebis : color1value;
        color2 = buf[1] ? color2valuebis : color2value;
      }
      else
      {
        CRGB color;
        rawFloat duration;
        byte anim;

        // Take care of the vanilla behavior
        switch (buf[1])
        { // Both the stock animations could be longer:
        // the meaningfull part of the customs animations occupy only the 2 first thirds
        case 0:
          color = CRGB::Black;
          duration.value = 0;
          anim = 0;
          break;
        case 1:
          color = color1;
          duration.value = 0;
          anim = 0;
          break;
        case 2:
          color = color1;
          duration.value = 0.6;
          anim = 1;
          break;
        case 3:
          color = color1;
          duration.value = 2;
          anim = 2;
          break;
        case 5:
          color = color2;
          duration.value = 0;
          anim = 0;
          break;
        case 6:
          color = color2;
          duration.value = 0.6;
          anim = 1;
          break;
        case 7:
          color = color2;
          duration.value = 2;
          anim = 2;
          break;
        }

        // parse the chroma events
        switch (chroma)
        {
        case 2:
          memcpy(duration.raw, buf + 3, 4);
          bytesRead += 11;
          break;
        case 1:
          color = colorParsing(buf + 2);
          bytesRead += 3;
          break;
        }
        byte lightIDLen = buf[bytesRead];
        bytesRead++;
        if (lightIDLen)
        {
          byte lightIDs[lightIDLen];
          memcpy(lightIDs, buf + bytesRead, lightIDLen);
          if (chroma == 2)
          { // Both color and animation specifications here are using an other overload than vanilla + rgb
            ledsZones.setAnim(ZONE, 3, buf[2], lightIDs, lightIDLen);
            ledsZones.setColor(ZONE, colorParsing(buf + 7), colorParsing(buf + 10), lightIDs, lightIDLen);
          }
          else
          {
            ledsZones.setAnim(ZONE, anim, lightIDs, lightIDLen);
            ledsZones.setColor(ZONE, color, lightIDs, lightIDLen);
          }
          ledsZones.setDuration(ZONE, duration.value, lightIDs, lightIDLen);
        }
        else
        {
          if (chroma == 2)
          {
            ledsZones.setAnim(ZONE, 3, buf[2]);
            ledsZones.setColor(ZONE, colorParsing(buf + 7), colorParsing(buf + 10));
          }
          else
          {
            ledsZones.setAnim(ZONE, anim);
            ledsZones.setColor(ZONE, color);
          }
          ledsZones.setDuration(ZONE, duration.value);
        }
        bytesRead += lightIDLen;
      }
    }
  }
  return bytesRead;
}

CRGB colorParsing(const char *buf)
{
  /**
     @brief parse color from buf

     @param buf needs to be the color with the 1st byte beeing the red value

     @return color CRGB object beeing the color parsed
  */
  return CRGB(buf[0], buf[1], buf[2]);
}

byte messageParsing(const char *buf)
{
  sceneMode = buf[0] & RECV_MODE;

  if (sceneMode)
    return bsMode(buf);
  else
    return normalMode(buf); // Number of bytes read
}

byte normalMode(const char *buf)
{
  if (buf[0] & 2)
  { // LED-by-LED
    ledsZones.setColor(ZONE, colorParsing(buf + 1));
    ledsZones.setAnim(ZONE, 3, 22);
    ledsZones.setDuration(ZONE, 5);
  }
  else
  { // Uniform
    for (int i = 0; i < NUM_ZONE + NUM_LEDSTRIPS; i++)
    {
      ledsZones.setColor(i, colorParsing(buf + 1));
      ledsZones.setAnim(i, 3, 22);
      ledsZones.setDuration(i, 5);
    }
  }
  return 4;
}

void messageReceived(const char *bytes, byte length)
{
  byte startAt = 0;
  do
  {
    startAt += messageParsing(bytes + startAt);
  } while (length > startAt);
}
