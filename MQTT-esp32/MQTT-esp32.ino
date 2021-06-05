#include <FastLED.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include "creditentials.h"

#define NUM_LEDS 209
#define NUM_ZONE 5
#define ALIM 26
#define LED 27

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

#define ZONE (buf[0] & RECV_ZONE) >> 3
#define SIGN(x) (x > 0) - (x < 0)

class LedsZones
{
public:
  void setZone(int led, byte zone)
  {
    m_zone[led] = zone;
  }
  void setTau(byte zone, float tau)
  {
    m_taus[zone] = tau;
  }
  void setColor(byte zone, CRGB *color)
  {
    m_color[zone] = color;
  }
  void setColorNoPointer(byte zone, CRGB color)
  {
    m_colorNoPointer[zone] = color;
    m_color[zone] = &m_colorNoPointer[zone];
  }
  void setAnim(byte zone, byte anim)
  {
    m_anim[zone] = anim;
  }

  void wipeFlash()
  {
    for (int i = 0; i < NUM_ZONE; i++){
      if (m_anim[i] == 2) setColorNoPointer(i, CRGB::Black);
      m_anim[i] = false;
    }
  }

  byte getZone(int led)
  {
    return m_zone[led];
  }
  float getTau(int led)
  {
    return getAnim(led) ? 0 : m_taus[getZone(led)];
  }
  byte getAnim(int led)
  {
    return m_anim[getZone(led)];
  }
  CRGB getColor(int led)
  {
    if (m_color[getZone(led)] == nullptr) setColorNoPointer(getZone(led), CRGB::Black);
    return getAnim(led) ? *m_color[getZone(led)] : dim(*m_color[getZone(led)]);
  }

private:
  byte  m_zone[NUM_LEDS];
  float m_taus[NUM_ZONE];
  byte  m_anim[NUM_ZONE];    // 0 = none, 1 = flash, 2 = flash + fade to black
  CRGB *m_color[NUM_ZONE];
  CRGB  m_colorNoPointer[NUM_ZONE];

  CRGB dim(CRGB colortodim){   // Doing it with a function like this allows to copy the color object therfore not affecting the original color
    return colortodim.nscale8(180);
  }
};

LedsZones ledsZones;
CRGB leds[NUM_LEDS];
bool leds_diff = false;
unsigned long timerLedFilter = 0;
CRGB CRGBBlack = CRGB::Black;
CRGB color1 = CRGB::Red;
CRGB color2 = CRGB::Blue;
CRGB color1value = color1;
CRGB color2value = color2;
CRGB color1valuebis = color1;
CRGB color2valuebis = color2;
bool bisColor = false;

const char letsEncyptCA[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n"
    "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
    "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n"
    "WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"
    "RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
    "AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n"
    "R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n"
    "sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n"
    "NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n"
    "Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n"
    "/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n"
    "AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n"
    "Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n"
    "FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n"
    "AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n"
    "Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n"
    "gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n"
    "PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n"
    "ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n"
    "CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n"
    "lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n"
    "avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n"
    "yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n"
    "yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n"
    "hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n"
    "HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n"
    "MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n"
    "nLRbwHOoq7hHwg==\n"
    "-----END CERTIFICATE-----\n";

WiFiClientSecure net;
MQTTClient client(256);
bool pong = false;

bool alimSwitch = false;
bool alimSwitchReal = false;
bool sceneMode = false;
bool inGame = false;
unsigned long timerLastInfo;

void setup()
{
  //Serial.begin(115200);

  FastLED.addLeds<WS2812B, LED, GRB>(leds, NUM_LEDS) // Led strip on pin 27
      .setCorrection(TypicalLEDStrip);
  for (int i = 0; i < NUM_LEDS; i++)
  { // Setting zones
    if (i <= 42)
      ledsZones.setZone(i, 2); // Left lasers
    else if (i > 167)
      ledsZones.setZone(i, 3); // Right Lasers
    else if (i > 83 && i <= 125)
      ledsZones.setZone(i, 4); // Center Light
    else if ((i > 42 && i <= 63) || (i > 125 && i <= 145))
      ledsZones.setZone(i, 1); // Ring Light
    else if ((i > 63 && i <= 83) || (i > 145 && i <= 167))
      ledsZones.setZone(i, 0); // Back Lasers
  }
  
  pinMode(ALIM, OUTPUT);

  WiFi.disconnect();
  WiFi.begin(ssid, pass);
  
  net.setCACert(letsEncyptCA);
  client.begin(host, 8883, net);
  client.onMessageAdvanced(messageReceived);
}

void loop()
{
  if (leds_diff && millis() - timerLedFilter > 10)
  {
    leds_diff = false; // Check again for diff and apply the filter
    alimSwitch = sceneMode;

    float dt = (millis() - timerLedFilter) / 1000.;
    dt = dt > 0.015 ? 0.015 : dt;
    timerLedFilter = millis();
    CRGB target;
    float dttau;
    float delta;
    for (int i = 0; i < NUM_LEDS; i++)
    {
      target = ledsZones.getColor(i);
      dttau = ledsZones.getTau(i); 
      if (!leds_diff && leds[i] != target) leds_diff = true;
      if (!alimSwitch && target) alimSwitch = true;

      if (dttau)
      {
        dttau = dt / dttau; // only compute once dt / tau, access tau only once in ledsZones for the 3 colours, hope it speeds up a bit
        // 1st order filter
        // Due to the CRGB's structure (no negative colors, impossible to multiplie with a float (needed here as dttau will most likely be < 1))
        //  I work with raw color channels ints but it involves to check and avoid cases where the delta is smaller than one
        delta = (target.r - leds[i].r) * dttau;
        leds[i].r += (abs(delta) < 1 && delta != 0) ? SIGN(delta) : delta;

        delta = (target.g - leds[i].g) * dttau;
        leds[i].g += (abs(delta) < 1 && delta != 0) ? SIGN(delta) : delta;

        delta = (target.b - leds[i].b) * dttau;
        leds[i].b += (abs(delta) < 1 && delta != 0) ? SIGN(delta) : delta;
      }
      else {
        leds[i] = target;
      }
      
    }
    ledsZones.wipeFlash();
    FastLED.show(); // Apply
  }
  else if (sceneMode && millis() - timerLastInfo > 600000)
  { // 10 minutes after the last info, disable BS mode if still enabled
    sceneMode = false;
  } else if (!(alimSwitchReal || alimSwitch) && !sceneMode) delay(1000);

  if (alimSwitch != alimSwitchReal)
  {
    alimSwitch ? digitalWrite(ALIM, HIGH) : digitalWrite(ALIM, LOW);
    alimSwitchReal = alimSwitch;
  }


  if (!client.connected())
  {
    while (WiFi.status() != WL_CONNECTED)
      yield();

    while (!client.connect("deskleds", user, mqttPass))
      delay(1000);

    client.subscribe("/led", 1);
    client.subscribe("/led/ping", 0);
    client.subscribe("/led/stream", 0);
  }
  if (pong)
  {
    client.publish("/led/ping", "pong");
    pong = false;
  }
  client.loop();
}

/* Description of the recieved message:
    "ping" for a ping, otherwise:

    1st byte:
     Bitfield
      +---------+-------------+----------------+----------------- +------------------+-----------------------------------+
      | Bit     | 0 mode      | 1 behavior     | 2 change color ? | 3,4,5 change for | 6,7 color slot to change (case #) |
      +---------+-------------+----------------+------------------+------------------+-----------------------------------+
      |         | 0: Normal  \| 0: Uniform     |    Always true   | 0: Back Lasers   | 0: Color 1                        |
      |         |            /| 1: LED by LED  >----------------->| 1: Ring Light    | 1: Color 2                        |
      | Meaning |         ----+----------------+------------------+ 2: Left Lasers   |                                   |
      |         | 1: BS mode \| 0: Out-game   \| 0: Change Color #| 3: Right Lasers  | 2: Color 1 bis                    |
      |         |            /| 1: In-game    /| 1: Don't        >| 4: Center Light  | 3: Color 2 bis                    |
      +---------+-------------+----------------+------------------+ 5: Color bis     +-----------------------------------+
                                                                  +------------------+
    When we don't change color out-game, everything is disregarded but still must be present
    If it changes a color (0bXXXXXX0 or 0bXXXX011):
      2nd, 3rd and 4th bytes are R, G and B
    Else:
      2nd byte is the value of the change (only available in BS mode):
        0: Turns the light group off.
        1: Changes the lights to 'color 1', and turns the lights on.
        2: Changes the lights to 'color 1', and flashes brightly before returning to normal.    τ ≅ 0.2s
        3: Changes the lights to 'color 1', and flashes brightly before fading to black.        τ ≅ 1s
        4: Unused.
        5: Changes the lights to 'color 2', and turns the lights on.
        6: Changes the lights to 'color 2', and flashes brightly before returning to normal.
        7: Changes the lights to 'color 2', and flashes brightly before fading to black.
       When the change group is Color bis:
        0: Uses main colors
        1: Uses secondary colors
*/

void bsMode(const char *buf, CRGB color)
{
  if (color){
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
  } else {
    if (!(buf[0] & RECV_BEHAVIOR) != !inGame){
      inGame = buf[0] & RECV_BEHAVIOR;
      for (int i = 0; i < NUM_ZONE; i++){
        ledsZones.setColor(i, inGame ? &CRGBBlack : i%2 ? &color1 : &color2);
        ledsZones.setTau(i, inGame ? 1 : 3);
      }
    }

    if (inGame){
      if(ZONE == 5){
        bisColor = buf[1];
        color1 = bisColor ? color1valuebis : color1value;
        color2 = bisColor ? color2valuebis : color2value;
      } else {
        switch (buf[1])
        {
        case 0:
          ledsZones.setColor(ZONE, &CRGBBlack);
          ledsZones.setTau(ZONE, 0);
          ledsZones.setAnim(ZONE, 0);
          break;
        case 1:
          ledsZones.setColor(ZONE, &color1);
          ledsZones.setTau(ZONE, 0);
          ledsZones.setAnim(ZONE, 0);
          break;
        case 2:
          ledsZones.setColor(ZONE, &color1);
          ledsZones.setTau(ZONE, 0.2);
          ledsZones.setAnim(ZONE, 1);
          break;
        case 3:
          ledsZones.setColor(ZONE, &color1);
          ledsZones.setTau(ZONE, 1);
          ledsZones.setAnim(ZONE, 2);
          break;
        case 5:
          ledsZones.setColor(ZONE, &color2);
          ledsZones.setTau(ZONE, 0);
          ledsZones.setAnim(ZONE, 0);
          break;
        case 6:
          ledsZones.setColor(ZONE, &color2);
          ledsZones.setTau(ZONE, 0.2);
          ledsZones.setAnim(ZONE, 1);
          break;
        case 7:
          ledsZones.setColor(ZONE, &color2);
          ledsZones.setTau(ZONE, 1);
          ledsZones.setAnim(ZONE, 2);
          break;
        
        default:
          break;
        }
      }
    }
  }
}

int messageParsing(const char* buf)
{
  sceneMode = buf[0] & RECV_MODE;
  leds_diff = true;
  CRGB color = false;
  if ((!(buf[0] & RECV_MODE)) || ((buf[0] & (0b00000101)) == 1))
    color = buf[1] << 16 | buf[2] << 8 | buf[3];    // Color changing mode => parse color

  if (buf[0] & RECV_MODE)
    bsMode(buf, color);
  else
    normalMode(buf, color);
  return color ? 4 : 2;   // Number of bytes read
}

void normalMode(const char *buf, CRGB color)
{
  if (buf[0] & 2)
  { // LED-by-LED
    ledsZones.setColorNoPointer(ZONE, color);
  }
  else
  { // Uniform
      for (int i = 0; i < NUM_ZONE; i++) ledsZones.setColorNoPointer(i, color);
  }
}

void messageReceived(MQTTClient *client, char topic[], char bytes[], int length)
{
  if (!strcmp(topic, "/led/ping") && !strcmp(bytes, "ping"))
  {
    pong = true;
  } else if (!strcmp(topic, "/led") || !strcmp(topic, "/led/stream")) {
    int startAt = 0;
    while (length > startAt){
      startAt += messageParsing(bytes+startAt);
    }
  }
  timerLastInfo = millis();
}
