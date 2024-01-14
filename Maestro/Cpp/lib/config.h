#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#include <string>
#include <optional>

struct Config
{
    /**
     * @brief Path to the config file
     */
    std::optional<std::string> config_file = "config.json";
    /**
     * @brief Provide a bit of debug information
     */
    std::optional<bool> is_debug;
    /**
     * @brief Enable usb dumb mode (blast RGB data)  &nbsp;  
     * "Smart" mode is not implemented yet
     */
    std::optional<bool> dumb = true;
    /**
     * @brief The number of leds in the strip
     */
    std::optional<int> led_count;
    /**
     * @brief Nano seconds between each refresh of the leds
     */
    std::optional<int> refresh_delay_ns = 13000000;

    /**
     * @brief Number of zones (probably shouldn't be changed, most of the code is hardcoded for 5 zones)
     */
    std::optional<int> nb_zones = 5;
    /**
     * @brief Zones configuration
     */
    struct Zones
    {
        /**
         * @brief Index of the first led in the zone (inclusive)
         */
        int start = 0;
        /**
         * @brief Index of the last led in the zone (inclusive)
         */
        int end = 0;
    } zones[5];
    
    /**
     * @brief tty to send data to
     */
    std::optional<std::string> device;
};

#endif