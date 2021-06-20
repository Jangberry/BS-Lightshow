#include <FastLED.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include "creditentials.h"
#include "LedsZones.h"

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

LedsZones ledsZones(NUM_ZONE, NUM_LEDS);
CRGB leds[NUM_LEDS];
bool leds_diff = false;
unsigned long timerLed = 0;
CRGB color1 = CRGB::Blue;
CRGB color2 = CRGB::Red;
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
  if (leds_diff && millis() - timerLed > 10)
  {
    leds_diff = ledsZones.computeAnimations(); // Check again for diff and apply the filter
    alimSwitch = sceneMode;

    for (int i = 0; i < NUM_LEDS; i++)
    {
      if (leds[i] != (CRGB)(CRGB::Black)) alimSwitch = true;
      if (leds[i] != (leds[i] = ledsZones.getColor(i))) leds_diff = true;
    }

    FastLED.show(); // Apply
  }
  else if (sceneMode && millis() - timerLastInfo > 600000)
  { // 10 minutes after the last info, disable BS mode if still enabled
    sceneMode = false;
  } else if (!(alimSwitchReal || alimSwitch) && !sceneMode) delay(1000);

  if (!sceneMode && alimSwitch != alimSwitchReal)
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
      +---------+-------------+----------------+----------------- +---------------------+--------------------------------------+
      | Bit     | 0 mode      | 1 behavior     | 2 change color ? | 3,4,5 change for    | 6,7 Depends on bit 2                 |
      +---------+-------------+----------------+------------------+---------------------+--------------------------------------+
      |         | 0: Normal  \| 0: Uniform     |    Always true   | 0: Back Lasers      | In case # (slot)| Else (Chroma)      |
      |         |            /| 1: LED by LED  >----------------->| 1: Ring Light       | 0: Color 1      | 0: No Chroma event |
      | Meaning |         ----+----------------+------------------+ 2: Left Lasers      | 1: Color 2      | 1: RGB             |
      |         | 1: BS mode \| 0: Out-game   \| 0: Change Color #| 3: Right Lasers     | 2: Color 1 bis  | 2: Gradient        |
      |         |            /| 1: In-game    /| 1: Don't        >| 4: Center Light     | 3: Color 2 bis  |                    |
      +---------+-------------+----------------+------------------+ 5: Color bis        +--------------------------------------+
                                                                  | 6: Interscope left  |
                                                                  | 7: Interscope right |
                                                                  +---------------------+
    When we don't change color out-game, everything is disregarded but still must be present
    If it changes a color (0bXXXXXX0 or 0b00XX011):
      2nd, 3rd and 4th bytes are R, G and B
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
          3rd is easing ID
          4th, 5th, 6th and 7th are duration (float)
          8th, 9th, 10th are the starting color
          11th, 12th, 13rd are the ending color
*/

int bsMode(const char *buf)
{
  int bytesRead = 0;
  if (!(buf[0] & RECV_BEHAVIOR) != !inGame) {
    inGame = buf[0] & RECV_BEHAVIOR;
    for (int i = 0; i < NUM_ZONE; i++) {
      ledsZones.setColor(i, inGame ? CRGB::Black : i % 2 ? color1 : color2);
      ledsZones.setAnim(i, 3, 4);  // cubic is cool
    }
  }

  if ((buf[0] & (0b00000101)) == 1) { // If we're changing a color
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
    if (!inGame) {
      for (int i = 0; i < NUM_ZONE; i++) {
        ledsZones.setColor(i, i % 2 ? color1 : color2);
        ledsZones.setAnim(i, 3, 3);
        ledsZones.setDuration(i, 3);
      }
    }
    leds_diff = true;
    bytesRead = 4;
  } else {
    byte chroma = buf[0] & 0b11000000 >> 6;
    bytesRead = 2;
    if (inGame) {
      if (ZONE == 5) {
        bisColor = buf[1];
        color1 = bisColor ? color1valuebis : color1value;
        color2 = bisColor ? color2valuebis : color2value;
      } else {
        switch (buf[1])
        {
          case 0:
            ledsZones.setColor(ZONE, CRGB::Black);
            ledsZones.setDuration(ZONE, 0);
            ledsZones.setAnim(ZONE, 0);
            break;
          case 1:
            ledsZones.setColor(ZONE, color1);
            ledsZones.setDuration(ZONE, 0);
            ledsZones.setAnim(ZONE, 0);
            break;
          case 2:
            ledsZones.setColor(ZONE, color1);
            ledsZones.setDuration(ZONE, 0.6);
            ledsZones.setAnim(ZONE, 1);
            break;
          case 3:
            ledsZones.setColor(ZONE, color1);
            ledsZones.setDuration(ZONE, 3);
            ledsZones.setAnim(ZONE, 2);
            break;
          case 5:
            ledsZones.setColor(ZONE, color2);
            ledsZones.setDuration(ZONE, 0);
            ledsZones.setAnim(ZONE, 0);
            break;
          case 6:
            ledsZones.setColor(ZONE, color2);
            ledsZones.setDuration(ZONE, 0.6);
            ledsZones.setAnim(ZONE, 1);
            break;
          case 7:
            ledsZones.setColor(ZONE, color2);
            ledsZones.setDuration(ZONE, 3);
            ledsZones.setAnim(ZONE, 2);
            break;
        }
      }
    }
  }
  return bytesRead;
}

CRGB colorParsing(const char* buf) {
  /**
     @brief parse color from buf

     @param buf needs to be the color with the 1st byte beeing the red value

     @return color CRGB object beeing the color parsed
  */
  return CRGB(buf[0], buf[1], buf[2]);
}

int messageParsing(const char* buf)
{
  sceneMode = buf[0] & RECV_MODE;
  leds_diff = true;

  if (buf[0] & RECV_MODE)
    return bsMode(buf);
  else
    return normalMode(buf);   // Number of bytes read
}

int normalMode(const char *buf)
{
  if (buf[0] & 2)
  { // LED-by-LED
    ledsZones.setColor(ZONE, colorParsing(buf + 1));
  }
  else
  { // Uniform
    for (int i = 0; i < NUM_ZONE; i++) ledsZones.setColor(i, colorParsing(buf + 1));
  }
  return 4;
}

void messageReceived(MQTTClient *client, char topic[], char bytes[], int length)
{
  if (!strcmp(topic, "/led/ping") && !strcmp(bytes, "ping"))
  {
    pong = true;
  } else if (!strcmp(topic, "/led") || !strcmp(topic, "/led/stream")) {
    int startAt = 0;
    while (length > startAt) {
      startAt += messageParsing(bytes + startAt);
    }
  }
  timerLastInfo = millis();
}
