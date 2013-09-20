#ifndef CONST_INCLUDED
#define CONST_INCLUDED

#include <math.h>
#define _USE_MATH_DEFINES

#define CAM_TRANSITION_DURATION   0.5      //How many seconds a camera transition lasts for.
#define CURSOR_MAX_DIST           200      //The cursor can only be these many pixels away from the captain.
#define GRAVITY_ADDER             -2.0f    //Accelerate the Z speed of mobs affected by gravity by this amount per second.
#define IDLE_GLOW_SPIN_SPEED      M_PI*0.5 //The idle glow spins these many radians per second.
#define INFO_SPOT_TRIGGER_RANGE   64       //If the current captain is at this distance or closer from an info spot, it gets triggered.
#define LEADER_MOVE_SPEED         100      //Max speed at which a leader can move.
#define MAX_SHADOW_WIDTH          100      //The shadows can't be any wider than this.
#define MAX_WHISTLE_RADIUS        80       //The whistle can't go past this radius.
#define MAX_ZOOM_LEVEL            2        //Maximum zoom level possible.
#define MIN_ONION_CHECK_RANGE     64       //The minimum distance a leader must be from the onion in order to check it.
#define MIN_PIKMIN_GRABBING_RANGE 60       //The leader needs to be at least this close to a Pikmin to grab it.
#define MIN_PIKMIN_TASK_RANGE     20       //If there's this gap between a Pikmin and a task, the Pikmin will take the task.
#define MIN_PLUCK_RANGE           30       //The leader needs to be at least this close to a burrowed Pikmin to pluck it.
#define MIN_ZOOM_LEVEL            0.5      //Minimum zoom level possible.
#define SHADOW_Y_MULTIPLIER       30       //For every unit above the ground that the mob is on, the shadow goes these many pixels down.
#define THROW_DISTANCE_MULTIPLIER 0.49     //When a leader throws a Pikmin, multiply their strength by this.
#define UNCALLABLE_PERIOD         1        //A mob cannot be called to a party during this period.
#define WHISTLE_MAX_HOLD_TIME     1.5      //After the whistle reaches its maximum size, hold it for these many seconds until it stops by itself.
#define WHISTLE_RADIUS_GROWTH_PS  180      //The whistle's radius grows these many pixels per second.

#define DEF_FPS 30
#define DEF_PIKMIN_SIZE 24
#define DEF_SCR_H 600
#define DEF_SCR_W 400

#endif //ifndef CONST_INCLUDED