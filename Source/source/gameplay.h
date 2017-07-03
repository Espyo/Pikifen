/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gameplay state class and
 * gameplay state-related functions.
 */

#ifndef GAMEPLAY_INCLUDED
#define GAMEPLAY_INCLUDED

#include "game_state.h"

/* ----------------------------------------------------------------------------
 * Standard gameplay state. This is where the action happens.
 */
class gameplay : public game_state {
private:
    ALLEGRO_BITMAP* bmp_bubble;
    ALLEGRO_BITMAP* bmp_counter_bubble_group;
    ALLEGRO_BITMAP* bmp_counter_bubble_field;
    ALLEGRO_BITMAP* bmp_counter_bubble_standby;
    ALLEGRO_BITMAP* bmp_counter_bubble_total;
    ALLEGRO_BITMAP* bmp_day_bubble;
    ALLEGRO_BITMAP* bmp_distant_pikmin_marker;
    ALLEGRO_BITMAP* bmp_hard_bubble;
    ALLEGRO_BITMAP* bmp_message_box;
    ALLEGRO_BITMAP* bmp_no_pikmin_bubble;
    ALLEGRO_BITMAP* bmp_sun;
    
    point cam_box[2];
    
    void do_aesthetic_logic();
    void do_game_drawing(
        ALLEGRO_BITMAP* bmp_output = NULL,
        ALLEGRO_TRANSFORM* bmp_transform = NULL
    );
    void do_gameplay_logic();
    ALLEGRO_BITMAP* draw_to_bitmap();
    void load_game_content();
    void load_hud_info();
    void load_hud_coordinates(const int item, string data);
    void process_mob(mob* m_ptr, size_t m);
    
public:
    gameplay();
    virtual void load();
    virtual void unload();
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual void update_transformations();
};

#endif //ifndef GAMEPLAY_INCLUDED
