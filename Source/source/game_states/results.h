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
    size_t points_obtained;
    //How many treasure points there were in total.
    size_t points_total;
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
    //Time taken GUI item.
    text_gui_item* time_text;
    //Pikmin born point total GUI item.
    text_gui_item* pikmin_born_point_text;
    //Pikmin deaths point total GUI item.
    text_gui_item* pikmin_death_points_text;
    //Seconds left point total GUI item.
    text_gui_item* seconds_left_points_text;
    //Seconds passed point total GUI item.
    text_gui_item* seconds_passed_points_text;
    //Treasure point point total GUI item.
    text_gui_item* treasure_points_points_text;
    //Enemy point point total GUI item.
    text_gui_item* enemy_points_points_text;
    //Final score GUI item.
    text_gui_item* final_score_text;
    
    void continue_playing();
    void leave();
    void retry_area();
    
    static const string GUI_FILE_PATH;
};


#endif //ifndef RESULTS_INCLUDED
