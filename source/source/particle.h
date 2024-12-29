/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the particle class and particle-related functions.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>

#include "const.h"
#include "content.h"
#include "utils/drawing_utils.h"
#include "utils/general_utils.h"
#include "utils/geometry_utils.h"
#include "world_component.h"


using std::vector;


class mob;


//Particle priorities.
enum PARTICLE_PRIORITY {

    //Low priority. Might be deleted to make way for most others.
    PARTICLE_PRIORITY_LOW,
    
    //Medium priority.
    PARTICLE_PRIORITY_MEDIUM,
    
    //High priority. Might delete others to make way.
    PARTICLE_PRIORITY_HIGH,
    
};


//IDs for specific types of particle generators.
enum MOB_PARTICLE_GENERATOR_ID {

    //None.
    MOB_PARTICLE_GENERATOR_ID_NONE,
    
    //Custom particle generator issued by the script.
    MOB_PARTICLE_GENERATOR_ID_SCRIPT,
    
    //Trail effect left behind by a throw.
    MOB_PARTICLE_GENERATOR_ID_THROW,
    
    //Ring-shaped wave when going in water.
    MOB_PARTICLE_GENERATOR_ID_WAVE_RING,
    
    //Specific status effects are numbered starting on this.
    //So make sure this is the last on the enum.
    MOB_PARTICLE_GENERATOR_ID_STATUS,
    
};


//Shapes for particles to emit from.
enum PARTICLE_EMISSION_SHAPE {

    //Circular emission area
    PARTICLE_EMISSION_SHAPE_CIRCLE,
    
    //Rectangular emission area
    PARTICLE_EMISSION_SHAPE_RECTANGLE,
    
};


//Blending modes for particle colors.
enum PARTICLE_BLEND_TYPE {

    //Normal blending.
    PARTICLE_BLEND_TYPE_NORMAL,
    
    //Additive blending.
    PARTICLE_BLEND_TYPE_ADDITIVE,
    
};


/**
 * @brief A description of how a particle
 * generator should emit particles.
 */
struct particle_emission_struct {

    public:
    
    //Shape for particles to emit from.
    PARTICLE_EMISSION_SHAPE shape = PARTICLE_EMISSION_SHAPE_RECTANGLE;
    
    //Number of particles to spawn.
    size_t number = 0;
    
    //Maximum random deviation of amount.
    size_t number_deviation = 0;
    
    //Interval at which to emit a new one. 0 means they're emitted once only.
    float interval = 0.0f;
    
    //Maximum random deviation of interval.
    float interval_deviation = 0.0f;
    
    //Maximum random deviation of position, for square shapes.
    point rect_outer_dist = point(0, 0);
    
    //Minimum random deviation of position, for square shapes.
    point rect_inner_dist = point(0, 0);
    
    //Maximum radius for circular emission.
    float circle_outer_dist = 0;
    
    //Minimum radius for circular emission.
    float circle_inner_dist = 0;
    
    //How many radians around the center particles can emit.
    float circle_arc = TAU;
    
    //How many radians the arc is rotated by.
    float circle_arc_rot = 0;
    
    
    //--- Function declarations ---
    
    explicit particle_emission_struct(
        const float emission_interval = 0.0f, const size_t number = 1
    );
    point get_emission_offset();
    
};


/**
 * @brief A particle is best described with examples:
 * A puff of smoke, a sparkle, a smack.
 * There are several different types, which
 * change the way they look, how they behave over time, etc.
 */
struct particle {

    //--- Members ---
    
    //Behavior stats.
    
    //How long its lifespan is.
    float duration = 0.0f;
    
    //Bitmap to use, if any.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //Angle the bitmap should be at.
    float bmp_angle = 0.0f;
    
    //The bitmap's internal name, or an empty string to use a circle.
    string file = "";
    
    //Current state.
    
    //Current time left to live. 0 means it's dead.
    float time = 0.0f;
    
    //Current coordinates.
    point pos;
    
    //Current Z.
    float z = 0.0f;
    
    //Where the particle generator was when this was emitted.
    point origin;
    
    //Current size, in diameter.
    keyframe_interpolator<float> size;
    
    //Linear velocity over time.
    keyframe_interpolator<point> linear_speed;
    
    //Outwards velocity over time.
    keyframe_interpolator<float> outwards_speed;
    
    //Orbital velocity over time.
    keyframe_interpolator<float> orbital_speed;
    
    //Current color.
    keyframe_interpolator<ALLEGRO_COLOR> color;
    
    //Friction.
    float friction = 0.0f;
    
    //How much the particle has been slowed since being created.
    point total_friction_applied = point(0, 0);
    
    //Blend type.
    PARTICLE_BLEND_TYPE blend_type = PARTICLE_BLEND_TYPE_NORMAL;
    
    //Other stuff.
    
    //Priority. If we reached the particle limit, only spawn
    //this particle if it can replace a lower-priority one.
    PARTICLE_PRIORITY priority;
    
    
    //--- Function declarations ---
    
    explicit particle(
        const point &pos = point(), const float z = 0.0f,
        const float initial_size = 0.0f,
        const float duration = 0.0f, const PARTICLE_PRIORITY priority =
            PARTICLE_PRIORITY_HIGH,
        const ALLEGRO_COLOR initial_color = COLOR_WHITE
    );
    void draw();
    void set_bitmap(
        const string &new_file_name,
        data_node* node = nullptr
    );
    void tick(float delta_t);
    
};


/**
 * @brief Manages a list of particles, allows the addition of new ones, etc.
 */
struct particle_manager {

    public:
    
    //--- Function declarations ---
    
    explicit particle_manager(size_t max_nr = 0);
    particle_manager(const particle_manager &pm2);
    particle_manager &operator=(const particle_manager &pm2);
    ~particle_manager();
    void add(const particle &p);
    void clear();
    void fill_component_list(
        vector<world_component> &list,
        const point &cam_tl = point(), const point &cam_br = point()
    );
    size_t get_count() const;
    void tick_all(float delta_t);
    
    private:
    
    //--- Members ---
    
    //This list works as follows:
    //The first "count" particles are alive.
    //The next particle is the beginning of the dead ones.
    //When a particle is deleted, swap places between it and the first
    //"dead" particle, to preserve the list's logic.
    //When a particle is added, if the entire list is filled with live ones,
    //delete the one on position 0 (presumably the oldest).
    particle* particles = nullptr;
    
    //How many particles are alive.
    size_t count = 0;
    
    //Maximum number that can be stored.
    size_t max_nr = 0;
    
    
    //--- Function declarations ---
    
    void remove(size_t pos);
    
};


/**
 * @brief Base class for the particle generator.
 * A particle generator creates particles in a steady flow and/or in a pattern.
 */
struct particle_generator : public content {

    public:
    
    //--- Members ---
    
    //Optional ID, if you need to identify it later on.
    MOB_PARTICLE_GENERATOR_ID id = MOB_PARTICLE_GENERATOR_ID_NONE;
    
    //All particles created are based on this one.
    particle base_particle;
    
    //How the generator should emit particles.
    particle_emission_struct emission;
    
    //Follow the given mob's coordinates.
    mob* follow_mob = nullptr;
    
    //Offset the follow mob coordinates by this, relative to the mob angle.
    point follow_pos_offset;
    
    //Offset the follow mob Z by this.
    float follow_z_offset = 0.0f;
    
    //Follow the given angle. e.g. a mob's angle.
    float* follow_angle = nullptr;
    
    //Maximum random deviation of the bitmap's rotation.
    float bmp_angle_deviation = 0.0f;
    
    //Maximum random deviation of duration.
    float duration_deviation = 0.0f;
    
    //Maximum random deviation of friction.
    float friction_deviation = 0.0f;
    
    //Maximum random deviation of size.
    float size_deviation = 0.0f;
    
    //Maximum random deviation of outward_speed.
    float outwards_speed_deviation = 0.0f;
    
    //Maximum random deviation of orbital speed.
    float orbital_speed_deviation = 0.0f;
    
    //Maximum random deviation of speed.
    point linear_speed_deviation = point(0, 0);
    
    //How many degress linear speed can be rotated by.
    float linear_speed_angle_deviation = 0.0f;
    
    
    //--- Function declarations ---
    
    explicit particle_generator(
        float emission_interval = 0.0f,
        const particle &base_particle = particle(), size_t number = 1
    );
    void tick(float delta_t, particle_manager &manager);
    void emit(particle_manager &manager);
    void reset();
    void load_from_data_node(data_node* node, CONTENT_LOAD_LEVEL level);
    void save_to_data_node(data_node* node);
    
    
    private:
    
    //--- Members ---
    
    //Time left before the next emission.
    float emission_timer;
    
};
