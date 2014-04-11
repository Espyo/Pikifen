#ifndef ANIMATION_INCLUDED
#define ANIMATION_INCLUDED

#include <iostream>
#include <map>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "hitbox.h"

using namespace std;

class animation;
class frame;
class hitbox;

//A set of animations and their necessary data.
class animation_set {
public:
    map<string, animation> animations;
    map<string, frame> frames;
    map<string, hitbox> hitboxes;
    
    animation_set(
        map<string, animation> a = map<string, animation>(),
        map<string, frame> f = map<string, frame>(),
        map<string, hitbox> h = map<string, hitbox>()) {
        
        animations = a;
        frames = f;
        hitboxes = h;
    }
};

//Frame of animation; a sprite.
class frame {
public:
    string name;
    ALLEGRO_BITMAP* parent_bmp;       //Parent bitmap, normally a spritesheet.
    string file;                      //Filename where the image is at.
    int file_x, file_y;               //Top-left corner of the sprite inside the image file.
    int file_w, file_h;               //Size of the sprite inside the image file.
    float game_w, game_h;             //In-game size of the sprite.
    float offs_x, offs_y;             //Offset. Move the sprite left/right/up/down to align with the previous frames and such.
    ALLEGRO_BITMAP* bitmap;           //Actual bitmap. This is a sub-bitmap of parent_bmp.
    vector<hitbox_instance> hitbox_instances; //List of hitboxes on this frame.
    
    frame(string name = "", ALLEGRO_BITMAP* b = NULL, float gw = 0, float gh = 0, vector<hitbox_instance> h = vector<hitbox_instance>()) {
        this->name = name;
        bitmap = b;
        game_w = gw;
        game_h = gh;
        hitbox_instances = h;
        file_x = file_y = file_w = file_h = offs_x = offs_y = 0;
        parent_bmp = NULL;
    }
    frame(string name, ALLEGRO_BITMAP* b, int bx, int by, int bw, int bh, float gw, float gh, vector<hitbox_instance> h) {
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
    frame(const frame &f2) {
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
    frame clone() {
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
};



//Instance of a frame inside an animation.
//A single frame can appear multiple times in the same animation (imagine an enemy shaking back and forth).
class frame_instance {
public:
    string frame_name;
    float duration; //How long this frame lasts for, in seconds.
    
    frame_instance(string fn = "", float d = 0) {
        frame_name = fn;
        duration = d;
    }
};



//A list of frames, basically.
class animation {
public:
    string name;
    vector<frame_instance> frame_instances; //List of frames.
    size_t loop_frame;    //The animation loops back to this frame when it reaches the end.
    
    animation(string name = "", vector<frame_instance> frame_instances = vector<frame_instance>(), size_t loop_frame = 0) {
        this->name = name;
        this->frame_instances = frame_instances;
        this->loop_frame = loop_frame;
    }
    animation(const animation &a2) {
        name = a2.name;
        frame_instances = a2.frame_instances;
        loop_frame = a2.loop_frame;
    }
};


//Instance of a running animation. This can be played and such.
class animation_instance {
public:
    animation* anim;
    animation_set* anim_set;
    float cur_frame_time;  //Time passed on the current frame.
    size_t cur_frame_nr;
    
    animation_instance(animation* anim = NULL, animation_set* anim_set = NULL) {
        this->anim = anim;
        this->anim_set = anim_set;
        start();
    }
    animation_instance(const animation_instance &ai2) {
        anim = ai2.anim;
        anim_set = ai2.anim_set;
        start();
    }
    
    void start() { //Starts or restarts an animation. It's called when the animation is created.
        cur_frame_time = 0;
        cur_frame_nr = 0;
    }
    bool tick(float time) { //Ticks the animation. Returns whether or not the animation reached its final frame.
        if(!anim) return false;
        size_t n_frames = anim->frame_instances.size();
        if(n_frames <= 1) return false;
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
    inline frame* get_frame() { //Gets a pointer to the current frame.
        if(!anim) return NULL;
        if(anim->frame_instances.size() == 0) return NULL;
        return &anim_set->frames[anim->frame_instances[cur_frame_nr].frame_name];
    }
};

#endif //ifndef ANIMATION_INCLUDED