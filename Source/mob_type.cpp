/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob type class and mob type-related functions.
 */

#include "const.h"
#include "functions.h"
#include "enemy_type.h"
#include "leader_type.h"
#include "mob_type.h"
#include "onion_type.h"
#include "pellet_type.h"
#include "pikmin_type.h"
#include "treasure_type.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a mob type.
 */
mob_type::mob_type() {
    size = move_speed = rotation_speed = 0;
    always_active = false;
    max_health = 0;
    max_carriers = 0;
    weight = 0;
    sight_radius = near_radius = 0;
    rotation_speed = DEF_ROTATION_SPEED;
    big_damage_interval = 0;
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
    
        vector<pair<size_t, string> > anim_conversions;
        
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
        
        data_node anim_file = data_node(folder + "/" + types[t] + "/Animations.txt");
        mt->anims = load_animation_set(&anim_file);
        
        mt->events = load_script(mt, file.get_child_by_name("script"));
        
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
            
            new_anim_conversion(PIKMIN_ANIM_IDLE, "idle");
            new_anim_conversion(PIKMIN_ANIM_WALK, "walk");
            new_anim_conversion(PIKMIN_ANIM_THROWN, "thrown");
            new_anim_conversion(PIKMIN_ANIM_ATTACK, "attack");
            new_anim_conversion(PIKMIN_ANIM_GRAB, "grab");
            new_anim_conversion(PIKMIN_ANIM_BURROWED, "burrowed");
            
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
            
            new_anim_conversion(LEADER_ANIM_IDLE, "idle");
            new_anim_conversion(LEADER_ANIM_WALK, "walk");
            new_anim_conversion(LEADER_ANIM_PLUCK, "pluck");
            new_anim_conversion(LEADER_ANIM_GET_UP, "get_up");
            new_anim_conversion(LEADER_ANIM_DISMISS, "dismiss");
            new_anim_conversion(LEADER_ANIM_THROW, "thrown");
            new_anim_conversion(LEADER_ANIM_WHISTLING, "whistling");
            new_anim_conversion(LEADER_ANIM_LIE, "lie");
            
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
        
        mt->anims.create_conversions(anim_conversions);
        
    }
    
}
