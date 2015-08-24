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
 * Fills class members from a data file.
 */
void mob_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
}



/* ----------------------------------------------------------------------------
 * Loads all mob types.
 */
void load_mob_types(bool load_resources) {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        load_mob_types(mob_categories.get_folder(c), c, load_resources);
    }
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
        mt = mob_categories.create_mob_type(category);
        
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
            mt->anims = load_animation_pool(&anim_file);
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
        
        mt->load_from_file(&file, load_resources, &anim_conversions);
        
        mob_categories.save_mob_type(category, mt);
        
        if(load_resources) {
            mt->anims.create_conversions(anim_conversions);
        }
        
    }
    
}
