#ifndef ANIMATION_INCLUDED
#define ANIMATION_INCLUDED

#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

using namespace std;

class frame {
public:
    ALLEGRO_BITMAP* bitmap;
    float duration;
    
    frame(ALLEGRO_BITMAP* b = NULL, float d = 0);
    frame(ALLEGRO_BITMAP* b, int x, int y, int w, int h, float d);
    frame(const frame &f2);
};

class animation {
private:
    //Helpers to save on CPU usage.
    size_t current_frame_nr; //Used when switching to the next frame.
    size_t n_frames; //Used instead of frames.size() (a function call).
    
    vector<frame> frames; //List of frames.
    frame* current_frame; //Current frame.
    float current_frame_time; //Time passed on the current frame.
    
public:
    animation(vector<frame> frames);
    animation(const animation &a2);
    animation();
    
    void start(); //Starts or restarts an animation. It's called when the animation is created.
    void tick(float time); //Ticks the animation.
    inline ALLEGRO_BITMAP* get_bitmap(); //Gets the current frame's bitmap.
};

/*
 * Creates a frame, given a bitmap and a duration.
 */
frame::frame(ALLEGRO_BITMAP* b, float d) {
    bitmap = b;
    duration = d;
}

/*
 * Creates a frame with the bitmap being a sub-bitmap of the specified bitmap, and a duration.
 */
frame::frame(ALLEGRO_BITMAP* b, int x, int y, int w, int h, float d) {
    bitmap = al_create_sub_bitmap(b, x, y, w, h);
    duration = d;
}

/*
 * Creates a frame by pointing to the same bitmap as the other one,
 * and having the same duration.
 */
frame::frame(const frame &f2) {
    bitmap = f2.bitmap;
    duration = f2.duration;
}

/*
 * Creates an animation given a vector of frames.
 */
animation::animation(vector<frame> frames) {
    n_frames = frames.size();
    this->frames = frames;
    
    start();
}

/*
 * Creates an animation by copying another one's frames.
 */
animation::animation(const animation &a2) {
    n_frames = a2.frames.size();
    frames = a2.frames;
    
    start();
}

/*
 * Creates an animation with no frames.
 */
animation::animation() {
    n_frames = 0;
    
    start();
}

/*
 * Sets an animation to start on the first frame.
 */
void animation::start() {
    current_frame_time = 0;
    current_frame_nr = 0;
    
    if(n_frames > 0) {
        current_frame = &frames[0];
    } else {
        current_frame = NULL;
    }
}

/*
 * Ticks the animation with a given time difference.
 */
void animation::tick(float time) {
    if(current_frame->duration == 0) return;
    
    current_frame_time += time;
    
    //This is a while instead of an if because if the framerate is too low and the next frame's duration
    //is too short, it could be that a tick goes over an entire frame, and lands 2 frames ahead.
    while(current_frame_time > current_frame->duration) {
        current_frame_time = current_frame_time - current_frame->duration;
        current_frame_nr = (current_frame_nr + 1) & n_frames;
        current_frame = &frames[current_frame_nr];
    }
}

/*
 * Returns the bitmap of the current frame of animation.
 */
inline ALLEGRO_BITMAP* animation::get_bitmap() {
    if(!current_frame) return NULL;
    return current_frame->bitmap;
}

#endif //ifndef ANIMATION_INCLUDED