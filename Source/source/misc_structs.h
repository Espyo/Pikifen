/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the miscellaneous structures,
 * too simple to warrant their own files.
 */

#pragma once

#include <functional>
#include <map>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>

#include "animation.h"
#include "controls.h"
#include "mob_categories/mob_category.h"
#include "mobs/mob_enums.h"
#include "hazard.h"
#include "particle.h"
#include "libs/data_file.h"
#include "utils/geometry_utils.h"
#include "utils/math_utils.h"


class pikmin_type;

using std::map;
using std::size_t;
using std::string;
using std::vector;

namespace MSG_BOX {
extern const float ADVANCE_BUTTON_FADE_SPEED;
extern const float MARGIN;
extern const float PADDING;
extern const float MISINPUT_PROTECTION_DURATION;
extern const float TOKEN_ANIM_DURATION;
extern const float TOKEN_ANIM_X_AMOUNT;
extern const float TOKEN_ANIM_Y_AMOUNT;
extern const float TOKEN_SWIPE_DURATION;
extern const float TOKEN_SWIPE_X_AMOUNT;
extern const float TOKEN_SWIPE_Y_AMOUNT;
}


namespace NOTIFICATION {
extern const float FADE_SPEED;
}


namespace WHISTLE {
constexpr unsigned char N_RING_COLORS = 8;
constexpr unsigned char N_DOT_COLORS = 6;
extern const unsigned char DOT_COLORS[N_DOT_COLORS][3];
extern const float DOT_INTERVAL;
extern const float DOT_SPIN_SPEED;
extern const float FADE_TIME;
extern const unsigned char RING_COLORS[N_RING_COLORS][3];
extern const float RING_SPEED;
extern const float RINGS_INTERVAL;
}


//List of maker tools.
enum MAKER_TOOL_TYPE {

    //None.
    MAKER_TOOL_TYPE_NONE,
    
    //Create an image of the whole area.
    MAKER_TOOL_TYPE_AREA_IMAGE,
    
    //Change gameplay speed.
    MAKER_TOOL_TYPE_CHANGE_SPEED,
    
    //Show collision box.
    MAKER_TOOL_TYPE_COLLISION,
    
    //Geometry info beneath mouse cursor.
    MAKER_TOOL_TYPE_GEOMETRY_INFO,
    
    //Show hitboxes.
    MAKER_TOOL_TYPE_HITBOXES,
    
    //Toggle HUD visibility.
    MAKER_TOOL_TYPE_HUD,
    
    //Hurt mob beneath mouse cursor.
    MAKER_TOOL_TYPE_HURT_MOB,
    
    //Get info on the mob beneath mouse cursor.
    MAKER_TOOL_TYPE_MOB_INFO,
    
    //Create a new Pikmin beneath mouse cursor.
    MAKER_TOOL_TYPE_NEW_PIKMIN,
    
    //Show path info.
    MAKER_TOOL_TYPE_PATH_INFO,
    
    //Teleport to mouse cursor.
    MAKER_TOOL_TYPE_TELEPORT,
    
    //Total amount of maker tools.
    N_MAKER_TOOLS,
    
};


namespace MAKER_TOOLS {
extern const string NAMES[N_MAKER_TOOLS];
}


//Types of string token.
enum STRING_TOKEN {

    //None.
    STRING_TOKEN_NONE,
    
    //A regular character.
    STRING_TOKEN_CHAR,
    
    //A line break.
    STRING_TOKEN_LINE_BREAK,
    
    //A control bind icon.
    STRING_TOKEN_CONTROL_BIND,
    
};


/**
 * @brief List of file names of system assets.
 */
struct asset_file_names_t {

    //--- Members ---
    
    //Graphics.
    
    //Area name font.
    string bmp_area_name_font = "Area_name_font.png";
    
    //Bright circle.
    string bmp_bright_circle = "Bright_circle.png";
    
    //Bright ring.
    string bmp_bright_ring = "Bright_ring.png";
    
    //Bubble box.
    string bmp_bubble_box = "Bubble_box.png";
    
    //Texture box for player input buttons.
    string bmp_button_box = "Button_box.png";
    
    //Checkbox with a checkmark.
    string bmp_checkbox_check = "Checkbox_check.png";
    
    //Checkbox without a checkmark.
    string bmp_checkbox_no_check = "Checkbox_no_check.png";
    
    //Counter font.
    string bmp_counter_font = "Counter_font.png";
    
    //Leader cursor.
    string bmp_cursor = "Cursor.png";
    
    //Cursor counter font.
    string bmp_cursor_counter_font = "Cursor_counter_font.png";
    
    //Editor icons.
    string bmp_editor_icons = "Editor_icons.png";
    
    //Enemy spirit.
    string bmp_enemy_spirit = "Enemy_spirit.png";
    
    //GUI focus box.
    string bmp_focus_box = "Focus_box.png";
    
    //Pikifen icon.
    string bmp_icon = "Icon.png";
    
    //Idle glow.
    string bmp_idle_glow = "Idle_glow.png";
    
    //Texture box for player input keys.
    string bmp_key_box = "Key_box.png";
    
    //Leader silhouette from the side.
    string bmp_leader_silhouette_side = "Leader_silhouette_side.png";
    
    //Leader silhouette from the top.
    string bmp_leader_silhouette_top = "Leader_silhouette_top.png";
    
    //Main font.
    string bmp_main_font = "Font.png";
    
    //Main menu background.
    string bmp_main_menu = "Main_menu.jpg";
    
    //Bronze medal.
    string bmp_medal_bronze = "Medal_bronze.png";
    
    //Gold medal.
    string bmp_medal_gold = "Medal_gold.png";
    
    //No medal.
    string bmp_medal_none = "Medal_none.png";
    
    //Platinum medal.
    string bmp_medal_platinum = "Medal_platinum.png";
    
    //Silver medal.
    string bmp_medal_silver = "Medal_silver.png";
    
    //Icons for menu buttons.
    string bmp_menu_icons = "Menu_icons.png";
    
    //Mission clear stamp.
    string bmp_mission_clear = "Mission_clear.png";
    
    //Mission fail stamp.
    string bmp_mission_fail = "Mission_fail.png";
    
    //"More..." icon.
    string bmp_more = "More.png";
    
    //Mouse cursor.
    string bmp_mouse_cursor = "Mouse_cursor.png";
    
    //Notification.
    string bmp_notification = "Notification.png";
    
    //Pikmin spirit.
    string bmp_pikmin_spirit = "Pikmin_spirit.png";
    
    //Misc. specific player input icons.
    string bmp_player_input_icons = "Player_input_icons.png";
    
    //Randomness symbol.
    string bmp_random = "Random.png";
    
    //A rock.
    string bmp_rock = "Rock.png";
    
    //Mob shadow.
    string bmp_shadow = "Shadow.png";
    
    //Smack effect.
    string bmp_smack = "Smack.png";
    
    //Smoke.
    string bmp_smoke = "Smoke.png";
    
    //Sparkle effect.
    string bmp_sparkle = "Sparkle.png";
    
    //Spotlight for blackout.
    string bmp_spotlight = "Spotlight.png";
    
    //Swarm arrow.
    string bmp_swarm_arrow = "Swarm_arrow.png";
    
    //Invalid throw marker.
    string bmp_throw_invalid = "Throw_invalid.png";
    
    //Throw preview texture.
    string bmp_throw_preview = "Throw_preview.png";
    
    //Dashed throw preview texture.
    string bmp_throw_preview_dashed = "Throw_preview_dashed.png";
    
    //Value font.
    string bmp_value_font = "Value_font.png";
    
    //Wave ring.
    string bmp_wave_ring = "Wave_ring.png";
    
    //Fonts.
    
    //Font for area names.
    string fnt_area_name = "Area_name_font.png";
    
    //Font for HUD counters.
    string fnt_counter = "Counter_font.png";
    
    //Font displayed next to the cursor.
    string fnt_cursor_counter = "Cursor_counter_font.png";
    
    //Font for slim text.
    string fnt_slim = "Slim_font.otf";
    
    //Font for standard text.
    string fnt_standard = "Font.png";
    
    //Font for the carrying / money values.
    string fnt_value = "Value_font.png";
    
    //Sound effects.
    
    //Attack.
    string sfx_attack = "Attack.ogg";
    
    //Camera zoom level.
    string sfx_camera = "Camera.ogg";
    
    //Menu item activation.
    string sfx_menu_activate = "Menu_activate.ogg";
    
    //Menu item back.
    string sfx_menu_back = "Menu_back.ogg";
    
    //Menu item selection.
    string sfx_menu_select = "Menu_select.ogg";
    
    //Obstacle clear jingle.
    string sfx_obstacle_cleared = "Obstacle_cleared.ogg";
    
    //Pluck sound effect.
    string sfx_pluck = "Pluck.ogg";
    
    //Using a spray.
    string sfx_spray = "Spray.ogg";
    
    //Switching standby Pikmin type.
    string sfx_switch_pikmin = "Switch_Pikmin.ogg";
    
    //Throwing.
    string sfx_throw = "Throw.ogg";
    
    
    //--- Function declarations ---
    
    void load(data_node* file);
    
};


/**
 * @brief Info about the game camera. Where it is, where it wants
 * to go, etc.
 */
struct camera_t {

    //--- Members ---
    
    //Top-left and bottom-right world coordinates that this camera can see.
    //These also include a margin of GAMEPLAY::CAMERA_BOX_MARGIN.
    point box[2];
    
    //Current position.
    point pos;
    
    //Position it wants to be at.
    point target_pos;
    
    //Zoom it wants to be at.
    float target_zoom = 1.0f;
    
    //Current zoom.
    float zoom = 1.0f;
    
    
    //--- Function declarations ---
    
    void set_pos(const point &new_pos);
    void set_zoom(const float new_zoom);
    void tick(const float delta_t);
    void update_box();
    
};


/**
 * @brief Manages any errors that occur with the engine's content or logic.
 */
struct error_manager {

    //--- Function declarations ---
    
    void report(const string &s, const data_node* d = nullptr);
    void report_area_load_errors();
    void prepare_area_load();
    bool session_has_errors();
    
    private:
    
    //--- Members ---
    
    //How many errors have been reported this application session.
    size_t nr_session_errors = 0;
    
    //Errors reported by the time the area load started.
    size_t nr_errors_on_area_load = 0;
    
    //First error reported during area load.
    string first_area_load_error;
    
    
    //--- Function declarations ---
    
    void emit_in_gameplay(const string &s);
    void log_to_console(const string &s);
    void log_to_file(const string &s);
    
};


/**
 * @brief Just a list of different elements in an enum and what their names are.
 */
struct enum_name_database {

    public:
    
    //--- Function declarations ---
    
    void register_item(const size_t enum_idx, const string &name);
    size_t get_idx(const string &name) const;
    string get_name(const size_t idx) const;
    size_t get_nr_of_items() const;
    void clear();
    
    private:
    
    //--- Members ---
    
    //Known items.
    vector<string> names;
    
};


/**
 * @brief Info about a token in a string.
 */
struct string_token {

    //--- Members ---
    
    //Type of token.
    STRING_TOKEN type = STRING_TOKEN_CHAR;
    
    //Its content.
    string content;
    
    //Width that it takes up, in pixels.
    int width = 0;
    
};


/**
 * @brief A timer. You can set it to start at a pre-determined time,
 * to tick, etc.
 */
struct timer {

    //--- Members ---
    
    //How much time is left until 0.
    float time_left = 0.0f;
    
    //When the timer starts, its time is set to this.
    float duration = 0.0f;
    
    //Code to run when the timer ends, if any.
    std::function<void()> on_end = nullptr;
    
    
    //--- Function declarations ---
    
    explicit timer(
        const float duration = 0,
        const std::function<void()> &on_end = nullptr
    );
    ~timer();
    void start(const bool can_restart = true);
    void start(const float new_duration);
    void stop();
    void tick(const float delta_t);
    float get_ratio_left() const;
    
};



/**
 * @brief Info about all of the maker tools.
 */
struct maker_tools_t {

    //--- Members ---
    
    //Are the tools enabled?
    bool enabled = true;
    
    //Padding around the area in the area image tool.
    float area_image_padding = 32.0f;
    
    //Show tree shadows in the area image tool?
    bool area_image_shadows = true;
    
    //Maximum width or height of the area image.
    int area_image_size = 2048;
    
    //Show mobs in the area image?
    bool area_image_mobs = true;
    
    //Automatically pick this from the list of the selected auto-entry mode.
    string auto_start_option;
    
    //Automatically enter this game mode when the game boots.
    string auto_start_mode;
    
    //Are we currently changing the game speed?
    bool change_speed = false;
    
    //Multiplier to change the game speed by.
    float change_speed_mult = 2.0f;
    
    //Are collision boxes visible in-game?
    bool collision = false;
    
    //Is the geometry information tool enabled?
    bool geometry_info = false;
    
    //Are hitboxes visible in-game?
    bool hitboxes = false;
    
    //Is the HUD visible?
    bool hud = true;
    
    //Mob currently locked-on to for the mob information tool. nullptr if off.
    mob* info_lock = nullptr;
    
    //If any maker info is being printed, this is how long it lasts on-screen.
    float info_print_duration = 5.0f;
    
    //If any maker info is being printed, this is how long its fade lasts.
    float info_print_fade_duration = 3.0f;
    
    //If any maker info is being printed, this is its text.
    string info_print_text;
    
    //If any maker info is being printed, this represents its time to live.
    timer info_print_timer;
    
    //For each key (F2 - F11, 0 - 9), what tool is bound to it?
    MAKER_TOOL_TYPE keys[20];
    
    //When we last spawned a Pikmin, what was its type?
    pikmin_type* last_pikmin_type = nullptr;
    
    //When hurting mobs with the hurting tool, dock this much of its max HP off.
    float mob_hurting_ratio = 0.75f;
    
    //Show path info?
    bool path_info = false;
    
    //Use the performance monitor?
    bool use_perf_mon = false;
    
    //Has the player made use of any tools that could help them play?
    bool used_helping_tools = false;
    
    
    //--- Function declarations ---
    
    maker_tools_t();
    void reset_for_gameplay();
    
};


/**
 * @brief Info about the operative system's mouse cursor.
 */
struct mouse_cursor_t {

    //--- Members ---
    
    //Position, in screen coordinates.
    point s_pos;
    
    //Position, in world coordinates, if applicable.
    point w_pos;
    
    //Spots the cursor has been through. Used for the faint trail left behind.
    vector<point> history;
    
    //Time left until the position of the cursor is saved on the vector.
    timer save_timer;
    
    
    //--- Function declarations ---
    
    void hide() const;
    void init();
    void reset();
    void show() const;
    void update_pos(
        const ALLEGRO_EVENT &ev,
        ALLEGRO_TRANSFORM &screen_to_world_transform
    );
    
};



/**
 * @brief This structure makes reading values in data files
 * and setting them to variables much easier.
 * On the set functions, specify the name of the child and the variable.
 * If the child is empty, the variable will not be set.
 */
struct reader_setter {

    //--- Members ---
    
    //Node that this reader-setter pertains to.
    data_node* node = nullptr;
    
    
    //--- Function declarations ---
    
    explicit reader_setter(data_node* dn = nullptr);
    void set(
        const string &child, string &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, size_t &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, int &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, unsigned int &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, unsigned char &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, bool &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, float &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, double &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, ALLEGRO_COLOR &var, data_node** child_node = nullptr
    );
    void set(
        const string &child, point &var, data_node** child_node = nullptr
    );
    
};



/**
 * @brief Makes it easy to read script variables, and make changes
 * based on which
 * ones exist, and what values they have.
 */
struct script_var_reader {

    //--- Members ---
    
    //Reference to the list of script variables it pertains to.
    map<string, string> &vars;
    
    
    //--- Function declarations ---
    
    explicit script_var_reader(map<string, string> &vars);
    bool get(const string &name, string &dest) const;
    bool get(const string &name, size_t &dest) const;
    bool get(const string &name, int &dest) const;
    bool get(const string &name, unsigned char &dest) const;
    bool get(const string &name, bool &dest) const;
    bool get(const string &name, float &dest) const;
    bool get(const string &name, ALLEGRO_COLOR &dest) const;
    bool get(const string &name, point &dest) const;
    
};



/**
 * @brief List of loaded system assets.
 */
struct system_asset_list {

    //--- Members ---
    
    //Graphics.
    
    //Bright circle.
    ALLEGRO_BITMAP* bmp_bright_circle = nullptr;
    
    //Bright ring.
    ALLEGRO_BITMAP* bmp_bright_ring = nullptr;
    
    //Bubble box.
    ALLEGRO_BITMAP* bmp_bubble_box = nullptr;
    
    //Box texture for player input buttons.
    ALLEGRO_BITMAP* bmp_button_box = nullptr;
    
    //Checkbox with a checkmark.
    ALLEGRO_BITMAP* bmp_checkbox_check = nullptr;
    
    //Checkbox without a checkmark.
    ALLEGRO_BITMAP* bmp_checkbox_no_check = nullptr;
    
    //Leader cursor.
    ALLEGRO_BITMAP* bmp_cursor = nullptr;
    
    //Enemy spirit.
    ALLEGRO_BITMAP* bmp_enemy_spirit = nullptr;
    
    //Focus box.
    ALLEGRO_BITMAP* bmp_focus_box = nullptr;
    
    //Pikifen icon.
    ALLEGRO_BITMAP* bmp_icon = nullptr;
    
    //Idle glow.
    ALLEGRO_BITMAP* bmp_idle_glow = nullptr;
    
    //Box texture for player input keys.
    ALLEGRO_BITMAP* bmp_key_box = nullptr;
    
    //Leader silhouette from the side.
    ALLEGRO_BITMAP* bmp_leader_silhouette_side = nullptr;
    
    //Leader silhouette from the top.
    ALLEGRO_BITMAP* bmp_leader_silhouette_top = nullptr;
    
    //Bronze mission medal.
    ALLEGRO_BITMAP* bmp_medal_bronze = nullptr;
    
    //Gold mission medal.
    ALLEGRO_BITMAP* bmp_medal_gold = nullptr;
    
    //No mission medal.
    ALLEGRO_BITMAP* bmp_medal_none = nullptr;
    
    //Platinum mission medal.
    ALLEGRO_BITMAP* bmp_medal_platinum = nullptr;
    
    //Silver mission medal.
    ALLEGRO_BITMAP* bmp_medal_silver = nullptr;
    
    //Icons for menu buttons.
    ALLEGRO_BITMAP* bmp_menu_icons = nullptr;
    
    //Mission clear stamp.
    ALLEGRO_BITMAP* bmp_mission_clear = nullptr;
    
    //Mission fail stamp.
    ALLEGRO_BITMAP* bmp_mission_fail = nullptr;
    
    //"More..." icon.
    ALLEGRO_BITMAP* bmp_more = nullptr;
    
    //Mouse cursor.
    ALLEGRO_BITMAP* bmp_mouse_cursor = nullptr;
    
    //Notification.
    ALLEGRO_BITMAP* bmp_notification = nullptr;
    
    //Pikmin spirit.
    ALLEGRO_BITMAP* bmp_pikmin_spirit = nullptr;
    
    //Misc. specific player input icons.
    ALLEGRO_BITMAP* bmp_player_input_icons = nullptr;
    
    //Randomness symbol.
    ALLEGRO_BITMAP* bmp_random = nullptr;
    
    //A rock.
    ALLEGRO_BITMAP* bmp_rock = nullptr;
    
    //Mob shadow.
    ALLEGRO_BITMAP* bmp_shadow = nullptr;
    
    //Smack effect.
    ALLEGRO_BITMAP* bmp_smack = nullptr;
    
    //Smoke.
    ALLEGRO_BITMAP* bmp_smoke = nullptr;
    
    //Sparkle effect.
    ALLEGRO_BITMAP* bmp_sparkle = nullptr;
    
    //Spotlight for blackout.
    ALLEGRO_BITMAP* bmp_spotlight = nullptr;
    
    //Swarm arrow.
    ALLEGRO_BITMAP* bmp_swarm_arrow = nullptr;
    
    //Invalid throw marker.
    ALLEGRO_BITMAP* bmp_throw_invalid = nullptr;
    
    //Throw preview texture.
    ALLEGRO_BITMAP* bmp_throw_preview = nullptr;
    
    //Dashed throw preview texture.
    ALLEGRO_BITMAP* bmp_throw_preview_dashed = nullptr;
    
    //Wave ring.
    ALLEGRO_BITMAP* bmp_wave_ring = nullptr;
    
    //Fonts.
    
    //Font for area names.
    ALLEGRO_FONT* fnt_area_name = nullptr;
    
    //Allegro's built-in font.
    ALLEGRO_FONT* fnt_builtin = nullptr;
    
    //Font for HUD counters.
    ALLEGRO_FONT* fnt_counter = nullptr;
    
    //Counter displayed next to the cursor.
    ALLEGRO_FONT* fnt_cursor_counter = nullptr;
    
    //Font for slim text.
    ALLEGRO_FONT* fnt_slim = nullptr;
    
    //Font for standard text.
    ALLEGRO_FONT* fnt_standard = nullptr;
    
    //Font for the carrying / money values.
    ALLEGRO_FONT* fnt_value = nullptr;
    
    //Sound effects.
    
    //Attack.
    ALLEGRO_SAMPLE* sfx_attack = nullptr;
    
    //Camera zoom level.
    ALLEGRO_SAMPLE* sfx_camera = nullptr;
    
    //Menu item activation.
    ALLEGRO_SAMPLE* sfx_menu_activate = nullptr;
    
    //Menu item back.
    ALLEGRO_SAMPLE* sfx_menu_back = nullptr;
    
    //Menu item selection.
    ALLEGRO_SAMPLE* sfx_menu_select = nullptr;
    
    //Obstacle clear jingle.
    ALLEGRO_SAMPLE* sfx_obstacle_cleared = nullptr;
    
    //Pluck sound effect.
    ALLEGRO_SAMPLE* sfx_pluck = nullptr;
    
    //Using a spray.
    ALLEGRO_SAMPLE* sfx_spray = nullptr;
    
    //Switching standby Pikmin type.
    ALLEGRO_SAMPLE* sfx_switch_pikmin = nullptr;
    
    //Throwing.
    ALLEGRO_SAMPLE* sfx_throw = nullptr;
    
    //Animations.
    
    //Leader damage spark.
    single_animation_suite spark_animation;
    
};


/**
 * @brief Manages fade ins/outs for transitions.
 */
struct fade_manager {

    public:
    
    //--- Function declarations ---
    
    void start_fade(const bool fade_in, const std::function<void()> &on_end);
    bool is_fade_in() const;
    bool is_fading() const;
    float get_perc_left() const;
    void tick(const float delta_t);
    void draw();
    
    private:
    
    //--- Members ---
    
    //Time left in the current fade in/out.
    float time_left = 0.0f;
    
    //True if fading in, false if fading out.
    bool fade_in = false;
    
    //Code to run when the fade in/out finishes.
    std::function<void()> on_end = nullptr;
    
};



/**
 * @brief Info about the current on-screen notification during gameplay.
 * This is stuff like a note above the leader telling the player
 * what button to press to do something, like plucking.
 */
struct notification_t {

    public:
    
    //--- Function declarations ---
    
    void draw() const;
    float get_visibility() const;
    void reset();
    void set_contents(
        const player_input &input, const string &text, const point &pos
    );
    void set_enabled(const bool enabled);
    void tick(const float delta_t);
    
    private:
    
    //--- Members ---
    
    //Is it meant to exist?
    bool enabled = true;
    
    //What player input icon to show.
    player_input input;
    
    //What text to write.
    string text;
    
    //Coordinates of the focal point.
    point pos;
    
    //Visibility. 0 is hidden, 1 is fully visible. Mid values for transitioning.
    float visibility = 0.0f;
    
};



/**
 * @brief Info on how a bitmap should be drawn, in regards to
 * translation, rotation, coloring, etc.
 */
struct bitmap_effect_t {

    //--- Members ---
    
    //Offset horizontally and vertically by this much.
    point translation;
    
    //Rotate the bitmap by this angle, in radians.
    float rotation = 0.0f;
    
    //Scale horizontally and vertically. LARGE_FLOAT = use the other's scale.
    point scale = point(1.0f, 1.0f);
    
    //Tint the bitmap by this color. Also makes it transparent.
    ALLEGRO_COLOR tint_color = COLOR_WHITE;
    
    //Re-draws the bitmap on top, in additive blend, with this color.
    ALLEGRO_COLOR glow_color = COLOR_BLACK;
    
};


//Performance monitor states.
enum PERF_MON_STATE {

    //Measuring loading times.
    PERF_MON_STATE_LOADING,
    
    //Measuring gameplay frame performance.
    PERF_MON_STATE_FRAME,
    
};


/**
 * @brief Info about how long certain things took. Useful for makers
 * to monitor performance with.
 */
struct performance_monitor_t {

    public:
    
    //--- Function declarations ---
    
    performance_monitor_t();
    void set_area_name(const string &name);
    void set_paused(const bool paused);
    void enter_state(const PERF_MON_STATE mode);
    void leave_state();
    void start_measurement(const string &name);
    void finish_measurement();
    void save_log();
    void reset();
    
    private:
    
    //--- Misc. declarations ---
    
    /**
     * @brief A page in the report.
     */
    struct page {
    
        public:
        
        //--- Members ---
        
        //How long it lasted for in total.
        double duration = 0.0f;
        
        //Measurements took, and how long each one took.
        vector<std::pair<string, double> > measurements;
        
        
        //--- Function declarations ---
        
        void write(string &s);
        
        private:
        
        //--- Function declarations ---
        
        void write_measurement(
            string &str, const string &name,
            const double time, const float total
        );
    };
    
    
    //--- Members ---
    
    //Name of the area being monitored.
    string area_name;
    
    //Current state.
    PERF_MON_STATE cur_state = PERF_MON_STATE_LOADING;
    
    //Is the monitoring currently paused?
    bool paused = false;
    
    //When the current state began.
    double cur_state_start_time = 0.0f;
    
    //When the current measurement began.
    double cur_measurement_start_time = 0.0f;
    
    //Name of the current measurement.
    string cur_measurement_name;
    
    //Page of information about the current working info.
    performance_monitor_t::page cur_page;
    
    //How many frames of gameplay have been sampled.
    size_t frame_samples = 0;
    
    //Page of information about the loading process.
    performance_monitor_t::page loading_page;
    
    //Page of information about the average frame.
    performance_monitor_t::page frame_avg_page;
    
    //Page of information about the fastest frame.
    performance_monitor_t::page frame_fastest_page;
    
    //Page of information about the slowest frame.
    performance_monitor_t::page frame_slowest_page;
    
};


struct subgroup_type_manager;


/**
 * @brief Represents a leader subgroup type;
 * a Red Pikmin, a Yellow Pikmin, a leader, etc.
 */
struct subgroup_type {

    public:
    
    //--- Function declarations ---
    
    SUBGROUP_TYPE_CATEGORY get_category() const { return category; }
    ALLEGRO_BITMAP* get_icon() const { return icon; }
    
private:

    friend subgroup_type_manager;
    
    //--- Members ---
    
    //Category this subgroup type belongs to.
    SUBGROUP_TYPE_CATEGORY category = SUBGROUP_TYPE_CATEGORY_LEADER;
    
    //Specific mob type it refers to.
    mob_type* specific_type = nullptr;
    
    //Icon used to represent this subgroup type.
    ALLEGRO_BITMAP* icon = nullptr;
    
};


/**
 * @brief Manages what types of subgroups exist.
 */
struct subgroup_type_manager {

    public:
    
    //--- Function declarations ---
    
    void register_type(
        const SUBGROUP_TYPE_CATEGORY category,
        mob_type* specific_type = nullptr,
        ALLEGRO_BITMAP* icon = nullptr
    );
    subgroup_type* get_type(
        const SUBGROUP_TYPE_CATEGORY category,
        const mob_type* specific_type = nullptr
    ) const;
    subgroup_type* get_first_type() const;
    subgroup_type* get_prev_type(const subgroup_type* sgt) const;
    subgroup_type* get_next_type(const subgroup_type* sgt) const;
    void clear();
    
    private:
    
    //--- Members ---
    
    //Known types.
    vector<subgroup_type*> types;
    
};


/**
 * @brief Info about the current amount of sprays and ingredients
 * for the available spray types.
 */
struct spray_stats_t {

    //--- Members ---
    
    //Number of sprays of this type owned.
    size_t nr_sprays = 0;
    
    //Number of concoction ingredients owned.
    size_t nr_ingredients = 0;
    
};


/**
 * @brief Info about the engine's lifetime statistics.
 */
struct statistics_t {

    //--- Members ---
    
    //Times Pikifen was started.
    uint32_t startups = 0;
    
    //Time Pikifen was running for, in seconds.
    double runtime = 0.0f;
    
    //Time gameplay happened for, in seconds.
    double gameplay_time = 0.0f;
    
    //Times areas were entered.
    uint32_t area_entries = 0;
    
    //Times Pikmin were born from an Onion.
    uint64_t pikmin_births = 0;
    
    //Times Pikmin died for other reasons.
    uint64_t pikmin_deaths = 0;
    
    //Times Pikmin died by being eaten.
    uint64_t pikmin_eaten = 0;
    
    //Times Pikmin died from a hazard.
    uint64_t pikmin_hazard_deaths = 0;
    
    //Times Pikmin bloomed (leaf to bud, leaf to flower, or bud to flower).
    uint64_t pikmin_blooms = 0;
    
    //Times Pikmin were saved from a hazard by being whistled.
    uint64_t pikmin_saved = 0;
    
    //Times enemies died.
    uint64_t enemy_deaths = 0;
    
    //Times Pikmin were thrown. Leaders thrown don't count.
    uint64_t pikmin_thrown = 0;
    
    //Times the whistle was used.
    uint64_t whistle_uses = 0;
    
    //Distance walked by an active leader, in pixels.
    double distance_walked = 0.0f;
    
    //Damage suffered by leaders.
    double leader_damage_suffered = 0.0f;
    
    //Damage caused by punches.
    double punch_damage_caused = 0.0f;
    
    //Times leaders were KO'd.
    uint64_t leader_kos = 0;
    
    //Times sprays were used.
    uint64_t sprays_used = 0;
    
};


/**
 * @brief Cached information about how an edge should draw its offset effects.
 */
struct edge_offset_cache {

    //--- Members ---
    
    //Length of the effect's "rectangle", per end vertex. 0 for none.
    float lengths[2] = {0.0f, 0.0f};
    
    //Angle of the effect's "rectangle", per end vertex.
    float angles[2] = {0.0f, 0.0f};
    
    //Color of the effect, per end vertex.
    ALLEGRO_COLOR colors[2] = {COLOR_EMPTY, COLOR_EMPTY};
    
    //Length of the effect's "elbow", per end vertex. 0 for none.
    float elbow_lengths[2] = {0.0f, 0.0f};
    
    //Angle of the effect's "elbow", per end vertex.
    float elbow_angles[2] = {0.0f, 0.0f};
    
    //Index of the vertex that should be processed first.
    unsigned char first_end_vertex_idx = 0;
    
};


/**
 * @brief Info about the current whistle usage.
 */
struct whistle_t {

    //--- Members ---
    
    //Current center.
    point center;
    
    //Current radius of the whistle.
    float radius = 0.0f;
    
    //Radius of every 6th dot.
    float dot_radius[6] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
    
    //Radius the whistle was at pre-fade.
    float fade_radius = 0.0f;
    
    //Time left for the whistle's fading animations.
    timer fade_timer = timer(WHISTLE::FADE_TIME);
    
    //Time left until the next series of dots begins.
    timer next_dot_timer = timer(WHISTLE::DOT_INTERVAL);
    
    //Time left until the next ring is spat out.
    timer next_ring_timer = timer(WHISTLE::RINGS_INTERVAL);
    
    //Color index of each ring.
    vector<unsigned char> ring_colors;
    
    //Color index of the previous ring.
    unsigned char ring_prev_color = 0;
    
    //Distance of each ring.
    vector<float> rings;
    
    //Is the whistle currently being blown?
    bool whistling = false;
    
    
    //--- Function declarations ---
    
    whistle_t();
    void start_whistling();
    void stop_whistling();
    void tick(
        const float delta_t, const point &center,
        const float whistle_range, const float leader_to_cursor_dist
    );
    
};


/**
 * @brief Asset manager.
 *
 * When you have the likes of an animation, every
 * frame in it is normally a sub-bitmap of the same
 * parent bitmap.
 * Naturally, loading from the disk and storing
 * in memory the same parent bitmap for every single
 * frame would be unbelievable catastrophical, so
 * that is why the asset manager was created.
 *
 * Whenever a frame of animation is being loaded,
 * it asks the asset manager to retrieve the
 * parent bitmap from memory. If the parent bitmap
 * has never been loaded, it gets loaded now.
 * When the next frame comes, and requests the
 * parent bitmap, the manager just returns the one already
 * loaded.
 * All the while, the manager is keeping track
 * of how many frames are referencing this parent bitmap.
 * When one of them doesn't need it any more, it sends
 * a free request (e.g.: when a frame is changed
 * in the animation editor, the entire bitmap
 * is destroyed and another is created).
 * This decreases the counter by one.
 * When the counter reaches 0, that means no frame
 * is needing the parent bitmap, so it gets destroyed.
 * If some other frame needs it, it'll be loaded from
 * the disk again.
 * This manager can also handle other types of asset, like audio samples.
 *
 * @tparam asset_t Asset type.
 */
template<typename asset_t>
class asset_manager {

public:

    //--- Function definitions ---
    
    /**
     * @brief Constructs a new asset manager object.
     *
     * @param base_dir Base folder path its files belong to,
     * relative to the program root folder.
     */
    explicit asset_manager(const string &base_dir) : base_dir(base_dir) {}
    
    /**
     * @brief Returns the specified asset, by name.
     *
     * @param name Name of the asset to get.
     * @param node If not nullptr, blame this data node if the file
     * doesn't exist.
     * @param report_errors Only issues errors if this is true.
     * @return The asset
     */
    asset_t get(
        const string &name, data_node* node = nullptr,
        const bool report_errors = true
    ) {
        if(name.empty()) return do_load("", node, report_errors);
        
        if(list.find(name) == list.end()) {
            asset_t asset_ptr =
                do_load(base_dir + "/" + name, node, report_errors);
            list[name] = asset_use_t(asset_ptr);
            total_uses++;
            return asset_ptr;
        } else {
            list[name].uses++;
            total_uses++;
            return list[name].ptr;
        }
    }
    
    /**
     * @brief Frees one use of the asset. If the asset has no more calls,
     * it's automatically cleared.
     *
     * @param ptr Asset to free.
     */
    void free(const asset_t ptr) {
        if(!ptr) return;
        auto it = list.begin();
        for(; it != list.end(); ++it) {
            if(it->second.ptr == ptr) break;
        }
        free(it);
    }
    
    /**
     * @brief Frees one use of the asset. If the asset has no more calls,
     * it's automatically cleared.
     *
     * @param ptr Name of the asset to free.
     */
    void free(const string &name) {
        if(name.empty()) return;
        free(list.find(name));
    }
    
    /**
     * @brief Unloads all assets loaded and clears the list.
     */
    void clear() {
        for(auto &asset : list) {
            do_unload(asset.second.ptr);
        }
        list.clear();
        total_uses = 0;
    }
    
    /**
     * @brief Returns the total number of uses. Used for debugging.
     *
     * @return The amount.
     */
    long get_total_uses() const {
        return total_uses;
    }
    
    /**
     * @brief Returns the size of the list. Used for debugging.
     *
     * @return The size.
     */
    size_t get_list_size() const {
        return list.size();
    }
    
protected:

    //--- Misc. declarations ---
    
    virtual asset_t do_load(
        const string &path, data_node* node, const bool report_errors
    ) = 0;
    virtual void do_unload(asset_t asset) = 0;
    
    /**
     * @brief Info about an asset.
     */
    struct asset_use_t {
    
        //--- Members ---
        
        //Asset pointer.
        asset_t ptr = nullptr;
        
        //How many uses it has.
        size_t uses = 1;
        
        
        //--- Function declarations ---
        
        explicit asset_use_t(asset_t ptr = nullptr) : ptr(ptr) {}
    };
    
    
    //--- Members ---
    
    //Base folder path that this manager works on,
    //relative to the program root folder.
    string base_dir;
    
    //List of loaded assets.
    map<string, asset_use_t> list;
    
    //Total sum of uses. Useful for debugging.
    long total_uses = 0;
    
    
    //--- Function definitions ---
    
    /**
     * @brief Frees one use of the asset. If the asset has no more calls,
     * it's automatically cleared.
     *
     * @param it Iterator of the asset from the list.
     */
    void free(typename map<string, asset_use_t>::iterator it) {
        if(it == list.end()) return;
        it->second.uses--;
        total_uses--;
        if(it->second.uses == 0) {
            do_unload(it->second.ptr);
            list.erase(it);
        }
    }
    
};


/**
 * @brief Audio stream manager. See asset_manager.
 */
class audio_stream_manager : public asset_manager<ALLEGRO_AUDIO_STREAM*> {

public:

    //--- Function definitions ---
    
    audio_stream_manager(const string &base_dir) : asset_manager(base_dir) {}
    
    
protected:

    //--- Function declarations ---
    
    ALLEGRO_AUDIO_STREAM* do_load(
        const string &path, data_node* node, const bool report_errors
    ) override;
    void do_unload(ALLEGRO_AUDIO_STREAM* asset) override;
    
};


/**
 * @brief Bitmap manager. See asset_manager.
 */
class bitmap_manager : public asset_manager<ALLEGRO_BITMAP*> {

public:

    //--- Function definitions ---
    
    bitmap_manager(const string &base_dir) : asset_manager(base_dir) {}
    
    
protected:

    //--- Function declarations ---
    
    ALLEGRO_BITMAP* do_load(
        const string &path, data_node* node, const bool report_errors
    ) override;
    void do_unload(ALLEGRO_BITMAP* asset) override;
    
};


/**
 * @brief Sound effect sample manager. See asset_manager.
 */
class sfx_sample_manager : public asset_manager<ALLEGRO_SAMPLE*> {

public:

    //--- Function definitions ---
    
    sfx_sample_manager(const string &base_dir) : asset_manager(base_dir) {}
    
    
protected:

    //--- Function declarations ---
    
    ALLEGRO_SAMPLE* do_load(
        const string &path, data_node* node, const bool report_errors
    ) override;
    void do_unload(ALLEGRO_SAMPLE* asset) override;
    
};
