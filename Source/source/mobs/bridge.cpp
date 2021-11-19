/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge class and bridge related functions.
 */

#include "bridge.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../utils/string_utils.h"


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
    total_chunks_needed(5),
    chunks(0),
    bri_type(type) {
    
    //Search neighboring sectors.
    get_sector(pos, NULL, true)->get_neighbor_sectors_conditionally(
    [] (sector * s) -> bool {
        return
        s->type == SECTOR_TYPE_BRIDGE ||
        s->type == SECTOR_TYPE_BRIDGE_RAIL;
    },
    secs
    );
    
    team = MOB_TEAM_OBSTACLE;
}


/* ----------------------------------------------------------------------------
 * Checks the bridge's health, and updates the chunks if necessary.
 */
void bridge::check_health() {
    //Figure out how many chunks should exist based on the bridge's completion.
    float completion = 1 - clamp(health / type->max_health, 0.0f, 1.0f);
    size_t expected_chunks = floor(total_chunks_needed * completion);
    
    if(chunks >= expected_chunks) {
        //Nothing to do here.
        return;
    }
    
    mob_category* custom_category =
        game.mob_categories.get(MOB_CATEGORY_CUSTOM);
    mob_type* bridge_component_type =
        custom_category->get_type("Bridge component");
    vector<mob*> new_mobs;
    
    const float BRIDGE_WIDTH = 192.0f;
    const float BRIDGE_RAIL_WIDTH = 16.0f;
    
    //Start creating all the necessary chunks.
    while(chunks < expected_chunks) {
        //First, the floor.
        point offset(50.0f * chunks, 0.0f);
        offset = rotate_point(offset, angle);
        mob* floor_component =
            create_mob(
                custom_category,
                pos + offset,
                bridge_component_type,
                angle,
                "railing=false; chunk=" + i2s(chunks)
            );
        floor_component->rectangular_dim.x = 50.0f;
        floor_component->rectangular_dim.y = BRIDGE_WIDTH;
        new_mobs.push_back(floor_component);
        
        //Then, the left railing.
        offset.x = 50.0f * chunks;
        offset.y = -BRIDGE_WIDTH / 2.0f - BRIDGE_RAIL_WIDTH / 2.0f;
        offset = rotate_point(offset, angle);
        mob* left_railing_component =
            create_mob(
                custom_category,
                pos + offset,
                bridge_component_type,
                angle,
                "railing=true; chunk=" + i2s(chunks)
            );
        left_railing_component->rectangular_dim.x =
            floor_component->rectangular_dim.x;
        left_railing_component->rectangular_dim.y = BRIDGE_RAIL_WIDTH;
        left_railing_component->height += SECTOR_STEP + 1.0f;
        new_mobs.push_back(left_railing_component);
        
        //Finall, the right railing.
        offset.x = 50.0f * chunks;
        offset.y = BRIDGE_WIDTH / 2.0f + BRIDGE_RAIL_WIDTH / 2.0f;
        offset = rotate_point(offset, angle);
        mob* right_railing_component =
            create_mob(
                custom_category,
                pos + offset,
                bridge_component_type,
                angle,
                "railing=true; chunk=" + i2s(chunks)
            );
        right_railing_component->rectangular_dim =
            left_railing_component->rectangular_dim;
        right_railing_component->height += SECTOR_STEP + 1.0f;
        new_mobs.push_back(right_railing_component);
        
        chunks++;
    }
    
    //Finish setting up the new component mobs.
    for(size_t m = 0; m < new_mobs.size(); ++m) {
        mob* m_ptr = new_mobs[m];
        m_ptr->can_move_in_midair = true;
        m_ptr->links.push_back(this);
    }
}


/* ----------------------------------------------------------------------------
 * Draws a bridge component, making sure to follow the right dimensions.
 * m:
 *   Bridge component mob.
 */
void bridge::draw_component(mob* m) {
    bridge* bri_ptr = (bridge*) m->links[0];
    bool is_railing = m->vars["railing"] == "true";
    ALLEGRO_BITMAP* texture =
        is_railing ?
        bri_ptr->bri_type->bmp_rail_texture :
        bri_ptr->bri_type->bmp_main_texture;
    int texture_h = al_get_bitmap_height(texture);
    int texture_v0 = texture_h / 2.0f - m->rectangular_dim.y / 2.0f;
    size_t chunk_idx = s2i(m->vars["chunk"]);
    
    ALLEGRO_TRANSFORM angle_transform;
    al_identity_transform(&angle_transform);
    al_rotate_transform(&angle_transform, m->angle);
    
    ALLEGRO_VERTEX vertexes[4];
    for(size_t v = 0; v < 4; ++v) {
        vertexes[v].color = al_map_rgb(255, 255, 255);
        vertexes[v].z = 0.0f;
    }
    
    vertexes[0].x = -m->rectangular_dim.x / 2.0f;
    vertexes[0].y = -m->rectangular_dim.y / 2.0f;
    vertexes[0].u = m->rectangular_dim.x * chunk_idx;
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
