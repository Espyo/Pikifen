/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the day results game state.
 */

#ifndef RESULTS_INCLUDED
#define RESULTS_INCLUDED

#include "game_state.h"
#include "../gui.h"


/* ----------------------------------------------------------------------------
 * Information about the area results menu.
 */
class results_state : public game_state {
public:
    //Can the player choose to continue playing?
    bool can_continue;
    //Name of the area.
    string area_name;
    //How many enemies were beaten.
    size_t enemies_beaten;
    //How many enemies there were in total.
    size_t enemies_total;
    //Did the player suffer a total leader KO?
    bool leader_ko;
    //Did the player run out of time?
    bool out_of_time;
    //How many Pikmin were born.
    size_t pikmin_born;
    //How many Pikmin died.
    size_t pikmin_deaths;
    //How many treasure points were gathered.
    size_t treasure_points_obtained;
    //How many treasure points there were in total.
    size_t treasure_points_total;
    //How much time was taken.
    float time_taken;
    
    results_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    void reset();
    
private:
    //GUI manager.
    gui_manager gui;
    //Time spent on this state.
    float time_spent;
    
    //Area name GUI item.
    text_gui_item* area_name_text;
    //Area subtitle GUI item.
    text_gui_item* area_subtitle_text;
    //Stats box GUI item.
    list_gui_item* stats_box;
    
    void add_stat(
        const string &label, const string &value,
        const ALLEGRO_COLOR &color = COLOR_WHITE
    );
    void continue_playing();
    void leave();
    void retry_area();
    
    static const string GUI_FILE_PATH;
};


#endif //ifndef RESULTS_INCLUDED
