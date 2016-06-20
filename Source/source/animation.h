/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
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

class animation_pool;

/* ----------------------------------------------------------------------------
 * Animations work as follows:
 * An animation links to frames.
 * A frame links to hitboxes.
 *
 * A hitbox (hitbox.h) specifies a spot
 * in which the mob is attacking, or a
 * spot in which it can be attacked.
 * A single hitbox can have several attributes,
 * and a frame refers to INSTANCES of hitboxes.
 *
 * A frame is basically a sprite.
 * It gathers its appearance from an image file,
 * with some tweaks, and the changes of frames
 * in moderately quick succession is what
 * creates an animation, as it's always been,
 * historically.
 * An animation refers to INSTANCES of frames,
 * in whichever order it wants.
 *
 * Finally, an animation contains data for
 * the loop frame, which is the frame the
 * animation goes back to when it reaches
 * the end.
 *
 * To get a mob to display an animation,
 * you need to create an animation instance.
 * This can be played, rewinded, etc., and
 * every mob may have a different animation
 * instance, with a different progress time and such.
 *
 * In order for the animations, frames and
 * hitboxes to connect, they're referred to using
 * pointers. The animation pool holds all of this data
 * so the animations, frames and hitboxes know
 * where to communicate with one another.
 */


/* ----------------------------------------------------------------------------
 * A frame of animation; a sprite.
 */
class frame {
private:
    void calculate_hitbox_span();
    
public:
    string name;
    //Parent bitmap, normally a spritesheet.
    ALLEGRO_BITMAP* parent_bmp;
    //File name where the image is at.
    string file;
    //Top-left corner of the sprite inside the image file.
    int file_x, file_y;
    //Size of the sprite inside the image file.
    int file_w, file_h;
    //In-game size of the sprite.
    float game_w, game_h;
    //Offset. Move the sprite left/right/up/down to align with
    //the previous frames and such.
    float offs_x, offs_y;
    //X&Y of the Pikmin's top (left/bud/flower).
    float top_x, top_y;
    //W&H of the Pikmin's top.
    float top_w, top_h;
    //Angle of the Pikmin's top.
    float top_angle;
    //Does this frame even have a visible Pikmin top?
    bool top_visible;
    //Actual bitmap. This is a sub-bitmap of parent_bmp.
    ALLEGRO_BITMAP* bitmap;
    //List of hitboxes on this frame.
    vector<hitbox_instance> hitbox_instances;
    //How far the hitboxes span.
    float hitbox_span;
    
    frame(
        const string &name = "", ALLEGRO_BITMAP* const b = NULL,
        const float gw = 0, const float gh = 0,
        const vector<hitbox_instance> &h = vector<hitbox_instance>()
    );
    frame(
        const string &name, ALLEGRO_BITMAP* const b, const int bx, const int by,
        const int bw, const int bh, const float gw, const float gh,
        const vector<hitbox_instance> &h
    );
    frame(const frame &f2);
    void create_hitbox_instances(animation_pool* const as);
    
    ~frame();
};


/* ----------------------------------------------------------------------------
 * Instance of a frame inside an animation.
 * A single frame can appear multiple times in the same animation
 * (imagine an enemy shaking back and forth).
 */
class frame_instance {
public:
    string frame_name;
    size_t frame_nr;  //Needed for performance.
    frame* frame_ptr; //Needed for performance.
    float duration;   //How long this frame lasts for, in seconds.
    
    frame_instance(
        const string &fn = "", const size_t fnr = INVALID,
        frame* fp = NULL, const float d = 0
    );
};


/* ----------------------------------------------------------------------------
 * A list of frames, basically.
 */
class animation {
public:
    string name;
    //List of frames.
    vector<frame_instance> frame_instances;
    //The animation loops back to this frame when it reaches the end.
    size_t loop_frame;
    
    animation(
        const string &name = "",
        vector<frame_instance> frame_instances = vector<frame_instance>(),
        const size_t loop_frame = 0
    );
    animation(const animation &a2);
};


/* ----------------------------------------------------------------------------
 * A set of animations and their necessary data.
 */
class animation_pool {
public:
    vector<animation*> animations;
    vector<frame*> frames;
    vector<hitbox*> hitboxes;
    
    //Conversion between pre-named animations and in-file animations.
    vector<size_t> pre_named_conversions;
    
    animation_pool(
        vector<animation*> a = vector<animation*>(),
        vector<frame*>     f = vector<frame*>(),
        vector<hitbox*>    h = vector<hitbox*>()
    );
    
    size_t find_animation(string name);
    size_t find_frame(    string name);
    size_t find_hitbox(   string name);
    
    void create_conversions(vector<pair<size_t, string> > conversions);
    void fix_hitbox_pointers();
    
    void destroy();
    
};


/* ----------------------------------------------------------------------------
 * Instance of a running animation. This can be played, rewinded, ...
 */
class animation_instance {
public:
    animation* anim;
    animation_pool* anim_pool; //The pool this belongs to.
    float cur_frame_time;      //Time passed on the current frame.
    size_t cur_frame_nr;
    bool done_once;
    
    animation_instance(animation_pool* anim_pool = NULL);
    animation_instance(const animation_instance &ai2);
    
    void start();
    bool tick(const float time);
    frame* get_frame();
};


/* ----------------------------------------------------------------------------
 * An animation pool and an animation_instance.
 */
struct single_animation_suite {
    animation_pool pool;
    animation_instance instance;
};



animation_pool load_animation_pool_from_file(data_node* frames_node);

#endif //ifndef ANIMATION_INCLUDED
