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

#include "../../core/const.h"
#include "../../core/world_component.h"
#include "../../util/drawing_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "../content.h"


using std::vector;


class Mob;


//Particle priorities.
enum PARTICLE_PRIORITY {

    //Low priority. Might be deleted to make way for most others.
    //Mostly useful for ambiance stuff that has no gameplay impact.
    PARTICLE_PRIORITY_LOW,
    
    //Medium priority.
    //Mostly useful for things revolving around gameplay elements.
    PARTICLE_PRIORITY_MEDIUM,
    
    //High priority. Might delete others to make way.
    //Mostly useful for things that are absolutely needed for gameplay clarity.
    PARTICLE_PRIORITY_HIGH,
    
};


//IDs for specific types of particle generators.
enum MOB_PARTICLE_GENERATOR_ID {

    //None.
    MOB_PARTICLE_GENERATOR_ID_NONE,
    
    //Particle generator issued by the script.
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


//Particle bitmap angle types.
enum PARTICLE_ANGLE_TYPE {

    //Fixed all throughout.
    PARTICLE_ANGLE_TYPE_FIXED,
    
    //Matches the direction of travel.
    PARTICLE_ANGLE_TYPE_DIRECTION,
    
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
struct ParticleEmission {

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
    Point rect_outer_dist = Point(0.0f);
    
    //Minimum random deviation of position, for square shapes.
    Point rect_inner_dist = Point(0.0f);
    
    //Maximum radius for circular emission.
    float circle_outer_dist = 0;
    
    //Minimum radius for circular emission.
    float circle_inner_dist = 0;
    
    //How many radians around the center particles can emit.
    float circle_arc = TAU;
    
    //How many radians the arc is rotated by.
    float circle_arc_rot = 0;
    
    //Are the particles placed evenly spread? If not, they're randomly spread.
    bool evenly_spread = false;
    
    
    //--- Function declarations ---
    
    explicit ParticleEmission(
        const float emission_interval = 0.0f, const size_t number = 1
    );
    Point get_emission_offset(float number_ratio);
    
};


/**
 * @brief A particle is best described with examples:
 * A puff of smoke, a sparkle, a smack.
 * There are several different types, which
 * change the way they look, how they behave over time, etc.
 */
struct Particle {

    //--- Members ---
    
    //Behavior stats.
    
    //How long its lifespan is.
    float duration = 0.0f;
    
    //Bitmap to use, if any.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //Angle the bitmap should be at.
    float bmp_angle = 0.0f;
    
    //Type of bitmap rotation.
    PARTICLE_ANGLE_TYPE bmp_angle_type = PARTICLE_ANGLE_TYPE_FIXED;
    
    //The bitmap's internal name, or an empty string to use a circle.
    string bmp_name = "";
    
    //Current state.
    
    //Current time left to live. 0 means it's dead.
    float time = 0.0f;
    
    //Current coordinates.
    Point pos;
    
    //Current Z.
    float z = 0.0f;
    
    //Where the particle generator was when this was emitted.
    Point origin;
    
    //Current size, in diameter.
    KeyframeInterpolator<float> size;
    
    //Linear velocity over time.
    KeyframeInterpolator<Point> linear_speed;
    
    //Outwards velocity over time.
    KeyframeInterpolator<float> outwards_speed;
    
    //Orbital velocity over time.
    KeyframeInterpolator<float> orbital_speed;
    
    //Current color.
    KeyframeInterpolator<ALLEGRO_COLOR> color;
    
    //Friction.
    float friction = 0.0f;
    
    //How much the particle has been slowed since being created.
    Point total_friction_applied = Point(0.0f);
    
    //Blend type.
    PARTICLE_BLEND_TYPE blend_type = PARTICLE_BLEND_TYPE_NORMAL;
    
    //Other stuff.
    
    //Priority. If we reached the particle limit, only spawn
    //this particle if it can replace a lower-priority one.
    PARTICLE_PRIORITY priority = PARTICLE_PRIORITY_MEDIUM;
    
    
    //--- Function declarations ---
    
    explicit Particle(
        const Point &pos = Point(), const float z = 0.0f,
        const float initial_size = 0.0f,
        const float duration = 0.0f, const PARTICLE_PRIORITY priority =
            PARTICLE_PRIORITY_MEDIUM,
        const ALLEGRO_COLOR initial_color = COLOR_WHITE
    );
    void draw();
    void set_bitmap(
        const string &new_bmp_name,
        DataNode* node = nullptr
    );
    void tick(float delta_t);
    
};


/**
 * @brief Manages a list of particles, allows the addition of new ones, etc.
 */
struct ParticleManager {

    public:
    
    //--- Function declarations ---
    
    explicit ParticleManager(size_t max_nr = 0);
    ParticleManager(const ParticleManager &pm2);
    ParticleManager &operator=(const ParticleManager &pm2);
    ~ParticleManager();
    void add(const Particle &p);
    void clear();
    void fill_component_list(
        vector<WorldComponent> &list,
        const Point &cam_tl = Point(), const Point &cam_br = Point()
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
    Particle* particles = nullptr;
    
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
struct ParticleGenerator : public Content {

    public:
    
    //--- Members ---
    
    //Optional ID, if you need to identify it later on.
    MOB_PARTICLE_GENERATOR_ID id = MOB_PARTICLE_GENERATOR_ID_NONE;
    
    //All particles created are based on this one.
    Particle base_particle;
    
    //How the generator should emit particles.
    ParticleEmission emission;
    
    //Follow the given mob's coordinates.
    Mob* follow_mob = nullptr;
    
    //Offset the follow mob coordinates by this, relative to the mob angle.
    Point follow_pos_offset;
    
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
    Point linear_speed_deviation = Point(0.0f);
    
    //How many degress linear speed can be rotated by.
    float linear_speed_angle_deviation = 0.0f;
    
    //Are the directions and angles provided absolute, or relative (to a mob)?
    bool angles_are_absolute = false;
    
    
    //--- Function declarations ---
    
    explicit ParticleGenerator(
        float emission_interval = 0.0f,
        const Particle &base_particle = Particle(), size_t number = 1
    );
    void tick(float delta_t, ParticleManager &manager);
    void emit(ParticleManager &manager);
    void restart_timer();
    void load_from_data_node(DataNode* node, CONTENT_LOAD_LEVEL level);
    void save_to_data_node(DataNode* node);
    
    
    private:
    
    //--- Members ---
    
    //Time left before the next emission.
    float emission_timer = 0.0f;
    
};
