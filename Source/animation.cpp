#include <map>
#include <vector>

#include "animation.h"

using namespace std;

frame::frame(string name, ALLEGRO_BITMAP* b, float gw, float gh, vector<hitbox_instance> h) {
    this->name = name;
    bitmap = b;
    game_w = gw;
    game_h = gh;
    hitbox_instances = h;
    file_x = file_y = file_w = file_h = offs_x = offs_y = 0;
    parent_bmp = NULL;
}

frame::frame(string name, ALLEGRO_BITMAP* b, int bx, int by, int bw, int bh, float gw, float gh, vector<hitbox_instance> h) {
    this->name = name;
    parent_bmp = b;
    bitmap = b ? al_create_sub_bitmap(b, bx, by, bw, bh) : NULL;
    game_w = gw;
    game_h = gh;
    hitbox_instances = h;
    file_x = bx; file_y = by;
    file_w = bw; file_h = bh;
    offs_x = offs_y = 0;
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
    f.parent_bmp = al_clone_bitmap(parent_bmp);
    f.bitmap = al_create_sub_bitmap(f.parent_bmp, f.file_x, f.file_y, f.file_w, f.file_h);
    return f;
}

frame_instance::frame_instance(string fn, float d) {
    frame_name = fn;
    duration = d;
}

animation::animation(string name, vector<frame_instance> frame_instances, size_t loop_frame) {
    this->name = name;
    this->frame_instances = frame_instances;
    this->loop_frame = loop_frame;
}

animation::animation(const animation &a2) {
    name = a2.name;
    frame_instances = a2.frame_instances;
    loop_frame = a2.loop_frame;
}

animation_instance::animation_instance(animation* anim, animation_set* anim_set) {
    this->anim = anim;
    this->anim_set = anim_set;
    start();
}

animation_instance::animation_instance(const animation_instance &ai2) {
    anim = ai2.anim;
    anim_set = ai2.anim_set;
    start();
}

void animation_instance::start() { //Starts or restarts an animation. It's called when the animation is created.
    cur_frame_time = 0;
    cur_frame_nr = 0;
}

bool animation_instance::tick(float time) { //Ticks the animation. Returns whether or not the animation reached its final frame.
    if(!anim) return false;
    size_t n_frames = anim->frame_instances.size();
    if(n_frames == 0) return false;
    frame_instance* cur_frame = &anim->frame_instances[cur_frame_nr];
    if(cur_frame->duration == 0) return false;
    
    bool finished = false;
    cur_frame_time += time;
    
    //This is a while instead of an if because if the framerate is too low and the next frame's duration
    //is too short, it could be that a tick goes over an entire frame, and lands 2 or more frames ahead.
    while(cur_frame_time > cur_frame->duration && cur_frame->duration != 0) {
        cur_frame_time = cur_frame_time - cur_frame->duration;
        cur_frame_nr++;
        if(cur_frame_nr >= anim->frame_instances.size()) {
            finished = true;
            cur_frame_nr = (anim->loop_frame >= anim->frame_instances.size()) ? 0 : anim->loop_frame;
        }
        cur_frame = &anim->frame_instances[cur_frame_nr];
    }
    
    return finished;
}

frame* animation_instance::get_frame() { //Gets a pointer to the current frame.
    if(!anim) return NULL;
    if(anim->frame_instances.size() == 0) return NULL;
    return &anim_set->frames[anim->frame_instances[cur_frame_nr].frame_name];
}

animation_set::animation_set(
    map<string, animation> a,
    map<string, frame> f,
    map<string, hitbox> h
) {

    animations = a;
    frames = f;
    hitboxes = h;
}