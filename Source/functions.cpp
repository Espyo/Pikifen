#define _USE_MATH_DEFINES

#pragma warning(disable : 4996) //Disables warning about localtime being deprecated.

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
    if(leaders[cur_leader_nr]->carrier_info) {
        //Getting up.
        leaders[cur_leader_nr]->anim.change("get_up", false, false);
    }
    make_uncarriable(leaders[cur_leader_nr]);
    stop_auto_pluck(leaders[cur_leader_nr]);
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
void angle_to_coordinates(const float angle, const float magnitude, float* x_coord, float* y_coord) {
    *x_coord = cos(angle) * magnitude;
    *y_coord = sin(angle) * magnitude;
}

/* ----------------------------------------------------------------------------
 * Makes m1 attack m2.
 * Stuff like status effects and maturity (Pikmin only) are taken into account.
 */
void attack(mob* m1, mob* m2, const bool m1_is_pikmin, const float damage, const float angle, const float knockback, const float new_invuln_period, const float new_knockdown_period) {
    if(m2->invuln_period > 0) return;
    
    pikmin* p_ptr = NULL;
    float total_damage = damage;
    if(m1_is_pikmin) {
        p_ptr = (pikmin*) m1;
        total_damage += p_ptr->maturity * damage * MATURITY_POWER_MULT;
    }
    
    m2->invuln_period = new_invuln_period;
    m2->knockdown_period = new_knockdown_period;
    m2->health -= damage;
    
    if(knockback != 0) {
        m2->speed_z = 500;
        m2->speed_x = cos(angle) * knockback;
        m2->speed_y = sin(angle) * knockback;
    }
    
    //If before taking damage, the interval was dividable X times, and after it's only dividable by Y (X>Y), an interval was crossed.
    if(m2->type->big_damage_interval > 0 && m2->health != m2->type->max_health) {
        if(floor((m2->health + damage) / m2->type->big_damage_interval) > floor(m2->health / m2->type->big_damage_interval)) {
            if(get_mob_event(m2, MOB_EVENT_BIG_DAMAGE, true)) {
                m2->events_queued[MOB_EVENT_BIG_DAMAGE] = 1;
            }
        }
    }
}

/* ----------------------------------------------------------------------------
 * Returns the color that was provided, but with the alpha changed.
 * color: The color to change the alpha on.
 * a:     The new alpha, [0-255].
 */
ALLEGRO_COLOR change_alpha(const ALLEGRO_COLOR c, const unsigned char a) {
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
void coordinates_to_angle(const float x_coord, const float y_coord, float* angle, float* magnitude) {
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
    remove_from_party(m);
    vector<mob*> focusers = m->focused_by;
    for(size_t m_nr = 0; m_nr < focusers.size(); m_nr++) {
        unfocus_mob(focusers[m_nr], m, true);
    }
    
    mobs.erase(find(mobs.begin(), mobs.end(), m));
    
    if(typeid(*m) == typeid(pikmin)) {
        pikmin* p_ptr = (pikmin*) m;
        drop_mob(p_ptr);
        pikmin_list.erase(find(pikmin_list.begin(), pikmin_list.end(), p_ptr));
        
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
        
    } else if(typeid(*m) == typeid(enemy)) {
        enemies.erase(find(enemies.begin(), enemies.end(), (enemy*) m));
        
    } else {
        //ToDo warn somehow.
        
    }
    
    delete m;
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
            
            type_dismiss_angles[pikmin_ptr->pik_type] = 0;
        }
    }
    
    //For each type, calculate the angle;
    size_t n_types = type_dismiss_angles.size();
    if(n_types == 1) {
        //Small hack. If there's only one Pikmin type, dismiss them directly towards the base angle.
        type_dismiss_angles.begin()->second = M_PI_4;
    } else {
        unsigned current_type_nr = 0;
        for(auto t = type_dismiss_angles.begin(); t != type_dismiss_angles.end(); t++) {
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
            
            angle = base_angle + type_dismiss_angles[pikmin_ptr->pik_type] - M_PI_4 + M_PI;
            
            member_ptr->set_target(
                cur_leader_ptr->x + cos(angle) * DISMISS_DISTANCE,
                cur_leader_ptr->y + sin(angle) * DISMISS_DISTANCE,
                NULL,
                NULL,
                false);
        }
    }
    
    sfx_pikmin_idle.play(0, false);
    cur_leader_ptr->lea_type->sfx_dismiss.play(0, false);
    cur_leader_ptr->anim.change("dismiss", false, false);
}

/* ----------------------------------------------------------------------------
 * Draws a key or button on the screen.
 * font:  Font to use for the name.
 * c:     Info on the control.
 * x, y:  Center of the place to draw at.
 * max_*: Max width or height. Used to compress it if needed.
 */
void draw_control(const ALLEGRO_FONT* const font, const control_info c, const float x, const float y, const float max_w, const float max_h) {
    string name;
    if(c.type == CONTROL_TYPE_KEYBOARD_KEY) {
        name = al_keycode_to_name(c.button);
    } else if(c.type == CONTROL_TYPE_JOYSTICK_AXIS_NEG || c.type == CONTROL_TYPE_JOYSTICK_AXIS_POS) {
        name = "AXIS " + itos(c.stick) + " " + itos(c.axis);
        name += c.type == CONTROL_TYPE_JOYSTICK_AXIS_NEG ? "-" : "+";
    } else if(c.type == CONTROL_TYPE_JOYSTICK_BUTTON) {
        name = itos(c.button + 1);
    } else if(c.type == CONTROL_TYPE_MOUSE_BUTTON) {
        name = "M" + itos(c.button);
    } else if(c.type == CONTROL_TYPE_MOUSE_WHEEL_DOWN) {
        name = "MWD";
    } else if(c.type == CONTROL_TYPE_MOUSE_WHEEL_LEFT) {
        name = "MWL";
    } else if(c.type == CONTROL_TYPE_MOUSE_WHEEL_RIGHT) {
        name = "MWR";
    } else if(c.type == CONTROL_TYPE_MOUSE_WHEEL_UP) {
        name = "MWU";
    }
    
    int x1, y1, x2, y2;
    al_get_text_dimensions(font, name.c_str(), &x1, &y1, &x2, &y2);
    float total_width = min((float) (x2 - x1 + 4), max_w);
    float total_height = min((float) (y2 - y1 + 4), max_h);
    total_width = max(total_width, total_height);
    
    if(c.type == CONTROL_TYPE_KEYBOARD_KEY) {
        al_draw_filled_rectangle(
            x - total_width * 0.5, y - total_height * 0.5,
            x + total_width * 0.5, y + total_height * 0.5,
            al_map_rgba(255, 255, 255, 192)
        );
        al_draw_rectangle(
            x - total_width * 0.5, y - total_height * 0.5,
            x + total_width * 0.5, y + total_height * 0.5,
            al_map_rgba(160, 160, 160, 192), 2
        );
    } else {
        al_draw_filled_ellipse(x, y, total_width * 0.5, total_height * 0.5, al_map_rgba(255, 255, 255, 192));
        al_draw_ellipse(x, y, total_width * 0.5, total_height * 0.5, al_map_rgba(160, 160, 160, 192), 2);
    }
    draw_compressed_text(font, al_map_rgba(255, 255, 255, 192), x, y, ALLEGRO_ALIGN_CENTER, 1, max_w - 2, max_h - 2, name);
}

/* ----------------------------------------------------------------------------
 * Draws text on the screen, but compresses (scales) it to fit within the specified range.
 * font - flags: The parameters you'd use for al_draw_text.
 * valign:       Vertical align: 0 = top, 1 = middle, 2 = bottom.
 * max_w, max_h: The maximum width and height. Use 0 to have no limit.
 * text:         Text to draw.
 */
void draw_compressed_text(const ALLEGRO_FONT* const font, const ALLEGRO_COLOR color, const float x, const float y, const int flags, const unsigned char valign, const float max_w, const float max_h, const string text) {
    int x1, x2, y1, y2;
    al_get_text_dimensions(font, text.c_str(), &x1, &y1, &x2, &y2);
    int text_width = x2 - x1, text_height = y2 - y1;
    float scale_x = 1, scale_y = 1;
    float final_text_height = text_height;
    
    if(text_width > max_w && max_w != 0) scale_x = max_w / text_width;
    if(text_height > max_h && max_h != 0) {
        scale_y = max_h / text_height;
        final_text_height = max_h;
    }
    
    ALLEGRO_TRANSFORM scale_transform, old_transform;
    al_copy_transform(&old_transform, al_get_current_transform());
    al_identity_transform(&scale_transform);
    al_scale_transform(&scale_transform, scale_x, scale_y);
    al_translate_transform(
        &scale_transform, x,
        ((valign == 1) ? y - final_text_height * 0.5 : ((valign == 2) ? y - final_text_height : y))
    );
    al_compose_transform(&scale_transform, &old_transform);
    
    al_use_transform(&scale_transform); {
        al_draw_text(font, color, 0, 0, flags, text.c_str());
    }; al_use_transform(&old_transform);
}

/* ----------------------------------------------------------------------------
 * Draws a strength/weight fraction, in the style of Pikmin 2.
 * The strength is above the weight.
 * c*:      Center of the text.
 * current: Current strength.
 * needed:  Needed strength to lift the object (weight).
 * color:   Color of the fraction's text.
 */
void draw_fraction(const float cx, const float cy, const unsigned int current, const unsigned int needed, const ALLEGRO_COLOR color) {
    float first_y = cy - (font_h * 3) / 2;
    al_draw_text(font_value, color, cx, first_y, ALLEGRO_ALIGN_CENTER, (itos(current).c_str()));
    al_draw_text(font_value, color, cx, first_y + font_h * 0.75, ALLEGRO_ALIGN_CENTER, "-");
    al_draw_text(font_value, color, cx, first_y + font_h * 1.5, ALLEGRO_ALIGN_CENTER, (itos(needed).c_str()));
}

/* ----------------------------------------------------------------------------
 * Draws a health wheel, with a pieslice that's fuller the more HP is full.
 * c*:         Center of the wheel.
 * health:     Current amount of health of the mob who's health we're representing.
 * max_health: Maximum amount of health of the mob; health for when it's fully healed.
 * radius:     Radius of the wheel (the whole wheel, not just the pieslice).
 * just_chart: If true, only draw the actual pieslice (pie-chart). Used for leader HP on the HUD.
 */
void draw_health(const float cx, const float cy, const unsigned int health, const unsigned int max_health, const float radius, const bool just_chart) {
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
void draw_sector(sector &s, const float x, const float y) {
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
void draw_shadow(const float cx, const float cy, const float size, const float delta_z, const float shadow_stretch) {
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
 * w/h:   Final width and height. Make this -1 one one of them to keep the aspect ratio.
 * angle: Angle to rotate the sprite by.
 * tint:  Tint the sprite with this color.
 */
void draw_sprite(ALLEGRO_BITMAP* bmp, const float cx, const float cy, const float w, const float h, const float angle, const ALLEGRO_COLOR tint) {
    if(!bmp) {
        bmp = bmp_error;
    }
    
    float bmp_w = al_get_bitmap_width(bmp);
    float bmp_h = al_get_bitmap_height(bmp);
    float x_scale = (w / bmp_w);
    float y_scale = (h / bmp_h);
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        tint,
        bmp_w / 2, bmp_h / 2,
        cx, cy,
        (w == -1) ? y_scale : x_scale,
        (h == -1) ? x_scale : y_scale,
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
void draw_text_lines(const ALLEGRO_FONT* const f, const ALLEGRO_COLOR c, const float x, const float y, const int fl, const unsigned char va, const string text) {
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
    mob* m = (p->carrying_mob ? p->carrying_mob : p->wants_to_carry);
    
    if(!m) return;
    
    //ToDo optimize this instead of running through the spot vector.
    for(size_t s = 0; s < m->carrier_info->max_carriers; s++) {
        if(m->carrier_info->carrier_spots[s] == p) {
            m->carrier_info->carrier_spots[s] = NULL;
            break;
        }
    }
    m->carrier_info->current_n_carriers--;
    
    if(p->carrying_mob) {
        m->carrier_info->current_carrying_strength -= p->pik_type->carry_strength;
        
        //Did this Pikmin leaving made the mob stop moving?
        if(p->carrying_mob->carrier_info->current_carrying_strength < p->carrying_mob->type->weight) {
            p->carrying_mob->remove_target(true);
            p->carrying_mob->carrier_info->decided_type = NULL;
            p->carrying_mob->state = MOB_STATE_IDLE;
            sfx_pikmin_carrying.stop();
        } else {
            start_carrying(p->carrying_mob, NULL, p); //Enter this code so that if this Pikmin leaving broke a tie, the Onion's picked correctly.
        }
    }
    
    p->carrying_mob = NULL;
    p->wants_to_carry = NULL;
    p->remove_target(true);
}

/* ----------------------------------------------------------------------------
 * Prints something onto the error log.
 * s: String that represents the error.
 * d: If not null, this will be used to obtain the filename and line that caused the error.
 */
void error_log(string s, data_node* d) {
    //ToDo
    if(d) {
        s += " (" + d->filename;
        if(d->line_nr != 0) s += " line " + itos(d->line_nr);
        s += ")";
    }
    s += "\n";
    
    if(no_error_logs_today) {
        no_error_logs_today = false;
        time_t tt;
        time(&tt);
        struct tm t = *localtime(&tt);
        s =
            "\n" +
            itos(t.tm_year + 1900) + "/" +
            leading_zero(t.tm_mon + 1) + "/" +
            leading_zero(t.tm_mday) + " " +
            leading_zero(t.tm_hour) + ":" +
            leading_zero(t.tm_min) + ":" +
            leading_zero(t.tm_sec) +
            "\n" + s;
    }
    
    string prev_error_log;
    string line;
    ALLEGRO_FILE* file_i = al_fopen("Error_log.txt", "r");
    if(file_i) {
        while(!al_feof(file_i)) {
            getline(file_i, line);
            prev_error_log += line + "\n";
        }
        prev_error_log.erase(prev_error_log.size() - 1);
        al_fclose(file_i);
    }
    
    ALLEGRO_FILE* file_o = al_fopen("Error_log.txt", "w");
    if(file_o) {
        al_fwrite(file_o, prev_error_log + s);
        al_fclose(file_o);
    }
}

/* ----------------------------------------------------------------------------
 * Returns whether or not the string s is inside the vector of strings v.
 */
bool find_in_vector(const vector<string> v, const string s) {
    for(auto i = v.begin(); i != v.end(); i++) if(*i == s) return true;
    return false;
}

/* ----------------------------------------------------------------------------
 * Makes m1 focus on m2.
 */
void focus_mob(mob* m1, mob* m2, const bool is_near, const bool call_event) {
    unfocus_mob(m1, m1->focused_prey, false);
    
    m1->focused_prey = m2;
    m1->focused_prey_near = true;
    m2->focused_by.push_back(m1);
    
    if(call_event) {
        m1->focused_prey_near = is_near;
        m1->events_queued[MOB_EVENT_LOSE_PREY] = 0;
        m1->events_queued[MOB_EVENT_NEAR_PREY] = (is_near ? 1 : 0);
        m1->events_queued[MOB_EVENT_SEE_PREY] = (is_near ? 0 : 1);
    }
}

/* ----------------------------------------------------------------------------
 * Stores the names of all files in a folder into a vector.
 * folder_name: Name of the folder.
 * folders:     If true, only read folders. If false, only read files.
 */
vector<string> folder_to_vector(string folder_name, const bool folders) {
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
                    if(pos_fs > pos_bs || pos_bs == string::npos) pos = pos_fs;
                    
                if(pos != string::npos) entry_name = entry_name.substr(pos + 1, entry_name.size() - pos - 1);
                v.push_back(entry_name);
            }
            al_destroy_fs_entry(entry);
        }
        al_close_directory(folder);
        al_destroy_fs_entry(folder);
    }
    
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
 * Returns the buried Pikmin closest to a leader. Used when auto-plucking.
 * x/y:             Coordinates of the leader.
 * d:               Variable to return the distance to. NULL for none.
 * ignore_reserved: If true, ignore any buried Pikmin that are "reserved"
 ** (i.e. already chosen to be plucked by another leader).
 */
pikmin* get_closest_buried_pikmin(const float x, const float y, float* d, const bool ignore_reserved) {
    float closest_distance = 0;
    pikmin* closest_pikmin = NULL;
    
    size_t n_pikmin = pikmin_list.size();
    for(size_t p = 0; p < n_pikmin; p++) {
        if(pikmin_list[p]->state != PIKMIN_STATE_BURIED) continue;
        
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
 * Returns the closest hitbox to a point, belonging to a mob's current frame of animation and position.
 * x, y: Point.
 * m:    The mob.
 */
hitbox_instance* get_closest_hitbox(const float x, const float y, mob* m) {
    frame* f = m->anim.get_frame();
    if(!f) return NULL;
    hitbox_instance* closest_hitbox = NULL;
    float closest_hitbox_dist = 0;
    
    for(size_t h = 0; h < f->hitbox_instances.size(); h++) {
        hitbox_instance* h_ptr = &f->hitbox_instances[h];
        float hx, hy;
        rotate_point(h_ptr->x, h_ptr->y, m->angle, &hx, &hy);
        float d = dist(x - m->x, y - m->y, hx, hy) - h_ptr->radius;
        if(h == 0 || d < closest_hitbox_dist) {
            closest_hitbox_dist = d;
            closest_hitbox = h_ptr;
        }
    }
    
    return closest_hitbox;
}

/* ----------------------------------------------------------------------------
 * Returns the daylight effect color for the current time, for the current weather.
 */
ALLEGRO_COLOR get_daylight_color() {
    //ToDo find out how to get the iterator to give me the value of the next point, instead of putting all points in a vector.
    vector<unsigned> point_nrs;
    for(auto p_nr = cur_weather.lighting.begin(); p_nr != cur_weather.lighting.end(); p_nr++) {
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
 * Returns the hitbox instance in the current animation with the specified name.
 */
hitbox_instance* get_hitbox(mob* m, const string name) {
    frame* f = m->anim.get_frame();
    for(size_t h = 0; h < f->hitbox_instances.size(); h++) {
        hitbox_instance* h_ptr = &f->hitbox_instances[h];
        if(h_ptr->hitbox_name == name) return h_ptr;
    }
    return NULL;
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
 * Returns NULL if this event can't run, because there's already another event going on, or because the mob is dead.
 * If query is true, then the caller only wants to know of the existence of the event, not actually do something with it.
   * This makes it return the pointer if it exists, regardless of it being able to run or not.
 */
mob_event* get_mob_event(mob* m, const unsigned char et, const bool query) {
    if(m->dead && et != MOB_EVENT_DEATH) return NULL;
    size_t n_events = m->type->events.size();
    for(size_t ev_nr = 0; ev_nr < n_events; ev_nr++) {
    
        mob_event* ev = m->type->events[ev_nr];
        if(ev->type == et) {
            if(query) return ev;
            if(m->script_wait != 0 && m->script_wait_event != ev && et != MOB_EVENT_DEATH) return NULL;
            return ev;
        }
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
void give_pikmin_to_onion(onion* o, const unsigned amount) {
    unsigned total_after = pikmin_list.size() + amount;
    unsigned pikmin_to_spit = amount;
    unsigned pikmin_to_keep = 0; //Pikmin to keep inside the Onion, without spitting.
    
    if(total_after > max_pikmin_in_field) {
        pikmin_to_keep = total_after - max_pikmin_in_field;
        pikmin_to_spit = amount - pikmin_to_keep;
    }
    
    for(unsigned p = 0; p < pikmin_to_spit; p++) {
        float angle = randomf(0, M_PI * 2);
        float sx = cos(angle) * 60;
        float sy = sin(angle) * 60;
        
        pikmin* new_pikmin = new pikmin(o->x, o->y, o->sec, o->oni_type->pik_type);
        new_pikmin->set_state(PIKMIN_STATE_BURIED);
        new_pikmin->z = 320;
        new_pikmin->speed_z = 200;
        new_pikmin->speed_x = sx;
        new_pikmin->speed_y = sy;
        create_mob(new_pikmin);
    }
    
    for(unsigned p = 0; p < pikmin_to_keep; p++) {
        pikmin_in_onions[o->oni_type->pik_type]++;
    }
}

/* ----------------------------------------------------------------------------
 * Makes a leader go pluck a Pikmin.
 */
void go_pluck(leader* l, pikmin* p) {
    l->auto_pluck_pikmin = p;
    l->pluck_time = -1;
    l->set_target(p->x, p->y, NULL, NULL, false);
    p->pluck_reserved = true;
}

/* ----------------------------------------------------------------------------
 * Returns the interpolation between two colors, given a number in an interval.
 * n: The number.
 * n1, n2: The interval the number falls on.
 ** The closer to n1, the closer the final color is to c1.
 * c1, c2: Colors.
 */
ALLEGRO_COLOR interpolate_color(const float n, const float n1, const float n2, const ALLEGRO_COLOR c1, const ALLEGRO_COLOR c2) {
    float progress = (float) (n - n1) / (float) (n2 - n1);
    return al_map_rgba_f(
               c1.r + progress * (c2.r - c1.r),
               c1.g + progress * (c2.g - c1.g),
               c1.b + progress * (c2.b - c1.b),
               c1.a + progress * (c2.a - c1.a)
           );
}

/* ----------------------------------------------------------------------------
 * Loads the animations from a file.
 */
animation_set load_animation_set(data_node* file_node) {
    map<string, animation*> animations;
    map<string, frame*> frames;
    map<string, hitbox*> hitboxes;
    
    //Hitboxes.
    data_node* hitboxes_node = file_node->get_child_by_name("hitboxes");
    size_t n_hitboxes = hitboxes_node->get_nr_of_children();
    for(size_t h = 0; h < n_hitboxes; h++) {
    
        data_node* hitbox_node = hitboxes_node->get_child(h);
        
        hitboxes[hitbox_node->name] = new hitbox();
        hitbox* cur_hitbox = hitboxes[hitbox_node->name];
        
        cur_hitbox->name = hitbox_node->name;
        cur_hitbox->type = toi(hitbox_node->get_child_by_name("type")->value);
        cur_hitbox->multiplier = tof(hitbox_node->get_child_by_name("multiplier")->value);
        cur_hitbox->elements = hitbox_node->get_child_by_name("elements")->value;
        cur_hitbox->can_pikmin_latch = tob(hitbox_node->get_child_by_name("can_pikmin_latch")->value);
        cur_hitbox->angle = tof(hitbox_node->get_child_by_name("angle")->value);
        cur_hitbox->knockback = tof(hitbox_node->get_child_by_name("knockback")->value);
    }
    
    //Frames.
    data_node* frames_node = file_node->get_child_by_name("frames");
    size_t n_frames = frames_node->get_nr_of_children();
    for(size_t f = 0; f < n_frames; f++) {
    
        data_node* frame_node = frames_node->get_child(f);
        vector<hitbox_instance> hitbox_instances;
        
        data_node* hitbox_instances_node = frame_node->get_child_by_name("hitbox_instances");
        size_t n_hitbox_instances = hitbox_instances_node->get_nr_of_children();
        
        for(size_t h = 0; h < n_hitbox_instances; h++) {
        
            data_node* hitbox_instance_node = hitbox_instances_node->get_child(h);
            
            float hx = 0, hy = 0, hz = 0;
            vector<string> coords = split(hitbox_instance_node->get_child_by_name("coords")->value);
            if(coords.size() >= 3) {
                hx = tof(coords[0]);
                hy = tof(coords[1]);
                hz = tof(coords[2]);
            }
            
            hitbox_instances.push_back(
                hitbox_instance(
                    hitbox_instance_node->name,
                    hitboxes[hitbox_instance_node->name],
                    hx, hy, hz,
                    tof(hitbox_instance_node->get_child_by_name("radius")->value)
                )
            );
        }
        
        ALLEGRO_BITMAP* parent = bitmaps.get(frame_node->get_child_by_name("file")->value, frame_node->get_child_by_name("file"));
        frame* f =
            new frame(
            frame_node->name,
            parent,
            toi(frame_node->get_child_by_name("file_x")->value),
            toi(frame_node->get_child_by_name("file_y")->value),
            toi(frame_node->get_child_by_name("file_w")->value),
            toi(frame_node->get_child_by_name("file_h")->value),
            tof(frame_node->get_child_by_name("game_w")->value),
            tof(frame_node->get_child_by_name("game_h")->value),
            hitbox_instances
        );
        frames[frame_node->name] = f;
        
        f->file = frame_node->get_child_by_name("file")->value;
        f->parent_bmp = parent;
        f->offs_x = tof(frame_node->get_child_by_name("offs_x")->value);
        f->offs_y = tof(frame_node->get_child_by_name("offs_y")->value);
        f->top_visible = tob(frame_node->get_child_by_name("top_visible")->value);
        f->top_x = tof(frame_node->get_child_by_name("top_x")->value);
        f->top_y = tof(frame_node->get_child_by_name("top_y")->value);
        f->top_w = tof(frame_node->get_child_by_name("top_w")->value);
        f->top_h = tof(frame_node->get_child_by_name("top_h")->value);
        f->top_angle = tof(frame_node->get_child_by_name("top_angle")->value);
    }
    
    //Animations.
    data_node* anims_node = file_node->get_child_by_name("animations");
    size_t n_anims = anims_node->get_nr_of_children();
    for(size_t a = 0; a < n_anims; a++) {
    
        data_node* anim_node = anims_node->get_child(a);
        vector<frame_instance> frame_instances;
        
        data_node* frame_instances_node = anim_node->get_child_by_name("frame_instances");
        size_t n_frame_instances = frame_instances_node->get_nr_of_children();
        
        for(size_t f = 0; f < n_frame_instances; f++) {
            data_node* frame_instance_node = frame_instances_node->get_child(f);
            frame_instances.push_back(
                frame_instance(
                    frame_instance_node->name,
                    frames[frame_instance_node->name],
                    tof(frame_instance_node->get_child_by_name("duration")->value)
                )
            );
        }
        
        animations[anim_node->name] =
            new animation(
            anim_node->name,
            frame_instances,
            toi(anim_node->get_child_by_name("loop_frame")->value)
        );
    }
    
    return animation_set(animations, frames, hitboxes);
}


/* ----------------------------------------------------------------------------
 * Loads an area into memory.
 */
void load_area(const string name) {

    data_node file = load_data_file(AREA_FOLDER "/" + name + ".txt");
    
    string weather_condition_name = file.get_child_by_name("weather")->value;
    if(weather_conditions.find(weather_condition_name) == weather_conditions.end()) {
        error_log("Area " + name + " refers to a non-existing weather condition!", &file);
        cur_weather = weather();
    } else {
        cur_weather = weather_conditions[weather_condition_name];
    }
    
    
    //Load sectors.
    
    sectors.clear();
    size_t n_sectors = file.get_child_by_name("sectors")->get_nr_of_children_by_name("sector");
    for(size_t s = 0; s < n_sectors; s++) {
        data_node* sector_data = file.get_child_by_name("sectors")->get_child_by_name("sector", s);
        sector new_sector = sector();
        
        size_t n_floors = sector_data->get_nr_of_children_by_name("floor");
        if(n_floors > 2) n_floors = 2;
        for(size_t f = 0; f < n_floors; f++) {  //ToDo this is not the way to do it.
            data_node* floor_data = sector_data->get_child_by_name("floor", f);
            floor_info new_floor = floor_info();
            
            new_floor.brightness = tof(floor_data->get_child_by_name("brightness")->get_value_or_default("1"));
            new_floor.rot = tof(floor_data->get_child_by_name("texture_rotate")->value);
            new_floor.scale = tof(floor_data->get_child_by_name("texture_scale")->value);
            new_floor.trans_x = tof(floor_data->get_child_by_name("texture_trans_x")->value);
            new_floor.trans_y = tof(floor_data->get_child_by_name("texture_trans_y")->value);
            new_floor.texture = load_bmp("Textures/" + floor_data->get_child_by_name("texture")->value, floor_data);  //ToDo don't load it every time.
            new_floor.z = tof(floor_data->get_child_by_name("z")->value);
            //ToDo terrain sound.
            
            new_sector.floors[f] = new_floor;
        }
        
        size_t n_linedefs = sector_data->get_nr_of_children_by_name("linedef");
        for(size_t l = 0; l < n_linedefs; l++) {
            data_node* linedef_data = sector_data->get_child_by_name("linedef", l);
            linedef* new_linedef = new linedef();
            
            new_linedef->x1 = tof(linedef_data->get_child_by_name("x")->value);
            new_linedef->y1 = tof(linedef_data->get_child_by_name("y")->value);
            
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
    
    
    //Load mobs.
    
    mobs.clear();
    size_t n_mobs = file.get_child_by_name("mobs")->get_nr_of_children();
    for(size_t m = 0; m < n_mobs; m++) {
    
        data_node* mob_node = file.get_child_by_name("mobs")->get_child(m);
        
        vector<string> coords = split(mob_node->get_child_by_name("coords")->value);
        float x = (coords.size() >= 1 ? tof(coords[0]) : 0);
        float y = (coords.size() >= 2 ? tof(coords[1]) : 0);
        
        if(mob_node->name == "enemy") {
        
            string et = mob_node->get_child_by_name("type")->value;
            if(enemy_types.find(et) != enemy_types.end()) {
                create_mob(new enemy(
                               x, y,
                               &sectors[0], //ToDo
                               enemy_types[et]
                           ));
                           
            } else error_log("Unknown enemy type \"" + et + "\"!", mob_node);
            
        } else if(mob_node->name == "leader") {
        
            string lt = mob_node->get_child_by_name("type")->value;
            if(leader_types.find(lt) != leader_types.end()) {
                create_mob(new leader(
                               x, y,
                               &sectors[0], //ToDo
                               leader_types[lt]
                           ));
                           
            } else error_log("Unknown leader type \"" + lt + "\"!", mob_node);
            
        } else if(mob_node->name == "ship") {
        
            create_mob(new ship(
                           x, y,
                           &sectors[0] //ToDo
                       ));
                       
        } else if(mob_node->name == "onion") {
        
            string ot = mob_node->get_child_by_name("type")->value;
            if(onion_types.find(ot) != onion_types.end()) {
                create_mob(new onion(
                               x, y,
                               &sectors[0], //ToDo
                               onion_types[ot]
                           ));
                           
            } else error_log("Unknown onion type \"" + ot + "\"!", mob_node);
            
        } else if(mob_node->name == "treasure") {
        
            string tt = mob_node->get_child_by_name("type")->value;
            if(treasure_types.find(tt) != treasure_types.end()) {
                create_mob(new treasure(
                               x, y,
                               &sectors[0], //ToDo
                               treasure_types[tt]
                           ));
                           
            } else error_log("Unknown treasure type \"" + tt + "\"!", mob_node);
            
        } else {
        
            error_log("Unknown mob type \"" + mob_node->name + "\"!", mob_node);
            continue;
        }
        
    }
}

/* ----------------------------------------------------------------------------
 * Loads a bitmap from the game's content.
 * If the node is present, it'll be used to report errors.
 */
ALLEGRO_BITMAP* load_bmp(const string filename, data_node* node) {
    ALLEGRO_BITMAP* b = NULL;
    b = al_load_bitmap((GRAPHICS_FOLDER "/" + filename).c_str());
    if(!b) {
        error_log("Could not open image " + filename + "!", node);
        b = bmp_error;
    }
    
    return b;
}

/* ----------------------------------------------------------------------------
 * Loads a game control.
 */
void load_control(const unsigned char action, const unsigned char player, const string name, data_node &file, const string def) {
    string s = file.get_child_by_name("p" + itos((player + 1)) + "_" + name)->get_value_or_default((player == 0) ? def : "");
    vector<string> possible_controls = split(s, ",");
    size_t n_possible_controls = possible_controls.size();
    
    for(size_t c = 0; c < n_possible_controls; c++) {
        controls.push_back(control_info(action, player, possible_controls[c]));
    }
}

/* ----------------------------------------------------------------------------
 * Loads a data file from the game's content.
 */
data_node load_data_file(const string filename) {
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
    statuses.push_back(status(0, 0, 1, true, al_map_rgb(128, 0, 255), STATUS_AFFECTS_ENEMIES));
    statuses.push_back(status(1.5, 1.5, 1, false, al_map_rgb(255, 64, 64), STATUS_AFFECTS_PIKMIN));
    
    spray_types.push_back(spray_type(&statuses[0], false, 10, al_map_rgb(128, 0, 255), NULL, NULL));
    spray_types.push_back(spray_type(&statuses[1], true, 40, al_map_rgb(255, 0, 0), NULL, NULL));
    //spray_types.push_back(spray_type(&statuses[1], true, 40, al_map_rgb(255, 255, 0), NULL, NULL));
    
    //Mob types.
    load_mob_types(PIKMIN_FOLDER, MOB_TYPE_PIKMIN);
    load_mob_types(ONIONS_FOLDER, MOB_TYPE_ONION);
    load_mob_types(LEADERS_FOLDER, MOB_TYPE_LEADER);
    load_mob_types(ENEMIES_FOLDER, MOB_TYPE_ENEMY);
    load_mob_types(TREASURES_FOLDER, MOB_TYPE_TREASURE);
    load_mob_types(PELLETS_FOLDER, MOB_TYPE_PELLET);
    
    //Weather.
    weather_conditions.clear();
    data_node weather_file = load_data_file(WEATHER_FILE);
    size_t n_weather_conditions = weather_file.get_nr_of_children_by_name("weather");
    
    for(size_t wc = 0; wc < n_weather_conditions; wc++) {
        data_node* cur_weather = weather_file.get_child_by_name("weather", wc);
        
        string name = cur_weather->get_child_by_name("name")->value;
        if(name.size() == 0) name = "default";
        
        map<unsigned, ALLEGRO_COLOR> lighting;
        size_t n_lighting_points = cur_weather->get_child_by_name("lighting")->get_nr_of_children();
        
        for(size_t lp = 0; lp < n_lighting_points; lp++) {
            data_node* lighting_node = cur_weather->get_child_by_name("lighting")->get_child(lp);
            
            unsigned point_time = toi(lighting_node->name);
            ALLEGRO_COLOR point_color = toc(lighting_node->value);
            
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
        
        unsigned char percipitation_type = toi(cur_weather->get_child_by_name("percipitation_type")->get_value_or_default(itos(PERCIPITATION_TYPE_NONE)));
        interval percipitation_frequency = interval(cur_weather->get_child_by_name("percipitation_frequency")->value);
        interval percipitation_speed = interval(cur_weather->get_child_by_name("percipitation_speed")->value);
        interval percipitation_angle = interval(cur_weather->get_child_by_name("percipitation_angle")->get_value_or_default(ftos((M_PI + M_PI_2))));
        
        weather_conditions[name] = weather(name, lighting, percipitation_type, percipitation_frequency, percipitation_speed, percipitation_angle);
    }
}

/* ----------------------------------------------------------------------------
 * Loads the hitboxes from a file.
 */
vector<hitbox> load_hitboxes(data_node* frame_node) {
    vector<hitbox> hitboxes;
    data_node* hitboxes_node = frame_node->get_child_by_name("hitboxes");
    size_t n_hitboxes = hitboxes_node->get_nr_of_children_by_name("hitbox");
    for(size_t h = 0; h < n_hitboxes; h++) {
        data_node* hitbox_node = hitboxes_node->get_child_by_name("hitbox", h);
        hitboxes.push_back(hitbox());
        hitbox* cur_hitbox = &hitboxes.back();
        
        cur_hitbox->name = hitbox_node->get_child_by_name("name")->value;
        cur_hitbox->type = toi(hitbox_node->get_child_by_name("type")->value);
        cur_hitbox->multiplier = tof(hitbox_node->get_child_by_name("multiplier")->value);
        cur_hitbox->can_pikmin_latch = tob(hitbox_node->get_child_by_name("can_pikmin_latch")->value);
        cur_hitbox->angle = tof(hitbox_node->get_child_by_name("angle")->value);
        cur_hitbox->knockback = tof(hitbox_node->get_child_by_name("knockback")->value);
    }
    
    return hitboxes;
}

/* ----------------------------------------------------------------------------
 * Loads the mob types from a folder.
 * type: Use MOB_TYPE_* for this.
 */
void load_mob_types(const string folder, const unsigned char type) {
    vector<string> types = folder_to_vector(folder, true);
    if(types.size() == 0) {
        error_log("Folder not found \"" + folder + "\"!");
    }
    
    for(size_t t = 0; t < types.size(); t++) {
    
        data_node file = data_node(folder + "/" + types[t] + "/Data.txt");
        if(!file.file_was_opened) return;
        
        mob_type* mt;
        if(type == MOB_TYPE_PIKMIN) {
            mt = new pikmin_type();
        } else if(type == MOB_TYPE_ONION) {
            mt = new onion_type();
        } else if(type == MOB_TYPE_LEADER) {
            mt = new leader_type();
        } else if(type == MOB_TYPE_ENEMY) {
            mt = new enemy_type();
        } else if(type == MOB_TYPE_PELLET) {
            mt = new pellet_type();
        } else {
            mt = new mob_type();
        }
        
        mt->name = file.get_child_by_name("name")->value;
        mt->always_active = tob(file.get_child_by_name("always_active")->value);
        mt->big_damage_interval = tof(file.get_child_by_name("big_damage_interval")->value);
        mt->chomp_max_victims = toi(file.get_child_by_name("chomp_max_victims")->get_value_or_default("100"));
        mt->main_color = toc(file.get_child_by_name("main_color")->value);
        mt->max_carriers = toi(file.get_child_by_name("max_carriers")->value);
        mt->max_health = toi(file.get_child_by_name("max_health")->value);
        mt->move_speed = tof(file.get_child_by_name("move_speed")->value);
        mt->near_radius = tof(file.get_child_by_name("near_radius")->value);
        mt->rotation_speed = tof(file.get_child_by_name("rotation_speed")->get_value_or_default(ftos(DEF_ROTATION_SPEED)));
        mt->sight_radius = tof(file.get_child_by_name("sight_radius")->value);
        mt->size = tof(file.get_child_by_name("size")->value);
        mt->weight = tof(file.get_child_by_name("weight")->value);
        
        mt->events = load_script(file.get_child_by_name("script"));
        
        data_node anim_file = data_node(folder + "/" + types[t] + "/Animations.txt");
        mt->anims = load_animation_set(&anim_file);
        
        if(type == MOB_TYPE_PIKMIN) {
            pikmin_type* pt = (pikmin_type*) mt;
            pt->attack_power = tof(file.get_child_by_name("attack_power")->value);
            pt->attack_interval = tof(file.get_child_by_name("attack_interval")->get_value_or_default("0.8"));
            pt->can_carry_bomb_rocks = tob(file.get_child_by_name("can_carry_bomb_rocks")->value);
            pt->can_dig = tob(file.get_child_by_name("can_dig")->value);
            pt->can_latch = tob(file.get_child_by_name("can_latch")->value);
            pt->can_swim = tob(file.get_child_by_name("can_swim")->value);
            pt->carry_speed = tof(file.get_child_by_name("carry_speed")->value);
            pt->carry_strength = tof(file.get_child_by_name("carry_strength")->value);
            pt->has_onion = tob(file.get_child_by_name("has_onion")->value);
            pt->bmp_top[0] = load_bmp(file.get_child_by_name("top_leaf")->value, &file); //ToDo don't load these for every Pikmin type.
            pt->bmp_top[1] = load_bmp(file.get_child_by_name("top_bud")->value, &file);
            pt->bmp_top[2] = load_bmp(file.get_child_by_name("top_flower")->value, &file);
            
            pikmin_types[pt->name] = pt;
            
        } else if(type == MOB_TYPE_ONION) {
            onion_type* ot = (onion_type*) mt;
            data_node* pik_type_node = file.get_child_by_name("pikmin_type");
            if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
                error_log("Unknown Pikmin type \"" + pik_type_node->value + "\"!", pik_type_node);
                continue;
            }
            ot->pik_type = pikmin_types[pik_type_node->value];
            
            onion_types[ot->name] = ot;
            
        } else if(type == MOB_TYPE_LEADER) {
            leader_type* lt = (leader_type*) mt;
            lt->sfx_dismiss = load_sample(file.get_child_by_name("dismiss_sfx")->value, mixer); //ToDo don't use load_sample.
            lt->sfx_name_call = load_sample(file.get_child_by_name("name_call_sfx")->value, mixer); //ToDo don't use load_sample.
            lt->pluck_delay = tof(file.get_child_by_name("pluck_delay")->value);
            lt->punch_strength = toi(file.get_child_by_name("punch_strength")->value); //ToDo default.
            lt->whistle_range = tof(file.get_child_by_name("whistle_range")->get_value_or_default(ftos(DEF_WHISTLE_RANGE)));
            lt->sfx_whistle = load_sample(file.get_child_by_name("whistle_sfx")->value, mixer); //ToDo don't use load_sample.
            
            leader_types[lt->name] = lt;
            
        } else if(type == MOB_TYPE_ENEMY) {
            enemy_type* et = (enemy_type*) mt;
            et->drops_corpse = tob(file.get_child_by_name("drops_corpse")->get_value_or_default("yes"));
            et->is_boss = tob(file.get_child_by_name("is_boss")->value);
            et->pikmin_seeds = toi(file.get_child_by_name("pikmin_seeds")->value);
            et->regenerate_speed = tob(file.get_child_by_name("regenerate_speed")->value);
            et->revive_speed = tof(file.get_child_by_name("revive_speed")->value);
            et->value = tof(file.get_child_by_name("value")->value);
            
            enemy_types[et->name] = et;
            
        } else if(type == MOB_TYPE_TREASURE) {
            treasure_type* tt = (treasure_type*) mt;
            tt->move_speed = 60; //ToDo should this be here?
            
            treasure_types[tt->name] = tt;
            
        } else if(type == MOB_TYPE_PELLET) {
            pellet_type* pt = (pellet_type*) mt;
            data_node* pik_type_node = file.get_child_by_name("pikmin_type");
            if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
                error_log("Unknown Pikmin type \"" + pik_type_node->value + "\"!", pik_type_node);
                continue;
            }
            
            pt->pik_type = pikmin_types[pik_type_node->value];
            pt->number = toi(file.get_child_by_name("number")->value);
            pt->weight = pt->number;
            pt->match_seeds = toi(file.get_child_by_name("match_seeds")->value);
            pt->non_match_seeds = toi(file.get_child_by_name("non_match_seeds")->value);
            
            pt->move_speed = 60; //ToDo should this be here?
            
            pellet_types[pt->name] = pt;
            
        }
        
    }
    
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
        mouse_moves_cursor[p] = tob(file.get_child_by_name("p" + itos((p + 1)) + "_mouse_moves_cursor")->get_value_or_default((p == 0) ? "true" : "false"));
    }
    
    //Other options.
    daylight_effect = tob(file.get_child_by_name("daylight_effect")->get_value_or_default("true"));
    draw_cursor_trail = tob(file.get_child_by_name("draw_cursor_trail")->get_value_or_default("true"));
    game_fps = toi(file.get_child_by_name("fps")->get_value_or_default("30"));
    scr_h = toi(file.get_child_by_name("height")->get_value_or_default(itos(DEF_SCR_H)));
    particle_quality = toi(file.get_child_by_name("particle_quality")->get_value_or_default("2"));
    pretty_whistle = tob(file.get_child_by_name("pretty_whistle")->get_value_or_default("true"));
    scr_w = toi(file.get_child_by_name("width")->get_value_or_default(itos(DEF_SCR_W)));
    smooth_scaling = tob(file.get_child_by_name("smooth_scaling")->get_value_or_default("true"));
    window_x = toi(file.get_child_by_name("window_x")->get_value_or_default(itos(INT_MAX)));
    window_y = toi(file.get_child_by_name("window_y")->get_value_or_default(itos(INT_MAX)));
}

/* ----------------------------------------------------------------------------
 * Loads an audio sample from the game's content.
 */
sample_struct load_sample(const string filename, ALLEGRO_MIXER* const mixer) {
    ALLEGRO_SAMPLE* sample = al_load_sample((AUDIO_FOLDER "/" + filename).c_str());
    if(!sample) {
        error_log("Could not open audio sample " + filename + "!");
    }
    
    return sample_struct(sample, mixer);
}

/* ----------------------------------------------------------------------------
 * Loads a script from a data node.
 */
vector<mob_event*> load_script(data_node* node) {

    vector<mob_event*> events;
    
    for(size_t e = 0; e < node->get_nr_of_children(); e++) {
        data_node* event_node = node->get_child(e);
        
        vector<mob_action*> actions;
        
        for(size_t a = 0; a < event_node->get_nr_of_children(); a++) {
            data_node* action_node = event_node->get_child(a);
            
            actions.push_back(new mob_action(action_node));
        }
        
        events.push_back(new mob_event(event_node, actions));
        
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
void move_point(const float x, const float y, const float tx, const float ty, const float speed, const float reach_radius, float* mx, float* my, float* angle, bool* reached) {
    float dx = tx - x, dy = ty - y;
    float dist = sqrt(dx * dx + dy * dy);
    
    if(dist > reach_radius) {
        float move_amount = min(dist / delta_t / 2, speed);
        
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
void pluck_pikmin(leader* new_leader, pikmin* p, leader* leader_who_plucked) {
    if(p->state != PIKMIN_STATE_BURIED) return;
    
    leader_who_plucked->pluck_time = -1;
    p->set_state(PIKMIN_STATE_IN_GROUP);
    add_to_party(new_leader, p);
    sfx_pikmin_plucked.play(0, false);
    sfx_pikmin_pluck.play(0, false);
}

/* ----------------------------------------------------------------------------
 * Returns a random float between the provided range, inclusive.
 */
float randomf(float min, float max) {
    if(min > max) swap(min, max);
    if(min == max) return min;
    return (float) rand() / ((float) RAND_MAX / (max - min)) + min;
}

/* ----------------------------------------------------------------------------
 * Returns a random integer between the provided range, inclusive.
 */
int randomi(int min, int max) {
    if(min > max) swap(min, max);
    if(min == max) return min;
    return ((rand()) % (max - min + 1)) + min;
}

/* ----------------------------------------------------------------------------
 * Generates random particles in an explosion fashion:
 ** they scatter from the center point at random angles,
 ** and drift off until they vanish.
 * type:     Type of particle. Use PARTICLE_TYPE_*.
 * bmp:      Bitmap to use.
 * center_*: Center point of the explosion.
 * speed_*:  Their speed is random within this range, inclusive.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_explosion(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float center_x, const float center_y, const float speed_min, const float speed_max, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color) {
    unsigned char n_particles = randomi(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        float angle = randomf(0, M_PI * 2);
        float speed = randomf(speed_min, speed_max);
        
        float speed_x = cos(angle) * speed;
        float speed_y = sin(angle) * speed;
        
        particles.push_back(
            particle(
                type,
                bmp,
                center_x, center_y,
                speed_x, speed_y,
                1,
                0,
                randomf(time_min, time_max),
                randomf(size_min, size_max),
                color
            )
        );
    }
}

/* ----------------------------------------------------------------------------
 * Generates random particles in a fire fashion:
 ** the particles go up and speed up as time goes by.
 * type:     Type of particle. Use PARTICLE_TYPE_*.
 * bmp:      Bitmap to use.
 * center_*: Center point of the fire.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_fire(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color) {
    unsigned char n_particles = randomi(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        particles.push_back(
            particle(
                type,
                bmp,
                origin_x, origin_y,
                randomf(-6, 6),
                randomf(-10, -20),
                0,
                -1,
                randomf(time_min, time_max),
                randomf(size_min, size_max),
                color
            )
        );
    }
}

/* ----------------------------------------------------------------------------
 * Generates random particles in a splash fashion:
 ** the particles go up and are scattered horizontally,
 ** and then go down with the effect of gravity.
 * type:     Type of particle. Use PARTICLE_TYPE_*.
 * bmp:      Bitmap to use.
 * center_*: Center point of the splash.
 * min/max:  The number of particles is random within this range, inclusive.
 * time_*:   Their lifetime is random within this range, inclusive.
 * size_*:   Their size is random within this range, inclusive.
 * color:    Particle color.
 */
void random_particle_splash(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const unsigned char min, const unsigned char max, const float time_min, const float time_max, const float size_min, const float size_max, const ALLEGRO_COLOR color) {
    unsigned char n_particles = randomi(min, max);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        particles.push_back(
            particle(
                type,
                bmp,
                origin_x, origin_y,
                randomf(-2, 2),
                randomf(-2, -4),
                0, 0.5,
                randomf(time_min, time_max),
                randomf(size_min, size_max),
                color
            )
        );
    }
}

/* ----------------------------------------------------------------------------
 * Generates random particles in a spray fashion:
 ** the particles go in the pointed direction,
 ** and move gradually slower as they fade into the air.
 ** Used on actual sprays in-game.
 * type:     Type of particle. Use PARTICLE_TYPE_*.
 * bmp:      Bitmap to use.
 * origin_*: Origin point of the spray.
 * angle:    Angle to shoot at.
 * color:    Color of the particles.
 */
void random_particle_spray(const unsigned char type, ALLEGRO_BITMAP* const bmp, const float origin_x, const float origin_y, const float angle, const ALLEGRO_COLOR color) {
    unsigned char n_particles = randomi(35, 40);
    
    for(unsigned char p = 0; p < n_particles; p++) {
        float angle_offset = randomf(-M_PI_4, M_PI_4);
        
        float power = randomf(30, 90);
        float speed_x = cos(angle + angle_offset) * power;
        float speed_y = sin(angle + angle_offset) * power;
        
        particles.push_back(
            particle(
                type,
                bmp,
                origin_x, origin_y,
                speed_x, speed_y,
                1,
                0,
                randomf(3, 4),
                randomf(6, 8),
                color
            )
        );
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
    member->unwhistlable_period = UNWHISTLABLE_PERIOD;
    member->untouchable_period = UNTOUCHABLE_PERIOD;
}

/* ----------------------------------------------------------------------------
 * Rotates a point by an angle. The x and y are meant to represent the difference between the point and the center of the rotation.
 */
void rotate_point(const float x, const float y, const float angle, float* final_x, float* final_y) {
    float c = cos(angle);
    float s = sin(angle);
    if(final_x) *final_x = c * x - s * y;
    if(final_y) *final_y = s * x + c * y;
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
        string prefix = "p" + itos((p + 1)) + "_";
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
        string name = "p" + itos((controls[c].player + 1)) + "_";
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
    for(auto c = grouped_controls.begin(); c != grouped_controls.end(); c++) {
        if(c->second.size()) c->second.erase(c->second.size() - 1); //Remove the final character, which is always an extra comma.
        
        al_fwrite(file, c->first + "=" + c->second + "\n");
    }
    
    for(unsigned char p = 0; p < 4; p++) {
        al_fwrite(file, "p" + itos((p + 1)) + "_mouse_moves_cursor=" + btos(mouse_moves_cursor[p]) + "\n");
    }
    
    //Other options.
    al_fwrite(file, "daylight_effect=" + btos(daylight_effect) + "\n");
    al_fwrite(file, "draw_cursor_trail=" + btos(draw_cursor_trail) + "\n");
    al_fwrite(file, "fps=" + itos(game_fps) + "\n");
    al_fwrite(file, "height=" + itos(scr_h) + "\n");
    al_fwrite(file, "particle_quality=" + itos(particle_quality) + "\n");
    al_fwrite(file, "pretty_whistle=" + btos(pretty_whistle) + "\n");
    al_fwrite(file, "width=" + itos(scr_w) + "\n");
    al_fwrite(file, "smooth_scaling=" + btos(smooth_scaling) + "\n");
    al_fwrite(file, "window_x=" + itos(window_x) + "\n");
    al_fwrite(file, "window_y=" + itos(window_y) + "\n");
    
    al_fclose(file);
}

/* ----------------------------------------------------------------------------
 * Should m1 attack m2? Teams are used to decide this.
 */
bool should_attack(mob* m1, mob* m2) {
    if(m2->team == MOB_TEAM_DECORATION) return false;
    if(m1->team == MOB_TEAM_NONE) return true;
    if(m1->team == m2->team) return false;
    return true;
}

/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:        The string to split.
 * del:         The delimiter. Default is space.
 * inc_empty:   If true, include empty substrings on the vector.
 ** i.e. if two delimiters come together in a row, keep an empty substring between.
 * inc_del:     If true, include the delimiters on the vector as a substring.
 */
vector<string> split(string text, const string del, const bool inc_empty, const bool inc_del) {
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
void start_camera_pan(const int final_x, const int final_y) {
    cam_trans_pan_initial_x = cam_x;
    cam_trans_pan_initial_y = cam_y;
    cam_trans_pan_final_x = final_x;
    cam_trans_pan_final_y = final_y;
    cam_trans_pan_time_left = CAM_TRANSITION_DURATION;
}

/* ----------------------------------------------------------------------------
 * Starts moving the camera towards another zoom level.
 */
void start_camera_zoom(const float final_zoom_level) {
    cam_trans_zoom_initial_level = cam_zoom;
    cam_trans_zoom_final_level = final_zoom_level;
    cam_trans_zoom_time_left = CAM_TRANSITION_DURATION;
    
    sfx_camera.play(0, false);
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
        
        //First, count how many of each type there are carrying.
        for(size_t p = 0; p < m->carrier_info->max_carriers; p++) {
            pikmin* pik_ptr = NULL;
            
            if(m->carrier_info->carrier_spots[p] == NULL) continue;
            if(typeid(*m->carrier_info->carrier_spots[p]) != typeid(pikmin)) continue;
            
            pik_ptr = (pikmin*) m->carrier_info->carrier_spots[p];
            
            if(!pik_ptr->pik_type->has_onion) continue; //If it doesn't have an Onion, it won't even count. //ToDo what if it hasn't been discovered / Onion not on this area?
            
            type_quantity[pik_ptr->pik_type]++;
        }
        
        //Then figure out what are the majority types.
        unsigned most = 0;
        for(auto t = type_quantity.begin(); t != type_quantity.end(); t++) {
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
            
        } else if(majority_types.size() == 1) {
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
            
            //If a Pikmin left, check if it is related to the majority types.
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
                m->carrier_info->decided_type = majority_types[randomi(0, majority_types.size() - 1)];
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
        m->set_state(MOB_STATE_BEING_CARRIED);
        sfx_pikmin_carrying.play(-1, true);
    }
}

/* ----------------------------------------------------------------------------
 * Starts the display of a text message. If the text is empty, it closes the message box.
 * text:        Text to display.
 * speaker_bmp: Bitmap representing the speaker.
 */
void start_message(string text, ALLEGRO_BITMAP* speaker_bmp) {
    if(text.size()) if(text.back() == '\n') text.pop_back();
    cur_message = text;
    cur_message_char = 0;
    cur_message_char_time = MESSAGE_CHAR_INTERVAL;
    cur_message_speaker = speaker_bmp;
    cur_message_stopping_chars.clear();
    cur_message_stopping_chars.push_back(0); //First character. Makes it easier.
    cur_message_section = 0;
    
    vector<string> lines = split(text, "\n");
    for(size_t line_trio = 0; line_trio < lines.size(); line_trio += 3) {
        cur_message_stopping_chars.push_back(0);
        for(size_t l = 0; l < (line_trio + 1) * 3 && l < lines.size(); l++) {
            cur_message_stopping_chars.back() += lines[l].size() + 1; //+1 because of the new line character.
        }
    }
    cur_message_stopping_chars.back()--; //Remove one because the last line doesn't have a new line character. Even if it does, it's invisible.
}

/* ----------------------------------------------------------------------------
 * Makes a leader get out of auto-pluck mode.
 */
void stop_auto_pluck(leader* l) {
    if(l->auto_pluck_pikmin) l->remove_target(true);
    l->auto_pluck_mode = false;
    if(l->auto_pluck_pikmin) l->auto_pluck_pikmin->pluck_reserved = false;
    l->auto_pluck_pikmin = NULL;
    l->pluck_time = -1;
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
    
    leaders[cur_leader_nr]->lea_type->sfx_whistle.stop();
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
 * Makes m1 lose focus on m2.
 */
void unfocus_mob(mob* m1, mob* m2, const bool call_event) {
    if(m2) {
        if(m1->focused_prey != m2) return;
        
        for(size_t m = 0; m < m2->focused_by.size();) {
            if(m2->focused_by[m] == m1) m2->focused_by.erase(m2->focused_by.begin() + m);
            else m++;
        }
    }
    
    m1->focused_prey = NULL;
    m1->focused_prey_near = false;
    if(call_event) {
        m1->events_queued[MOB_EVENT_SEE_PREY] = 0;
        m1->events_queued[MOB_EVENT_NEAR_PREY] = 0;
        m1->events_queued[MOB_EVENT_LOSE_PREY] = 1;
    }
}

/* ----------------------------------------------------------------------------
 * Uses up a spray.
 */
void use_spray(const size_t spray_nr) {
    if(spray_amounts[spray_nr] == 0) return;
    
    leader* cur_leader_ptr = leaders[cur_leader_nr];
    float shoot_angle = cursor_angle + ((spray_types[spray_nr].burpable) ? M_PI : 0);
    
    random_particle_spray(
        PARTICLE_TYPE_CIRCLE,
        NULL,
        cur_leader_ptr->x + cos(shoot_angle) * cur_leader_ptr->type->size / 2,
        cur_leader_ptr->y + sin(shoot_angle) * cur_leader_ptr->type->size / 2,
        shoot_angle,
        spray_types[spray_nr].main_color
    );
    
    spray_amounts[spray_nr]--;
    
    cur_leader_ptr->anim.change("dismiss", false, false);
}

//Calls al_fwrite, but with an std::string instead of a c-string.
void al_fwrite(ALLEGRO_FILE* f, string s) { al_fwrite(f, s.c_str(), s.size()); }
//Converts a boolean to a string, returning either "true" or "false".
string btos(bool b) { return b ? "true" : "false"; }
//Converts a string to a boolean, judging by the English language words that represent true and false.
bool tob(string s) {
    s = str_to_lower(s);
    s = trim_spaces(s);
    if(s == "yes" || s == "true" || s == "y" || s == "t") return true;
    else return (toi(s) != 0);
}
//Converts a string to an Allegro color. Components are separated by spaces, and the final one (alpha) is optional.
ALLEGRO_COLOR toc(string s) {
    s = trim_spaces(s);
    
    unsigned char alpha = 255;
    vector<string> components = split(s);
    if(components.size() >= 2) alpha = toi(components[1]);
    
    if(s == "nothing" || s == "none") return al_map_rgba(0,   0,   0,   0);
    if(s == "black")                  return al_map_rgba(0,   0,   0,   alpha);
    if(s == "gray" || s == "grey")    return al_map_rgba(128, 128, 128, alpha);
    if(s == "white")                  return al_map_rgba(255, 255, 255, alpha);
    if(s == "yellow")                 return al_map_rgba(255, 255, 0,   alpha);
    if(s == "orange")                 return al_map_rgba(255, 128, 0,   alpha);
    if(s == "brown")                  return al_map_rgba(128, 64,  0,   alpha);
    if(s == "red")                    return al_map_rgba(255, 0,   0,   alpha);
    if(s == "violet")                 return al_map_rgba(255, 0,   255, alpha);
    if(s == "purple")                 return al_map_rgba(128, 0,   255, alpha);
    if(s == "blue")                   return al_map_rgba(0,   0,   255, alpha);
    if(s == "cyan")                   return al_map_rgba(0,   255, 255, alpha);
    if(s == "green")                  return al_map_rgba(0,   255, 0,   alpha);
    
    ALLEGRO_COLOR c =
        al_map_rgba(
            ((components.size() > 0) ? toi(components[0]) : 0),
            ((components.size() > 1) ? toi(components[1]) : 0),
            ((components.size() > 2) ? toi(components[2]) : 0),
            ((components.size() > 3) ? toi(components[3]) : 255)
        );
    return c;
}
//Converts a string to a float, trimming the spaces and accepting commas or points.
double tof(string s) { s = trim_spaces(s); replace(s.begin(), s.end(), ',', '.'); return atof(s.c_str()); }
//Converts a string to an integer.
int toi(string s) { return tof(s); }
