#ifndef ANIMATION_INCLUDED
#define ANIMATION_INCLUDED

#include <iostream>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "hitbox.h"

using namespace std;

//Frame of animation; a sprite.
class frame {
public:
    ALLEGRO_BITMAP* parent_bmp; //Parent bitmap, normally a spritesheet.
    string file;                //Filename where the image is at.
    int file_x, file_y;         //Top-left corner of the sprite inside the image file.
    int file_w, file_h;         //Size of the sprite inside the image file.
    float offs_x, offs_y;       //Offset. Move the sprite left/right/up/down to align with the previous frames and such.
    ALLEGRO_BITMAP* bitmap;     //Actual bitmap. This is a sub-bitmap of parent_bmp.
    vector<hitbox> hitboxes;    //List of hitboxes on this frame.
    float duration;             //How long the frame lasts for, in seconds.
    float game_w, game_h;       //In-game size of the sprite.
    
    frame(ALLEGRO_BITMAP* b = NULL, float gw = 0, float gh = 0, float d = 0, vector<hitbox> h = vector<hitbox>()) {
        bitmap = b;
        duration = d;
        game_w = gw;
        game_h = gh;
        hitboxes = h;
        file_x = file_y = file_w = file_h = offs_x = offs_y = 0;
        parent_bmp = NULL;
    }
    frame(ALLEGRO_BITMAP* b, int bx, int by, int bw, int bh, float gw, float gh, float d, vector<hitbox> h) {
        parent_bmp = b;
        bitmap = b ? al_create_sub_bitmap(b, bx, by, bw, bh) : NULL;
        duration = d;
        game_w = gw;
        game_h = gh;
        hitboxes = h;
        file_x = bx; file_y = by;
        file_w = bw; file_h = bh;
        offs_x = offs_y = 0;
    }
    frame(const frame &f2) {
        parent_bmp = f2.parent_bmp;
        bitmap = f2.bitmap;
        hitboxes = f2.hitboxes;
        duration = f2.duration;
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
        frame f = frame(NULL, game_w, game_h, duration, hitboxes);
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



//A list of frames, basically.
class animation {
public:
    vector<frame> frames; //List of frames.
    size_t loop_frame;    //The animation loops back to this frame when it reaches the end.
    
    animation(vector<frame> frames = vector<frame>(), size_t loop_frame = 0) {
        this->frames = frames;
        this->loop_frame = loop_frame;
    }
    animation(const animation &a2) {
        frames = a2.frames;
        loop_frame = a2.loop_frame;
    }
};



//Instance of a running animation. This can be played and such.
class animation_instance {
public:
    animation* anim;
    float cur_frame_time;  //Time passed on the current frame.
    size_t cur_frame_nr;
    
    animation_instance(animation* anim = NULL) {
        this->anim = anim;
        start();
    }
    animation_instance(const animation_instance &ai2) {
        anim = ai2.anim;
        start();
    }
    
    void start() { //Starts or restarts an animation. It's called when the animation is created.
        cur_frame_time = 0;
        cur_frame_nr = 0;
    }
    void tick(float time) { //Ticks the animation.
        size_t n_frames = anim->frames.size();
        if(n_frames == 0) return;
        frame* cur_frame = &anim->frames[cur_frame_nr];
        if(cur_frame->duration == 0) return;
        
        cur_frame_time += time;
        
        //This is a while instead of an if because if the framerate is too low and the next frame's duration
        //is too short, it could be that a tick goes over an entire frame, and lands 2 frames ahead.
        while(cur_frame_time > cur_frame->duration) {
            cur_frame_time = cur_frame_time - cur_frame->duration;
            if(cur_frame_nr >= anim->frames.size()) {
                cur_frame_nr = (anim->loop_frame >= anim->frames.size()) ? 0 : anim->loop_frame;
            }
            cur_frame = &anim->frames[cur_frame_nr];
        }
    }
    inline frame* get_frame() { //Gets a pointer to the current frame.
        if(anim->frames.size() == 0) return NULL;
        return &anim->frames[cur_frame_nr];
    }
};

#endif //ifndef ANIMATION_INCLUDED