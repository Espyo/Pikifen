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
 * @param diameter Diameter.
 * @param duration Total lifespan.
 * @param priority Lower priority particles will be removed in favor
 * of higher ones.
 */
particle::particle(
    const PARTICLE_TYPE type, const point &pos, const float z,
    const float initial_size, const float duration, const PARTICLE_PRIORITY priority,
    const ALLEGRO_COLOR initial_color
) :
    type(type),
    duration(duration),
    time(duration),
    pos(pos),
    size(initial_size),
    z(z),
    priority(priority),
    color(initial_color){
    
}


/**
 * @brief Draws this particle onto the world.
 */
void particle::draw() {
    ALLEGRO_COLOR target_color = color.get((duration - time) / duration);
    float target_size = size.get((duration - time) / duration);

    int old_op, old_source, old_dest;
    al_get_blender(&old_op, &old_source, &old_dest);

    switch (blend_type)
    {
    case PARTICLE_BLEND_TYPE_ADDITIVE:
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
        break;
    default:
        break;
    }

    switch(type) {
    case PARTICLE_TYPE_SQUARE: {
        al_draw_filled_rectangle(
            pos.x - target_size * 0.5,
            pos.y - target_size * 0.5,
            pos.x + target_size * 0.5,
            pos.y + target_size * 0.5,
            target_color
        );
        break;
        
    } case PARTICLE_TYPE_CIRCLE: {
        al_draw_filled_circle(
            pos.x, pos.y,
            target_size * 0.5,
            target_color
        );
        break;
        
    } case PARTICLE_TYPE_BITMAP: {
        draw_bitmap(
            bitmap, pos, point(target_size, -1),
            rotation, target_color
        );
        break;
        
    } case PARTICLE_TYPE_PIKMIN_SPIRIT: {
        draw_bitmap(
            bitmap, pos, point(target_size, -1),
            rotation, target_color
        );
        break;
        
    } case PARTICLE_TYPE_ENEMY_SPIRIT: {
        float s = sin((time / duration) * TAU / 2);
        draw_bitmap(
            bitmap,
            point(pos.x + s * 16, pos.y),
            point(target_size, -1), s * TAU / 2,
            change_alpha(
                target_color, fabs(s) * target_color.a * 255
            )
        );
        break;
        
    } case PARTICLE_TYPE_SMACK: {
        float r = time / duration;
        float s = target_size;
        float opacity = 255;
        if(r <= 0.5) s *= r * 2;
        else opacity *= (1 - r) * 2;
        
        draw_bitmap(
            bitmap, pos, point(s, s),
            0, change_alpha(target_color, opacity)
        );
        break;
        
    } case PARTICLE_TYPE_DING: {
        float r = time / duration;
        float s = target_size;
        float opacity = 255;
        if(r >= 0.5) s *= (1 - r) * 2;
        else opacity *= r * 2;
        
        draw_bitmap(
            bitmap, pos, point(s, s),
            0, change_alpha(target_color, opacity)
        );
        break;
    }
    }

    al_set_blender(old_op, old_source, old_dest);
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
    pos = rotate_point(pos, deg_to_rad(angular_speed) * delta_t);

    speed.x *= 1 - (delta_t* friction);
    speed.y *= 1 - (delta_t* friction);
    speed.y += delta_t* acceleration.y;
    speed.x += delta_t * acceleration.x;

    speed = rotate_point(speed, deg_to_rad(angular_speed) * delta_t);
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
    const string& new_file_name, data_node* node
) {
    if(new_file_name != file && bitmap) {
        game.bitmaps.free(file);
        bitmap = nullptr;
    }

    if(new_file_name.empty()) {
        file.clear();
        type = PARTICLE_TYPE_CIRCLE;
        return;
    }

    if (new_file_name != file || !bitmap) {
        bitmap = game.bitmaps.get(new_file_name, node, node != nullptr);
    }

    type = PARTICLE_TYPE_BITMAP;

    file = new_file_name;
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
    base_particle(base_particle){
    emission = particle_emission_struct(emission_interval, number);
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
            (int) emission.number +
            randomi((int) (0 - emission.number_deviation), (int)emission.number_deviation)
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
        new_p.acceleration.x +=
            randomf(-acceleration_deviation.x, acceleration_deviation.x);
        new_p.acceleration.y +=
            randomf(-acceleration_deviation.y, acceleration_deviation.y);
        new_p.rotation +=     
            randomf(-rotation_deviation, rotation_deviation);


        new_p.pos = base_p_pos;
        point offset = emission.get_emission_offset();
        if(follow_angle) {
            offset = rotate_point(offset, *follow_angle);
        }
        new_p.pos += offset;
        
        new_p.z = base_p_z;

        float s_dev = randomf(-size_deviation, size_deviation);
        for(size_t s = 0; s < new_p.size.keyframe_count(); s++) {
            auto kf = new_p.size.get_keyframe(s);
            new_p.size.set_keyframe_value(s, kf.second + s_dev);
        }

        float angle_to_use = angle + randomf(-angle_deviation, angle_deviation);
        if (follow_angle) angle_to_use += (*follow_angle);

        new_p.speed.x +=
            randomf(-speed_deviation.x, speed_deviation.x);
        new_p.speed.y +=
            randomf(-speed_deviation.y, speed_deviation.y);

        new_p.speed =
            rotate_point(
                new_p.speed, angle_to_use
            );
        float outwards_angle = get_angle(new_p.pos);

        if (new_p.pos == point(0, 0)) {
            outwards_angle = randomf(-180, 180);
        }

        new_p.speed += angle_to_coordinates(outwards_angle,
            outwards_speed + randomf(-outwards_speed_deviation, outwards_speed_deviation)
        );

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
    data_node* e_node = node->get_child_by_name("emission");
    reader_setter ers(e_node);
    
    float emission_interval_float = 0.0f;
    size_t number_int = 1;
    size_t shape_int = 0;

    ers.set("number", number_int);
    ers.set("interval", emission_interval_float);
    emission = particle_emission_struct(emission_interval_float, number_int);

    ers.set("interval_deviation", emission.interval_deviation);
    ers.set("number_deviation", emission.number_deviation);
    ers.set("shape", shape_int);

    switch (shape_int)
    {
    case PARTICLE_EMISSION_SHAPE_CIRCLE:
        ers.set("max_radius", emission.max_circular_radius);
        ers.set("min_radius", emission.min_circular_radius);
        ers.set("arc", emission.circular_arc);
        break;
    case PARTICLE_EMISSION_SHAPE_RECTANGLE:
        ers.set("max_offset", emission.max_rectangular_offset);
        ers.set("min_offset", emission.min_rectangular_offset);
        break;
    }

    emission.shape = (PARTICLE_EMISSION_SHAPE)shape_int;

    data_node * bitmap_node = nullptr;

    size_t blend_int = 0;

    prs.set("bitmap", base_particle.file, &bitmap_node);
    prs.set("rotation", base_particle.rotation);
    prs.set("duration", base_particle.duration);
    prs.set("friction", base_particle.friction);
    prs.set("acceleration", base_particle.acceleration);
    prs.set("velocity", base_particle.speed);
    prs.set("angular_speed", base_particle.angular_speed);
    prs.set("blend_type", blend_int);

    base_particle.blend_type = (PARTICLE_BLEND_TYPE)blend_int;
    base_particle.rotation = deg_to_rad(base_particle.rotation);

    data_node* color_node = p_node->get_child_by_name("color");
    keyframe_interpolator ki_c(COLOR_WHITE);
    for(size_t c = 0; c < color_node->get_nr_of_children(); c++) {
        data_node* c_node = color_node->get_child(c);
        ALLEGRO_COLOR color = s2c(c_node->value);

        if (c == 0) {
            ki_c.set_keyframe_value(0, color);
            ki_c.set_keyframe_time(0, s2f(c_node->name));
        }
        else {
            ki_c.add(s2f(c_node->name), color, EASE_METHOD_NONE);
        }
    }
    base_particle.color = ki_c;

    data_node* size_node = p_node->get_child_by_name("size");
    keyframe_interpolator ki_s(32.0f);
    for(size_t c = 0; c < size_node->get_nr_of_children(); c++) {
        data_node* s_node = size_node->get_child(c);
        float size = std::max(0.0f, (float)s2f(s_node->value));

        if (c == 0) {
            ki_s.set_keyframe_value(0, size);
            ki_s.set_keyframe_time(0, s2f(s_node->name));
        }
        else {
            ki_s.add(s2f(s_node->name), size, EASE_METHOD_NONE);
        }
    }
    base_particle.size = ki_s;
    
    if(bitmap_node) {
        if(load_resources) {
            base_particle.bitmap =
                game.bitmaps.get(
                    base_particle.file, bitmap_node
                );
        }
        base_particle.type = PARTICLE_TYPE_BITMAP;
    } else {
        base_particle.type = PARTICLE_TYPE_CIRCLE;
        base_particle.file = "";
    }
    
    base_particle.time = base_particle.duration;
    base_particle.priority = PARTICLE_PRIORITY_MEDIUM;

    grs.set("rotation_deviation", rotation_deviation);
    grs.set("duration_deviation", duration_deviation);
    grs.set("friction_deviation", friction_deviation);
    grs.set("gravity_deviation", acceleration_deviation);
    grs.set("size_deviation", size_deviation);
    grs.set("speed_deviation", speed_deviation);
    grs.set("angle", angle);
    grs.set("angle_deviation", angle_deviation);
    grs.set("outwards_speed", outwards_speed);
    grs.set("outwards_speed_deviation", outwards_speed_deviation);
    
    angle = deg_to_rad(angle);
    angle_deviation = deg_to_rad(angle_deviation);
    rotation_deviation = deg_to_rad(rotation_deviation);
    
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

    data_node* emission_particle_node = new data_node("emission", "");
    node->add(emission_particle_node);

    emission_particle_node->add(new data_node("number", i2s(emission.number)));
    emission_particle_node->add(new data_node("number_deviation", i2s(emission.number_deviation)));
    emission_particle_node->add(new data_node("interval", f2s(emission.interval)));
    emission_particle_node->add(new data_node("interval_deviation", f2s(emission.interval_deviation)));
    emission_particle_node->add(new data_node("shape", i2s(emission.shape)));

    switch (emission.shape)
    {
    case PARTICLE_EMISSION_SHAPE_CIRCLE:
        emission_particle_node->add(new data_node("max_radius", f2s(emission.max_circular_radius)));
        emission_particle_node->add(new data_node("min_radius", f2s(emission.min_circular_radius)));
        emission_particle_node->add(new data_node("arc", f2s(emission.circular_arc)));
        break;
    case PARTICLE_EMISSION_SHAPE_RECTANGLE:
        emission_particle_node->add(new data_node("max_offset", p2s(emission.max_rectangular_offset)));
        emission_particle_node->add(new data_node("min_offset", p2s(emission.min_rectangular_offset)));
        break;
    }
    
    data_node* base_particle_node = new data_node("base", "");
    node->add(base_particle_node);

    base_particle_node->add(new data_node("bitmap", base_particle.file));
    base_particle_node->add(new data_node("rotation", f2s(rad_to_deg(base_particle.rotation))));
    base_particle_node->add(new data_node("duration", f2s(base_particle.duration)));
    base_particle_node->add(new data_node("friction", f2s(base_particle.friction)));
    base_particle_node->add(new data_node("acceleration", p2s(base_particle.acceleration)));
    base_particle_node->add(new data_node("velocity", p2s(base_particle.speed)));
    base_particle_node->add(new data_node("blend_type", i2s(base_particle.blend_type)));
    base_particle_node->add(new data_node("angular_speed", f2s(base_particle.angular_speed)));

    data_node* color_node = new data_node("color", "");
    base_particle_node->add(color_node);

    for (size_t c = 0; c < base_particle.color.keyframe_count(); c++) {
        auto keyframe = base_particle.color.get_keyframe(c);
        color_node->add(new data_node(f2s(keyframe.first), c2s(keyframe.second)));
    }

    data_node* size_node = new data_node("size", "");
    base_particle_node->add(size_node);

    for (size_t c = 0; c < base_particle.size.keyframe_count(); c++) {
        auto keyframe = base_particle.size.get_keyframe(c);
        size_node->add(new data_node(f2s(keyframe.first), f2s(keyframe.second)));
    }

    node->add(new data_node("rotation_deviation", f2s(rad_to_deg(rotation_deviation))));
    node->add(new data_node("duration_deviation", f2s(duration_deviation)));
    node->add(new data_node("friction_deviation", f2s(friction_deviation)));
    node->add(new data_node("acceleration_deviation", p2s(acceleration_deviation)));
    node->add(new data_node("size_deviation", f2s(size_deviation)));
    node->add(new data_node("outwards_speed", f2s(outwards_speed)));
    node->add(new data_node("outwards_speed_deviation", f2s(outwards_speed_deviation)));
    node->add(new data_node("speed_deviation", p2s(speed_deviation)));
    node->add(new data_node("angle", f2s(rad_to_deg(angle))));
    node->add(new data_node("angle_deviation", f2s(rad_to_deg(angle_deviation))));
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
            randomf(
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
void particle_generator::tick(const float delta_t, particle_manager &manager) {
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
                randomf(
                    std::max(0.0f, emission.interval - emission.interval_deviation),
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
        float p_size = p_ptr->size.get((p_ptr->duration - p_ptr->time) / p_ptr->duration);
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


point particle_emission_struct::get_emission_offset() {
    switch (shape)
    {
    case PARTICLE_EMISSION_SHAPE_CIRCLE:
        {
            //Created using
            //https://stackoverflow.com/questions/30564015/how-to-generate-random-points-in-a-circular-distribution
            float r = min_circular_radius + (max_circular_radius - min_circular_radius) * sqrt(randomf(0, 1));

            
            float theta = randomf(
                -circular_arc / 2 + circular_arc_rotation, 
                circular_arc / 2 + circular_arc_rotation
            );

            return point(r * cos(theta), r * sin(theta));
        }
    case PARTICLE_EMISSION_SHAPE_RECTANGLE:
        {
            //Not perfectly uniform, but it works.
            int xSign = (randomi(0, 1) * 2) - 1;
            int ySign = (randomi(0, 1) * 2) - 1;

            float x = randomf(0, max_rectangular_offset.x);
            float y = x < min_rectangular_offset.x ? randomf(min_rectangular_offset.y, max_rectangular_offset.y) : randomf(0, max_rectangular_offset.y);

            return point(
                x * xSign,
                y * ySign
            );
        }
    default:
        return point(0, 0);
    }

}
