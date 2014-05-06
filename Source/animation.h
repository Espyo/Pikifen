/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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

#include "data_file.h"
#include "hitbox.h"

using namespace std;

/*
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
 * pointers. The animation set holds all of this data
 * so the animations, frames and hitboxes know
 * where to communicate with one another.
 */


/*
 * A frame of animation; a sprite.
 */
class frame {
public:
    string name;
    ALLEGRO_BITMAP* parent_bmp;       //Parent bitmap, normally a spritesheet.
    string file;                      //Filename where the image is at.
    int file_x, file_y;               //Top-left corner of the sprite inside the image file.
    int file_w, file_h;               //Size of the sprite inside the image file.
    float game_w, game_h;             //In-game size of the sprite.
    float offs_x, offs_y;             //Offset. Move the sprite left/right/up/down to align with the previous frames and such.
    float top_x, top_y;               //X&Y of the Pikmin's top (left/bud/flower).
    float top_w, top_h;               //W&H of the Pikmin's top.
    float top_angle;                  //Angle of the Pikmin's top.
    bool top_visible;                 //Does this frame even have a visible Pikmin top?
    ALLEGRO_BITMAP* bitmap;           //Actual bitmap. This is a sub-bitmap of parent_bmp.
    vector<hitbox_instance> hitbox_instances; //List of hitboxes on this frame.
    
    frame(const string &name = "", ALLEGRO_BITMAP* const b = NULL, const float gw = 0, const float gh = 0, const vector<hitbox_instance> &h = vector<hitbox_instance>());
    frame(const string &name, ALLEGRO_BITMAP* const b, const int bx, const int by, const int bw, const int bh, const float gw, const float gh, const vector<hitbox_instance> &h);
    frame(const frame &f2);
    frame clone();
    
    ~frame();
};



/*
 * Instance of a frame inside an animation.
 * A single frame can appear multiple times in the same animation (imagine an enemy shaking back and forth).
 */
class frame_instance {
public:
    string frame_name;
    size_t frame_nr;  //Needed for performance.
    frame* frame_ptr; //Needed for performance.
    float duration;   //How long this frame lasts for, in seconds.
    
    frame_instance(const string &fn = "", const size_t fnr = string::npos, frame* fp = NULL, const float d = 0);
};



/*
 * A list of frames, basically.
 */
class animation {
public:
    string name;
    vector<frame_instance> frame_instances; //List of frames.
    size_t loop_frame;    //The animation loops back to this frame when it reaches the end.
    
    animation(const string &name = "", vector<frame_instance> frame_instances = vector<frame_instance>(), const size_t loop_frame = 0);
    animation(const animation &a2);
};

/*
 * A set of animations and their necessary data.
 */
class animation_set {
public:
    vector<animation*> animations;
    vector<frame*> frames;
    vector<hitbox*> hitboxes;
    
    vector<size_t> pre_named_conversions; //Conversion between pre-named animations and in-file animations.
    
    animation_set(
        vector<animation*> a = vector<animation*>(),
        vector<frame*>     f = vector<frame*>(),
        vector<hitbox*>    h = vector<hitbox*>()
    );
    
    size_t find_animation(string name);
    size_t find_frame(    string name);
    size_t find_hitbox(   string name);
    
    void create_conversions(vector<pair<size_t, string> > conversions);
    
    void destroy();
    
};

/*
 * Instance of a running animation. This can be played, rewinded, ...
 */
class animation_instance {
public:
    animation* anim;
    animation_set* anim_set;
    float cur_frame_time;  //Time passed on the current frame.
    size_t cur_frame_nr;
    bool done_once;
    
    animation_instance(animation_set* anim_set = NULL);
    animation_instance(const animation_instance &ai2);
    
    void change(const size_t new_anim_nr, const bool pre_named, const bool only_if_new, const bool only_if_done);
    void start();
    bool tick(const float time);
    frame* get_frame();
};

animation_set load_animation_set(data_node* frames_node);

#endif //ifndef ANIMATION_INCLUDED