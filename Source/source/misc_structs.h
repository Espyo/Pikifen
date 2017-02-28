/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
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
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "geometry_utils.h"
#include "mobs/mob_type.h"
#include "mobs/pikmin_type.h"

class mob;
struct mob_gen;

using namespace std;

/* ----------------------------------------------------------------------------
 * Structure with info for the bitmap manager.
 */
struct bmp_info {
    ALLEGRO_BITMAP* b;
    size_t calls;
    bmp_info(ALLEGRO_BITMAP* b = NULL);
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
    map<string, bmp_info> list;
    ALLEGRO_BITMAP* get(const string &name, data_node* node = NULL);
    void detach(const string &name);
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
 * This number is actually the distance SQUARED.
 * It's better to compare two squared distances than square-rooting them both.
 */
struct dist {
private:
    float distance_squared;
    float normal_distance;
    bool has_normal_distance;
    
public:
    dist(const point p1, const point p2);
    dist(const float d = 0.0f);
    float to_float();
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
};



/* ----------------------------------------------------------------------------
 * Info about a mob category.
 */
struct mob_category_info {
    string plural_name;
    string singular_name;
    string folder;
    ALLEGRO_COLOR editor_color;
    //Dumps the name of all mob types names in the category onto the vector.
    function<void (vector<string> &list)> lister;
    //Returns a pointer to the mob type of the given name.
    function<mob_type* (const string &name)> type_getter;
    //Creates a new mob type of this category.
    function<mob_type* ()> type_constructor;
    //Saves a mob from this type onto its corresponding vector.
    function<void (mob_type*)> type_saver;
    
};

/* ----------------------------------------------------------------------------
 * A list of the different mob categories.
 * The MOB_CATEGORY_* constants are meant to be used here.
 * Read the sector type manager's comments for more info.
 */
struct mob_category_manager {
private:
    vector<mob_category_info> categories;
    
public:
    void register_category(
        unsigned char nr,
        string pname, string sname, string folder,
        ALLEGRO_COLOR editor_color,
        function<void (vector<string> &list)> lister,
        function<mob_type* (const string &name)> type_getter,
        function<mob_type* ()> type_constructor,
        function<void (mob_type*)> type_saver
    );
    unsigned char get_nr_from_pname(const string &pname);
    unsigned char get_nr_from_sname(const string &sname);
    string get_pname(const unsigned char cat_nr);
    string get_sname(const unsigned char cat_nr);
    ALLEGRO_COLOR get_editor_color(const unsigned char cat_nr);
    unsigned char get_nr_of_categories();
    void get_list(vector<string> &l, unsigned char cat_nr);
    string get_folder(const unsigned char cat_nr);
    void set_mob_type_ptr(mob_gen* m, const string &type_name);
    mob_type* create_mob_type(const unsigned char cat_nr);
    void save_mob_type(const unsigned char cat_nr, mob_type* mt);
    
};



/* ----------------------------------------------------------------------------
 * This structure holds information about how to move something
 * that is user-controlled. It contains the amount of movement
 * on each of the four main directions, ranging from 0 to 1.
 */
struct movement_struct {
    float right;
    float up;
    float left;
    float down;
    
    movement_struct();
    float get_intensity();
    point get_coords();
};



/* ----------------------------------------------------------------------------
 * Group spots. The way this works is that a Pikmin group
 * surrounds a central point. There are several wheels surrounding
 * the original spot, starting from the center and growing in size,
 * each with several spots of their own.
 * A Pikmin occupies the central spot first.
 * Then other Pikmin come by, and occupy spots at random on the next wheel.
 * When that wheel has all of its spots full,
 * the next wheel will be used, and so on.
 */
struct group_spot_info {

    float spot_radius;
    
    vector<vector<point> > coords;
    unsigned n_wheels;
    
    vector<vector<mob*> > mobs_in_spots;
    unsigned current_wheel;
    unsigned n_current_wheel_members;
    
    group_spot_info(const unsigned max_mobs, const float spot_size);
    void add(mob* m);
    void remove(mob* m);
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
    void register_type(unsigned char nr, string name);
    unsigned char get_nr(const string &name);
    string get_name(const unsigned char nr);
    unsigned char get_nr_of_types();
    
};



/* ----------------------------------------------------------------------------
 * A timer. You can set it to start at a pre-determined time, to tick, etc.
 */
struct timer {
    float time_left; //How much time is left until 0.
    float duration;  //When the timer starts, its time is set to this.
    function<void()> on_end;
    
    timer(const float duration, const function<void()> &on_end = nullptr);
    ~timer();
    void start(const bool can_restart = true);
    void start(const float new_duration);
    void tick(const float amount);
    float get_ratio_left();
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
    void start_fade(const bool fade_in, function<void()> on_end);
    bool is_fade_in();
    bool is_fading();
    float get_perc_left();
    void tick(const float time);
    void draw();
};



/* ----------------------------------------------------------------------------
 * Holds information about how a sprite should be resized, colored, etc.,
 * over time, right before it is drawn to the screen.
 */
struct sprite_effect_props {
    point translation;
    float rotation;
    point scale;
    ALLEGRO_COLOR tint_color;
    ALLEGRO_COLOR glow_color;
    
    sprite_effect_props();
};



/* ----------------------------------------------------------------------------
 * A full sprite effect. It is made of several properties, and has the
 * information necessary to interpolate the properties' values over time.
 * If an effect only has one keyframe, no interpolations are made.
 */
struct sprite_effect {
private:
    map<float, sprite_effect_props> keyframes;
    float cur_time;
    
public:
    void add_keyframe(const float time, sprite_effect_props props);
    void set_cur_time(const float cur_time);
    sprite_effect_props get_final_properties();
    
    sprite_effect();
};



/* ----------------------------------------------------------------------------
 * Holds several sprite effect structs.
 */
struct sprite_effect_manager {
private:
    vector<sprite_effect> effects;
    
public:
    void add_effect(sprite_effect e);
    sprite_effect_props get_final_properties();
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
