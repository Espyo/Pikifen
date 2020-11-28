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


class results_state : public game_state {
public:
    //Name of the area.
    string area_name;
    //How many enemies were beaten.
    size_t enemies_beaten;
    //How many enemies there were in total.
    size_t enemies_total;
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
    virtual void load();
    virtual void unload();
    virtual void handle_allegro_event(ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual string get_name() const;
    
private:
    //Area name widget.
    menu_text* area_name_widget;
    //Enemy count text widget.
    menu_text* enemies_widget;
    //Pikmin born text widget.
    menu_text* pikmin_born_widget;
    //Pikmin deaths text widget.
    menu_text* pikmin_deaths_widget;
    //Treasure point count text widget.
    menu_text* points_widget;
    //Time spent on this state.
    float time_spent;
    //Time taken text widget.
    menu_text* time_widget;
    
    void continue_playing();
    void leave();
    void retry_area();
};


#endif //ifndef RESULTS_INCLUDED
