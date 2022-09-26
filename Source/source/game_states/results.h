/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the exploration/mission results game state.
 */

#ifndef RESULTS_INCLUDED
#define RESULTS_INCLUDED

#include "game_state.h"
#include "../gui.h"

namespace RESULTS {
extern const string GUI_FILE_PATH;
}


/* ----------------------------------------------------------------------------
 * Information about the area results menu.
 */
class results_state : public game_state {
public:
    results_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    //GUI manager.
    gui_manager gui;
    //Time spent on this state.
    float gui_time_spent;
    //GUI items that need to grow during the periodic text animation.
    vector<gui_item*> text_to_animate;
    //Stats box GUI item.
    list_gui_item* stats_box;
    //Final mission score. Cache for convenience.
    int final_mission_score;
    
    void add_stat(
        const string &label, const string &value,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
    void continue_playing();
    void leave();
    void retry_area();
};


#endif //ifndef RESULTS_INCLUDED
