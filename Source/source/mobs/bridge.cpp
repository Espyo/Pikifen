/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge class and bridge related functions.
 */

#include <algorithm>

#include "bridge.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../utils/string_utils.h"


namespace BRIDGE {
//Width of the bridge's main floor, i.e., sans rails.
const float FLOOR_WIDTH = 192.0f;
//How far apart bridge steps are
const float STEP_HEIGHT = 10;
}


/* ----------------------------------------------------------------------------
 * Creates a bridge mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Bridge type this mob belongs to.
 * angle:
 *   Starting angle.
 */
bridge::bridge(const point &pos, bridge_type* type, const float angle) :
    mob(pos, type, angle),
    total_chunks_needed(10),
    total_length(192.0f),
    delta_z(0.0f),
    start_pos(pos),
    start_z(0.0f),
    chunks(0),
    prev_chunk_z_offset(LARGE_FLOAT),
    prev_chunk_combo(0),
    bri_type(type) {
    
    team = MOB_TEAM_OBSTACLE;
    
    start_z = z;
}


/* ----------------------------------------------------------------------------
 * Checks the bridge's health, and updates the chunks if necessary.
 * Returns true if new chunks were created.
 */
bool bridge::check_health() {
    //Figure out how many chunks should exist based on the bridge's completion.
    float completion = 1.0f - clamp(health / max_health, 0.0f, 1.0f);
    size_t expected_chunks = floor(total_chunks_needed * completion);
    
    if(chunks >= expected_chunks) {
        //Nothing to do here.
        return false;
    }
    
    mob_category* custom_category =
        game.mob_categories.get(MOB_CATEGORY_CUSTOM);
    mob_type* bridge_component_type =
        custom_category->get_type("Bridge component");
    float chunk_width = total_length / total_chunks_needed;
    vector<mob*> new_mobs;
    
    //Start creating all the necessary chunks.
    while(chunks < expected_chunks) {
        float x_offset = chunk_width / 2.0 + chunk_width * chunks;
        
        //Find the Z that this chunk should be at.
        float z_offset;
        if(chunks == total_chunks_needed - 1) {
            z_offset = delta_z;
        } else {
            size_t steps_needed =
                ceil(fabs(delta_z) / BRIDGE::STEP_HEIGHT) + 1;
            float cur_completion =
                chunks / (float) total_chunks_needed;
            size_t step_idx =
                cur_completion * steps_needed;
            z_offset =
                step_idx * BRIDGE::STEP_HEIGHT * sign(delta_z);
        }
        
        if(z_offset == prev_chunk_z_offset) {
        
            //Just expand the existing components!
            float old_component_width = chunk_width * prev_chunk_combo;
            prev_chunk_combo++;
            float new_component_width = chunk_width * prev_chunk_combo;
            point offset =
                rotate_point(
                    point(
                        (new_component_width - old_component_width) / 2.0f,
                        0.0f
                    ),
                    angle
                );
                
            for(size_t m = 0; m < prev_chunk_components.size(); ++m) {
                prev_chunk_components[m]->pos +=
                    offset;
                prev_chunk_components[m]->set_rectangular_dim(
                    point(
                        new_component_width,
                        prev_chunk_components[m]->rectangular_dim.y
                    )
                );
            }
            
        } else {
        
            //Create new components. First, the floor component.
            point offset(x_offset, 0.0f);
            offset = rotate_point(offset, angle);
            mob* floor_component =
                create_mob(
                    custom_category,
                    start_pos + offset,
                    bridge_component_type,
                    angle,
                    "side=center; offset=" + f2s(x_offset - chunk_width / 2.0f)
                );
            if(!floor_component->center_sector) {
                //Maybe the bridge component was forced to be created over
                //the void or something? Abort!
                break;
            }
            floor_component->z = start_z + z_offset;
            floor_component->set_rectangular_dim(
                point(chunk_width, BRIDGE::FLOOR_WIDTH)
            );
            new_mobs.push_back(floor_component);
            
            //Then, the left rail component.
            offset.x = x_offset;
            offset.y =
                -BRIDGE::FLOOR_WIDTH / 2.0f - bri_type->rail_width / 2.0f;
            offset = rotate_point(offset, angle);
            mob* left_rail_component =
                create_mob(
                    custom_category,
                    start_pos + offset,
                    bridge_component_type,
                    angle,
                    "side=left; offset=" + f2s(x_offset - chunk_width / 2.0f)
                );
            if(!left_rail_component->center_sector) {
                //Maybe the bridge component was forced to be created over
                //the void or something? Abort!
                break;
            }
            left_rail_component->z = start_z + z_offset;
            left_rail_component->set_rectangular_dim(
                point(
                    floor_component->rectangular_dim.x,
                    bri_type->rail_width
                )
            );
            left_rail_component->height += GEOMETRY::STEP_HEIGHT * 2.0 + 1.0f;
            new_mobs.push_back(left_rail_component);
            
            //Finally, the right rail component.
            offset.x = x_offset;
            offset.y = BRIDGE::FLOOR_WIDTH / 2.0f + bri_type->rail_width / 2.0f;
            offset = rotate_point(offset, angle);
            mob* right_rail_component =
                create_mob(
                    custom_category,
                    start_pos + offset,
                    bridge_component_type,
                    angle,
                    "side=right; offset=" + f2s(x_offset - chunk_width / 2.0f)
                );
            if(!right_rail_component->center_sector) {
                //Maybe the bridge component was forced to be created over
                //the void or something? Abort!
                break;
            }
            right_rail_component->z = start_z + z_offset;
            right_rail_component->set_rectangular_dim(
                left_rail_component->rectangular_dim
            );
            right_rail_component->height = left_rail_component->height;
            new_mobs.push_back(right_rail_component);
            
            prev_chunk_z_offset = z_offset;
            prev_chunk_components.clear();
            prev_chunk_components.push_back(floor_component);
            prev_chunk_components.push_back(left_rail_component);
            prev_chunk_components.push_back(right_rail_component);
            prev_chunk_combo = 1;
            
        }
        
        chunks++;
        
    }
    
    //Finish setting up the new component mobs.
    for(size_t m = 0; m < new_mobs.size(); ++m) {
        mob* m_ptr = new_mobs[m];
        enable_flag(m_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
        m_ptr->links.push_back(this);
    }
    
    //Move the bridge object proper to the farthest point of the bridge.
    point offset(chunk_width * chunks - 32.0f, 0);
    offset = rotate_point(offset, angle);
    pos = start_pos + offset;
    z = start_z + prev_chunk_components[0]->z;
    ground_sector = prev_chunk_components[0]->ground_sector;

    return true;
}


/* ----------------------------------------------------------------------------
 * Draws a bridge component, making sure to follow the right dimensions.
 * m:
 *   Bridge component mob.
 */
void bridge::draw_component(mob* m) {
    if(m->links.empty() || !m->links[0]) return;
    
    bridge* bri_ptr = (bridge*) m->links[0];
    string side = m->vars["side"];
    ALLEGRO_BITMAP* texture =
        side == "left" ?
        bri_ptr->bri_type->bmp_left_rail_texture :
        side == "right" ?
        bri_ptr->bri_type->bmp_right_rail_texture :
        bri_ptr->bri_type->bmp_main_texture;
    int texture_h = al_get_bitmap_height(texture);
    int texture_v0 = texture_h / 2.0f - m->rectangular_dim.y / 2.0f;
    float texture_offset = s2f(m->vars["offset"]);
    
    ALLEGRO_TRANSFORM angle_transform;
    al_identity_transform(&angle_transform);
    al_rotate_transform(&angle_transform, m->angle);
    
    ALLEGRO_VERTEX vertexes[4];
    for(size_t v = 0; v < 4; ++v) {
        vertexes[v].color = COLOR_WHITE;
        vertexes[v].z = 0.0f;
    }
    
    vertexes[0].x = -m->rectangular_dim.x / 2.0f;
    vertexes[0].y = -m->rectangular_dim.y / 2.0f;
    vertexes[0].u = texture_offset;
    vertexes[0].v = texture_v0;
    
    vertexes[1].x = m->rectangular_dim.x / 2.0f;
    vertexes[1].y = -m->rectangular_dim.y / 2.0f;
    vertexes[1].u = vertexes[0].u + m->rectangular_dim.x;
    vertexes[1].v = texture_v0;
    
    vertexes[2].x = m->rectangular_dim.x / 2.0f;
    vertexes[2].y = m->rectangular_dim.y / 2.0f;
    vertexes[2].u = vertexes[1].u;
    vertexes[2].v = texture_v0 + m->rectangular_dim.y;
    
    vertexes[3].x = -m->rectangular_dim.x / 2.0f;
    vertexes[3].y = m->rectangular_dim.y / 2.0f;
    vertexes[3].u = vertexes[0].u;
    vertexes[3].v = vertexes[2].v;
    
    for(size_t v = 0; v < 4; ++v) {
        al_transform_coordinates(
            &angle_transform, &vertexes[v].x, &vertexes[v].y
        );
        vertexes[v].x += m->pos.x;
        vertexes[v].y += m->pos.y;
    }
    
    al_draw_prim(vertexes, NULL, texture, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
}


/* ----------------------------------------------------------------------------
 * Returns the starting point of the bridge.
 */
point bridge::get_start_point() {
    return start_pos;
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 * svr:
 *   Script var reader to use.
 */
void bridge::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    svr.get("chunks", total_chunks_needed);
}


/* ----------------------------------------------------------------------------
 * Sets up the bridge with the data surrounding it, like its linked destination
 * object.
 */
void bridge::setup() {
    if(!links.empty() && links[0]) {
        total_length = dist(pos, links[0]->pos).to_float();
        face(get_angle(pos, links[0]->pos), NULL, true);
        delta_z = links[0]->z - z;
        total_chunks_needed =
            std::max(
                total_chunks_needed,
                (size_t) (ceil(fabs(delta_z) / GEOMETRY::STEP_HEIGHT) + 1)
            );
    }
    
    check_health();
}
