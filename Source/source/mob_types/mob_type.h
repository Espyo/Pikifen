/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob type class and mob type-related functions.
 */

#ifndef MOB_TYPE_INCLUDED
#define MOB_TYPE_INCLUDED

#include <functional>
#include <map>

#include <allegro5/allegro.h>

#include "../animation.h"
#include "../data_file.h"
#include "../misc_structs.h"
#include "../mob_categories/mob_category.h"
#include "../mob_script.h"

using namespace std;

enum ENEMY_EXTRA_STATES {
    ENEMY_EXTRA_STATE_CARRIABLE_WAITING,
    ENEMY_EXTRA_STATE_CARRIABLE_MOVING,
    ENEMY_EXTRA_STATE_CARRIABLE_STUCK,
    ENEMY_EXTRA_STATE_BEING_DELIVERED,
};

typedef vector<pair<size_t, string> > anim_conversion_vector;

const size_t ANIM_IDLING = 0;


/* ----------------------------------------------------------------------------
 * A mob type.
 * There are specific types, like Pikmin,
 * leader, etc., but these are used
 * to create more generic mob types,
 * like some teleporter pad, or a door.
 */
class mob_type {
public:
    struct reach_struct {
        string name;
        float radius_1;
        float angle_1;
        float radius_2;
        float angle_2;
        
        reach_struct() :
            radius_1(-1), angle_1(-1),
            radius_2(-1), angle_2(-1) { }
    };
    
    struct spawn_struct {
        string name;
        string mob_type_name;
        bool relative;
        point coords_xy;
        float coords_z;
        float angle;
        string vars;
        bool link_object_to_spawn;
        bool link_spawn_to_object;
        float momentum;
        
        spawn_struct() :
            relative(true),
            coords_z(0),
            angle(0),
            link_object_to_spawn(false),
            link_spawn_to_object(false),
            momentum(0) {}
    };
    
    struct child_struct {
        string name;
        string spawn_name;
        bool parent_holds;
        string hold_body_part;
        float hold_offset_dist;
        float hold_offset_angle;
        
        bool handle_damage;
        bool relay_damage;
        bool handle_events;
        bool relay_events;
        bool handle_statuses;
        bool relay_statuses;
        
        string limb_anim_name;
        float limb_thickness;
        string limb_parent_body_part;
        float limb_parent_offset;
        string limb_child_body_part;
        float limb_child_offset;
        unsigned char limb_draw_method;
        
        child_struct() :
            parent_holds(false),
            hold_offset_dist(0.0f),
            hold_offset_angle(0.0f),
            handle_damage(false),
            relay_damage(false),
            handle_events(false),
            relay_events(false),
            handle_statuses(false),
            relay_statuses(false),
            limb_thickness(32.0f),
            limb_parent_offset(0),
            limb_child_offset(0) {}
    };
    
    //Technical things.
    string name;
    string folder_name;
    mob_category* category;
    
    //Visual things.
    animation_database anims;
    ALLEGRO_COLOR main_color;
    bool show_health;
    bool casts_shadow;
    
    //Space-related things.
    float radius;
    float height;
    float move_speed;
    float rotation_speed;
    bool can_free_move;
    //If true, this mob is always active, even if it's off-camera.
    bool always_active;
    bool pushes; //Blocks passage of other mobs.
    bool pushable; //Can be pushed by other mobs.
    bool pushes_with_hitboxes; //Is the push via hitbox, or mob radius?
    bool walkable;
    point rectangular_dim;
    
    //Behavior things.
    float max_health;
    float health_regen; //Health points per second.
    float territory_radius;
    vector<reach_struct> reaches;
    vector<spawn_struct> spawns;
    vector<child_struct> children;
    size_t max_carriers;
    float weight;          //Pikmin strength needed to carry it.
    float itch_damage;
    float itch_time;
    
    //Script things.
    vector<mob_action*> init_actions; //Actions to run on spawn.
    vector<mob_state*> states;        //The states, events and actions.
    size_t first_state_nr;            //Number of the state a mob starts at.
    string death_state_name;          //Name of the state to go to on death.
    size_t death_state_nr;            //Number of the state to go to on death.
    vector<string> states_ignoring_death; //States that ignore the death event.
    vector<string> states_ignoring_spray; //States that ignore the spray event.
    
    //Misc.
    bool appears_in_area_editor;
    bool is_obstacle;
    bool is_projectile;
    bool blocks_carrier_pikmin;
    bool projectiles_can_damage;
    float default_vulnerability;
    map<hazard*, float> hazard_vulnerabilities;
    spike_damage_type* spike_damage;
    map<spike_damage_type*, float> spike_damage_vulnerabilities;
    
    //General functions.
    mob_type(size_t category_id);
    virtual ~mob_type();
    virtual void load_parameters(data_node* file);
    virtual void load_resources(data_node* file);
    virtual anim_conversion_vector get_anim_conversions();
    virtual void unload_resources();
    void add_carrying_states();
};


/* A mob type that has animation groups.
 * These have a series of "base" animations, like idling, dying, etc.,
 * but can also have several looks for these same base animations.
 * So in practice, it can have an idling blue animation, idling yellow,
 * dying red, etc. Because this would otherwise be a nightmare to organize,
 * this base class comes with some helpful functions and members.
 * A "group" is the "look" mentioned before, so "red", "yellow", "blue", etc.
 * The mob type should load a parameter somewhere that lists what suffixes to
 * use for each group when loading animation names from the animation database.
 */
class mob_type_with_anim_groups {
public:
    vector<string> animation_group_suffixes;
    anim_conversion_vector get_anim_conversions_with_groups(
        const anim_conversion_vector &v, const size_t base_anim_total
    );
};


void create_special_mob_types();
void load_mob_types(mob_category* category, bool load_resources);
void load_mob_types(bool load_resources);
void load_mob_type_from_file(
    mob_type* mt, data_node &file, const bool load_resources,
    const string &folder
);
void unload_mob_types(const bool unload_resources);
void unload_mob_types(mob_category* category, const bool unload_resources);

#endif //ifndef MOB_TYPE_INCLUDED
