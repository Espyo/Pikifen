#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "functions.h"
#include "vars.h"

void do_drawing() {

    /*  ***************************************
      *** |  |                           |  | ***
    ***** |__|          DRAWING          |__| *****
      ***  \/                             \/  ***
        ***************************************/
    
    if(!paused) {
    
        size_t n_leaders =     leaders.size();
        size_t n_particles =   particles.size();
        size_t n_pikmin =      pikmin_list.size();
        size_t n_spray_types = spray_types.size();
        size_t n_treasures =   treasures.size();
        
        al_clear_to_color(al_map_rgb(0, 0, 0));
        
        ALLEGRO_TRANSFORM normal_transform;
        al_identity_transform(&normal_transform);
        
        ALLEGRO_TRANSFORM world_to_screen_transform = get_world_to_screen_transform();
        al_use_transform(&world_to_screen_transform);
        
        /* Layer 1
        *************************
        *                ^^^^^^ *
        *   Background   ^^^^^^ *
        *                ^^^^^^ *
        ************************/
        
        //ToDo optimize
        size_t area_image_cols = area_images.size();
        for(size_t x = 0; x < area_image_cols; x++) {
            size_t area_image_rows = area_images[x].size();
            for(size_t y = 0; y < area_image_rows; y++) {
                al_draw_bitmap(area_images[x][y], x * AREA_IMAGE_SIZE + area_x1, y * AREA_IMAGE_SIZE + area_y1, 0);
            }
        }
        
        
        /* Layer 2
        ************************
        *                  ##  *
        *   Mob shadows   #### *
        *                  ##  *
        ***********************/
        
        float shadow_stretch = 0;
        
        if(day_minutes < 60 * 5 || day_minutes > 60 * 20) {
            shadow_stretch = 1;
        } else if(day_minutes < 60 * 12) {
            shadow_stretch = 1 - ((day_minutes - 60 * 5) / (60 * 12 - 60 * 5));
        } else {
            shadow_stretch = (day_minutes - 60 * 12) / (60 * 20 - 60 * 12);
        }
        
        for(size_t l = 0; l < n_leaders; l++) {
            draw_shadow(leaders[l]->x, leaders[l]->y, 32, leaders[l]->z - leaders[l]->sec->floors[0].z, shadow_stretch);
        }
        
        for(size_t p = 0; p < n_pikmin; p++) {
            draw_shadow(pikmin_list[p]->x, pikmin_list[p]->y, 18, pikmin_list[p]->z - pikmin_list[p]->sec->floors[0].z, shadow_stretch);
        }
        
        
        /* Layer 3
        *********************
        *             .-.   *
        *   Cursor   ( = )> *
        *             `-´   *
        ********************/
        
        size_t n_arrows = move_group_arrows.size();
        for(size_t a = 0; a < n_arrows; a++) {
            float x = cos(moving_group_angle) * move_group_arrows[a];
            float y = sin(moving_group_angle) * move_group_arrows[a];
            draw_sprite(
                bmp_move_group_arrow,
                leaders[cur_leader_nr]->x + x, leaders[cur_leader_nr]->y + y,
                16, 26,
                moving_group_angle);
        }
        
        size_t n_rings = whistle_rings.size();
        for(size_t r = 0; r < n_rings; r++) {
            float x = leaders[cur_leader_nr]->x + cos(leaders[cur_leader_nr]->angle) * whistle_rings[r];
            float y = leaders[cur_leader_nr]->y + sin(leaders[cur_leader_nr]->angle) * whistle_rings[r];
            unsigned char n = whistle_ring_colors[r];
            al_draw_filled_circle(x, y, 8, al_map_rgba(WHISTLE_RING_COLORS[n][0], WHISTLE_RING_COLORS[n][1], WHISTLE_RING_COLORS[n][2], 192));
        }
        
        if(whistle_radius > 0 || whistle_fade_time > 0) {
            if(pretty_whistle) {
                unsigned char n_dots = 16 * 6;
                for(unsigned char d = 0; d < 6; d++) {
                    for(unsigned char d2 = 0; d2 < 16; d2++) {
                        unsigned char current_dot = d2 * 6 + d;
                        float angle = M_PI * 2 / n_dots * current_dot + whistle_dot_offset;
                        
                        float x = cursor_x + cos(angle) * whistle_dot_radius[d];
                        float y = cursor_y + sin(angle) * whistle_dot_radius[d];
                        
                        ALLEGRO_COLOR c;
                        float alpha_mult;
                        if(whistle_fade_time > 0)
                            alpha_mult = whistle_fade_time / WHISTLE_FADE_TIME;
                        else
                            alpha_mult = 1;
                            
                        if(d == 0)        c = al_map_rgba(255, 0,   0,   255 * alpha_mult);
                        else if(d == 1) c = al_map_rgba(255, 128, 0,   210 * alpha_mult);
                        else if(d == 2) c = al_map_rgba(128, 255, 0,   165 * alpha_mult);
                        else if(d == 3) c = al_map_rgba(0,   255, 255, 120 * alpha_mult);
                        else if(d == 4) c = al_map_rgba(0,   0,   255, 75  * alpha_mult);
                        else            c = al_map_rgba(128, 0,   255, 30  * alpha_mult);
                        
                        al_draw_filled_circle(x, y, 2, c);
                    }
                }
            } else {
                unsigned char alpha = whistle_fade_time / WHISTLE_FADE_TIME * 255;
                float radius = whistle_fade_radius;
                if(whistle_radius > 0) {
                    alpha = 255;
                    radius = whistle_radius;
                }
                al_draw_circle(cursor_x, cursor_y, radius, al_map_rgba(192, 192, 0, alpha), 2);
            }
        }
        
        al_use_transform(&normal_transform);
        draw_sprite(
            bmp_mouse_cursor,
            mouse_cursor_x, mouse_cursor_y,
            cam_zoom * 48, cam_zoom * 48,
            leaders[cur_leader_nr]->angle);
        al_use_transform(&world_to_screen_transform);
        draw_sprite(
            bmp_cursor,
            cursor_x, cursor_y,
            48, 48,
            leaders[cur_leader_nr]->angle);
            
            
        /* Layer 4
        ****************
        *          \o/ *
        *   Mobs    |  *
        *          / \ *
        ***************/
        
        //Nectar.
        size_t n_nectars = nectars.size();
        for(size_t n = 0; n < n_nectars; n++) {
            float size = nectars[n]->size * (nectars[n]->amount_left + NECTAR_AMOUNT) / (NECTAR_AMOUNT * 2) * 2;
            draw_sprite(
                bmp_nectar,
                nectars[n]->x, nectars[n]->y,
                size, size);
        }
        
        //Treasures.
        for(size_t t = 0; t < n_treasures; t++) {
            al_draw_filled_circle(treasures[t]->x, treasures[t]->y, treasures[t]->size * 0.5, al_map_rgb(128, 255, 255));
        }
        
        //Pellets.
        size_t n_pellets = pellets.size();
        for(size_t p = 0; p < n_pellets; p++) {
            ALLEGRO_BITMAP* bm = NULL;
            if(pellets[p]->weight == 1) bm = bmp_red_pellet[0];
            else if(pellets[p]->weight == 5) bm = bmp_red_pellet[1];
            else if(pellets[p]->weight == 10) bm = bmp_red_pellet[2];
            else if(pellets[p]->weight == 20) bm = bmp_red_pellet[3];
            
            draw_sprite(
                bm,
                pellets[p]->x, pellets[p]->y,
                pellets[p]->size, pellets[p]->size);
        }
        
        //Pikmin
        n_pikmin = pikmin_list.size();
        for(size_t p = 0; p < n_pikmin; p++) {
            ALLEGRO_BITMAP* bm = NULL;
            
            bool idling = !pikmin_list[p]->following_party && !pikmin_list[p]->carrying_mob;
            
            if(pikmin_list[p]->type->name == "R") {
                if(pikmin_list[p]->burrowed) bm = bmp_red_burrowed[pikmin_list[p]->maturity];
                else if(idling) bm = bmp_red_idle[pikmin_list[p]->maturity];
                else bm = bmp_red[pikmin_list[p]->maturity];
            } else if(pikmin_list[p]->type->name == "Y") {
                if(pikmin_list[p]->burrowed) bm = bmp_yellow_burrowed[pikmin_list[p]->maturity];
                else if(idling) bm = bmp_yellow_idle[pikmin_list[p]->maturity];
                else bm = bmp_yellow[pikmin_list[p]->maturity];
            } if(pikmin_list[p]->type->name == "B") {
                if(pikmin_list[p]->burrowed) bm = bmp_blue_burrowed[pikmin_list[p]->maturity];
                else if(idling) bm = bmp_blue_idle[pikmin_list[p]->maturity];
                else bm = bmp_blue[pikmin_list[p]->maturity];
            }
            draw_sprite(
                bm,
                pikmin_list[p]->x, pikmin_list[p]->y,
                pikmin_list[p]->size, pikmin_list[p]->size,
                pikmin_list[p]->angle);
                
            if(idling) {
                draw_sprite(
                    bmp_idle_glow,
                    pikmin_list[p]->x, pikmin_list[p]->y,
                    30, 30,
                    idle_glow_angle,
                    change_alpha(pikmin_list[p]->main_color, 160));
            }
        }
        
        //Leaders.
        for(size_t l = 0; l < n_leaders; l++) {
            ALLEGRO_BITMAP* bm = NULL;
            if(leaders[l]->carrier_info) {
                bm = (l == 0) ? bmp_olimar_lying : ((l == 1) ? bmp_louie_lying : bmp_president_lying);
            } else {
                bm = (l == 0) ? bmp_olimar : ((l == 1) ? bmp_louie : bmp_president);
            }
            draw_sprite(
                bm,
                leaders[l]->x, leaders[l]->y,
                32,
                32,
                leaders[l]->angle
            );
        }
        
        //Onions.
        size_t n_onions = onions.size();
        for(size_t o = 0; o < n_onions; o++) {
            ALLEGRO_BITMAP* bm = NULL;
            if(onions[o]->type->name == "R") bm = bmp_red_onion;
            else if(onions[o]->type->name == "Y") bm = bmp_yellow_onion;
            if(onions[o]->type->name == "B") bm = bmp_blue_onion;
            
            draw_sprite(
                bm,
                onions[o]->x, onions[o]->y,
                185, 160,
                0, al_map_rgba(255, 255, 255, 224));
        }
        
        //Info spots.
        size_t n_info_spots = info_spots.size();
        for(size_t i = 0; i < n_info_spots; i++) {
            al_draw_filled_rectangle(
                info_spots[i]->x - info_spots[i]->size * 0.5,
                info_spots[i]->y - info_spots[i]->size * 0.5,
                info_spots[i]->x + info_spots[i]->size * 0.5,
                info_spots[i]->y + info_spots[i]->size * 0.5,
                al_map_rgb(192, 64, 192)
            );
        }
        
        //Ship(s).
        size_t n_ships = ships.size();
        for(size_t s = 0; s < n_ships; s++) {
            draw_sprite(
                bmp_ship,
                ships[s]->x, ships[s]->y,
                138, 112);
            al_draw_circle(ships[s]->x + ships[s]->size / 2 + SHIP_BEAM_RANGE, ships[s]->y, SHIP_BEAM_RANGE, al_map_rgb(ship_beam_ring_color[0], ship_beam_ring_color[1], ship_beam_ring_color[2]), 1);
        }
        
        
        /* Layer 5
        ***************************
        *                   Help  *
        *   In-game text   --  -- *
        *                    \/   *
        **************************/
        
        //Fractions.
        size_t n_mobs = mobs.size();
        for(size_t m = 0; m < n_mobs; m++) {
            if(!mobs[m]->carrier_info) continue;
            //ToDo it's not taking Pikmin strength into account.
            if(mobs[m]->carrier_info->current_n_carriers > 0) {
                ALLEGRO_COLOR color;
                if(mobs[m]->carrier_info->current_n_carriers >= mobs[m]->weight) { //Being carried.
                    if(mobs[m]->carrier_info->carry_to_ship) {
                        color = al_map_rgb(255, 255, 255); //ToDo what if Whites have an Onion on this game? Make it changeable per game.
                    } else {
                        color = mobs[m]->carrier_info->decided_type->color;
                    }
                } else {
                    color = al_map_rgb(96, 192, 192);
                }
                draw_fraction(mobs[m]->x, mobs[m]->y - mobs[m]->size * 0.5 - font_h * 1.25, mobs[m]->carrier_info->current_n_carriers, mobs[m]->weight, color);
            }
        }
        
        //Info spots.
        for(size_t i = 0; i < n_info_spots; i++) {
            if(dist(leaders[cur_leader_nr]->x, leaders[cur_leader_nr]->y, info_spots[i]->x, info_spots[i]->y) <= INFO_SPOT_TRIGGER_RANGE) {
                string text;
                if(!info_spots[i]->fullscreen)
                    text = info_spots[i]->text;
                else
                    text = "(...)";
                    
                al_draw_text(font, al_map_rgb(255, 255, 255), info_spots[i]->x, info_spots[i]->y - info_spots[i]->size * 0.5 - font_h * 2, ALLEGRO_ALIGN_CENTER, text.c_str());
                if(!info_spots[i]->fullscreen) {
                    int line_y = info_spots[i]->y - info_spots[i]->size * 0.5 - font_h * 0.75;
                    
                    al_draw_line(
                        info_spots[i]->x - info_spots[i]->text_w * 0.5,
                        line_y,
                        info_spots[i]->x - 8,
                        line_y,
                        al_map_rgb(192, 192, 192), 1);
                    al_draw_line(
                        info_spots[i]->x + info_spots[i]->text_w * 0.5,
                        line_y,
                        info_spots[i]->x + 8,
                        line_y,
                        al_map_rgb(192, 192, 192), 1);
                    al_draw_line(
                        info_spots[i]->x - 8,
                        line_y,
                        info_spots[i]->x,
                        info_spots[i]->y - info_spots[i]->size * 0.5 - font_h * 0.25,
                        al_map_rgb(192, 192, 192), 1);
                    al_draw_line(
                        info_spots[i]->x + 8,
                        line_y,
                        info_spots[i]->x,
                        info_spots[i]->y - info_spots[i]->size * 0.5 - font_h * 0.25,
                        al_map_rgb(192, 192, 192), 1);
                }
            }
        }
        
        
        /* Layer 6
        ***********************
        *                 *   *
        *   Particles   *   * *
        *                ***  *
        **********************/
        
        if(particle_quality > 0) {
            n_particles = particles.size();
            for(size_t p = 0; p < n_particles; p++) {
                if(particle_quality == 1) {
                    al_draw_filled_rectangle(
                        particles[p].x - particles[p].size * 0.5,
                        particles[p].y - particles[p].size * 0.5,
                        particles[p].x + particles[p].size * 0.5,
                        particles[p].y + particles[p].size * 0.5,
                        change_alpha(particles[p].color, (particles[p].time / particles[p].starting_time) * particles[p].color.a * 255)
                    );
                } else {
                    al_draw_filled_circle(
                        particles[p].x,
                        particles[p].y,
                        particles[p].size * 0.5,
                        change_alpha(particles[p].color, (particles[p].time / particles[p].starting_time) * particles[p].color.a * 255)
                    );
                }
            }
        }
        
        //ToDo remove:
        /*for(unsigned w = 0; w < leaders[cur_leader_nr]->party->party_spots->n_wheels; w++){
            size_t n_spots = leaders[cur_leader_nr]->party->party_spots->x_coords[w].size();
            for(unsigned s=0; s<n_spots; s++){
                al_draw_filled_circle(leaders[cur_leader_nr]->x + leaders[cur_leader_nr]->party->party_spots->x_coords[w][s], leaders[cur_leader_nr]->y + 100 + leaders[cur_leader_nr]->party->party_spots->y_coords[w][s], 4, al_map_rgb(0, 0, w * 32));
            }
        }*/
        
        
        /* Layer 7
        ***********************
        *              --==## *
        *   Daylight   --==## *
        *              --==## *
        **********************/
        
        al_use_transform(&normal_transform);
        
        if(daylight_effect) {
            al_draw_filled_rectangle(0, 0, scr_w, scr_h, get_daylight_color());
        }
        
        
        /* Layer 8
        *****************
        *           (1) *
        *   HUD         *
        *         1/2/3 *
        ****************/
        
        //Leader health.
        for(size_t l = 0; l < 3; l++) {
            if(n_leaders < l + 1) continue;
            
            size_t l_nr = (cur_leader_nr + l) % n_leaders;
            ALLEGRO_BITMAP* bm = (l_nr == 0) ? bmp_olimar : ((l_nr == 1) ? bmp_louie : bmp_president);
            
            int icons_size;
            if(l == 0) icons_size = 32; else icons_size = 20;
            
            int y_offset;
            if(l == 0) y_offset = 0; else if(l == 1) y_offset = 44; else y_offset = 80;
            
            draw_sprite(
                bm,
                32, scr_h - (32 + y_offset),
                icons_size, icons_size);
            draw_sprite(
                bmp_bubble,
                32, scr_h - (32 + y_offset),
                icons_size * 1.6, icons_size * 1.6);
                
            draw_health(
                32 + icons_size * 1.5,
                scr_h - (32 + y_offset),
                leaders[l_nr]->health, leaders[l_nr]->max_health,
                icons_size * 0.5, true);
            draw_sprite(
                bmp_health_bubble,
                32 + icons_size * 1.5,
                scr_h - (32 + y_offset),
                icons_size * 1.2, icons_size * 1.2);
        }
        
        //Day hour.
        al_draw_text(font, al_map_rgb(255, 255, 255), 8, 8, 0,
                     (to_string((long long) (day_minutes / 60)) + ":" + to_string((long long) ((int) (day_minutes) % 60))).c_str());
                     
        //Sun Meter.
        unsigned char n_hours = (day_minutes_end - day_minutes_start) / 60;
        float sun_meter_span = (scr_w - 150) - 20; //Width, from the center of the first dot to the center of the last.
        float interval = sun_meter_span / (float) n_hours;
        
        for(unsigned char h = 0; h < n_hours + 1; h++) {
            draw_sprite(
                bmp_bubble,
                (20 + h * interval), 40,
                16, 16);
        }
        
        draw_sprite(bmp_bubble, 20, 40, 24, 24); //Day start big circle.
        draw_sprite(bmp_bubble, 20 + sun_meter_span * 0.5, 40, 24, 24); //Day middle big circle.
        draw_sprite(bmp_bubble, 20 + sun_meter_span, 40, 24, 24); //Day end big circle.
        
        float day_passed_ratio = (float) (day_minutes - day_minutes_start) / (float) (day_minutes_end - day_minutes_start);
        draw_sprite(
            bmp_health_bubble,
            20 + day_passed_ratio * sun_meter_span, 40,
            48, 48,
            0, al_map_rgba(255, 255, 128, 192)); //Bubble behind the Sun.
        draw_sprite(
            bmp_sun,
            20 + day_passed_ratio * sun_meter_span, 40,
            48, 48); //Static sun.
        draw_sprite(
            bmp_sun,
            20 + day_passed_ratio * sun_meter_span, 40,
            48, 48,
            sun_meter_sun_angle); //Spinning sun.
            
        //Pikmin count.
        //Count how many Pikmin only.
        n_leaders = leaders.size();
        size_t pikmin_in_party = leaders[cur_leader_nr]->party->members.size();
        for(size_t l = 0; l < n_leaders; l++) {
            //If this leader is following the current one, then he's not a Pikmin, subtract him from the party count total.
            if(leaders[l]->following_party == leaders[cur_leader_nr]) pikmin_in_party--;
        }
        
        //Closest party member.
        if(closest_party_member) {
            ALLEGRO_BITMAP* bm = NULL;
            if(typeid(*closest_party_member) == typeid(pikmin)) {
                pikmin* pikmin_ptr = dynamic_cast<pikmin*>(closest_party_member);
                if(pikmin_ptr->type->name == "R") bm = bmp_red[pikmin_ptr->maturity];
                else if(pikmin_ptr->type->name == "Y") bm = bmp_yellow[pikmin_ptr->maturity];
                else if(pikmin_ptr->type->name == "B") bm = bmp_blue[pikmin_ptr->maturity];
            } else if(typeid(*closest_party_member) == typeid(leader)) {
                leader* leader_ptr = dynamic_cast<leader*>(closest_party_member);
                if(leader_ptr == leaders[0]) bm = bmp_olimar;
                else if(leader_ptr == leaders[1]) bm = bmp_louie;
                else if(leader_ptr == leaders[2]) bm = bmp_president;
            }
            
            if(bm) {
                draw_sprite(bm, 261, scr_h - font_h - 9, 32, 32);
            }
        }
        
        draw_sprite(
            bmp_bubble,
            260, scr_h - font_h - 10,
            40, 40);
            
        //Pikmin count numbers.
        size_t n_pikmin_types = pikmin_types.size();
        unsigned long total_pikmin = pikmin_list.size();
        for(size_t t = 0; t < n_pikmin_types; t++) total_pikmin += pikmin_in_onions[&pikmin_types[t]];
        
        al_draw_text(
            font, al_map_rgb(255, 255, 255), scr_w - 20, scr_h - 20 - font_h, ALLEGRO_ALIGN_RIGHT,
            (to_string((long long) pikmin_in_party) + "/" +
             to_string((long long) pikmin_list.size()) + "/" +
             to_string((long long) total_pikmin)).c_str()
        );
        
        //Day number.
        draw_sprite(
            bmp_day_bubble,
            scr_w - 50, 45,
            60, 70);
            
        al_draw_text(
            font, al_map_rgb(255, 255, 255), scr_w - 50, 40, ALLEGRO_ALIGN_CENTER,
            (to_string((long long) day)).c_str());
            
        //Sprays.
        if(n_spray_types > 0) {
            size_t top_spray_nr;
            if(n_spray_types < 3) top_spray_nr = 0; else top_spray_nr = selected_spray;
            
            draw_sprite(
                spray_types[top_spray_nr].bmp_spray,
                34, scr_h / 2 - 20,
                20, 24);
            al_draw_text(
                font, al_map_rgb(255, 255, 255), 48, scr_h / 2 - 32, 0,
                ("x" + to_string((long long) sprays[top_spray_nr])).c_str());
                
            if(n_spray_types == 2) {
                draw_sprite(
                    spray_types[1].bmp_spray,
                    34, scr_h / 2 + 20,
                    20, 24);
                al_draw_text(
                    font, al_map_rgb(255, 255, 255), 48, scr_h / 2 + 8, 0,
                    ("x" + to_string((long long) sprays[1])).c_str());
            }
            
        }
        
    } else { //Paused.
    
    }
    
    al_flip_display();
}
