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
#include "../../mission.h"
#include "../../replay.h"
#include "../game_state.h"
#include "../../gui.h"
#include "gameplay_utils.h"
#include "hud.h"
#include "onion_menu.h"
#include "pause_menu.h"

namespace GAMEPLAY {
extern const float AREA_INTRO_HUD_MOVE_TIME;
extern const float AREA_TITLE_FADE_DURATION;
extern const float BIG_MSG_GO_DUR;
extern const string BIG_MSG_GO_TEXT;
extern const float BIG_MSG_MISSION_CLEAR_DUR;
extern const string BIG_MSG_MISSION_CLEAR_TEXT;
extern const float BIG_MSG_MISSION_FAILED_DUR;
extern const string BIG_MSG_MISSION_FAILED_TEXT;
extern const float BIG_MSG_READY_DUR;
extern const string BIG_MSG_READY_TEXT;
extern const float CAMERA_BOX_MARGIN;
extern const float CAMERA_SMOOTHNESS_MULT;
extern const unsigned char COLLISION_OPACITY;
extern const unsigned char CURSOR_TRAIL_MAX_ALPHA;
extern const float CURSOR_TRAIL_MAX_WIDTH;
extern const float CURSOR_TRAIL_MIN_SPOT_DIFF;
extern const int FOG_BITMAP_SIZE;
extern const unsigned char PREVIEW_OPACITY;
extern const float PREVIEW_TEXTURE_SCALE;
extern const float PREVIEW_TEXTURE_TIME_MULT;
extern const float REPLAY_SAVE_FREQUENCY;
extern const float TREE_SHADOW_SWAY_AMOUNT;
extern const float TREE_SHADOW_SWAY_SPEED;
extern const float CURSOR_TRAIL_SAVE_INTERVAL;
extern const unsigned char CURSOR_TRAIL_SAVE_N_SPOTS;
extern const float MENU_ENTRY_HUD_MOVE_TIME;
extern const float MENU_EXIT_HUD_MOVE_TIME;
extern const float SWARM_ARROW_SPEED;
}


//Types of interludes -- stuff before or after gameplay proper in the area.
enum INTERLUDES {
    //None.
    INTERLUDE_NONE,
    //Ready?
    INTERLUDE_READY,
    //Mission end, be it due to a clear or a fail.
    INTERLUDE_MISSION_END,
};


//Types of big messages -- text that appears in large letters on-screen.
enum BIG_MESSAGES {
    //None.
    BIG_MESSAGE_NONE,
    //Ready?
    BIG_MESSAGE_READY,
    //Go!
    BIG_MESSAGE_GO,
    //Mission clear!
    BIG_MESSAGE_MISSION_CLEAR,
    //Mission failed...
    BIG_MESSAGE_MISSION_FAILED,
};


/* ----------------------------------------------------------------------------
 * Standard gameplay state. This is where the action happens.
 */
class gameplay_state : public game_state {
public:

    gameplay_state();
    
    //Is the player playing after hours?
    bool after_hours;
    //How many seconds since area load. Only counts during actual gameplay.
    float area_time_passed;
    //Timer used to fade out the area's title when the area is entered.
    timer area_title_fade_timer;
    //Leaders available to control.
    vector<leader*> available_leaders;
    //Fog effect buffer.
    ALLEGRO_BITMAP* bmp_fog;
    //Closest to player 1's leader, for the previous, current, next type.
    mob* closest_group_member[3];
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
    //Multiply the delta_t by this much. Only affects gameplay stuff, not menus.
    float delta_t_mult;
    //Replay of the gameplay.
    replay gameplay_replay;
    //Information about the in-game HUD.
    hud_struct* hud;
    //Position of the last enemy killed. LARGE_FLOAT for none.
    point last_enemy_killed_pos;
    //Position of the last leader to get hurt. LARGE_FLOAT for none.
    point last_hurt_leader_pos;
    //Position of the last Pikmin born. LARGE_FLOAT for none.
    point last_pikmin_born_pos;
    //Position of the last Pikmin that died. LARGE_FLOAT for none.
    point last_pikmin_death_pos;
    //Position of the last ship that got a treasure. LARGE_FLOAT for none.
    point last_ship_that_got_treasure_pos;
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
    //ID of the next mob to be created.
    size_t next_mob_id;
    //Current notification.
    notification_struct notification;
    //Manager of all particles.
    particle_manager particles;
    //Path manager.
    path_manager path_mgr;
    //Path of the folder of the area to be loaded.
    string path_of_area_to_load;
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
    //Are we currently unloading the gameplay state?
    bool unloading;
    //Have we went to the results screen yet?
    bool went_to_results;
    //Information about player 1's whistle.
    whistle_struct whistle;
    //IDs of mobs remaining for the current mission goal, if applicable.
    unordered_set<size_t> mission_remaining_mob_ids;
    //How many mobs are required for the mission goal. Cache for convenience.
    size_t mission_required_mob_amount;
    //How many Pikmin born so far.
    size_t pikmin_born;
    //How many Pikmin deaths so far.
    size_t pikmin_deaths;
    //How many treasures collected so far.
    size_t treasures_collected;
    //How many treasures exist in total.
    size_t treasures_total;
    //How many treasure points collected so far.
    size_t treasure_points_collected;
    //How many treasure points exist in total.
    size_t treasure_points_total;
    //How many enemy deaths so far.
    size_t enemy_deaths;
    //How many enemies exist in total.
    size_t enemy_total;
    //How many enemy points collected so far.
    size_t enemy_points_collected;
    //How many enemy points exist in total.
    size_t enemy_points_total;
    //Reason for mission failure, if any. INVALID for none.
    MISSION_FAIL_CONDITIONS mission_fail_reason;
    //Current mission score. Cache for convenience.
    int mission_score;
    //How many leaders are in the mission exit. Cache for convenience.
    size_t cur_leaders_in_mission_exit;
    //How many leaders have been lost so far. Cache for convenience.
    size_t leaders_kod;
    //Starting number of leader mobs.
    size_t starting_nr_of_leaders;
    //Ratio of the mission goal HUD item's indicator.
    float goal_indicator_ratio;
    //Position of the mission score HUD item's indicator.
    float score_indicator;
    //Current interlude, if any.
    INTERLUDES cur_interlude;
    //Time passed in the current interlude.
    float interlude_time;
    //Current big message, if any.
    BIG_MESSAGES cur_big_msg;
    //Time passed in the current big message.
    float big_msg_time;
    
    //Target to leave towards.
    enum LEAVE_TARGET {
        //Leave in order to retry the area.
        LEAVE_TO_RETRY,
        //Leave in order to end the exploration/mission.
        LEAVE_TO_END,
        //Leave in order to go to the area selection.
        LEAVE_TO_AREA_SELECT,
    };
    
    void enter();
    void leave(const LEAVE_TARGET target);
    void start_leaving(const LEAVE_TARGET target);
    void change_spray_count(const size_t type_nr, signed int amount);
    size_t get_total_pikmin_amount();
    void update_available_leaders();
    void update_closest_group_members();
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    void update_transformations() override;
    string get_name() const override;
    
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
    
    void do_aesthetic_leader_logic(const float delta_t);
    void do_aesthetic_logic(const float delta_t);
    void do_game_drawing(
        ALLEGRO_BITMAP* bmp_output = NULL,
        ALLEGRO_TRANSFORM* bmp_transform = NULL
    );
    void do_gameplay_leader_logic(const float delta_t);
    void do_gameplay_logic(const float delta_t);
    void do_menu_logic();
    void draw_background(ALLEGRO_BITMAP* bmp_output);
    void draw_leader_cursor(const ALLEGRO_COLOR &color);
    void draw_ingame_text();
    void draw_big_msg();
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
    void end_mission(const bool cleared);
    ALLEGRO_BITMAP* generate_fog_bitmap(
        const float near_radius, const float far_radius
    );
    mob* get_closest_group_member(subgroup_type* type);
    void handle_button(
        const BUTTONS button, const float pos, const size_t player
    );
    void init_hud();
    bool is_mission_clear_met();
    bool is_mission_fail_met(MISSION_FAIL_CONDITIONS* reason);
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
