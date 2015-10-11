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
    pushes(false),
    pushable(false),
    sight_radius(0),
    near_radius(0),
    rotation_speed(DEF_ROTATION_SPEED),
    big_damage_interval(0),
    create_mob(nullptr),
    main_color(al_map_rgb(128, 128, 128)),
    territory_radius(0),
    near_angle(0),
    first_state_nr(0),
    show_health(true),
    casts_shadow(true) {
    
}


/* ----------------------------------------------------------------------------
 * Fills class members from a data file.
 */
void mob_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    if(load_from_file_func) load_from_file_func(file, load_resources, anim_conversions);
}



/* ----------------------------------------------------------------------------
 * Loads all mob types.
 */
void load_mob_types(bool load_resources) {
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
        load_mob_types(mob_categories.get_folder(c), c, load_resources);
    }
    
    //Load the special mob types.
    for(auto mt = spec_mob_types.begin(); mt != spec_mob_types.end(); ++mt) {
        string folder = SPECIAL_MOBS_FOLDER + "/" + mt->first;
        data_node file = data_node(folder + "/Data.txt");
        if(!file.file_was_opened) return;
        
        load_mob_type_from_file(mt->second, file, load_resources, folder);
    }
}


/* ----------------------------------------------------------------------------
 * Loads the mob types from a folder.
 * folder: Name of the folder on the hard drive.
 * category: Use MOB_CATEGORY_* for this.
 * load_resources: False if you don't need the images and sounds, so it loads faster.
 */
void load_mob_types(const string &folder, const unsigned char category, bool load_resources) {
    if(folder.empty()) return;
    bool folder_found;
    vector<string> types = folder_to_vector(folder, true, &folder_found);
    if(!folder_found) {
        error_log("Folder \"" + folder + "\" not found!");
    }
    
    for(size_t t = 0; t < types.size(); ++t) {
    
        data_node file = data_node(folder + "/" + types[t] + "/Data.txt");
        if(!file.file_was_opened) return;
        
        mob_type* mt;
        mt = mob_categories.create_mob_type(category);
        
        load_mob_type_from_file(mt, file, load_resources, folder + "/" + types[t]);
        
        mob_categories.save_mob_type(category, mt);
        
    }
    
}

/* ----------------------------------------------------------------------------
 * Loads a mob type's info from a text file.
 */
void load_mob_type_from_file(
    mob_type* mt, data_node &file,
    const bool load_resources, const string &folder
) {

    vector<pair<size_t, string> > anim_conversions;
    
    set_if_exists(file.get_child_by_name("name")->value,                mt->name);
    set_if_exists(file.get_child_by_name("always_active")->value,       mt->always_active);
    set_if_exists(file.get_child_by_name("big_damage_interval")->value, mt->big_damage_interval);
    set_if_exists(file.get_child_by_name("main_color")->value,          mt->main_color);
    set_if_exists(file.get_child_by_name("max_carriers")->value,        mt->max_carriers);
    set_if_exists(file.get_child_by_name("max_health")->value,          mt->max_health);
    set_if_exists(file.get_child_by_name("move_speed")->value,          mt->move_speed);
    set_if_exists(file.get_child_by_name("near_radius")->value,         mt->near_radius);
    set_if_exists(file.get_child_by_name("near_angle")->value,          mt->near_angle);
    set_if_exists(file.get_child_by_name("rotation_speed")->value,      mt->rotation_speed);
    set_if_exists(file.get_child_by_name("sight_radius")->value,        mt->sight_radius);
    set_if_exists(file.get_child_by_name("territory_radius")->value,    mt->territory_radius);
    set_if_exists(file.get_child_by_name("radius")->value,              mt->radius);
    set_if_exists(file.get_child_by_name("height")->value,              mt->height);
    set_if_exists(file.get_child_by_name("weight")->value,              mt->weight);
    set_if_exists(file.get_child_by_name("pushes")->value,              mt->pushes);
    set_if_exists(file.get_child_by_name("pushable")->value,            mt->pushable);
    set_if_exists(file.get_child_by_name("show_health")->value,         mt->show_health);
    set_if_exists(file.get_child_by_name("casts_shadow")->value,        mt->casts_shadow);
    
    if(load_resources) {
        data_node anim_file = data_node(folder + "/Animations.txt");
        mt->anims = load_animation_pool(&anim_file);
        mt->anims.fix_hitbox_pointers();
        
        if(mt->states.empty()) {
            mt->states = load_script(mt, file.get_child_by_name("script"));
            if(mt->states.size()) {
                string first_state_name = file.get_child_by_name("first_state")->value;
                for(size_t s = 0; s < mt->states.size(); ++s) {
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
    
    if(load_resources) {
        mt->anims.create_conversions(anim_conversions);
    }
}