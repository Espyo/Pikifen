#ifndef MISC_STRUCTS_INCLUDED
#define MISC_STRUCTS_INCLUDED

#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "mob.h"

using namespace std;

struct party_spot_info {
    /* Group spots. The way this works is that a Pikmin group surrounds a central point.
     * There are several wheels surrounding the original spot,
     * starting from the center and growing in size, each with several spots of their own.
     * A Pikmin occupies the central spot first.
     * Then other Pikmin come by, and occupy spots at random on the next wheel.
     * When that wheel has all of its spots full, the next wheel will be used, and so on.
     */
    float spot_radius;
    
    vector<vector<float> > x_coords;
    vector<vector<float> > y_coords;
    unsigned n_wheels;
    
    vector<vector<mob*> > mobs_in_spots;
    unsigned current_wheel;
    unsigned n_current_wheel_members;
    
    party_spot_info(unsigned max_mobs, float spot_size);
    void add(mob* m, float* x, float* y);
    void remove(mob* m);
};

struct point {
    float x, y;
    point(float x = 0, float y = 0) { this->x = x; this->y = y; }
    bool operator!=(const point &p2) { return x != p2.x || y != p2.y; }
};

struct sample_struct {
    ALLEGRO_SAMPLE*          sample;   //Pointer to the sample.
    ALLEGRO_SAMPLE_INSTANCE* instance; //Pointer to the instance.
    //ALLEGRO_SAMPLE_ID   id;     //Sample id.
    
    sample_struct(ALLEGRO_SAMPLE* sample = NULL, ALLEGRO_MIXER* mixer = NULL);
    void play(float max_override_pos, bool loop, float gain = 1.0, float pan = 0.5, float speed = 1.0);
    void stop();
};

class bmp_info {
public:
    ALLEGRO_BITMAP* b;
    size_t calls;
    bmp_info(ALLEGRO_BITMAP* b = NULL);
};

class bmp_manager {
public:
    map<string, bmp_info> list;
    ALLEGRO_BITMAP* get(string name, data_node* node);
    void detach(string name);
};

#endif //ifndef MISC_STRUCTS_INCLUDED