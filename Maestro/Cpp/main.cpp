// main.cpp

#include <iostream>
#include <fstream>
#include <math.h>
#include <filesystem>
#include <vector>
#include <optional>
#include <cstdint>
#include <algorithm>
#include <thread>

#include "lib/LedsZones.hpp"
#include "lib/json.hpp"
#include "lib/config.h"
#include "lib/argparse.hpp"
#include "lib/SongData.hpp"

#include "lib/stb_vorbis.c"
#define MINIAUDIO_IMPLEMENTATION
#include "lib/miniaudio.h"

#define abs(x) ((x) > 0 ? (x) : -(x))

Config config;
struct Args : public argparse::Args
{
    std::optional<std::string> &config_file = kwarg("c,config", "Path to the config file");
    std::optional<bool> &is_debug = flag("d,debug", "Provide debug information and ignore non-fatal errors");
    std::optional<int> &led_count = kwarg("l,led-count", "Number of leds in the strip.\tZone definition still needs a config file");
    std::optional<std::string> &device = kwarg("D,device", "tty to send data to. Implicit value will search for /dev/ttyASM or /dev/ttyUSB", "");
    std::optional<int> &refresh_delay_ns = kwarg("r,refresh-delay", "Nano seconds between each refresh of the leds");
    std::string &data = arg("data", "Data to send to the leds");

    void apply(Config &config)
    {
        if (config_file)
        {
            config.config_file = *config_file;
        }
        if (is_debug)
        {
            config.is_debug = *is_debug;
        }
        if (led_count)
        {
            config.led_count = *led_count;
        }
        if (refresh_delay_ns)
        {
            config.refresh_delay_ns = *refresh_delay_ns;
        }
        if (device)
        {
            config.device = *device;
        }
    }
};

std::string find_device()
{
    std::filesystem::directory_iterator dir("/dev");
    for (auto &file : dir)
    {
        if (file.path().string().find("ttyUSB") != std::string::npos || file.path().string().find("ttyACM") != std::string::npos)
        {
            printf("Found device: %s\n", file.path().string().c_str());
            return file.path().string();
        }
    }
    std::cerr << "Error: no ttyUSB or ttyACM found in /dev" << std::endl;
    config.is_debug.value_or(false) ? printf("Ignoring this error as is_debug is true\n") : throw std::runtime_error("Error: no ttyUSB or ttyACM found in /dev");
    return "";
}

std::string colored_square(uint8_t r, uint8_t g, uint8_t b)
{
    // "\033[48;2;{r};{g};{b}m   \033[0m"
    std::string red = std::to_string(r);
    std::string green = std::to_string(g);
    std::string blue = std::to_string(b);
    std::string out = "\033[48;2;" +
           std::string(3 - std::min(3, (int) red.length()), '0') + red + ";" +
           std::string(3 - std::min(3, (int) green.length()), '0') + green + ";" +
           std::string(3 - std::min(3, (int) blue.length()), '0') + blue + "m \033[0m";
    return out;
}

void create_config()
{
    std::ofstream temp(config.config_file.value().empty() ? "config.json" : config.config_file.value(), std::ofstream::out);
    if (!temp.is_open())
    {
        std::cerr << "Error creating config file" << std::endl;
        throw std::runtime_error("Error creating config file");
    }

    nlohmann::json data;
    data["is_debug"] = config.is_debug.value_or(false);
#ifdef DEBUG
    data["is_debug"] = true;
#endif
    data["dumb"] = config.dumb.has_value() ? config.dumb.value() : true;
    data["led_count"] = config.led_count.has_value() ? config.led_count.value() : 0;
    data["refresh_delay_ns"] = config.refresh_delay_ns.has_value() ? config.refresh_delay_ns.value() : 13000000;
    for (int i = 0; i < 5; i++)
    {
        data["zones"][i]["start"] = 0;
        data["zones"][i]["end"] = 0;
    }
    if (data["led_count"] > 0)
        data["zones"][4]["end"] = data["led_count"]; // allow for quick tests
    data["device"] = config.device.has_value() ? config.device.value() : find_device();

    temp << std::setw(4) << data << std::endl;

    temp.close();
}

void open_config()
{
    std::ifstream file(config.config_file.value().empty() ? "config.json" : config.config_file.value(), std::ifstream::in);
    if (!file.is_open())
    {
        std::cerr << "Error opening config file, creating one using default values... Please change the led count" << std::endl;

        create_config();

        file.open(config.config_file.value().empty() ? "config.json" : config.config_file.value(), std::ifstream::in);

        if (!file.is_open())
        {
            std::cerr << "Error opening the config file I created, aborting" << std::endl;
            throw std::runtime_error("Error opening config file");
        }
    }

    nlohmann::json data = nlohmann::json::parse(file);
    file.close();

    config.is_debug = config.is_debug.value_or(data["is_debug"]);
    config.dumb = config.dumb.value_or(data["dumb"]);
    config.led_count = config.led_count.value_or(data["led_count"]);
    config.refresh_delay_ns = config.refresh_delay_ns.value_or(data["refresh_delay_ns"]);
    config.device = config.device.value_or(data["device"]);
    for (int i = 0; i < 5; i++)
    {
        config.zones[i].start = data["zones"][i]["start"];
        config.zones[i].end = data["zones"][i]["end"];
    }

    if (config.led_count == 0)
    {
        std::cerr << "Error: led_count is 0" << std::endl;
        config.is_debug.value_or(false) ? printf("Ignoring this error as is_debug is true\n") : throw std::runtime_error("Error: led_count is 0");
    }

    if (config.is_debug.value_or(false))
    {
        printf("Config: \n");
        printf("\tisDebug: %d\n", config.is_debug.value());
        printf("\tledCount: %d\n", config.led_count.value());
        printf("\trefreshDelay: %d\n", config.refresh_delay_ns.value());
        for (int i = 0; i < 5; i++)
        {
            printf("\tzone %d: [%d\t%d]\n", i, config.zones[i].start, config.zones[i].end);
        }
        printf("\tdevice: %s\n", config.device.value().c_str());
    }
}

void reset_leds(LedsZones &leds)
{
    for (int i = 0; i < config.nb_zones.value(); i++)
    { // Fade to black (0, 0, 0) in 100 ms
        leds.setAnim(i, 2);
        leds.setColor(i, CRGB(0, 0, 0));
        leds.setDuration(i, 100000);
    }
}

void send_data(std::filebuf &device, uint8_t *last_data, LedsZones &leds)
{
    uint8_t data[leds.getNbLEDs() * 3];
    leds.getRaw(data);
    // TODO: windowing and repeat support

    device.sputc(0x01);                               // Start of frame
    device.sputn((char *)data, leds.getNbLEDs() * 3); // Data
    device.sputc(0x02);                               // End of frame

    if (config.is_debug.value_or(false))
    {
        for (int i = 0; i < leds.getNbLEDs()*24 + 20; i++)
            std::cout << "\b";
        std::cout << "Current leds: ";
        for (int i = 0; i < leds.getNbLEDs(); i++)
        {
            std::cout << colored_square(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
        }
        std::cout << std::flush;
    }
}

int main(int argc, char **argv)
{
    Args args = argparse::parse<Args>(argc, argv);
    if (args.is_debug.value_or(false))
    {
        args.print();
        printf("Args parsed\nLoading config file...\n");
    }
    args.apply(config);

    try
    {
        open_config();
    }
    catch (nlohmann::json::type_error &e)
    {
        std::cerr << "Error parsing config file: " << e.what() << "\nSome types are wrong, please fix or backup your file to let me create a new one" << std::endl;
        return 1;
    }
    catch (nlohmann::json::parse_error &e)
    {
        std::cerr << "Error parsing config file: " << e.what() << "\nFile is probably malformed, when in doubt, backup this file under an other name to let me create a new sane one" << std::endl;
        return 1;
    }

    if (config.is_debug.value_or(false))
        printf("Config file loaded\nDefining leds...\n");

    LedsZones leds(config.nb_zones.value(), config.led_count.value());

    for (int i = 0; i < config.nb_zones.value(); i++)
    {
        for (int j = config.zones[i].start; j <= config.zones[i].end; j++)
        {
            leds.setZone(j, i);
        }
    }

    leds.computeAttributes();

    if (config.is_debug.value_or(false))
        printf("Leds defined\nInitializing audio engine...\n");

    ma_engine engine;
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS)
    {
        printf("Failed to initialize audio engine.\n");
        return -1;
    }

    if (config.is_debug.value_or(false))
        printf("Audio engine initialized\nOpenning serial port...\n");

    std::filebuf device;
    if (!device.open(config.device.value(), std::ios::out | std::ios::out | std::ios::binary))
    {
        std::cerr << "Error opening serial port" << std::endl;
        config.is_debug.value_or(false) ? printf("Ignoring this error as is_debug is true, using 'out.bin' instead\n") : throw std::runtime_error("Error opening serial port");
        if (!device.open("out.bin", std::ios::out | std::ios::out | std::ios::binary))
        {
            std::cerr << "Error opening out.bin, impossible to recover" << std::endl;
            throw std::runtime_error("Error opening out.bin");
        }
    }
    device.pubsetbuf(0, 0);

    if (config.is_debug.value_or(false))
        printf("Serial port opened\n");

    if (config.is_debug.value_or(false))
        printf("Starting main loop\n");

    reset_leds(leds);

    if (args.data.back() != '/')
        args.data.append("/");

    SongData current_song;
    current_song.parse_song(args.data);
    current_song.parse_track();
    current_song.load_audio(&engine);

    timespec now;
    uint64_t last_update;
    uint64_t start = 0;
    uint64_t event_index = 0;
    bool changed = false;
    uint8_t last_data[leds.getNbLEDs() * 3];

    clock_gettime(CLOCK_MONOTONIC, &now);
    last_update = now.tv_nsec + now.tv_sec * 1000000000;
    start = last_update;
    result = ma_sound_start(&current_song.audio);
    if (result != MA_SUCCESS)
    {
        printf("Failed to start sound.\n");
        return -1;
    }

    while (!ma_sound_at_end(&current_song.audio))
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t now_ns = now.tv_nsec + now.tv_sec * 1000000000;
        uint64_t music_now = ma_sound_get_time_in_milliseconds(&current_song.audio) * 1000000; // probably not reliable => 19ms definition; * 10^6 to get ns from ms

        if (music_now > now_ns - start + 20000000)
        {
            // Difference between music and leds is too big, change start time
            if (config.is_debug.value_or(false))
                printf("Difference between music time and PC time is %ld\n", now_ns - music_now);
            start = now_ns - music_now;
        }

        while (current_song.events[event_index].time < now_ns - start && event_index < current_song.events.size())
        {
            if (config.is_debug.value_or(false))
            {
                for (int i = 0; i < leds.getNbLEDs()*24 + 20; i++)
                    std::cout << "\b" << " " << "\b";
                std::cout << std::flush;
                printf("Event %ld\tat %ld\ttype %d value %d\n", event_index, now_ns, current_song.events[event_index].type, current_song.events[event_index].value);
            }
            leds.register_event(current_song.events[event_index++]);
        }

        if (now_ns - last_update > config.refresh_delay_ns)
        {
            changed = leds.computeAnimations();
            last_update = now_ns;
        }
        if (changed)
        {
            send_data(device, last_data, leds);
            changed = false;
        }
        else
        {
            if (event_index < current_song.events.size())
                std::this_thread::sleep_for(std::chrono::nanoseconds(std::min(now_ns - start - current_song.events[event_index].time, (uint64_t)(config.refresh_delay_ns.value() - (now_ns - last_update)))));
            else
                std::this_thread::sleep_for(std::chrono::nanoseconds(config.refresh_delay_ns.value() - (now_ns - last_update)));
        }
    }

    return 0;
}