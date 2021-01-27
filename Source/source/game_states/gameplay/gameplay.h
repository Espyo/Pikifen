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

#include "../../mobs/interactable.h"
#include "../../mobs/onion.h"
#include "../../mobs/pikmin.h"
#include "../../mobs/ship.h"
#include "../../replay.h"
#include "../game_state.h"
#include "../../gui.h"


/* ----------------------------------------------------------------------------
 * Standard gameplay state. This is where the action happens.
 */
class gameplay_state : public game_state {
public:

    gameplay_state();
    
    //Is the player playing after hours?
    bool after_hours;
    //How much real time has passed since the area was loaded.
    float area_time_passed;
    //Timer used to fade out the area's title when the area is entered.
    timer area_title_fade_timer;
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
    //Replay of the current day.
    replay day_replay;
    //Manager for the gameplay HUD.
    gui_manager hud;
    //Mob that player 1's leader cursor is on top of, if any.
    mob* leader_cursor_mob;
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
    //How many of each spray/ingredients player 1 has.
    vector<spray_stats_struct> spray_stats;
    //All types of subgroups.
    subgroup_type_manager subgroup_types;
    //Angle at which player 1 is swarming.
    float swarm_angle;
    //General intensity of player 1's swarm in the specified angle.
    float swarm_magnitude;
    //Have we went to the results screen yet?
    bool went_to_results;
    //Information about player 1's whistle.
    whistle_struct whistle;
    
    enum LEAVE_TARGET {
        LEAVE_TO_RETRY,
        LEAVE_TO_FINISH,
        LEAVE_TO_AREA_SELECT,
    };
    
    void enter();
    void leave(const LEAVE_TARGET target);
    void update_closest_group_member();
    
    virtual void load();
    virtual void unload();
    virtual void handle_allegro_event(ALLEGRO_EVENT &ev);
    virtual void do_logic();
    virtual void do_drawing();
    virtual void update_transformations();
    virtual string get_name() const;
    
private:

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
    
    //Contains information about a given Pikmin type in an Onion menu.
    struct onion_menu_type_struct {
        //The player wants to add/subtract these many from the group.
        int delta;
        //Index of this type in the Onion's list. Cache for convenience.
        size_t type_idx;
        //Index in the on-screen list, or INVALID. Cache for convenience.
        size_t on_screen_idx;
        //Pikmin type associated. Cache for convenience.
        pikmin_type* pik_type;
        
        onion_menu_type_struct(const size_t idx, pikmin_type* pik_type);
    };
    
    //Contains information about the Onion menu currently being presented to
    //the player.
    struct onion_menu_struct {
    public:
        //Pointer to the struct with nest information.
        pikmin_nest_struct* n_ptr;
        //Pointer to the leader responsible.
        leader* l_ptr;
        //Information on every type's management.
        vector<onion_menu_type_struct> types;
        //GUI manager.
        gui_manager gui;
        //Is "select all" currently on?
        bool select_all;
        //If it manages more than 5, this is the Pikmin type page index.
        size_t page;
        //Which GUI items are in red right now, if any, and how much time left.
        map<gui_item*, float> red_items;
        //Total page amount. Cache for convenience.
        size_t nr_pages;
        //Pikmin types currently on-screen. Cache for convenience.
        vector<onion_menu_type_struct*> on_screen_types;
        //List of GUI items for the Onion icons. Cache for convenience.
        vector<gui_item*> onion_icon_items;
        //List of GUI items for the Onion buttons. Cache for convenience.
        vector<gui_item*> onion_button_items;
        //List of GUI items for the Onion amounts. Cache for convenience.
        vector<gui_item*> onion_amount_items;
        //List of GUI items for the group icons. Cache for convenience.
        vector<gui_item*> group_icon_items;
        //List of GUI items for the group buttons. Cache for convenience.
        vector<gui_item*> group_button_items;
        //List of GUI items for the group amounts. Cache for convenience.
        vector<gui_item*> group_amount_items;
        //The button that controls all Onions. Cache for convenience.
        gui_item* onion_all_button;
        //The button that controls all groups. Cache for convenience.
        gui_item* group_all_button;
        //Left Onion "more..." icon. Cache for convenience.
        gui_item* onion_more_l_icon;
        //Right Onion "more..." icon. Cache for convenience.
        gui_item* onion_more_r_icon;
        //Left group "more..." icon. Cache for convenience.
        gui_item* group_more_l_icon;
        //Right group "more..." icon. Cache for convenience.
        gui_item* group_more_r_icon;
        //Previous page button. Cache for convenience.
        gui_item* prev_page_button;
        //Next page button. Cache for convenience.
        gui_item* next_page_button;
        //Field amount text. Cache for convenience.
        gui_item* field_amount_text;
        //Is the struct meant to be deleted?
        bool to_delete;
        
        onion_menu_struct(pikmin_nest_struct* n_ptr, leader* l_ptr);
        ~onion_menu_struct();
        void add_all_to_group();
        void add_all_to_onion();
        void add_to_group(const size_t type_idx);
        void add_to_onion(const size_t type_idx);
        void confirm();
        void go_to_page(const size_t page);
        void tick(const float delta_t);
        void toggle_select_all();
        
        static const float RED_TEXT_DURATION;
        
    private:
        void make_gui_item_red(gui_item* item);
        void update();
        
        static const string GUI_FILE_PATH;
    };
    
    //Contains information about the pause menu currently being presented to
    //the player.
    struct pause_menu_struct {
    public:
        //GUI manager.
        gui_manager gui;
        //Is the struct meant to be deleted?
        bool to_delete;
        
        pause_menu_struct();
        ~pause_menu_struct();
        void tick(const float delta_t);
        
    private:
        static const string GUI_FILE_PATH;
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
    //What day it is, in-game.
    size_t day;
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
    //Spray that player 1 has currently selected.
    size_t selected_spray;
    //Player 1's swarm mode arrows.
    vector<float> swarm_arrows;
    //Time until the next arrow in player 1's list of swarm arrows appears.
    timer swarm_next_arrow_timer;
    //Is player 1 holding the "swarm to cursor" button?
    bool swarm_cursor;
    //Reach of player 1's swarm.
    movement_struct swarm_movement;
    //Is it possible for the currently held Pikmin to reach the cursor?
    bool throw_can_reach_cursor;
    
    void do_aesthetic_logic();
    void do_game_drawing(
        ALLEGRO_BITMAP* bmp_output = NULL,
        ALLEGRO_TRANSFORM* bmp_transform = NULL
    );
    void do_gameplay_logic();
    void do_menu_logic();
    void draw_background(ALLEGRO_BITMAP* bmp_output);
    void draw_leader_cursor(
        ALLEGRO_TRANSFORM &world_to_screen_drawing_transform
    );
    void draw_mouse_cursor(const ALLEGRO_COLOR &color);
    void draw_ingame_text();
    void draw_lighting_filter();
    void draw_message_box();
    void draw_onion_menu();
    void draw_pause_menu();
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
    
    static const float AREA_INTRO_HUD_MOVE_TIME;
    static const float AREA_TITLE_FADE_DURATION;
    static const float CURSOR_INVALID_EFFECT_SPEED;
    static const float CURSOR_SAVE_INTERVAL;
    static const string HUD_FILE_NAME;
    static const size_t ONION_MENU_TYPES_PER_PAGE;
    static const float SWARM_ARROW_SPEED;
    static const float SWARM_ARROWS_INTERVAL;
};


#endif //ifndef GAMEPLAY_INCLUDED
