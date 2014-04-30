#include <algorithm>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "animation.h"
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
        ****************
        *          \o/ *
        *   Mobs    |  *
        *          / \ *
        ***************/
        
        //Nectar.
        size_t n_nectars = nectars.size();
        for(size_t n = 0; n < n_nectars; n++) {
            float size = nectars[n]->type->size * (nectars[n]->amount_left + NECTAR_AMOUNT) / (NECTAR_AMOUNT * 2) * 2;
            draw_sprite(
                bmp_nectar,
                nectars[n]->x, nectars[n]->y,
                size, size);
        }
        
        //Treasures.
        for(size_t t = 0; t < n_treasures; t++) {
            float size = treasures[t]->type->size;
            if(treasures[t]->state == MOB_STATE_BEING_DELIVERED) {
                size *= 1 - (treasures[t]->time_in_state / DELIVERY_SUCK_TIME);
                size = max(0.0f, size);
            }
            draw_sprite(
                bmp_tp,
                treasures[t]->x, treasures[t]->y,
                size, size
            );
        }
        
        //Pellets.
        size_t n_pellets = pellets.size();
        for(size_t p = 0; p < n_pellets; p++) {
            ALLEGRO_BITMAP* bm = NULL;
            if(pellets[p]->type->weight == 1) bm = bmp_red_pellet[0];
            else if(pellets[p]->type->weight == 5) bm = bmp_red_pellet[1];
            else if(pellets[p]->type->weight == 10) bm = bmp_red_pellet[2];
            else if(pellets[p]->type->weight == 20) bm = bmp_red_pellet[3];
            
            draw_sprite(
                bm,
                pellets[p]->x, pellets[p]->y,
                pellets[p]->type->size, pellets[p]->type->size);
        }
        
        //Enemies.
        size_t n_enemies = enemies.size();
        for(size_t e = 0; e < n_enemies; e++) {
            enemy* e_ptr = enemies[e];
            frame* f_ptr = e_ptr->anim.get_frame();
            if(f_ptr) {
                float c = cos(e_ptr->angle), s = sin(e_ptr->angle);
                //ToDo test if stuff that offsets both verticall and horizontally is working. I know it's working for horizontal only.
                float width = f_ptr->game_w;
                float height = f_ptr->game_h;
                if(e_ptr->state == MOB_STATE_BEING_DELIVERED) {
                    float mult = 1 - (e_ptr->time_in_state / DELIVERY_SUCK_TIME);
                    width = max(0.0f, width * mult);
                    height = max(0.0f, height * mult);
                }
                
                draw_sprite(
                    f_ptr->bitmap,
                    e_ptr->x + c * f_ptr->offs_x + c * f_ptr->offs_y,
                    e_ptr->y - s * f_ptr->offs_y + s * f_ptr->offs_x,
                    width, height,
                    e_ptr->angle
                );
            }
        }
        
        //Pikmin.
        n_pikmin = pikmin_list.size();
        for(size_t p = 0; p < n_pikmin; p++) {
            pikmin* pik_ptr = pikmin_list[p];
            bool idling = !pik_ptr->following_party && !pik_ptr->carrying_mob && !pik_ptr->attacking_mob && !pik_ptr->was_thrown;
            
            frame* f = pik_ptr->anim.get_frame();
            if(f) {
                float c = cos(pik_ptr->angle), s = sin(pik_ptr->angle);
                float sprite_x = pik_ptr->x + c * f->offs_x + c * f->offs_y;
                float sprite_y = pik_ptr->y - s * f->offs_y + s * f->offs_x;
                draw_sprite(
                    f->bitmap,
                    sprite_x, sprite_y,
                    f->game_w + pik_ptr->z * 0.1, f->game_h + pik_ptr->z * 0.1,
                    pik_ptr->angle
                );
                
                if(idling) {
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
                    draw_sprite(
                        f->bitmap,
                        sprite_x, sprite_y,
                        f->game_w + pik_ptr->z * 0.1, f->game_h + pik_ptr->z * 0.1,
                        pik_ptr->angle
                    );
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
                }
                
                if(f->top_visible) {
                    draw_sprite(
                        pik_ptr->pik_type->bmp_top[pik_ptr->maturity],
                        sprite_x + c * f->top_x + c * f->top_y,
                        sprite_y - s * f->top_y + s * f->top_x,
                        f->top_w, f->top_h,
                        f->top_angle + pik_ptr->angle
                    );
                }
                
                if(idling) {
                    draw_sprite(
                        bmp_idle_glow,
                        pik_ptr->x, pik_ptr->y,
                        30, 30,
                        idle_glow_angle,
                        change_alpha(pik_ptr->type->main_color, 160));
                }
                
            }
            
        }
        
        //Leaders.
        for(size_t l = 0; l < n_leaders; l++) {
            frame* f = leaders[l]->anim.get_frame();
            if(f) {
                float c = cos(leaders[l]->angle), s = sin(leaders[l]->angle);
                draw_sprite(
                    f->bitmap,
                    leaders[l]->x + c * f->offs_x + c * f->offs_y,
                    leaders[l]->y - s * f->offs_y + s * f->offs_x,
                    f->game_w, f->game_h,
                    leaders[l]->angle
                );
            }
        }
        
        //Onions.
        size_t n_onions = onions.size();
        for(size_t o = 0; o < n_onions; o++) {
            ALLEGRO_BITMAP* bm = NULL;
            if(onions[o]->type->name == "Red onion") bm = bmp_red_onion;
            else if(onions[o]->type->name == "Yellow onion") bm = bmp_yellow_onion;
            else if(onions[o]->type->name == "Blue onion") bm = bmp_blue_onion;
            
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
                info_spots[i]->x - info_spots[i]->type->size * 0.5,
                info_spots[i]->y - info_spots[i]->type->size * 0.5,
                info_spots[i]->x + info_spots[i]->type->size * 0.5,
                info_spots[i]->y + info_spots[i]->type->size * 0.5,
                al_map_rgb(192, 64, 192)
            );
            al_draw_text(
                font, al_map_rgb(255, 255, 255),
                info_spots[i]->x, info_spots[i]->y - font_h / 2,
                ALLEGRO_ALIGN_CENTER, "?"
            );
        }
        
        //Ship(s).
        size_t n_ships = ships.size();
        for(size_t s = 0; s < n_ships; s++) {
            draw_sprite(
                bmp_ship,
                ships[s]->x, ships[s]->y,
                138, 112);
            al_draw_circle(ships[s]->x + ships[s]->type->size / 2 + SHIP_BEAM_RANGE, ships[s]->y, SHIP_BEAM_RANGE, al_map_rgb(ship_beam_ring_color[0], ship_beam_ring_color[1], ship_beam_ring_color[2]), 1);
        }
        
        //ToDo debugging -- remove.
        /*for(size_t m = 0; m < mobs.size(); m++) {
            if(typeid(*mobs[m]) == typeid(pikmin)) continue;
            mob* m_ptr = mobs[m];
            frame* f_ptr = m_ptr->anim.get_frame();
            if(f_ptr == NULL) continue; //ToDo report
        
            for(size_t h = 0; h < f_ptr->hitboxes.size(); h++) {
                hitbox* h_ptr = &f_ptr->hitboxes[h];
                float s = sin(m_ptr->angle);
                float c = cos(m_ptr->angle);
                float h_x = m_ptr->x + (h_ptr->x * c - h_ptr->y * s);
                float h_y = m_ptr->y + (h_ptr->x * s + h_ptr->y * c);
        
                al_draw_filled_circle(h_x, h_y, h_ptr->radius, al_map_rgba(128, 0, 0, 192));
            }
        }*/
        
        
        /* Layer 4
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
            float x = leaders[cur_leader_nr]->x + cos(cursor_angle) * whistle_rings[r];
            float y = leaders[cur_leader_nr]->y + sin(cursor_angle) * whistle_rings[r];
            unsigned char n = whistle_ring_colors[r];
            al_draw_circle(x, y, 8, al_map_rgba(WHISTLE_RING_COLORS[n][0], WHISTLE_RING_COLORS[n][1], WHISTLE_RING_COLORS[n][2], 192), 3);
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
                            
                        if(d == 0)      c = al_map_rgba(255, 0,   0,   255 * alpha_mult);
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
        
        //Cursor trail
        al_use_transform(&normal_transform);
        if(draw_cursor_trail) {
            for(size_t p = 1; p < cursor_spots.size(); p++) {
                point* p_ptr = &cursor_spots[p];
                point* pp_ptr = &cursor_spots[p - 1]; //Previous point.
                if((*p_ptr) != (*pp_ptr) && dist(p_ptr->x, p_ptr->y, pp_ptr->x, pp_ptr->y) > 4) {
                    al_draw_line(
                        p_ptr->x, p_ptr->y,
                        pp_ptr->x, pp_ptr->y,
                        al_map_rgba(255, 0, 255, (p / (float) cursor_spots.size()) * 64),
                        p * 3
                    );
                }
            }
        }
        
        //The actual cursor and mouse cursor
        draw_sprite(
            bmp_mouse_cursor,
            mouse_cursor_x, mouse_cursor_y,
            cam_zoom * 54, cam_zoom * 54,
            cursor_spin_angle,
            al_map_rgba(255, 255, 255, (mouse_cursor_valid ? 255 : 255 * ((sin(cursor_invalid_effect) + 1) / 2)))
        );
        al_use_transform(&world_to_screen_transform);
        draw_sprite(
            bmp_cursor,
            cursor_x, cursor_y,
            54, 54,
            cursor_angle);
            
            
        /* Layer 5
        ***************************
        *                   Help  *
        *   In-game text   --  -- *
        *                    \/   *
        **************************/
        
        //Fractions and health.
        size_t n_mobs = mobs.size();
        for(size_t m = 0; m < n_mobs; m++) {
            mob* mob_ptr = mobs[m];
            
            if(mob_ptr->carrier_info) {
                if(mob_ptr->carrier_info->current_carrying_strength > 0) {
                    ALLEGRO_COLOR color;
                    if(mob_ptr->carrier_info->current_carrying_strength >= mob_ptr->type->weight && (mob_ptr->carrier_info->decided_type || mob_ptr->carrier_info->carry_to_ship)) { //Being carried.
                        if(mob_ptr->carrier_info->carry_to_ship) {
                            color = al_map_rgb(255, 255, 255); //ToDo what if Whites have an Onion on this game? Make it changeable per game.
                        } else {
                            color = mob_ptr->carrier_info->decided_type->main_color;
                        }
                    } else {
                        color = al_map_rgb(96, 192, 192);
                    }
                    draw_fraction(mob_ptr->x, mob_ptr->y - mob_ptr->type->size * 0.5 - font_h * 1.25, mob_ptr->carrier_info->current_carrying_strength, mob_ptr->type->weight, color);
                }
            }
            
            if(mob_ptr->health < mob_ptr->type->max_health && mob_ptr->health > 0) {
                draw_health(mob_ptr->x, mob_ptr->y - mob_ptr->type->size - 8, mob_ptr->health, mob_ptr->type->max_health);
            }
        }
        
        //Info spots.
        for(size_t i = 0; i < n_info_spots; i++) {
            if(dist(leaders[cur_leader_nr]->x, leaders[cur_leader_nr]->y, info_spots[i]->x, info_spots[i]->y) <= INFO_SPOT_TRIGGER_RANGE) {
                string text;
                if(!info_spots[i]->opens_box)
                    text = info_spots[i]->text;
                else
                    text = "(...)";
                    
                draw_text_lines(font, al_map_rgb(255, 255, 255), info_spots[i]->x, info_spots[i]->y - info_spots[i]->type->size * 0.5 - font_h, ALLEGRO_ALIGN_CENTER, 2, text);
                if(!info_spots[i]->opens_box) {
                    int line_y = info_spots[i]->y - info_spots[i]->type->size * 0.5 - font_h * 0.75;
                    
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
                        info_spots[i]->y - info_spots[i]->type->size * 0.5 - font_h * 0.25,
                        al_map_rgb(192, 192, 192), 1);
                    al_draw_line(
                        info_spots[i]->x + 8,
                        line_y,
                        info_spots[i]->x,
                        info_spots[i]->y - info_spots[i]->type->size * 0.5 - font_h * 0.25,
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
                particle* p_ptr = &particles[p];
                
                if(p_ptr->type == PARTICLE_TYPE_SQUARE) {
                    al_draw_filled_rectangle(
                        p_ptr->x - p_ptr->size * 0.5,
                        p_ptr->y - p_ptr->size * 0.5,
                        p_ptr->x + p_ptr->size * 0.5,
                        p_ptr->y + p_ptr->size * 0.5,
                        change_alpha(p_ptr->color, (p_ptr->time / p_ptr->duration) * p_ptr->color.a * 255)
                    );
                    
                } else if(p_ptr->type == PARTICLE_TYPE_CIRCLE) {
                    al_draw_filled_circle(
                        p_ptr->x,
                        p_ptr->y,
                        p_ptr->size * 0.5,
                        change_alpha(p_ptr->color, (p_ptr->time / p_ptr->duration) * p_ptr->color.a * 255)
                    );
                    
                } else if(p_ptr->type == PARTICLE_TYPE_BITMAP) {
                    draw_sprite(
                        p_ptr->bitmap,
                        p_ptr->x,
                        p_ptr->y,
                        p_ptr->size, p_ptr->size,
                        0, change_alpha(p_ptr->color, (p_ptr->time / p_ptr->duration) * p_ptr->color.a * 255)
                    );
                    
                } else if(p_ptr->type == PARTICLE_TYPE_PIKMIN_SPIRIT) {
                    draw_sprite(
                        p_ptr->bitmap, p_ptr->x, p_ptr->y, p_ptr->size, -1,
                        0, change_alpha(p_ptr->color,
                                        abs(sin((p_ptr->time / p_ptr->duration) * M_PI)) * p_ptr->color.a * 255
                                       )
                    );
                    
                } else if(p_ptr->type == PARTICLE_TYPE_ENEMY_SPIRIT) {
                    float s = sin((p_ptr->time / p_ptr->duration) * M_PI);
                    draw_sprite(
                        p_ptr->bitmap, p_ptr->x + s * 16, p_ptr->y, p_ptr->size, -1,
                        s * M_PI, change_alpha(p_ptr->color, abs(s) * p_ptr->color.a * 255)
                    );
                    
                } else if(p_ptr->type == PARTICLE_TYPE_SMACK) {
                    float r = p_ptr->time / p_ptr->duration;
                    float size = p_ptr->size;
                    float opacity = 255;
                    if(r <= 0.5) size *= r * 2;
                    else opacity *= (1 - r) * 2;
                    
                    draw_sprite(p_ptr->bitmap, p_ptr->x, p_ptr->y, size, size, 0, change_alpha(p_ptr->color, opacity));
                }
            }
        }
        
        
        /* Layer 7
        ***************************
        *                    /  / *
        *   Percipitation     / / *
        *                   /  /  *
        **************************/
        
        if(cur_weather.percipitation_type != PERCIPITATION_TYPE_NONE) {
            size_t n_percipitation_particles = percipitation.size();
            for(size_t p = 0; p < n_percipitation_particles; p++) {
                al_draw_filled_circle(percipitation[p].x, percipitation[p].y, 3, al_map_rgb(255, 255, 255));
            }
        }
        
        
        /* Layer 8
        ***********************
        *              --==## *
        *   Daylight   --==## *
        *              --==## *
        **********************/
        
        al_use_transform(&normal_transform);
        
        if(daylight_effect) {
            al_draw_filled_rectangle(0, 0, scr_w, scr_h, get_daylight_color());
        }
        
        
        /* Layer 9
        *****************
        *           (1) *
        *   HUD         *
        *         1/2/3 *
        ****************/
        
        if(cur_message.size() == 0) {
        
            //Leader health.
            for(size_t l = 0; l < 3; l++) {
                if(n_leaders < l + 1) continue;
                
                size_t l_nr = (cur_leader_nr + l) % n_leaders;
                
                float size;
                if(l == 0) size = scr_w * 0.08; else size = scr_w * 0.06;
                
                int y_offset;
                if(l == 0) y_offset = 0; else if(l == 1) y_offset = scr_h * 0.10; else y_offset = scr_h * 0.19;
                
                //ToDo
                /*draw_sprite(
                    bm,
                    scr_w * 0.08, scr_h * 0.88 - y_offset,
                    size * 0.8, size * 0.8);*/
                draw_sprite(
                    bmp_bubble,
                    scr_w * 0.08, scr_h * 0.88 - y_offset,
                    size, size);
                    
                draw_health(
                    scr_w * 0.08 + size * 1.1,
                    scr_h * 0.88 - y_offset,
                    leaders[l_nr]->health, leaders[l_nr]->type->max_health,
                    size * 0.3, true);
                draw_sprite(
                    bmp_hard_bubble,
                    scr_w * 0.08 + size * 1.1,
                    scr_h * 0.88 - y_offset,
                    size * 0.8, size * 0.8);
            }
            
            //Sun Meter.
            unsigned char n_hours = (day_minutes_end - day_minutes_start) / 60;
            float sun_meter_start = scr_w * 0.06; //Center of the first dot.
            float sun_meter_end = scr_w * 0.70;
            float sun_meter_y = scr_h * 0.10; //Center.
            float sun_meter_span = sun_meter_end - sun_meter_start; //Width, from the center of the first dot to the center of the last.
            float interval = sun_meter_span / (float) n_hours;
            
            //Larger bubbles at the start, middle and end of the meter.
            draw_sprite(bmp_hard_bubble, sun_meter_start, sun_meter_y, scr_w * 0.03, scr_w * 0.03);
            draw_sprite(bmp_hard_bubble, sun_meter_start + sun_meter_span * 0.5, sun_meter_y, scr_w * 0.03, scr_w * 0.03);
            draw_sprite(bmp_hard_bubble, sun_meter_start + sun_meter_span, sun_meter_y, scr_w * 0.03, scr_w * 0.03);
            
            for(unsigned char h = 0; h < n_hours + 1; h++) {
                draw_sprite(
                    bmp_hard_bubble,
                    sun_meter_start + h * interval, sun_meter_y,
                    scr_w * 0.02, scr_w * 0.02);
            }
            
            float day_passed_ratio = (float) (day_minutes - day_minutes_start) / (float) (day_minutes_end - day_minutes_start);
            draw_sprite(
                bmp_sun,
                sun_meter_start + day_passed_ratio * sun_meter_span, sun_meter_y,
                scr_w * 0.07, scr_w * 0.07); //Static sun.
            draw_sprite(
                bmp_sun,
                sun_meter_start + day_passed_ratio * sun_meter_span, sun_meter_y,
                scr_w * 0.07, scr_w * 0.07,
                sun_meter_sun_angle); //Spinning sun.
            draw_sprite(
                bmp_sun_bubble,
                sun_meter_start + day_passed_ratio * sun_meter_span, sun_meter_y,
                scr_w * 0.08, scr_w * 0.08); //Bubble in front the Sun.
                
            //Day number.
            draw_sprite(
                bmp_day_bubble,
                scr_w * 0.89, scr_h * 0.13,
                scr_w * 0.11, scr_h * 0.18);
                
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255), scr_w * 0.89, scr_h * 0.15,
                ALLEGRO_ALIGN_CENTER, 1, scr_w * 0.09, scr_h * 0.07, itos(day));
                
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
                    //ToDo
                } else if(typeid(*closest_party_member) == typeid(leader)) {
                    leader* leader_ptr = dynamic_cast<leader*>(closest_party_member);
                    //ToDo
                }
                
                if(bm) {
                    draw_sprite(bm, scr_w * 0.30, scr_h * 0.89, scr_w * 0.06, scr_w * 0.06);
                }
            }
            
            draw_sprite(
                bmp_bubble,
                scr_w * 0.30, scr_h * 0.89,
                scr_w * 0.10, scr_w * 0.10);
            draw_compressed_text(font_counter, al_map_rgb(255, 255, 255), scr_w * 0.38, scr_h * 0.91, ALLEGRO_ALIGN_CENTER, 1, scr_w * 0.07, scr_h * 0.08, "x");
            
            //Pikmin count numbers.
            unsigned long total_pikmin = pikmin_list.size();
            for(auto o = pikmin_in_onions.begin(); o != pikmin_in_onions.end(); o++) total_pikmin += o->second;
            
            draw_sprite(bmp_number_bubble, scr_w * 0.50, scr_h * 0.90, scr_w * 0.16, scr_h * 0.1);
            draw_sprite(bmp_number_bubble, scr_w * 0.68, scr_h * 0.91, scr_w * 0.14, scr_h * 0.08);
            draw_sprite(bmp_number_bubble, scr_w * 0.87, scr_h * 0.91, scr_w * 0.19, scr_h * 0.08);
            draw_compressed_text(font_counter, al_map_rgb(255, 255, 255), scr_w * 0.59, scr_h * 0.92, ALLEGRO_ALIGN_CENTER, 1, scr_w * 0.04, scr_h * 0.08, "/");
            draw_compressed_text(font_counter, al_map_rgb(255, 255, 255), scr_w * 0.76, scr_h * 0.92, ALLEGRO_ALIGN_CENTER, 1, scr_w * 0.04, scr_h * 0.08, "/");
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255), scr_w * 0.57, scr_h * 0.90,
                ALLEGRO_ALIGN_RIGHT, 1, scr_w * 0.14, scr_h * 0.08,
                itos(pikmin_in_party)
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255), scr_w * 0.74, scr_h * 0.91,
                ALLEGRO_ALIGN_RIGHT, 1, scr_w * 0.12, scr_h * 0.05,
                itos(pikmin_list.size())
            );
            draw_compressed_text(
                font_counter, al_map_rgb(255, 255, 255), scr_w * 0.955, scr_h * 0.91,
                ALLEGRO_ALIGN_RIGHT, 1, scr_w * 0.17, scr_h * 0.05,
                itos(total_pikmin)
            );
            
            //Sprays.
            if(n_spray_types > 0) {
                size_t top_spray_nr;
                if(n_spray_types < 3) top_spray_nr = 0; else top_spray_nr = selected_spray;
                
                draw_sprite(
                    spray_types[top_spray_nr].bmp_spray,
                    scr_w * 0.06, scr_h * 0.36,
                    scr_w * 0.04, scr_h * 0.07);
                draw_compressed_text(
                    font_counter, al_map_rgb(255, 255, 255), scr_w * 0.10, scr_h * 0.37, ALLEGRO_ALIGN_CENTER, 1,
                    scr_w * 0.03, scr_h * 0.05, "x");
                draw_compressed_text(
                    font_counter, al_map_rgb(255, 255, 255), scr_w * 0.11, scr_h * 0.37, 0, 1,
                    scr_w * 0.06, scr_h * 0.05,
                    itos(spray_amounts[top_spray_nr]));
                for(size_t c = 0; c < controls.size(); c++) {
                    if(controls[c].action == BUTTON_USE_SPRAY_1 || controls[c].action == BUTTON_USE_SPRAY) {
                        draw_control(font, controls[c], scr_w * 0.10, scr_h * 0.42, scr_w * 0.10, scr_h * 0.05);
                        break;
                    }
                }
                
                if(n_spray_types == 2) {
                    draw_sprite(
                        spray_types[1].bmp_spray,
                        scr_w * 0.06, scr_h * 0.52,
                        scr_w * 0.04, scr_h * 0.07);
                    draw_compressed_text(
                        font_counter, al_map_rgb(255, 255, 255), scr_w * 0.10, scr_h * 0.53, ALLEGRO_ALIGN_CENTER, 1,
                        scr_w * 0.03, scr_h * 0.05, "x");
                    draw_compressed_text(
                        font_counter, al_map_rgb(255, 255, 255), scr_w * 0.11, scr_h * 0.53, 0, 1,
                        scr_w * 0.06, scr_h * 0.05,
                        itos(spray_amounts[1]));
                    for(size_t c = 0; c < controls.size(); c++) {
                        if(controls[c].action == BUTTON_USE_SPRAY_2) {
                            draw_control(font, controls[c], scr_w * 0.10, scr_h * 0.47, scr_w * 0.10, scr_h * 0.05);
                            break;
                        }
                    }
                    
                } else if(n_spray_types > 2) {
                    draw_sprite(
                        spray_types[(selected_spray == 0 ? spray_types.size() - 1 : selected_spray - 1)].bmp_spray,
                        scr_w * 0.06, scr_h * 0.52,
                        scr_w * 0.03, scr_h * 0.05);
                    draw_sprite(
                        spray_types[(selected_spray + 1) % spray_types.size()].bmp_spray,
                        scr_w * 0.13, scr_h * 0.52,
                        scr_w * 0.03, scr_h * 0.05);
                    for(size_t c = 0; c < controls.size(); c++) {
                        if(controls[c].action == BUTTON_SWITCH_SPRAY_LEFT) {
                            draw_control(font, controls[c], scr_w * 0.06, scr_h * 0.47, scr_w * 0.04, scr_h * 0.04);
                            break;
                        }
                    }
                    for(size_t c = 0; c < controls.size(); c++) {
                        if(controls[c].action == BUTTON_SWITCH_SPRAY_RIGHT) {
                            draw_control(font, controls[c], scr_w * 0.13, scr_h * 0.47, scr_w * 0.04, scr_h * 0.04);
                            break;
                        }
                    }
                }
            }
            
            //ToDo test stuff, remove.
            //Day hour.
            /*al_draw_text(font, al_map_rgb(255, 255, 255), 8, 8, 0,
                         (itos((day_minutes / 60)) + ":" + itos(((int) (day_minutes) % 60))).c_str());
            for(size_t p = 0; p < 7; p++) { draw_sprite(bmp_test, 25, 20 + 24 * p, 14, 24); }
            draw_sprite(bmp_test, 10, 20 + ((24 * 6) - pikmin_list[0]->z / 2), 14, 24);
            al_draw_text(font, al_map_rgb(255, 128, 128), 0, 0, 0, ftos(pikmin_list[0]->z).c_str());*/
            
        } else { //Show a message.
        
            draw_sprite(
                bmp_message_box,
                scr_w / 2, scr_h - font_h * 2 - 4, scr_w - 16, font_h * 4
            );
            
            if(cur_message_speaker) {
                draw_sprite(
                    cur_message_speaker,
                    40, scr_h - font_h * 4 - 16,
                    48, 48);
                draw_sprite(
                    bmp_bubble,
                    40, scr_h - font_h * 4 - 16,
                    64, 64);
            }
            
            string text = cur_message.substr(cur_message_stopping_chars[cur_message_section], cur_message_char - cur_message_stopping_chars[cur_message_section]);
            vector<string> lines = split(text, "\n");
            
            for(size_t l = 0; l < lines.size(); l++) {
            
                draw_compressed_text(
                    font, al_map_rgb(255, 255, 255),
                    24, scr_h - font_h * (4 - l) + 8,
                    ALLEGRO_ALIGN_LEFT, 0, scr_w - 64, 0,
                    lines[l]
                );
                
            }
            
        }
        
    } else { //Paused.
    
    }
    
    al_flip_display();
}
