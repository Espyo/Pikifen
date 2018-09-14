/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
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

#include "geometry_utils.h"
#include "mobs/mob_category.h"
#include "particle.h"

class mob;
class pikmin_type;
struct mob_gen;

using namespace std;

/* ----------------------------------------------------------------------------
 * A timer. You can set it to start at a pre-determined time, to tick, etc.
 */
struct timer {
    float time_left; //How much time is left until 0.
    float duration;  //When the timer starts, its time is set to this.
    function<void()> on_end;
    
    timer(const float duration = 0, const function<void()> &on_end = nullptr);
    ~timer();
    void start(const bool can_restart = true);
    void start(const float new_duration);
    void tick(const float amount);
    float get_ratio_left();
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
private:
    struct bmp_info {
        ALLEGRO_BITMAP* b;
        size_t calls;
        bmp_info(ALLEGRO_BITMAP* b = NULL);
    };
    string base_dir;
    map<string, bmp_info> list;
    long total_calls; //Useful for debugging.
    void detach(map<string, bmp_info>::iterator it);
    
public:
    bmp_manager(const string &base_dir);
    ALLEGRO_BITMAP* get(
        const string &name, data_node* node = NULL,
        const bool report_errors = true
    );
    void detach(ALLEGRO_BITMAP* bmp);
    void detach(const string &name);
    void clear();
    
    long get_total_calls();
    size_t get_list_size();
};



/* ----------------------------------------------------------------------------
 * Manager for the different gameplay "buttons", associated with the controls.
 */
struct button_manager {
    struct button {
        size_t id;
        string name;
        string option_name;
        string default_control_str;
    };
    
    vector<button> list;
    void add(
        const size_t id, const string &name, const string &option_name,
        const string &default_control_str
    );
};



/* ----------------------------------------------------------------------------
 * A distance.
 * Basically this is just a number, but for optimization's sake,
 * this number is actually the distance SQUARED.
 * It's faster to compare two squared distances than square-rooting them both,
 * since sqrt() is so costly. If we do need to sqrt() a number, we keep it in
 * a cache inside the class, so that we can use it at will next time.
 * Fun fact, keeping an extra boolean in the class that indicates whether or
 * not the sqrt()'d number is in cache is around twice as fast as keeping
 * only the squared and sqrt()'d numbers, and setting the sqrt()'d number
 * to LARGE_FLOAT if it is uncached.
 */
struct dist {
private:
    float distance_squared;
    float normal_distance;
    bool has_normal_distance;
    
public:
    dist(const point &p1, const point &p2);
    dist(const float d = 0.0f);
    dist &operator =(const float d);
    bool operator <(const float d2);
    bool operator <(const dist &d2);
    bool operator <=(const float d2);
    bool operator <=(const dist &d2);
    bool operator >(const float d2);
    bool operator >(const dist &d2);
    bool operator >=(const float d2);
    bool operator >=(const dist &d2);
    bool operator ==(const float d2);
    bool operator ==(const dist &d2);
    bool operator !=(const float d2);
    bool operator !=(const dist &d2);
    void operator +=(const dist &d2);
    float to_float();
};



/* ----------------------------------------------------------------------------
 * Represents a HUD item. It contains data about where it should be placed,
 * where it should be drawn, etc.
 */
struct hud_item {
    point center; //In screen ratio.
    point size;   //In screen ratio.
    hud_item(const point center = point(), const point size = point());
};



/* ----------------------------------------------------------------------------
 * Manages the HUD items.
 */
struct hud_item_manager {
private:
    vector<hud_item> items;
    bool move_in;
    timer move_timer;
    bool offscreen;
    void update_offscreen();
    
public:
    void set_item(
        const size_t id,
        const float x, const float y, const float w, const float h
    );
    bool get_draw_data(const size_t id, point* center, point* size);
    void start_move(const bool in, const float duration);
    void tick(const float time);
    hud_item_manager(const size_t item_total);
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
    float right;
    float up;
    float left;
    float down;
    
    void get_raw_info(point* coords, float* angle, float* magnitude);
    void get_clean_info(point* coords, float* angle, float* magnitude);
    movement_struct();
};



/* ----------------------------------------------------------------------------
 * This structure makes reading values in data files
 * and setting them to variables much easier.
 * On the set functions, specify the name of the child and the variable.
 * If the child is empty, the variable will not be set.
 */
struct reader_setter {
    data_node* node;
    void set(const string &child, string &var);
    void set(const string &child, size_t &var);
    void set(const string &child, int &var);
    void set(const string &child, unsigned char &var);
    void set(const string &child, bool &var);
    void set(const string &child, float &var);
    void set(const string &child, ALLEGRO_COLOR &var);
    void set(const string &child, point &var);
    reader_setter(data_node* dn = NULL);
};



/* ----------------------------------------------------------------------------
 * Structure that holds informatio about a sample.
 * It also has info about sample instances, which control
 * the sound playing from the sample.
 */
struct sample_struct {
    ALLEGRO_SAMPLE*          sample;   //Pointer to the sample.
    ALLEGRO_SAMPLE_INSTANCE* instance; //Pointer to the instance.
    
    sample_struct(ALLEGRO_SAMPLE* sample = NULL, ALLEGRO_MIXER* mixer = NULL);
    void play(
        const float max_override_pos, const bool loop,
        const float gain = 1.0, const float pan = 0.5, const float speed = 1.0
    );
    void stop();
    void destroy();
};



/* ----------------------------------------------------------------------------
 * Just a list of the different sector types.
 * The SECTOR_TYPE_* constants are meant to be used here.
 * This is a vector instead of a map because hopefully,
 * the numbers are filled in sequence, as they're from
 * an enum, hence, there are no gaps.
 */
struct sector_types_manager {
private:
    vector<string> names;
    
public:
    void register_type(const unsigned char nr, const string &name);
    unsigned char get_nr(const string &name);
    string get_name(const unsigned char nr);
    unsigned char get_nr_of_types();
    
};



/* ----------------------------------------------------------------------------
 * Manages fade ins/outs for transitions.
 */
struct fade_manager {
private:
    float time_left;
    bool fade_in;
    function<void()> on_end;
    
public:
    static const float FADE_DURATION;
    
    fade_manager();
    void start_fade(const bool fade_in, const function<void()> &on_end);
    bool is_fade_in();
    bool is_fading();
    float get_perc_left();
    void tick(const float time);
    void draw();
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
    
    spike_damage_type() :
        damage(0),
        ingestion_only(false),
        is_damage_ratio(false),
        particle_gen(nullptr) {
        
    }
};



/* ----------------------------------------------------------------------------
 * Holds information about how a sprite should be resized, colored, etc.,
 * over time, right before it is drawn to the screen.
 */
struct bitmap_effect_props {
    point translation;
    float rotation;
    point scale;
    ALLEGRO_COLOR tint_color;
    ALLEGRO_COLOR glow_color;
    
    bitmap_effect_props();
};



/* ----------------------------------------------------------------------------
 * A full bitmap effect. It is made of several properties, and has the
 * information necessary to interpolate the properties' values over time.
 * If an effect only has one keyframe, no interpolations are made.
 */
struct bitmap_effect {
private:
    map<float, bitmap_effect_props> keyframes;
    float cur_time;
    
public:
    void add_keyframe(const float time, const bitmap_effect_props &props);
    void set_cur_time(const float cur_time);
    bitmap_effect_props get_final_properties();
    
    bitmap_effect();
};



/* ----------------------------------------------------------------------------
 * Holds several bitmap effect structs.
 */
struct bitmap_effect_manager {
private:
    vector<bitmap_effect> effects;
    
public:
    void add_effect(bitmap_effect e);
    bitmap_effect_props get_final_properties();
};



enum SUBGROUP_TYPE_CATEGORIES {
    SUBGROUP_TYPE_CATEGORY_PIKMIN,
    SUBGROUP_TYPE_CATEGORY_LEADER,
    SUBGROUP_TYPE_CATEGORY_BOMB,
};
struct subgroup_type_manager;

/* ----------------------------------------------------------------------------
 * Represents a leader subgroup type;
 * a Red Pikmin, a Yellow Pikmin, a leader, etc.
 */
struct subgroup_type {
private:
    friend subgroup_type_manager;
    SUBGROUP_TYPE_CATEGORIES category;
    pikmin_type* pik_type;
    subgroup_type() : pik_type(nullptr) { }
};


/* ----------------------------------------------------------------------------
 * Manages what types of subgroups exist.
 */
struct subgroup_type_manager {
private:
    vector<subgroup_type*> types;
public:
    void register_type(
        const SUBGROUP_TYPE_CATEGORIES category,
        pikmin_type* pik_type = NULL
    );
    subgroup_type* get_type(
        const SUBGROUP_TYPE_CATEGORIES category,
        pikmin_type* pik_type = NULL
    );
    subgroup_type* get_first_type();
    subgroup_type* get_prev_type(subgroup_type* sgt);
    subgroup_type* get_next_type(subgroup_type* sgt);
    void clear();
};


#endif //ifndef MISC_STRUCTS_INCLUDED
