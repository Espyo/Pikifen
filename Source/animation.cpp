/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation, frame, animation instance,
 * frame instance, and animation pool classes,
 * and animation-related functions.
 */

#include <algorithm>
#include <map>
#include <vector>

#include "animation.h"
#include "functions.h"
#include "vars.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Creates a frame of animation, with a pre-existing bitmap.
 * name:   Internal name, should be unique.
 * b:      Bitmap.
 * gw, gh: In-game width and height of the sprite.
 * h:      List of hitbox instances.
 */
frame::frame(const string &name, ALLEGRO_BITMAP* const b, const float gw, const float gh, const vector<hitbox_instance> &h) :
    name(name),
    bitmap(b),
    game_w(gw),
    game_h(gh),
    hitbox_instances(h),
    file_x(0),
    file_y(0),
    file_w(0),
    file_h(0),
    offs_x(0),
    offs_y(0),
    top_visible(true),
    top_x(0),
    top_y(0),
    top_angle(0),
    top_w(32),
    top_h(32),
    parent_bmp(nullptr) {
    
    calculate_hitbox_span();
}


/* ----------------------------------------------------------------------------
 * Creates a frame of animation, and creates its sprite using a parent bitmap and its coordinates.
 * name:   Internal name, should be unique.
 * b:      Parent bitmap.
 * bx, by: X and Y of the top-left corner of the sprite, in the parent's bitmap.
 * bw, bh: Width and height of the sprite, in the parent's bitmap.
 * gw, gh: In-game width and height of the sprite.
 * h:      List of hitbox instances.
 */
frame::frame(const string &name, ALLEGRO_BITMAP* const b, const int bx, const int by, const int bw, const int bh, const float gw, const float gh, const vector<hitbox_instance> &h) :
    name(name),
    parent_bmp(b),
    bitmap(b ? al_create_sub_bitmap(b, bx, by, bw, bh) : nullptr),
    game_w(gw),
    game_h(gh),
    hitbox_instances(h),
    file_x(bx),
    file_y(by),
    file_w(bw),
    file_h(bh),
    offs_x(0),
    offs_y(0),
    top_visible(true),
    top_x(0),
    top_y(0),
    top_angle(0),
    top_w(32),
    top_h(32) {
    
    calculate_hitbox_span();
}


/* ----------------------------------------------------------------------------
 * Creates a frame by copying info from another frame.
 */
frame::frame(const frame &f2) :
    name(f2.name),
    parent_bmp(f2.parent_bmp),
    bitmap(f2.bitmap),
    hitbox_instances(f2.hitbox_instances),
    game_w(f2.game_w),
    game_h(f2.game_h),
    file(f2.file),
    file_x(f2.file_x),
    file_y(f2.file_y),
    file_w(f2.file_w),
    file_h(f2.file_h),
    offs_x(f2.offs_x),
    offs_y(f2.offs_y),
    top_visible(f2.top_visible),
    top_x(f2.top_x),
    top_y(f2.top_y),
    top_w(f2.top_w),
    top_h(f2.top_h),
    top_angle(f2.top_angle),
    hitbox_span(f2.hitbox_span) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a frame by cloning the data from another frame.
 */
frame frame::clone() {
    //TODO should the hitboxes be cloned too?
    frame f = frame(name, NULL, game_w, game_h, hitbox_instances);
    f.file = file;
    f.file_x = file_x;
    f.file_y = file_y;
    f.file_w = file_w;
    f.file_h = file_h;
    f.offs_x = offs_x;
    f.offs_y = offs_y;
    f.top_x = top_x;
    f.top_y = top_y;
    f.top_w = top_w;
    f.top_h = top_h;
    f.top_angle = top_angle;
    f.top_visible = top_visible;
    f.parent_bmp = al_clone_bitmap(parent_bmp);
    f.bitmap = al_create_sub_bitmap(f.parent_bmp, f.file_x, f.file_y, f.file_w, f.file_h);
    f.hitbox_span = hitbox_span;
    return f;
}


/* ----------------------------------------------------------------------------
 * Destroys a frame and its bitmaps.
 */
frame::~frame() {
    if(parent_bmp) bitmaps.detach(file);
    if(bitmap) al_destroy_bitmap(bitmap);
}

/* ----------------------------------------------------------------------------
 * Calculates the span of the hitboxes.
 */
void frame::calculate_hitbox_span() {
    hitbox_span = 0;
    for(size_t hi = 0; hi < hitbox_instances.size(); ++hi) {
        hitbox_instance* hi_ptr = &hitbox_instances[hi];
        
        float d = dist(0, 0, hi_ptr->x, hi_ptr->y).to_float();
        d += hi_ptr->radius;
        hitbox_span = max(hitbox_span, d);
    }
}

/* ----------------------------------------------------------------------------
 * Creates the hitbox instances, based on the hitboxes.
 */
void frame::create_hitbox_instances(animation_pool* const as) {
    hitbox_instances.clear();
    for(size_t h = 0; h < as->hitboxes.size(); ++h) {
        hitbox_instances.push_back(
            hitbox_instance(
                as->hitboxes[h]->name,
                h,
                as->hitboxes[h]
            )
        );
    }
    calculate_hitbox_span();
}


/* ----------------------------------------------------------------------------
 * Creates a frame instance.
 * fn:  Name of the frame.
 * fnr: Numbr of the frame on the animation pool.
 * fp:  Pointer to the frame.
 * d:   Duration.
 */
frame_instance::frame_instance(const string &fn, const size_t fnr, frame* fp, const float d) :
    frame_name(fn),
    frame_nr(fnr),
    frame_ptr(fp),
    duration(d) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an animation.
 * name:            Name, should be unique.
 * frame_instances: List of frame instances.
 * loop_frame:      Loop frame number.
 */
animation::animation(const string &name, vector<frame_instance> frame_instances, const size_t loop_frame) :
    name(name),
    frame_instances(frame_instances),
    loop_frame(loop_frame) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an animation by copying info from another animation.
 */
animation::animation(const animation &a2) :
    name(a2.name),
    frame_instances(a2.frame_instances),
    loop_frame(a2.loop_frame) {
}


/* ----------------------------------------------------------------------------
 * Creates an animation instance.
 * anim_pool: The animation pool. Used when changing animations.
 */
animation_instance::animation_instance(animation_pool* anim_pool) :
    anim_pool(anim_pool),
    anim(nullptr),
    cur_frame_time(0),
    cur_frame_nr(0),
    done_once(false) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an animation instance by copying info from another.
 */
animation_instance::animation_instance(const animation_instance &ai2) :
    anim(ai2.anim),
    anim_pool(ai2.anim_pool) {
    
    start();
}


/* ----------------------------------------------------------------------------
 * Starts or restarts the animation.
 * It's called when the animation is set.
 */
void animation_instance::start() {
    cur_frame_time = 0;
    cur_frame_nr = 0;
    done_once = false;
}


/* ----------------------------------------------------------------------------
 * Ticks the animation with the given amount of time.
 * Returns whether or not the animation ended its final frame.
 */
bool animation_instance::tick(const float time) {
    if(!anim) return false;
    size_t n_frames = anim->frame_instances.size();
    if(n_frames == 0) return false;
    frame_instance* cur_frame = &anim->frame_instances[cur_frame_nr];
    if(cur_frame->duration == 0) { done_once = true; return true; }
    
    cur_frame_time += time;
    
    //This is a while instead of an if because if the framerate is too low and the next frame's duration
    //is too short, it could be that a tick goes over an entire frame, and lands 2 or more frames ahead.
    while(cur_frame_time > cur_frame->duration && cur_frame->duration != 0) {
        cur_frame_time = cur_frame_time - cur_frame->duration;
        cur_frame_nr++;
        if(cur_frame_nr >= n_frames) {
            done_once = true;
            cur_frame_nr = (anim->loop_frame >= n_frames) ? 0 : anim->loop_frame;
        }
        cur_frame = &anim->frame_instances[cur_frame_nr];
    }
    
    return done_once;
}


/* ----------------------------------------------------------------------------
 * Returns the current frame of animation.
 */
frame* animation_instance::get_frame() {
    if(!anim) return NULL;
    if(anim->frame_instances.empty()) return NULL;
    return anim->frame_instances[cur_frame_nr].frame_ptr;
}


/* ----------------------------------------------------------------------------
 * Creates an animation pool.
 */
animation_pool::animation_pool(vector<animation*> a, vector<frame*> f, vector<hitbox*> h) :
    animations(a),
    frames(f),
    hitboxes(h) {
    
}


/* ----------------------------------------------------------------------------
 * Returns the position of the specified animation.
 * Returns string::npos if not found.
 */
size_t animation_pool::find_animation(string name) {
    for(size_t a = 0; a < animations.size(); ++a) {
        if(animations[a]->name == name) return a;
    }
    return string::npos;
}


/* ----------------------------------------------------------------------------
 * Returns the position of the specified frame.
 * Returns string::npos if not found.
 */
size_t animation_pool::find_frame(string name) {
    for(size_t f = 0; f < frames.size(); ++f) {
        if(frames[f]->name == name) return f;
    }
    return string::npos;
}


/* ----------------------------------------------------------------------------
 * Returns the position of the specified hitbox.
 * Returns string::npos if not found.
 */
size_t animation_pool::find_hitbox(string name) {
    for(size_t h = 0; h < hitboxes.size(); ++h) {
        if(hitboxes[h]->name == name) return h;
    }
    return string::npos;
}


/* ----------------------------------------------------------------------------
 * Fixes the pointers for hitboxes on the frames.
 */
void animation_pool::fix_hitbox_pointers() {
    for(size_t f = 0; f < frames.size(); ++f) {
        frame* f_ptr = frames[f];
        for(size_t hi = 0; hi < f_ptr->hitbox_instances.size(); ++hi) {
            hitbox_instance* hi_ptr = &f_ptr->hitbox_instances[hi];
            
            for(size_t h = 0; h < hitboxes.size(); ++h) {
                hitbox* h_ptr = hitboxes[h];
                if(h_ptr->name == hi_ptr->hitbox_name) {
                    hi_ptr->hitbox_nr = h;
                    hi_ptr->hitbox_ptr = h_ptr;
                    break;
                }
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Enemies and such have a regular list of animations.
 * The only way to change these animations is through the script.
 * So animation control is done entirely through game data.
 * However, the animations names for Pikmin, leaders, etc. is pre-named.
 * e.g.: The game wants there to be an "idle" animation, a "walk" animation, etc.
 * Because we are NOT looking up with strings, if we want more than 20FPS,
 * we need a way to convert from a numeric ID (one that stands for walking, one for idling, etc.)
 * into the corresponding number on the animation file.
 * This is where this comes in.
 *
 * conversions: A vector of size_t and strings.
   * The size_t is the hardcoded ID (probably in some constant or enum).
   * The string is the name of the animation in the animation file.
 */
void animation_pool::create_conversions(vector<pair<size_t, string> > conversions) {
    pre_named_conversions.clear();
    
    if(conversions.empty()) return;
    
    //First, find the highest number.
    size_t highest = conversions[0].first;
    for(size_t c = 1; c < conversions.size(); ++c) {
        highest = max(highest, conversions[c].first);
    }
    
    pre_named_conversions.assign(highest + 1, string::npos);
    
    for(size_t c = 0; c < conversions.size(); ++c) {
        size_t a_pos = find_animation(conversions[c].second);
        pre_named_conversions[conversions[c].first] = a_pos;
    }
}


/* ----------------------------------------------------------------------------
 * Destroys an animation pool and all of its animations, frames, and hitboxes.
 */
void animation_pool::destroy() {
    for(auto a = animations.begin(); a != animations.end(); ++a) {
        delete *a;
    }
    for(auto f = frames.begin(); f != frames.end(); ++f) {
        delete *f;
    }
    for(auto h = hitboxes.begin(); h != hitboxes.end(); ++h) {
        delete *h;
    }
}


/* ----------------------------------------------------------------------------
 * Loads the animations from a file.
 */
animation_pool load_animation_pool_from_file(data_node* file_node) {
    animation_pool as;
    
    vector<animation*> animations;
    vector<frame*> frames;
    vector<hitbox*> hitboxes;
    
    //Hitboxes.
    data_node* hitboxes_node = file_node->get_child_by_name("hitboxes");
    size_t n_hitboxes = hitboxes_node->get_nr_of_children();
    for(size_t h = 0; h < n_hitboxes; ++h) {
    
        data_node* hitbox_node = hitboxes_node->get_child(h);
        
        hitbox* cur_hitbox = new hitbox(hitbox_node->name);
        hitboxes.push_back(cur_hitbox);
    }
    
    as.hitboxes = hitboxes;
    
    //Frames.
    data_node* frames_node = file_node->get_child_by_name("frames");
    size_t n_frames = frames_node->get_nr_of_children();
    for(size_t f = 0; f < n_frames; ++f) {
    
        data_node* frame_node = frames_node->get_child(f);
        vector<hitbox_instance> hitbox_instances;
        
        data_node* hitbox_instances_node = frame_node->get_child_by_name("hitbox_instances");
        size_t n_hitbox_instances = hitbox_instances_node->get_nr_of_children();
        
        for(size_t h = 0; h < n_hitbox_instances; ++h) {
        
            data_node* hitbox_instance_node = hitbox_instances_node->get_child(h);
            hitbox_instance cur_hitbox_instance = hitbox_instance();
            
            vector<string> coords = split(hitbox_instance_node->get_child_by_name("coords")->value);
            if(coords.size() >= 3) {
                cur_hitbox_instance.x = s2f(coords[0]);
                cur_hitbox_instance.y = s2f(coords[1]);
                cur_hitbox_instance.z = s2f(coords[2]);
            }
            cur_hitbox_instance.height = s2f(hitbox_instance_node->get_child_by_name("height")->value);
            cur_hitbox_instance.radius = s2f(hitbox_instance_node->get_child_by_name("radius")->value);
            cur_hitbox_instance.hitbox_name = hitbox_instance_node->name;
            cur_hitbox_instance.type = s2i(hitbox_instance_node->get_child_by_name("type")->value);
            cur_hitbox_instance.multiplier = s2f(hitbox_instance_node->get_child_by_name("multiplier")->value);
            cur_hitbox_instance.hazards = hitbox_instance_node->get_child_by_name("elements")->value;
            cur_hitbox_instance.can_pikmin_latch = s2b(hitbox_instance_node->get_child_by_name("can_pikmin_latch")->value);
            cur_hitbox_instance.knockback_outward = s2b(hitbox_instance_node->get_child_by_name("outward")->value);
            cur_hitbox_instance.knockback_angle = s2f(hitbox_instance_node->get_child_by_name("angle")->value);
            cur_hitbox_instance.knockback = s2f(hitbox_instance_node->get_child_by_name("knockback")->value);
            
            hitbox_instances.push_back(cur_hitbox_instance);
            
        }
        
        ALLEGRO_BITMAP* parent = bitmaps.get(frame_node->get_child_by_name("file")->value, frame_node->get_child_by_name("file"));
        frame* new_f =
            new frame(
            frame_node->name,
            parent,
            s2i(frame_node->get_child_by_name("file_x")->value),
            s2i(frame_node->get_child_by_name("file_y")->value),
            s2i(frame_node->get_child_by_name("file_w")->value),
            s2i(frame_node->get_child_by_name("file_h")->value),
            s2f(frame_node->get_child_by_name("game_w")->value),
            s2f(frame_node->get_child_by_name("game_h")->value),
            hitbox_instances
        );
        frames.push_back(new_f);
        
        new_f->file = frame_node->get_child_by_name("file")->value;
        new_f->parent_bmp = parent;
        new_f->offs_x = s2f(frame_node->get_child_by_name("offs_x")->value);
        new_f->offs_y = s2f(frame_node->get_child_by_name("offs_y")->value);
        new_f->top_visible = s2b(frame_node->get_child_by_name("top_visible")->value);
        new_f->top_x = s2f(frame_node->get_child_by_name("top_x")->value);
        new_f->top_y = s2f(frame_node->get_child_by_name("top_y")->value);
        new_f->top_w = s2f(frame_node->get_child_by_name("top_w")->value);
        new_f->top_h = s2f(frame_node->get_child_by_name("top_h")->value);
        new_f->top_angle = s2f(frame_node->get_child_by_name("top_angle")->value);
    }
    
    as.frames = frames;
    
    //Animations.
    data_node* anims_node = file_node->get_child_by_name("animations");
    size_t n_anims = anims_node->get_nr_of_children();
    for(size_t a = 0; a < n_anims; ++a) {
    
        data_node* anim_node = anims_node->get_child(a);
        vector<frame_instance> frame_instances;
        
        data_node* frame_instances_node = anim_node->get_child_by_name("frame_instances");
        size_t n_frame_instances = frame_instances_node->get_nr_of_children();
        
        for(size_t f = 0; f < n_frame_instances; ++f) {
            data_node* frame_instance_node = frame_instances_node->get_child(f);
            size_t f_pos = as.find_frame(frame_instance_node->name);
            frame_instances.push_back(
                frame_instance(
                    frame_instance_node->name,
                    f_pos,
                    (f_pos == string::npos) ? NULL : frames[f_pos],
                    s2f(frame_instance_node->get_child_by_name("duration")->value)
                )
            );
        }
        
        animations.push_back(
            new animation(
                anim_node->name,
                frame_instances,
                s2i(anim_node->get_child_by_name("loop_frame")->value)
            )
        );
    }
    
    as.animations = animations;
    
    return as;
}
