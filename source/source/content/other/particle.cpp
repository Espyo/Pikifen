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
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/geometry_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Constructs a new particle object.
 *
 * @param pos Starting coordinates.
 * @param z Starting Z coordinate.
 * @param initialSize Initial size.
 * @param duration Total lifespan.
 * @param priority Lower priority particles will be removed in favor
 * of higher ones.
 * @param initialColor Initial color.
 */
Particle::Particle(
    const Point& pos, const float z,
    const float initialSize, const float duration,
    const PARTICLE_PRIORITY priority, const ALLEGRO_COLOR initialColor
) :
    duration(duration),
    time(duration),
    pos(pos),
    z(z),
    size(initialSize),
    color(initialColor),
    priority(priority) {
}


/**
 * @brief Draws this particle onto the world.
 */
void Particle::draw() {
    float t = 1.0f - time / duration;
    ALLEGRO_COLOR finalColor = color.get(t);
    float finalSize = size.get(t);
    if(finalSize <= 0.0f) return;
    
    bool usedCustomBlend = false;
    int oldOp = 0, oldSource = 0, oldDest = 0;
    
    switch(blendType) {
    case PARTICLE_BLEND_TYPE_ADDITIVE: {
        al_get_blender(&oldOp, &oldSource, &oldDest);
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
        usedCustomBlend = true;
        break;
    } default: {
        break;
    }
    }
    
    if(bitmap) {
        drawBitmap(
            bitmap, pos, Point(finalSize, -1),
            bmpAngle, finalColor
        );
    } else {
        al_draw_filled_circle(
            pos.x, pos.y,
            finalSize * 0.5,
            finalColor
        );
    }
    
    if(usedCustomBlend) {
        al_set_blender(oldOp, oldSource, oldDest);
    }
}


/**
 * @brief Sets the bitmap, according to the given information.
 * This automatically manages bitmap un/loading and such.
 * If the file name string is empty, sets to a nullptr bitmap
 * (and still unloads the old bitmap).
 *
 * @param newBmpName Internal name of the bitmap.
 * @param node If not nullptr, this will be used to report an error with,
 * in case something happens.
 */
void Particle::setBitmap(
    const string& newBmpName, DataNode* node
) {
    if(newBmpName != bmpName && bitmap) {
        game.content.bitmaps.list.free(bmpName);
        bitmap = nullptr;
    }
    
    if(newBmpName.empty()) {
        bmpName.clear();
        return;
    }
    
    if(newBmpName != bmpName || !bitmap) {
        bitmap =
            game.content.bitmaps.list.get(
                newBmpName, node, node != nullptr
            );
    }
    
    bmpName = newBmpName;
}


/**
 * @brief Ticks a particle's time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @return Whether its lifespan is over (meaning it should be deleted).
 */
void Particle::tick(const float deltaT) {
    time -= deltaT;
    
    if(time <= 0.0f) {
        time = 0.0f;
        return;
    }
    
    float t = 1.0f - time / duration;
    
    Point totalVelocity = linearSpeed.get(t);
    float outwardsAngle = getAngle(pos - origin);
    
    if(pos == origin) {
        outwardsAngle = game.rng.f(-180, 180);
    }
    totalVelocity +=
        angleToCoordinates(outwardsAngle, outwardsSpeed.get(t));
        
    //Add 90 degrees to make the angle tangential.
    totalVelocity +=
        angleToCoordinates(outwardsAngle + (TAU / 4), orbitalSpeed.get(t));
        
    //Accumulate and apply friction.
    totalVelocity -= totalFrictionApplied;
    Point newFriction = totalVelocity * (deltaT * friction);
    totalFrictionApplied += newFriction;
    totalVelocity -= newFriction;
    
    pos += totalVelocity * deltaT;
    
    if(bmpAngleType == PARTICLE_ANGLE_TYPE_DIRECTION) {
        coordinatesToAngle(totalVelocity, &bmpAngle, nullptr);
    }
}


/**
 * @brief Constructs a new particle emission object.
 *
 * @param emissionInterval Interval to spawn a new set of particles in,
 * in seconds. 0 means it spawns only one set and that's it.
 * @param num Number of particles to spawn.
 * This number is also deviated by numberDeviation.
 */
ParticleEmission::ParticleEmission(
    const float emissionInterval, const size_t num
) {
    number = num;
    interval = emissionInterval;
}


/**
 * @brief Returns a randomly-picked offset for a new particle.
 *
 * @param numberRatio Ratio of which number particle this is in the emission,
 * over the total particles to emit in this emission.
 * @return The offset.
 */
Point ParticleEmission::getEmissionOffset(float numberRatio) {
    switch (shape) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        if(evenlySpread) {
            return
                getRatioPointInRing(
                    circleInnerDist, circleOuterDist,
                    circleArc, circleArcRot, numberRatio
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


/**
 * @brief Constructs a new particle generator object.
 *
 * @param emissionInterval Interval to spawn a new set of particles in,
 * in seconds. 0 means it spawns only one set and that's it.
 * @param baseParticle All particles created will be based on this one.
 * Their properties will deviate randomly based on the
 * deviation members of the particle generator object.
 * @param number Number of particles to spawn.
 * This number is also deviated by numberDeviation.
 */
ParticleGenerator::ParticleGenerator(
    const float emissionInterval,
    const Particle& baseParticle, const size_t number
) :
    baseParticle(baseParticle) {
    emission = ParticleEmission(emissionInterval, number);
}


/**
 * @brief Emits the particles, regardless of the timer.
 *
 * @param manager The particle manager to place these particles on.
 */
void ParticleGenerator::emit(ParticleManager& manager) {
    Point basePPos = baseParticle.pos;
    float basePZ = baseParticle.z;
    Point offs = followPosOffset;
    if(followAngle) {
        offs = rotatePoint(offs, *followAngle);
    }
    basePPos += offs;
    basePZ += followZOffset;
    
    bool visible = false;
    for(size_t v = 0; v < manager.viewports.size(); v++) {
        if(
            basePPos.x >= manager.viewports[v]->box[0].x &&
            basePPos.x <= manager.viewports[v]->box[1].x &&
            basePPos.y >= manager.viewports[v]->box[0].y &&
            basePPos.y <= manager.viewports[v]->box[1].y
        ) {
            visible = true;
            break;
        }
    }
    if(!visible) {
        //Too far off-camera.
        return;
    }
    
    size_t finalNr =
        std::max(
            0,
            (int) emission.number +
            game.rng.i(
                (int) (0 - emission.numberDeviation),
                (int) emission.numberDeviation
            )
        );
        
    for(size_t p = 0; p < finalNr; p++) {
        Particle newP = baseParticle;
        
        newP.duration =
            std::max(
                0.0f,
                newP.duration +
                game.rng.f(-durationDeviation, durationDeviation)
            );
        newP.time = newP.duration;
        
        if(newP.bmpAngleType == PARTICLE_ANGLE_TYPE_FIXED) {
            newP.bmpAngle +=
                game.rng.f(-bmpAngleDeviation, bmpAngleDeviation);
        }
        newP.friction +=
            game.rng.f(-frictionDeviation, frictionDeviation);
            
        newP.pos = basePPos;
        newP.origin = basePPos;
        Point offset = emission.getEmissionOffset(p / (float) finalNr);
        if(followAngle) {
            offset = rotatePoint(offset, *followAngle);
        }
        newP.pos += offset;
        
        newP.z = basePZ;
        
        float sDev = game.rng.f(-sizeDeviation, sizeDeviation);
        for(size_t s = 0; s < newP.size.getKeyframeCount(); s++) {
            auto kf = newP.size.getKeyframe(s);
            newP.size.setKeyframeValue((int) s, kf.second + sDev);
        }
        
        float angleToUse =
            game.rng.f(
                -linearSpeedAngleDeviation,
                linearSpeedAngleDeviation
            );
        if(followAngle && !anglesAreAbsolute) {
            angleToUse += (*followAngle);
        }
        
        float vDevX =
            game.rng.f(-linearSpeedDeviation.x, linearSpeedDeviation.x);
        float vDevY =
            game.rng.f(-linearSpeedDeviation.y, linearSpeedDeviation.y);
        for(size_t s = 0; s < newP.linearSpeed.getKeyframeCount(); s++) {
            auto kf = newP.linearSpeed.getKeyframe(s);
            Point base = kf.second;
            Point result = Point(base.x + vDevX, base.y + vDevY);
            result =
                rotatePoint(
                    result, angleToUse
                );
            newP.linearSpeed.setKeyframeValue((int) s, result);
        }
        
        float outDev =
            game.rng.f(-outwardsSpeedDeviation, outwardsSpeedDeviation);
        for(size_t s = 0; s < newP.outwardsSpeed.getKeyframeCount(); s++) {
            auto kf = newP.outwardsSpeed.getKeyframe(s);
            newP.outwardsSpeed.setKeyframeValue((int) s, kf.second + outDev);
        }
        
        float orbDev =
            game.rng.f(-orbitalSpeedDeviation, orbitalSpeedDeviation);
        for(size_t s = 0; s < newP.orbitalSpeed.getKeyframeCount(); s++) {
            auto kf = newP.orbitalSpeed.getKeyframe(s);
            newP.orbitalSpeed.setKeyframeValue((int) s, kf.second + orbDev);
        }
        
        manager.add(newP);
    }
}


/**
 * @brief Loads particle generator data from a data node.
 *
 * @param node Data node to load from.
 * @param level Level to load at.
 */
void ParticleGenerator::loadFromDataNode(
    DataNode* node, CONTENT_LOAD_LEVEL level
) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter gRS(node);
    DataNode* baseParticleNode = node->getChildByName("base");
    ReaderSetter pRS(baseParticleNode);
    DataNode* emissionNode = node->getChildByName("emission");
    ReaderSetter eRS(emissionNode);
    
    float emissionIntervalFloat = 0.0f;
    size_t numberInt = 1;
    size_t shapeInt = 0;
    
    eRS.set("number", numberInt);
    eRS.set("interval", emissionIntervalFloat);
    emission = ParticleEmission(emissionIntervalFloat, numberInt);
    
    eRS.set("interval_deviation", emission.intervalDeviation);
    eRS.set("number_deviation", emission.numberDeviation);
    eRS.set("shape", shapeInt);
    
    switch (shapeInt) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        eRS.set("circle_outer_dist", emission.circleOuterDist);
        eRS.set("circle_inner_dist", emission.circleInnerDist);
        eRS.set("circle_arc", emission.circleArc);
        eRS.set("circle_arc_rot", emission.circleArcRot);
        eRS.set("evenly_spread", emission.evenlySpread);
        break;
    } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
        eRS.set("rect_outer_dist", emission.rectOuterDist);
        eRS.set("rect_inner_dist", emission.rectInnerDist);
        break;
    }
    }
    
    emission.shape = (PARTICLE_EMISSION_SHAPE) shapeInt;
    
    DataNode* bitmapNode = nullptr;
    size_t angleTypeInt = 0;
    size_t blendInt = 0;
    
    pRS.set("bitmap", baseParticle.bmpName, &bitmapNode);
    pRS.set("bitmap_angle", baseParticle.bmpAngle);
    pRS.set("bitmap_angle_type", angleTypeInt);
    pRS.set("duration", baseParticle.duration);
    pRS.set("friction", baseParticle.friction);
    pRS.set("blend_type", blendInt);
    
    baseParticle.bmpAngleType = (PARTICLE_ANGLE_TYPE) angleTypeInt;
    baseParticle.blendType = (PARTICLE_BLEND_TYPE) blendInt;
    baseParticle.bmpAngle = degToRad(baseParticle.bmpAngle);
    
    baseParticle.color.loadFromDataNode(
        baseParticleNode->getChildByName("color")
    );
    baseParticle.size.loadFromDataNode(
        baseParticleNode->getChildByName("size")
    );
    baseParticle.linearSpeed.loadFromDataNode(
        baseParticleNode->getChildByName("linear_speed")
    );
    baseParticle.outwardsSpeed.loadFromDataNode(
        baseParticleNode->getChildByName("outwards_speed")
    );
    baseParticle.orbitalSpeed.loadFromDataNode(
        baseParticleNode->getChildByName("orbital_speed")
    );
    
    if(bitmapNode) {
        if(level >= CONTENT_LOAD_LEVEL_FULL) {
            baseParticle.bitmap =
                game.content.bitmaps.list.get(
                    baseParticle.bmpName, bitmapNode
                );
        }
    } else {
        baseParticle.bmpName = "";
        baseParticle.bitmap = nullptr;
    }
    
    baseParticle.time = baseParticle.duration;
    baseParticle.priority = PARTICLE_PRIORITY_MEDIUM;
    
    gRS.set("bitmap_angle_deviation", bmpAngleDeviation);
    gRS.set("duration_deviation", durationDeviation);
    gRS.set("friction_deviation", frictionDeviation);
    gRS.set("size_deviation", sizeDeviation);
    gRS.set("angle_deviation", linearSpeedAngleDeviation);
    gRS.set("linear_speed_deviation", linearSpeedDeviation);
    gRS.set("orbital_speed_deviation", orbitalSpeedDeviation);
    gRS.set("outwards_speed_deviation", outwardsSpeedDeviation);
    gRS.set("angles_are_absolute", anglesAreAbsolute);
    
    bmpAngleDeviation = degToRad(bmpAngleDeviation);
    linearSpeedAngleDeviation = degToRad(linearSpeedAngleDeviation);
    
    id =
        (MOB_PARTICLE_GENERATOR_ID) (
            MOB_PARTICLE_GENERATOR_ID_STATUS +
            game.content.particleGens.list.size()
        );
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
 * @brief Saves particle generator data to a data node.
 *
 * @param node Node to save to.
 */
void ParticleGenerator::saveToDataNode(DataNode* node) {
    //Content metadata.
    saveMetadataToDataNode(node);
    
    //Emission.
    DataNode* emissionNode = node->addNew("emission");
    GetterWriter eGW(emissionNode);
    
    eGW.write("number", emission.number);
    eGW.write("number_deviation", emission.numberDeviation);
    eGW.write("interval", emission.interval);
    eGW.write("interval_deviation", emission.intervalDeviation);
    eGW.write("shape", emission.shape);
    
    switch (emission.shape) {
    case PARTICLE_EMISSION_SHAPE_CIRCLE: {
        eGW.write("circle_outer_dist", emission.circleOuterDist);
        eGW.write("circle_inner_dist", emission.circleInnerDist);
        eGW.write("circle_arc", emission.circleArc);
        eGW.write("circle_arc_rot", emission.circleArcRot);
        eGW.write("evenly_spread", emission.evenlySpread);
        break;
    } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
        eGW.write("rect_outer_dist", emission.rectOuterDist);
        eGW.write("rect_inner_dist", emission.rectInnerDist);
        break;
    }
    }
    
    //Base particle.
    DataNode* baseParticleNode = node->addNew("base");
    GetterWriter pGW(baseParticleNode);
    
    pGW.write("bitmap", baseParticle.bmpName);
    pGW.write("bitmap_angle", baseParticle.bmpAngle);
    pGW.write("bitmap_angle_type", baseParticle.bmpAngleType);
    pGW.write("duration", baseParticle.duration);
    pGW.write("friction", baseParticle.friction);
    pGW.write("blend_type", baseParticle.blendType);
    
    DataNode* colorNode = baseParticleNode->addNew("color");
    GetterWriter pcGW(colorNode);
    
    for(size_t c = 0; c < baseParticle.color.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.color.getKeyframe(c);
        pcGW.write(f2s(keyframe.first), keyframe.second);
    }
    
    DataNode* sizeNode = baseParticleNode->addNew("size");
    GetterWriter psGW(sizeNode);
    
    for(size_t c = 0; c < baseParticle.size.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.size.getKeyframe(c);
        psGW.write(f2s(keyframe.first), keyframe.second);
    }
    
    DataNode* linSpeedNode = baseParticleNode->addNew("linear_speed");
    GetterWriter plsGW(linSpeedNode);
    
    for(size_t c = 0; c < baseParticle.linearSpeed.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.linearSpeed.getKeyframe(c);
        plsGW.write(f2s(keyframe.first), keyframe.second);
    }
    
    DataNode* outSpeedNode = baseParticleNode->addNew("outwards_speed");
    GetterWriter posGW(outSpeedNode);
    
    for(size_t c = 0; c < baseParticle.outwardsSpeed.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.outwardsSpeed.getKeyframe(c);
        posGW.write(f2s(keyframe.first), keyframe.second);
    }
    
    DataNode* orbSpeedNode = baseParticleNode->addNew("orbital_speed");
    GetterWriter porsGW(orbSpeedNode);
    
    for(size_t c = 0; c < baseParticle.orbitalSpeed.getKeyframeCount(); c++) {
        auto keyframe = baseParticle.orbitalSpeed.getKeyframe(c);
        porsGW.write(f2s(keyframe.first), keyframe.second);
    }
    
    //Generator.
    GetterWriter gGW(node);
    
    gGW.write("bitmap_angle_deviation", radToDeg(bmpAngleDeviation));
    gGW.write("duration_deviation", durationDeviation);
    gGW.write("friction_deviation", frictionDeviation);
    gGW.write("size_deviation", sizeDeviation);
    gGW.write("orbital_speed_deviation", orbitalSpeedDeviation);
    gGW.write("outwards_speed_deviation", outwardsSpeedDeviation);
    gGW.write("angle_deviation", radToDeg(linearSpeedAngleDeviation));
    gGW.write("linear_speed_deviation", linearSpeedDeviation);
    gGW.write("angles_are_absolute", anglesAreAbsolute);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @param manager The manager of all particles.
 */
void ParticleGenerator::tick(float deltaT, ParticleManager& manager) {
    if(followMob) {
        baseParticle.pos = followMob->pos;
        baseParticle.z = followMob->z;
    }
    emissionTimer -= deltaT;
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
 * @param maxNr Maximum number of particles it can manage.
 */
ParticleManager::ParticleManager(size_t maxNr) :
    maxNr(maxNr) {
    
    if(maxNr == 0) return;
    particles = new Particle[maxNr];
    clear();
}


/**
 * @brief Constructs a new particle manager object by copying data from another.
 *
 * @param pm2 Particle manager to copy from.
 */
ParticleManager::ParticleManager(const ParticleManager& pm2) :
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
ParticleManager& ParticleManager::operator =(
    const ParticleManager& pm2
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
void ParticleManager::add(const Particle& p) {
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
 * @param camTL Only draw particles below and to the right of this coordinate.
 * @param camBR Only draw particles above and to the left of this coordinate.
 */
void ParticleManager::fillComponentList(
    vector<WorldComponent>& list,
    const Point& camTL, const Point& camBR
) {
    for(size_t c = 0; c < count; c++) {
    
        Particle* pPtr = &particles[c];
        float pSize =
            pPtr->size.get((pPtr->duration - pPtr->time) / pPtr->duration);
        if(
            camTL != camBR &&
            !rectanglesIntersect(
                pPtr->pos - pSize, pPtr->pos + pSize,
                camTL, camBR
            )
        ) {
            //Off-camera.
            continue;
        }
        
        WorldComponent wc;
        wc.particlePtr = pPtr;
        wc.z = pPtr->z;
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
 * @param deltaT How long the frame's tick is, in seconds.
 */
void ParticleManager::tickAll(float deltaT) {
    for(size_t c = 0; c < count;) {
        particles[c].tick(deltaT);
        if(particles[c].time == 0.0f) {
            remove(c);
        } else {
            c++;
        }
    }
}
