/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob class and mob-related functions.
 */

#ifndef MOB_INCLUDED
#define MOB_INCLUDED

#include <map>
#include <vector>

#include <allegro5/allegro.h>

#include "animation.h"
#include "mob_script.h"
#include "pikmin_type.h"
#include "sector.h"

using namespace std;

struct party_spot_info;

class mob_type;
class mob;

/* ----------------------------------------------------------------------------
 * Information on a mob's party.
 * This includes a list of its members,
 * and the location and info of the spots in the
 * circle, when the members are following the mob.
 */
struct party_info {
    vector<mob*> members;
    party_spot_info* party_spots;
    float party_center_x, party_center_y;
    
    party_info(party_spot_info* ps, const float center_x, const float center_y) {
        party_spots = ps;
        party_center_x = center_x;
        party_center_y = center_y;
    }
};



/* ----------------------------------------------------------------------------
 * Structure with information on how
 * the mob should be carried.
 */
struct carrier_info_struct {
    unsigned int max_carriers;
    bool carry_to_ship;            // If true, this is carried to the ship. Otherwise, it's carried to an Onion.
    vector<float> carrier_spots_x; // These are the relative coordinates of each spot. They avoid calculating several sines and cosines over and over.
    vector<float> carrier_spots_y;
    
    float current_carrying_strength; // This is to avoid going through the vector only to find out the total strength.
    size_t current_n_carriers;       // Likewise, this is to avoid going through the vector only to find out the number. Note that this is the number of spaces reserved. A Pikmin could be on its way to its spot, not necessarily there already.
    vector<mob*> carrier_spots;      // Pikmin carrying, and their spots.
    pikmin_type* decided_type;       // Current Onion type it's being taken to.
    
    carrier_info_struct(mob* m, const unsigned int max_carriers, const bool carry_to_ship);
    ~carrier_info_struct();
};



/* ----------------------------------------------------------------------------
 * A mob, short for "mobile object" or "map object",
 * or whatever tickles your fancy, is any instance of
 * an object in the game world. It can move, follow a point,
 * has health, and can be a variety of different sub-types,
 * like leader, Pikmin, enemy, Onion, etc.
 */
class mob {
private:
    void tick_animation();
    void tick_brain();
    void tick_misc_logic();
    void tick_physics();
    void tick_script();
    
    void get_final_target(float* x, float* y);
    
public:
    mob(const float x, const float y, mob_type* type, const float angle, const string &vars);
    virtual ~mob(); // Needed so that typeid works.
    
    mob_type* type;
    
    animation_instance anim;
    
    // Flags.
    bool to_delete; // If true, this mob should be deleted.
    bool reached_destination;
    
    // Actual moving and other physics.
    float x, y, z;                    // Coordinates. Z is height, the higher the value, the higher in the sky.
    float speed_x, speed_y, speed_z;  // Physics only. Don't touch.
    float home_x, home_y;             // Starting coordinates; what the mob calls "home".
    float move_speed_mult;            // Multiply the normal moving speed by this.
    float acceleration;               // Speed multiplies by this much each second.
    float speed;                      // Speed moving forward.
    float angle;                      // 0: Right. PI*0.5: Up. PI: Left. PI*1.5: Down.
    float intended_angle;             // Angle the mob wants to be facing.
    float ground_z;                   // Z of the highest ground it's on.
    float lighting;                   // How light the mob is. Depends on the sector(s) it's on.
    bool affected_by_gravity;         // Is the mob currently affected by gravity? Wollywogs stop in mid-air when jumping, for instance.
    void face(const float new_angle); // Makes the mob face an angle, but it'll turn at its own pace.
    virtual float get_base_speed();   // Returns the normal speed of this mob. Subclasses are meant to override this.
    
    // Target things.
    float target_x, target_y;           // When movement is automatic, this is the spot the mob is trying to go to.
    float* target_z;                    // When following a target in teleport mode, also change the z accordingly.
    float* target_rel_x, *target_rel_y; // Follow these coordinates.
    unsigned char target_code;          // Code ID for a special target, like home. Used for scripting.
    bool go_to_target;                  // If true, it'll try to go to the target spot on its own.
    bool gtt_instant;                   // If true, teleport instantly.
    bool gtt_free_move;                 // If true, the mob can move in a direction it's not facing.
    float target_distance;              // Distance from the target in which the mob is considered as being there.
    bool can_move;                      // If true, this mob can control its movement.
    void set_target(const float target_x, const float target_y, float* target_rel_x, float* target_rel_y, const bool instant, float* target_z = NULL, bool free_move = false, float target_distance = 3);
    void remove_target(const bool stop);
    
    // Party things.
    mob* following_party;      // The current mob is following this mob's party.
    bool was_thrown;           // Is the mob airborne because it was thrown?
    float unwhistlable_period; // During this period, the mob cannot be whistled into a party.
    float untouchable_period;  // During this period, the mob cannot be touched into a party.
    party_info* party;         // Info on the party this mob is a leader of.
    
    // Other properties.
    float health;           // Current health.
    float invuln_period;    // During this period, the mob cannot be attacked.
    float knockdown_period; // During this period, the mob cannot move, as it's been knocked down.
    unsigned char team;     // Mob's team (who it can damage), use MOB_TEAM_*.
    
    // Script.
    mob_fsm fsm;                      // Finitate-state machine.
    bool spawned;                     // Has the mob actually "spawned" yet?
    mob* focused_opponent;            // The opponent (prey) it has focus on.
    bool focused_opponent_near;       // Was the opponent near the mob in the previous frame?
    vector<mob*> focused_by;          // Mobs that are focusing on it.
    float timer;                      // The timer.
    float timer_interval;             // The timer's interval.
    map<string, string> vars;         // Variables.
    mob_event* script_wait_event;     // What event is the script waiting on?
    size_t script_wait_action;        // Number of the action the script returns to after the wait is over.
    
    bool dead;                     // Is the mob dead?
    unsigned char state;           // Current state.
    float time_in_state;           // For how long as the mob been in this state?
    vector<int> chomp_hitboxes;    // List of hitboxes that will chomp Pikmin.
    vector<mob*> chomping_pikmin;  // Mobs being chomped.
    void set_state(const unsigned char new_state);
    
    // Carrying.
    carrier_info_struct* carrier_info; // Structure holding information on how this mob should be carried. If NULL, it cannot be carried.
    
    void tick();
};



void add_to_party(mob* party_leader, mob* new_member);
void attack(mob* m1, mob* m2, const bool m1_is_pikmin, const float damage, const float angle, const float knockback, const float new_invuln_period, const float new_knockdown_period);
void create_mob(mob* m);
void delete_mob(mob* m);
void focus_mob(mob* m1, mob* m2, const bool is_near, const bool call_event);
hitbox_instance* get_closest_hitbox(const float x, const float y, mob* m);
hitbox_instance* get_hitbox_instance(mob* m, const size_t nr);
void make_uncarriable(mob* m);
void remove_from_party(mob* member);
bool should_attack(mob* m1, mob* m2);
void unfocus_mob(mob* m1, mob* m2, const bool call_event);



const float GRAVITY_ADDER = -1300.0f; // Accelerate the Z speed of mobs affected by gravity by this amount per second.

enum MOB_TEAMS {
    MOB_TEAM_NONE,       // Can hurt/target anyone and be hurt/targeted by anyone, on any team.
    MOB_TEAM_PLAYER_1,
    MOB_TEAM_PLAYER_2,
    MOB_TEAM_PLAYER_3,
    MOB_TEAM_PLAYER_4,
    MOB_TEAM_ENEMIES_1,
    MOB_TEAM_ENEMIES_2,
    MOB_TEAM_DECORATION, // Cannot be hurt or targeted by anything.
};

// Special targets to chase. Used by the scripts.
enum MOB_TARGETS {
    MOB_TARGET_NONE,
    MOB_TARGET_HOME,
    MOB_TARGET_POINT,
};

enum MOB_STATES {
    MOB_STATE_IDLE,
    MOB_STATE_BEING_CARRIED,
    MOB_STATE_BEING_DELIVERED, // Into an Onion.
    PIKMIN_STATE_IN_GROUP,
    PIKMIN_STATE_BURIED,
    PIKMIN_STATE_MOVING_TO_CARRY_SPOT,
    PIKMIN_STATE_CARRYING,
    PIKMIN_STATE_ATTACKING_MOB,
    PIKMIN_STATE_CELEBRATING,
};

#endif // ifndef MOB_INCLUDED
