/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
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
mob_type::mob_type() :
    radius(0),
    height(0),
    move_speed(0),
    always_active(false),
    max_health(0),
    max_carriers(0),
    weight(0),
    sight_radius(0),
    near_radius(0),
    rotation_speed(DEF_ROTATION_SPEED),
    big_damage_interval(0),
    create_mob(nullptr),
    main_color(al_map_rgb(128, 128, 128)),
    territory_radius(0),
    near_angle(0),
    chomp_max_victims(0),
    first_state_nr(0) {
    
}


/* ----------------------------------------------------------------------------
 * Loads all mob types.
 */
void load_mob_types(bool load_resources) {
    load_mob_types(PIKMIN_FOLDER,    MOB_CATEGORY_PIKMIN,    load_resources);
    load_mob_types(ONIONS_FOLDER,    MOB_CATEGORY_ONIONS,    load_resources);
    load_mob_types(LEADERS_FOLDER,   MOB_CATEGORY_LEADERS,   load_resources);
    load_mob_types(ENEMIES_FOLDER,   MOB_CATEGORY_ENEMIES,   load_resources);
    load_mob_types(TREASURES_FOLDER, MOB_CATEGORY_TREASURES, load_resources);
    load_mob_types(PELLETS_FOLDER,   MOB_CATEGORY_PELLETS,   load_resources);
    load_mob_types(SHIPS_FOLDER,     MOB_CATEGORY_SHIPS,     load_resources);
    load_mob_types(GATES_FOLDER,     MOB_CATEGORY_GATES,     load_resources);
}


/* ----------------------------------------------------------------------------
 * Loads the mob types from a folder.
 * folder: Name of the folder on the hard drive.
 * category: Use MOB_CATEGORY_* for this.
 * load_resources: False if you don't need the images and sounds, so it loads faster.
 */
void load_mob_types(const string &folder, const unsigned char category, bool load_resources) {
    bool folder_found;
    vector<string> types = folder_to_vector(folder, true, &folder_found);
    if(!folder_found) {
        error_log("Folder \"" + folder + "\" not found!");
    }
    
    for(size_t t = 0; t < types.size(); t++) {
    
        vector<pair<size_t, string> > anim_conversions;
        
        data_node file = data_node(folder + "/" + types[t] + "/Data.txt");
        if(!file.file_was_opened) return;
        
        mob_type* mt;
        if(category == MOB_CATEGORY_PIKMIN) {
            mt = new pikmin_type();
        } else if(category == MOB_CATEGORY_ONIONS) {
            mt = new onion_type();
        } else if(category == MOB_CATEGORY_LEADERS) {
            mt = new leader_type();
        } else if(category == MOB_CATEGORY_GATES) {
            mt = new gate_type();
        } else if(category == MOB_CATEGORY_ENEMIES) {
            mt = new enemy_type();
        } else if(category == MOB_CATEGORY_PELLETS) {
            mt = new pellet_type();
        } else if(category == MOB_CATEGORY_SHIPS) {
            mt = new ship_type();
        } else {
            mt = new mob_type();
        }
        
        mt->name = file.get_child_by_name("name")->value;
        mt->always_active = s2b(file.get_child_by_name("always_active")->value);
        mt->big_damage_interval = s2f(file.get_child_by_name("big_damage_interval")->value);
        mt->chomp_max_victims = s2i(file.get_child_by_name("chomp_max_victims")->get_value_or_default("100"));
        mt->main_color = s2c(file.get_child_by_name("main_color")->value);
        mt->max_carriers = s2i(file.get_child_by_name("max_carriers")->value);
        mt->max_health = s2i(file.get_child_by_name("max_health")->value);
        mt->move_speed = s2f(file.get_child_by_name("move_speed")->value);
        mt->near_radius = s2f(file.get_child_by_name("near_radius")->value);
        mt->near_angle = s2f(file.get_child_by_name("near_angle")->value);
        mt->rotation_speed = s2f(file.get_child_by_name("rotation_speed")->get_value_or_default(f2s(DEF_ROTATION_SPEED)));
        mt->sight_radius = s2f(file.get_child_by_name("sight_radius")->value);
        mt->territory_radius = s2f(file.get_child_by_name("territory_radius")->value);
        mt->radius = s2f(file.get_child_by_name("radius")->value);
        mt->height = s2f(file.get_child_by_name("height")->value);
        mt->weight = s2f(file.get_child_by_name("weight")->value);
        
        if(load_resources) {
            data_node anim_file = data_node(folder + "/" + types[t] + "/Animations.txt");
            mt->anims = load_animation_set(&anim_file);
            mt->anims.fix_hitbox_pointers();
            
            if(mt->states.empty()) {
                mt->states = load_script(mt, file.get_child_by_name("script"));
                if(mt->states.size()) {
                    string first_state_name = file.get_child_by_name("first_state")->value;
                    for(size_t s = 0; s < mt->states.size(); s++) {
                        if(mt->states[s]->name == first_state_name) {
                            mt->first_state_nr = s;
                            break;
                        }
                    }
                } else {
                    mt->first_state_nr = string::npos;
                }
            }
        }
        
        if(category == MOB_CATEGORY_PIKMIN) {
            pikmin_type* pt = (pikmin_type*) mt;
            pt->attack_power = s2f(file.get_child_by_name("attack_power")->value);
            pt->attack_interval = s2f(file.get_child_by_name("attack_interval")->get_value_or_default("0.8"));
            pt->throw_height_mult = s2f(file.get_child_by_name("throw_height_mult")->get_value_or_default("1"));
            pt->can_carry_bomb_rocks = s2b(file.get_child_by_name("can_carry_bomb_rocks")->value);
            pt->can_dig = s2b(file.get_child_by_name("can_dig")->value);
            pt->can_latch = s2b(file.get_child_by_name("can_latch")->value);
            pt->can_swim = s2b(file.get_child_by_name("can_swim")->value);
            pt->carry_speed = s2f(file.get_child_by_name("carry_speed")->value);
            pt->carry_strength = s2f(file.get_child_by_name("carry_strength")->value);
            pt->has_onion = s2b(file.get_child_by_name("has_onion")->value);
            
            if(load_resources) {
                pt->bmp_top[0] = bitmaps.get(file.get_child_by_name("top_leaf")->value,   &file);
                pt->bmp_top[1] = bitmaps.get(file.get_child_by_name("top_bud")->value,    &file);
                pt->bmp_top[2] = bitmaps.get(file.get_child_by_name("top_flower")->value, &file);
                pt->bmp_icon[0] = bitmaps.get(file.get_child_by_name("icon_leaf")->value,   &file);
                pt->bmp_icon[1] = bitmaps.get(file.get_child_by_name("icon_bud")->value,    &file);
                pt->bmp_icon[2] = bitmaps.get(file.get_child_by_name("icon_flower")->value, &file);
            }
            new_anim_conversion(PIKMIN_ANIM_IDLE,     "idle");
            new_anim_conversion(PIKMIN_ANIM_WALK,     "walk");
            new_anim_conversion(PIKMIN_ANIM_THROWN,   "thrown");
            new_anim_conversion(PIKMIN_ANIM_ATTACK,   "attack");
            new_anim_conversion(PIKMIN_ANIM_GRAB,     "grab");
            new_anim_conversion(PIKMIN_ANIM_BURROWED, "burrowed");
            new_anim_conversion(PIKMIN_ANIM_LYING,    "lying");
            new_anim_conversion(PIKMIN_ANIM_GET_UP,   "get_up");
            
            pikmin_types[pt->name] = pt;
            
        } else if(category == MOB_CATEGORY_ONIONS) {
            onion_type* ot = (onion_type*) mt;
            data_node* pik_type_node = file.get_child_by_name("pikmin_type");
            if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
                error_log("Unknown Pikmin type \"" + pik_type_node->value + "\"!", pik_type_node);
                continue;
            }
            ot->pik_type = pikmin_types[pik_type_node->value];
            
            onion_types[ot->name] = ot;
            
        } else if(category == MOB_CATEGORY_LEADERS) {
            leader_type* lt = (leader_type*) mt;
            lt->pluck_delay = s2f(file.get_child_by_name("pluck_delay")->value);
            lt->whistle_range = s2f(file.get_child_by_name("whistle_range")->get_value_or_default(f2s(DEF_WHISTLE_RANGE)));
            lt->punch_strength = s2i(file.get_child_by_name("punch_strength")->value); //TODO default.
            
            if(load_resources) {
                lt->sfx_dismiss = load_sample(file.get_child_by_name("dismiss_sfx")->value, mixer); //TODO don't use load_sample.
                lt->sfx_name_call = load_sample(file.get_child_by_name("name_call_sfx")->value, mixer); //TODO don't use load_sample.
                lt->sfx_whistle = load_sample(file.get_child_by_name("whistle_sfx")->value, mixer); //TODO don't use load_sample.
                lt->bmp_icon = bitmaps.get(file.get_child_by_name("icon")->value, &file);
            }
            
            new_anim_conversion(LEADER_ANIM_IDLE,      "idle");
            new_anim_conversion(LEADER_ANIM_WALK,      "walk");
            new_anim_conversion(LEADER_ANIM_PLUCK,     "pluck");
            new_anim_conversion(LEADER_ANIM_GET_UP,    "get_up");
            new_anim_conversion(LEADER_ANIM_DISMISS,   "dismiss");
            new_anim_conversion(LEADER_ANIM_THROW,     "thrown");
            new_anim_conversion(LEADER_ANIM_WHISTLING, "whistling");
            new_anim_conversion(LEADER_ANIM_LIE,       "lie");
            
            leader_types[lt->name] = lt;
            
        } else if(category == MOB_CATEGORY_ENEMIES) {
            enemy_type* et = (enemy_type*) mt;
            et->drops_corpse = s2b(file.get_child_by_name("drops_corpse")->get_value_or_default("yes"));
            et->is_boss = s2b(file.get_child_by_name("is_boss")->value);
            et->pikmin_seeds = s2i(file.get_child_by_name("pikmin_seeds")->value);
            et->regenerate_speed = s2b(file.get_child_by_name("regenerate_speed")->value);
            et->revive_speed = s2f(file.get_child_by_name("revive_speed")->value);
            et->value = s2f(file.get_child_by_name("value")->value);
            
            enemy_types[et->name] = et;
            
        } else if(category == MOB_CATEGORY_TREASURES) {
            treasure_type* tt = (treasure_type*) mt;
            tt->move_speed = 60; //TODO should this be here?
            
            treasure_types[tt->name] = tt;
            
        } else if(category == MOB_CATEGORY_PELLETS) {
            pellet_type* pt = (pellet_type*) mt;
            data_node* pik_type_node = file.get_child_by_name("pikmin_type");
            if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
                error_log("Unknown Pikmin type \"" + pik_type_node->value + "\"!", pik_type_node);
                continue;
            }
            
            pt->pik_type = pikmin_types[pik_type_node->value];
            pt->number = s2i(file.get_child_by_name("number")->value);
            pt->weight = pt->number;
            pt->match_seeds = s2i(file.get_child_by_name("match_seeds")->value);
            pt->non_match_seeds = s2i(file.get_child_by_name("non_match_seeds")->value);
            
            if(load_resources) {
                pt->bmp_number = bitmaps.get(file.get_child_by_name("number_image")->value, &file);
            }
            
            new_anim_conversion(ANIM_IDLE, "idle");
            
            pt->move_speed = 60; //TODO should this be here?
            
            pellet_types[pt->name] = pt;
            
        } else if(category == MOB_CATEGORY_SHIPS) {
            ship_type* st = (ship_type*) mt;
            st->can_heal = file.get_child_by_name("can_heal");
            
            ship_types[st->name] = st;
            
        } else if(category == MOB_CATEGORY_GATES) {
            gate_type* gt = (gate_type*) mt;
            
            new_anim_conversion(GATE_ANIM_IDLE, "idle");
            new_anim_conversion(GATE_ANIM_NOTHING, "nothing");
            
            gate_types[gt->name] = gt;
            
        }
        
        if(load_resources) {
            
            mt->anims.create_conversions(anim_conversions);
        }
        
    }
    
}
