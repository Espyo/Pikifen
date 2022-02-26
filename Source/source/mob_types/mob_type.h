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
#include "../const.h"
#include "../misc_structs.h"
#include "../mob_categories/mob_category.h"
#include "../mob_script.h"
#include "../status.h"
#include "../utils/data_file.h"
#include "../mobs/mob_enums.h"


using std::size_t;
using std::string;
using std::vector;


typedef vector<std::pair<size_t, string> > anim_conversion_vector;

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
        HOLD_ROTATION_METHODS hold_rotation_method;
        
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
        LIMB_DRAW_METHODS limb_draw_method;
        
        child_struct() :
            parent_holds(false),
            hold_offset_dist(0.0f),
            hold_offset_angle(0.0f),
            hold_rotation_method(HOLD_ROTATION_METHOD_NEVER),
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
    
    //Info on a widget to present in the area editor,
    //to better help users set the properties of a mob instance.
    struct area_editor_prop_struct {
        //Name of the widget.
        string name;
        //Variable it sets.
        string var;
        //What type of content this var has.
        AEMP_TYPES type;
        //Default value.
        string def_value;
        //Minimum value.
        float min_value;
        //Maximum value.
        float max_value;
        //If it's a list, this list the values.
        vector<string> value_list;
        //Tooltip to show on the widget, if any.
        string tooltip;
        area_editor_prop_struct();
    };
    
    //Info on how vulnerable the object is to a certain source.
    struct vulnerability_struct {
        //Multiply damage taken by this.
        float damage_mult;
        //When affected by the source, receive this status effect.
        status_type* status_to_apply;
        //If "status_to_apply" overrides any status effect that'd be received.
        bool status_overrides;
        
        vulnerability_struct();
    };
    
    
    //Technical things.
    //Its full name.
    string name;
    //Name of the folder its data is on.
    string folder_name;
    //Mob category.
    mob_category* category;
    
    //Visual things.
    //Database with all its animation data.
    animation_database anims;
    //A color that represents this mob.
    ALLEGRO_COLOR main_color;
    //Show its health?
    bool show_health;
    //Does it cast a shadow?
    bool casts_shadow;
    
    //Space-related things.
    //Radius of the space it occupies. Can be overridden on a per-mob basis.
    float radius;
    //Height. Can be overridden on a per-mob basis.
    float height;
    //Moves these many units per second.
    float move_speed;
    //Acceleration. This is in units per second per second.
    float acceleration;
    //Rotates these many radians per second.
    float rotation_speed;
    //True if it can move in any direction, as opposed to just forward.
    bool can_free_move;
    //Pushes other mobs (only those that can be pushed).
    bool pushes;
    //Can be pushed by other mobs.
    bool pushable;
    //If true, the push is via hitbox, as opposed to the mob's radius?
    bool pushes_with_hitboxes;
    //Radius for terrain collision. Negative = use regular radius property.
    float terrain_radius;
    //Can you walk on top of this mob?
    bool walkable;
    //Rectangular dimensions, if it's meant to use them instead of a radius.
    point rectangular_dim;
    
    //Behavior things.
    //Maximum health value.
    float max_health;
    //Regenerates these many health points per second.
    float health_regen;
    //How far its territory reaches from the home point.
    float territory_radius;
    //Information on all of its "reaches".
    vector<reach_struct> reaches;
    //Information on everything it can spawn.
    vector<spawn_struct> spawns;
    //Information on its children mobs.
    vector<child_struct> children;
    //How many Pikmin can carry it, at most.
    size_t max_carriers;
    //Pikmin strength needed to carry it.
    float weight;
    //After it takes this much damage, it sends an "itch" event to the FSM.
    float itch_damage;
    //Only send an "itch" event after these many seconds have passed.
    float itch_time;
    //Does this mob have a group of other mobs following it (e.g. leader)?
    bool has_group;
    //Other mobs decide if they can/want to hurt it by this target type.
    MOB_TARGET_TYPES target_type;
    //What types of targets this mob can hunt down.
    uint16_t huntable_targets;
    //What types of targets this mob can hurt.
    uint16_t hurtable_targets;
    //Its initial team.
    MOB_TEAMS starting_team;
    
    //Script things.
    //Actions to run on spawn.
    vector<mob_action_call*> init_actions;
    //The states, events and actions. Basically, the FSM.
    vector<mob_state*> states;
    //Number of the state a mob starts at.
    size_t first_state_nr;
    //Name of the state to go to on death.
    string death_state_name;
    //Number of the state to go to on death.
    size_t death_state_nr;
    //States that ignore the death event.
    vector<string> states_ignoring_death;
    //States that ignore the spray event.
    vector<string> states_ignoring_spray;
    //Widgets to show on the area editor, to help parametrize each mob.
    vector<area_editor_prop_struct> area_editor_props;
    
    //Misc.
    //Tips to show in the area editor about this mob type, if any.
    string area_editor_tips;
    //Can the player choose to place one of these in the area editor?
    bool appears_in_area_editor;
    //If true, carrier Pikmin will be considered blocked if it's in the way.
    bool can_block_paths;
    //All damage received is multiplied by this much.
    float default_vulnerability;
    //For every hazard, multiply damage taken by this much.
    map<hazard*, vulnerability_struct> hazard_vulnerabilities;
    //What sort of spike damage it causes, if any.
    spike_damage_type* spike_damage;
    //For every type of spike damage, multiply damage taken by this much.
    map<spike_damage_type*, vulnerability_struct> spike_damage_vulnerabilities;
    //For every type of status, multiply damage taken by this much.
    map<status_type*, vulnerability_struct> status_vulnerabilities;
    
    //Caches and such.
    //How far its hitboxes or radius can reach from the center.
    float max_span;
    
    //General functions.
    mob_type(MOB_CATEGORIES category_id);
    virtual ~mob_type();
    virtual void load_properties(data_node* file);
    virtual void load_resources(data_node* file);
    virtual anim_conversion_vector get_anim_conversions() const;
    virtual void unload_resources();
    void add_carrying_states();
    
    //Custom behavior callbacks.
    void(*draw_mob_callback)(mob* m);
};


/* ----------------------------------------------------------------------------
 * A mob type that has animation groups.
 * These have a series of "base" animations, like idling, dying, etc.,
 * but can also have several looks for these same base animations.
 * So in practice, it can have an idling blue animation, idling yellow,
 * dying red, etc. Because this would otherwise be a nightmare to organize,
 * this base class comes with some helpful functions and members.
 * A "group" is the "look" mentioned before, so "red", "yellow", "blue", etc.
 * The mob type should load a property somewhere that lists what suffixes to
 * use for each group when loading animation names from the animation database.
 */
class mob_type_with_anim_groups {
public:
    vector<string> animation_group_suffixes;
    anim_conversion_vector get_anim_conversions_with_groups(
        const anim_conversion_vector &v, const size_t base_anim_total
    ) const;
    
    virtual ~mob_type_with_anim_groups() = default;
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
