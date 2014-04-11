#include <climits>

#include "const.h"
#include "functions.h"
#include "misc_structs.h"

sample_struct::sample_struct(ALLEGRO_SAMPLE* s, ALLEGRO_MIXER* mixer) {
    sample = s;
    instance = NULL;
    
    if(!s) return;
    instance = al_create_sample_instance(s);
    al_attach_sample_instance_to_mixer(instance, mixer);
    
    /*
    //ToDo remove this?
    //I don't think I should be messing with these... But they'll give an error otherwise.
    id._id = 0;
    id._index = 0;*/
}

/*
 * Play the sample.
 * max_override_pos: Override the currently playing sound only if it's already in this position, or beyond.
 ** This is in seconds. 0 means always override. -1 means never override.
 * loop: Loop the sound?
 * gain: Volume, 0 - 1.
 * pan: Panning, 0 - 1 (0.5 is centered).
 * speed: Playing speed.
 */
void sample_struct::play(float max_override_pos, bool loop, float gain, float pan, float speed) {
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

void sample_struct::stop() {
    al_set_sample_instance_playing(instance, false);
}

party_spot_info::party_spot_info(unsigned max_mobs, float spot_radius) {
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
            x_coords.back().push_back(dist_from_center * cos(angle * s) + random(-PARTY_SPOT_INTERVAL, PARTY_SPOT_INTERVAL));
            y_coords.back().push_back(dist_from_center * sin(angle * s) + random(-PARTY_SPOT_INTERVAL, PARTY_SPOT_INTERVAL));
            mobs_in_spots.back().push_back(NULL);
        }
        
        total_spots += n_spots_on_wheel;
        w++;
    }
    
    n_wheels = w;
    current_wheel = n_current_wheel_members = 0;
}

void party_spot_info::add(mob* m, float* x, float* y) {
    if(n_current_wheel_members == mobs_in_spots[current_wheel].size()) {
        current_wheel++;
        n_current_wheel_members = 0;
    }
    
    unsigned chosen_spot;
    size_t n_spots_in_wheel = mobs_in_spots[current_wheel].size();
    do {
        chosen_spot = random(0, n_spots_in_wheel - 1);
    } while(mobs_in_spots[current_wheel][chosen_spot]);
    
    mobs_in_spots[current_wheel][chosen_spot] = m;
    
    n_current_wheel_members++;
    
    if(x) *x = x_coords[current_wheel][chosen_spot];
    if(y) *y = y_coords[current_wheel][chosen_spot];
}

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
            replacement_spot = random(0, n_spots - 1);
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
