#include "const.h"
#include "functions.h"
#include "misc_structs.h"

sample_struct::sample_struct(ALLEGRO_SAMPLE* s) {
    sample = s;
    
    //I don't think I should be messing with these... But they'll give an error otherwise.
    id._id = 0;
    id._index = 0;
}

group_spot_info::group_spot_info(unsigned max_mobs, float pikmin_size) {
    //Center spot first.
    x_coords.push_back(vector<float>(1, 0));
    y_coords.push_back(vector<float>(1, 0));
    mobs_in_spots.push_back(vector<mob*>(1, NULL));
    
    unsigned total_spots = 1; //Starts at 1 because we did the center spot already.
    unsigned w = 1; //Current wheel.
    while(total_spots < max_mobs) {
    
        //First, calculate how far the center of these spots are from the central spot.
        float dist_from_center =
            pikmin_size * w + //Spots.
            GROUP_SPOT_INTERVAL * w; //Interval between spots.
            
        /* Now we need to figure out what's the angular distance between each spot.
         * For that, we need the actual diameter (distance from one point to the other),
         * and the central distance, which is distance between the center
         * and the middle of two spots.
         */
        
        /* We can get the middle distance because we know the actual diameter,
         * which should be the size of a Pikmin and one interval unit,
         * and we know the distance from one spot to the center.
         */
        float actual_diameter = pikmin_size + GROUP_SPOT_INTERVAL;
        
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
            x_coords.back().push_back(dist_from_center * cos(angle * s) + random(-GROUP_SPOT_INTERVAL, GROUP_SPOT_INTERVAL));
            y_coords.back().push_back(dist_from_center * sin(angle * s) + random(-GROUP_SPOT_INTERVAL, GROUP_SPOT_INTERVAL));
            mobs_in_spots.back().push_back(NULL);
        }
        
        total_spots += n_spots_on_wheel;
        w++;
    }
    
    n_wheels = w;
    current_wheel = n_current_wheel_members = 0;
}

void group_spot_info::add(mob* m, float* x, float* y) {
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

void group_spot_info::remove(mob* m) {
    unsigned mob_wheel = MAXUINT; //Wheel number of the mob we're trying to remove.
    unsigned mob_spot = MAXUINT; //Spot number of the mob we're trying to remove.
    
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
        
        if(mob_wheel != MAXUINT) break;
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