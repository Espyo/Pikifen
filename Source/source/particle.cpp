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
#include "utils/allegro_utils.h"
#include "utils/geometry_utils.h"
#include "utils/string_utils.h"


/**
 * @brief Constructs a new particle object.
 *
 * @param type The type of particle.
 * @param pos Starting coordinates.
 * @param z Starting Z coordinate.
 * @param size Diameter.
 * @param duration Total lifespan.
 * @param priority Lower priority particles will be removed in favor
 * of higher ones.
 */
particle::particle(
    const PARTICLE_TYPE type, const point &pos, const float z,
    const float size, const float duration, const PARTICLE_PRIORITY priority
) :
    type(type),
    duration(duration),
    time(duration),
    pos(pos),
    z(z),
    size(size),
    priority(priority) {
    
}


/**
 * @brief Draws this particle onto the world.
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


/**
 * @brief Ticks a particle's time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 * @return Whether its lifespan is over (meaning it should be deleted).
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


/**
 * @brief Constructs a new particle generator object.
 *
 * @param emission_interval Interval to spawn a new set of particles in,
 * in seconds. 0 means it spawns only one set and that's it.
 * @param base_particle All particles created will be based on this one.
 * Their properties will deviate randomly based on the
 * deviation members of the particle generator object.
 * @param number Number of particles to spawn.
 * This number is also deviated by number_deviation.
 */
particle_generator::particle_generator(
    const float emission_interval,
    const particle &base_particle, const size_t number
) :
    base_particle(base_particle),
    number(number),
    emission_interval(emission_interval),
    emission_timer(emission_interval) {
    
}
/**
 * @brief Unloads.
 */
void particle_generator::destroy() {
    
}
/**
 * @brief Emits the particles, regardless of the timer.
 *
 * @param manager The particle manager to place these particles on.
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


/**
 * @brief Loads particle generator data from a data node.
 * 
 * @param node Data node to load from.
 * @param load_resources If true, things like bitmaps and the like will
 * be loaded as well. If you don't need those, set this to false to make
 * it load faster.
 */
void particle_generator::load_from_data_node(
    data_node* node, bool load_resources
) {
    //Content metadata.
    load_metadata_from_data_node(node);

    //Standard data.
    reader_setter grs(node);
    data_node* p_node = node->get_child_by_name("base");
    reader_setter prs(p_node);
    
    float emission_interval_float = 0.0f;
    size_t number_int = 1;
    data_node* bitmap_node = nullptr;
    
    grs.set("emission_interval", emission_interval_float);
    grs.set("number", number_int);
    
    prs.set("bitmap", base_particle.bitmap_str, &bitmap_node);
    prs.set("duration", base_particle.duration);
    prs.set("friction", base_particle.friction);
    prs.set("gravity", base_particle.gravity);
    prs.set("size_grow_speed", base_particle.size_grow_speed);
    prs.set("size", base_particle.size);
    prs.set("speed", base_particle.speed);
    prs.set("color", base_particle.color);
    
    if(bitmap_node) {
        if(load_resources) {
            base_particle.bitmap =
                game.bitmaps.get(
                    base_particle.bitmap_str, bitmap_node
                );
        }
        base_particle.type = PARTICLE_TYPE_BITMAP;
    } else {
        base_particle.type = PARTICLE_TYPE_CIRCLE;
    }
    
    base_particle.time = base_particle.duration;
    base_particle.priority = PARTICLE_PRIORITY_MEDIUM;
    
    emission_interval = emission_interval_float;
    number = number_int;
    
    grs.set("interval_deviation", interval_deviation);
    grs.set("number_deviation", number_deviation);
    grs.set("duration_deviation", duration_deviation);
    grs.set("friction_deviation", friction_deviation);
    grs.set("gravity_deviation", gravity_deviation);
    grs.set("size_deviation", size_deviation);
    grs.set("pos_deviation", pos_deviation);
    grs.set("speed_deviation", speed_deviation);
    grs.set("angle", angle);
    grs.set("angle_deviation", angle_deviation);
    grs.set("total_speed", total_speed);
    grs.set("total_speed_deviation", total_speed_deviation);
    
    angle = deg_to_rad(angle);
    angle_deviation = deg_to_rad(angle_deviation);
    
    id =
        (MOB_PARTICLE_GENERATOR_ID) (
            MOB_PARTICLE_GENERATOR_ID_STATUS +
            game.content.custom_particle_generators.size()
        );
}


void particle_generator::save_to_data_node(
    data_node* node
) {
    //Content metadata.
    save_metadata_to_data_node(node);

    node->add(new data_node("emission_interval", f2s(emission_interval)));
    node->add(new data_node("number", i2s(number)));
    node->add(new data_node("number_deviation", i2s(number_deviation)));
    data_node* bitmap_node = new data_node("base", "");
    bitmap_node->add(new data_node("bitmap", base_particle.bitmap_str));
    bitmap_node->add(new data_node("duration", f2s(base_particle.duration)));
    bitmap_node->add(new data_node("friction", f2s(base_particle.friction)));
    bitmap_node->add(new data_node("gravity", f2s(base_particle.gravity)));
    bitmap_node->add(new data_node("size_grow_speed", f2s(base_particle.size_grow_speed)));
    bitmap_node->add(new data_node("size", f2s(base_particle.size)));
    bitmap_node->add(new data_node("speed", p2s(base_particle.speed)));
    bitmap_node->add(new data_node("color", c2s(base_particle.color)));

    node->add(new data_node("duration_deviation", f2s(duration_deviation)));
    node->add(new data_node("friciton_deviation", f2s(friction_deviation)));
    node->add(new data_node("gravity_deviation", f2s(gravity_deviation)));
    node->add(new data_node("size_deviation", f2s(size_deviation)));
    node->add(new data_node("pos_deviation", p2s(pos_deviation)));
    node->add(new data_node("speed_deviation", p2s(speed_deviation)));
    node->add(new data_node("angle", f2s(angle)));
    node->add(new data_node("angle_deviation", f2s(angle_deviation)));
    node->add(new data_node("total_speed", f2s(total_speed)));
    node->add(new data_node("total_speed_deviation", f2s(total_speed_deviation)));
}


/**
 * @brief Resets data about the particle generator, to make it ready to
 * be used. Call this when copying from another generator.
 */
void particle_generator::reset() {
    if(interval_deviation == 0.0f) {
        emission_timer = emission_interval;
    } else {
        emission_timer =
            randomf(
                std::max(0.0f, emission_interval - interval_deviation),
                emission_interval + interval_deviation
            );
    }
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 * @param manager The manager of all particles.
 */
void particle_generator::tick(const float delta_t, particle_manager &manager) {
    if(follow_mob) {
        base_particle.pos = follow_mob->pos;
        base_particle.z = follow_mob->z;
    }
    emission_timer -= delta_t;
    if(emission_timer <= 0.0f) {
        emit(manager);
        if(interval_deviation == 0.0f) {
            emission_timer = emission_interval;
        } else {
            emission_timer =
                randomf(
                    std::max(0.0f, emission_interval - interval_deviation),
                    emission_interval + interval_deviation
                );
        }
    }
}


/**
 * @brief Constructs a new particle manager object.
 *
 * @param max_nr Maximum number of particles it can manage.
 */
particle_manager::particle_manager(const size_t &max_nr) :
    max_nr(max_nr) {
    
    if(max_nr == 0) return;
    particles = new particle[max_nr];
    clear();
}


/**
 * @brief Constructs a new particle manager object by copying data from another.
 *
 * @param pm2 Particle manager to copy from.
 */
particle_manager::particle_manager(const particle_manager &pm2) :
    count(pm2.count),
    max_nr(pm2.max_nr) {
    
    particles = new particle[max_nr];
    for(size_t p = 0; p < count; ++p) {
        this->particles[p] = pm2.particles[p];
    }
}


/**
 * @brief Copies a particle manager from another one.
 *
 * @param pm2 Particle manager to copy from.
 * @return The current object.
 */
particle_manager &particle_manager::operator =(
    const particle_manager &pm2
) {

    if(this != &pm2) {
        if(this->particles) {
            delete[] this->particles;
        }
        this->particles = nullptr;
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


/**
 * @brief Destroys the particle manager object.
 */
particle_manager::~particle_manager() {
    if(particles) delete[] particles;
}


/**
 * @brief Adds a new particle to the list. It will fail if there is no slot
 * where it can be added to.
 *
 * @param p Particle to add.
 */
void particle_manager::add(const particle &p) {
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


/**
 * @brief Clears the list.
 */
void particle_manager::clear() {
    for(size_t p = 0; p < max_nr; ++p) {
        particles[p].time = 0.0f;
    }
    count = 0;
}


/**
 * @brief Adds the particle pointers to the provided list of world components,
 * so that the particles can be drawn, after being Z-sorted.
 *
 * @param list The list to populate.
 * @param cam_tl Only draw particles below and to the right of this coordinate.
 * @param cam_br Only draw particles above and to the left of this coordinate.
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


/**
 * @brief Returns how many are in the list.
 *
 * @return The amount.
 */
size_t particle_manager::get_count() const {
    return count;
}


/**
 * @brief Removes a particle from the list.
 *
 * @param pos Position in the list.
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


/**
 * @brief Ticks time of all particles in the list by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
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
