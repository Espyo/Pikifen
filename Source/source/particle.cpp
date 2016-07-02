/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle class and particle-related functions.
 */

#include "drawing.h"
#include "functions.h"
#include "particle.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a particle.
 * type:     The type of particle. Use PARTICLE_TYPE_*.
 * x, y:     Starting coordinates.
 */
particle::particle(
    const unsigned char type, const float x, const float y,
    const float size, const float duration, const unsigned char priority
) :
    type(type),
    duration(duration),
    bitmap(nullptr),
    friction(1.0f),
    gravity(1.0f),
    size_grow_speed(0.0f),
    x(x),
    y(y),
    size(size),
    speed_x(0.0f),
    speed_y(0.0f),
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
    
    x += delta_t * speed_x;
    y += delta_t * speed_y;
    
    speed_x *= 1 - (delta_t * friction);
    speed_y *= 1 - (delta_t * friction);
    speed_y += delta_t * gravity;
    
    size += delta_t * size_grow_speed;
    size = max(0.0f, size);
}


/* ----------------------------------------------------------------------------
 * Draws this particle onto the world.
 */
void particle::draw() {
    if(type == PARTICLE_TYPE_SQUARE) {
        al_draw_filled_rectangle(
            x - size * 0.5,
            y - size * 0.5,
            x + size * 0.5,
            y + size * 0.5,
            change_alpha(
                color,
                (time / duration) *
                color.a * 255
            )
        );
        
    } else if(type == PARTICLE_TYPE_CIRCLE) {
        al_draw_filled_circle(
            x,
            y,
            size * 0.5,
            change_alpha(
                color,
                (time / duration) *
                color.a * 255
            )
        );
        
    } else if(type == PARTICLE_TYPE_BITMAP) {
        draw_sprite(
            bitmap,
            x,
            y,
            size, -1,
            0, change_alpha(
                color,
                (time / duration) *
                color.a * 255
            )
        );
        
    } else if(type == PARTICLE_TYPE_PIKMIN_SPIRIT) {
        draw_sprite(
            bitmap, x, y, size, -1,
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
            bitmap, x + s * 16, y,
            size, -1, s * M_PI,
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
            bitmap, x, y,
            s, s, 0, change_alpha(color, opacity)
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
    particles = NULL;
    max_nr = pg.max_nr;
    if(max_nr == 0) return *this;
    count = pg.count;
    particles = new particle[max_nr];
    for(size_t p = 0; p < count; ++p) {
        particles[p] = pg.particles[p];
    }
    
    return *this;
}


/* ----------------------------------------------------------------------------
 * Destroys a particle manager.
 */
particle_manager::~particle_manager() {
    if(particles) delete particles;
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
   * that are meant to appear BEFORE (under) the mobs.
   * So, you should call this function before and after drawing all mobs,
   * and set before_mobs to true before, and false after.
 */
void particle_manager::draw_all(const bool before_mobs) {
    for(size_t c = 0; c < count; ++c) {
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
   * in seconds. 0 means it spawns only one set and that's it.
 * base_particle:     All particles created will be based on this one.
   * Their properties will deviate randomly based on the
   * deviation members of the particle generator object.
 * number:            Number of particles to spawn.
   * This number is also deviated by number_deviation.
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
    x_deviation(0),
    y_deviation(0),
    speed_x_deviation(0),
    speed_y_deviation(0),
    size_deviation(0),
    angle(0),
    angle_deviation(0),
    speed(0),
    speed_deviation(0),
    follow_x(nullptr),
    follow_y(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Ticks one game frame of logic.
 */
void particle_generator::tick(const float delta_t, particle_manager &manager) {
    if(follow_x) {
        base_particle.x = *follow_x;
    }
    if(follow_y) {
        base_particle.y = *follow_y;
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
            (int) number + randomi(-number_deviation, number_deviation)
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
        new_p.x +=
            randomf(-x_deviation, x_deviation);
        new_p.y +=
            randomf(-y_deviation, y_deviation);
        new_p.size =
            max(
                0.0f,
                new_p.size +
                randomf(-size_deviation, size_deviation)
            );
        //For speed, let's decide if we should use
        //(speed_x and speed_y) or (speed and angle).
        //We'll use whichever one is not all zeros.
        if(angle != 0 || speed != 0) {
            angle_to_coordinates(
                angle + randomf(-angle_deviation, angle_deviation),
                speed + randomf(-speed_deviation, speed_deviation),
                &new_p.speed_x, &new_p.speed_y
            );
        } else {
            new_p.speed_x +=
                randomf(-speed_x_deviation, speed_x_deviation);
            new_p.speed_y +=
                randomf(-speed_y_deviation, speed_y_deviation);
        }
        
        manager.add(new_p);
    }
}


/* ----------------------------------------------------------------------------
 * Resets data about the particle generator, to make it ready to
 * be used. Call this when copying from another generator.
 */
void particle_generator::reset() {
    emission_interval = emission_interval;
}
