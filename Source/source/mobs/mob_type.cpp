/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob type class and mob type-related functions.
 */

#include "../const.h"
#include "../functions.h"
#include "enemy_type.h"
#include "leader_type.h"
#include "mob_fsm.h"
#include "mob_type.h"
#include "onion_type.h"
#include "pellet_type.h"
#include "pikmin_type.h"
#include "treasure_type.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a non-specific mob type.
 */
mob_type::mob_type(size_t category_id) :
    category(mob_categories.get(category_id)),
    radius(0),
    height(0),
    move_speed(0),
    always_active(false),
    max_health(0),
    health_regen(0),
    max_carriers(0),
    weight(0),
    pushes(false),
    pushable(false),
    rotation_speed(DEF_ROTATION_SPEED),
    big_damage_interval(0),
    create_mob(nullptr),
    main_color(al_map_rgb(128, 128, 128)),
    territory_radius(0),
    first_state_nr(INVALID),
    is_obstacle(false),
    show_health(true),
    casts_shadow(true) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a mob type.
 */
mob_type::~mob_type() {
    states.clear();
    anims.destroy();
}


/* ----------------------------------------------------------------------------
 * Fills class members from a data file.
 */
void mob_type::load_from_file(
    data_node* file, const bool load_resources,
    vector<pair<size_t, string> >* anim_conversions
) {
    if(load_from_file_func) {
        load_from_file_func(file, load_resources, anim_conversions);
    }
}


/* ----------------------------------------------------------------------------
 * Loads all mob types.
 */
void load_mob_types(bool load_resources) {
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
        mob_category* category = mob_categories.get(c);
        load_mob_types(category, load_resources);
    }
    
    //Load the special mob types.
    for(auto mt = spec_mob_types.begin(); mt != spec_mob_types.end(); ++mt) {
        string folder = SPECIAL_MOBS_FOLDER_PATH + "/" + mt->first;
        data_node file = data_node(folder + "/Data.txt");
        if(!file.file_was_opened) return;
        
        load_mob_type_from_file(mt->second, file, load_resources, folder);
    }
}


/* ----------------------------------------------------------------------------
 * Loads the mob types from a category's folder.
 * category:       Pointer to the mob category.
 * load_resources: False if you don't need the images and sounds,
 *   so it loads faster.
 */
void load_mob_types(
    mob_category* category, bool load_resources
) {
    if(category->folder.empty()) return;
    bool folder_found;
    vector<string> types =
        folder_to_vector(category->folder, true, &folder_found);
    if(!folder_found) {
        log_error("Folder \"" + category->folder + "\" not found!");
    }
    
    for(size_t t = 0; t < types.size(); ++t) {
    
        data_node file =
            data_node(category->folder + "/" + types[t] + "/Data.txt");
        if(!file.file_was_opened) continue;
        
        mob_type* mt;
        mt = category->create_type();
        
        load_mob_type_from_file(
            mt, file, load_resources, category->folder + "/" + types[t]
        );
        
        category->register_type(mt);
        
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
    
    reader_setter rs(&file);
    rs.set("name",                mt->name);
    rs.set("always_active",       mt->always_active);
    rs.set("big_damage_interval", mt->big_damage_interval);
    rs.set("main_color",          mt->main_color);
    rs.set("max_carriers",        mt->max_carriers);
    rs.set("max_health",          mt->max_health);
    rs.set("health_regen",        mt->health_regen);
    rs.set("move_speed",          mt->move_speed);
    rs.set("rotation_speed",      mt->rotation_speed);
    rs.set("territory_radius",    mt->territory_radius);
    rs.set("radius",              mt->radius);
    rs.set("height",              mt->height);
    rs.set("weight",              mt->weight);
    rs.set("pushes",              mt->pushes);
    rs.set("pushable",            mt->pushable);
    rs.set("show_health",         mt->show_health);
    rs.set("casts_shadow",        mt->casts_shadow);
    rs.set("is_obstacle",         mt->is_obstacle);
    
    mt->rotation_speed = deg_to_rad(mt->rotation_speed);
    
    data_node* reaches_node = file.get_child_by_name("reaches");
    size_t n_reaches = reaches_node->get_nr_of_children();
    for(size_t r = 0; r < n_reaches; ++r) {
    
        mob_type::reach_struct new_reach;
        new_reach.name = reaches_node->get_child(r)->name;
        vector<string> r_strings = split(reaches_node->get_child(r)->value);
        
        if(r_strings.size() != 2 && r_strings.size() != 4) {
            log_error(
                "Reach \"" + new_reach.name +
                "\" isn't made up of 2 or 4 words!",
                reaches_node->get_child(r)
            );
            continue;
        }
        
        new_reach.radius_1 = s2f(r_strings[0]);
        new_reach.angle_1 = deg_to_rad(s2f(r_strings[1]));
        if(r_strings.size() == 4) {
            new_reach.radius_2 = s2f(r_strings[2]);
            new_reach.angle_2 = deg_to_rad(s2f(r_strings[3]));
        }
        mt->reaches.push_back(new_reach);
    }
    
    if(load_resources) {
        data_node anim_file = data_node(folder + "/Animations.txt");
        mt->anims = load_animation_database_from_file(&anim_file);
        mt->anims.fix_body_part_pointers();
        
        data_node script_file = data_node(folder + "/Script.txt");
        size_t old_n_states = mt->states.size();
        load_script(mt, script_file.get_child_by_name("script"), &mt->states);
        if(mt->states.size() > old_n_states) {
            string first_state_name =
                script_file.get_child_by_name("first_state")->value;
            for(size_t s = 0; s < mt->states.size(); ++s) {
                if(mt->states[s]->name == first_state_name) {
                    mt->first_state_nr = s;
                    break;
                }
            }
        }
    }
    
    mt->load_from_file(&file, load_resources, &anim_conversions);
    
    if(load_resources) {
        mt->anims.create_conversions(anim_conversions);
    }
}


/* ----------------------------------------------------------------------------
 * Adds carrying-related states to the FSM.
 */
void mob_type::add_carrying_states() {

    easy_fsm_creator efc;
    
    efc.new_state("carriable_waiting", ENEMY_EXTRA_STATE_CARRIABLE_WAITING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRY_KEEP_GOING); {
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("carriable_moving");
        }
    }
    
    efc.new_state("carriable_moving", ENEMY_EXTRA_STATE_CARRIABLE_MOVING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_WAIT_UP); {
            efc.change_state("carriable_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("carriable_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_begin_move);
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
    }
    
    efc.new_state("being_delivered", ENEMY_EXTRA_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::start_being_delivered);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(gen_mob_fsm::handle_delivery);
        }
    }
    
    
    vector<mob_state*> new_states = efc.finish();
    fix_states(new_states, "");
    
    states.insert(states.end(), new_states.begin(), new_states.end());
    
}
