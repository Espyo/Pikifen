/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob type class and mob type-related functions.
 */

#pragma once

#include <functional>
#include <map>

#include <allegro5/allegro.h>

#include "../animation.h"
#include "../audio.h"
#include "../const.h"
#include "../content.h"
#include "../misc_structs.h"
#include "../mob_categories/mob_category.h"
#include "../mob_script.h"
#include "../status.h"
#include "../libs/data_file.h"
#include "../mobs/mob_enums.h"
#include "../spike_damage.h"
#include "../utils/general_utils.h"


using std::size_t;
using std::string;
using std::vector;


typedef vector<std::pair<size_t, string> > anim_conversion_vector;

namespace MOB_TYPE {
extern const size_t ANIM_IDLING;
extern const float DEF_ACCELERATION;
extern const float DEF_ROTATION_SPEED;
}


/**
 * @brief A mob type.
 *
 * There are specific types, like Pikmin,
 * leader, etc., but these are used
 * to create more generic mob types,
 * like some teleporter pad, or a door.
 */
class mob_type : public content {

public:

    //--- Misc. declarations ---
    
    /**
     * @brief Info about a mob's reach.
     */
    struct reach_t {
    
        //--- Members ---
        
        //Name of this reach.
        string name;
        
        //Radius of possibility 1.
        float radius_1 = -1.0f;
        
        //Angle of possibility 1.
        float angle_1 = -1.0f;
        
        //Radius of possibility 2.
        float radius_2 = -1.0f;
        
        //Angle of possibility 2.
        float angle_2 = -1.0f;
        
    };
    
    /**
     * @brief Info about how a mob spawns another.
     */
    struct spawn_t {
    
        //--- Members ---
        
        //Name of this spawn information block.
        string name;
        
        //Name of the mob type to spawn.
        string mob_type_name;
        
        //Spawn in coordinates relative to the spawner?
        bool relative = true;
        
        //Coordenates to spawn on.
        point coords_xy;
        
        //Z coordinate to spawn on.
        float coords_z = 0.0f;
        
        //Angle of the spawned object. Could be relative or absolute.
        float angle = 0.0f;
        
        //Script vars to give the spawned object.
        string vars;
        
        //Should the spawner link to the spawned?
        bool link_object_to_spawn = false;
        
        //Should the spawned link to the spawner?
        bool link_spawn_to_object = false;
        
        //Momentum to apply in a random direction upon spawn, if any.
        float momentum = 0.0f;
        
    };
    
    /**
     * @brief Info about how a mob can be a child of another.
     */
    struct child_t {
    
        //--- Members ---
        
        //Name of this child information block.
        string name;
        
        //Name of the spawn information block to use.
        string spawn_name;
        
        //Does the parent mob hold the child mob?
        bool parent_holds = false;
        
        //If the parent holds, this is the name of the body part that holds.
        string hold_body_part;
        
        //If the parent holds, this is how far from the body part center.
        float hold_offset_dist = 0.0f;
        
        //If the parent holds, this is how far from the body part Z.
        float hold_offset_vert_dist = 0.0f;
        
        //If the parent holds, this is in what direction from the body part.
        float hold_offset_angle = 0.0f;
        
        //Method by which the parent should hold the child.
        HOLD_ROTATION_METHOD hold_rotation_method = HOLD_ROTATION_METHOD_NEVER;
        
        //Should the child handle damage?
        bool handle_damage = false;
        
        //Should the child relay damage to the parent?
        bool relay_damage = false;
        
        //Should the child handle script events?
        bool handle_events = false;
        
        //Should the child relay script events to the parent?
        bool relay_events = false;
        
        //Should the child handle status effects?
        bool handle_statuses = false;
        
        //Should the child relay status effects to the parent?
        bool relay_statuses = false;
        
        //Name of the limb animation between parent and child.
        string limb_anim_name;
        
        //Thickness of the limb.
        float limb_thickness = 32.0f;
        
        //Body part of the parent to link the limb to.
        string limb_parent_body_part;
        
        //Offset from the parent body part to link the limb at.
        float limb_parent_offset = 0.0f;
        
        //Body part of the child to link the limb to.
        string limb_child_body_part;
        
        //Offset from the child body part to link the limb at.
        float limb_child_offset = 0.0f;
        
        //Method by which the limb should be drawn.
        LIMB_DRAW_METHOD limb_draw_method = LIMB_DRAW_METHOD_ABOVE_BOTH;
        
    };
    
    /**
     * @brief Info on a widget to present in the area editor,
     * to better help users set the properties of a mob instance.
     */
    struct area_editor_prop_t {
    
        //--- Members ---
        
        //Name of the widget.
        string name;
        
        //Variable it sets.
        string var;
        
        //What type of content this var has.
        AEMP_TYPE type = AEMP_TYPE_TEXT;
        
        //Default value.
        string def_value;
        
        //Minimum value.
        float min_value = -LARGE_FLOAT;
        
        //Maximum value.
        float max_value = LARGE_FLOAT;
        
        //If it's a list, this list the values.
        vector<string> value_list;
        
        //Tooltip to show on the widget, if any.
        string tooltip;
        
    };
    
    /**
     * @brief Info on how vulnerable the object is to a certain source.
     */
    struct vulnerability_t {
    
        //--- Members ---
        
        //Multiply damage taken by this.
        float damage_mult = 1.0f;
        
        //When affected by the source, receive this status effect.
        status_type* status_to_apply = nullptr;
        
        //If "status_to_apply" overrides any status effect that'd be received.
        bool status_overrides = true;
        
    };
    
    /**
     * @brief Info on a sound effect this mob can emit.
     */
    struct sound_t {
    
        //--- Members ---
        
        //Its name.
        string name;
        
        //The loaded sample.
        ALLEGRO_SAMPLE* sample = nullptr;
        
        //Type of sound.
        SOUND_TYPE type = SOUND_TYPE_WORLD_POS;
        
        //Configuration.
        sound_source_config_t config;
        
    };
    
    
    //--- Members ---
    
    //- Basic information -
    
    //Mob category.
    mob_category* category = nullptr;
    
    //Custom category name. Used in editors.
    string custom_category_name;
    
    //- Visuals -
    
    //Database with all its animation data.
    animation_database* anim_db = nullptr;
    
    //A color that represents this mob.
    ALLEGRO_COLOR main_color = al_map_rgb(128, 128, 128);
    
    //Show its health?
    bool show_health = true;
    
    //Does it cast a shadow?
    bool casts_shadow = true;
    
    //How much light does it cast in a blackout? <0 to use the mob's radius.
    float blackout_radius = -1.0f;
    
    //List of sounds it can play.
    vector<sound_t> sounds;
    
    //- Movement -
    
    //Moves these many units per second.
    float move_speed = 0.0f;
    
    //Acceleration. This is in units per second per second.
    float acceleration = MOB_TYPE::DEF_ACCELERATION;
    
    //Rotates these many radians per second.
    float rotation_speed = MOB_TYPE::DEF_ROTATION_SPEED;
    
    //True if it can move in any direction, as opposed to just forward.
    bool can_free_move = false;
    
    //- Physical space -
    
    //Radius of the space it occupies. Can be overridden on a per-mob basis.
    float radius = 0.0f;
    
    //Height. Can be overridden on a per-mob basis.
    float height = 0.0f;
    
    //Rectangular dimensions, if it's meant to use them instead of a radius.
    point rectangular_dim;
    
    //Pikmin strength needed to carry it.
    float weight = 0.0f;
    
    //How many Pikmin can carry it, at most.
    size_t max_carriers = 0;
    
    //Pushes other mobs (only those that can be pushed).
    bool pushes = false;
    
    //Can be pushed by other mobs.
    bool pushable = false;
    
    //If true, the push is soft and allows squeezing through with persistance.
    bool pushes_softly = false;
    
    //If true, the push is via hitbox, as opposed to the mob's radius.
    bool pushes_with_hitboxes = false;
    
    //Radius for terrain collision. Negative = use regular radius property.
    float terrain_radius = - 1.0f;
    
    //Can you walk on top of this mob?
    bool walkable = false;
    
    //Can this mob walk on top of other mobs?
    bool can_walk_on_others = false;
    
    //If true, carrier Pikmin will be considered blocked if it's in the way.
    bool can_block_paths = false;
    
    //Override the carrying spots with these coordinates, if not-empty.
    vector<point> custom_carry_spots;
    
    //- General behavior -
    
    //Maximum health. Can be overridden on a per-mob basis.
    float max_health = 100.0f;
    
    //Regenerates these many health points per second.
    float health_regen = 0.0f;
    
    //How far its territory reaches from the home point.
    float territory_radius = 0.0f;
    
    //Information on all of its "reaches".
    vector<reach_t> reaches;
    
    //After it takes this much damage, it sends an "itch" event to the FSM.
    float itch_damage = 0.0f;
    
    //Only send an "itch" event after these many seconds have passed.
    float itch_time = 0.0f;
    
    //Other mobs decide if they can/want to hurt it by this target type.
    MOB_TARGET_FLAG target_type = MOB_TARGET_FLAG_NONE;
    
    //What types of targets this mob can hunt down.
    bitmask_16_t huntable_targets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY;
        
    //What types of targets this mob can hurt.
    bitmask_16_t hurtable_targets =
        MOB_TARGET_FLAG_PLAYER |
        MOB_TARGET_FLAG_ENEMY |
        MOB_TARGET_FLAG_FRAGILE;
        
    //Its initial team.
    MOB_TEAM starting_team = MOB_TEAM_NONE;
    
    //Logic for when it's inactive. Use INACTIVE_LOGIC_FLAG.
    bitmask_8_t inactive_logic = 0;
    
    //Custom behavior callbacks.
    void(*draw_mob_callback)(mob* m) = nullptr;
    
    //- Script -
    
    //Actions to run on spawn.
    vector<mob_action_call*> init_actions;
    
    //The states, events and actions. Basically, the FSM.
    vector<mob_state*> states;
    
    //Index of the state a mob starts at.
    size_t first_state_idx = INVALID;
    
    //Name of the state to go to on death.
    string death_state_name;
    
    //Index of the state to go to on death.
    size_t death_state_idx = INVALID;
    
    //States that ignore the death event.
    vector<string> states_ignoring_death;
    
    //States that ignore the spray event.
    vector<string> states_ignoring_spray;
    
    //States that ignore the hazard events.
    vector<string> states_ignoring_hazard;
    
    //Interactions with other objects
    
    //Information on everything it can spawn.
    vector<spawn_t> spawns;
    
    //Information on its children mobs.
    vector<child_t> children;
    
    //Does this mob have a group of other mobs following it (e.g. leader)?
    bool has_group = false;
    
    //- Vulnerabilities -
    
    //All damage received is multiplied by this much.
    float default_vulnerability = 1.0f;
    
    //For every hazard, multiply damage taken by this much.
    map<hazard*, vulnerability_t> hazard_vulnerabilities;
    
    //What sort of spike damage it causes, if any.
    spike_damage_type* spike_damage = nullptr;
    
    //For every type of spike damage, multiply damage taken by this much.
    map<spike_damage_type*, vulnerability_t> spike_damage_vulnerabilities;
    
    //For every type of status, multiply damage taken by this much.
    map<status_type*, vulnerability_t> status_vulnerabilities;
    
    //- Editor info -
    
    //Tips to show in the area editor about this mob type, if any.
    string area_editor_tips;
    
    //Widgets to show on the area editor, to help parametrize each mob.
    vector<area_editor_prop_t> area_editor_props;
    
    //Can the player choose to place one of these in the area editor?
    bool appears_in_area_editor = true;
    
    //Should it have links going out of it?
    bool area_editor_recommend_links_from = false;
    
    //Should it have links going into it?
    bool area_editor_recommend_links_to = false;
    
    //- Caches -
    
    //How far its radius or hitboxes reach from the center.
    //Cache for performance.
    float physical_span = 0.0f;
    
    
    //--- Function declarations ---
    
    explicit mob_type(MOB_CATEGORY category_id);
    virtual ~mob_type();
    void load_from_data_node(
        data_node* node, CONTENT_LOAD_LEVEL level, const string &folder
    );
    virtual void load_cat_properties(data_node* file);
    virtual void load_cat_resources(data_node* file);
    virtual anim_conversion_vector get_anim_conversions() const;
    virtual void unload_resources();
    void add_carrying_states();
    
};


/**
 * @brief A mob type that has animation groups.
 *
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

    //--- Members ---
    
    //Suffixes used for each animation group.
    vector<string> animation_group_suffixes;
    
    
    //--- Function declarations ---
    
    anim_conversion_vector get_anim_conversions_with_groups(
        const anim_conversion_vector &v, size_t base_anim_total
    ) const;
    virtual ~mob_type_with_anim_groups() = default;
    
};


void create_special_mob_types();
