#ifndef ANIMATION_INCLUDED
#define ANIMATION_INCLUDED

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "hitbox.h"

using namespace std;

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
    float top_x, top_y;               //X&Y of the Pikmin's top (left/bud/flower).
    float top_w, top_h;               //W&H of the Pikmin's top.
    float top_angle;                  //Angle of the Pikmin's top.
    bool top_visible;                 //Does this frame even have a visible Pikmin top?
    ALLEGRO_BITMAP* bitmap;           //Actual bitmap. This is a sub-bitmap of parent_bmp.
    vector<hitbox_instance> hitbox_instances; //List of hitboxes on this frame.
    
    frame(string name = "", ALLEGRO_BITMAP* b = NULL, float gw = 0, float gh = 0, vector<hitbox_instance> h = vector<hitbox_instance>());
    frame(string name, ALLEGRO_BITMAP* b, int bx, int by, int bw, int bh, float gw, float gh, vector<hitbox_instance> h);
    frame(const frame &f2);
    frame clone();
    
    ~frame();
};



//Instance of a frame inside an animation.
//A single frame can appear multiple times in the same animation (imagine an enemy shaking back and forth).
class frame_instance {
public:
    string frame_name;
    frame* frame_ptr; //Needed for performance.
    float duration; //How long this frame lasts for, in seconds.
    
    frame_instance(string fn = "", frame* fp = NULL, float d = 0);
};



//A list of frames, basically.
class animation {
public:
    string name;
    vector<frame_instance> frame_instances; //List of frames.
    size_t loop_frame;    //The animation loops back to this frame when it reaches the end.
    
    animation(string name = "", vector<frame_instance> frame_instances = vector<frame_instance>(), size_t loop_frame = 0);
    animation(const animation &a2);
};

//A set of animations and their necessary data.
class animation_set {
public:
    map<string, animation*> animations;
    map<string, frame*> frames;
    map<string, hitbox*> hitboxes;
    
    animation_set(
        map<string, animation*> a = map<string, animation*>(),
        map<string, frame*> f = map<string, frame*>(),
        map<string, hitbox*> h = map<string, hitbox*>()
    );
    
    void destroy();
    
};

//Instance of a running animation. This can be played and such.
class animation_instance {
public:
    animation* anim;
    animation_set* anim_set;
    float cur_frame_time;  //Time passed on the current frame.
    size_t cur_frame_nr;
    bool done_once;
    
    animation_instance(animation_set* anim_set = NULL);
    animation_instance(const animation_instance &ai2);
    
    void change(string new_anim, bool only_if_new, bool only_if_done);
    void start();
    bool tick(float time);
    frame* get_frame();
};

#endif //ifndef ANIMATION_INCLUDED