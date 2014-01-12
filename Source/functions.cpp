#define _USE_MATH_DEFINES
#include <algorithm>
#include <math.h>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "data_file.h"
#include "functions.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Call this whenever an "active" control is inputted. An "active" control is anything that moves the captain in some way.
 * This function makes the captain wake up from lying down, stop auto-plucking, etc.
 */
void active_control() {
    make_uncarriable(leaders[cur_leader_nr]);
    leaders[cur_leader_nr]->auto_pluck_mode = false;
}

/* ----------------------------------------------------------------------------
 * Adds a mob to another mob's party.
 */
void add_to_party(mob* party_leader, mob* new_member) {
    if(new_member->following_party == party_leader) return; //Already following, never mind.
    
    new_member->following_party = party_leader;
    party_leader->party->members.push_back(new_member);
    
    //Find a spot.
    if(party_leader->party) {
        if(party_leader->party->party_spots) {
            float spot_x = 0, spot_y = 0;
            
            party_leader->party->party_spots->add(new_member, &spot_x, &spot_y);
            
            new_member->set_target(
                spot_x, spot_y,
                &party_leader->party->party_center_x, &party_leader->party->party_center_y,
                false
            );
        }
    }
    
    make_uncarriable(new_member);
}

/* ----------------------------------------------------------------------------
 * Returns the vector coordinates of an angle.
 * angle:    The angle.
 * magnitue: Its magnitude.
 * *_coord:  Variables to return the coordinates to.
 */
void angle_to_coordinates(float angle, float magnitude, float* x_coord, float* y_coord) {
    *x_coord = cos(angle) * magnitude;
    *y_coord = sin(angle) * magnitude;
}

/* ----------------------------------------------------------------------------
 * Returns the color that was provided, but with the alpha changed.
 * color: The color to change the alpha on.
 * a:     The new alpha, [0-255].
 */
ALLEGRO_COLOR change_alpha(ALLEGRO_COLOR c, unsigned char a) {
    ALLEGRO_COLOR c2;
    c2.r = c.r; c2.g = c.g; c2.b = c.b;
    c2.a = a / 255.0;
    return c2;
}

/* ----------------------------------------------------------------------------
 * Returns the angle and magnitude of vector coordinates.
 * *_coord:   The coordinates.
 * angle:     Variable to return the angle to.
 * magnitude: Variable to return the magnitude to.
 */
void coordinates_to_angle(float x_coord, float y_coord, float* angle, float* magnitude) {
    *angle = atan2(y_coord, x_coord);
    *magnitude = dist(0, 0, x_coord, y_coord);
}

/* ----------------------------------------------------------------------------
 * Creates a mob, adding it to the corresponding vectors.
 */
void create_mob(mob* m) {
    mobs.push_back(m);
    
    if(typeid(*m) == typeid(pikmin)) {
        pikmin_list.push_back((pikmin*) m);
        
    } else if(typeid(*m) == typeid(leader)) {
        leaders.push_back((leader*) m);
        
    } else if(typeid(*m) == typeid(onion)) {
        onions.push_back((onion*) m);
        
    } else if(typeid(*m) == typeid(nectar)) {
        nectars.push_back((nectar*) m);
        
    } else if(typeid(*m) == typeid(pellet)) {
        pellets.push_back((pellet*) m);
        
    } else if(typeid(*m) == typeid(ship)) {
        ships.push_back((ship*) m);
        
    } else if(typeid(*m) == typeid(treasure)) {
        treasures.push_back((treasure*) m);
        
    } else if(typeid(*m) == typeid(info_spot)) {
        info_spots.push_back((info_spot*) m);
        
    } else if(typeid(*m) == typeid(enemy)) {
        enemies.push_back((enemy*) m);
        
    }
}

/* ----------------------------------------------------------------------------
 * Deletes a mob from the relevant vectors.
 * It's always removed from the vector of mobs, but it's
 * also removed from the vector of Pikmin if it's a Pikmin,
 * leaders if it's a leader, etc.
 */
void delete_mob(mob* m) {
    mobs.erase(find(mobs.begin(), mobs.end(), m));
    
    if(typeid(*m) == typeid(pikmin)) {
        pikmin_list.erase(find(pikmin_list.begin(), pikmin_list.end(), (pikmin*) m));
        
    } else if(typeid(*m) == typeid(leader)) {
        leaders.erase(find(leaders.begin(), leaders.end(), (leader*) m));
        
    } else if(typeid(*m) == typeid(onion)) {
        onions.erase(find(onions.begin(), onions.end(), (onion*) m));
        
    } else if(typeid(*m) == typeid(nectar)) {
        nectars.erase(find(nectars.begin(), nectars.end(), (nectar*) m));
        
    } else if(typeid(*m) == typeid(pellet)) {
        pellets.erase(find(pellets.begin(), pellets.end(), (pellet*) m));
        
    } else if(typeid(*m) == typeid(ship)) {
        ships.erase(find(ships.begin(), ships.end(), (ship*) m));
        
    } else if(typeid(*m) == typeid(treasure)) {
        treasures.erase(find(treasures.begin(), treasures.end(), (treasure*) m));
        
    } else if(typeid(*m) == typeid(info_spot)) {
        info_spots.erase(find(info_spots.begin(), info_spots.end(), (info_spot*) m));
        
    } else {
        //ToDo warn somehow.
        
    }
}

/* ----------------------------------------------------------------------------
 * Makes the current leader dismiss their party.
 * The party is then organized in groups, by type,
 * and is dismissed close to the leader.
 */
void dismiss() {
    leader* cur_leader_ptr = leaders[cur_leader_nr];
    
    float
    min_x = 0, min_y = 0, max_x = 0, max_y = 0, //Leftmost member coordinate, rightmost, etc.
    cx, cy, //Center of the group.
    base_angle; //They are dismissed towards this angle. This is then offset a bit depending on the Pikmin type, so they spread out.
    
    //ToDo what if there are a lot of Pikmin types?
    size_t n_party_members = cur_leader_ptr->party->members.size();
    if(n_party_members == 0) return;
    
    //First, calculate what direction the party should be dismissed to.
    if(moving_group_intensity > 0) {
        //If the leader's moving the group, they should be dismissed towards the cursor.
        base_angle = moving_group_angle + M_PI;
    } else {
        for(size_t m = 0; m < n_party_members; m++) {
            mob* member_ptr = cur_leader_ptr->party->members[m];
            
            if(member_ptr->x < min_x || m == 0) min_x = member_ptr->x;
            if(member_ptr->x > max_x || m == 0) max_x = member_ptr->x;
            if(member_ptr->y < min_y || m == 0) min_y = member_ptr->y;
            if(member_ptr->y > max_y || m == 0) max_y = member_ptr->y;
        }
        
        cx = (min_x + max_x) / 2;
        cy = (min_y + max_y) / 2;
        base_angle = atan2(cy - cur_leader_ptr->y, cx - cur_leader_ptr->x) + M_PI;
    }
    
    //Then, calculate how many Pikmin types there are in the party.
    map<pikmin_type*, float> type_dismiss_angles;
    for(size_t m = 0; m < n_party_members; m++) {
    
        if(typeid(*cur_leader_ptr->party->members[m]) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->party->members[m]);
            
            type_dismiss_angles[pikmin_ptr->type] = 0;
        }
    }
    
    //For each type, calculate the angle;
    size_t n_types = type_dismiss_angles.size();
    if(n_types == 1) {
        //Small hack. If there's only one Pikmin type, dismiss them directly towards the base angle.
        type_dismiss_angles.begin()->second = M_PI_4;
    } else {
        unsigned current_type_nr = 0;
        for(map<pikmin_type*, float>::iterator t = type_dismiss_angles.begin(); t != type_dismiss_angles.end(); t++) {
            t->second = current_type_nr * (M_PI_2 / (n_types - 1));
            current_type_nr++;
        }
    }
    
    //Now, dismiss them.
    for(size_t m = 0; m < n_party_members; m++) {
        mob* member_ptr = cur_leader_ptr->party->members[0];
        remove_from_party(member_ptr);
        
        float angle = 0;
        
        if(typeid(*member_ptr) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(member_ptr);
            
            angle = base_angle + type_dismiss_angles[pikmin_ptr->type] - M_PI_4 + M_PI;
            
            member_ptr->set_target(
                cur_leader_ptr->x + cos(angle) * DISMISS_DISTANCE,
                cur_leader_ptr->y + sin(angle) * DISMISS_DISTANCE,
                NULL,
                NULL,
                false);
        }
    }
}

/* ----------------------------------------------------------------------------
 * Draws a strength/weight fraction, in the style of Pikmin 2.
 * The strength is above the weight.
 * c*:      Center of the text.
 * current: Current strength.
 * needed:  Needed strength to lift the object (weight).
 * color:   Color of the fraction's text.
 */
void draw_fraction(float cx, float cy, unsigned int current, unsigned int needed, ALLEGRO_COLOR color) {
    float first_y = cy - (font_h * 3) / 2;
    al_draw_text(font, color, cx, first_y, ALLEGRO_ALIGN_CENTER, (to_string((long long) current).c_str()));
    al_draw_text(font, color, cx, first_y + font_h * 1.5, ALLEGRO_ALIGN_CENTER, (to_string((long long) needed).c_str()));
    
    ALLEGRO_TRANSFORM scale, old;
    al_copy_transform(&old, al_get_current_transform());
    
    al_identity_transform(&scale);
    al_scale_transform(&scale, 5, 1);
    al_translate_transform(&scale, cx, 0);
    al_compose_transform(&scale, &old);
    
    al_use_transform(&scale); {
        al_draw_text(font, color, 0, first_y + font_h * 0.75, ALLEGRO_ALIGN_CENTER, "-");
    }; al_use_transform(&old);
}

/* ----------------------------------------------------------------------------
 * Draws a health wheel, with a pieslice that's fuller the more HP is full.
 * c*:         Center of the wheel.
 * health:     Current amount of health of the mob who's health we're representing.
 * max_health: Maximum amount of health of the mob; health for when it's fully healed.
 * radius:     Radius of the wheel (the whole wheel, not just the pieslice).
 * just_chart: If true, only draw the actual pieslice (pie-chart). Used for leader HP on the HUD.
 */
void draw_health(float cx, float cy, unsigned int health, unsigned int max_health, float radius, bool just_chart) {
    float ratio = (float) health / (float) max_health;
    ALLEGRO_COLOR c;
    if(ratio >= 0.5) {
        c = al_map_rgb_f(1 - (ratio - 0.5) * 2, 1, 0);
    } else {
        c = al_map_rgb_f(1, (ratio * 2), 0);
    }
    
    if(!just_chart) al_draw_filled_circle(cx, cy, radius, al_map_rgba(0, 0, 0, 128));
    al_draw_filled_pieslice(cx, cy, radius, -M_PI_2, -ratio * M_PI * 2, c);
    if(!just_chart) al_draw_circle(cx, cy, radius + 1, al_map_rgb(0, 0, 0), 2);
}

/* ----------------------------------------------------------------------------
 * Draws a sector on the current bitmap.
 * s:   The sector to draw.
 * x/y: Top-left coordinates.
 */
void draw_sector(sector &s, float x, float y) {
    ALLEGRO_VERTEX vs[200]; //ToDo 200?
    size_t n_linedefs = s.linedefs.size();
    unsigned char current_floor;
    unsigned char floors_to_draw;
    
    current_floor = (s.floors[0].z > s.floors[1].z) ? 1 : 0;
    floors_to_draw = (s.floors[0].z == s.floors[1].z) ? 1 : 2;  //ToDo remove this check?
    
    for(unsigned char f = 0; f < floors_to_draw; f++) {
    
        for(size_t l = 0; l < n_linedefs; l++) {
            vs[l].x = s.linedefs[l]->x1 - x;
            vs[l].y = s.linedefs[l]->y1 - y;
            vs[l].u = s.linedefs[l]->x1;
            vs[l].v = s.linedefs[l]->y1;
            vs[l].z = 0;
            vs[l].color = al_map_rgba_f(s.floors[current_floor].brightness, s.floors[current_floor].brightness, s.floors[current_floor].brightness, 1);
        }
        
        al_draw_prim(vs, NULL, s.floors[current_floor].texture, 0, n_linedefs, ALLEGRO_PRIM_TRIANGLE_FAN);
        
        current_floor = (current_floor == 1) ? 0 : 1;
        
    }
    
}

/* ----------------------------------------------------------------------------
 * Draws a mob's shadow.
 * c*:             Center of the mob.
 * size:           Size of the mob.
 * delta_z:        The mob is these many units above the floor directly below it.
 * shadow_stretch: How much to stretch the shadow by (used to simulate sun shadow direction casting).
 */
void draw_shadow(float cx, float cy, float size, float delta_z, float shadow_stretch) {
    if(shadow_stretch <= 0) return;
    
    float shadow_x = 0, shadow_w = size + (size * 3 * shadow_stretch);
    
    if(day_minutes < 60 * 12) {
        //Shadows point to the West.
        shadow_x = -shadow_w + size * 0.5;
        shadow_x -= shadow_stretch * delta_z * SHADOW_Y_MULTIPLIER;
    } else {
        //Shadows point to the East.
        shadow_x = -(size * 0.5);
        shadow_x += shadow_stretch * delta_z * SHADOW_Y_MULTIPLIER;
    }
    
    
    draw_sprite(
        bmp_shadow,
        cx + shadow_x + shadow_w / 2, cy,
        shadow_w, size,
        0, al_map_rgba(255, 255, 255, 255 * (1 - shadow_stretch)));
}

/* ----------------------------------------------------------------------------
 * Draws a sprite.
 * bmp:   The bitmap.
 * c*:    Center coordinates.
 * w/h:   Final width and height
 * angle: Angle to rotate the sprite by.
 * tint:  Tint the sprite with this color.
 */
void draw_sprite(ALLEGRO_BITMAP* bmp, float cx, float cy, float w, float h, float angle, ALLEGRO_COLOR tint) {
    if(!bmp) {
        bmp = bmp_error;
    }
    
    float bmp_w = al_get_bitmap_width(bmp);
    float bmp_h = al_get_bitmap_height(bmp);
    float x_scale = w / bmp_w;
    float y_scale = h / bmp_h;
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        tint,
        bmp_w / 2, bmp_h / 2,
        cx, cy,
        x_scale, y_scale,
        angle,
        0);
}

/* ----------------------------------------------------------------------------
 * Draws text, but if there are line breaks, it'll draw every line one under the other.
 * It basically calls Allegro's text drawing functions, but for each line.
 * f:    Font to use.
 * c:    Color.
 * x/y:  Coordinates of the text.
 * fl:   Flags, just like the ones you'd pass to al_draw_text.
 * va:   Vertical align: 1 for top, 2 for center, 3 for bottom.
 * text: Text to write, line breaks included ('\n').
 */
void draw_text_lines(ALLEGRO_FONT* f, ALLEGRO_COLOR c, float x, float y, int fl, unsigned char va, string text) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(f);
    size_t n_lines = lines.size();
    float top;
    
    if(va == 0) {
        top = y;
    } else {
        int total_height = n_lines * fh + (n_lines - 1);  //We add n_lines - 1 because there is a 1px gap between each line.
        if(va == 1) {
            top = y - total_height / 2;
        } else {
            top = y - total_height;
        }
    }
    
    for(size_t l = 0; l < n_lines; l++) {
        float line_y = (fh + 1) * l + top;
        al_draw_text(f, c, x, line_y, fl, lines[l].c_str());
    }
}

/* ----------------------------------------------------------------------------
 * Makes a Pikmin release a mob it's carrying.
 */
void drop_mob(pikmin* p) {
    if(!p->carrying_mob) return;
    
    //ToDo optimize this instead of running through the spot vector.
    if(p->carrying_mob) {
        for(size_t s = 0; s < p->carrying_mob->carrier_info->max_carriers; s++) {
            if(p->carrying_mob->carrier_info->carrier_spots[s] == p) {
                p->carrying_mob->carrier_info->carrier_spots[s] = NULL;
                p->carrying_mob->carrier_info->current_n_carriers--;
            }
        }
    }
    
    //Did this Pikmin leaving made the mob stop moving?
    if(p->carrying_mob->carrier_info->current_n_carriers < p->carrying_mob->type->weight) {
        p->carrying_mob->remove_target(true);
    } else {
        start_carrying(p->carrying_mob, NULL, p); //Enter this code so that if this Pikmin leaving broke a tie, the Onion's picked correctly.
    }
    
    p->carrying_mob = NULL;
    p->remove_target(true);
}

/* ----------------------------------------------------------------------------
 * Prints something onto the error log.
 */
void error_log(string s) {
    //ToDo
    total_error_log += s + "\n";
}

/* ----------------------------------------------------------------------------
 * Stores the names of all files in a folder into a vector.
 * folder_name: Name of the folder.
 * folders:     If true, only read folders. If false, only read files.
 */
vector<string> folder_to_vector(string folder_name, bool folders) {
    vector<string> v;
    
    //Normalize the folder's path.
    replace(folder_name.begin(), folder_name.end(), '\\', '/');
    if(folder_name[folder_name.size() - 1] == '/') folder_name.erase(folder_name.size() - 1, 1);
    
    ALLEGRO_FS_ENTRY* folder = NULL;
    folder = al_create_fs_entry(folder_name.c_str());
    if(!folder) return v;
    
    ALLEGRO_FS_ENTRY* entry = NULL;
    
    if(al_open_directory(folder)) {
        while((entry = al_read_directory(folder)) != NULL) {
            if(
                (folders && (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR)) ||
                (!folders && !(al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR))) {
                
                string entry_name = al_get_fs_entry_name(entry);
                if(folders) {   //If we're using folders, remove the trailing slash, lest the string be fully deleted.
                    if(entry_name[entry_name.size() - 1] == '/' || entry_name[entry_name.size() - 1] == '\\') {
                        entry_name = entry_name.substr(0, entry_name.size() - 1);
                    }
                }
                
                //Only save what's after the final slash.
                size_t pos_bs = entry_name.find_last_of("\\");
                size_t pos_fs = entry_name.find_last_of("/");
                size_t pos = pos_bs;
                if(pos_fs != string::npos)
                    if(pos_fs > pos_bs) pos = pos_bs;
                    
                if(pos != string::npos) entry_name = entry_name.substr(pos + 1, entry_name.size() - pos - 1);
                v.push_back(entry_name);
            }
        }
        al_close_directory(folder);
    }
    if(folder) al_destroy_fs_entry(folder);
    if(entry) al_destroy_fs_entry(entry);
    
    return v;
}

/* ----------------------------------------------------------------------------
 * Generates the images that make up the area.
 */
void generate_area_images() {
    //First, clear all existing area images.
    for(size_t x = 0; x < area_images.size(); x++) {
        for(size_t y = 0; y < area_images[x].size(); y++) {
            al_destroy_bitmap(area_images[x][y]);
        }
        area_images[x].clear();
    }
    area_images.clear();
    
    //Now, figure out how big our area is.
    size_t n_sectors = sectors.size();
    if(n_sectors == 0) return;
    if(sectors[0].linedefs.size() == 0) return;
    
    float min_x, max_x, min_y, max_y;
    min_x = max_x = sectors[0].linedefs[0]->x1;
    min_y = max_y = sectors[0].linedefs[0]->y1;
    
    for(size_t s = 0; s < n_sectors; s++) {
        size_t n_linedefs = sectors[s].linedefs.size();
        for(size_t l = 0; l < n_linedefs; l++) {
            float x = sectors[s].linedefs[l]->x1;
            float y = sectors[s].linedefs[l]->y1;
            
            min_x = min(x, min_x);
            max_x = max(x, max_x);
            min_y = min(y, min_y);
            max_y = max(y, max_y);
        }
    }
    
    area_x1 = min_x; area_y1 = min_y;
    
    //Create the new areas on the vectors.
    float area_width = max_x - min_x;
    float area_height = max_y - min_y;
    unsigned area_image_cols = ceil(area_width / AREA_IMAGE_SIZE);
    unsigned area_image_rows = ceil(area_height / AREA_IMAGE_SIZE);
    
    for(size_t x = 0; x < area_image_cols; x++) {
        area_images.push_back(vector<ALLEGRO_BITMAP*>());
        
        for(size_t y = 0; y < area_image_rows; y++) {
            area_images[x].push_back(al_create_bitmap(AREA_IMAGE_SIZE, AREA_IMAGE_SIZE));
        }
    }
    
    //For every sector, draw it on the area images it belongs on.
    for(size_t s = 0; s < n_sectors; s++) {
        size_t n_linedefs = sectors[s].linedefs.size();
        if(n_linedefs == 0) continue;
        
        float s_min_x, s_max_x, s_min_y, s_max_y;
        unsigned sector_start_col, sector_end_col, sector_start_row, sector_end_row;
        s_min_x = s_max_x = sectors[s].linedefs[0]->x1;
        s_min_y = s_max_y = sectors[s].linedefs[0]->y1;
        
        for(size_t l = 1; l < n_linedefs; l++) { //Start at 1, because we already have the first linedef's values.
            float x = sectors[s].linedefs[l]->x1;
            float y = sectors[s].linedefs[l]->y1;
            
            s_min_x = min(x, s_min_x);
            s_max_x = max(x, s_max_x);
            s_min_y = min(y, s_min_y);
            s_max_y = max(y, s_max_y);
        }
        
        sector_start_col = (s_min_x - area_x1) / AREA_IMAGE_SIZE;
        sector_end_col =   ceil((s_max_x - area_x1) / AREA_IMAGE_SIZE) - 1;
        sector_start_row = (s_min_y - area_y1) / AREA_IMAGE_SIZE;
        sector_end_row =   ceil((s_max_y - area_y1) / AREA_IMAGE_SIZE) - 1;
        
        for(size_t x = sector_start_col; x <= sector_end_col; x++) {
            for(size_t y = sector_start_row; y <= sector_end_row; y++) {
                ALLEGRO_BITMAP* current_target_bmp = al_get_target_bitmap();
                al_set_target_bitmap(area_images[x][y]); {
                
                    draw_sector(sectors[s], x * AREA_IMAGE_SIZE + area_x1, y * AREA_IMAGE_SIZE + area_y1);
                    
                } al_set_target_bitmap(current_target_bmp);
            }
        }
        
    }
    
}

/* ----------------------------------------------------------------------------
 * Returns the burrowed Pikmin closest to a leader. Used when auto-plucking.
 * x/y:             Coordinates of the leader.
 * d:               Variable to return the distance to. NULL for none.
 * ignore_reserved: If true, ignore any burrowed Pikmin that are "reserved"
 ** (i.e. already chosen to be plucked by another leader).
 */
pikmin* get_closest_burrowed_pikmin(float x, float y, float* d, bool ignore_reserved) {
    float closest_distance = 0;
    pikmin* closest_pikmin = NULL;
    
    size_t n_pikmin = pikmin_list.size();
    for(size_t p = 0; p < n_pikmin; p++) {
        if(!pikmin_list[p]->burrowed) continue;
        
        float dis = dist(x, y, pikmin_list[p]->x, pikmin_list[p]->y);
        if(closest_pikmin == NULL || dis < closest_distance) {
        
            if(!(ignore_reserved && pikmin_list[p]->pluck_reserved)) {
                closest_distance = dis;
                closest_pikmin = pikmin_list[p];
            }
        }
    }
    
    if(d) *d = closest_distance;
    return closest_pikmin;
}

/* ----------------------------------------------------------------------------
 * Returns the daylight effect color for the current time, for the current weather.
 */
ALLEGRO_COLOR get_daylight_color() {
    //ToDo find out how to get the iterator to give me the value of the next point, instead of putting all points in a vector.
    vector<unsigned> point_nrs;
    for(map<unsigned, ALLEGRO_COLOR>::iterator p_nr = cur_weather.lighting.begin(); p_nr != cur_weather.lighting.end(); p_nr++) {
        point_nrs.push_back(p_nr->first);
    }
    
    size_t n_points = point_nrs.size();
    if(n_points > 1) {
        for(size_t p = 0; p < n_points - 1; p++) {
            if(day_minutes >= point_nrs[p] && day_minutes < point_nrs[p + 1]) {
                return interpolate_color(
                           day_minutes,
                           point_nrs[p],
                           point_nrs[p + 1],
                           cur_weather.lighting[point_nrs[p]],
                           cur_weather.lighting[point_nrs[p + 1]]
                       );
            }
        }
    }
    
    //If anything goes wrong, don't apply lighting at all.
    return al_map_rgba(0, 0, 0, 0);
}

/* ----------------------------------------------------------------------------
 * Returns the distance between a leader and the center of its group.
 */
float get_leader_to_group_center_dist(mob* l) {
    return
        (l->party->party_spots->current_wheel + 1) *
        l->party->party_spots->spot_radius +
        (l->party->party_spots->current_wheel + 1) *
        PARTY_SPOT_INTERVAL;
}

/* ----------------------------------------------------------------------------
 * Returns the pointer to a mob event, if the mob is listening to that event.
 */
mob_event* get_mob_event(mob* m, unsigned char e) {
    size_t n_events = m->type->events.size();
    for(size_t ev_nr = 0; ev_nr < n_events; ev_nr++) {
        mob_event* ev = m->type->events[ev_nr];
        if(ev->type == e) return ev;
    }
    return NULL;
}

/* ----------------------------------------------------------------------------
 * Returns an ALLEGRO_TRANSFORM that transforms world coordinates into screen coordinates.
 */
ALLEGRO_TRANSFORM get_world_to_screen_transform() {
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_translate_transform(
        &t,
        -cam_x + scr_w / 2 * 1 / (cam_zoom),
        -cam_y + scr_h / 2 * 1 / (cam_zoom)
    );
    al_scale_transform(&t, cam_zoom, cam_zoom);
    return t;
}

/* ----------------------------------------------------------------------------
 * Gives an Onion some Pikmin, and makes the Onion spew seeds out,
 ** depending on how many Pikmin there are in the field (don't spew if 100).
 */
void give_pikmin_to_onion(onion* o, unsigned amount) {
    unsigned total_after = pikmin_list.size() + amount;
    unsigned pikmin_to_spit = amount;
    unsigned pikmin_to_keep = 0; //Pikmin to keep inside the Onion, without spitting.
    
    if(total_after > max_pikmin_in_field) {
        pikmin_to_keep = total_after - max_pikmin_in_field;
        pikmin_to_spit = amount - pikmin_to_keep;
    }
    
    for(unsigned p = 0; p < pikmin_to_spit; p++) {
        float angle = random(0, M_PI * 2);
        float x = o->x + cos(angle) * o->type->size * 2;
        float y = o->y + sin(angle) * o->type->size * 2;
        
        //ToDo throw them, don't teleport them.
        pikmin* new_pikmin = new pikmin(x, y, o->sec, o->oni_type->pik_type);
        new_pikmin->burrowed = true;
        create_mob(new_pikmin);
    }
    
    for(unsigned p = 0; p < pikmin_to_keep; p++) {
        pikmin_in_onions[o->oni_type->pik_type]++;
    }
}

/* ----------------------------------------------------------------------------
 * Returns the interpolation between two colors, given a number in an interval.
 * n: The number.
 * n1, n2: The interval the number falls on.
 ** The closer to n1, the closer the final color is to c1.
 * c1, c2: Colors.
 */
ALLEGRO_COLOR interpolate_color(float n, float n1, float n2, ALLEGRO_COLOR c1, ALLEGRO_COLOR c2) {
    float progress = (float) (n - n1) / (float) (n2 - n1);
    return al_map_rgba_f(
               c1.r + progress * (c2.r - c1.r),
               c1.g + progress * (c2.g - c1.g),
               c1.b + progress * (c2.b - c1.b),
               c1.a + progress * (c2.a - c1.a)
           );
}

/* ----------------------------------------------------------------------------
 * Loads an area into memory.
 */
void load_area(string name) {
    sectors.clear();
    
    data_node file = load_data_file(AREA_FOLDER "/" + name + ".txt");
    
    string weather_condition_name = trim_spaces(file["weather"].get_value());
    if(weather_conditions.find(weather_condition_name) == weather_conditions.end()) {
        error_log("Area " + name + " refers to a non-existing weather condition!");
        cur_weather = weather();
    } else {
        cur_weather = weather_conditions[weather_condition_name];
    }
    
    size_t n_sectors = file["sector"].size();
    for(size_t s = 0; s < n_sectors; s++) {
        data_node sector_data = file["sector"][s];
        sector new_sector = sector();
        
        size_t n_floors = sector_data["floor"].size();
        if(n_floors > 2) n_floors = 2;
        for(size_t f = 0; f < n_floors; f++) {  //ToDo this is not the way to do it.
            data_node floor_data = sector_data["floor"][f];
            floor_info new_floor = floor_info();
            
            new_floor.brightness = tof(floor_data["brightness"].get_value("1"));
            new_floor.rot = tof(floor_data["texture_rotate"].get_value());
            new_floor.scale = tof(floor_data["texture_scale"].get_value());
            new_floor.trans_x = tof(floor_data["texture_trans_x"].get_value());
            new_floor.trans_y = tof(floor_data["texture_trans_y"].get_value());
            new_floor.texture = load_bmp("Textures/" + floor_data["texture"].get_value());  //ToDo don't load it every time.
            new_floor.z = tof(floor_data["z"].get_value().c_str());
            //ToDo terrain sound.
            
            new_sector.floors[f] = new_floor;
        }
        
        size_t n_linedefs = sector_data["linedef"].size();
        for(size_t l = 0; l < n_linedefs; l++) {
            data_node linedef_data = sector_data["linedef"][l];
            linedef* new_linedef = new linedef();
            
            new_linedef->x1 = tof(linedef_data["x"].get_value());
            new_linedef->y1 = tof(linedef_data["y"].get_value());
            
            if(new_sector.linedefs.size()) {
                new_linedef->x2 = new_sector.linedefs.back()->x1;
                new_linedef->y2 = new_sector.linedefs.back()->y1;
            }
            
            //ToDo missing things.
            
            new_sector.linedefs.push_back(new_linedef);
        }
        
        if(new_sector.linedefs.size() > 2) {
            new_sector.linedefs[0]->x2 = new_sector.linedefs.back()->x1;
            new_sector.linedefs[0]->y2 = new_sector.linedefs.back()->y1;
        }
        
        //ToDo missing things.
        
        sectors.push_back(new_sector);
    }
}

/* ----------------------------------------------------------------------------
 * Loads a bitmap from the game's content.
 */
ALLEGRO_BITMAP* load_bmp(string filename) {
    ALLEGRO_BITMAP* b = NULL;
    b = al_load_bitmap((GRAPHICS_FOLDER "/" + filename).c_str());
    if(!b) {
        error_log("Could not open image " + filename + "!");
        b = bmp_error;
    }
    
    return b;
}

/* ----------------------------------------------------------------------------
 * Loads a game control.
 */
void load_control(unsigned char action, unsigned char player, string name, data_node &file, string def) {
    string s = file["p" + to_string((long long) (player + 1)) + "_" + name].get_value((player == 0) ? def : "");
    vector<string> possible_controls = split(s, ",");
    size_t n_possible_controls = possible_controls.size();
    
    for(size_t c = 0; c < n_possible_controls; c++) {
        controls.push_back(control_info(action, player, possible_controls[c]));
    }
}

/* ----------------------------------------------------------------------------
 * Loads a data file from the game's content.
 */
data_node load_data_file(string filename) {
    data_node n = data_node(filename);
    if(!n.file_was_opened) {
        error_log("Could not open data file " + filename + "!");
    }
    
    return n;
}

/* ----------------------------------------------------------------------------
 * Loads all of the game's content.
 */
void load_game_content() {
    //ToDo.
    pikmin_type* red_pikmin = new pikmin_type();
    red_pikmin->name = "Red Pikmin";
    red_pikmin->color = al_map_rgb(255, 0, 0);
    red_pikmin->move_speed = 80;
    red_pikmin->size = 20;
    pikmin_types["Red Pikmin"] = red_pikmin;
    
    pikmin_type* yellow_pikmin = new pikmin_type();
    yellow_pikmin->name = "Yellow Pikmin";
    yellow_pikmin->color = al_map_rgb(255, 255, 0);
    yellow_pikmin->move_speed = 80;
    yellow_pikmin->size = 20;
    pikmin_types["Yellow Pikmin"] = yellow_pikmin;
    
    pikmin_type* blue_pikmin = new pikmin_type();
    blue_pikmin->name = "Blue Pikmin";
    blue_pikmin->color = al_map_rgb(0, 0, 255);
    blue_pikmin->move_speed = 80;
    blue_pikmin->size = 20;
    pikmin_types["Blue Pikmin"] = blue_pikmin;
    
    /*
    pikmin_types.push_back(pikmin_type());
    pikmin_types.back().color = al_map_rgb(255, 255, 255);
    pikmin_types.back().name = "W";
    pikmin_types.back().move_speed = 100;
    pikmin_types.back().has_onion = false;
    
    pikmin_types.push_back(pikmin_type());
    pikmin_types.back().color = al_map_rgb(64, 0, 255);
    pikmin_types.back().name = "P";
    pikmin_types.back().move_speed = 60;
    pikmin_types.back().has_onion = false;
    */
    
    leader_type* normal_leader_type = new leader_type();
    normal_leader_type->move_speed = LEADER_MOVE_SPEED;
    normal_leader_type->size = 32;
    normal_leader_type->weight = 1;
    leader_types["Normal"] = normal_leader_type;
    
    onion_type* red_onion_type = new onion_type(pikmin_types["Red Pikmin"]);
    red_onion_type->size = 32;
    red_onion_type->name = "Red";
    onion_types["Red"] = red_onion_type;
    onion_type* yellow_onion_type = new onion_type(pikmin_types["Yellow Pikmin"]);
    yellow_onion_type->size = 32;
    yellow_onion_type->name = "Yellow";
    onion_types["Yellow"] = yellow_onion_type;
    onion_type* blue_onion_type = new onion_type(pikmin_types["Blue Pikmin"]);
    blue_onion_type->size = 32;
    blue_onion_type->name = "Blue";
    onion_types["Blue"] = blue_onion_type;
    
    treasure_type* normal_treasure_type = new treasure_type(100, 40, 50);
    treasure_types["Test"] = normal_treasure_type;
    
    statuses.push_back(status(0, 0, 1, 0, true, al_map_rgb(128, 0, 255), STATUS_AFFECTS_ENEMIES));
    statuses.push_back(status(1.5, 1.5, 1, 1, false, al_map_rgb(255, 64, 64), STATUS_AFFECTS_PIKMIN));
    
    spray_types.push_back(spray_type(&statuses[0], false, 10, al_map_rgb(128, 0, 255), NULL, NULL));
    spray_types.push_back(spray_type(&statuses[1], true, 40, al_map_rgb(255, 0, 0), NULL, NULL));
    
    pellet_types["Red 1"] = new pellet_type(pikmin_types["Red Pikmin"], 32, 2, 1, 2, 1);
    pellet_types["Red 5"] = new pellet_type(pikmin_types["Red Pikmin"], 64, 10, 5, 5, 3);
    pellet_types["Red 10"] = new pellet_type(pikmin_types["Red Pikmin"], 96, 20, 10, 10, 5);
    pellet_types["Red 20"] = new pellet_type(pikmin_types["Red Pikmin"], 128, 50, 20, 20, 10);
    
    //Mob types.
    vector<string> enemy_files = folder_to_vector(ENEMIES_FOLDER, false);
    for(size_t f = 0; f < enemy_files.size(); f++) {
        load_mob_type(ENEMIES_FOLDER "/" + enemy_files[f], true);
    }
    
    //Weather.
    weather_conditions.clear();
    data_node weather_file = load_data_file(WEATHER_FILE);
    size_t n_weather_conditions = weather_file["weather"].size();
    
    for(size_t wc = 0; wc < n_weather_conditions; wc++) {
        data_node* cur_weather = &weather_file["weather"][wc];
        
        string name = trim_spaces(cur_weather->operator[]("name").get_value());
        if(name.size() == 0) name = "default";
        
        map<unsigned, ALLEGRO_COLOR> lighting;
        size_t n_lighting_points = cur_weather->operator[]("lighting")[0].size();
        
        for(size_t lp = 0; lp < n_lighting_points; lp++) {
            string node_name;
            string node_value = cur_weather->operator[]("lighting")[0].get_node_list_by_nr(lp, &node_name).get_value();
            
            unsigned point_time = toi(node_name);
            ALLEGRO_COLOR point_color = toc(node_value);
            
            lighting[point_time] = point_color;
        }
        
        if(lighting.size() == 0) {
            error_log("Weather condition " + name + " has no lighting!");
        } else {
            if(lighting.find(24 * 60) == lighting.end()) {
                //If there is no data for the last hour, use the data from the first point (this is because the day loops after 24:00; needed for interpolation)
                lighting[24 * 60] = lighting.begin()->second;
            }
        }
        
        unsigned char percipitation_type = toi(cur_weather->operator[]("percipitation_type").get_value(to_string((long long) PERCIPITATION_TYPE_NONE)));
        interval percipitation_frequency = interval(cur_weather->operator[]("percipitation_frequency").get_value());
        interval percipitation_speed = interval(cur_weather->operator[]("percipitation_speed").get_value());
        interval percipitation_angle = interval(cur_weather->operator[]("percipitation_angle").get_value(to_string((long double) (M_PI + M_PI_2))));
        
        weather_conditions[name] = weather(name, lighting, percipitation_type, percipitation_frequency, percipitation_speed, percipitation_angle);
    }
}

/* ----------------------------------------------------------------------------
 * Loads a mob type from a file.
 */
void load_mob_type(string filename, bool enemy) {
    data_node file = data_node(filename);
    if(!file.file_was_opened) return;
    
    mob_type* mt = (enemy ? (new enemy_type()) : (new mob_type()));
    
    mt->always_active = tob(file["always_active"].get_value("no"));
    mt->max_health = toi(file["max_health"].get_value("0"));
    mt->move_speed = tof(file["move_speed"].get_value("0"));
    mt->name = trim_spaces(file["name"].get_value());
    mt->near_radius = tof(file["near_radius"].get_value("0"));
    mt->rotation_speed = tof(file["rotation_speed"].get_value("0"));
    mt->sight_radius = tof(file["sight_radius"].get_value("0"));
    mt->size = tof(file["size"].get_value("0"));
    mt->weight = toi(file["weight"].get_value("0"));
    
    if(enemy) {
        enemy_type* et = (enemy_type*) mt;
        et->can_regenerate = tob(file["can_regenerate"].get_value("no"));
        et->drops_corpse = tob(file["drops_corpse"].get_value("yes"));
        et->is_boss = tob(file["is_boss"].get_value("no"));
        et->pikmin_seeds = toi(file["pikmin_seeds"].get_value("0"));
        et->revive_speed = tof(file["revive_speed"].get_value("0"));
        et->value = tof(file["value"].get_value("0"));
    }
    
    mt->events = load_script(file["script"][0]);
    
    mob_types.push_back(mt);
}

/* ----------------------------------------------------------------------------
 * Loads the player's options.
 */
void load_options() {
    data_node file = data_node("Options.txt");
    if(!file.file_was_opened) return;
    
    //Load joysticks.
    joystick_numbers.clear();
    int n_joysticks = al_get_num_joysticks();
    for(int j = 0; j < n_joysticks; j++) {
        joystick_numbers[al_get_joystick(j)] = j;
    }
    
    //Load controls.
    //Format of a control: "p<player number>_<action>=<possible control 1>,<possible control 2>,<...>"
    //Format of a possible control: "<input method>_<parameters, underscore separated>"
    //Input methods: "k" (keyboard key), "mb" (mouse button), "mwu" (mouse wheel up), "mwd" (down),
    //"mwl" (left), "mwr" (right), "jb" (joystick button), "jap" (joystick axis, positive), "jan" (joystick axis, negative).
    //The parameters are the key/button number, joystick number, joystick stick and axis, etc.
    //Check the constructor of control_info for more information.
    controls.clear();
    
    for(unsigned char p = 0; p < 4; p++) {
        load_control(BUTTON_PUNCH,                p, "punch", file, "mb_1");
        load_control(BUTTON_WHISTLE,              p, "whistle", file, "mb_2");
        load_control(BUTTON_MOVE_RIGHT,           p, "move_right", file, "k_4");
        load_control(BUTTON_MOVE_UP,              p, "move_up", file, "k_23");
        load_control(BUTTON_MOVE_LEFT,            p, "move_left", file, "k_1");
        load_control(BUTTON_MOVE_DOWN,            p, "move_down", file, "k_19");
        load_control(BUTTON_MOVE_CURSOR_RIGHT,    p, "move_cursor_right", file, "");
        load_control(BUTTON_MOVE_CURSOR_UP,       p, "move_cursor_up", file, "");
        load_control(BUTTON_MOVE_CURSOR_LEFT,     p, "move_cursor_left", file, "");
        load_control(BUTTON_MOVE_CURSOR_DOWN,     p, "move_cursor_down", file, "");
        load_control(BUTTON_MOVE_GROUP_TO_CURSOR, p, "move_group_to_cursor", file, "k_75");
        load_control(BUTTON_MOVE_GROUP_RIGHT,     p, "move_group_right", file, "");
        load_control(BUTTON_MOVE_GROUP_UP,        p, "move_group_up", file, "");
        load_control(BUTTON_MOVE_GROUP_LEFT,      p, "move_group_left", file, "");
        load_control(BUTTON_MOVE_GROUP_DOWN,      p, "move_group_down", file, "");
        load_control(BUTTON_SWITCH_CAPTAIN_RIGHT, p, "switch_captain_right", file, "k_64");
        load_control(BUTTON_SWITCH_CAPTAIN_LEFT,  p, "switch_captain_left", file, "");
        load_control(BUTTON_DISMISS,              p, "dismiss", file, "k_217");
        load_control(BUTTON_USE_SPRAY_1,          p, "use_spray_1", file, "k_18");
        load_control(BUTTON_USE_SPRAY_2,          p, "use_spray_2", file, "k_6");
        load_control(BUTTON_USE_SPRAY,            p, "use_spray", file, "k_18");
        load_control(BUTTON_SWITCH_SPRAY_RIGHT,   p, "switch_spray_right", file, "k_5");
        load_control(BUTTON_SWITCH_SPRAY_LEFT,    p, "switch_spray_left", file, "k_17");
        load_control(BUTTON_SWITCH_ZOOM,          p, "switch_zoom", file, "k_3");
        load_control(BUTTON_ZOOM_IN,              p, "zoom_in", file, "mwu");
        load_control(BUTTON_ZOOM_OUT,             p, "zoom_out", file, "mwd");
        load_control(BUTTON_SWITCH_TYPE_RIGHT,    p, "switch_type_right", file, "");
        load_control(BUTTON_SWITCH_TYPE_LEFT,     p, "switch_type_left", file, "");
        load_control(BUTTON_SWITCH_MATURITY_UP,   p, "switch_maturity_up", file, "");
        load_control(BUTTON_SWITCH_MATURITY_DOWN, p, "switch_maturity_down", file, "");
        load_control(BUTTON_LIE_DOWN,             p, "lie_down", file, "k_26");
        load_control(BUTTON_PAUSE,                p, "pause", file, "k_59");
    }
    
    //Weed out controls that didn't parse correctly.
    size_t n_controls = controls.size();
    for(size_t c = 0; c < n_controls; ) {
        if(controls[c].action == BUTTON_NONE) {
            controls.erase(controls.begin() + c);
        } else {
            c++;
        }
    }
    
    for(unsigned char p = 0; p < 4; p++) {
        mouse_moves_cursor[p] = tob(file["p" + to_string((long long) (p + 1)) + "_mouse_moves_cursor"].get_value((p == 0) ? "true" : "false"));
    }
    
    //Other options.
    daylight_effect = tob(file["daylight_effect"].get_value("true"));
    game_fps = toi(file["fps"].get_value("30"));
    scr_h = toi(file["height"].get_value(to_string((long long) DEF_SCR_H)));
    particle_quality = toi(file["particle_quality"].get_value("2"));
    pretty_whistle = tob(file["pretty_whistle"].get_value("true"));
    scr_w = toi(file["width"].get_value(to_string((long long) DEF_SCR_W)));
    smooth_scaling = tob(file["smooth_scaling"].get_value("true"));
}

/* ----------------------------------------------------------------------------
 * Loads an audio sample from the game's content.
 */
sample_struct load_sample(string filename) {
    sample_struct s;
    s.sample = al_load_sample((AUDIO_FOLDER "/" + filename).c_str());
    if(!s.sample) {
        error_log("Could not open audio sample " + filename + "!");
    }
    
    return s;
}

/* ----------------------------------------------------------------------------
 * Loads a script from a data node.
 */
vector<mob_event*> load_script(data_node node) {

    vector<mob_event*> events;
    
    for(size_t e = 0; e < node.size(); e++) {
        string event_name;
        unsigned char event_type = 0;
        data_node* event_node = &node.get_node_list_by_nr(e, &event_name)[0];
        event_name = trim_spaces(event_name);
        
        if(event_name == "on_attack_hit") event_type = MOB_EVENT_ATTACK_HIT;
        else if(event_name == "on_attack_miss") event_type = MOB_EVENT_ATTACK_MISS;
        else if(event_name == "on_big_damage") event_type = MOB_EVENT_BIG_DAMAGE;
        else if(event_name == "on_damage") event_type = MOB_EVENT_DAMAGE;
        else if(event_name == "on_death") event_type = MOB_EVENT_DEATH;
        else if(event_name == "on_enter_hazard") event_type = MOB_EVENT_ENTER_HAZARD;
        else if(event_name == "on_idle") event_type = MOB_EVENT_IDLE;
        else if(event_name == "on_leave_hazard") event_type = MOB_EVENT_LEAVE_HAZARD;
        else if(event_name == "on_lose_object") event_type = MOB_EVENT_LOSE_OBJECT;
        else if(event_name == "on_lose_pikmin") event_type = MOB_EVENT_LOSE_PIKMIN;
        else if(event_name == "on_near_object") event_type = MOB_EVENT_NEAR_OBJECT;
        else if(event_name == "on_near_pikmin") event_type = MOB_EVENT_NEAR_PIKMIN;
        else if(event_name == "on_pikmin_land") event_type = MOB_EVENT_PIKMIN_LAND;
        else if(event_name == "on_pikmin_latch") event_type = MOB_EVENT_PIKMIN_LATCH;
        else if(event_name == "on_pikmin_touch") event_type = MOB_EVENT_PIKMIN_TOUCH;
        else if(event_name == "on_revival") event_type = MOB_EVENT_REVIVAL;
        else if(event_name == "on_see_object") event_type = MOB_EVENT_SEE_OBJECT;
        else if(event_name == "on_see_pikmin") event_type = MOB_EVENT_SEE_PIKMIN;
        else if(event_name == "on_spawn") event_type = MOB_EVENT_SPAWN;
        else if(event_name == "on_timer") event_type = MOB_EVENT_TIMER;
        else if(event_name == "on_wall") event_type = MOB_EVENT_WALL;
        else {
            error_log("Unknown script event name \"" + event_name + "\"!");
            continue;
        }
        
        vector<mob_action*> actions;
        
        for(size_t a = 0; a < event_node->size(); a++) {
        
            string action_name;
            unsigned char action_type = 0;
            string action_data = event_node->get_node_list_by_nr(a, &action_name).get_value();
            action_name = trim_spaces(action_name);
            action_data = trim_spaces(action_data);
            
            if(action_name == "move") action_type = MOB_ACTION_MOVE;
            else if(action_name == "play_sound") action_type = MOB_ACTION_PLAY_SOUND;
            else if(action_name == "animation") action_type = MOB_ACTION_SET_ANIMATION;
            else if(action_name == "gravity") action_type = MOB_ACTION_SET_GRAVITY;
            else if(action_name == "health") action_type = MOB_ACTION_SET_HEALTH;
            else if(action_name == "speed") action_type = MOB_ACTION_SET_SPEED;
            else if(action_name == "timer") action_type = MOB_ACTION_SET_TIMER;
            else if(action_name == "var") action_type = MOB_ACTION_SET_VAR;
            else if(action_name == "particle") action_type = MOB_ACTION_SPAWN_PARTICLE;
            else if(action_name == "projectile") action_type = MOB_ACTION_SPAWN_PROJECTILE;
            else if(action_name == "special_function") action_type = MOB_ACTION_SPECIAL_FUNCTION;
            else if(action_name == "turn") action_type = MOB_ACTION_TURN;
            else if(action_name == "wait") action_type = MOB_ACTION_WAIT;
            else {
                error_log("Unknown script action name \"" + action_name + "\"!");
            }
            
            actions.push_back(new mob_action(action_type, action_data));
            
        }
        
        events.push_back(new mob_event(event_type, actions));
        
    }
    
    return events;
}

/* ----------------------------------------------------------------------------
 * Makes a mob impossible to be carried, and makes the Pikmin carrying it drop it.
 */
void make_uncarriable(mob* m) {
    if(!m->carrier_info) return;
    
    delete m->carrier_info;
    m->carrier_info = NULL;
}

/* ----------------------------------------------------------------------------
 * Returns the movement necessary to move a point.
 * x/y:          Coordinates of the initial point.
 * tx/ty:        Coordinates of the target point.
 * speed:        Speed at which the point can move.
 * reach_radius: If the point is within this range of the target, consider it as already being there.
 * mx/my:        Variables to return the amount of movement to.
 * angle:        Variable to return the angle the point faces to.
 * reached:      Variable to return whether the point reached the target or not to.
 */
void move_point(float x, float y, float tx, float ty, float speed, float reach_radius, float* mx, float* my, float* angle, bool* reached) {
    float dx = tx - x, dy = ty - y;
    float dist = sqrt(dx * dx + dy * dy);
    
    if(dist > reach_radius) {
        float move_amount = min(dist * game_fps / 2, speed);
        
        dx *= move_amount / dist;
        dy *= move_amount / dist;
        
        if(mx) *mx = dx;
        if(my) *my = dy;
        if(angle) *angle = atan2(dy, dx);
        if(reached) *reached = false;
    } else {
        if(mx) *mx = 0;
        if(my) *my = 0;
        if(reached) *reached = true;
    }
}

/* ----------------------------------------------------------------------------
 * Plucks a Pikmin from the ground, if possible, and adds it to a leader's group.
 */
void pluck_pikmin(leader* l, pikmin* p) {
    if(!p->burrowed) return;
    
    p->burrowed = false;
    add_to_party(l, p);
    al_play_sample(sfx_pikmin_plucked.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_pikmin_plucked.id);
}

/* ----------------------------------------------------------------------------
 * Returns a random number between the provided range, inclusive.
 */
inline float random(float min, float max) {
    if(max == min) return min;
    return (float) rand() / ((float) RAND_MAX / (max - min)) + min;
}

/* ----------------------------------------------------------------------------
 * Generates random particles in an explosion fashion:
 ** they scatter from the center point at random angles,
 ** and drift off until they vanish.
 * center_*: Center point of the explosion.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_explosion(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color) {
    unsigned char n_particles = random(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        float angle = (random(0, (unsigned) (M_PI * 2) * 100)) / 100.0;
        
        float speed_x = cos(angle) * 30;
        float speed_y = sin(angle) * 30;
        
        particles.push_back(particle(
                                center_x,
                                center_y,
                                speed_x,
                                speed_y,
                                1,
                                0,
                                (random((unsigned) (time_min * 100), (unsigned) (time_max * 100))) / 100.0,
                                (random((unsigned) (size_min * 100), (unsigned) (size_max * 100))) / 100.0,
                                color
                            ));
    }
}

/* ----------------------------------------------------------------------------
 * Generates random particles in a fire fashion:
 ** the particles go up and speed up as time goes by.
 * center_*: Center point of the fire.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_fire(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color) {
    unsigned char n_particles = random(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        particles.push_back(particle(
                                center_x,
                                center_y,
                                (6 - random(0, 12)),
                                -(random(10, 20)),
                                0,
                                -1,
                                (random((unsigned) (time_min * 100), (unsigned) (time_max * 100))) / 100.0,
                                (random((unsigned) (size_min * 100), (unsigned) (size_max * 100))) / 100.0,
                                color
                            ));
    }
}

/* ----------------------------------------------------------------------------
 * Generates random particles in a splash fashion:
 ** the particles go up and are scattered horizontally,
 ** and then go down with the effect of gravity.
 * center_*: Center point of the splash.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_splash(float center_x, float center_y, unsigned char min, unsigned char max, float time_min, float time_max, float size_min, float size_max, ALLEGRO_COLOR color) {
    unsigned char n_particles = random(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        particles.push_back(particle(
                                center_x,
                                center_y,
                                (2 - random(0, 4)),
                                -random(2, 4),
                                0, 0.5,
                                random((int)(time_min * 10), (int)(time_max * 10)) / 10,
                                random((int)(size_min * 10), (int)(size_max * 10)) / 10,
                                color
                            ));
    }
}

/* ----------------------------------------------------------------------------
 * Generates random particles in a spray fashion:
 ** the particles go in the pointed direction,
 ** and move gradually slower as they fade into the air.
 ** Used on actual sprays in-game.
 * origin_*: Origin point of the spray.
 * angle:    Angle to shoot at.
 * color:    Color of the particles.
 */
void random_particle_spray(float origin_x, float origin_y, float angle, ALLEGRO_COLOR color) {
    unsigned char n_particles = random(35, 40);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        float angle_offset = ((random(0, (unsigned) (M_PI_2 * 100))) / 100.0) - M_PI_4;
        
        float power = random(30, 90);
        float speed_x = cos(angle + angle_offset) * power;
        float speed_y = sin(angle + angle_offset) * power;
        
        particles.push_back(particle(
                                origin_x,
                                origin_y,
                                speed_x,
                                speed_y,
                                1,
                                0,
                                (random(30, 40)) / 10.0,
                                (random(60, 80)) / 10.0,
                                color
                            ));
    }
}

/* ----------------------------------------------------------------------------
 * Removes a mob from its leader's party.
 */
void remove_from_party(mob* member) {
    if(!member->following_party) return;
    
    member->following_party->party->members.erase(find(
                member->following_party->party->members.begin(),
                member->following_party->party->members.end(),
                member));
                
    if(member->following_party->party->party_spots) {
        member->following_party->party->party_spots->remove(member);
    }
    
    member->following_party = NULL;
    member->remove_target(false);
    member->uncallable_period = UNCALLABLE_PERIOD;
}

/* ----------------------------------------------------------------------------
 * Saves the player's options.
 */
void save_options() {
    //ToDo make this prettier. Like a list of constants somewhere where it associates an action with the name on the text file.
    ALLEGRO_FILE* file = al_fopen("Options.txt", "w");
    
    if(!file) return;
    
    //First, group the controls by action and player.
    map<string, string> grouped_controls;
    
    //Tell the map what they are.
    for(unsigned char p = 0; p < 4; p++) {
        string prefix = "p" + to_string((long long) (p + 1)) + "_";
        grouped_controls[prefix + "punch"] = "";
        grouped_controls[prefix + "whistle"] = "";
        grouped_controls[prefix + "move_right"] = "";
        grouped_controls[prefix + "move_up"] = "";
        grouped_controls[prefix + "move_left"] = "";
        grouped_controls[prefix + "move_down"] = "";
        grouped_controls[prefix + "move_cursor_right"] = "";
        grouped_controls[prefix + "move_cursor_up"] = "";
        grouped_controls[prefix + "move_cursor_left"] = "";
        grouped_controls[prefix + "move_cursor_down"] = "";
        grouped_controls[prefix + "move_group_right"] = "";
        grouped_controls[prefix + "move_group_up"] = "";
        grouped_controls[prefix + "move_group_left"] = "";
        grouped_controls[prefix + "move_group_down"] = "";
        grouped_controls[prefix + "move_group_to_cursor"] = "";
        grouped_controls[prefix + "switch_captain_right"] = "";
        grouped_controls[prefix + "switch_captain_left"] = "";
        grouped_controls[prefix + "dismiss"] = "";
        grouped_controls[prefix + "use_spray_1"] = "";
        grouped_controls[prefix + "use_spray_2"] = "";
        grouped_controls[prefix + "use_spray"] = "";
        grouped_controls[prefix + "switch_spray_right"] = "";
        grouped_controls[prefix + "switch_spray_left"] = "";
        grouped_controls[prefix + "switch_zoom"] = "";
        grouped_controls[prefix + "zoom_in"] = "";
        grouped_controls[prefix + "zoom_out"] = "";
        grouped_controls[prefix + "switch_type_right"] = "";
        grouped_controls[prefix + "switch_type_left"] = "";
        grouped_controls[prefix + "switch_maturity_up"] = "";
        grouped_controls[prefix + "switch_maturity_down"] = "";
        grouped_controls[prefix + "lie_down"] = "";
        grouped_controls[prefix + "pause"] = "";
    }
    
    size_t n_controls = controls.size();
    for(size_t c = 0; c < n_controls; c++) {
        string name = "p" + to_string((long long) (controls[c].player + 1)) + "_";
        if(controls[c].action == BUTTON_PUNCH)                     name += "punch";
        else if(controls[c].action == BUTTON_WHISTLE)              name += "whistle";
        else if(controls[c].action == BUTTON_MOVE_RIGHT)           name += "move_right";
        else if(controls[c].action == BUTTON_MOVE_UP)              name += "move_up";
        else if(controls[c].action == BUTTON_MOVE_LEFT)            name += "move_left";
        else if(controls[c].action == BUTTON_MOVE_DOWN)            name += "move_down";
        else if(controls[c].action == BUTTON_MOVE_CURSOR_RIGHT)    name += "move_cursor_right";
        else if(controls[c].action == BUTTON_MOVE_CURSOR_UP)       name += "move_cursor_up";
        else if(controls[c].action == BUTTON_MOVE_CURSOR_LEFT)     name += "move_cursor_left";
        else if(controls[c].action == BUTTON_MOVE_CURSOR_DOWN)     name += "move_cursor_down";
        else if(controls[c].action == BUTTON_MOVE_GROUP_RIGHT)     name += "move_group_right";
        else if(controls[c].action == BUTTON_MOVE_GROUP_UP)        name += "move_group_up";
        else if(controls[c].action == BUTTON_MOVE_GROUP_LEFT)      name += "move_group_left";
        else if(controls[c].action == BUTTON_MOVE_GROUP_DOWN)      name += "move_group_down";
        else if(controls[c].action == BUTTON_MOVE_GROUP_TO_CURSOR) name += "move_group_to_cursor";
        else if(controls[c].action == BUTTON_SWITCH_CAPTAIN_RIGHT) name += "switch_captain_right";
        else if(controls[c].action == BUTTON_SWITCH_CAPTAIN_LEFT)  name += "switch_captain_left";
        else if(controls[c].action == BUTTON_DISMISS)              name += "dismiss";
        else if(controls[c].action == BUTTON_USE_SPRAY_1)          name += "use_spray_1";
        else if(controls[c].action == BUTTON_USE_SPRAY_2)          name += "use_spray_2";
        else if(controls[c].action == BUTTON_USE_SPRAY)            name += "use_spray";
        else if(controls[c].action == BUTTON_SWITCH_SPRAY_RIGHT)   name += "switch_spray_right";
        else if(controls[c].action == BUTTON_SWITCH_SPRAY_LEFT)    name += "switch_spray_left";
        else if(controls[c].action == BUTTON_SWITCH_ZOOM)          name += "switch_zoom";
        else if(controls[c].action == BUTTON_ZOOM_IN)              name += "zoom_in";
        else if(controls[c].action == BUTTON_ZOOM_OUT)             name += "zoom_out";
        else if(controls[c].action == BUTTON_SWITCH_TYPE_RIGHT)    name += "switch_type_right";
        else if(controls[c].action == BUTTON_SWITCH_TYPE_LEFT)     name += "switch_type_left";
        else if(controls[c].action == BUTTON_SWITCH_MATURITY_UP)   name += "switch_maturity_up";
        else if(controls[c].action == BUTTON_SWITCH_MATURITY_DOWN) name += "switch_maturity_down";
        else if(controls[c].action == BUTTON_LIE_DOWN)             name += "lie_down";
        else if(controls[c].action == BUTTON_PAUSE)                name += "pause";
        
        grouped_controls[name] += controls[c].stringify() + ",";
    }
    
    //Save controls.
    for(map<string, string>::iterator c = grouped_controls.begin(); c != grouped_controls.end(); c++) {
        if(c->second.size()) c->second.erase(c->second.size() - 1); //Remove the final character, which is always an extra comma.
        
        al_fwrite(file, c->first + "=" + c->second + "\n");
    }
    
    for(unsigned char p = 0; p < 4; p++) {
        al_fwrite(file, "p" + to_string((long long) (p + 1)) + "_mouse_moves_cursor=" + btos(mouse_moves_cursor[p]) + "\n");
    }
    
    //Other options.
    al_fwrite(file, "daylight_effect=" + btos(daylight_effect) + "\n");
    al_fwrite(file, "fps=" + to_string((long long) game_fps) + "\n");
    al_fwrite(file, "height=" + to_string((long long) scr_h) + "\n");
    al_fwrite(file, "particle_quality=" + to_string((long long) particle_quality) + "\n");
    al_fwrite(file, "pretty_whistle=" + btos(pretty_whistle) + "\n");
    al_fwrite(file, "width=" + to_string((long long) scr_w) + "\n");
    al_fwrite(file, "smooth_scaling=" + btos(smooth_scaling) + "\n");
    
    al_fclose(file);
}

/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:        The string to split.
 * del:         The delimiter. Default is space.
 * inc_empty:   If true, include empty substrings on the vector.
 ** i.e. if two delimiters come together in a row, keep an empty substring between.
 * inc_del:     If true, include the delimiters on the vector as a substring.
 */
vector<string> split(string text, string del, bool inc_empty, bool inc_del) {
    vector<string> v;
    size_t pos;
    size_t del_size = del.size();
    
    do {
        pos = text.find(del);
        if (pos != string::npos) {  //If it DID find the delimiter.
            //Get the text between the start and the delimiter.
            string sub = text.substr(0, pos);
            
            //Add the text before the delimiter to the vector.
            if(sub != "" || inc_empty)
                v.push_back(sub);
                
            //Add the delimiter to the vector, but only if requested.
            if(inc_del)
                v.push_back(del);
                
            text.erase(text.begin(), text.begin() + pos + del_size);    //Delete everything before the delimiter, including the delimiter itself, and search again.
        }
    } while (pos != string::npos);
    
    //Text after the final delimiter. (If there is one. If not, it's just the whole string.)
    
    if (text != "" || inc_empty) //If it's a blank string, only add it if we want empty strings.
        v.push_back(text);
        
    return v;
}

/* ----------------------------------------------------------------------------
 * Starts panning the camera towards another point.
 */
void start_camera_pan(int final_x, int final_y) {
    cam_trans_pan_initi_x = cam_x;
    cam_trans_pan_initi_y = cam_y;
    cam_trans_pan_final_x = final_x;
    cam_trans_pan_final_y = final_y;
    cam_trans_pan_time_left = CAM_TRANSITION_DURATION;
}

/* ----------------------------------------------------------------------------
 * Starts moving the camera towards another zoom level.
 */
void start_camera_zoom(float final_zoom_level) {
    cam_trans_zoom_initi_level = cam_zoom;
    cam_trans_zoom_final_level = final_zoom_level;
    cam_trans_zoom_time_left = CAM_TRANSITION_DURATION;
    
    al_stop_sample(&sfx_camera.id);
    al_play_sample(sfx_camera.sample, 1, 0.5, 1, ALLEGRO_PLAYMODE_ONCE, &sfx_camera.id);
}

/* ----------------------------------------------------------------------------
 * Makes a mob move to a spot because it's being carried.
 * m:  Mob to start moving (the treasure, for instance).
 * np: New Pikmin; the Pikmin that justed joined the carriers. Used to detect ties and tie-breaking.
 * lp: Leaving Pikmin; the Pikmin that just left the carriers. Used to detect ties and tie-breaking.
 */
void start_carrying(mob* m, pikmin* np, pikmin* lp) {
    //ToDo what if an Onion hasn't been revelead yet?
    if(!m->carrier_info) return;
    
    if(m->carrier_info->carry_to_ship) {
    
        m->set_target(
            ships[0]->x + ships[0]->type->size * 0.5 + m->type->size * 0.5 + 8,
            ships[0]->y,
            NULL,
            NULL,
            false);
        m->carrier_info->decided_type = NULL;
        
    } else {
    
        map<pikmin_type*, unsigned> type_quantity; //How many of each Pikmin type are carrying.
        vector<pikmin_type*> majority_types; //The Pikmin type with the most carriers.
        
        //First, count how many of each type there are.
        for(size_t p = 0; p < m->carrier_info->max_carriers; p++) {
            pikmin* pik_ptr = NULL;
            
            if(m->carrier_info->carrier_spots[p] == NULL) continue;
            if(typeid(*m->carrier_info->carrier_spots[p]) != typeid(pikmin)) continue;
            
            pik_ptr = (pikmin*) m->carrier_info->carrier_spots[p];
            
            if(!pik_ptr->type->has_onion) continue; //If it doesn't have an Onion, it won't even count. //ToDo what if it hasn't been discovered / Onion not on this area?
            
            if(type_quantity.find(pik_ptr->type) == type_quantity.end()) type_quantity[pik_ptr->type] = 0; //ToDo maps don't start the number at 0, so that's why I need this line, correct?
            type_quantity[pik_ptr->type]++;
        }
        
        //Then figure out what are the majority types.
        unsigned most = 0;
        for(map<pikmin_type*, unsigned>::iterator t = type_quantity.begin(); t != type_quantity.end(); t++) {
            if(t->second > most) {
                most = t->second;
                majority_types.clear();
            }
            if(t->second == most) majority_types.push_back(t->first);
        }
        
        //If we ended up with no candidates, pick a type at random, out of all possible types.
        if(majority_types.size() == 0) {
            for(auto t = pikmin_types.begin(); t != pikmin_types.end(); t++) {
                if(t->second->has_onion) { //ToDo what if it hasn't been discovered / Onion not on this area?
                    majority_types.push_back(t->second);
                }
            }
        }
        
        //Now let's pick an Onion.
        if(majority_types.size() == 0) {
            return; //ToDo warn that something went horribly wrong?
            
        } if(majority_types.size() == 1) {
            //If there's only one possible type to pick, pick it.
            m->carrier_info->decided_type = majority_types[0];
            
        } else {
            //If there's a tie, let's take a careful look.
            bool new_tie = false;
            
            //Is the Pikmin that just joined part of the majority types?
            //If so, that means this Pikmin just created a NEW tie!
            //So let's pick a random Onion again.
            if(np) {
                for(size_t mt = 0; mt < majority_types.size(); mt++) {
                    if(np->type == majority_types[mt]) {
                        new_tie = true;
                        break;
                    }
                }
            }
            
            //If a Pikmin left, check if they are related to the majority types.
            //If not, then a new tie wasn't made, no worries.
            //If it was related, a new tie was created.
            if(lp) {
                new_tie = false;
                for(size_t mt = 0; mt < majority_types.size(); mt++) {
                    if(lp->type == majority_types[mt]) {
                        new_tie = true;
                        break;
                    }
                }
            }
            
            //Check if the previously decided type belongs to one of the majorities.
            //If so, it can be chosen again, but if not, it cannot.
            bool can_continue = false;
            for(size_t mt = 0; mt < majority_types.size(); mt++) {
                if(majority_types[mt] == m->carrier_info->decided_type) {
                    can_continue = true;
                    break;
                }
            }
            if(!can_continue) m->carrier_info->decided_type = NULL;
            
            //If the Pikmin that just joined is not a part of the majorities,
            //then it had no impact on the existing ties.
            //Go with the Onion that had been decided before.
            if(new_tie || !m->carrier_info->decided_type) {
                m->carrier_info->decided_type = majority_types[random(0, majority_types.size() - 1)];
            }
        }
        
        
        //Figure out where that type's Onion is.
        size_t onion_nr = 0;
        for(; onion_nr < onions.size(); onion_nr++) {
            if(onions[onion_nr]->oni_type->pik_type == m->carrier_info->decided_type) {
                break;
            }
        }
        
        //Finally, start moving the mob.
        m->set_target(onions[onion_nr]->x, onions[onion_nr]->y, NULL, NULL, false);
    }
}

/* ----------------------------------------------------------------------------
 * Makes the current leader stop whistling.
 */
void stop_whistling() {
    if(!whistling) return;
    
    whistle_fade_time = WHISTLE_FADE_TIME;
    whistle_fade_radius = whistle_radius;
    
    whistling = false;
    whistle_radius = 0;
    whistle_max_hold = 0;
    
    al_stop_sample(&leaders[cur_leader_nr]->sfx_whistle.id);
}

/* ----------------------------------------------------------------------------
 * Converts an entire string into lowercase.
 */
string str_to_lower(string s) {
    unsigned short n_characters = s.size();
    for(unsigned short c = 0; c < n_characters; c++) {
        s[c] = tolower(s[c]);
    }
    return s;
}

/* ----------------------------------------------------------------------------
 * Returns whether a point is inside a sector or not.
 * x/y:      Coordinates of the point.
 * linedefs: Linedefs that make up the sector.
 */
/*bool temp_point_inside_sector(float x, float y, vector<linedef> &linedefs){
    //ToDo
    return true;
}*/

/* ----------------------------------------------------------------------------
 * Uses up a spray.
 */
void use_spray(size_t spray_nr) {
    if(spray_amounts[spray_nr] == 0) return;
    
    leader* cur_leader_ptr = leaders[cur_leader_nr];
    float shoot_angle = cursor_angle + ((spray_types[spray_nr].burpable) ? M_PI : 0);
    
    random_particle_spray(
        cur_leader_ptr->x + cos(shoot_angle) * cur_leader_ptr->type->size / 2,
        cur_leader_ptr->y + sin(shoot_angle) * cur_leader_ptr->type->size / 2,
        shoot_angle,
        spray_types[spray_nr].main_color
    );
    
    spray_amounts[spray_nr]--;
}

//Calls al_fwrite, but with an std::string instead of a c-string.
inline void al_fwrite(ALLEGRO_FILE* f, string s) { al_fwrite(f, s.c_str(), s.size()); }
//Converts a boolean to a string, returning either "true" or "false".
inline string btos(bool b) { return b ? "true" : "false"; }
//Converts a string to a boolean, judging by the English language words that represent true and false.
inline bool tob(string s) {
    s = str_to_lower(s);
    s = trim_spaces(s);
    if(s == "yes" || s == "true" || s == "y" || s == "t") return true;
    else return (toi(s) != 0);
}
//Converts a string to an Allegro color. Components are separated by spaces, and the final one (alpha) is optional.
ALLEGRO_COLOR toc(string s) {
    s = trim_spaces(s);
    vector<string> components = split(s);
    ALLEGRO_COLOR c = al_map_rgba(
                          ((components.size() > 0) ? toi(components[0]) : 0),
                          ((components.size() > 1) ? toi(components[1]) : 0),
                          ((components.size() > 2) ? toi(components[2]) : 0),
                          ((components.size() > 3) ? toi(components[3]) : 255)
                      );
    return c;
}
//Converts a string to a float, trimming the spaces and accepting commas or points.
inline double tof(string s) { s = trim_spaces(s); replace(s.begin(), s.end(), ',', '.'); return atof(s.c_str()); }
//Converts a string to an integer.
inline int toi(string s) { return tof(s); }
