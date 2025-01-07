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

#pragma once

#include "../../controls.h"
#include "../../mobs/interactable.h"
#include "../../mobs/onion.h"
#include "../../mobs/pikmin.h"
#include "../../mobs/ship.h"
#include "../../mission.h"
#include "../../replay.h"
#include "../../gui.h"
#include "../../utils/general_utils.h"
#include "../game_state.h"
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
extern const float BOSS_MUSIC_DISTANCE;
extern const string BOSS_SONG_NAME;
extern const string BOSS_VICTORY_SONG_NAME;
extern const float CAMERA_BOX_MARGIN;
extern const float CAMERA_SMOOTHNESS_MULT;
extern const unsigned char COLLISION_OPACITY;
extern const float ENEMY_MIX_DISTANCE;
extern const int FOG_BITMAP_SIZE;
extern const unsigned char PREVIEW_OPACITY;
extern const float PREVIEW_TEXTURE_SCALE;
extern const float PREVIEW_TEXTURE_TIME_MULT;
extern const float REPLAY_SAVE_FREQUENCY;
extern const float TREE_SHADOW_SWAY_AMOUNT;
extern const float TREE_SHADOW_SWAY_SPEED;
extern const float MENU_ENTRY_HUD_MOVE_TIME;
extern const float MENU_EXIT_HUD_MOVE_TIME;
extern const float SWARM_ARROW_SPEED;
}


//Types of interludes -- stuff before or after gameplay proper in the area.
enum INTERLUDE {

    //None.
    INTERLUDE_NONE,
    
    //Ready?
    INTERLUDE_READY,
    
    //Mission end, be it due to a clear or a fail.
    INTERLUDE_MISSION_END,
    
};


//Types of big messages -- text that appears in large letters on-screen.
enum BIG_MESSAGE {

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


//States for the area's boss music.
enum BOSS_MUSIC_STATE {

    //Hasn't played yet.
    BOSS_MUSIC_STATE_NEVER_PLAYED,
    
    //Is playing.
    BOSS_MUSIC_STATE_PLAYING,
    
    //Is not playing right now, but has played before.
    BOSS_MUSIC_STATE_PAUSED,
    
    //Playing the victory theme.
    BOSS_MUSIC_STATE_VICTORY,
};


struct msg_box_t;


/**
 * @brief Standard gameplay state. This is where the action happens.
 */
class gameplay_state : public game_state {

public:

    //--- Members ---
    
    //Is the player playing after hours?
    bool after_hours = false;
    
    //Divides the area into cells, and lists whether a given cell is
    //active for this frame or not.
    vector<vector<bool> > area_active_cells;
    
    //How many seconds since area load. Doesn't count when things are paused.
    float area_time_passed = 0.0f;
    
    //Timer used to fade out the area's title when the area is entered.
    timer area_title_fade_timer = timer(GAMEPLAY::AREA_TITLE_FADE_DURATION);
    
    //Leaders available to control.
    vector<leader*> available_leaders;
    
    //Fog effect buffer.
    ALLEGRO_BITMAP* bmp_fog = nullptr;
    
    //Closest to player 1's leader, for the previous, current, next type.
    mob* closest_group_member[3] = { nullptr, nullptr, nullptr };
    
    //Is the group member closest to player 1's leader distant?
    bool closest_group_member_distant = false;
    
    //Index of player 1's current leader, in the array of available leaders.
    size_t cur_leader_idx = 0;
    
    //Pointer to player 1's leader. Cache for convenience.
    leader* cur_leader_ptr = nullptr;
    
    //What day it is, in-game.
    size_t day = 1;
    
    //What time of the day is it in-game? In minutes.
    float day_minutes = 0.0f;
    
    //Multiply the delta_t by this much. Only affects gameplay stuff, not menus.
    float delta_t_mult = 1.0f;
    
    //Replay of the gameplay.
    replay gameplay_replay;
    
    //How many seconds of actual playtime. Only counts on player control.
    float gameplay_time_passed = 0.0f;
    
    //Information about the in-game HUD.
    hud_t* hud = nullptr;
    
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
    sector* leader_cursor_sector = nullptr;
    
    //Player 1's leader cursor, in world coordinates.
    point leader_cursor_w;
    
    //List of all mobs in the area.
    mob_lists mobs;
    
    //Information about the message box currently active on player 1, if any.
    msg_box_t* msg_box = nullptr;
    
    //ID of the next mob to be created.
    size_t next_mob_id = 0;
    
    //Current notification.
    notification_t notification;
    
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
    size_t selected_spray = 0;
    
    //How many of each spray/ingredients player 1 has.
    vector<spray_stats_t> spray_stats;
    
    //All types of subgroups.
    subgroup_type_manager subgroup_types;
    
    //Angle at which player 1 is swarming.
    float swarm_angle = 0.0f;
    
    //General intensity of player 1's swarm in the specified angle.
    float swarm_magnitude = 0.0f;
    
    //Destination of player 1's throw.
    point throw_dest;
    
    //Mob that player 1's throw will land on, if any.
    mob* throw_dest_mob = nullptr;
    
    //Sector that player 1's throw will land on, if any.
    sector* throw_dest_sector = nullptr;
    
    //Are we currently loading the gameplay state?
    bool loading = false;
    
    //Are we currently unloading the gameplay state?
    bool unloading = false;
    
    //Have we went to the results screen yet?
    bool went_to_results = false;
    
    //Information about player 1's whistle.
    whistle_t whistle;
    
    //IDs of mobs remaining for the current mission goal, if applicable.
    unordered_set<size_t> mission_remaining_mob_ids;
    
    //How many mobs are required for the mission goal. Cache for convenience.
    size_t mission_required_mob_amount = 0;
    
    //How many Pikmin were born so far.
    size_t pikmin_born = 0;
    
    //How many Pikmin deaths so far.
    size_t pikmin_deaths = 0;
    
    //How many treasures collected so far.
    size_t treasures_collected = 0;
    
    //How many treasures exist in total.
    size_t treasures_total = 0;
    
    //How many mission goal treasures were collected so far.
    size_t goal_treasures_collected = 0;
    
    //How many mission goal treasures exist in total.
    size_t goal_treasures_total = 0;
    
    //How many treasure points collected so far.
    size_t treasure_points_collected = 0;
    
    //How many treasure points exist in total.
    size_t treasure_points_total = 0;
    
    //How many enemy deaths so far.
    size_t enemy_deaths = 0;
    
    //How many enemies exist in total.
    size_t enemy_total = 0;
    
    //How many enemy points collected so far.
    size_t enemy_points_collected = 0;
    
    //How many enemy points exist in total.
    size_t enemy_points_total = 0;
    
    //Reason for mission fail, if any. INVALID for none.
    MISSION_FAIL_COND mission_fail_reason = (MISSION_FAIL_COND) INVALID;
    
    //Current mission score, for use in the HUD.
    int mission_score = 0;
    
    //Mission score in the previous frame.
    int old_mission_score = 0;
    
    //GUI item with the mission score's text.
    gui_item* mission_score_cur_text = nullptr;
    
    //Mission goal current count in the previous frame.
    int old_mission_goal_cur = 0;
    
    //GUI item with the mission goal current count's text.
    gui_item* mission_goal_cur_text = nullptr;
    
    //Mission goal primary fail condition count in the previous frame.
    int old_mission_fail_1_cur = 0;
    
    //GUI item with the mission primary fail condition current count's text.
    gui_item* mission_fail_1_cur_text = nullptr;
    
    //Mission goal secondary fail condition count in the previous frame.
    int old_mission_fail_2_cur = 0;
    
    //GUI item with the mission secondary fail condition current count's text.
    gui_item* mission_fail_2_cur_text = nullptr;
    
    //How many leaders are in the mission exit. Cache for convenience.
    size_t cur_leaders_in_mission_exit = 0;
    
    //Current number of living leaders. Cache for convenience.
    size_t nr_living_leaders = 0;
    
    //How many leaders have been lost so far. Cache for convenience.
    size_t leaders_kod = 0;
    
    //Starting number of leader mobs.
    size_t starting_nr_of_leaders = 0;
    
    //Ratio of the mission goal HUD item's indicator.
    float goal_indicator_ratio = 0.0f;
    
    //Ratio of the mission primary fail condition HUD item's indicator.
    float fail_1_indicator_ratio = 0.0f;
    
    //Ratio of the mission secondary fail condition HUD item's indicator.
    float fail_2_indicator_ratio = 0.0f;
    
    //Position of the mission score HUD item's indicator.
    float score_indicator = 0.0f;
    
    //Current interlude, if any.
    INTERLUDE cur_interlude = INTERLUDE_NONE;
    
    //Time passed in the current interlude.
    float interlude_time = 0.0f;
    
    //Current big message, if any.
    BIG_MESSAGE cur_big_msg = BIG_MESSAGE_NONE;
    
    //Time passed in the current big message.
    float big_msg_time = 0.0f;
    
    //Current state of the boss music.
    BOSS_MUSIC_STATE boss_music_state = BOSS_MUSIC_STATE_NEVER_PLAYED;
    
    //Zoom level to use on the radar.
    float radar_zoom = PAUSE_MENU::RADAR_DEF_ZOOM;
    
    //Number of Pikmin born so far, per type.
    map<pikmin_type*, long> pikmin_born_per_type;
    
    //Number of Pikmin lost so far, per type.
    map<pikmin_type*, long> pikmin_deaths_per_type;
    
    
    //--- Function declarations ---
    
    void enter();
    void leave(const GAMEPLAY_LEAVE_TARGET target);
    void start_leaving(const GAMEPLAY_LEAVE_TARGET target);
    void change_spray_count(size_t type_idx, signed int amount);
    size_t get_amount_of_field_pikmin(const pikmin_type* filter = nullptr);
    size_t get_amount_of_group_pikmin(const pikmin_type* filter = nullptr);
    size_t get_amount_of_idle_pikmin(const pikmin_type* filter = nullptr);
    long get_amount_of_onion_pikmin(const pikmin_type* filter = nullptr);
    long get_amount_of_total_pikmin(const pikmin_type* filter = nullptr);
    void is_near_enemy_and_boss(bool* near_enemy, bool* near_boss);
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

    //--- Members ---
    
    //Points to an interactable close enough for player 1 to use, if any.
    interactable* close_to_interactable_to_use = nullptr;
    
    //Points to a nest-like object close enough for player 1 to open, if any.
    pikmin_nest_t* close_to_nest_to_open = nullptr;
    
    //Points to a Pikmin close enough for player 1 to pluck, if any.
    pikmin* close_to_pikmin_to_pluck = nullptr;
    
    //Points to a ship close enough for player 1 to heal in, if any.
    ship* close_to_ship_to_heal = nullptr;
    
    //Ligthten player 1's cursor by this due to leader/cursor height difference.
    float cursor_height_diff_light = 0.0f;
    
    //Movement of player 1's cursor via non-mouse means.
    movement_t cursor_movement;
    
    //Is input enabled, for reasons outside the ready_for_input variable?
    bool is_input_allowed = false;
    
    //Bitmap that lights up the area when in blackout mode.
    ALLEGRO_BITMAP* lightmap_bmp = nullptr;
    
    //Movement of player 1's leader.
    movement_t leader_movement;
    
    //Information about the current Onion menu, if any.
    onion_menu_t* onion_menu = nullptr;
    
    //Information about the current pause menu, if any.
    pause_menu_t* pause_menu = nullptr;
    
    //Is the gameplay paused?
    bool paused = false;
    
    //The first frame shouldn't allow for input just yet, because
    //some things are still being set up within the first logic loop.
    //So forbid input until the second frame.
    bool ready_for_input = false;
    
    //Timer for the next replay state save.
    timer replay_timer;
    
    //Is player 1 holding the "swarm to cursor" button?
    bool swarm_cursor = false;
    
    //Reach of player 1's swarm.
    movement_t swarm_movement;
    
    
    //--- Function declarations ---
    
    void do_aesthetic_leader_logic(float delta_t);
    void do_aesthetic_logic(float delta_t);
    void do_game_drawing(
        ALLEGRO_BITMAP* bmp_output = nullptr,
        const ALLEGRO_TRANSFORM* bmp_transform = nullptr
    );
    void do_gameplay_leader_logic(float delta_t);
    void do_gameplay_logic(float delta_t);
    void do_menu_logic();
    void draw_background(ALLEGRO_BITMAP* bmp_output);
    void draw_debug_tools();
    void draw_leader_cursor(const ALLEGRO_COLOR &color);
    void draw_ingame_text();
    void draw_big_msg();
    void draw_lighting_filter();
    void draw_message_box();
    void draw_onion_menu();
    void draw_pause_menu();
    void draw_precipitation();
    void draw_system_stuff();
    void draw_throw_preview();
    void draw_tree_shadows();
    void draw_world_components(ALLEGRO_BITMAP* bmp_output);
    ALLEGRO_BITMAP* draw_to_bitmap();
    void end_mission(bool cleared);
    ALLEGRO_BITMAP* generate_fog_bitmap(
        float near_radius, float far_radius
    );
    mob* get_closest_group_member(const subgroup_type* type, bool* distant = nullptr);
    void handle_player_action(const player_action &action);
    void init_hud();
    bool is_mission_clear_met();
    bool is_mission_fail_met(MISSION_FAIL_COND* reason);
    void load_game_content();
    void mark_area_cells_active(
        const point &top_left, const point &bottom_right
    );
    void mark_area_cells_active(
        int from_x, int to_x, int from_y, int to_y
    );
    void process_mob_interactions(mob* m_ptr, size_t m);
    void process_mob_misc_interactions(
        mob* m_ptr, mob* m2_ptr, size_t m, size_t m2, const dist &d,
        vector<pending_intermob_event> &pending_intermob_events
    );
    void process_mob_reaches(
        mob* m_ptr, mob* m2_ptr, size_t m, size_t m2, const dist &d,
        vector<pending_intermob_event> &pending_intermob_events
    );
    void process_mob_touches(
        mob* m_ptr, mob* m2_ptr, size_t m, size_t m2, dist &d
    );
    void process_system_key_press(int keycode);
    bool should_ignore_player_action(const player_action &action);
    void unload_game_content();
    void update_area_active_cells();
    void update_mob_is_active_flag();
    
};


/**
 * @brief Info about the current on-screen message box, if any.
 */
struct msg_box_t {

    //--- Members ---
    
    //Full list of message tokens, per line.
    vector<vector<string_token> > tokens_per_line;
    
    //Icon that represents the speaker, if any.
    ALLEGRO_BITMAP* speaker_icon = nullptr;
    
    //What section of the message are we in?
    size_t cur_section = 0;
    
    //What token of the section are we in, for the typing animation?
    size_t cur_token = 0;
    
    //From what token did the player perform a typing skip? INVALID = none.
    size_t skipped_at_token = INVALID;
    
    //How long have we been animating tokens for.
    float total_token_anim_time = 0.0f;
    
    //How long have we been animating the skipped tokens for.
    float total_skip_anim_time = 0.0f;
    
    //Timer where the player can't advance. Stops accidental misinputs.
    float misinput_protection_timer = 0.0f;
    
    //Opacity of the advance button icon.
    float advance_button_alpha = 0.0f;
    
    //Time left to swipe the current section away with.
    float swipe_timer = 0.0f;
    
    //Time left in the current transition.
    float transition_timer = GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME;
    
    //Is it transitioning into view, or out of view?
    bool transition_in = true;
    
    //Is the message box meant to be deleted?
    bool to_delete = false;
    
    
    //--- Function declarations ---
    
    msg_box_t(const string &text, ALLEGRO_BITMAP* speaker_icon);
    void advance();
    void close();
    void tick(float delta_t);
    
};
