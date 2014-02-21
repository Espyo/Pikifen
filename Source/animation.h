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
    float game_w, game_h; //In-game size of the sprite.
    
    frame(ALLEGRO_BITMAP* b = NULL, float fw = 0, float fh = 0, float d = 0, vector<hitbox> h = vector<hitbox>()) {
        bitmap = b;
        duration = d;
        game_w = fw;
        game_h = fh;
        hitboxes = h;
    }
    frame(ALLEGRO_BITMAP* b, int bx, int by, int bw, int bh, float fw, float fh, float d, vector<hitbox> h) {
        bitmap = b ? al_create_sub_bitmap(b, bx, by, bw, bh) : NULL;
        duration = d;
        game_w = fw;
        game_h = fh;
        hitboxes = h;
    }
    frame(const frame &f2) {
        bitmap = f2.bitmap;
        hitboxes = f2.hitboxes;
        duration = f2.duration;
        game_w = f2.game_w;
        game_h = f2.game_h;
    }
    frame clone() {
        //ToDo hitbox cloning?
        return frame(al_clone_bitmap(bitmap), game_w, game_h, duration, hitboxes);
    }
};

class animation {
private:
    //Helpers to save on CPU usage.
    size_t cur_frame_nr; //Used when switching to the next frame.
    size_t n_frames; //Used instead of frames.size() (a function call).
    
public:
    vector<frame> frames; //List of frames.
    float cur_frame_time;  //Time passed on the current frame.
    
    animation(vector<frame> frames) {
        n_frames = frames.size();
        this->frames = frames;
        
        start();
    }
    animation(const animation &a2) {
        n_frames = a2.frames.size();
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
        if(n_frames == 0) return;
        frame* cur_frame = &frames[cur_frame_nr];
        if(cur_frame->duration == 0) return;
        
        cur_frame_time += time;
        
        //This is a while instead of an if because if the framerate is too low and the next frame's duration
        //is too short, it could be that a tick goes over an entire frame, and lands 2 frames ahead.
        while(cur_frame_time > cur_frame->duration) {
            cur_frame_time = cur_frame_time - cur_frame->duration;
            cur_frame_nr = (cur_frame_nr + 1) % n_frames;
            cur_frame = &frames[cur_frame_nr];
        }
    }
    inline frame* get_frame() { //Gets a pointer to the current frame.
        if(n_frames == 0) return NULL;
        return &frames[cur_frame_nr];
    }
};

//Extended frame data. Used on the animation editor, this struct also saves the filename and file coords.
struct ext_frame {
    ALLEGRO_BITMAP* parent_bmp;
    frame f;
    string file;        //Filename where the image is at.
    int file_x, file_y; //Top-left corner of the sprite inside the image file.
    int file_w, file_h; //Size of the sprite inside the image file.
    
    ext_frame(frame f = frame(), string fi = "", int x = 0, int y = 0, int w = 0, int h = 0, ALLEGRO_BITMAP* p = NULL) {
        this->f = f; file = fi; this->file_x = x; this->file_y = y; this->file_w = w; this->file_h = h; parent_bmp = p;
    }
    ext_frame(const ext_frame &ef2) {
        f = ef2.f; file = ef2.file; file_x = ef2.file_x; file_y = ef2.file_y; file_w = ef2.file_w; file_h = ef2.file_h;
        parent_bmp = ef2.parent_bmp;
    }
    ext_frame clone() {
        return ext_frame(f.clone(), file, file_x, file_y, file_w, file_h, al_clone_bitmap(parent_bmp));
    }
};

#endif //ifndef ANIMATION_INCLUDED