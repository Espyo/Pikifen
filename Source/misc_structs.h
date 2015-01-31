/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the miscellaneous structures,
 * too simple to warrant their own files.
 */

#ifndef MISC_STRUCTS_INCLUDED
#define MISC_STRUCTS_INCLUDED

#include <functional>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "mob.h"
#include "mob_type.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Structure with info for the bitmap manager.
 */
class bmp_info {
public:
    ALLEGRO_BITMAP* b;
    size_t calls;
    bmp_info(ALLEGRO_BITMAP* b = NULL);
};



/* ----------------------------------------------------------------------------
 * Bitmap manager.
 * When you have the likes of an animation, every
 * frame in it is normally a sub-bitmap of the same
 * parent bitmap.
 * Naturally, loading from the disk and storing
 * in memory the same parent bitmap for every single
 * frame would be unbelievable catastrophical, so
 * that is why the bitmap manager was created.
 *
 * Whenever a frame of animation is being loaded,
 * it asks the bitmap manager to retrieve the
 * parent bitmap from memory. If the parent bitmap
 * has never been loaded, it gets loaded now.
 * When the next frame comes, and requests the
 * parent bitmap, the manager just returns the one already
 * loaded.
 * All the while, the manager is keeping track
 * of how many frames are referencing this parent bitmap.
 * When one of them doesn't need it any more, it sends
 * a detach request (e.g.: when a frame is changed
 * in the animation editor, the entire bitmap
 * is destroyed and another is created).
 * This decreases the counter by one.
 * When the counter reaches 0, that means no frame
 * is needing the parent bitmap, so it gets destroyed.
 * If some other frame needs it, it'll be loaded from
 * the disk again.
 * Finally, it should be noted that animation frames
 * are not the only thing using the bitmap manager.
 */
class bmp_manager {
public:
    map<string, bmp_info> list;
    ALLEGRO_BITMAP* get(const string &name, data_node* node);
    void detach(const string &name);
};



/* ----------------------------------------------------------------------------
 * A list of the different mob categories.
 * The MOB_CATEGORY_* constants are meant to be used here.
 * Read the sector type manager's comments for more info.
 */
struct mob_category_manager {
private:
    vector<string> pnames; // Plural names.
    vector<string> snames; // Singular names.
    vector<function<void (vector<string> &list)> > listers; // Lists all types' names onto the vector.
    vector<function<mob_type* (const string &name)> > type_getters; // Returns pointer to the type of the matching name.
    
public:
    void register_category(
        unsigned char nr, string pname, string sname,
        function<void (vector<string> &list)> lister,
        function<mob_type* (const string &name)> type_getter
    );
    unsigned char get_nr_from_pname(const string &pname);
    unsigned char get_nr_from_sname(const string &sname);
    string get_pname(const unsigned char nr);
    string get_sname(const unsigned char nr);
    unsigned char get_nr_of_categories();
    void get_list(vector<string> &l, unsigned char nr);
    void set_mob_type_ptr(mob_gen* m, const string &type_name);
    
};



/* ----------------------------------------------------------------------------
 * This structure holds information about how to move something
 * that is user-controlled. It contains the amount of movement
 * on each of the four main directions, ranging from 0 to 1.
 */
struct movement_struct {
    float right;
    float up;
    float left;
    float down;
    
    movement_struct();
    float get_intensity();
    float get_x();
    float get_y();
};



/* ----------------------------------------------------------------------------
 * Group spots. The way this works is that a Pikmin group surrounds a central point.
 * There are several wheels surrounding the original spot,
 * starting from the center and growing in size, each with several spots of their own.
 * A Pikmin occupies the central spot first.
 * Then other Pikmin come by, and occupy spots at random on the next wheel.
 * When that wheel has all of its spots full, the next wheel will be used, and so on.
 */
struct party_spot_info {

    float spot_radius;
    
    vector<vector<float> > x_coords;
    vector<vector<float> > y_coords;
    unsigned n_wheels;
    
    vector<vector<mob*> > mobs_in_spots;
    unsigned current_wheel;
    unsigned n_current_wheel_members;
    
    party_spot_info(const unsigned max_mobs, const float spot_size);
    void add(mob* m, float* x, float* y);
    void remove(mob* m);
};



/* ----------------------------------------------------------------------------
 * Simple 2D point.
 */
struct point {
    float x, y;
    point(const float x = 0, const float y = 0) { this->x = x; this->y = y; }
    bool operator!=(const point &p2) { return x != p2.x || y != p2.y; }
};



/* ----------------------------------------------------------------------------
 * Structure that holds informatio about a sample.
 * It also has info about sample instances, which control
 * the sound playing from the sample.
 */
struct sample_struct {
    ALLEGRO_SAMPLE*          sample;   // Pointer to the sample.
    ALLEGRO_SAMPLE_INSTANCE* instance; // Pointer to the instance.
    
    sample_struct(ALLEGRO_SAMPLE* sample = NULL, ALLEGRO_MIXER* mixer = NULL);
    void play(const float max_override_pos, const bool loop, const float gain = 1.0, const float pan = 0.5, const float speed = 1.0);
    void stop();
};



/* ----------------------------------------------------------------------------
 * Just a list of the different sector types.
 * The ERROR_TYPE_* constants are meant to be used here.
 * This is a vector instead of a map because hopefully,
 * the numbers are filled in sequence, as they're from
 * an enum, hence, there are no gaps.
 */
struct sector_types_manager {
private:
    vector<string> names;
    
public:
    void register_type(unsigned char nr, string name);
    unsigned char get_nr(const string &name);
    string get_name(const unsigned char nr);
    unsigned char get_nr_of_types();
    
};

#endif // ifndef MISC_STRUCTS_INCLUDED
