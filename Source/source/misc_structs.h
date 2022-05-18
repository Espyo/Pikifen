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

#ifndef MISC_STRUCTS_INCLUDED
#define MISC_STRUCTS_INCLUDED

#include <functional>
#include <map>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>

#include "animation.h"
#include "mob_categories/mob_category.h"
#include "mobs/mob_enums.h"
#include "hazard.h"
#include "particle.h"
#include "utils/data_file.h"
#include "utils/geometry_utils.h"


class pikmin_type;

using std::map;
using std::size_t;
using std::string;
using std::vector;

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
enum MAKER_TOOL_TYPES {
    //None.
    MAKER_TOOL_NONE,
    //Create an image of the whole area.
    MAKER_TOOL_AREA_IMAGE,
    //Change gameplay speed.
    MAKER_TOOL_CHANGE_SPEED,
    //Geometry info beneath mouse cursor.
    MAKER_TOOL_GEOMETRY_INFO,
    //Show hitboxes.
    MAKER_TOOL_HITBOXES,
    //Hurt mob beneath mouse cursor.
    MAKER_TOOL_HURT_MOB,
    //Get info on the mob beneath mouse cursor.
    MAKER_TOOL_MOB_INFO,
    //Create a new Pikmin beneath mouse cursor.
    MAKER_TOOL_NEW_PIKMIN,
    //Teleport to mouse cursor.
    MAKER_TOOL_TELEPORT,
    
    //Total amount of maker tools.
    N_MAKER_TOOLS,
};


namespace MAKER_TOOLS {
extern const string NAMES[N_MAKER_TOOLS];
}


//Types of string token.
enum STRING_TOKEN_TYPES {
    //None.
    STRING_TOKEN_NONE,
    //A regular character.
    STRING_TOKEN_CHAR,
    //A line break.
    STRING_TOKEN_NEWLINE,
    //A control icon.
    STRING_TOKEN_CONTROL,
};


/* ----------------------------------------------------------------------------
 * List of file names of system assets.
 */
struct asset_file_names_struct {
    //Area name font.
    string area_name_font;
    //Bright circle.
    string bright_circle;
    //Bright ring.
    string bright_ring;
    //Bubble box.
    string bubble_box;
    //Checkbox checkmark.
    string checkbox_check;
    //Misc. control icons.
    string control_icons;
    //Counter font.
    string counter_font;
    //Leader cursor.
    string cursor;
    //Cursor counter font.
    string cursor_counter_font;
    //Editor icons.
    string editor_icons;
    //Enemy spirit.
    string enemy_spirit;
    //GUI focus box.
    string focus_box;
    //Pikifen icon.
    string icon;
    //Idle glow.
    string idle_glow;
    //Main font.
    string main_font;
    //Main menu background.
    string main_menu;
    //"More..." icon.
    string more;
    //Mouse cursor.
    string mouse_cursor;
    //Notification.
    string notification;
    //Pikmin silhouette.
    string pikmin_silhouette;
    //Pikmin spirit.
    string pikmin_spirit;
    //A rock.
    string rock;
    //Slim font.
    string slim_font;
    //Mob shadow.
    string shadow;
    //Smack effect.
    string smack;
    //Smoke.
    string smoke;
    //Sparkle effect.
    string sparkle;
    //Spotlight for blackout.
    string spotlight;
    //Swarm arrow.
    string swarm_arrow;
    //Invalid throw marker.
    string throw_invalid;
    //Throw preview texture.
    string throw_preview;
    //Dashed throw preview texture.
    string throw_preview_dashed;
    //Value font.
    string value_font;
    //Wave ring.
    string wave_ring;
    
    void load(data_node* file);
    
    asset_file_names_struct();
};


/* ----------------------------------------------------------------------------
 * Information about the game camera. Where it is, where it wants to go, etc.
 */
struct camera_info {
    //Top-left and bottom-right world coordinates that this camera can see.
    point box[2];
    //Current position.
    point pos;
    //Position it wants to be at.
    point target_pos;
    //Zoom it wants to be at.
    float target_zoom;
    //Current zoom.
    float zoom;
    
    void set_pos(const point &new_pos);
    void set_zoom(const float new_zoom);
    void tick(const float delta_t);
    void update_box();
    
    camera_info();
};


/* ----------------------------------------------------------------------------
 * Contains info about a token in a string.
 */
struct string_token {
    //Type of token.
    STRING_TOKEN_TYPES type;
    //Its content.
    string content;
    //Width that it takes up, in pixels.
    int width;
    //Constructor.
    string_token() : type(STRING_TOKEN_CHAR), width(0) {}
};


/* ----------------------------------------------------------------------------
 * A timer. You can set it to start at a pre-determined time, to tick, etc.
 */
struct timer {
    //How much time is left until 0.
    float time_left;
    //When the timer starts, its time is set to this.
    float duration;
    //Code to run when the timer ends, if any.
    std::function<void()> on_end;
    
    timer(
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



/* ----------------------------------------------------------------------------
 * Information about all of the maker tools.
 */
struct maker_tools_info {
    //Are the tools enabled?
    bool enabled;
    //Show tree shadows in the area image tool?
    bool area_image_shadows;
    //Maximum width or height of the area image.
    int area_image_size;
    //Show mobs in the area image?
    bool area_image_mobs;
    //Automatically pick this from the list of the selected auto-entry mode.
    string auto_start_option;
    //Automatically enter this game mode when the game boots.
    string auto_start_mode;
    //Are we currently changing the game speed?
    bool change_speed;
    //Multiplier to change the game speed by.
    float change_speed_mult;
    //Is the geometry information tool enabled?
    bool geometry_info;
    //Are hitboxes visible in-game?
    bool hitboxes;
    //Mob currently locked-on to for the mob information tool. NULL if off.
    mob* info_lock;
    //If any maker info is being printed, this is how long it lasts on-screen.
    float info_print_duration;
    //If any maker info is being printed, this is how long its fade lasts.
    float info_print_fade_duration;
    //If any maker info is being printed, this is its text.
    string info_print_text;
    //If any maker info is being printed, this represents its time to live.
    timer info_print_timer;
    //For each key (F2 - F11, 0 - 9), what tool is bound to it?
    MAKER_TOOL_TYPES keys[20];
    //When we last spawned a Pikmin, what was its type?
    pikmin_type* last_pikmin_type;
    //When hurting mobs with the hurting tool, dock this much of its max HP off.
    float mob_hurting_ratio;
    //Use the performance monitor?
    bool use_perf_mon;
    //Has the player made use of any tools that could help them play?
    bool used_helping_tools;
    
    maker_tools_info();
    void reset_for_gameplay();
};


/* ----------------------------------------------------------------------------
 * Bitmap manager.
 * When you have the likes of an animation, every
 * frame in it is normally a sub-bitmap of the same
 * parent bitmap.
 * Naturally, loading from the disk and storing
 * in memory the same parent bitmap for every single
 * frame would be unbelievable catastrophical, so
 * that is why the bitmap manager was created.
 *
 * Whenever a frame of animation is being loaded,
 * it asks the bitmap manager to retrieve the
 * parent bitmap from memory. If the parent bitmap
 * has never been loaded, it gets loaded now.
 * When the next frame comes, and requests the
 * parent bitmap, the manager just returns the one already
 * loaded.
 * All the while, the manager is keeping track
 * of how many frames are referencing this parent bitmap.
 * When one of them doesn't need it any more, it sends
 * a detach request (e.g.: when a frame is changed
 * in the animation editor, the entire bitmap
 * is destroyed and another is created).
 * This decreases the counter by one.
 * When the counter reaches 0, that means no frame
 * is needing the parent bitmap, so it gets destroyed.
 * If some other frame needs it, it'll be loaded from
 * the disk again.
 * Finally, it should be noted that animation frames
 * are not the only thing using the bitmap manager.
 */
struct bmp_manager {
public:
    bmp_manager(const string &base_dir);
    ALLEGRO_BITMAP* get(
        const string &name, data_node* node = NULL,
        const bool report_errors = true
    );
    void detach(ALLEGRO_BITMAP* bmp);
    void detach(const string &name);
    void clear();
    
    long get_total_calls() const;
    size_t get_list_size() const;
    
private:
    struct bmp_info {
        //Bitmap pointer.
        ALLEGRO_BITMAP* b;
        //How many calls it has.
        size_t calls;
        
        bmp_info(ALLEGRO_BITMAP* b = NULL);
    };
    //Base directory that this manager works on.
    string base_dir;
    //List of loaded bitmaps.
    map<string, bmp_info> list;
    //Total sum of calls. Useful for debugging.
    long total_calls;
    
    void detach(map<string, bmp_info>::iterator it);
    
};



/* ----------------------------------------------------------------------------
 * List of fonts used in the game.
 */
struct font_list {
    //Font for the area's name in loading screens.
    ALLEGRO_FONT* area_name;
    //Allegro's built-in font.
    ALLEGRO_FONT* builtin;
    //Font for HUD counters.
    ALLEGRO_FONT* counter;
    //Counter displayed next to the cursor.
    ALLEGRO_FONT* cursor_counter;
    //Font for slim text.
    ALLEGRO_FONT* slim;
    //Font for standard text.
    ALLEGRO_FONT* standard;
    //Font for the carrying / money values.
    ALLEGRO_FONT* value;
    
    font_list();
};



/* ----------------------------------------------------------------------------
 * This structure holds information about where the player wants a leader
 * (or something else) to go, based on the player's inputs
 * (analog stick tilts, D-pad presses, keyboard key presses, etc.).
 *
 * It can also churn out "clean" information based on this.
 * For D-pads or keyboard presses, the "clean" result is basically the same
 * direction and magnitude, but for joysticks, deadzones are taken into account.
 *
 * A loose joystick that's not being touched can send signals we don't want,
 * since they're not player input, but the "clean" information filters out
 * these minimal stick tilts. Similarly, some controllers might not
 * send 1.0 when the player holds fully right, for instance. So there should
 * also be a top deadzone, like 90%, where if the player is beyond that, we'll
 * just consider it as 100%.
 */
struct movement_struct {
    //Amount to the east.
    float right;
    //Amount to the north.
    float up;
    //Amount to the west.
    float left;
    //Amount to the south.
    float down;
    
    movement_struct();
    void get_raw_info(point* coords, float* angle, float* magnitude) const;
    void get_clean_info(point* coords, float* angle, float* magnitude) const;
    void reset();
};



/* ----------------------------------------------------------------------------
 * Information about the current on-screen message box, if any.
 */
struct msg_box_info {
    //Timer until the next character shows up.
    timer char_timer;
    //What character are we in?
    size_t cur_char;
    //What section of the message are we in?
    size_t cur_section;
    //Full text of the message.
    string message;
    //Icon that represents the speaker, if any.
    ALLEGRO_BITMAP* speaker_icon;
    //Stops scrolling when it reaches one of these. There's one per section.
    vector<size_t> stopping_chars;
    //Time left in the current transition.
    float transition_timer;
    //Is it transitioning into view, or out of view?
    bool transition_in;
    //Is the message box meant to be deleted?
    bool to_delete;
    
    void advance();
    vector<string> get_current_lines() const;
    void tick(const float delta_t);
    
    msg_box_info(const string &text, ALLEGRO_BITMAP* speaker_icon);
};



/* ----------------------------------------------------------------------------
 * This structure makes reading values in data files
 * and setting them to variables much easier.
 * On the set functions, specify the name of the child and the variable.
 * If the child is empty, the variable will not be set.
 */
struct reader_setter {
    //Node that this reader-setter pertains to.
    data_node* node;
    
    void set(
        const string &child, string &var, data_node** child_node = NULL
    );
    void set(
        const string &child, size_t &var, data_node** child_node = NULL
    );
    void set(
        const string &child, int &var, data_node** child_node = NULL
    );
    void set(
        const string &child, unsigned char &var, data_node** child_node = NULL
    );
    void set(
        const string &child, bool &var, data_node** child_node = NULL
    );
    void set(
        const string &child, float &var, data_node** child_node = NULL
    );
    void set(
        const string &child, ALLEGRO_COLOR &var, data_node** child_node = NULL
    );
    void set(
        const string &child, point &var, data_node** child_node = NULL
    );
    reader_setter(data_node* dn = NULL);
};



/* ----------------------------------------------------------------------------
 * Structure that holds informatio about a sample.
 * It also has info about sample instances, which control
 * the sound playing from the sample.
 */
struct sample_struct {
    //Pointer to the sample.
    ALLEGRO_SAMPLE* sample;
    //Pointer to the instance.
    ALLEGRO_SAMPLE_INSTANCE* instance;
    
    sample_struct(ALLEGRO_SAMPLE* sample = NULL, ALLEGRO_MIXER* mixer = NULL);
    void play(
        const float max_override_pos, const bool loop,
        const float gain = 1.0, const float pan = 0.5, const float speed = 1.0
    );
    void stop();
    void destroy();
};



/* ----------------------------------------------------------------------------
 * Makes it easy to read script variables, and make changes based on which
 * ones exist, and what values they have.
 */
struct script_var_reader {
    //Reference to the list of script variables it pertains to.
    map<string, string> &vars;
    
    bool get(const string &name, string &dest) const;
    bool get(const string &name, size_t &dest) const;
    bool get(const string &name, int &dest) const;
    bool get(const string &name, unsigned char &dest) const;
    bool get(const string &name, bool &dest) const;
    bool get(const string &name, float &dest) const;
    bool get(const string &name, ALLEGRO_COLOR &dest) const;
    bool get(const string &name, point &dest) const;
    script_var_reader(map<string, string> &vars);
};



/* ----------------------------------------------------------------------------
 * List of loaded system assets.
 */
struct system_asset_list {
    //Bright circle.
    ALLEGRO_BITMAP* bmp_bright_circle;
    //Bright ring.
    ALLEGRO_BITMAP* bmp_bright_ring;
    //Bubble box.
    ALLEGRO_BITMAP* bmp_bubble_box;
    //Checkbox checkmark.
    ALLEGRO_BITMAP* bmp_checkbox_check;
    //Misc. control icons.
    ALLEGRO_BITMAP* bmp_control_icons;
    //Leader cursor.
    ALLEGRO_BITMAP* bmp_cursor;
    //Enemy spirit.
    ALLEGRO_BITMAP* bmp_enemy_spirit;
    //Focus box.
    ALLEGRO_BITMAP* bmp_focus_box;
    //Pikifen icon.
    ALLEGRO_BITMAP* bmp_icon;
    //Idle glow.
    ALLEGRO_BITMAP* bmp_idle_glow;
    //"More..." icon.
    ALLEGRO_BITMAP* bmp_more;
    //Mouse cursor.
    ALLEGRO_BITMAP* bmp_mouse_cursor;
    //Notification.
    ALLEGRO_BITMAP* bmp_notification;
    //Pikmin silhouette.
    ALLEGRO_BITMAP* bmp_pikmin_silhouette;
    //Pikmin spirit.
    ALLEGRO_BITMAP* bmp_pikmin_spirit;
    //A rock.
    ALLEGRO_BITMAP* bmp_rock;
    //Mob shadow.
    ALLEGRO_BITMAP* bmp_shadow;
    //Smack effect.
    ALLEGRO_BITMAP* bmp_smack;
    //Smoke.
    ALLEGRO_BITMAP* bmp_smoke;
    //Sparkle effect.
    ALLEGRO_BITMAP* bmp_sparkle;
    //Spotlight for blackout.
    ALLEGRO_BITMAP* bmp_spotlight;
    //Swarm arrow.
    ALLEGRO_BITMAP* bmp_swarm_arrow;
    //Invalid throw marker.
    ALLEGRO_BITMAP* bmp_throw_invalid;
    //Throw preview texture.
    ALLEGRO_BITMAP* bmp_throw_preview;
    //Dashed throw preview texture.
    ALLEGRO_BITMAP* bmp_throw_preview_dashed;
    //Wave ring.
    ALLEGRO_BITMAP* bmp_wave_ring;
    
    //Sound effects.
    //Attack.
    sample_struct sfx_attack;
    //Camera zoom level.
    sample_struct sfx_camera;
    //Pikmin attacking.
    sample_struct sfx_pikmin_attack;
    //Pikmin called.
    sample_struct sfx_pikmin_called;
    //Pikmin carrying.
    sample_struct sfx_pikmin_carrying;
    //Pikmin grabbing on to carry.
    sample_struct sfx_pikmin_carrying_grab;
    //Pikmin caught.
    sample_struct sfx_pikmin_caught;
    //Pikmin dying.
    sample_struct sfx_pikmin_dying;
    //Pikmin held by leader.
    sample_struct sfx_pikmin_held;
    //Pikmin idling.
    sample_struct sfx_pikmin_idle;
    //Pluck sound effect.
    sample_struct sfx_pluck;
    //Pikmin being plucked.
    sample_struct sfx_pikmin_plucked;
    //Pikmin being thrown.
    sample_struct sfx_pikmin_thrown;
    //Switching standby Pikmin type.
    sample_struct sfx_switch_pikmin;
    //Throwing.
    sample_struct sfx_throw;
    
    //Animations.
    //Leader damage spark.
    single_animation_suite spark_animation;
    
    system_asset_list();
};


/* ----------------------------------------------------------------------------
 * Manages fade ins/outs for transitions.
 */
struct fade_manager {
public:
    static const float FADE_DURATION;
    
    fade_manager();
    void start_fade(const bool fade_in, const std::function<void()> &on_end);
    bool is_fade_in() const;
    bool is_fading() const;
    float get_perc_left() const;
    void tick(const float delta_t);
    void draw();
    
private:
    //Time left in the current fade in/out.
    float time_left;
    //True if fading in, false if fading out.
    bool fade_in;
    //Code to run when the fade in/out finishes.
    std::function<void()> on_end;
    
};



/* ----------------------------------------------------------------------------
 * Type of spike damage.
 * When a mob is attacked, it can instantly deal some damage to the mob
 * that attacked it.
 */
struct spike_damage_type {
    //Name of the type. "Poison", "Ice", etc.
    string name;
    //Amount of damage to cause, either in absolute HP or max HP ratio.
    float damage;
    //If true, damage is only dealt if the victim is eaten. e.g. White Pikmin.
    bool ingestion_only;
    //If true, the damage var represents max HP ratio. If false, absolute HP.
    bool is_damage_ratio;
    //Particle generator to use to generate particles, if any.
    particle_generator* particle_gen;
    //Offset the particles by this much, horizontally.
    point particle_offset_pos;
    //Offset the particles by this much, vertically.
    float particle_offset_z;
    //Apply this status effect when the spike damage is applied.
    status_type* status_to_apply;
    
    spike_damage_type() :
        damage(0),
        ingestion_only(false),
        is_damage_ratio(false),
        particle_gen(nullptr),
        status_to_apply(nullptr) {
        
    }
};



/* ----------------------------------------------------------------------------
 * Contains information on how a bitmap should be drawn, in regards to
 * translation, rotation, coloring, etc.
 */
struct bitmap_effect_info {
    //Offset horizontally and vertically by this much.
    point translation;
    //Rotate the bitmap by this angle, in radians.
    float rotation;
    //Scale horizontally and vertically. LARGE_FLOAT = use the other's scale.
    point scale;
    //Tint the bitmap by this color. Also makes it transparent.
    ALLEGRO_COLOR tint_color;
    //Re-draws the bitmap on top, in additive blend, with this color.
    ALLEGRO_COLOR glow_color;
    
    bitmap_effect_info();
};


//Performance monitor states.
enum PERF_MON_STATES {
    //Measuring loading times.
    PERF_MON_STATE_LOADING,
    //Measuring gameplay frame performance.
    PERF_MON_STATE_FRAME,
};


/* ----------------------------------------------------------------------------
 * Contains information about how long certain things took. Useful for makers
 * to monitor performance with.
 */
struct performance_monitor_struct {
public:
    performance_monitor_struct();
    void set_area_name(const string &name);
    void set_paused(const bool paused);
    void enter_state(const PERF_MON_STATES mode);
    void leave_state();
    void start_measurement(const string &name);
    void finish_measurement();
    void save_log();
    void reset();
    
private:

    struct page {
    public:
        //How long it lasted for in total.
        double duration;
        //Measurements took, and how long each one took.
        vector<std::pair<string, double> > measurements;
        
        page();
        void write(string &s);
        
    private:
        void write_measurement(
            string &str, const string &name,
            const double time, const float total
        );
    };
    
    //Name of the area being monitored.
    string area_name;
    //Current state.
    PERF_MON_STATES cur_state;
    //Is the monitoring currently paused?
    bool paused;
    //When the current state began.
    double cur_state_start_time;
    //When the current measurement began.
    double cur_measurement_start_time;
    //Name of the current measurement.
    string cur_measurement_name;
    //Page of information about the current working info.
    performance_monitor_struct::page cur_page;
    //How many frames of gameplay have been sampled.
    size_t frame_samples;
    //Page of information about the loading process.
    performance_monitor_struct::page loading_page;
    //Page of information about the average frame.
    performance_monitor_struct::page frame_avg_page;
    //Page of information about the fastest frame.
    performance_monitor_struct::page frame_fastest_page;
    //Page of information about the slowest frame.
    performance_monitor_struct::page frame_slowest_page;
};


struct subgroup_type_manager;


/* ----------------------------------------------------------------------------
 * Represents a leader subgroup type;
 * a Red Pikmin, a Yellow Pikmin, a leader, etc.
 */
struct subgroup_type {
public:
    SUBGROUP_TYPE_CATEGORIES get_category() const { return category; }
    ALLEGRO_BITMAP* get_icon() const { return icon; }
    
private:
    friend subgroup_type_manager;
    //Category this subgroup type belongs to.
    SUBGROUP_TYPE_CATEGORIES category;
    //Specific mob type it refers to.
    mob_type* specific_type;
    //Icon used to represent this subgroup type.
    ALLEGRO_BITMAP* icon;
    
    subgroup_type() : specific_type(nullptr), icon(nullptr) { }
};


/* ----------------------------------------------------------------------------
 * Manages what types of subgroups exist.
 */
struct subgroup_type_manager {
public:
    void register_type(
        const SUBGROUP_TYPE_CATEGORIES category,
        mob_type* specific_type = NULL,
        ALLEGRO_BITMAP* icon = NULL
    );
    subgroup_type* get_type(
        const SUBGROUP_TYPE_CATEGORIES category,
        mob_type* specific_type = NULL
    ) const;
    subgroup_type* get_first_type() const;
    subgroup_type* get_prev_type(subgroup_type* sgt) const;
    subgroup_type* get_next_type(subgroup_type* sgt) const;
    void clear();
    
private:
    //Known types.
    vector<subgroup_type*> types;
};


/* ----------------------------------------------------------------------------
 * Contains info about the current amount of sprays and ingredients
 * for the available spray types.
 */
struct spray_stats_struct {
    //Number of sprays of this type owned.
    size_t nr_sprays;
    //Number of concoction ingredients owned.
    size_t nr_ingredients;
    
    spray_stats_struct() : nr_sprays(0), nr_ingredients(0) { }
};


/* ----------------------------------------------------------------------------
 * Contains info about the current whistle usage.
 */
struct whistle_struct {
    //Current center.
    point center;
    //Current radius of the whistle.
    float radius;
    //Radius of every 6th dot.
    float dot_radius[6];
    //Radius the whistle was at pre-fade.
    float fade_radius;
    //Time left for the whistle's fading animations.
    timer fade_timer;
    //Time left until the next series of dots begins.
    timer next_dot_timer;
    //Time left until the next ring is spat out.
    timer next_ring_timer;
    //Color index of each ring.
    vector<unsigned char> ring_colors;
    //Color index of the previous ring.
    unsigned char ring_prev_color;
    //Distance of each ring.
    vector<float> rings;
    //Is the whistle currently being blown?
    bool whistling;
    
    void start_whistling();
    void stop_whistling();
    void tick(
        const float delta_t, const point &center,
        const float whistle_range, const float leader_to_cursor_dist
    );
    
    whistle_struct();
};


#endif //ifndef MISC_STRUCTS_INCLUDED
