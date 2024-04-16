/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the animation-related classes and functions.
 */

#pragma once

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "const.h"
#include "hitbox.h"
#include "libs/data_file.h"


using std::size_t;
using std::string;
using std::vector;

class animation_database;
class mob_type;


/*
 * Animations work as follows:
 * An animation is a set of frames.
 * A frame contains hitboxes.
 *
 * A hitbox (hitbox.h) is defined by a body part,
 * a type of hitbox (can be hurt, or hurts other mobs),
 * and some other properties, like position.
 *
 * A frame in an animation is defined by a sprite
 * in a spritesheet, as well as its duration.
 *
 * Finally, an animation contains a list of frames,
 * and the loop frame, which is the one the
 * animation goes back to when it reaches
 * the end.
 *
 * To get a mob to display an animation,
 * you need to create an animation INSTANCE.
 * This can be played, rewinded, etc., and
 * every mob may have a different animation
 * instance, with a different progress time and such.
 *
 * In order for all the different classes
 * to connect, they're referred to using
 * pointers. The animation database holds all of
 * this data so they know where each other is.
 */


/**
 * @brief A sprite in a spritesheet.
 */
class sprite {

public:

    //--- Members ---
    
    //Name of the sprite.
    string name;
    
    //Parent bitmap, normally a spritesheet.
    ALLEGRO_BITMAP* parent_bmp = nullptr;
    
    //File name where the parent bitmap is at.
    string file;
    
    //Top-left corner of the sprite inside the image file.
    point file_pos;
    
    //Size of the sprite inside the image file.
    point file_size;
    
    //Offset. Move the sprite left/right/up/down to align with
    //the previous frames and such.
    point offset;
    
    //Scale multiplier.
    point scale = point(1.0, 1.0);
    
    //Angle to rotate the image by.
    float angle = 0.0f;
    
    //Tint the image with this color.
    ALLEGRO_COLOR tint = COLOR_WHITE;
    
    //X&Y of the Pikmin's top (left/bud/flower).
    point top_pos;
    
    //W&H of the Pikmin's top.
    point top_size = point(5.5, 10);
    
    //Angle of the Pikmin's top.
    float top_angle = 0.0f;
    
    //Does this sprite even have a visible Pikmin top?
    bool top_visible = true;
    
    //The sprite's actual bitmap. This is a sub-bitmap of parent_bmp.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //List of hitboxes on this frame.
    vector<hitbox> hitboxes;
    
    
    //--- Function declarations ---
    
    explicit sprite(
        const string &name = "", ALLEGRO_BITMAP* const b = nullptr,
        const vector<hitbox> &h = vector<hitbox>()
    );
    sprite(
        const string &name, ALLEGRO_BITMAP* const b, const point &b_pos,
        const point &b_size, const vector<hitbox> &h
    );
    sprite(const sprite &s2);
    ~sprite();
    sprite &operator=(const sprite &s2);
    void create_hitboxes(
        animation_database* const adb,
        const float height = 0, const float radius = 0
    );
    void set_bitmap(
        const string &new_file_name,
        const point &new_file_pos, const point &new_file_size,
        data_node* node = nullptr
    );
    
};


/**
 * @brief A frame inside an animation.
 * A single sprite can appear multiple times in the same animation
 * (imagine an enemy shaking back and forth).
 */
class frame {

public:

    //--- Members ---
    
    //Name of the sprite to use in this frame.
    string sprite_name;
    
    //Index of the sprite. Cache for performance.
    size_t sprite_idx = INVALID;
    
    //Pointer to the sprite. Cache for performance.
    sprite* sprite_ptr = nullptr;
    
    //How long this frame lasts for, in seconds.
    float duration = 0.0f;
    
    //Interpolate transformation data between this frame and the next.
    bool interpolate = false;
    
    //Sound to play, if any. This is a sound info block in the mob's data.
    string sound;
    
    //Index of the sound to play, or INVALID. Cache for performance.
    size_t sound_idx = INVALID;
    
    //Signal to send, if any. INVALID = none.
    size_t signal = INVALID;
    
    
    //--- Function declarations ---
    
    explicit frame(
        const string &sn = "", const size_t si = INVALID,
        sprite* sp = nullptr, const float d = 0.1,
        const bool in = false, const string &snd = "", const size_t s = INVALID
    );
    
};


/**
 * @brief An animation. A list of frames, basically.
 */
class animation {

public:

    //--- Members ---
    
    //Name of the animation.
    string name;
    
    //List of frames.
    vector<frame> frames;
    
    //The animation loops back to this frame index when it reaches the end.
    size_t loop_frame = 0;
    
    //If this animation represents an attack that can miss,
    //this represents the successful hit rate.
    //100 means it cannot miss and/or is a normal animation.
    unsigned char hit_rate = 100;
    
    
    //--- Function declarations ---
    
    explicit animation(
        const string &name = "",
        const vector<frame> &frames = vector<frame>(),
        const size_t loop_frame = 0, const unsigned char hit_rate = 100
    );
    animation(const animation &a2);
    animation &operator=(const animation &a2);
    float get_duration();
    float get_loop_duration();
    void get_frame_and_time(
        const float t, size_t* frame_idx, float* frame_time
    );
    float get_time(const size_t frame_idx, const float frame_time);
    
};


/**
 * @brief A database of animations, sprites, and body parts.
 *
 * Basically, an animation file.
 */
class animation_database {

public:

    //--- Members ---
    
    //List of known animations.
    vector<animation*> animations;
    
    //List of known sprites.
    vector<sprite*> sprites;
    
    //List of known body parts.
    vector<body_part*> body_parts;
    
    //Conversion between pre-named animations and in-file animations.
    vector<size_t> pre_named_conversions;
    
    //Version of the engine this animation database was built in.
    string engine_version;
    
    //Maximum span of the hitboxes. Cache for performance.
    float max_span = 0.0f;
    
    
    //--- Function declarations ---
    
    explicit animation_database(
        const vector<animation*> &a = vector<animation*>(),
        const vector<sprite*>    &s = vector<sprite*>(),
        const vector<body_part*> &b = vector<body_part*>()
    );
    size_t find_animation(const string &name) const;
    size_t find_sprite(   const string &name) const;
    size_t find_body_part(const string &name) const;
    void calculate_max_span();
    void create_conversions(
        vector<std::pair<size_t, string> > conversions, const data_node* file
    );
    void fill_sound_idx_caches(mob_type* mt_ptr);
    void fix_body_part_pointers();
    void sort_alphabetically();
    void destroy();
    
};


/**
 * @brief Instance of a running animation. This can be played, rewinded, etc.
 */
class animation_instance {

public:

    //--- Members ---
    
    //The animation currently running.
    animation* cur_anim = nullptr;
    
    //The database this belongs to.
    animation_database* anim_db = nullptr;
    
    //Time passed on the current frame.
    float cur_frame_time = 0.0f;
    
    //Index of the current frame of animation, or INVALID for none.
    size_t cur_frame_idx = INVALID;
    
    
    //--- Function declarations ---
    
    explicit animation_instance(animation_database* anim_db = nullptr);
    animation_instance(const animation_instance &ai2);
    animation_instance &operator=(const animation_instance &ai2);
    void clear();
    void to_start();
    void skip_ahead_randomly();
    bool tick(
        const float delta_t,
        vector<size_t>* signals = nullptr,
        vector<size_t>* sounds = nullptr
    );
    bool valid_frame() const;
    void get_sprite_data(
        sprite** out_cur_sprite_ptr, sprite** out_next_sprite_ptr,
        float* out_interpolation_factor
    ) const;
    size_t get_next_frame_idx(bool* out_reached_end = nullptr) const;
    
};


/**
 * @brief An animation_database and an animation_instance.
 */
struct single_animation_suite {

    //--- Members ---
    
    //Animation database.
    animation_database database;
    
    //Animation instance.
    animation_instance instance;
    
};



void get_sprite_basic_effects(
    const point &base_pos, float base_angle,
    float base_angle_cos_cache, float base_angle_sin_cache,
    sprite* cur_s_ptr, sprite* next_s_ptr, float interpolation_factor,
    point* out_eff_trans, float* out_eff_angle,
    point* out_eff_scale, ALLEGRO_COLOR* out_eff_tint
);
animation_database load_animation_database_from_file(data_node* frames_node);
void get_sprite_basic_top_effects(
    sprite* cur_s_ptr, sprite* next_s_ptr, float interpolation_factor,
    point* out_eff_trans, float* out_eff_angle,
    point* out_eff_size
);
