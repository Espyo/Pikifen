#ifndef ANIMATION_INCLUDED
#define ANIMATION_INCLUDED

#include <iostream>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "hitbox.h"

using namespace std;

class frame {
public:
    ALLEGRO_BITMAP* bitmap;
    vector<hitbox> hitboxes;
    float duration;
    float final_w, final_h; //In-game size of the sprite.
    
    frame(ALLEGRO_BITMAP* b = NULL, float fw = 0, float fh = 0, float d = 0, vector<hitbox> h = vector<hitbox>()) {
        bitmap = b;
        duration = d;
        final_w = fw;
        final_h = fh;
        hitboxes = h;
    }
    frame(ALLEGRO_BITMAP* b, int x1, int y1, int x2, int y2, float fw, float fh, float d, vector<hitbox> h) {
        bitmap = al_create_sub_bitmap(b, x1, y1, x2 - x1, y2 - y1);
        duration = d;
        final_w = fw;
        final_h = fh;
        hitboxes = h;
    }
    frame(const frame &f2) {
        bitmap = f2.bitmap;
        hitboxes = f2.hitboxes;
        duration = f2.duration;
        final_w = f2.final_w;
        final_h = f2.final_h;
    }
};

class animation {
private:
    //Helpers to save on CPU usage.
    size_t cur_frame_nr; //Used when switching to the next frame.
    size_t n_frames; //Used instead of frames.size() (a function call).
    
public:
    vector<frame>* frames; //List of frames.
    float cur_frame_time;  //Time passed on the current frame.
    
    animation(vector<frame>* frames) {
        n_frames = frames->size();
        this->frames = frames;
        
        start();
    }
    animation(const animation &a2) {
        n_frames = a2.frames->size();
        frames = a2.frames;
        
        start();
    }
    animation() {
        n_frames = 0;
        
        start();
    }
    
    void start() { //Starts or restarts an animation. It's called when the animation is created.
        cur_frame_time = 0;
        cur_frame_nr = 0;
    }
    void tick(float time) { //Ticks the animation.
        frame* cur_frame = &frames->at(cur_frame_nr);
        if(cur_frame->duration == 0) return;
        
        cur_frame_time += time;
        
        //This is a while instead of an if because if the framerate is too low and the next frame's duration
        //is too short, it could be that a tick goes over an entire frame, and lands 2 frames ahead.
        while(cur_frame_time > cur_frame->duration) {
            cur_frame_time = cur_frame_time - cur_frame->duration;
            cur_frame_nr = (cur_frame_nr + 1) & n_frames;
            cur_frame = &frames->at(cur_frame_nr);
        }
    }
    inline frame* get_frame() { //Gets a pointer to the current frame.
        if(n_frames == 0) return NULL;
        return &frames->at(cur_frame_nr);
    }
};

#endif //ifndef ANIMATION_INCLUDED