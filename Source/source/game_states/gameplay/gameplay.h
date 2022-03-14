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

#include "../../controls.h"
#include "../../mobs/interactable.h"
#include "../../mobs/onion.h"
#include "../../mobs/pikmin.h"
#include "../../mobs/ship.h"
#include "../../replay.h"
#include "../game_state.h"
#include "../../gui.h"
#include "gameplay_utils.h"
#include "hud.h"
#include "onion_menu.h"
#include "pause_menu.h"


/* ----------------------------------------------------------------------------
 * Standard gameplay state. This is where the action happens.
 */
class gameplay_state : public game_state {
public:

    static const float AREA_INTRO_HUD_MOVE_TIME;
    static const float AREA_TITLE_FADE_DURATION;
    static const float CURSOR_TRAIL_SAVE_INTERVAL;
    static const unsigned char CURSOR_TRAIL_SAVE_N_SPOTS;
    static const float MENU_ENTRY_HUD_MOVE_TIME;
    static const float MENU_EXIT_HUD_MOVE_TIME;
    static const float SWARM_ARROW_SPEED;
    
    gameplay_state();
    
    //Is the player playing after hours?
    bool after_hours;
    //How much real time has passed since the area was loaded.
    float area_time_passed;
    //Timer used to fade out the area's title when the area is entered.
    timer area_title_fade_timer;
    //Name of the area to be loaded.
    string area_to_load;
    //Leaders available to control.
    vector<leader*> available_leaders;
    //Fog effect buffer.
    ALLEGRO_BITMAP* bmp_fog;
    //Group member closest to player 1's leader.
    mob* closest_group_member;
    //Is the group member closest to player 1's leader distant?
    bool closest_group_member_distant;
    //Index of player 1's current leader, in the array of available leaders.
    size_t cur_leader_nr;
    //Pointer to player 1's leader. Cache for convenience.
    leader* cur_leader_ptr;
    //What day it is, in-game.
    size_t day;
    //What time of the day is it in-game? In minutes.
    float day_minutes;
    //Replay of the current day.
    replay day_replay;
    //Information about the in-game HUD.
    hud_struct* hud;
    //Player 1's leader cursor, in screen coordinates.
    point leader_cursor_s;
    //Sector that player 1's leader cursor is on, if any.
    sector* leader_cursor_sector;
    //Player 1's leader cursor, in world coordinates.
    point leader_cursor_w;
    //List of all mobs in the area.
    mob_lists mobs;
    //Information about the message box currently active on player 1, if any.
    msg_box_info* msg_box;
    //Manager of all particles.
    particle_manager particles;
    //Path manager.
    path_manager path_mgr;
    //All droplets of precipitation.
    vector<point> precipitation;
    //Time until the next drop of precipitation.
    timer precipitation_timer;
    //Spray that player 1 has currently selected.
    size_t selected_spray;
    //How many of each spray/ingredients player 1 has.
    vector<spray_stats_struct> spray_stats;
    //All types of subgroups.
    subgroup_type_manager subgroup_types;
    //Angle at which player 1 is swarming.
    float swarm_angle;
    //General intensity of player 1's swarm in the specified angle.
    float swarm_magnitude;
    //Destination of player 1's throw.
    point throw_dest;
    //Mob that player 1's throw will land on, if any.
    mob* throw_dest_mob;
    //Sector that player 1's throw will land on, if any.
    sector* throw_dest_sector;
    //Have we went to the results screen yet?
    bool went_to_results;
    //Information about player 1's whistle.
    whistle_struct whistle;
    
    //Target to leave towards.
    enum LEAVE_TARGET {
        //Leave in order to retry the area.
        LEAVE_TO_RETRY,
        //Leave in order to finish the area.
        LEAVE_TO_FINISH,
        //Leave in order to go to the area selection.
        LEAVE_TO_AREA_SELECT,
    };
    
    void enter();
    void leave(const LEAVE_TARGET target);
    void start_leaving(const LEAVE_TARGET target);
    void change_spray_count(const size_t type_nr, signed int amount);
    void update_available_leaders();
    void update_closest_group_member();
    
    void load();
    void unload();
    void handle_allegro_event(ALLEGRO_EVENT &ev);
    void do_logic();
    void do_drawing();
    void update_transformations();
    string get_name() const;
    
private:

    //Control ID for player 1's cancel button. Cache for convenience.
    size_t cancel_control_id;
    //Points to an interactable close enough for player 1 to use, if any.
    interactable* close_to_interactable_to_use;
    //Points to a nest-like object close enough for player 1 to open, if any.
    pikmin_nest_struct* close_to_nest_to_open;
    //Points to a Pikmin close enough for player 1 to pluck, if any.
    pikmin* close_to_pikmin_to_pluck;
    //Points to a ship close enough for player 1 to heal in, if any.
    ship* close_to_ship_to_heal;
    //Ligthten player 1's cursor by this due to leader/cursor height difference.
    float cursor_height_diff_light;
    //Movement of player 1's cursor via non-mouse means.
    movement_struct cursor_movement;
    //Spots the cursor has been through. Used for the faint trail left behind.
    vector<point> cursor_spots;
    //Time left until the position of the cursor is saved on the vector.
    timer cursor_save_timer;
    //Is input enabled, for reasons outside the ready_for_input variable?
    bool is_input_allowed;
    //Bitmap that lights up the area when in blackout mode.
    ALLEGRO_BITMAP* lightmap_bmp;
    //Control ID for player 1's main button. Cache for convenience.
    size_t main_control_id;
    //Movement of player 1's leader.
    movement_struct leader_movement;
    //Information about the current Onion menu, if any.
    onion_menu_struct* onion_menu;
    //Information about the current pause menu, if any.
    pause_menu_struct* pause_menu;
    //Is the gameplay paused?
    bool paused;
    //The first frame shouldn't allow for input just yet, because
    //some things are still being set up within the first logic loop.
    //So forbid input until the second frame.
    bool ready_for_input;
    //Timer for the next replay state save.
    timer replay_timer;
    //Is player 1 holding the "swarm to cursor" button?
    bool swarm_cursor;
    //Reach of player 1's swarm.
    movement_struct swarm_movement;
    //Are we currently unloading the gameplay state?
    bool unloading;
    
    void do_aesthetic_logic();
    void do_game_drawing(
        ALLEGRO_BITMAP* bmp_output = NULL,
        ALLEGRO_TRANSFORM* bmp_transform = NULL
    );
    void do_gameplay_logic();
    void do_menu_logic();
    void draw_background(ALLEGRO_BITMAP* bmp_output);
    void draw_leader_cursor(const ALLEGRO_COLOR &color);
    void draw_ingame_text();
    void draw_lighting_filter();
    void draw_message_box();
    void draw_mouse_cursor(const ALLEGRO_COLOR &color);
    void draw_onion_menu();
    void draw_pause_menu();
    void draw_precipitation();
    void draw_system_stuff();
    void draw_throw_preview();
    void draw_tree_shadows();
    void draw_world_components(ALLEGRO_BITMAP* bmp_output);
    ALLEGRO_BITMAP* draw_to_bitmap();
    ALLEGRO_BITMAP* generate_fog_bitmap(
        const float near_radius, const float far_radius
    );
    void handle_button(
        const BUTTONS button, const float pos, const size_t player
    );
    void init_hud();
    void load_game_content();
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
    void process_system_key_press(const int keycode);
    void unload_game_content();
};


#endif //ifndef GAMEPLAY_INCLUDED
