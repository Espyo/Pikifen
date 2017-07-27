/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle class and particle-related functions.
 */

#include <algorithm>

#include "drawing.h"
#include "functions.h"
#include "geometry_utils.h"
#include "particle.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a particle.
 * type:     The type of particle. Use PARTICLE_TYPE_*.
 * pos:      Starting coordinates.
 * size:     Diameter.
 * duration: Total lifespan.
 * priority: Lower priority particles will be removed in favor of higher ones.
 */
particle::particle(
    const unsigned char type, const point &pos, const float size,
    const float duration, const unsigned char priority
) :
    type(type),
    duration(duration),
    bitmap(nullptr),
    friction(1.0f),
    gravity(1.0f),
    size_grow_speed(0.0f),
    pos(pos),
    size(size),
    time(duration),
    color(al_map_rgb(255, 255, 255)),
    before_mobs(false),
    priority(priority) {
    
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
    
    speed.x *= 1 - (delta_t * friction);
    speed.y *= 1 - (delta_t * friction);
    speed.y += delta_t * gravity;
    
    size += delta_t * size_grow_speed;
    size = max(0.0f, size);
}


/* ----------------------------------------------------------------------------
 * Draws this particle onto the world.
 */
void particle::draw() {
    if(type == PARTICLE_TYPE_SQUARE) {
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
        
    } else if(type == PARTICLE_TYPE_CIRCLE) {
        al_draw_filled_circle(
            pos.x, pos.y,
            size * 0.5,
            change_alpha(
                color,
                (time / duration) *
                color.a * 255
            )
        );
        
    } else if(type == PARTICLE_TYPE_BITMAP) {
        draw_sprite(
            bitmap, pos, point(size, -1),
            0, change_alpha(
                color,
                (time / duration) *
                color.a * 255
            )
        );
        
    } else if(type == PARTICLE_TYPE_PIKMIN_SPIRIT) {
        draw_sprite(
            bitmap, pos, point(size, -1),
            0, change_alpha(
                color,
                fabs(
                    sin((time / duration) * M_PI)
                ) * color.a * 255
            )
        );
        
    } else if(type == PARTICLE_TYPE_ENEMY_SPIRIT) {
        float s = sin((time / duration) * M_PI);
        draw_sprite(
            bitmap,
            point(pos.x + s * 16, pos.y),
            point(size, -1), s * M_PI,
            change_alpha(
                color, fabs(s) * color.a * 255
            )
        );
        
    } else if(type == PARTICLE_TYPE_SMACK) {
        float r = time / duration;
        float s = size;
        float opacity = 255;
        if(r <= 0.5) s *= r * 2;
        else opacity *= (1 - r) * 2;
        
        draw_sprite(
            bitmap, pos, point(s, s),
            0, change_alpha(color, opacity)
        );
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
 * Copies a particle manager from another one.
 */
particle_manager &particle_manager::operator =(const particle_manager &pg) {

    if(this != &pg) {
        this->particles = NULL;
        max_nr = pg.max_nr;
        if(max_nr == 0) return *this;
        count = pg.count;
        this->particles = new particle[max_nr];
        for(size_t p = 0; p < count; ++p) {
            this->particles[p] = pg.particles[p];
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


/* ----------------------------------------------------------------------------
 * Draws all particles, if they're meant to be drawn.
 * before_mobs: If true, we're trying to draw the particles
 *   that are meant to appear BEFORE (under) the mobs.
 *   So, you should call this function before and after drawing all mobs,
 *   and set before_mobs to true before, and false after.
 * cam_box:     Only draw particles inside this frame.
 */
void particle_manager::draw_all(
    const bool before_mobs,
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
        
        if(before_mobs == particles[c].before_mobs) {
            particles[c].draw();
        }
    }
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
    particle base_particle, const size_t number
) :
    id(0),
    emission_timer(emission_interval),
    base_particle(base_particle),
    number(number),
    emission_interval(emission_interval),
    number_deviation(0),
    duration_deviation(0),
    friction_deviation(0),
    gravity_deviation(0),
    size_deviation(0),
    angle(0),
    angle_deviation(0),
    total_speed(0),
    total_speed_deviation(0),
    follow(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Ticks one game frame of logic.
 */
void particle_generator::tick(const float delta_t, particle_manager &manager) {
    if(follow) {
        base_particle.pos = *follow;
    }
    emission_timer -= delta_t;
    if(emission_timer <= 0.0f) {
        emit(manager);
        emission_timer = emission_interval;
    }
}


/* ----------------------------------------------------------------------------
 * Emits the particles, regardless of the timer.
 * manager: The particle manager to place these particles on.
 */
void particle_generator::emit(particle_manager &manager) {
    size_t final_nr =
        max(
            0,
            (int) number +
            randomi((int) (0 - number_deviation), (int) number_deviation)
        );
        
    for(size_t p = 0; p < final_nr; ++p) {
        particle new_p = base_particle;
        
        new_p.duration =
            max(
                0.0f,
                new_p.duration +
                randomf(-duration_deviation, duration_deviation)
            );
        new_p.time = new_p.duration;
        new_p.friction +=
            randomf(-friction_deviation, friction_deviation);
        new_p.gravity +=
            randomf(-gravity_deviation, gravity_deviation);
        new_p.pos.x +=
            randomf(-pos_deviation.x, pos_deviation.x);
        new_p.pos.y +=
            randomf(-pos_deviation.y, pos_deviation.y);
        new_p.size =
            max(
                0.0f,
                new_p.size +
                randomf(-size_deviation, size_deviation)
            );
        //For speed, let's decide if we should use
        //(speed.x and speed.y) or (speed and angle).
        //We'll use whichever one is not all zeros.
        if(angle != 0 || total_speed != 0) {
            new_p.speed =
                angle_to_coordinates(
                    angle + randomf(-angle_deviation, angle_deviation),
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
