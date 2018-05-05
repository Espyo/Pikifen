/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the animation-related classes and functions.
 */

#ifndef ANIMATION_INCLUDED
#define ANIMATION_INCLUDED

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "const.h"
#include "data_file.h"
#include "hitbox.h"

using namespace std;

class animation_database;

/* ----------------------------------------------------------------------------
 * Animations work as follows:
 * An animation is a set of frames.
 * A frame contains hitboxes.
 *
 * A hitbox (hitbox.h) is defined by a body part,
 * a type of hitbox (can be hurt, or hurts other mobs),
 * and some other parameters, like position.
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


/* ----------------------------------------------------------------------------
 * A sprite in a spritesheet.
 */
class sprite {
private:
    void calculate_hitbox_span();
    
public:
    string name;
    //Parent bitmap, normally a spritesheet.
    ALLEGRO_BITMAP* parent_bmp;
    //File name where the parent bitmap is at.
    string file;
    //Top-left corner of the sprite inside the image file.
    point file_pos;
    //Size of the sprite inside the image file.
    point file_size;
    //In-game size of the sprite.
    point game_size;
    //Offset. Move the sprite left/right/up/down to align with
    //the previous frames and such.
    point offset;
    //X&Y of the Pikmin's top (left/bud/flower).
    point top_pos;
    //W&H of the Pikmin's top.
    point top_size;
    //Angle of the Pikmin's top.
    float top_angle;
    //Does this sprite even have a visible Pikmin top?
    bool top_visible;
    //The sprite's actual bitmap. This is a sub-bitmap of parent_bmp.
    ALLEGRO_BITMAP* bitmap;
    //List of hitboxes on this frame.
    vector<hitbox> hitboxes;
    //How far the hitboxes span from the center.
    float hitbox_span;
    
    sprite(
        const string &name = "", ALLEGRO_BITMAP* const b = NULL,
        const point &g_size = point(),
        const vector<hitbox> &h = vector<hitbox>()
    );
    sprite(
        const string &name, ALLEGRO_BITMAP* const b, const point &b_pos,
        const point &b_size, const point &g_size,
        const vector<hitbox> &h
    );
    sprite(const sprite &s2);
    void create_hitboxes(animation_database* const adb);
    void set_bitmap(
        const string &file_name, const point &file_pos, const point &file_size,
        data_node* node = NULL
    );
    
    ~sprite();
};


/* ----------------------------------------------------------------------------
 * A frame inside an animation.
 * A single sprite can appear multiple times in the same animation
 * (imagine an enemy shaking back and forth).
 */
class frame {
public:
    string sprite_name;
    size_t sprite_index;  //Cache for performance.
    sprite* sprite_ptr;   //Cache for performance.
    float duration;    //How long this frame lasts for, in seconds.
    size_t signal;     //Signal to send, if any. INVALID = none.
    
    frame(
        const string &sn = "", const size_t si = INVALID,
        sprite* sp = NULL, const float d = 0, const size_t s = INVALID
    );
};


/* ----------------------------------------------------------------------------
 * An animation. A list of frames, basically.
 */
class animation {
public:
    string name;
    //List of frames.
    vector<frame> frames;
    //The animation loops back to this frame when it reaches the end.
    size_t loop_frame;
    //If this animation represents an attack that can miss,
    //this represents the successful hit rate.
    //100 means it cannot miss and/or is a normal animation.
    unsigned char hit_rate;
    
    animation(
        const string &name = "",
        const vector<frame> &frames = vector<frame>(),
        const size_t loop_frame = 0, const unsigned char hit_rate = 100
    );
    animation(const animation &a2);
};


/* ----------------------------------------------------------------------------
 * A database of animations, sprites, and body parts.
 */
class animation_database {
public:
    vector<animation*> animations;
    vector<sprite*> sprites;
    vector<body_part*> body_parts;
    
    //Conversion between pre-named animations and in-file animations.
    vector<size_t> pre_named_conversions;
    
    animation_database(
        const vector<animation*> &a = vector<animation*>(),
        const vector<sprite*>    &s = vector<sprite*>(),
        const vector<body_part*> &b = vector<body_part*>()
    );
    
    size_t find_animation(const string &name);
    size_t find_sprite(   const string &name);
    size_t find_body_part(const string &name);
    
    void create_conversions(vector<pair<size_t, string> > conversions);
    void fix_body_part_pointers();
    
    void destroy();
    
};


/* ----------------------------------------------------------------------------
 * Instance of a running animation. This can be played, rewinded, etc.
 */
class animation_instance {
public:
    animation* cur_anim;         //The current animation.
    animation_database* anim_db; //The database this belongs to.
    float cur_frame_time;        //Time passed on the current frame.
    size_t cur_frame_index;
    
    animation_instance(animation_database* anim_db = NULL);
    animation_instance(const animation_instance &ai2);
    
    void start();
    bool tick(const float time, vector<size_t>* signals = NULL);
    sprite* get_cur_sprite();
};


/* ----------------------------------------------------------------------------
 * An animation_database and an animation_instance.
 */
struct single_animation_suite {
    animation_database database;
    animation_instance instance;
};



animation_database load_animation_database_from_file(data_node* frames_node);

#endif //ifndef ANIMATION_INCLUDED
