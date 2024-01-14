#ifndef SONGDATA_HPP
#define SONGDATA_HPP
#pragma once

#include <vector>
#include <optional>
#include <cstdint>
#include <string>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <iostream>

#include "json.hpp"
#include "CRGB.hpp"
#include "miniaudio.h"

/**
 * @brief Struct describing a chroma event
 */
struct ChromaEvent
{
    /**
     * @brief LightIDs to affect
     */
    std::vector<int> lightID;
    /**
     * @brief Color to apply
     */
    CRGB color;
    /**
     * @brief Easing to use
     */
    enum Easing
    {
        LINEAR = 0,
        STEP = 1,
        SINE_IN = 2,
        SINE_OUT = 3,
        SINE_INOUT = 4,
        CUBIC_IN = 5,
        CUBIC_OUT = 6,
        CUBIC_INOUT = 7,
        QUINT_IN = 8,
        QUINT_OUT = 9,
        QUINT_INOUT = 10,
        CIRC_IN = 11,
        CIRC_OUT = 12,
        CIRC_INOUT = 13,
        ELASTIC_IN = 14,
        ELASTIC_OUT = 15,
        ELASTIC_INOUT = 16,
        QUAD_IN = 17,
        QUAD_OUT = 18,
        QUAD_INOUT = 19,
        QUART_IN = 20,
        QUART_OUT = 21,
        QUART_INOUT = 22,
        EXPO_IN = 23,
        EXPO_OUT = 24,
        EXPO_INOUT = 25,
        BACK_IN = 26,
        BACK_OUT = 27,
        BACK_INOUT = 28,
        BOUNCE_IN = 29,
        BOUNCE_OUT = 30,
        BOUNCE_INOUT = 31,
        FLASH = 40,
        FADE = 41,
    } easing;

    /**
     * @brief Get easing enum object from a string
     * @param easing String to convert
     * @return Easing enum object
     */
    Easing get_easing(std::string easing)
    {
        if (easing == "easeLinear")
            return LINEAR;
        else if (easing == "easeStep")
            return STEP;
        else if (easing == "easeSineIn")
            return SINE_IN;
        else if (easing == "easeSineOut")
            return SINE_OUT;
        else if (easing == "easeSineInOut")
            return SINE_INOUT;
        else if (easing == "easeCubicIn")
            return CUBIC_IN;
        else if (easing == "easeCubicOut")
            return CUBIC_OUT;
        else if (easing == "easeCubicInOut")
            return CUBIC_INOUT;
        else if (easing == "easeQuintIn")
            return QUINT_IN;
        else if (easing == "easeQuintOut")
            return QUINT_OUT;
        else if (easing == "easeQuintInOut")
            return QUINT_INOUT;
        else if (easing == "easeCircIn")
            return CIRC_IN;
        else if (easing == "easeCircOut")
            return CIRC_OUT;
        else if (easing == "easeCircInOut")
            return CIRC_INOUT;
        else if (easing == "easeElasticIn")
            return ELASTIC_IN;
        else if (easing == "easeElasticOut")
            return ELASTIC_OUT;
        else if (easing == "easeElasticInOut")
            return ELASTIC_INOUT;
        else if (easing == "easeQuadIn")
            return QUAD_IN;
        else if (easing == "easeQuadOut")
            return QUAD_OUT;
        else if (easing == "easeQuadInOut")
            return QUAD_INOUT;
        else if (easing == "easeQuartIn")
            return QUART_IN;
        else if (easing == "easeQuartOut")
            return QUART_OUT;
        else if (easing == "easeQuartInOut")
            return QUART_INOUT;
        else if (easing == "easeExpoIn")
            return EXPO_IN;
        else if (easing == "easeExpoOut")
            return EXPO_OUT;
        else if (easing == "easeExpoInOut")
            return EXPO_INOUT;
        else if (easing == "easeBackIn")
            return BACK_IN;
        else if (easing == "easeBackOut")
            return BACK_OUT;
        else if (easing == "easeBackInOut")
            return BACK_INOUT;
        else if (easing == "easeBounceIn")
            return BOUNCE_IN;
        else if (easing == "easeBounceOut")
            return BOUNCE_OUT;
        else if (easing == "easeBounceInOut")
            return BOUNCE_INOUT;
        else
            throw std::invalid_argument("Invalid easing string");
    }
};

struct Event
{
    /**
     * @brief Time in ns (from beats in the json)
     */
    uint64_t time;

    /**
     * @brief Compare two events
     * @param other Event to compare to
     * @return true if the time of this event is lower than the other
     */
    bool operator<(const Event &other) const
    {
        return time < other.time;
    }

    /**
     * @brief Type of event
     */
    enum Type
    {
        BACK_LASERS = 0,
        RING_LIGHTS = 1,
        LEFT_LASERS = 2,
        RIGHT_LASERS = 3,
        CENTER_LIGHTS = 4,
        EXTRA_LIGHTS = 10
    } type;

    /**
     * @brief Check wether the type is valid
     * @return true if the type has a value from the enum
     */
    bool valid_type()
    {
        return type == BACK_LASERS || type == RING_LIGHTS || type == LEFT_LASERS || type == RIGHT_LASERS || type == CENTER_LIGHTS || type == EXTRA_LIGHTS;
    }

    /**
     * @brief Value of the event (0-12) xx1 is blue, xx2 is red, xx3 is white
     */
    enum Value
    {
        OFF = 0,
        ON1 = 1,
        FLASH1 = 2,
        FADE1 = 3,
        TRANSITION1 = 4,
        ON2 = 5,
        FLASH2 = 6,
        FADE2 = 7,
        TRANSITION2 = 8,
        ON3 = 9,
        FLASH3 = 10,
        FADE3 = 11,
        TRANSITION3 = 12,
    } value;

    /**
     * @brief float value from the JSON (check https://bsmg.wiki/mapping/difficulty-format-v2.html#events-1 for more info)
     */
    std::optional<float> float_value;
    /**
     * @brief Optional chroma event
     */
    std::optional<ChromaEvent> chroma_event;
};

/**
 * @brief Struct describing a song
 */
struct SongData
{
    /**
     * @brief Path to the song folder
     */
    std::string path;
    /**
     * @brief Song path
     */
    std::string audio_path;
    /**
     * @brief Path to the event track file
     */
    std::string track_path;
    /**
     * @brief Song BPM
     */
    double bpm;
    /**
     * @brief Song events
     */
    std::vector<Event> events;
    /**
     * @brief Song object
     */
    ma_sound audio;

    /**
     * @brief Parse the event track present from `this.path`
     */
    void parse_track()
    {
        std::ifstream file(track_path, std::ifstream::in);
        if (!file.is_open())
        {
            std::cerr << "Error opening track file" << std::endl;
            throw std::runtime_error("Error opening track file");
        }
        nlohmann::json track_data = nlohmann::json::parse(file);
        file.close();

        int ver = track_data.contains("version") ? 3 : 2;

        if (ver == 3)
            throw std::runtime_error("Version 3 is not supported yet");
        // TODO: implement chroma and V3

        uint64_t last_time = 0;
        bool sorted = true;

        for (nlohmann::json::iterator it = ver == 2 ? track_data["_events"].begin() : track_data["basicBeatmapEvents"].begin(); it != track_data["_events"].end(); it++)
        {
            Event event;

            event.type = (*it)["_type"];
            if (!event.valid_type())
            {
                continue;
            }

            event.time = (((double)(*it)["_time"]) * (60. / bpm) * (1000000000));
            sorted &= event.time >= last_time;

            event.value = (*it)["_value"];
            // TODO : implement chroma
            // if ((*it)["_customData"].is_null())
            // {
            //     events.push_back(event);
            //     continue;
            // }
            // if ((*it)["_customData"]["_color"].is_null())
            // {
            //     events.push_back(event);
            //     continue;
            // }
            // event.chroma_event = ChromaEvent();
            // event.chroma_event->r = (*it)["_customData"]["_color"][0];
            // event.chroma_event->g = (*it)["_customData"]["_color"][1];
            // event.chroma_event->b = (*it)["_customData"]["_color"][2];
            events.push_back(event);
        }

        if (!sorted)
        {
#ifdef DEBUG
            printf("Events are not sorted, sorting...\n");
#endif
            std::sort(events.begin(), events.end());
#ifdef DEBUG
            printf("Events sorted\n");
#endif
        }

#ifdef DEBUG
        printf("Found %ld events\n", events.size());
#endif
    }

    /**
     * @brief Parse the song informations without the events (Populate avery objects except events and audio)
     */
    void parse_song()
    {
        // TODO: implement more stuff (https://bsmg.wiki/mapping/infodat-format.html)
        std::string info_path = path;
        info_path.append("info.dat");
        audio_path = path;
        track_path = path;

#ifdef DEBUG
        printf("Parsing info at %s\n", info_path.c_str());
#endif
        std::ifstream file(info_path, std::ifstream::in);
        if (!file.is_open())
        {
            // let's try with a capital I
            info_path = path;
            info_path.append("Info.dat");
            file.open(info_path, std::ifstream::in);
#ifdef DEBUG
            printf("File didn't open, parsing info at %s\n", info_path.c_str());
#endif
        }
        if (!file.is_open())
        {
            std::cerr << "Error opening info file" << std::endl;
            throw std::runtime_error("Error opening info file");
        }
        nlohmann::json info = nlohmann::json::parse(file);
        file.close();

        bpm = info["_beatsPerMinute"];
        audio_path.append(info["_songFilename"]);
        track_path.append(info["_difficultyBeatmapSets"][0]["_difficultyBeatmaps"][0]["_beatmapFilename"]);

#ifdef DEBUG
        printf("BPM: %f\n", bpm);
        printf("Audio path: %s\n", audio_path.c_str());
        printf("Track path: %s\n", track_path.c_str());
#endif
    }

    /**
     * @brief Parse the song informations without the events (Populate avery objects except events and audio)
     * @param path path to the folder of the song to parse
     */
    void parse_song(std::string path){
        this->path = path;
        parse_song();
    }

    /**
     * @brief Load the audio file
     * @param engine Engine to load the audio to
     */
    void load_audio(ma_engine *engine)
    {
        ma_result result = ma_sound_init_from_file(engine, audio_path.c_str(), 0, NULL, NULL, &audio);

        if (result != MA_SUCCESS)
        {
            printf("Failed to load sound.\n");
            throw std::runtime_error("Failed to load sound");
        }
    }
};
#endif