#ifndef MOB_INCLUDED
#define MOB_INCLUDED

#include <map>
#include <vector>

#include <allegro5/allegro.h>

#include "animation.h"
#include "mob_event.h"
#include "pikmin_type.h"
#include "sector.h"

using namespace std;

struct party_spot_info;

class mob_type;
class mob;

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

struct carrier_info_struct {
    unsigned int max_carriers;
    bool carry_to_ship;            //If true, this is carried to the ship. Otherwise, it's carried to an Onion.
    vector<float> carrier_spots_x; //These are the relative coordinates of each spot. They avoid calculating several sines and cosines over and over.
    vector<float> carrier_spots_y;
    
    float current_carrying_strength; //This is to avoid going through the vector only to find out the total strength.
    size_t current_n_carriers;       //Likewise, this is to avoid going through the vector only to find out the number. Note that this is the number of spaces reserved. A Pikmin could be on its way to its spot, not necessarily there already.
    vector<mob*> carrier_spots;      //Pikmin carrying, and their spots.
    pikmin_type* decided_type;       //Current Onion type it's being taken to.
    
    carrier_info_struct(mob* m, const unsigned int max_carriers, const bool carry_to_ship);
    ~carrier_info_struct();
};

class mob {
public:
    mob(const float x, const float y, const float z, mob_type* t, sector* sec);
    virtual ~mob(); //Needed so that typeid works.
    
    mob_type* type;
    
    animation_instance anim;
    
    //Flags.
    bool to_delete; //If true, this mob should be deleted.
    bool reached_destination;
    
    //Actual moving and other physics.
    float x, y, z;                   //Coordinates. Z is height, the higher the value, the higher in the sky.
    float speed_x, speed_y, speed_z; //Physics only. Don't touch.
    float home_x, home_y;            //Starting coordinates; what the mob calls "home".
    float move_speed_mult;           //Multiply the normal moving speed by this.
    float acceleration;              //Speed multiplies by this much each second.
    float angle;                     //0: Right. PI*0.5: Up. PI: Left. PI*1.5: Down.
    float intended_angle;            //Angle the mob wants to be facing.
    sector* sec;                     //Sector it's on.
    bool affected_by_gravity;        //Is the mob currently affected by gravity? Wollywogs stop in mid-air when jumping, for instance.
    void face(const float new_angle);      //Makes the mob face an angle, but it'll turn at its own pace.
    virtual float get_base_speed();  //Returns the normal speed of this mob. Subclasses are meant to override this.
    
    //Target things.
    float target_x, target_y;           //When movement is automatic, this is the spot the mob is trying to go to.
    float* target_rel_x, *target_rel_y; //Follow these coordinates.
    unsigned char target_code;          //Code ID for a special target, like home. Used for scripting.
    bool go_to_target;                  //If true, it'll try to go to the target spot on its own.
    bool gtt_instant;                   //If true, teleport instantly.
    void set_target(const float target_x, const float target_y, float* target_rel_x, float* target_rel_y, const bool instant);
    void remove_target(const bool stop);
    
    //Party things.
    mob* following_party;      //The current mob is following this mob's party.
    bool was_thrown;           //Is the mob airborne because it was thrown?
    float unwhistlable_period; //During this period, the mob cannot be whistled into a party.
    float untouchable_period;  //During this period, the mob cannot be touched into a party.
    party_info* party;         //Info on the party this mob is a leader of.
    
    //Other properties.
    float health;           //Current health.
    float invuln_period;    //During this period, the mob cannot be attacked.
    float knockdown_period; //During this period, the mob cannot move, as it's been knocked down.
    unsigned char team;     //Mob's team (who it can damage), use MOB_TEAM_*.
    
    //Script.
    mob* focused_prey;                //The prey it has focus on.
    vector<mob*> focused_by;          //Mobs that are focusing on it.
    bool focused_prey_near;           //Is the focused prey what's considered "near" the mob?
    float timer;                      //The timer.
    float timer_interval;             //The timer's interval.
    map<string, string> vars;         //Variables.
    float script_wait;                //Wait this much time before going on with the script. 0 = Not waiting. -1 = Waiting for the animation.
    mob_event* script_wait_event;     //What event is the script waiting on?
    size_t script_wait_action;        //Number of the action the script returns to after the wait is over.
    unsigned char events_queued[N_MOB_EVENTS]; //Events waiting to be ran. 0: Not waiting. 1: Waiting. 2: Waiting, but only run if nothing else is running.
    
    bool dead;                     //Is the mob dead?
    unsigned char state;           //Current state.
    float time_in_state;           //For how long as the mob been in this state?
    vector<string> chomp_hitboxes; //List of hitboxes that will chomp Pikmin.
    vector<mob*> chomping_pikmin;  //Mobs being chomped.
    void set_state(const unsigned char new_state);
    
    //Carrying.
    carrier_info_struct* carrier_info; //Structure holding information on how this mob should be carried. If NULL, it cannot be carried.
    
    void tick();
};

#define GRAVITY_ADDER -1300.0f //Accelerate the Z speed of mobs affected by gravity by this amount per second.

enum MOB_TYPES {
    MOB_TYPE_PIKMIN,
    MOB_TYPE_ONION,
    MOB_TYPE_LEADER,
    MOB_TYPE_ENEMY,
    MOB_TYPE_TREASURE,
    MOB_TYPE_PELLET,
};

enum MOB_TEAMS {
    MOB_TEAM_NONE,       //Can hurt/target anyone and be hurt/targeted by anyone, on any team.
    MOB_TEAM_PLAYER_1,
    MOB_TEAM_PLAYER_2,
    MOB_TEAM_PLAYER_3,
    MOB_TEAM_PLAYER_4,
    MOB_TEAM_ENEMIES,
    MOB_TEAM_DECORATION, //Cannot be hurt or targeted by anything.
};

//Special targets to chase.
enum MOB_TARGETS {
    MOB_TARGET_NONE,
    MOB_TARGET_HOME,
    MOB_TARGET_POINT,
};

enum MOB_STATES {
    MOB_STATE_IDLE,
    MOB_STATE_BEING_CARRIED,
    MOB_STATE_BEING_DELIVERED, //Into an Onion.
    PIKMIN_STATE_IN_GROUP,
    PIKMIN_STATE_BURIED,
    PIKMIN_STATE_MOVING_TO_CARRY_SPOT,
    PIKMIN_STATE_CARRYING,
    PIKMIN_STATE_ATTACKING_MOB,
    PIKMIN_STATE_CELEBRATING,
};

#endif //ifndef MOB_INCLUDED
