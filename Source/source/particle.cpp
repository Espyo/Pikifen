/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle class and particle-related functions.
 */

#include <algorithm>

#include "particle.h"

#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "mobs/mob.h"
#include "utils/geometry_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a particle.
 * type:     The type of particle. Use PARTICLE_TYPE_*.
 * pos:      Starting coordinates.
 * size:     Diameter.
 * duration: Total lifespan.
 * priority: Lower priority particles will be removed in favor of higher ones.
 */
particle::particle(
    const unsigned char type, const point &pos, const float z,
    const float size, const float duration, const unsigned char priority
) :
    type(type),
    duration(duration),
    bitmap(nullptr),
    friction(1.0f),
    gravity(1.0f),
    size_grow_speed(0.0f),
    time(duration),
    pos(pos),
    z(z),
    size(size),
    color(al_map_rgb(255, 255, 255)),
    priority(priority) {
    
}


/* ----------------------------------------------------------------------------
 * Draws this particle onto the world.
 */
void particle::draw() {
    switch(type) {
    case PARTICLE_TYPE_SQUARE: {
        al_draw_filled_rectangle(
            pos.x - size * 0.5,
            pos.y - size * 0.5,
            pos.x + size * 0.5,
            pos.y + size * 0.5,
            change_alpha(
                color,
                (time / duration) *
                color.a * 255
            )
        );
        break;
        
    } case PARTICLE_TYPE_CIRCLE: {
        al_draw_filled_circle(
            pos.x, pos.y,
            size * 0.5,
            change_alpha(
                color,
                (time / duration) *
                color.a * 255
            )
        );
        break;
        
    } case PARTICLE_TYPE_BITMAP: {
        draw_bitmap(
            bitmap, pos, point(size, -1),
            0, change_alpha(
                color,
                (time / duration) *
                color.a * 255
            )
        );
        break;
        
    } case PARTICLE_TYPE_PIKMIN_SPIRIT: {
        draw_bitmap(
            bitmap, pos, point(size, -1),
            0, change_alpha(
                color,
                fabs(
                    sin((time / duration) * TAU / 2)
                ) * color.a * 255
            )
        );
        break;
        
    } case PARTICLE_TYPE_ENEMY_SPIRIT: {
        float s = sin((time / duration) * TAU / 2);
        draw_bitmap(
            bitmap,
            point(pos.x + s * 16, pos.y),
            point(size, -1), s * TAU / 2,
            change_alpha(
                color, fabs(s) * color.a * 255
            )
        );
        break;
        
    } case PARTICLE_TYPE_SMACK: {
        float r = time / duration;
        float s = size;
        float opacity = 255;
        if(r <= 0.5) s *= r * 2;
        else opacity *= (1 - r) * 2;
        
        draw_bitmap(
            bitmap, pos, point(s, s),
            0, change_alpha(color, opacity)
        );
        break;
        
    } case PARTICLE_TYPE_DING: {
        float r = time / duration;
        float s = size;
        float opacity = 255;
        if(r >= 0.5) s *= (1 - r) * 2;
        else opacity *= r * 2;
        
        draw_bitmap(
            bitmap, pos, point(s, s),
            0, change_alpha(color, opacity)
        );
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Makes a particle follow a game tick.
 * Returns false if its lifespan is over and it should be deleted.
 */
void particle::tick(const float delta_t) {
    time -= delta_t;
    
    if(time <= 0.0f) {
        time = 0.0f;
        return;
    }
    
    pos += speed * delta_t;
    
    speed.x *= 1 - (delta_t* friction);
    speed.y *= 1 - (delta_t* friction);
    speed.y += delta_t* gravity;
    
    size += delta_t* size_grow_speed;
    size = std::max(0.0f, size);
}


/* ----------------------------------------------------------------------------
 * Creates a particle generator.
 * type:              Type of generator. Use PARTICLE_GENERATOR_*.
 * emission_interval: Interval to spawn a new set of particles in,
 *   in seconds. 0 means it spawns only one set and that's it.
 * base_particle:     All particles created will be based on this one.
 *   Their properties will deviate randomly based on the
 *   deviation members of the particle generator object.
 * number:            Number of particles to spawn.
 *   This number is also deviated by number_deviation.
 */
particle_generator::particle_generator(
    const float emission_interval,
    const particle &base_particle, const size_t number
) :
    emission_timer(emission_interval),
    id(0),
    base_particle(base_particle),
    number(number),
    emission_interval(emission_interval),
    follow_mob(nullptr),
    follow_z_offset(0),
    follow_angle(nullptr),
    number_deviation(0),
    duration_deviation(0),
    friction_deviation(0),
    gravity_deviation(0),
    size_deviation(0),
    angle(0),
    angle_deviation(0),
    total_speed(0),
    total_speed_deviation(0) {
    
}


/* ----------------------------------------------------------------------------
 * Emits the particles, regardless of the timer.
 * manager: The particle manager to place these particles on.
 */
void particle_generator::emit(particle_manager &manager) {
    point base_p_pos = base_particle.pos;
    float base_p_z = base_particle.z;
    point offs = follow_pos_offset;
    if(follow_angle) {
        offs = rotate_point(offs, *follow_angle);
    }
    base_p_pos += offs;
    base_p_z += follow_z_offset;
    
    if(
        base_p_pos.x < game.cam.box[0].x ||
        base_p_pos.x > game.cam.box[1].x ||
        base_p_pos.y < game.cam.box[0].y ||
        base_p_pos.y > game.cam.box[1].y
    ) {
        //Too far off-camera.
        return;
    }
    
    size_t final_nr =
        std::max(
            0,
            (int) number +
            randomi((int) (0 - number_deviation), (int) number_deviation)
        );
        
    for(size_t p = 0; p < final_nr; ++p) {
        particle new_p = base_particle;
        
        new_p.duration =
            std::max(
                0.0f,
                new_p.duration +
                randomf(-duration_deviation, duration_deviation)
            );
        new_p.time = new_p.duration;
        new_p.friction +=
            randomf(-friction_deviation, friction_deviation);
        new_p.gravity +=
            randomf(-gravity_deviation, gravity_deviation);
            
        new_p.pos = base_p_pos;
        point pos_offset_to_use(
            randomf(-pos_deviation.x, pos_deviation.x),
            randomf(-pos_deviation.y, pos_deviation.y)
        );
        if(follow_angle) {
            pos_offset_to_use = rotate_point(pos_offset_to_use, *follow_angle);
        }
        new_p.pos += pos_offset_to_use;
        
        new_p.z = base_p_z;
        new_p.size =
            std::max(
                0.0f,
                new_p.size +
                randomf(-size_deviation, size_deviation)
            );
        //For speed, let's decide if we should use
        //(speed.x and speed.y) or (speed and angle).
        //We'll use whichever one is not all zeros.
        if(angle != 0 || total_speed != 0) {
            float angle_to_use = angle;
            if(follow_angle) angle_to_use += (*follow_angle);
            
            new_p.speed =
                angle_to_coordinates(
                    angle_to_use + randomf(-angle_deviation, angle_deviation),
                    total_speed +
                    randomf(-total_speed_deviation, total_speed_deviation)
                );
        } else {
            new_p.speed.x +=
                randomf(-speed_deviation.x, speed_deviation.x);
            new_p.speed.y +=
                randomf(-speed_deviation.y, speed_deviation.y);
        }
        
        manager.add(new_p);
    }
}


/* ----------------------------------------------------------------------------
 * Resets data about the particle generator, to make it ready to
 * be used. Call this when copying from another generator.
 */
void particle_generator::reset() {
    emission_timer = emission_interval;
}


/* ----------------------------------------------------------------------------
 * Ticks one game frame of logic.
 */
void particle_generator::tick(const float delta_t, particle_manager &manager) {
    if(follow_mob) {
        base_particle.pos = follow_mob->pos;
        base_particle.z = follow_mob->z;
    }
    emission_timer -= delta_t;
    if(emission_timer <= 0.0f) {
        emit(manager);
        emission_timer = emission_interval;
    }
}


/* ----------------------------------------------------------------------------
 * Creates a particle manager.
 */
particle_manager::particle_manager(const size_t &max_nr) :
    particles(nullptr),
    max_nr(max_nr) {
    
    if(max_nr == 0) return;
    particles = new particle[max_nr];
    clear();
}


/* ----------------------------------------------------------------------------
 * Creates a particle manager by copying data from another.
 */
particle_manager::particle_manager(const particle_manager &pm2) :
    particles(NULL),
    count(pm2.count),
    max_nr(pm2.max_nr) {
    
    particles = new particle[max_nr];
    for(size_t p = 0; p < count; ++p) {
        this->particles[p] = pm2.particles[p];
    }
}


/* ----------------------------------------------------------------------------
 * Copies a particle manager from another one.
 */
const particle_manager &particle_manager::operator =(
    const particle_manager &pm2
) {

    if(this != &pm2) {
        if(this->particles) {
            delete[] this->particles;
        }
        this->particles = NULL;
        max_nr = pm2.max_nr;
        count = pm2.count;
        if(max_nr == 0) return *this;
        this->particles = new particle[max_nr];
        for(size_t p = 0; p < count; ++p) {
            this->particles[p] = pm2.particles[p];
        }
    }
    
    return *this;
}


/* ----------------------------------------------------------------------------
 * Destroys a particle manager.
 */
particle_manager::~particle_manager() {
    if(particles) delete[] particles;
}


/* ----------------------------------------------------------------------------
 * Adds a new particle to the list. It will fail if there is no slot
 * where it can be added to.
 */
void particle_manager::add(particle p) {
    if(max_nr == 0) return;
    
    //The first "count" particles are alive. Add the new one after.
    //...Unless count already equals the max. That means the list is full.
    //Let's try to dump a particle with lower priority.
    //Starting from 0 will (hopefully) give us the oldest one first.
    bool success = true;
    if(count == max_nr) {
        success = false;
        for(size_t i = 0; i < max_nr; ++i) {
            if(particles[i].priority < p.priority) {
                remove(i);
                success = true;
                break;
            }
        }
    }
    
    //No room for this particle.
    if(!success)
        return;
        
    particles[count] = p;
    count++;
}


/* ----------------------------------------------------------------------------
 * Clears the list.
 */
void particle_manager::clear() {
    for(size_t p = 0; p < max_nr; ++p) {
        particles[p].time = 0.0f;
    }
    count = 0;
}


/* ----------------------------------------------------------------------------
 * Adds the particle pointers to the provided list of world component,
 * so that the particles can be drawn, after being Z-sorted.
 * list:           The list to populate.
 * cam_tl, cam_br: Only draw particles inside this frame.
 */
void particle_manager::fill_component_list(
    vector<world_component> &list,
    const point &cam_tl, const point &cam_br
) {
    for(size_t c = 0; c < count; ++c) {
    
        particle* p_ptr = &particles[c];
        
        if(
            cam_tl != cam_br &&
            !rectangles_intersect(
                p_ptr->pos - p_ptr->size, p_ptr->pos + p_ptr->size,
                cam_tl, cam_br
            )
        ) {
            //Off-camera.
            continue;
        }
        
        world_component wc;
        wc.particle_ptr = p_ptr;
        wc.z = p_ptr->z;
        list.push_back(wc);
    }
}


/* ----------------------------------------------------------------------------
 * Returns how many are in the list.
 */
size_t particle_manager::get_count() const {
    return count;
}


/* ----------------------------------------------------------------------------
 * Removes a particle from the list.
 */
void particle_manager::remove(const size_t pos) {
    if(pos > count) return;
    
    //To remove a particle, let's simply move its data to the start of
    //the "dead" particles. A particle is considered dead if its time is 0.
    particles[pos].time = 0.0f;
    
    //Because the first "count" members are alive, we'll swap this dead
    //particle with the last living one. This means this particle
    //will represent the new start of the dead ones.
    
    //But hey, if we only had one particle, we can just skip this!
    if(count == 1) {
        count = 0;
        return;
    }
    
    //Place the last live particle on this now-unused position.
    particles[pos] = particles[count - 1];
    //And this new "dead" particle should be marked as such.
    particles[count - 1].time = 0.0f;
    
    count--;
}


/* ----------------------------------------------------------------------------
 * Ticks all particles in the list.
 */
void particle_manager::tick_all(const float delta_t) {
    for(size_t c = 0; c < count;) {
        particles[c].tick(delta_t);
        if(particles[c].time == 0.0f) {
            remove(c);
        } else {
            ++c;
        }
    }
}
