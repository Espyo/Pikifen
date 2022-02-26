/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the particle class and particle-related functions.
 */

#ifndef PARTICLE_INCLUDED
#define PARTICLE_INCLUDED

#include <vector>

#include <allegro5/allegro.h>

#include "utils/geometry_utils.h"
#include "world_component.h"


using std::vector;


class mob;


//Types of particle. This controls their behavior and appearance.
enum PARTICLE_TYPES {
    //A simple square.
    PARTICLE_TYPE_SQUARE,
    //A simple circle.
    PARTICLE_TYPE_CIRCLE,
    //A bitmap.
    PARTICLE_TYPE_BITMAP,
    //A Pikmin spirit that moves up and vanishes.
    PARTICLE_TYPE_PIKMIN_SPIRIT,
    //An enemy spirit that moves up and wobbles.
    PARTICLE_TYPE_ENEMY_SPIRIT,
    //A smack that grows and shrinks real quick.
    PARTICLE_TYPE_SMACK,
    //A ding that grows and shrinks.
    PARTICLE_TYPE_DING,
};


//Particle priorities.
enum PARTICLE_PRIORITIES {
    //Low priority. Might be deleted to make way for most others.
    PARTICLE_PRIORITY_LOW,
    //Medium priority.
    PARTICLE_PRIORITY_MEDIUM,
    //High priority. Might delete others to make way.
    PARTICLE_PRIORITY_HIGH,
};


//IDs for specific types of particle generators.
enum MOB_PARTICLE_GENERATOR_IDS {
    //None.
    MOB_PARTICLE_GENERATOR_NONE,
    //Custom particle generator issued by the script.
    MOB_PARTICLE_GENERATOR_SCRIPT,
    //Trail effect left behind by a throw.
    MOB_PARTICLE_GENERATOR_THROW,
    //Ring-shaped wave when going in water.
    MOB_PARTICLE_GENERATOR_WAVE_RING,
    
    //Specific status effects are numbered starting on this.
    //So make sure this is the last on the enum.
    MOB_PARTICLE_GENERATOR_STATUS,
};


/* ----------------------------------------------------------------------------
 * A particle is best described with examples:
 * A puff of smoke, a sparkle, a smack.
 * There are several different types, which
 * change the way they look, how they behave over time, etc.
 */
struct particle {
    //Behavior stats.
    //Type.
    PARTICLE_TYPES type;
    //How long its lifespan is.
    float duration;
    //Bitmap to use, if any.
    ALLEGRO_BITMAP* bitmap;
    //Every second, speed is lost by this much.
    float friction;
    //Every second, the vertical speed is increased by this.
    float gravity;
    //Every second, the size is increased by this much.
    float size_grow_speed;
    
    //Current state.
    //Current time left to live. 0 means it's dead.
    float time;
    //Current coordinates.
    point pos;
    //Current Z.
    float z;
    //Current size, in diameter.
    float size;
    //Current movement speed.
    point speed;
    //Current color.
    ALLEGRO_COLOR color;
    
    //Other stuff.
    //Priority. If we reached the particle limit, only spawn
    //this particle if it can replace a lower-priority one.
    PARTICLE_PRIORITIES priority;
    
    particle(
        const PARTICLE_TYPES type = PARTICLE_TYPE_BITMAP,
        const point &pos = point(), const float z = 0.0f,
        const float size = 0.0f,
        const float duration = 0.0f, const PARTICLE_PRIORITIES priority =
            PARTICLE_PRIORITY_HIGH
    );
    void tick(const float delta_t);
    void draw();
};


/* ----------------------------------------------------------------------------
 * Manages a list of particles, allows the addition of new ones, etc.
 */
struct particle_manager {
public:
    void add(particle p);
    void clear();
    void fill_component_list(
        vector<world_component> &list,
        const point &cam_tl = point(), const point &cam_br = point()
    );
    size_t get_count() const;
    void tick_all(const float delta_t);
    
    particle_manager(const size_t &max_nr = 0);
    particle_manager(const particle_manager &pm2);
    particle_manager &operator=(const particle_manager &pm2);
    ~particle_manager();
    
private:
    //This list works as follows:
    //The first "count" particles are alive.
    //The next particle is the beginning of the dead ones.
    //When a particle is deleted, swap places between it and the first
    //"dead" particle, to preserve the list's logic.
    //When a particle is added, if the entire list is filled with live ones,
    //delete the one on position 0 (presumably the oldest).
    particle* particles;
    //How many particles are alive.
    size_t count;
    //Maximum number that can be stored.
    size_t max_nr;

    void remove(const size_t pos);
    
};


/* ----------------------------------------------------------------------------
 * Base class for the particle generator.
 * A particle generator creates particles in a steady flow and/or in a pattern.
 */
struct particle_generator {
public:
    //Optional ID, if you need to identify it later on.
    MOB_PARTICLE_GENERATOR_IDS id;
    //All particles created are based on this one.
    particle base_particle;
    //Number of particles to spawn.
    size_t number;
    //Interval at which to emit a new one. 0 means once only.
    float emission_interval;
    //Follow the given mob's coordinates.
    mob* follow_mob;
    //Offset the follow mob coordinates by this.
    point follow_pos_offset;
    //Offset the follow mob Z by this.
    float follow_z_offset;
    //Follow the given angle. e.g. a mob's angle.
    float* follow_angle;
    
    //Maximum random deviation of amount.
    size_t number_deviation;
    //Maximum random deviation of duration.
    float duration_deviation;
    //Maximum random deviation of friction.
    float friction_deviation;
    //Maximum random deviation of gravity.
    float gravity_deviation;
    //Maximum random deviation of size.
    float size_deviation;
    //Maximum random deviation of position.
    point pos_deviation;
    //Maximum random deviation of speed.
    point speed_deviation;
    //Angle they move at.
    float angle;
    //Maximum random deviation of angle.
    float angle_deviation;
    //Total speed they move at.
    float total_speed;
    //Maximum random deviation of total speed.
    float total_speed_deviation;
    
    particle_generator(
        const float emission_interval = 0.0f,
        const particle &base_particle = particle(), const size_t number = 1
    );
    void tick(const float delta_t, particle_manager &manager);
    void emit(particle_manager &manager);
    void reset();
    
private:
    //Time left before the next emission.
    float emission_timer;
    
};


#endif //ifndef PARTICLE_INCLUDED
