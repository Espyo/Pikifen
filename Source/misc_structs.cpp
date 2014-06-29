/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Miscellaneous structures, too small
 * to warrant their own files.
 */

#include <algorithm>
#include <climits>

#include "const.h"
#include "functions.h"
#include "misc_structs.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a structure with sample info.
 */
sample_struct::sample_struct(ALLEGRO_SAMPLE* s, ALLEGRO_MIXER* mixer) {
    sample = s;
    instance = NULL;
    
    if(!s) return;
    instance = al_create_sample_instance(s);
    al_attach_sample_instance_to_mixer(instance, mixer);
}

/* ----------------------------------------------------------------------------
 * Play the sample.
 * max_override_pos: Override the currently playing sound only if it's already in this position, or beyond.
 ** This is in seconds. 0 means always override. -1 means never override.
 * loop: Loop the sound?
 * gain: Volume, 0 - 1.
 * pan: Panning, 0 - 1 (0.5 is centered).
 * speed: Playing speed.
 */
void sample_struct::play(const float max_override_pos, const bool loop, const float gain, const float pan, const float speed) {
    if(!sample || !instance) return;
    
    if(max_override_pos != 0 && al_get_sample_instance_playing(instance)) {
        float secs = al_get_sample_instance_position(instance) / (float) 44100;
        if((secs < max_override_pos && max_override_pos > 0) || max_override_pos == -1) return;
    }
    
    al_set_sample_instance_playmode(instance, (loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE));
    al_set_sample_instance_gain(    instance, gain);
    al_set_sample_instance_pan(     instance, pan);
    al_set_sample_instance_speed(   instance, speed);
    
    al_set_sample_instance_position(instance, 0);
    al_set_sample_instance_playing( instance, true);
}

/* ----------------------------------------------------------------------------
 * Stops a playing sample instance.
 */
void sample_struct::stop() {
    al_set_sample_instance_playing(instance, false);
}

/* ----------------------------------------------------------------------------
 * Creates a structure with info about party spots.
 */
party_spot_info::party_spot_info(const unsigned max_mobs, const float spot_radius) {
    this->spot_radius = spot_radius;
    
    //Center spot first.
    x_coords.push_back(vector<float>(1, 0));
    y_coords.push_back(vector<float>(1, 0));
    mobs_in_spots.push_back(vector<mob*>(1, NULL));
    
    unsigned total_spots = 1; //Starts at 1 because we did the center spot already.
    unsigned w = 1; //Current wheel.
    while(total_spots < max_mobs) {
    
        //First, calculate how far the center of these spots are from the central spot.
        float dist_from_center =
            spot_radius * w + //Spots.
            PARTY_SPOT_INTERVAL * w; //Interval between spots.
            
        /* Now we need to figure out what's the angular distance between each spot.
         * For that, we need the actual diameter (distance from one point to the other),
         * and the central distance, which is distance between the center
         * and the middle of two spots.
         */
        
        /* We can get the middle distance because we know the actual diameter,
         * which should be the size of a Pikmin and one interval unit,
         * and we know the distance from one spot to the center.
         */
        float actual_diameter = spot_radius + PARTY_SPOT_INTERVAL;
        
        //Just calculate the remaining side of the triangle, now that we know
        //the hypotenuse and the actual diameter (one side of the triangle).
        float middle_distance = sqrt(
                                    (dist_from_center * dist_from_center) -
                                    (actual_diameter * 0.5 * actual_diameter * 0.5));
                                    
        //Now, get the angular distance.
        float angular_dist = 2 * atan2(actual_diameter, 2 * middle_distance);
        
        //Finally, we can calculate where the other spots are.
        unsigned n_spots_on_wheel = floor(M_PI * 2 / angular_dist);
        //Get a better angle. One that can evenly distribute the spots.
        float angle = M_PI * 2 / n_spots_on_wheel;
        
        x_coords.push_back(vector<float>());
        y_coords.push_back(vector<float>());
        mobs_in_spots.push_back(vector<mob*>());
        for(unsigned s = 0; s < n_spots_on_wheel; s++) {
            x_coords.back().push_back(dist_from_center * cos(angle * s) + randomf(-PARTY_SPOT_INTERVAL, PARTY_SPOT_INTERVAL));
            y_coords.back().push_back(dist_from_center * sin(angle * s) + randomf(-PARTY_SPOT_INTERVAL, PARTY_SPOT_INTERVAL));
            mobs_in_spots.back().push_back(NULL);
        }
        
        total_spots += n_spots_on_wheel;
        w++;
    }
    
    n_wheels = w;
    current_wheel = n_current_wheel_members = 0;
}

/* ----------------------------------------------------------------------------
 * Adds a member to a leader's party spots.
 */
void party_spot_info::add(mob* m, float* x, float* y) {
    if(n_current_wheel_members == mobs_in_spots[current_wheel].size()) {
        current_wheel++;
        n_current_wheel_members = 0;
    }
    
    size_t n_spots_in_wheel = mobs_in_spots[current_wheel].size();
    size_t chosen_spot_nr = randomi(0, (n_spots_in_wheel - n_current_wheel_members) - 1);
    size_t chosen_spot = 0;
    auto v = &mobs_in_spots[current_wheel];
    for(unsigned s = 0, c = 0; s < n_spots_in_wheel; s++) {
        if((*v)[s]) continue;
        if(c == chosen_spot_nr) {
            chosen_spot = s;
            break;
        }
        c++;
    }
    
    mobs_in_spots[current_wheel][chosen_spot] = m;
    
    n_current_wheel_members++;
    
    if(x) *x = x_coords[current_wheel][chosen_spot];
    if(y) *y = y_coords[current_wheel][chosen_spot];
}

/* ----------------------------------------------------------------------------
 * Removes a member from a leader's party spots.
 */
void party_spot_info::remove(mob* m) {
    unsigned mob_wheel = UINT_MAX; //Wheel number of the mob we're trying to remove.
    unsigned mob_spot = UINT_MAX; //Spot number of the mob we're trying to remove.
    
    size_t n_wheels = mobs_in_spots.size();
    for(size_t w = 0; w < n_wheels; w++) {
    
        size_t n_spots = mobs_in_spots[w].size();
        for(size_t s = 0; s < n_spots; s++) {
        
            if(mobs_in_spots[w][s] == m) {
                mob_wheel = w;
                mob_spot = s;
                break;
            }
            
        }
        
        if(mob_wheel != UINT_MAX) break;
    }
    
    //If the member to remove is the only one from the outermost wheel, let it go.
    if(n_current_wheel_members == 1 && current_wheel == mob_wheel) {
        if(current_wheel == 0) {
            n_current_wheel_members = 0;
        } else {
            current_wheel--;
            n_current_wheel_members = mobs_in_spots[current_wheel].size();
        }
        mobs_in_spots[mob_wheel][mob_spot] = NULL;
    } else {
        //If it's not from the outermost wheel, find some other mob (from the outermost wheel) to replace it.
        unsigned replacement_spot;
        unsigned n_spots = mobs_in_spots[current_wheel].size();
        
        do {
            replacement_spot = randomi(0, n_spots - 1);
        } while(!mobs_in_spots[current_wheel][replacement_spot] || (current_wheel == mob_wheel && replacement_spot == mob_spot));
        
        mobs_in_spots[mob_wheel][mob_spot] = mobs_in_spots[current_wheel][replacement_spot];
        mobs_in_spots[current_wheel][replacement_spot] = NULL;
        
        //ToDo remove this temporary hack:
        mobs_in_spots[mob_wheel][mob_spot]->target_x = x_coords[mob_wheel][mob_spot];
        mobs_in_spots[mob_wheel][mob_spot]->target_y = y_coords[mob_wheel][mob_spot] + 30;
        
        n_current_wheel_members--;
        if(n_current_wheel_members == 0) {
            current_wheel--;
            n_current_wheel_members = mobs_in_spots[current_wheel].size();
        }
    }
}

/* ----------------------------------------------------------------------------
 * Creates a structure with information about a bitmap, for the manager.
 */
bmp_info::bmp_info(ALLEGRO_BITMAP* b) {
    this->b = b;
    calls = 1;
}

/* ----------------------------------------------------------------------------
 * Returns the specified bitmap, by name.
 */
ALLEGRO_BITMAP* bmp_manager::get(const string &name, data_node* node) {
    if(name.size() == 0) return load_bmp("", node);
    
    if(list.find(name) == list.end()) {
        ALLEGRO_BITMAP* b = load_bmp(name, node);
        list[name] = bmp_info(b);
        return b;
    } else {
        list[name].calls++;
        return list[name].b;
    }
};

/* ----------------------------------------------------------------------------
 * Marks a bitmap to have one less call.
 * If it has 0 calls, it's automatically cleared.
 */
void bmp_manager::detach(const string &name) {
    if(name.size() == 0) return;
    
    auto it = list.find(name);
    if(it == list.end()) return;
    
    it->second.calls--;
    if(it->second.calls == 0) {
        if(it->second.b != bmp_error) {
            al_destroy_bitmap(it->second.b);
        }
        list.erase(it);
    }
}

/* ----------------------------------------------------------------------------
 * Registers a new type of sector.
 */
void sector_types_manager::register_type(unsigned char nr, string name) {
    if(nr >= names.size()) {
        names.insert(names.end(), (nr + 1) - names.size(), "");
    }
    names[nr] = name;
}

/* ----------------------------------------------------------------------------
 * Returns the number of a sector type, given its name.
 * Returns 255 on error.
 */
unsigned char sector_types_manager::get_nr(const string &name) {
    for(unsigned char n = 0; n < names.size(); n++) {
        if(names[n] == name) return n;
    }
    return 255;
}

/* ----------------------------------------------------------------------------
 * Returns the name of a sector type, given its number.
 * Returns an empty string on error.
 */
string sector_types_manager::get_name(const unsigned char nr) {
    if(nr < names.size()) return names[nr];
    return "";
}

/* ----------------------------------------------------------------------------
 * Returns the number of sector types registered.
 */
unsigned char sector_types_manager::get_nr_of_types() {
    return names.size();
}

/* ----------------------------------------------------------------------------
 * Registers a new mob folder.
 */
void mob_folder_manager::register_folder(
    unsigned char nr, string pname, string sname,
    function<void (vector<string> &list)> lister,
    function<mob_type* (const string &name)> type_getter
) {
    if(nr >= pnames.size()) {
        pnames.insert(pnames.end(), (nr + 1) - pnames.size(), "");
        snames.insert(snames.end(), (nr + 1) - snames.size(), "");
        listers.insert(listers.end(), (nr + 1) - listers.size(), function<void(vector<string> &)>());
        type_getters.insert(type_getters.end(), (nr + 1) - type_getters.size(), function<mob_type* (const string &)>());
    }
    pnames[nr] = pname;
    snames[nr] = sname;
    listers[nr] = lister;
    type_getters[nr]  = type_getter;
}

/* ----------------------------------------------------------------------------
 * Returns the number of a folder given its plural name.
 * Returns 255 on error.
 */
unsigned char mob_folder_manager::get_nr_from_pname(const string &pname) {
    for(unsigned char n = 0; n < pnames.size(); n++) {
        if(pnames[n] == pname) return n;
    }
    return 255;
}

/* ----------------------------------------------------------------------------
 * Returns the number of a folder given its singular name.
 * Returns 255 on error.
 */
unsigned char mob_folder_manager::get_nr_from_sname(const string &sname) {
    for(unsigned char n = 0; n < snames.size(); n++) {
        if(snames[n] == sname) return n;
    }
    return 255;
}

/* ----------------------------------------------------------------------------
 * Returns the plural name of a folder given its number.
 * Returns an empty string on error.
 */
string mob_folder_manager::get_pname(const unsigned char nr) {
    if(nr < pnames.size()) return pnames[nr];
    return "";
}

/* ----------------------------------------------------------------------------
 * Returns the singular name of a folder given its number.
 * Returns an empty string on error.
 */
string mob_folder_manager::get_sname(const unsigned char nr) {
    if(nr < snames.size()) return snames[nr];
    return "";
}

/* ----------------------------------------------------------------------------
 * Returns the number of registered mob folders.
 */
unsigned char mob_folder_manager::get_nr_of_folders() {
    return pnames.size();
}

/* ----------------------------------------------------------------------------
 * Lists the names of all mob types in a folder onto a vector of strings.
 */
void mob_folder_manager::get_list(vector<string> &l, unsigned char nr) {
    if(nr < listers.size()) listers[nr](l);
}

/* ----------------------------------------------------------------------------
 * Sets a mob generator's type pointer, given the type's name.
 * It uses the mob gen's existing folder to search for the name.
 */
void mob_folder_manager::set_mob_type_ptr(mob_gen* m, const string &type_name) {
    m->type = type_getters[m->folder](type_name);
}