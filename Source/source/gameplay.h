/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gameplay state class and
 * gameplay state-related functions.
 */

#ifndef GAMEPLAY_INCLUDED
#define GAMEPLAY_INCLUDED

#include "game_state.h"
#include "mobs/interactable.h"
#include "mobs/onion.h"
#include "mobs/pikmin.h"
#include "mobs/ship.h"

/* ----------------------------------------------------------------------------
 * Standard gameplay state. This is where the action happens.
 */
class gameplay : public game_state {
private:

    static const float AREA_INTRO_HUD_MOVE_TIME;
    static const float CURSOR_INVALID_EFFECT_SPEED;
    static const float CURSOR_SAVE_INTERVAL;

    //When processing inter-mob events, we want the mob to follow them from the
    //closest mob to the one farthest away. As such, this struct saves data on
    //a viable mob, its distance, and the corresponding event.
    //We can then go through a vector of these pending intermob events in order.
    struct pending_intermob_event {
        dist d;
        mob_event* event_ptr;
        mob* mob_ptr;
        pending_intermob_event(
            const dist &d, mob_event* event_ptr, mob* mob_ptr
        ):
            d(d),
            event_ptr(event_ptr),
            mob_ptr(mob_ptr) {
            
        }
    };
    
    ALLEGRO_BITMAP* bmp_bubble;
    ALLEGRO_BITMAP* bmp_counter_bubble_group;
    ALLEGRO_BITMAP* bmp_counter_bubble_field;
    ALLEGRO_BITMAP* bmp_counter_bubble_standby;
    ALLEGRO_BITMAP* bmp_counter_bubble_total;
    ALLEGRO_BITMAP* bmp_day_bubble;
    ALLEGRO_BITMAP* bmp_distant_pikmin_marker;
    ALLEGRO_BITMAP* bmp_fog;
    ALLEGRO_BITMAP* bmp_hard_bubble;
    ALLEGRO_BITMAP* bmp_message_box;
    ALLEGRO_BITMAP* bmp_no_pikmin_bubble;
    ALLEGRO_BITMAP* bmp_sun;
    
    //Control ID for player 1's cancel button. Cache for convenience.
    size_t cancel_control_id;
    //Ligthten player 1's cursor by this due to leader/cursor height difference.
    float cursor_height_diff_light;
    //Movement of player 1's cursor via non-mouse means.
    movement_struct cursor_movement;
    //Spots the cursor has been through. Used for the faint trail left behind.
    vector<point> cursor_spots;
    //Time left until the position of the cursor is saved on the vector.
    timer cursor_save_timer;
    //Points to an interactable close enough for player 1 to use, if any.
    interactable* close_to_interactable_to_use;
    //Points to an Onion close enough for player 1 to open, if any.
    onion* close_to_onion_to_open;
    //Points to a Pikmin close enough for player 1 to pluck, if any.
    pikmin* close_to_pikmin_to_pluck;
    //Points to a ship close enough for player 1 to heal in, if any.
    ship* close_to_ship_to_heal;
    //What day it is, in-game.
    size_t day;
    //Control ID for player 1's main button. Cache for convenience.
    size_t main_control_id;
    //Timer for the next replay state save.
    timer replay_timer;
    
    void do_aesthetic_logic();
    void do_game_drawing(
        ALLEGRO_BITMAP* bmp_output = NULL,
        ALLEGRO_TRANSFORM* bmp_transform = NULL
    );
    void do_gameplay_logic();
    void draw_background(ALLEGRO_BITMAP* bmp_output);
    void draw_cursor(ALLEGRO_TRANSFORM &world_to_screen_drawing_transform);
    void draw_hud();
    void draw_ingame_text();
    void draw_lighting_filter();
    void draw_message_box();
    void draw_precipitation();
    void draw_system_stuff();
    void draw_tree_shadows();
    void draw_world_components(ALLEGRO_BITMAP* bmp_output);
    ALLEGRO_BITMAP* draw_to_bitmap();
    ALLEGRO_BITMAP* generate_fog_bitmap(
        const float near_radius, const float far_radius
    );
    void handle_button(
        const size_t button, const float pos, const size_t player
    );
    void load_game_content();
    void load_hud_info();
    void load_hud_coordinates(const int item, string data);
    void process_mob_interactions(mob* m_ptr, size_t m);
    void process_mob_misc_interactions(
        mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d,
        vector<pending_intermob_event> &pending_intermob_events
    );
    void process_mob_reaches(
        mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d,
        vector<pending_intermob_event> &pending_intermob_events
    );
    void process_mob_touches(
        mob* m_ptr, mob* m2_ptr, const size_t m, const size_t m2, dist &d
    );
    void unload_game_content();
    
public:
    gameplay();
    
    //How much real time has passed since the area was loaded.
    float area_time_passed;
    //Name of the area to be loaded.
    string area_to_load;
    //Group member closest to player 1's leader.
    mob* closest_group_member;
    //Is the group member closest to player 1's leader distant?
    bool closest_group_member_distant;
    //Index of player 1's current leader, in the array of leaders.
    size_t cur_leader_nr;
    //Pointer to player 1's leader. Cache for convenience.
    leader* cur_leader_ptr;
    //What time of the day is it in-game? In minutes.
    float day_minutes;
    
    void leave();
    void update_closest_group_member();
    
    virtual void load();
    virtual void unload();
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual void update_transformations();
    virtual string get_name();
};

#endif //ifndef GAMEPLAY_INCLUDED
