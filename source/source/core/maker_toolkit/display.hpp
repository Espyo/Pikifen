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

#include "../misc_structs.hpp"

using std::string;



/**
 * @brief Represents the non-interactive display that shows up at the top of
 * the screen, allowing makers to debug their content.
 */
struct MakerDisplay {

    //--- Public function declarations ---
    
    int draw(int y) const;
    void tick(float deltaT);
    void write(const string& text, float duration = 0.1f);
    void open(bool instant = false);
    void close(bool instant = false);
    void reset();
    
    
    private:
    
    //--- Private members ---
    
    //Visibility state.
    OPEN_CLOSE_STATE visibilityState = OPEN_CLOSE_STATE_CLOSED;
    
    //How much of it is visible [0 - 1].
    //Used when fading it in and out of view.
    float visibility = 0.0f;
    
    //Text that is currently being shown.
    string text;
    
    //How long until the display is closed automatically.
    float timeLeft = 0.0f;
    
};
