/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the maker tool structures and functions.
 */

#pragma once

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../misc_structs.hpp"

using std::string;
using std::vector;


namespace CONSOLE {
extern const float CARET_BLINK_INTERVAL;
extern const ALLEGRO_COLOR COLOR_BG;
extern const ALLEGRO_COLOR COLOR_DECOR_DARK;
extern const ALLEGRO_COLOR COLOR_DECOR_LIGHT;
extern const ALLEGRO_COLOR COLOR_ERROR;
extern const ALLEGRO_COLOR COLOR_MAIN;
extern const ALLEGRO_COLOR COLOR_MUTED;
extern const ALLEGRO_COLOR COLOR_WARNING;
extern const float INPUT_ICON_SIZE;
extern const size_t INPUT_TEXT_MAX_SIZE;
extern const int LINE_PADDING;
extern const size_t OUTPUT_INDENT;
extern const size_t OUTPUT_MAX_CHARS_PER_LINE;
extern const size_t OUTPUT_MAX_SIZE;
extern const size_t OUTPUT_MAX_VISIBLE_LINES;
extern const int PADDING;
extern const float VISIBILITY_CHANGE_SPEED;
}


/**
 * @brief Represents the interactive terminal portion of the maker toolkit
 * console. Users can use this to write commands and see previous outputs.
 */
struct MakerConsoleTerminal {

    //--- Public function declarations ---
    
    MakerConsoleTerminal();
    void clear();
    int draw(int y) const;
    bool handleAllegroEvent(const ALLEGRO_EVENT& ev);
    void tick(float deltaT);
    void write(LOG_LEVEL level, const string& text);
    void open(bool instant = false);
    void close(bool instant = false);
    void toggle(bool instant = false);
    OPEN_CLOSE_STATE getVisibilityState() const;
    
    
    private:
    
    //--- Private members ---
    
    //Visibility state.
    OPEN_CLOSE_STATE visibilityState = OPEN_CLOSE_STATE_CLOSED;
    
    //How much of it is visible [0 - 1].
    //Used when scrolling it in and out of view.
    float visibility = 0.0f;
    
    //How many entries to scroll up from the bottom.
    size_t scroll = 0;
    
    //Text that had been written out, in chronological order.
    vector<string> output;
    
    //Timestamp for each output line.
    vector<float> outputTimestamps;
    
    //Log level of each output line.
    vector<LOG_LEVEL> outputLevels;
    
    //Currently written input text.
    TextInput input;
    
    
    //--- Private function declarations ---
    
    void submitCommand();
    
};


/**
 * @brief Represents a small window at the top of the screen that shows
 * any important warnings, errors, or other outputs, informing the user
 * to open the terminal for more information.
 */
struct MakerConsoleNotifier {

    //--- Public function declarations ---
    
    void clear();
    int draw(int y) const;
    void tick(float deltaT);
    void write(LOG_LEVEL level, const string& text, float duration);
    void open(bool instant = false);
    void close(bool instant = false);
    
    
    private:
    
    //--- Private members ---
    
    //Visibility state.
    OPEN_CLOSE_STATE visibilityState = OPEN_CLOSE_STATE_CLOSED;
    
    //How much of it is visible [0 - 1].
    //Used when fading it in and out of view.
    float visibility = 0.0f;
    
    //Text that is currently being shown.
    string text;
    
    //Log level of the current text.
    LOG_LEVEL level = LOG_LEVEL_INFO;
    
    //How long until the notifier is closed automatically.
    float timeLeft = 0.0f;
    
};


/**
 * @brief Represents the interactive console that shows up at the top of
 * the screen, allowing makers to debug their content.
 */
struct MakerConsole {

    //--- Public function declarations ---
    
    void clear();
    int draw(int y) const;
    bool handleAllegroEvent(const ALLEGRO_EVENT& ev);
    void tick(float deltaT);
    void write(
        LOG_LEVEL level, const string& text,
        float notificationDuration = 10.0f
    );
    void openTerminal(bool instant = false);
    void closeTerminal(bool instant = false);
    void toggleTerminal(bool instant = false);
    
    
    private:
    
    //--- Private members ---
    
    //The terminal.
    MakerConsoleTerminal terminal;
    
    //The notifier.
    MakerConsoleNotifier notifier;
    
};
