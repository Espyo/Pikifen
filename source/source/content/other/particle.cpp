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
Particle::Particle(
    const Point &pos, const float z,
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
void Particle::draw() {
    float t = 1.0f - time / duration;
    ALLEGRO_COLOR final_color = color.get(t);
    float final_size = size.get(t);
    if(final_size <= 0.0f) return;
    
    bool used_custom_blend = false;
    int old_op = 0, old_source = 0, old_dest = 0;
    
    switch(blendType) {
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
        drawBitmap(
            bitmap, pos, Point(final_size, -1),
            bmpAngle, final_color
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
void Particle::tick(const float delta_t) {
    time -= delta_t;
    
    if(time <= 0.0f) {
        time = 0.0f;
        return;
    }
    
    float t = 1.0f - time / duration;
    
    Point total_velocity = linearSpeed.get(t);
    float outwards_angle = getAngle(pos - origin);
    
    if(pos == origin) {
        outwards_angle = game.rng.f(-180, 180);
    }
    total_velocity +=
        angleToCoordinates(outwards_angle, outwardsSpeed.get(t));
        
    //Add 90 degrees to make the angle tangential.
    total_velocity +=
        angleToCoordinates(outwards_angle + (TAU / 4), orbitalSpeed.get(t));
        
    //Accumulate and apply friction.
    total_velocity -= totalFrictionApplied;
    Point new_friction = total_velocity * (delta_t* friction);
    totalFrictionApplied += new_friction;
    total_velocity -= new_friction;
    
    pos += total_velocity * delta_t;
    
    if(bmpAngleType == PARTICLE_ANGLE_TYPE_DIRECTION) {
        coordinatesToAngle(total_velocity, &bmpAngle, nullptr);
    }
}


/**
 * @brief Sets the bitmap, according to the given information.
 * This automatically manages bitmap un/loading and such.
 * If the file name string is empty, sets to a nullptr bitmap
 * (and still unloads the old bitmap).
 *
 * @param new_bmp_name Internal name of the bitmap.
 * @param node If not nullptr, this will be used to report an error with,
 * in case something happens.
 */
void Particle::setBitmap(
    const string &new_bmp_name, DataNode* node
) {
    if(new_bmp_name != bmpName && bitmap) {
        game.content.bitmaps.list.free(bmpName);
        bitmap = nullptr;
    }
    
    if(new_bmp_name.empty()) {
        bmpName.clear();
        return;
    }
    
    if(new_bmp_name != bmpName || !bitmap) {
        bitmap =
            game.content.bitmaps.list.get(
                new_bmp_name, node, node != nullptr
            );
    }
    
    bmpName = new_bmp_name;
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
 * This number is also deviated by numberDeviation.
 */
ParticleGenerator::ParticleGenerator(
    const float emission_interval,
    const Particle &base_particle, const size_t number
) :
    baseParticle(base_particle) {
    emission = ParticleEmission(emission_interval, number);
}


/**
 * @brief Emits the particles, regardless of the timer.
 *
 * @param manager The particle manager to place these particles on.
 */
void ParticleGenerator::emit(ParticleManager &manager) {
    Point base_p_pos = baseParticle.pos;
    float base_p_z = baseParticle.z;
    Point offs = followPosOffset;
    if(followAngle) {
        offs = rotatePoint(offs, *followAngle);
    }
    base_p_pos += offs;
    base_p_z += followZOffset;
    
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
                (int) (0 - emission.numberDeviation),
                (int) emission.numberDeviation
            )
        );
        
    for(size_t p = 0; p < final_nr; p++) {
        Particle new_p = baseParticle;
        
        new_p.duration =
            std::max(
                0.0f,
                new_p.duration +
                game.rng.f(-durationDeviation, durationDeviation)
            );
        new_p.time = new_p.duration;
        
        if(new_p.bmpAngleType == PARTICLE_ANGLE_TYPE_FIXED) {
            new_p.bmpAngle +=
                game.rng.f(-bmpAngleDeviation, bmpAngleDeviation);
        }
        new_p.friction +=
            game.rng.f(-frictionDeviation, frictionDeviation);
            
        new_p.pos = base_p_pos;
        new_p.origin = base_p_pos;
        Point offset = emission.getEmissionOffset(p / (float) final_nr);
        if(followAngle) {
            offset = rotatePoint(offset, *followAngle);
        }
        new_p.pos += offset;
        
        new_p.z = base_p_z;
        
        float s_dev = game.rng.f(-sizeDeviation, sizeDeviation);
        for(size_t s = 0; s < new_p.size.getKeyframeCount(); s++) {
            auto kf = new_p.size.getKeyframe(s);
            new_p.size.setKeyframeValue((int) s, kf.second + s_dev);
        }
        
        float angle_to_use =
            game.rng.f(
                -linearSpeedAngleDeviation,
                linearSpeedAngleDeviation
            );
        if(followAngle && !anglesAreAbsolute) {
            angle_to_use += (*followAngle);
        }
        
        float v_dev_x =
            game.rng.f(-linearSpeedDeviation.x, linearSpeedDeviation.x);
        float v_dev_y =
            game.rng.f(-linearSpeedDeviation.y, linearSpeedDeviation.y);
        for(size_t s = 0; s < new_p.linearSpeed.getKeyframeCount(); s++) {
            auto kf = new_p.linearSpeed.getKeyframe(s);
            Point base = kf.second;
            Point result = Point(base.x + v_dev_x, base.y + v_dev_y);
            result =
                rotatePoint(
                    result, angle_to_use
                );
            new_p.linearSpeed.setKeyframeValue((int) s, result);
        }
        
        float out_dev =
            game.rng.f(-outwardsSpeedDeviation, outwardsSpeedDeviation);
        for(size_t s = 0; s < new_p.outwardsSpeed.getKeyframeCount(); s++) {
            auto kf = new_p.outwardsSpeed.getKeyframe(s);
            new_p.outwardsSpeed.setKeyframeValue((int) s, kf.second + out_dev);
        }
        
        float orb_dev =
            game.rng.f(-orbitalSpeedDeviation, orbitalSpeedDeviation);
        for(size_t s = 0; s < new_p.orbitalSpeed.getKeyframeCount(); s++) {
            auto kf = new_p.orbitalSpeed.getKeyframe(s);
            new_p.orbitalSpeed.setKeyframeValue((int) s, kf.second + orb_dev);
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
void ParticleGenerator::loadFromDataNode(
    DataNode* node, CONTENT_LOAD_LEVEL level
) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter grs(node);
    DataNode* base_particle_node = node->getChildByName("base");
    ReaderSetter prs(base_particle_node);
    DataNode* emission_node = node->getChildByName("emission");
    ReaderSetter ers(emission_node);
    
    float emission_interval_float = 0.0f;
    size_t number_int = 1;
    size_t shape_int = 0;
    
    ers.set("number", number_int);
    ers.set("interval", emission_interval_float);
    emission = ParticleEmission(emission_interval_float, number_int);
    
    ers.set("interval_deviation", emission.intervalDeviation);
    ers.set("number_deviation", emission.numberDeviation);
    ers.set("shape", shape_int);
    
    switch (shape_int) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        ers.set("circle_outer_dist", emission.circleOuterDist);
        ers.set("circle_inner_dist", emission.circleInnerDist);
        ers.set("circle_arc", emission.circleArc);
        ers.set("circle_arc_rot", emission.circleArcRot);
        ers.set("evenly_spread", emission.evenlySpread);
        break;
    } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
        ers.set("rect_outer_dist", emission.rectOuterDist);
        ers.set("rect_inner_dist", emission.rectInnerDist);
        break;
    }
    }
    
    emission.shape = (PARTICLE_EMISSION_SHAPE) shape_int;
    
    DataNode* bitmap_node = nullptr;
    size_t angle_type_int = 0;
    size_t blend_int = 0;
    
    prs.set("bitmap", baseParticle.bmpName, &bitmap_node);
    prs.set("bitmap_angle", baseParticle.bmpAngle);
    prs.set("bitmap_angle_type", angle_type_int);
    prs.set("duration", baseParticle.duration);
    prs.set("friction", baseParticle.friction);
    prs.set("blend_type", blend_int);
    
    baseParticle.bmpAngleType = (PARTICLE_ANGLE_TYPE) angle_type_int;
    baseParticle.blendType = (PARTICLE_BLEND_TYPE) blend_int;
    baseParticle.bmpAngle = degToRad(baseParticle.bmpAngle);
    
    baseParticle.color.loadFromDataNode(
        base_particle_node->getChildByName("color")
    );
    baseParticle.size.loadFromDataNode(
        base_particle_node->getChildByName("size")
    );
    baseParticle.linearSpeed.loadFromDataNode(
        base_particle_node->getChildByName("linear_speed")
    );
    baseParticle.outwardsSpeed.loadFromDataNode(
        base_particle_node->getChildByName("outwards_speed")
    );
    baseParticle.orbitalSpeed.loadFromDataNode(
        base_particle_node->getChildByName("orbital_speed")
    );
    
    if(bitmap_node) {
        if(level >= CONTENT_LOAD_LEVEL_FULL) {
            baseParticle.bitmap =
                game.content.bitmaps.list.get(
                    baseParticle.bmpName, bitmap_node
                );
        }
    } else {
        baseParticle.bmpName = "";
        baseParticle.bitmap = nullptr;
    }
    
    baseParticle.time = baseParticle.duration;
    baseParticle.priority = PARTICLE_PRIORITY_MEDIUM;
    
    grs.set("bitmap_angle_deviation", bmpAngleDeviation);
    grs.set("duration_deviation", durationDeviation);
    grs.set("friction_deviation", frictionDeviation);
    grs.set("size_deviation", sizeDeviation);
    grs.set("angle_deviation", linearSpeedAngleDeviation);
    grs.set("linear_speed_deviation", linearSpeedDeviation);
    grs.set("orbital_speed_deviation", orbitalSpeedDeviation);
    grs.set("outwards_speed_deviation", outwardsSpeedDeviation);
    grs.set("angles_are_absolute", anglesAreAbsolute);
    
    bmpAngleDeviation = degToRad(bmpAngleDeviation);
    linearSpeedAngleDeviation = degToRad(linearSpeedAngleDeviation);
    
    id =
        (MOB_PARTICLE_GENERATOR_ID) (
            MOB_PARTICLE_GENERATOR_ID_STATUS +
            game.content.particleGens.list.size()
        );
}


/**
 * @brief Saves particle generator data to a data node.
 *
 * @param node Node to save to.
 */
void ParticleGenerator::saveToDataNode(DataNode* node) {
    //Content metadata.
    saveMetadataToDataNode(node);
    
    //Emission.
    DataNode* emission_node = node->addNew("emission");
    GetterWriter egw(emission_node);
    
    egw.get("number", emission.number);
    egw.get("number_deviation", emission.numberDeviation);
    egw.get("interval", emission.interval);
    egw.get("interval_deviation", emission.intervalDeviation);
    egw.get("shape", emission.shape);
    
    switch (emission.shape) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        egw.get("circle_outer_dist", emission.circleOuterDist);
        egw.get("circle_inner_dist", emission.circleInnerDist);
        egw.get("circle_arc", emission.circleArc);
        egw.get("circle_arc_rot", emission.circleArcRot);
        egw.get("evenly_spread", emission.evenlySpread);
        break;
    } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
        egw.get("rect_outer_dist", emission.rectOuterDist);
        egw.get("rect_inner_dist", emission.rectInnerDist);
        break;
    }
    }
    
    //Base particle.
    DataNode* base_particle_node = node->addNew("base");
    GetterWriter pgw(base_particle_node);
    
    pgw.get("bitmap", baseParticle.bmpName);
    pgw.get("bitmap_angle", baseParticle.bmpAngle);
    pgw.get("bitmap_angle_type", baseParticle.bmpAngleType);
    pgw.get("duration", baseParticle.duration);
    pgw.get("friction", baseParticle.friction);
    pgw.get("blend_type", baseParticle.blendType);
    
    DataNode* color_node = base_particle_node->addNew("color");
    GetterWriter pcgw(color_node);
    
    for(size_t c = 0; c < baseParticle.color.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.color.getKeyframe(c);
        pcgw.get(f2s(keyframe.first), keyframe.second);
    }
    
    DataNode* size_node = base_particle_node->addNew("size");
    GetterWriter psgw(size_node);
    
    for(size_t c = 0; c < baseParticle.size.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.size.getKeyframe(c);
        psgw.get(f2s(keyframe.first), keyframe.second);
    }
    
    DataNode* lin_speed_node = base_particle_node->addNew("linear_speed");
    GetterWriter plsgw(lin_speed_node);
    
    for(size_t c = 0; c < baseParticle.linearSpeed.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.linearSpeed.getKeyframe(c);
        plsgw.get(f2s(keyframe.first), keyframe.second);
    }
    
    DataNode* out_speed_node = base_particle_node->addNew("outwards_speed");
    GetterWriter posgw(out_speed_node);
    
    for(size_t c = 0; c < baseParticle.outwardsSpeed.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.outwardsSpeed.getKeyframe(c);
        posgw.get(f2s(keyframe.first), keyframe.second);
    }
    
    DataNode* orb_speed_node = base_particle_node->addNew("orbital_speed");
    GetterWriter porsgw(orb_speed_node);
    
    for(size_t c = 0; c < baseParticle.orbitalSpeed.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.orbitalSpeed.getKeyframe(c);
        porsgw.get(f2s(keyframe.first), keyframe.second);
    }
    
    //Generator.
    GetterWriter ggw(node);
    
    ggw.get("bitmap_angle_deviation", radToDeg(bmpAngleDeviation));
    ggw.get("duration_deviation", durationDeviation);
    ggw.get("friction_deviation", frictionDeviation);
    ggw.get("size_deviation", sizeDeviation);
    ggw.get("orbital_speed_deviation", orbitalSpeedDeviation);
    ggw.get("outwards_speed_deviation", outwardsSpeedDeviation);
    ggw.get("angle_deviation", radToDeg(linearSpeedAngleDeviation));
    ggw.get("linear_speed_deviation", linearSpeedDeviation);
    ggw.get("angles_are_absolute", anglesAreAbsolute);
}


/**
 * @brief Resets timer information about the particle generator.
 * Call this when copying from another generator.
 */
void ParticleGenerator::restartTimer() {
    if(emission.intervalDeviation == 0.0f) {
        emissionTimer = emission.interval;
    } else {
        emissionTimer =
            game.rng.f(
                std::max(0.0f, emission.interval - emission.intervalDeviation),
                emission.interval + emission.intervalDeviation
            );
    }
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 * @param manager The manager of all particles.
 */
void ParticleGenerator::tick(float delta_t, ParticleManager &manager) {
    if(followMob) {
        baseParticle.pos = followMob->pos;
        baseParticle.z = followMob->z;
    }
    emissionTimer -= delta_t;
    if(emissionTimer <= 0.0f) {
        emit(manager);
        if(emission.intervalDeviation == 0.0f) {
            emissionTimer = emission.interval;
        } else {
            emissionTimer =
                game.rng.f(
                    std::max(
                        0.0f, emission.interval - emission.intervalDeviation
                    ),
                    emission.interval + emission.intervalDeviation
                );
        }
    }
}


/**
 * @brief Constructs a new particle manager object.
 *
 * @param max_nr Maximum number of particles it can manage.
 */
ParticleManager::ParticleManager(size_t max_nr) :
    maxNr(max_nr) {
    
    if(max_nr == 0) return;
    particles = new Particle[max_nr];
    clear();
}


/**
 * @brief Constructs a new particle manager object by copying data from another.
 *
 * @param pm2 Particle manager to copy from.
 */
ParticleManager::ParticleManager(const ParticleManager &pm2) :
    count(pm2.count),
    maxNr(pm2.maxNr) {
    
    particles = new Particle[maxNr];
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
ParticleManager &ParticleManager::operator =(
    const ParticleManager &pm2
) {

    if(this != &pm2) {
        if(this->particles) {
            delete[] this->particles;
        }
        this->particles = nullptr;
        maxNr = pm2.maxNr;
        count = pm2.count;
        if(maxNr == 0) return *this;
        this->particles = new Particle[maxNr];
        for(size_t p = 0; p < count; p++) {
            this->particles[p] = pm2.particles[p];
        }
    }
    
    return *this;
}


/**
 * @brief Destroys the particle manager object.
 */
ParticleManager::~ParticleManager() {
    if(particles) delete[] particles;
}


/**
 * @brief Adds a new particle to the list. It will fail if there is no slot
 * where it can be added to.
 *
 * @param p Particle to add.
 */
void ParticleManager::add(const Particle &p) {
    if(maxNr == 0) return;
    
    //The first "count" particles are alive. Add the new one after.
    //...Unless count already equals the max. That means the list is full.
    //Let's try to dump a particle with lower priority.
    //Starting from 0 will (hopefully) give us the oldest one first.
    bool success = true;
    if(count == maxNr) {
        success = false;
        for(size_t i = 0; i < maxNr; i++) {
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
void ParticleManager::clear() {
    for(size_t p = 0; p < maxNr; p++) {
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
void ParticleManager::fillComponentList(
    vector<WorldComponent> &list,
    const Point &cam_tl, const Point &cam_br
) {
    for(size_t c = 0; c < count; c++) {
    
        Particle* p_ptr = &particles[c];
        float p_size =
            p_ptr->size.get((p_ptr->duration - p_ptr->time) / p_ptr->duration);
        if(
            cam_tl != cam_br &&
            !rectanglesIntersect(
                p_ptr->pos - p_size, p_ptr->pos + p_size,
                cam_tl, cam_br
            )
        ) {
            //Off-camera.
            continue;
        }
        
        WorldComponent wc;
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
size_t ParticleManager::getCount() const {
    return count;
}


/**
 * @brief Removes a particle from the list.
 *
 * @param pos Position in the list.
 */
void ParticleManager::remove(size_t pos) {
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
void ParticleManager::tickAll(float delta_t) {
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
 * This number is also deviated by numberDeviation.
 */
ParticleEmission::ParticleEmission(
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
Point ParticleEmission::getEmissionOffset(float number_ratio) {
    switch (shape) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        if(evenlySpread) {
            return
                getRatioPointInRing(
                    circleInnerDist, circleOuterDist,
                    circleArc, circleArcRot, number_ratio
                );
        } else {
            return
                getRandomPointInRing(
                    circleInnerDist, circleOuterDist,
                    circleArc, circleArcRot,
                    game.rng.f(0.0, 1.0f), game.rng.f(0.0, 1.0f)
                );
        }
        break;
        
    } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
        return
            getRandomPointInRectangularRing(
                rectInnerDist, rectOuterDist,
                game.rng.i(0.0, 1.0f), game.rng.f(0.0, 1.0f),
                game.rng.f(0.0, 1.0f), game.rng.f(0.0, 1.0f),
                game.rng.i(0.0, 1.0f)
            );
        break;
        
    } default: {
        return Point();
        break;
        
    }
    }
    
}
