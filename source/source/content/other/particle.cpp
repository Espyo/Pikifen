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

#include "../../content/mob/mob.h"
#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/geometry_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Constructs a new particle object.
 *
 * @param type The type of particle.
 * @param pos Starting coordinates.
 * @param z Starting Z coordinate.
 * @param diameter Diameter.
 * @param duration Total lifespan.
 * @param priority Lower priority particles will be removed in favor
 * of higher ones.
 */
particle::particle(
    const point &pos, const float z,
    const float initial_size, const float duration, const PARTICLE_PRIORITY priority,
    const ALLEGRO_COLOR initial_color
) :
    duration(duration),
    time(duration),
    pos(pos),
    z(z),
    size(initial_size),
    color(initial_color),
    priority(priority) {
}


/**
 * @brief Draws this particle onto the world.
 */
void particle::draw() {
    float t = 1.0f - time / duration;
    ALLEGRO_COLOR final_color = color.get(t);
    float final_size = size.get(t);
    
    bool used_custom_blend = false;
    int old_op = 0, old_source = 0, old_dest = 0;
    
    switch(blend_type) {
    case PARTICLE_BLEND_TYPE_ADDITIVE: {
        al_get_blender(&old_op, &old_source, &old_dest);
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
        used_custom_blend = true;
        break;
    } default: {
        break;
    }
    }
    
    if(bitmap) {
        draw_bitmap(
            bitmap, pos, point(final_size, -1),
            bmp_angle, final_color
        );
    } else {
        al_draw_filled_circle(
            pos.x, pos.y,
            final_size * 0.5,
            final_color
        );
    }
    
    if(used_custom_blend) {
        al_set_blender(old_op, old_source, old_dest);
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
    
    float t = 1.0f - time / duration;
    
    point total_velocity = linear_speed.get(t);
    float outwards_angle = get_angle(pos - origin);
    
    if(pos == origin) {
        outwards_angle = game.rng.f(-180, 180);
    }
    total_velocity +=
        angle_to_coordinates(outwards_angle, outwards_speed.get(t));
        
    //Add 90 degrees to make the angle tangential.
    total_velocity +=
        angle_to_coordinates(outwards_angle + (TAU / 4), orbital_speed.get(t));
        
    //Accumulate and apply friction.
    total_velocity -= total_friction_applied;
    point new_friction = total_velocity * (delta_t* friction);
    total_friction_applied += new_friction;
    total_velocity -= new_friction;
    
    pos += total_velocity * delta_t;
    
    if(bmp_angle_type == PARTICLE_ANGLE_TYPE_DIRECTION) {
        coordinates_to_angle(total_velocity, &bmp_angle, nullptr);
    }
}


/**
 * @brief Sets the bitmap, according to the given information.
 * This automatically manages bitmap un/loading and such.
 * If the file name string is empty, sets to a nullptr bitmap
 * (and still unloads the old bitmap).
 *
 * @param new_file_name File name of the bitmap.
 * @param node If not nullptr, this will be used to report an error with,
 * in case something happens.
 */
void particle::set_bitmap(
    const string &new_file_name, data_node* node
) {
    if(new_file_name != bmp_name && bitmap) {
        game.content.bitmaps.list.free(bmp_name);
        bitmap = nullptr;
    }
    
    if(new_file_name.empty()) {
        bmp_name.clear();
        return;
    }
    
    if(new_file_name != bmp_name || !bitmap) {
        bitmap =
            game.content.bitmaps.list.get(
                new_file_name, node, node != nullptr
            );
    }
    
    bmp_name = new_file_name;
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
    base_particle(base_particle) {
    emission = particle_emission_struct(emission_interval, number);
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
            (int) emission.number +
            game.rng.i(
                (int) (0 - emission.number_deviation),
                (int) emission.number_deviation
            )
        );
        
    for(size_t p = 0; p < final_nr; p++) {
        particle new_p = base_particle;
        
        new_p.duration =
            std::max(
                0.0f,
                new_p.duration +
                game.rng.f(-duration_deviation, duration_deviation)
            );
        new_p.time = new_p.duration;
        
        if(new_p.bmp_angle_type == PARTICLE_ANGLE_TYPE_FIXED) {
            new_p.bmp_angle +=
                game.rng.f(-bmp_angle_deviation, bmp_angle_deviation);
        }
        new_p.friction +=
            game.rng.f(-friction_deviation, friction_deviation);
            
        new_p.pos = base_p_pos;
        new_p.origin = base_p_pos;
        point offset = emission.get_emission_offset(p / (float) final_nr);
        if(follow_angle) {
            offset = rotate_point(offset, *follow_angle);
        }
        new_p.pos += offset;
        
        new_p.z = base_p_z;
        
        float s_dev = game.rng.f(-size_deviation, size_deviation);
        for(size_t s = 0; s < new_p.size.keyframe_count(); s++) {
            auto kf = new_p.size.get_keyframe(s);
            new_p.size.set_keyframe_value((int) s, kf.second + s_dev);
        }
        
        float angle_to_use =
            game.rng.f(
                -linear_speed_angle_deviation,
                linear_speed_angle_deviation
            );
        if(follow_angle && !angles_are_absolute) {
            angle_to_use += (*follow_angle);
        }
        
        float v_dev_x =
            game.rng.f(-linear_speed_deviation.x, linear_speed_deviation.x);
        float v_dev_y =
            game.rng.f(-linear_speed_deviation.y, linear_speed_deviation.y);
        for(size_t s = 0; s < new_p.linear_speed.keyframe_count(); s++) {
            auto kf = new_p.linear_speed.get_keyframe(s);
            point base = kf.second;
            point result = point(base.x + v_dev_x, base.y + v_dev_y);
            result =
                rotate_point(
                    result, angle_to_use
                );
            new_p.linear_speed.set_keyframe_value((int) s, result);
        }
        
        float out_dev =
            game.rng.f(-outwards_speed_deviation, outwards_speed_deviation);
        for(size_t s = 0; s < new_p.outwards_speed.keyframe_count(); s++) {
            auto kf = new_p.outwards_speed.get_keyframe(s);
            new_p.outwards_speed.set_keyframe_value((int) s, kf.second + out_dev);
        }
        
        float orb_dev =
            game.rng.f(-orbital_speed_deviation, orbital_speed_deviation);
        for(size_t s = 0; s < new_p.orbital_speed.keyframe_count(); s++) {
            auto kf = new_p.orbital_speed.get_keyframe(s);
            new_p.orbital_speed.set_keyframe_value((int) s, kf.second + orb_dev);
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
    data_node* node, CONTENT_LOAD_LEVEL level
) {
    //Content metadata.
    load_metadata_from_data_node(node);
    
    //Standard data.
    reader_setter grs(node);
    data_node* base_particle_node = node->get_child_by_name("base");
    reader_setter prs(base_particle_node);
    data_node* emission_node = node->get_child_by_name("emission");
    reader_setter ers(emission_node);
    
    float emission_interval_float = 0.0f;
    size_t number_int = 1;
    size_t shape_int = 0;
    
    ers.set("number", number_int);
    ers.set("interval", emission_interval_float);
    emission = particle_emission_struct(emission_interval_float, number_int);
    
    ers.set("interval_deviation", emission.interval_deviation);
    ers.set("number_deviation", emission.number_deviation);
    ers.set("shape", shape_int);
    
    switch (shape_int) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        ers.set("circle_outer_dist", emission.circle_outer_dist);
        ers.set("circle_inner_dist", emission.circle_inner_dist);
        ers.set("circle_arc", emission.circle_arc);
        ers.set("circle_arc_rot", emission.circle_arc_rot);
        ers.set("evenly_spread", emission.evenly_spread);
        break;
    } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
        ers.set("rect_outer_dist", emission.rect_outer_dist);
        ers.set("rect_inner_dist", emission.rect_inner_dist);
        break;
    }
    }
    
    emission.shape = (PARTICLE_EMISSION_SHAPE) shape_int;
    
    data_node* bitmap_node = nullptr;
    size_t angle_type_int = 0;
    size_t blend_int = 0;
    
    prs.set("bitmap", base_particle.bmp_name, &bitmap_node);
    prs.set("bitmap_angle", base_particle.bmp_angle);
    prs.set("bitmap_angle_type", angle_type_int);
    prs.set("duration", base_particle.duration);
    prs.set("friction", base_particle.friction);
    prs.set("blend_type", blend_int);
    
    base_particle.bmp_angle_type = (PARTICLE_ANGLE_TYPE) angle_type_int;
    base_particle.blend_type = (PARTICLE_BLEND_TYPE) blend_int;
    base_particle.bmp_angle = deg_to_rad(base_particle.bmp_angle);
    
    base_particle.color.load_from_data_node(
        base_particle_node->get_child_by_name("color")
    );
    base_particle.size.load_from_data_node(
        base_particle_node->get_child_by_name("size")
    );
    base_particle.linear_speed.load_from_data_node(
        base_particle_node->get_child_by_name("linear_speed")
    );
    base_particle.outwards_speed.load_from_data_node(
        base_particle_node->get_child_by_name("outwards_speed")
    );
    base_particle.orbital_speed.load_from_data_node(
        base_particle_node->get_child_by_name("orbital_speed")
    );
    
    if(bitmap_node) {
        if(level >= CONTENT_LOAD_LEVEL_FULL) {
            base_particle.bitmap =
                game.content.bitmaps.list.get(
                    base_particle.bmp_name, bitmap_node
                );
        }
    } else {
        base_particle.bmp_name = "";
        base_particle.bitmap = nullptr;
    }
    
    base_particle.time = base_particle.duration;
    base_particle.priority = PARTICLE_PRIORITY_MEDIUM;
    
    grs.set("bitmap_angle_deviation", bmp_angle_deviation);
    grs.set("duration_deviation", duration_deviation);
    grs.set("friction_deviation", friction_deviation);
    grs.set("size_deviation", size_deviation);
    grs.set("angle_deviation", linear_speed_angle_deviation);
    grs.set("linear_speed_deviation", linear_speed_deviation);
    grs.set("orbital_speed_deviation", orbital_speed_deviation);
    grs.set("outwards_speed_deviation", outwards_speed_deviation);
    grs.set("angles_are_absolute", angles_are_absolute);
    
    bmp_angle_deviation = deg_to_rad(bmp_angle_deviation);
    linear_speed_angle_deviation = deg_to_rad(linear_speed_angle_deviation);
    
    id =
        (MOB_PARTICLE_GENERATOR_ID) (
            MOB_PARTICLE_GENERATOR_ID_STATUS +
            game.content.custom_particle_gen.list.size()
        );
}


/**
 * @brief Saves particle generator data to a data node.
 *
 * @param node Node to save to.
 */
void particle_generator::save_to_data_node(data_node* node) {
    //Content metadata.
    save_metadata_to_data_node(node);
    
    data_node* emission_particle_node = new data_node("emission", "");
    node->add(emission_particle_node);
    
    emission_particle_node->add(
        new data_node("number", i2s(emission.number))
    );
    emission_particle_node->add(
        new data_node("number_deviation", i2s(emission.number_deviation))
    );
    emission_particle_node->add(
        new data_node("interval", f2s(emission.interval))
    );
    emission_particle_node->add(
        new data_node("interval_deviation", f2s(emission.interval_deviation))
    );
    emission_particle_node->add(
        new data_node("shape", i2s(emission.shape))
    );
    
    switch (emission.shape) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        emission_particle_node->add(
            new data_node("circle_outer_dist", f2s(emission.circle_outer_dist))
        );
        emission_particle_node->add(
            new data_node("circle_inner_dist", f2s(emission.circle_inner_dist))
        );
        emission_particle_node->add(
            new data_node("circle_arc", f2s(emission.circle_arc))
        );
        emission_particle_node->add(
            new data_node("circle_arc_rot", f2s(emission.circle_arc_rot))
        );
        emission_particle_node->add(
            new data_node("evenly_spread", f2s(emission.evenly_spread))
        );
        break;
    } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
        emission_particle_node->add(
            new data_node("rect_outer_dist", p2s(emission.rect_outer_dist))
        );
        emission_particle_node->add(
            new data_node("rect_inner_dist", p2s(emission.rect_inner_dist))
        );
        break;
    }
    }
    
    data_node* base_particle_node = new data_node("base", "");
    node->add(base_particle_node);
    
    base_particle_node->add(
        new data_node("bitmap", base_particle.bmp_name)
    );
    base_particle_node->add(
        new data_node("bitmap_angle", f2s(rad_to_deg(base_particle.bmp_angle)))
    );
    base_particle_node->add(
        new data_node("bitmap_angle_type", i2s(base_particle.bmp_angle_type))
    );
    base_particle_node->add(
        new data_node("duration", f2s(base_particle.duration))
    );
    base_particle_node->add(
        new data_node("friction", f2s(base_particle.friction))
    );
    base_particle_node->add(
        new data_node("blend_type", i2s(base_particle.blend_type))
    );
    
    data_node* color_node = new data_node("color", "");
    base_particle_node->add(color_node);
    
    for(size_t c = 0; c < base_particle.color.keyframe_count(); c++) {
        auto keyframe = base_particle.color.get_keyframe(c);
        color_node->add(
            new data_node(f2s(keyframe.first), c2s(keyframe.second))
        );
    }
    
    data_node* size_node = new data_node("size", "");
    base_particle_node->add(size_node);
    
    for(size_t c = 0; c < base_particle.size.keyframe_count(); c++) {
        auto keyframe = base_particle.size.get_keyframe(c);
        size_node->add(
            new data_node(f2s(keyframe.first), f2s(keyframe.second))
        );
    }
    
    data_node* lin_speed_node = new data_node("linear_speed", "");
    base_particle_node->add(lin_speed_node);
    
    for(size_t c = 0; c < base_particle.linear_speed.keyframe_count(); c++) {
        auto keyframe = base_particle.linear_speed.get_keyframe(c);
        lin_speed_node->add(
            new data_node(f2s(keyframe.first), p2s(keyframe.second))
        );
    }
    
    data_node* out_speed_node = new data_node("outwards_speed", "");
    base_particle_node->add(out_speed_node);
    
    for(size_t c = 0; c < base_particle.outwards_speed.keyframe_count(); c++) {
        auto keyframe = base_particle.outwards_speed.get_keyframe(c);
        out_speed_node->add(
            new data_node(f2s(keyframe.first), f2s(keyframe.second))
        );
    }
    
    data_node* orb_speed_node = new data_node("orbital_speed", "");
    base_particle_node->add(orb_speed_node);
    
    for(size_t c = 0; c < base_particle.orbital_speed.keyframe_count(); c++) {
        auto keyframe = base_particle.orbital_speed.get_keyframe(c);
        orb_speed_node->add(
            new data_node(f2s(keyframe.first), f2s(keyframe.second))
        );
    }
    
    node->add(
        new data_node(
            "bitmap_angle_deviation",
            f2s(rad_to_deg(bmp_angle_deviation))
        )
    );
    node->add(
        new data_node("duration_deviation", f2s(duration_deviation))
    );
    node->add(
        new data_node("friction_deviation", f2s(friction_deviation))
    );
    node->add(
        new data_node("size_deviation", f2s(size_deviation))
    );
    node->add(
        new data_node(
            "orbital_speed_deviation",
            f2s(orbital_speed_deviation)
        )
    );
    node->add(
        new data_node(
            "outwards_speed_deviation",
            f2s(outwards_speed_deviation)
        )
    );
    node->add(
        new data_node(
            "angle_deviation",
            f2s(rad_to_deg(linear_speed_angle_deviation))
        )
    );
    node->add(
        new data_node(
            "linear_speed_deviation",
            p2s(linear_speed_deviation)
        )
    );
    node->add(
        new data_node(
            "angles_are_absolute",
            b2s(angles_are_absolute)
        )
    );
}


/**
 * @brief Resets data about the particle generator, to make it ready to
 * be used. Call this when copying from another generator.
 */
void particle_generator::reset() {
    if(emission.interval_deviation == 0.0f) {
        emission_timer = emission.interval;
    } else {
        emission_timer =
            game.rng.f(
                std::max(0.0f, emission.interval - emission.interval_deviation),
                emission.interval + emission.interval_deviation
            );
    }
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 * @param manager The manager of all particles.
 */
void particle_generator::tick(float delta_t, particle_manager &manager) {
    if(follow_mob) {
        base_particle.pos = follow_mob->pos;
        base_particle.z = follow_mob->z;
    }
    emission_timer -= delta_t;
    if(emission_timer <= 0.0f) {
        emit(manager);
        if(emission.interval_deviation == 0.0f) {
            emission_timer = emission.interval;
        } else {
            emission_timer =
                game.rng.f(
                    std::max(
                        0.0f, emission.interval - emission.interval_deviation
                    ),
                    emission.interval + emission.interval_deviation
                );
        }
    }
}


/**
 * @brief Constructs a new particle manager object.
 *
 * @param max_nr Maximum number of particles it can manage.
 */
particle_manager::particle_manager(size_t max_nr) :
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
    for(size_t p = 0; p < count; p++) {
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
        for(size_t p = 0; p < count; p++) {
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
        for(size_t i = 0; i < max_nr; i++) {
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
    for(size_t p = 0; p < max_nr; p++) {
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
    for(size_t c = 0; c < count; c++) {
    
        particle* p_ptr = &particles[c];
        float p_size =
            p_ptr->size.get((p_ptr->duration - p_ptr->time) / p_ptr->duration);
        if(
            cam_tl != cam_br &&
            !rectangles_intersect(
                p_ptr->pos - p_size, p_ptr->pos + p_size,
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
void particle_manager::remove(size_t pos) {
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
void particle_manager::tick_all(float delta_t) {
    for(size_t c = 0; c < count;) {
        particles[c].tick(delta_t);
        if(particles[c].time == 0.0f) {
            remove(c);
        } else {
            c++;
        }
    }
}

/**
 * @brief Constructs a new particle em object.
 *
 * @param emission_interval Interval to spawn a new set of particles in,
 * in seconds. 0 means it spawns only one set and that's it.
 * @param base_particle All particles created will be based on this one.
 * Their properties will deviate randomly based on the
 * deviation members of the particle generator object.
 * @param number Number of particles to spawn.
 * This number is also deviated by number_deviation.
 */
particle_emission_struct::particle_emission_struct(
    const float emission_interval, const size_t num
) {
    number = num;
    interval = emission_interval;
}


/**
 * @brief Returns a randomly-picked offset for a new particle.
 *
 * @param number_ratio Ratio of which number particle this is in the emission,
 * over the total particles to emit in this emission.
 * @return The offset.
 */
point particle_emission_struct::get_emission_offset(float number_ratio) {
    switch (shape) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        if(evenly_spread) {
            return
                get_ratio_point_in_ring(
                    circle_inner_dist, circle_outer_dist,
                    circle_arc, circle_arc_rot, number_ratio
                );
        } else {
            return
                get_random_point_in_ring(
                    circle_inner_dist, circle_outer_dist,
                    circle_arc, circle_arc_rot, &game.rng.seed
                );
        }
        break;
        
    } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
        return
            get_random_point_in_rectangular_ring(
                rect_inner_dist, rect_outer_dist, &game.rng.seed
            );
        break;
        
    } default: {
        return point();
        break;
        
    }
    }
    
}
