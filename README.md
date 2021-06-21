# BS-Lightshow

This repository hosts code made to show a lightshow out of a beataber map. It currently works with ESP32 and MQTT.

I plan to make it easily movable to other platforms (there isn't so much hope for AVR boards however, the framerate I get on leds isn't satisfying)

I also would like to make a BS mode to show this while playing however I'm not used enough with mods/C# to do so for now, help would be highly appreciated.

## How to use

### Requirements

* ESP32 (other platforms supported by FastLED may work) with the libraries
  * [Arduino-MQTT](https://github.com/256dpi/arduino-mqtt)
  * [FastLED](https://github.com/FastLED/FastLED)
  * WiFiSecure (Integrated with arduino-esp)
* An adressable LED strip (I personnaly use [this one from AliExpress](https://aliexpress.com/item/32682015405.html)) and a corresponding power supply
* A MQTT server

### Steps

1. Configure your MQTT server
2. Modify [MQTT-esp32.ino](MQTT-esp32/MQTT-esp32.ino) to match your LED strip (and MQTT server)
    1. Modify compilator constants *(`NUM_LEDS` is the number of LEDs on your strip, `NUM_ZONE` is the number of zones you want to map on them (there is 5 of them on BS), `ALIM` is a pin where you can put something to cut the LEDs alimentation when they're all black (HIGH = on), `LED` is the data pin for the LEDs)*
    2. In the function `setup`
        1. In `FastLED.addLeds<WS2812B, LED, GRB>(leds, NUM_LEDS)` replace "WS2812B" by your LEDs model and GRB by your LEDs color order (see [FastLED documentation](https://github.com/FastLED/FastLED/wiki/Chipset-reference))
        2. In the for loop `Setting zones`, tweak the conditions to match your zone mapping
    3. *If your MQTT server doesn't use TLS*, change the port in `client.begin(host, 8883, net);` and replace the line `net.setCACert(letsEncyptCA);` with `net.setInsecure();`
    4. *If your MQTT server do use TLS but not with a Let's encrypt certificate*, change `const char letsEncyptCA[]` value to your AC's public key pam encoded.
3. Create the file `creditentials.h` in the same directory as [MQTT-esp32.ino](MQTT-esp32/MQTT-esp32.ino) and fill it with :

    ```cpp
    const char *ssid = "Your WiFi SSID";
    const char *pass = "Your verry secret WPA key";
    const char *host = "The host name of your MQTT server";
    const char *user = "Your MQTT username";
    const char *mqttPass = "Duh look at the variable's name (it's your MQTT password, who guessed !)";
    ```

4. Compile and send to your ESP32

5. Enjoy (For now the only way is through [theses scripts](Maestro/))

## TODO

- [x] Integrate Chroma events
    - [x] Remaster `LedsZones` to integrate easing functions
    - [x] Rethink the message object to allow passing/parsing chroma events
    - [x] Make the parser
- [ ] Improve the Chroma integration
    - [ ] Find a way to deal with `_lightID`
- [x] Try to make the normal behavior cleaner (done using a custom ~~easing~~ function)
- [ ] Clean up code
- [ ] Otpimize (some behavior are probably more complicated than needed)
