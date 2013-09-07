#ifndef CONST_INCLUDED
#define CONST_INCLUDED

#define CURSOR_MAX_DIST           300    //The cursor can only be these many pixels away from the captain.
#define GRAVITY_ADDER             -0.1f  //Accelerate the Z speed of mobs affected by gravity by this amount per second.
#define HEALTH_CIRCLE_RADIUS      20     //Radius of the health meter's circle.
#define LEADER_MOVE_SPEED         100    //Max speed at which a leader can move.
#define MAX_WHISTLE_RADIUS        80     //The whistle can't go past this radius.
#define MIN_PIKMIN_GRABBING_RANGE 60     //The leader needs to be at least this close to a Pikmin to grab it.
#define MIN_PLUCK_RANGE           30     //The leader needs to be at least this close to a burrowed Pikmin to pluck it.
#define WHISTLE_MAX_HOLD_TIME     1.5    //After the whistle reaches its maximum size, hold it for these many seconds until it stops by itself.
#define WHISTLE_RADIUS_GROWTH_PS  180    //The whistle's radius grows these many pixels per second.

#define DEF_FPS 30
#define DEF_PIKMIN_SIZE 24
#define DEF_SCR_H 600
#define DEF_SCR_W 400

#endif //ifndef CONST_INCLUDED