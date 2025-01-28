/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the exploration/mission results game state.
 */

#pragma once

#include "../content/area/mission.h"
#include "../content/other/gui.h"
#include "../util/drawing_utils.h"
#include "game_state.h"


namespace RESULTS {
extern const string GUI_FILE_PATH;
}


/**
 * @brief Info about the area results menu.
 */
class results_state : public game_state {

public:

    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:

    //--- Members ---
    
    //GUI manager.
    gui_manager gui;
    
    //Time spent on this state.
    float gui_time_spent = 0.0f;
    
    //GUI items that need to grow during the periodic text animation.
    vector<gui_item*> text_to_animate;
    
    //Stats box GUI item.
    list_gui_item* stats_list = nullptr;
    
    //Final mission score. Cache for convenience.
    int final_mission_score = 0;
    
    
    //--- Function declarations ---
    
    void add_score_stat(const MISSION_SCORE_CRITERIA criterion);
    void add_stat(
        const string &label, const string &value,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
    void continue_playing();
    void leave();
    void retry_area();
    
};
