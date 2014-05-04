#include <map>
#include <vector>

#include "animation.h"
#include "vars.h"

using namespace std;

frame::frame(const string &name, ALLEGRO_BITMAP* const b, const float gw, const float gh, const vector<hitbox_instance> &h) {
    this->name = name;
    bitmap = b;
    game_w = gw;
    game_h = gh;
    hitbox_instances = h;
    file_x = file_y = file_w = file_h = offs_x = offs_y = 0;
    top_visible = true;
    top_x = top_y = top_angle = 0;
    top_w = top_h = 32;
    parent_bmp = NULL;
}

frame::frame(const string &name, ALLEGRO_BITMAP* const b, const int bx, const int by, const int bw, const int bh, const float gw, const float gh, const vector<hitbox_instance> &h) {
    this->name = name;
    parent_bmp = b;
    bitmap = b ? al_create_sub_bitmap(b, bx, by, bw, bh) : NULL;
    game_w = gw;
    game_h = gh;
    hitbox_instances = h;
    file_x = bx; file_y = by;
    file_w = bw; file_h = bh;
    offs_x = offs_y = 0;
    top_visible = true;
    top_x = top_y = top_angle = 0;
    top_w = top_h = 32;
}

frame::frame(const frame &f2) {
    name = f2.name;
    parent_bmp = f2.parent_bmp;
    bitmap = f2.bitmap;
    hitbox_instances = f2.hitbox_instances;
    game_w = f2.game_w;
    game_h = f2.game_h;
    file = f2.file;
    file_x = f2.file_x;
    file_y = f2.file_y;
    file_w = f2.file_w;
    file_h = f2.file_h;
    offs_x = f2.offs_x;
    offs_y = f2.offs_y;
    top_visible = f2.top_visible;
    top_x = f2.top_x;
    top_y = f2.top_y;
    top_w = f2.top_w;
    top_h = f2.top_h;
    top_angle = f2.top_angle;
}

frame frame::clone() {
    //ToDo hitbox cloning?
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
    return f;
}

frame::~frame() {
    if(parent_bmp) bitmaps.detach(file);
    if(bitmap) al_destroy_bitmap(bitmap);
}

frame_instance::frame_instance(const string &fn, const size_t fnr, frame* fp, const float d) {
    frame_name = fn;
    frame_nr = fnr;
    frame_ptr = fp;
    duration = d;
}

animation::animation(const string &name, vector<frame_instance> frame_instances, const size_t loop_frame) {
    this->name = name;
    this->frame_instances = frame_instances;
    this->loop_frame = loop_frame;
}

animation::animation(const animation &a2) {
    name = a2.name;
    frame_instances = a2.frame_instances;
    loop_frame = a2.loop_frame;
}

animation_instance::animation_instance(animation_set* anim_set) {
    anim = NULL;
    this->anim_set = anim_set;
}

animation_instance::animation_instance(const animation_instance &ai2) {
    anim = ai2.anim;
    anim_set = ai2.anim_set;
    start();
}

/*
 * Changes to a new animation within the same animation set.
 * new_anim_nr: Number of the new animation. Check the next parameter.
 * pre_named:
   * If true, the previous argument is the hardcoded ID (for pre-named animations).
   * For this, the "conversions" must be created. This normally happens in load_mob_types().
   * If false, this is the number on the animation file (don't worry, the enemy script knows the numbers).
 * only_if_new: Only change to this animation if we're not already in it.
 * only_if_done: Only change to this animation if the previous one looped at least once.
 */
void animation_instance::change(const size_t new_anim_nr, const bool pre_named, const bool only_if_new, const bool only_if_done) {
    size_t final_nr;
    if(pre_named) {
        if(anim_set->pre_named_conversions.size() <= new_anim_nr) return;
        final_nr = anim_set->pre_named_conversions[new_anim_nr];
    } else {
        final_nr = new_anim_nr;
    }
    
    if(final_nr == string::npos) return;
    
    animation* new_anim = anim_set->animations[final_nr];
    if(only_if_new && anim == new_anim) return;
    if(only_if_done && !done_once) return;
    
    anim = new_anim;
    start();
}

void animation_instance::start() { //Starts or restarts an animation. It's called when the animation is created.
    cur_frame_time = 0;
    cur_frame_nr = 0;
    done_once = false;
}

bool animation_instance::tick(const float time) { //Ticks the animation. Returns whether or not the animation ended its final frame.
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
        if(cur_frame_nr >= anim->frame_instances.size()) {
            done_once = true;
            cur_frame_nr = (anim->loop_frame >= anim->frame_instances.size()) ? 0 : anim->loop_frame;
        }
        cur_frame = &anim->frame_instances[cur_frame_nr];
    }
    
    return done_once;
}

frame* animation_instance::get_frame() { //Gets a pointer to the current frame.
    if(!anim) return NULL;
    if(anim->frame_instances.size() == 0) return NULL;
    return anim->frame_instances[cur_frame_nr].frame_ptr;
}

animation_set::animation_set(
    vector<animation*> a,
    vector<frame*> f,
    vector<hitbox*> h
) {

    animations = a;
    frames = f;
    hitboxes = h;
}

size_t animation_set::find_animation(string name) {
    for(size_t a = 0; a < animations.size(); a++) {
        if(animations[a]->name == name) return a;
    }
    return string::npos;
}

size_t animation_set::find_frame(string name) {
    for(size_t f = 0; f < frames.size(); f++) {
        if(frames[f]->name == name) return f;
    }
    return string::npos;
}

size_t animation_set::find_hitbox(string name) {
    for(size_t h = 0; h < hitboxes.size(); h++) {
        if(hitboxes[h]->name == name) return h;
    }
    return string::npos;
}

/*
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
void animation_set::create_conversions(vector<pair<size_t, string> > conversions) {
    pre_named_conversions.clear();
    
    if(conversions.size() == 0) return;
    
    //First, find the highest number.
    size_t highest = conversions[0].first;
    for(size_t c = 1; c < conversions.size(); c++) {
        highest = max(highest, conversions[c].first);
    }
    
    pre_named_conversions.assign(highest + 1, string::npos);
    
    for(size_t c = 0; c < conversions.size(); c++) {
        size_t a_pos = find_animation(conversions[c].second);
        pre_named_conversions[conversions[c].first] = a_pos;
    }
}

void animation_set::destroy() {
    for(auto a = animations.begin(); a != animations.end(); a++) {
        delete *a;
    }
    for(auto f = frames.begin(); f != frames.end(); f++) {
        delete *f;
    }
    for(auto h = hitboxes.begin(); h != hitboxes.end(); h++) {
        delete *h;
    }
}