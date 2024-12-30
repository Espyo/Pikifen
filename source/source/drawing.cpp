/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Global drawing functions.
 */

#include <algorithm>
#include <typeinfo>

#include "drawing.h"

#include "animation.h"
#include "const.h"
#include "functions.h"
#include "game.h"
#include "game_states/gameplay/gameplay.h"
#include "mobs/group_task.h"
#include "mobs/pile.h"
#include "mobs/scale.h"
#include "utils/allegro_utils.h"
#include "utils/drawing_utils.h"
#include "utils/drawing_utils.h"
#include "utils/general_utils.h"
#include "utils/geometry_utils.h"
#include "utils/string_utils.h"

namespace CONTROL_BIND_ICON {

//Base rectangle outline color.
const ALLEGRO_COLOR BASE_OUTLINE_COLOR = {0.10f, 0.10f, 0.10f, 1.0f};

//Base rectangle body color.
const ALLEGRO_COLOR BASE_RECT_COLOR = {0.45f, 0.45f, 0.45f, 1.0f};

//Base text color.
const ALLEGRO_COLOR BASE_TEXT_COLOR = {0.95f, 0.95f, 0.95f, 1.0f};

//Rectangle outline thickness.
const float OUTLINE_THICKNESS = 2.0f;

//Padding between text and rectangle limit.
const float PADDING = 4.0f;

}


namespace DRAWING {

//Default healt wheel radius.
const float DEF_HEALTH_WHEEL_RADIUS = 20;

//Liquid surfaces wobble by offsetting X by this much, at most.
const float LIQUID_WOBBLE_DELTA_X = 3.0f;

//Liquid surfaces wobble using this time scale.
const float LIQUID_WOBBLE_TIME_SCALE = 2.0f;

//Loading screen subtext padding.
const int LOADING_SCREEN_PADDING = 64;

//Loading screen subtext scale.
const float LOADING_SCREEN_SUBTEXT_SCALE = 0.6f;

//Loading screen text height, in screen ratio.
const float LOADING_SCREEN_TEXT_HEIGHT = 0.10f;

//Loading screen text width, in screen ratio.
const float LOADING_SCREEN_TEXT_WIDTH = 0.70f;

//Notification opacity.
const unsigned char NOTIFICATION_ALPHA = 160;

//Size of a control bind icon in a notification.
const float NOTIFICATION_CONTROL_SIZE = 24.0f;

//Padding between a notification's text and its limit.
const float NOTIFICATION_PADDING = 8.0f;

}


/**
 * @brief Draws a series of logos, to serve as a background.
 * They move along individually, and wrap around when they reach a screen edge.
 *
 * @param time_spent How much time has passed.
 * @param rows Rows of logos to draw.
 * @param cols Columns of logos to draw.
 * @param logo_size Width and height of the logos.
 * @param tint Tint the logos with this color.
 * @param speed Horizontal and vertical movement speed of each logo.
 * @param rotation_speed Rotation speed of each logo.
 */
void draw_background_logos(
    float time_spent, size_t rows, size_t cols,
    const point &logo_size, const ALLEGRO_COLOR &tint,
    const point &speed, float rotation_speed
) {
    al_hold_bitmap_drawing(true);
    
    float spacing_x = (game.win_w + logo_size.x) / cols;
    float spacing_y = (game.win_h + logo_size.y) / rows;
    
    for(size_t c = 0; c < cols; c++) {
        for(size_t r = 0; r < rows; r++) {
            float x = (c * spacing_x) + time_spent * speed.x;
            if(r % 2 == 0) {
                x += spacing_x / 2.0f;
            }
            x =
                wrap_float(
                    x,
                    0 - logo_size.x * 0.5f,
                    game.win_w + logo_size.x * 0.5f
                );
            float y =
                wrap_float(
                    (r * spacing_y) + time_spent * speed.y,
                    0 - logo_size.y * 0.5f,
                    game.win_h + logo_size.y * 0.5f
                );
            draw_bitmap(
                game.sys_assets.bmp_icon,
                point(x, y),
                point(logo_size.x, logo_size.y),
                time_spent * rotation_speed,
                tint
            );
        }
    }
    
    al_hold_bitmap_drawing(false);
}


/**
 * @brief Draws a bitmap, applying bitmap effects.
 *
 * @param bmp The bitmap.
 * @param effects Effects to use.
 */
void draw_bitmap_with_effects(
    ALLEGRO_BITMAP* bmp, const bitmap_effect_t &effects
) {

    point bmp_size(al_get_bitmap_width(bmp), al_get_bitmap_height(bmp));
    float scale_x =
        (effects.scale.x == LARGE_FLOAT) ? effects.scale.y : effects.scale.x;
    float scale_y =
        (effects.scale.y == LARGE_FLOAT) ? effects.scale.x : effects.scale.y;
    al_draw_tinted_scaled_rotated_bitmap(
        bmp,
        effects.tint_color,
        bmp_size.x / 2, bmp_size.y / 2,
        effects.translation.x, effects.translation.y,
        scale_x, scale_y,
        effects.rotation,
        0
    );
    
    if(effects.glow_color.a > 0) {
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(
            &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
        );
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
        al_draw_tinted_scaled_rotated_bitmap(
            bmp,
            effects.glow_color,
            bmp_size.x / 2, bmp_size.y / 2,
            effects.translation.x, effects.translation.y,
            scale_x, scale_y,
            effects.rotation,
            0
        );
        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
    }
}


/**
 * @brief Draws a button on the screen.
 *
 * @param center Center coordinates.
 * @param size Width and height.
 * @param text Text inside the button.
 * @param font What font to write the text in.
 * @param color Color to draw the text with.
 * @param selected Is the button currently selected?
 * @param juicy_grow_amount If it's in the middle of a juicy grow animation,
 * specify the amount here.
 */
void draw_button(
    const point &center, const point &size, const string &text,
    const ALLEGRO_FONT* font, const ALLEGRO_COLOR &color,
    bool selected, float juicy_grow_amount
) {
    draw_text(
        text, font, center, size * GUI::STANDARD_CONTENT_SIZE, color,
        ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW,
        point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount)
    );
    
    ALLEGRO_COLOR box_tint =
        selected ? al_map_rgb(87, 200, 208) : COLOR_WHITE;
        
    draw_textured_box(
        center, size, game.sys_assets.bmp_bubble_box, box_tint
    );
    
    if(selected) {
        draw_textured_box(
            center,
            size + 10.0 + sin(game.time_passed * TAU) * 2.0f,
            game.sys_assets.bmp_focus_box
        );
    }
}


/**
 * @brief Draws a fraction, so one number above another, divided by a bar.
 * The top number usually represents the current value of some attribute,
 * and the bottom number usually represents the required value for some goal.
 *
 * @param bottom Bottom center point of the text.
 * @param value_nr Number that represents the current value.
 * @param requirement_nr Number that represents the requirement.
 * @param color Color of the fraction's text.
 * @param scale Scale the text by this much.
 */
void draw_fraction(
    const point &bottom, size_t value_nr,
    size_t requirement_nr, const ALLEGRO_COLOR &color, float scale
) {
    const float value_nr_y = bottom.y - IN_WORLD_FRACTION::ROW_HEIGHT * 3;
    const float value_nr_scale = value_nr >= requirement_nr ? 1.2f : 1.0f;
    draw_text(
        i2s(value_nr), game.sys_assets.fnt_value, point(bottom.x, value_nr_y),
        point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * scale),
        color, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_TOP, 0,
        point(value_nr_scale, value_nr_scale)
    );
    
    const float bar_y = bottom.y - IN_WORLD_FRACTION::ROW_HEIGHT * 2;
    draw_text(
        "-", game.sys_assets.fnt_value, point(bottom.x, bar_y),
        point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * scale),
        color, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_TOP, 0
    );
    
    float req_nr_y = bottom.y - IN_WORLD_FRACTION::ROW_HEIGHT;
    float req_nr_scale = requirement_nr > value_nr ? 1.2f : 1.0f;
    draw_text(
        i2s(requirement_nr), game.sys_assets.fnt_value,
        point(bottom.x, req_nr_y),
        point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * scale),
        color, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_TOP, 0,
        point(req_nr_scale, req_nr_scale)
    );
}


/**
 * @brief Draws a health wheel, with a pieslice that's fuller the more
 * HP is full.
 *
 * @param center Center of the wheel.
 * @param ratio Ratio of health that is filled. 0 is empty, 1 is full.
 * @param alpha Total opacity of the health wheel.
 * @param radius Radius of the wheel (the whole wheel, not just the pieslice).
 * @param just_chart If true, only draw the actual pieslice (pie-chart).
 * Used for leader HP on the HUD.
 */
void draw_health(
    const point &center,
    float ratio, float alpha,
    float radius, bool just_chart
) {
    ALLEGRO_COLOR c;
    if(ratio >= 0.5) {
        c = al_map_rgba_f(1 - (ratio - 0.5) * 2, 1, 0, alpha);
    } else {
        c = al_map_rgba_f(1, (ratio * 2), 0, alpha);
    }
    
    if(!just_chart) {
        al_draw_filled_circle(
            center.x, center.y, radius, al_map_rgba(0, 0, 0, 128 * alpha)
        );
    }
    al_draw_filled_pieslice(
        center.x, center.y, radius, -TAU / 4, -ratio * TAU, c
    );
    if(!just_chart) {
        al_draw_circle(
            center.x, center.y, radius + 1, al_map_rgba(0, 0, 0, alpha * 255), 2
        );
    }
}


/**
 * @brief Draws a liquid sector.
 *
 * @param s_ptr Pointer to the sector.
 * @param l_ptr Pointer to the liquid.
 * @param where X and Y offset.
 * @param scale Scale the sector by this much.
 * @param time How much time has passed. Used to animate.
 */
void draw_liquid(
    sector* s_ptr, liquid* l_ptr, const point &where, float scale,
    float time
) {

    if(!s_ptr) return;
    if(s_ptr->is_bottomless_pit) return;

    ALLEGRO_SHADER* liq_shader = game.shaders.get_shader(SHADER_TYPE_LIQUID);
    al_use_shader(liq_shader);
    float liquid_opacity_mult = 1.0f;
    if(s_ptr->draining_liquid) {
        liquid_opacity_mult =
            s_ptr->liquid_drain_left / GEOMETRY::LIQUID_DRAIN_DURATION;
    }
    float brightness_mult = s_ptr->brightness / 255.0;

    float liq_tint[4] = {
        l_ptr->main_color.r * brightness_mult,
        l_ptr->main_color.g * brightness_mult,
        l_ptr->main_color.b * brightness_mult,
        l_ptr->main_color.a
    };
    
    /*
        We need to get a list of edges that the shader needs to check, 
        this can extend to other sectors whenever a liquid occupies more than one sector,
        so we need to loop through all of the connected sectors.
    */
    vector<sector*> checked_s {s_ptr};
    vector<edge*> border_edges;

    for(size_t s = 0; s < checked_s.size(); s++) {
        sector* s2_ptr = checked_s[s];
        for(size_t e = 0; e < s2_ptr->edges.size(); e++) {
            edge* e_ptr = s2_ptr->edges[e];
            sector* u_s = nullptr;
            sector* a_s = nullptr;
            if(does_edge_have_liquid_limit(e_ptr, &u_s, &a_s)) {
                border_edges.push_back(e_ptr);
            }

            sector* other_ptr = e_ptr->get_other_sector(s2_ptr);
            if(other_ptr) {
                for(size_t h = 0; h < other_ptr->hazards.size(); h++) {
                    if(other_ptr->hazards[h]->associated_liquid) {
                        if(std::find(checked_s.begin(), checked_s.end(), other_ptr) == checked_s.end()) {
                            checked_s.push_back(other_ptr);
                        }
                    }
                }
            }
        }
    }

    uint edgeCount = border_edges.size();

    float buffer_edges[edgeCount * 4];

    for(size_t e = 0; e < edgeCount; e++) {
        edge* edge = border_edges[e];

        buffer_edges[4 * e    ] = edge->vertexes[0]->x;
        buffer_edges[4 * e + 1] = edge->vertexes[0]->y;
        buffer_edges[4 * e + 2] = edge->vertexes[1]->x;
        buffer_edges[4 * e + 3] = edge->vertexes[1]->y;
    }

    //Put the buffer onto the shader
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(buffer_edges), buffer_edges, GL_STREAM_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    al_set_shader_float("time", time);
    al_set_shader_float("fill_level", liquid_opacity_mult);
    al_set_shader_float("tex_brightness", brightness_mult);
    al_set_shader_int("edge_count", edgeCount);
    al_set_shader_float_vector("liq_tint", 4, &liq_tint[0], 1);
    
    //Draw the sector now!
    unsigned char n_textures = 1;
    sector* texture_sector[2] = {nullptr, nullptr};
    if(s_ptr->fade) {
        s_ptr->get_texture_merge_sectors(
            &texture_sector[0], &texture_sector[1]
        );
        if(!texture_sector[0] && !texture_sector[1]) {
            //Can't draw this sector.
            return;
        }
        n_textures = 2;
        
    } else {
        texture_sector[0] = s_ptr;
        
    }

    for(unsigned char t = 0; t < n_textures; t++) {
    
        bool draw_sector_0 = true;
        if(!texture_sector[0]) draw_sector_0 = false;
        else if(texture_sector[0]->is_bottomless_pit) {
            draw_sector_0 = false;
        }
        
        if(n_textures == 2 && !draw_sector_0 && t == 0) {
            //Allows fading into the void.
            continue;
        }
        
        if(!texture_sector[t] || texture_sector[t]->is_bottomless_pit) {
            continue;
        }
        size_t n_vertexes = s_ptr->triangles.size() * 3;
        ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
        
        sector_texture_t* texture_info_to_use =
            &texture_sector[t]->texture_info;

        //Texture transformations.
        ALLEGRO_TRANSFORM tra;
        al_build_transform(
            &tra,
            -texture_info_to_use->translation.x,
            -texture_info_to_use->translation.y,
            1.0f / texture_info_to_use->scale.x,
            1.0f / texture_info_to_use->scale.y,
            -texture_info_to_use->rot
        );
        
        float textureOffset[2] = {
            texture_info_to_use->translation.x, 
            texture_info_to_use->translation.y
        };
        float textureScale[2] = {
            texture_info_to_use->scale.x, 
            texture_info_to_use->scale.y
        };

        for(size_t v = 0; v < n_vertexes; v++) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            float alpha_mult = 1;
            float brightness_mult = texture_sector[t]->brightness / 255.0;
            
            if(t == 1) {
                if(!draw_sector_0) {
                    alpha_mult = 0;
                    for(
                        size_t e = 0; e < texture_sector[1]->edges.size(); e++
                    ) {
                        if(
                            texture_sector[1]->edges[e]->vertexes[0] == v_ptr ||
                            texture_sector[1]->edges[e]->vertexes[1] == v_ptr
                        ) {
                            alpha_mult = 1;
                        }
                    }
                } else {
                    for(
                        size_t e = 0; e < texture_sector[0]->edges.size(); e++
                    ) {
                        if(
                            texture_sector[0]->edges[e]->vertexes[0] == v_ptr ||
                            texture_sector[0]->edges[e]->vertexes[1] == v_ptr
                        ) {
                            alpha_mult = 0;
                        }
                    }
                }
            }
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            if(texture_sector[t]) al_transform_coordinates(&tra, &vx, &vy);
            av[v].u = vx;
            av[v].v = vy;
            av[v].z = 0;
            av[v].color =
                al_map_rgba_f(
                    texture_sector[t]->texture_info.tint.r * brightness_mult,
                    texture_sector[t]->texture_info.tint.g * brightness_mult,
                    texture_sector[t]->texture_info.tint.b * brightness_mult,
                    texture_sector[t]->texture_info.tint.a * alpha_mult
                );
        }
        
        for(size_t v = 0; v < n_vertexes; v++) {
            av[v].x *= scale;
            av[v].y *= scale;
        }
        
        ALLEGRO_BITMAP* tex =
            texture_sector[t] ?
            texture_sector[t]->texture_info.bitmap :
            texture_sector[t == 0 ? 1 : 0]->texture_info.bitmap;

        int bmpSize[2] = {
            al_get_bitmap_width(tex), 
            al_get_bitmap_height(tex)
        };
        al_set_shader_float_vector("tex_translation", 2, textureOffset, 1);
        al_set_shader_float_vector("tex_scale", 2, textureScale, 1);
        al_set_shader_float("tex_rotation", texture_info_to_use->rot);
        al_set_shader_int_vector("tex_size", 2, &bmpSize[0], 1);

        al_draw_prim(
            av, nullptr, tex,
            0, (int) n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
        
        delete[] av;
    }
    al_use_shader(NULL);

}



/**
 * @brief Draws the loading screen for an area (or anything else, really).
 *
 * @param text The main text to show, optional.
 * @param subtext Subtext to show under the main text, optional.
 * @param opacity 0 to 1. The background blackness lowers in opacity
 * much faster.
 */
void draw_loading_screen(
    const string &text, const string &subtext, float opacity
) {
    const float text_w = game.win_w * DRAWING::LOADING_SCREEN_TEXT_WIDTH;
    const float text_h = game.win_h * DRAWING::LOADING_SCREEN_TEXT_HEIGHT;
    const float subtext_w = text_w * DRAWING::LOADING_SCREEN_SUBTEXT_SCALE;
    const float subtext_h = text_h * DRAWING::LOADING_SCREEN_SUBTEXT_SCALE;
    
    unsigned char blackness_alpha = 255.0f * std::max(0.0f, opacity * 4 - 3);
    al_draw_filled_rectangle(
        0, 0, game.win_w, game.win_h, al_map_rgba(0, 0, 0, blackness_alpha)
    );
    
    int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
    al_get_separate_blender(
        &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
    );
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
    
    //Set up the bitmap that will hold the text if it doesn't exist.
    if(!text.empty() && !game.loading_text_bmp) {
        game.loading_text_bmp = al_create_bitmap(text_w, text_h);
        
        al_set_target_bitmap(game.loading_text_bmp); {
            al_clear_to_color(COLOR_EMPTY);
            draw_text(
                text, game.sys_assets.fnt_area_name,
                point(text_w * 0.5f, text_h * 0.5f),
                point(text_w, text_h),
                COLOR_GOLD, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0
            );
        } al_set_target_backbuffer(game.display);
        
    }
    
    //Set up the bitmap that will hold the text if it doesn't exist.
    if(!subtext.empty() && !game.loading_subtext_bmp) {
        game.loading_subtext_bmp = al_create_bitmap(subtext_w, subtext_h);
        
        al_set_target_bitmap(game.loading_subtext_bmp); {
            al_clear_to_color(COLOR_EMPTY);
            draw_text(
                subtext, game.sys_assets.fnt_area_name,
                point(subtext_w * 0.5f, subtext_h * 0.5f),
                point(subtext_w, subtext_h),
                al_map_rgb(224, 224, 224),
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0
            );
            
        } al_set_target_backbuffer(game.display);
        
    }
    
    al_set_separate_blender(
        old_op, old_src, old_dst, old_aop, old_asrc, old_adst
    );
    
    //Draw the text bitmap in its place.
    const float text_x = game.win_w * 0.5 - text_w * 0.5;
    float text_y = game.win_h * 0.5 - text_h * 0.5;
    if(!text.empty()) {
        if(!subtext.empty()) {
            text_y -= DRAWING::LOADING_SCREEN_PADDING * 0.5;
        }
        al_draw_tinted_bitmap(
            game.loading_text_bmp,
            al_map_rgba(255, 255, 255, 255.0 * opacity),
            game.win_w * 0.5 - text_w * 0.5, text_y, 0
        );
        
    }
    
    //Draw the subtext bitmap in its place.
    const float subtext_x = game.win_w * 0.5 - subtext_w * 0.5;
    float subtext_y = game.win_h * 0.5 + DRAWING::LOADING_SCREEN_PADDING * 0.5;
    if(!subtext.empty()) {
    
        al_draw_tinted_bitmap(
            game.loading_subtext_bmp,
            al_map_rgba(255, 255, 255, 255.0 * opacity),
            game.win_w * 0.5 - subtext_w * 0.5, subtext_y, 0
        );
        
    }
    
    const unsigned char reflection_alpha = 128.0 * opacity;
    
    //Now, draw the polygon that will hold the reflection for the text.
    if(!text.empty()) {
    
        ALLEGRO_VERTEX text_vertexes[4];
        const float text_reflection_h = text_h * 0.80f;
        //Top-left vertex.
        text_vertexes[0].x = text_x;
        text_vertexes[0].y = text_y + text_h;
        text_vertexes[0].z = 0;
        text_vertexes[0].u = 0;
        text_vertexes[0].v = text_h;
        text_vertexes[0].color = al_map_rgba(255, 255, 255, reflection_alpha);
        //Top-right vertex.
        text_vertexes[1].x = text_x + text_w;
        text_vertexes[1].y = text_y + text_h;
        text_vertexes[1].z = 0;
        text_vertexes[1].u = text_w;
        text_vertexes[1].v = text_h;
        text_vertexes[1].color = al_map_rgba(255, 255, 255, reflection_alpha);
        //Bottom-right vertex.
        text_vertexes[2].x = text_x + text_w;
        text_vertexes[2].y = text_y + text_h + text_reflection_h;
        text_vertexes[2].z = 0;
        text_vertexes[2].u = text_w;
        text_vertexes[2].v = text_h - text_reflection_h;
        text_vertexes[2].color = al_map_rgba(255, 255, 255, 0);
        //Bottom-left vertex.
        text_vertexes[3].x = text_x;
        text_vertexes[3].y = text_y + text_h + text_reflection_h;
        text_vertexes[3].z = 0;
        text_vertexes[3].u = 0;
        text_vertexes[3].v = text_h - text_reflection_h;
        text_vertexes[3].color = al_map_rgba(255, 255, 255, 0);
        
        al_draw_prim(
            text_vertexes, nullptr, game.loading_text_bmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //And the polygon for the subtext.
    if(!subtext.empty()) {
    
        ALLEGRO_VERTEX subtext_vertexes[4];
        const float subtext_reflection_h = subtext_h * 0.80f;
        //Top-left vertex.
        subtext_vertexes[0].x = subtext_x;
        subtext_vertexes[0].y = subtext_y + subtext_h;
        subtext_vertexes[0].z = 0;
        subtext_vertexes[0].u = 0;
        subtext_vertexes[0].v = subtext_h;
        subtext_vertexes[0].color =
            al_map_rgba(255, 255, 255, reflection_alpha);
        //Top-right vertex.
        subtext_vertexes[1].x = subtext_x + subtext_w;
        subtext_vertexes[1].y = subtext_y + subtext_h;
        subtext_vertexes[1].z = 0;
        subtext_vertexes[1].u = subtext_w;
        subtext_vertexes[1].v = subtext_h;
        subtext_vertexes[1].color =
            al_map_rgba(255, 255, 255, reflection_alpha);
        //Bottom-right vertex.
        subtext_vertexes[2].x = subtext_x + subtext_w;
        subtext_vertexes[2].y = subtext_y + subtext_h + subtext_reflection_h;
        subtext_vertexes[2].z = 0;
        subtext_vertexes[2].u = subtext_w;
        subtext_vertexes[2].v = subtext_h - subtext_reflection_h;
        subtext_vertexes[2].color = al_map_rgba(255, 255, 255, 0);
        //Bottom-left vertex.
        subtext_vertexes[3].x = subtext_x;
        subtext_vertexes[3].y = subtext_y + subtext_h + subtext_reflection_h;
        subtext_vertexes[3].z = 0;
        subtext_vertexes[3].u = 0;
        subtext_vertexes[3].v = subtext_h - subtext_reflection_h;
        subtext_vertexes[3].color = al_map_rgba(255, 255, 255, 0);
        
        al_draw_prim(
            subtext_vertexes, nullptr, game.loading_subtext_bmp,
            0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    }
    
    //Draw the game's logo to the left of the "Loading..." text,
    //if we're not fading.
    if(opacity == 1.0f) {
        const point text_box(game.win_w * 0.11f, game.win_h * 0.03f);
        
        if(
            game.sys_assets.bmp_icon &&
            game.sys_assets.bmp_icon != game.bmp_error
        ) {
            point icon_pos(
                game.win_w - 8 - text_box.x - 8 - text_box.y / 2.0f,
                game.win_h - 8 - text_box.y / 2.0f
            );
            draw_bitmap(
                game.sys_assets.bmp_icon, icon_pos,
                point(-1, text_box.y),
                0, al_map_rgba(255, 255, 255, opacity * 255.0)
            );
        }
        
        draw_text(
            "Loading...", game.sys_assets.fnt_standard,
            point(game.win_w - 8, game.win_h - 8), text_box,
            al_map_rgb(192, 192, 192), ALLEGRO_ALIGN_RIGHT, V_ALIGN_MODE_BOTTOM
        );
    }
    
}


/**
 * @brief Draws the icon for a menu button.
 *
 * @param icon Icon ID.
 * @param button_center Center coordinates of the button.
 * @param button_size Dimensions of the button.
 * @param left_side If true, place the icon to the left side of the button.
 * If false, place it to the right.
 */
void draw_menu_button_icon(
    MENU_ICON icon, const point &button_center, const point &button_size,
    bool left_side
) {
    //All icons are square, and in a row, so the spritesheet height works.
    int icon_size =
        al_get_bitmap_height(game.sys_assets.bmp_menu_icons);
    ALLEGRO_BITMAP* bmp =
        al_create_sub_bitmap(
            game.sys_assets.bmp_menu_icons,
            (icon_size + 1) * (int) icon, 0,
            icon_size, icon_size
        );
    point icon_center(
        left_side ?
        button_center.x - button_size.x * 0.5 + button_size.y * 0.5 :
        button_center.x + button_size.x * 0.5 - button_size.y * 0.5,
        button_center.y
    );
    draw_bitmap_in_box(
        bmp, icon_center,
        point(button_size.y, button_size.y),
        true
    );
    al_destroy_bitmap(bmp);
}


/**
 * @brief Draws a mob's shadow.
 *
 * @param m mob to draw the shadow for.
 * @param delta_z The mob is these many units above the floor directly below it.
 * @param shadow_stretch How much to stretch the shadow by
 * (used to simulate sun shadow direction casting).
 */
void draw_mob_shadow(
    const mob* m,
    float delta_z, float shadow_stretch
) {

    point shadow_size = point(m->radius * 2.2f, m->radius * 2.2f);
    if(m->rectangular_dim.x != 0) {
        shadow_size = m->rectangular_dim * 1.1f;
    }
    
    if(shadow_stretch <= 0) return;
    
    float diameter = shadow_size.x;
    float shadow_x = 0;
    float shadow_w =
        diameter + (diameter * shadow_stretch * MOB::SHADOW_STRETCH_MULT);
        
    if(game.states.gameplay->day_minutes < 60 * 12) {
        //Shadows point to the West.
        shadow_x = -shadow_w + diameter * 0.5;
        shadow_x -= shadow_stretch * delta_z * MOB::SHADOW_Y_MULT;
    } else {
        //Shadows point to the East.
        shadow_x = -(diameter * 0.5);
        shadow_x += shadow_stretch * delta_z * MOB::SHADOW_Y_MULT;
    }
    
    if(m->rectangular_dim.x != 0) {
        draw_bitmap(
            game.sys_assets.bmp_shadow_square,
            point(m->pos.x + shadow_x + shadow_w / 2, m->pos.y),
            shadow_size,
            m->angle,
            map_alpha(255 * (1 - shadow_stretch))
        );
    } else {
        draw_bitmap(
            game.sys_assets.bmp_shadow,
            point(m->pos.x + shadow_x + shadow_w / 2, m->pos.y),
            point(shadow_w, diameter),
            0,
            map_alpha(255 * (1 - shadow_stretch))
        );
    }
}


/**
 * @brief Draws the mouse cursor.
 *
 * @param color Color to tint it with.
 */
void draw_mouse_cursor(const ALLEGRO_COLOR &color) {
    al_use_transform(&game.identity_transform);
    
    //Cursor trail.
    if(game.options.draw_cursor_trail) {
        size_t anchor = 0;
        
        for(size_t s = 1; s < game.mouse_cursor.history.size(); s++) {
            point anchor_diff =
                game.mouse_cursor.history[anchor] -
                game.mouse_cursor.history[s];
            if(
                fabs(anchor_diff.x) < GAME::CURSOR_TRAIL_MIN_SPOT_DIFF &&
                fabs(anchor_diff.y) < GAME::CURSOR_TRAIL_MIN_SPOT_DIFF
            ) {
                continue;
            }
            
            float start_ratio =
                anchor / (float) game.mouse_cursor.history.size();
            float start_thickness =
                GAME::CURSOR_TRAIL_MAX_WIDTH * start_ratio;
            unsigned char start_alpha =
                GAME::CURSOR_TRAIL_MAX_ALPHA * start_ratio;
            ALLEGRO_COLOR start_color =
                change_alpha(color, start_alpha);
            point start_p1;
            point start_p2;
            
            float end_ratio =
                s / (float) GAME::CURSOR_TRAIL_SAVE_N_SPOTS;
            float end_thickness =
                GAME::CURSOR_TRAIL_MAX_WIDTH * end_ratio;
            unsigned char end_alpha =
                GAME::CURSOR_TRAIL_MAX_ALPHA * end_ratio;
            ALLEGRO_COLOR end_color =
                change_alpha(color, end_alpha);
            point end_p1;
            point end_p2;
            
            if(anchor == 0) {
                point cur_to_next =
                    game.mouse_cursor.history[s] -
                    game.mouse_cursor.history[anchor];
                point cur_to_next_normal(-cur_to_next.y, cur_to_next.x);
                cur_to_next_normal = normalize_vector(cur_to_next_normal);
                point spot_offset = cur_to_next_normal * start_thickness / 2.0f;
                start_p1 = game.mouse_cursor.history[anchor] - spot_offset;
                start_p2 = game.mouse_cursor.history[anchor] + spot_offset;
            } else {
                get_miter_points(
                    game.mouse_cursor.history[anchor - 1],
                    game.mouse_cursor.history[anchor],
                    game.mouse_cursor.history[anchor + 1],
                    -start_thickness,
                    &start_p1,
                    &start_p2,
                    30.0f
                );
            }
            
            if(s == game.mouse_cursor.history.size() - 1) {
                point prev_to_cur =
                    game.mouse_cursor.history[s] -
                    game.mouse_cursor.history[anchor];
                point prev_to_cur_normal(-prev_to_cur.y, prev_to_cur.x);
                prev_to_cur_normal = normalize_vector(prev_to_cur_normal);
                point spot_offset = prev_to_cur_normal * start_thickness / 2.0f;
                end_p1 = game.mouse_cursor.history[s] - spot_offset;
                end_p2 = game.mouse_cursor.history[s] + spot_offset;
            } else {
                get_miter_points(
                    game.mouse_cursor.history[s - 1],
                    game.mouse_cursor.history[s],
                    game.mouse_cursor.history[s + 1],
                    -end_thickness,
                    &end_p1,
                    &end_p2,
                    30.0f
                );
            }
            
            ALLEGRO_VERTEX vertexes[4];
            for(unsigned char v = 0; v < 4; v++) {
                vertexes[v].z = 0.0f;
            }
            
            vertexes[0].x = start_p1.x;
            vertexes[0].y = start_p1.y;
            vertexes[0].color = start_color;
            vertexes[1].x = start_p2.x;
            vertexes[1].y = start_p2.y;
            vertexes[1].color = start_color;
            vertexes[2].x = end_p1.x;
            vertexes[2].y = end_p1.y;
            vertexes[2].color = end_color;
            vertexes[3].x = end_p2.x;
            vertexes[3].y = end_p2.y;
            vertexes[3].color = end_color;
            
            al_draw_prim(
                vertexes, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP
            );
            
            anchor = s;
        }
    }
    
    //Mouse cursor graphic.
    draw_bitmap(
        game.sys_assets.bmp_mouse_cursor,
        game.mouse_cursor.s_pos,
        point(
            al_get_bitmap_width(game.sys_assets.bmp_mouse_cursor),
            al_get_bitmap_height(game.sys_assets.bmp_mouse_cursor)
        ),
        -(game.time_passed * game.config.cursor_spin_speed),
        color
    );
}


/**
 * @brief Draws an icon representing some control bind.
 *
 * @param font Font to use for the name, if necessary.
 * @param i Info on the player input. If invalid, a "NONE" icon will be used.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param where Center of the place to draw at.
 * @param max_size Max width or height. Used to compress it if needed.
 * 0 = unlimited.
 * @param alpha Opacity.
 */
void draw_player_input_icon(
    const ALLEGRO_FONT* const font, const player_input &i,
    bool condensed, const point &where, const point &max_size,
    unsigned char alpha
) {
    if(alpha == 0) return;
    
    //Final text color.
    const ALLEGRO_COLOR final_text_color =
        change_alpha(CONTROL_BIND_ICON::BASE_TEXT_COLOR, alpha);
        
    //Start by getting the icon's info for drawing.
    PLAYER_INPUT_ICON_SHAPE shape;
    PLAYER_INPUT_ICON_SPRITE bitmap_sprite;
    string text;
    get_player_input_icon_info(
        i, condensed,
        &shape, &bitmap_sprite, &text
    );
    
    //If it's a bitmap, just draw it and be done with it.
    if(shape == PLAYER_INPUT_ICON_SHAPE_BITMAP) {
        //All icons are square, and in a row, so the spritesheet height works.
        int icon_size =
            al_get_bitmap_height(game.sys_assets.bmp_player_input_icons);
        ALLEGRO_BITMAP* bmp =
            al_create_sub_bitmap(
                game.sys_assets.bmp_player_input_icons,
                (icon_size + 1) * (int) bitmap_sprite, 0,
                icon_size, icon_size
            );
        draw_bitmap_in_box(bmp, where, max_size, true, 0.0f, map_alpha(alpha));
        al_destroy_bitmap(bmp);
        return;
    }
    
    //The size of the rectangle will depend on the text within.
    int text_ox;
    int text_oy;
    int text_w;
    int text_h;
    al_get_text_dimensions(
        font, text.c_str(),
        &text_ox, &text_oy, &text_w, &text_h
    );
    float total_width =
        std::min(
            (float) (text_w + CONTROL_BIND_ICON::PADDING * 2),
            (max_size.x == 0 ? FLT_MAX : max_size.x)
        );
    float total_height =
        std::min(
            (float) (text_h + CONTROL_BIND_ICON::PADDING * 2),
            (max_size.y == 0 ? FLT_MAX : max_size.y)
        );
    //Force it to always be a square or horizontal rectangle. Never vertical.
    total_width = std::max(total_width, total_height);
    
    //Now, draw the rectangle, either sharp or rounded.
    switch(shape) {
    case PLAYER_INPUT_ICON_SHAPE_RECTANGLE: {
        draw_textured_box(
            where, point(total_width, total_height),
            game.sys_assets.bmp_key_box
        );
        break;
    }
    case PLAYER_INPUT_ICON_SHAPE_ROUNDED: {
        draw_textured_box(
            where, point(total_width, total_height),
            game.sys_assets.bmp_button_box
        );
        break;
    }
    case PLAYER_INPUT_ICON_SHAPE_BITMAP: {
        break;
    }
    }
    
    //And finally, the text inside.
    draw_text(
        text, font, where,
        point(
            (max_size.x == 0 ? 0 : max_size.x - CONTROL_BIND_ICON::PADDING),
            (max_size.y == 0 ? 0 : max_size.y - CONTROL_BIND_ICON::PADDING)
        ),
        final_text_color, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW | TEXT_SETTING_COMPENSATE_Y_OFFSET
    );
}


/**
 * @brief Draws a sector, but only the texture (no wall shadows).
 *
 * @param s_ptr Pointer to the sector.
 * @param where X and Y offset.
 * @param scale Scale the sector by this much.
 * @param opacity Draw the textures at this opacity, 0 - 1.
 */
void draw_sector_texture(
    sector* s_ptr, const point &where, float scale, float opacity
) {
    if(!s_ptr) return;
    if(s_ptr->is_bottomless_pit) return;
    
    unsigned char n_textures = 1;
    sector* texture_sector[2] = {nullptr, nullptr};
    
    if(s_ptr->fade) {
        s_ptr->get_texture_merge_sectors(
            &texture_sector[0], &texture_sector[1]
        );
        if(!texture_sector[0] && !texture_sector[1]) {
            //Can't draw this sector.
            return;
        }
        n_textures = 2;
        
    } else {
        texture_sector[0] = s_ptr;
        
    }
    
    for(unsigned char t = 0; t < n_textures; t++) {
    
        bool draw_sector_0 = true;
        if(!texture_sector[0]) draw_sector_0 = false;
        else if(texture_sector[0]->is_bottomless_pit) {
            draw_sector_0 = false;
        }
        
        if(n_textures == 2 && !draw_sector_0 && t == 0) {
            //Allows fading into the void.
            continue;
        }
        
        if(!texture_sector[t] || texture_sector[t]->is_bottomless_pit) {
            continue;
        }
        
        size_t n_vertexes = s_ptr->triangles.size() * 3;
        ALLEGRO_VERTEX* av = new ALLEGRO_VERTEX[n_vertexes];
        
        sector_texture_t* texture_info_to_use =
            &texture_sector[t]->texture_info;
            
        //Texture transformations.
        ALLEGRO_TRANSFORM tra;
        al_build_transform(
            &tra,
            -texture_info_to_use->translation.x,
            -texture_info_to_use->translation.y,
            1.0f / texture_info_to_use->scale.x,
            1.0f / texture_info_to_use->scale.y,
            -texture_info_to_use->rot
        );
        
        for(size_t v = 0; v < n_vertexes; v++) {
        
            const triangle* t_ptr = &s_ptr->triangles[floor(v / 3.0)];
            vertex* v_ptr = t_ptr->points[v % 3];
            float vx = v_ptr->x;
            float vy = v_ptr->y;
            
            float alpha_mult = 1;
            float brightness_mult = texture_sector[t]->brightness / 255.0;
            
            if(t == 1) {
                if(!draw_sector_0) {
                    alpha_mult = 0;
                    for(
                        size_t e = 0; e < texture_sector[1]->edges.size(); e++
                    ) {
                        if(
                            texture_sector[1]->edges[e]->vertexes[0] == v_ptr ||
                            texture_sector[1]->edges[e]->vertexes[1] == v_ptr
                        ) {
                            alpha_mult = 1;
                        }
                    }
                } else {
                    for(
                        size_t e = 0; e < texture_sector[0]->edges.size(); e++
                    ) {
                        if(
                            texture_sector[0]->edges[e]->vertexes[0] == v_ptr ||
                            texture_sector[0]->edges[e]->vertexes[1] == v_ptr
                        ) {
                            alpha_mult = 0;
                        }
                    }
                }
            }
            
            av[v].x = vx - where.x;
            av[v].y = vy - where.y;
            if(texture_sector[t]) al_transform_coordinates(&tra, &vx, &vy);
            av[v].u = vx;
            av[v].v = vy;
            av[v].z = 0;
            av[v].color =
                al_map_rgba_f(
                    texture_sector[t]->texture_info.tint.r * brightness_mult,
                    texture_sector[t]->texture_info.tint.g * brightness_mult,
                    texture_sector[t]->texture_info.tint.b * brightness_mult,
                    texture_sector[t]->texture_info.tint.a * alpha_mult *
                    opacity
                );
        }
        
        for(size_t v = 0; v < n_vertexes; v++) {
            av[v].x *= scale;
            av[v].y *= scale;
        }
        
        ALLEGRO_BITMAP* tex =
            texture_sector[t] ?
            texture_sector[t]->texture_info.bitmap :
            texture_sector[t == 0 ? 1 : 0]->texture_info.bitmap;
            
        al_draw_prim(
            av, nullptr, tex,
            0, (int) n_vertexes, ALLEGRO_PRIM_TRIANGLE_LIST
        );
        
        delete[] av;
    }
}


/**
 * @brief Draws a status effect's bitmap.
 *
 * @param m Mob that has this status effect.
 * @param effects List of bitmap effects to use.
 */
void draw_status_effect_bmp(const mob* m, bitmap_effect_t &effects) {
    float status_bmp_scale;
    ALLEGRO_BITMAP* status_bmp = m->get_status_bitmap(&status_bmp_scale);
    
    if(!status_bmp) return;
    
    draw_bitmap(
        status_bmp,
        m->pos,
        point(m->radius * 2 * status_bmp_scale, -1)
    );
}


/**
 * @brief Draws string tokens.
 *
 * @param tokens Vector of tokens to draw.
 * @param text_font Text font.
 * @param control_font Font for control bind icons.
 * @param controls_condensed Whether control binds should be condensed.
 * @param where Top-left coordinates to draw at.
 * @param flags Allegro text flags.
 * @param max_size Maximum width and height of the whole thing.
 * @param scale Scale each token by this amount.
 */
void draw_string_tokens(
    const vector<string_token> &tokens, const ALLEGRO_FONT* const text_font,
    const ALLEGRO_FONT* const control_font, bool controls_condensed,
    const point &where, int flags, const point &max_size,
    const point &scale
) {
    unsigned int total_width = 0;
    float x_scale = 1.0f;
    for(size_t t = 0; t < tokens.size(); t++) {
        total_width += tokens[t].width;
    }
    if(total_width > max_size.x) {
        x_scale = max_size.x / total_width;
    }
    float y_scale = 1.0f;
    unsigned int line_height = al_get_font_line_height(text_font);
    if(line_height > max_size.y) {
        y_scale = max_size.y / line_height;
    }
    
    float start_x = where.x;
    if(has_flag(flags, ALLEGRO_ALIGN_CENTER)) {
        start_x -= (total_width * x_scale) / 2.0f;
    } else if(has_flag(flags, ALLEGRO_ALIGN_RIGHT)) {
        start_x -= total_width * x_scale;
    }
    
    float caret = start_x;
    for(size_t t = 0; t < tokens.size(); t++) {
        float token_final_width = tokens[t].width * x_scale;
        switch(tokens[t].type) {
        case STRING_TOKEN_CHAR: {
            draw_text(
                tokens[t].content, text_font, point(caret, where.y),
                point(LARGE_FLOAT, LARGE_FLOAT), COLOR_WHITE,
                ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP,
                TEXT_SETTING_FLAG_CANT_GROW,
                point(x_scale * scale.x, y_scale * scale.y)
            );
            break;
        }
        case STRING_TOKEN_CONTROL_BIND: {
            draw_player_input_icon(
                control_font,
                game.controls.find_bind(tokens[t].content).input,
                controls_condensed,
                point(
                    caret + token_final_width / 2.0f,
                    where.y + max_size.y / 2.0f
                ),
                point(token_final_width * scale.x, max_size.y * scale.y)
            );
            break;
        }
        default: {
            break;
        }
        }
        caret += token_final_width;
    }
}


/**
 * @brief Returns information about how a control bind icon should be drawn.
 *
 * @param i Info on the player input. If invalid, a "NONE" icon will be used.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param shape The shape is returned here.
 * @param bitmap_sprite If it's one of the icons in the control bind
 * icon spritesheet, the index of the sprite is returned here.
 * @param text The text to be written inside is returned here, or an
 * empty string is returned if there's nothing to write.
 */
void get_player_input_icon_info(
    const player_input &i, bool condensed,
    PLAYER_INPUT_ICON_SHAPE* shape,
    PLAYER_INPUT_ICON_SPRITE* bitmap_sprite,
    string* text
) {
    *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
    *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_LMB;
    *text = "(NONE)";
    
    if(i.type == INPUT_TYPE_NONE) return;
    
    //Figure out if it's one of those that has a bitmap icon.
    //If so, just return that.
    if(i.type == INPUT_TYPE_MOUSE_BUTTON) {
        if(i.button_nr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_LMB;
            return;
        } else if(i.button_nr == 2) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_RMB;
            return;
        } else if(i.button_nr == 3) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_MMB;
            return;
        }
    } else if(i.type == INPUT_TYPE_MOUSE_WHEEL_UP) {
        *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
        *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_MWU;
        return;
    } else if(i.type == INPUT_TYPE_MOUSE_WHEEL_DOWN) {
        *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
        *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_MWD;
        return;
    } else if(i.type == INPUT_TYPE_KEYBOARD_KEY) {
        if(i.button_nr == ALLEGRO_KEY_UP) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_UP;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_LEFT) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_LEFT;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_DOWN) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_DOWN;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_RIGHT) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_RIGHT;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_BACKSPACE) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_BACKSPACE;
            return;
        } else if(
            condensed &&
            (
                i.button_nr == ALLEGRO_KEY_LSHIFT ||
                i.button_nr == ALLEGRO_KEY_RSHIFT
            )
        ) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_SHIFT;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_TAB) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_TAB;
            return;
        } else if(i.button_nr == ALLEGRO_KEY_ENTER) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_ENTER;
            return;
        }
    } else if(i.type == INPUT_TYPE_CONTROLLER_AXIS_NEG && condensed) {
        if(i.axis_nr == 0) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_STICK_LEFT;
            return;
        } else if(i.axis_nr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_STICK_UP;
            return;
        }
    } else if(i.type == INPUT_TYPE_CONTROLLER_AXIS_POS && condensed) {
        if(i.axis_nr == 0) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_STICK_RIGHT;
            return;
        } else if(i.axis_nr == 1) {
            *shape = PLAYER_INPUT_ICON_SHAPE_BITMAP;
            *bitmap_sprite = PLAYER_INPUT_ICON_SPRITE_STICK_DOWN;
            return;
        }
    }
    
    //Otherwise, use an actual shape and some text inside.
    switch(i.type) {
    case INPUT_TYPE_KEYBOARD_KEY: {
        *shape = PLAYER_INPUT_ICON_SHAPE_RECTANGLE;
        *text = get_key_name(i.button_nr, condensed);
        break;
        
    } case INPUT_TYPE_CONTROLLER_AXIS_NEG:
    case INPUT_TYPE_CONTROLLER_AXIS_POS: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text =
                "Pad " + i2s(i.device_nr + 1) +
                " stick " + i2s(i.stick_nr + 1);
            if(
                i.axis_nr == 0 &&
                i.type == INPUT_TYPE_CONTROLLER_AXIS_NEG
            ) {
                *text += " left";
            } else if(
                i.axis_nr == 0 &&
                i.type == INPUT_TYPE_CONTROLLER_AXIS_POS
            ) {
                *text += " right";
            } else if(
                i.axis_nr == 1 &&
                i.type == INPUT_TYPE_CONTROLLER_AXIS_NEG
            ) {
                *text += " up";
            } else if(
                i.axis_nr == 1 &&
                i.type == INPUT_TYPE_CONTROLLER_AXIS_POS
            ) {
                *text += " down";
            } else {
                *text +=
                    " axis " + i2s(i.axis_nr) +
                    (
                        i.type == INPUT_TYPE_CONTROLLER_AXIS_NEG ?
                        "-" :
                        "+"
                    );
            }
            
        } else {
            *text = "Stick " + i2s(i.stick_nr);
        }
        break;
        
    } case INPUT_TYPE_CONTROLLER_BUTTON: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text =
                "Pad " + i2s(i.device_nr + 1) +
                " button " + i2s(i.button_nr + 1);
        } else {
            *text = i2s(i.button_nr + 1);
        }
        break;
        
    } case INPUT_TYPE_MOUSE_BUTTON: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text = "Mouse button " + i2s(i.button_nr);
        } else {
            *text = "M" + i2s(i.button_nr);
        }
        break;
        
    } case INPUT_TYPE_MOUSE_WHEEL_LEFT: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text = "Mouse wheel left";
        } else {
            *text = "MWL";
        }
        break;
        
    } case INPUT_TYPE_MOUSE_WHEEL_RIGHT: {
        *shape = PLAYER_INPUT_ICON_SHAPE_ROUNDED;
        if(!condensed) {
            *text = "Mouse wheel right";
        } else {
            *text = "MWR";
        }
        break;
        
    } default: {
        break;
        
    }
    }
}


/**
 * @brief Returns the width of a control bind icon, for drawing purposes.
 *
 * @param font Font to use for the name, if necessary.
 * @param i Info on the player input. If invalid, a "NONE" icon will be used.
 * @param condensed If true, only the icon's fundamental information is
 * presented. If false, disambiguation information is included too.
 * For instance, keyboard keys that come in pairs specify whether they are
 * the left or right key, controller inputs specify what controller number
 * it is, etc.
 * @param max_bitmap_height If bitmap icons need to be condensed vertically
 * to fit a certain space, then their width will be affected too.
 * Specify the maximum height here. Use 0 to indicate no maximum height.
 * @return The width.
 */
float get_player_input_icon_width(
    const ALLEGRO_FONT* font, const player_input &i, bool condensed,
    float max_bitmap_height
) {
    PLAYER_INPUT_ICON_SHAPE shape;
    PLAYER_INPUT_ICON_SPRITE bitmap_sprite;
    string text;
    get_player_input_icon_info(
        i, condensed,
        &shape, &bitmap_sprite, &text
    );
    
    if(shape == PLAYER_INPUT_ICON_SHAPE_BITMAP) {
        //All icons are square, and in a row, so the spritesheet height works.
        int bmp_height =
            al_get_bitmap_height(game.sys_assets.bmp_player_input_icons);
        if(max_bitmap_height == 0.0f || bmp_height < max_bitmap_height) {
            return bmp_height;
        } else {
            return max_bitmap_height;
        }
    } else {
        return
            al_get_text_width(font, text.c_str()) +
            CONTROL_BIND_ICON::PADDING * 2;
    }
}
