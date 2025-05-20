/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation database, animation, animation instance, frame,
 * and sprite classes, and animation-related functions.
 */

#include <algorithm>
#include <map>
#include <vector>

#include "animation.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


using std::size_t;
using std::string;
using std::vector;


/**
 * @brief Constructs a new animation object.
 *
 * @param name Name, should be unique.
 * @param frames List of frames.
 * @param loopFrame Loop frame index.
 * @param hitRate If this has an attack, this is the chance of hitting.
 * 0 - 100.
 */
Animation::Animation(
    const string &name, const vector<Frame> &frames,
    size_t loopFrame, unsigned char hitRate
) :
    name(name),
    frames(frames),
    loopFrame(loopFrame),
    hitRate(hitRate) {
    
}


/**
 * @brief Constructs a new animation object.
 *
 * @param a2 The other animation.
 */
Animation::Animation(const Animation &a2) :
    name(a2.name),
    frames(a2.frames),
    loopFrame(a2.loopFrame),
    hitRate(a2.hitRate) {
}


/**
 * @brief Creates an animation by copying info from another animation.
 *
 * @param a2 The other animation.
 * @return The current object.
 */
Animation &Animation::operator=(const Animation &a2) {
    if(this != &a2) {
        name = a2.name;
        frames = a2.frames;
        loopFrame = a2.loopFrame;
        hitRate = a2.hitRate;
    }
    
    return *this;
}


/**
 * @brief Deletes one of the animation's frames.
 *
 * @param idx Frame index.
 */
void Animation::deleteFrame(size_t idx) {
    if(idx == INVALID) return;
    
    if(idx < loopFrame) {
        //Let the loop frame stay the same.
        loopFrame--;
    }
    if(
        idx == loopFrame &&
        loopFrame == frames.size() - 1
    ) {
        //Stop the loop frame from going out of bounds.
        loopFrame--;
    }
    frames.erase(frames.begin() + idx);
}


/**
 * @brief Returns the total duration of the animation.
 *
 * @return The duration.
 */
float Animation::getDuration() {
    float duration = 0.0f;
    for(size_t f = 0; f < frames.size(); f++) {
        duration += frames[f].duration;
    }
    return duration;
}


/**
 * @brief Returns the frame index, and time within that frame,
 * that matches the specified time.
 *
 * @param t Time to check.
 * @param frameIdx The frame index is returned here.
 * @param frameTime The time within the frame is returned here.
 */
void Animation::getFrameAndTime(
    float t, size_t* frameIdx, float* frameTime
) {
    *frameIdx = 0;
    *frameTime = 0.0f;
    
    if(frames.empty() || t <= 0.0f) {
        return;
    }
    
    float durationSoFar = 0.0f;
    float prevDurationSoFar = 0.0f;
    size_t f = 0;
    for(f = 0; f < frames.size(); f++) {
        prevDurationSoFar = durationSoFar;
        durationSoFar += frames[f].duration;
        
        if(durationSoFar > t) {
            break;
        }
    }
    
    *frameIdx = std::clamp(f, (size_t) 0, frames.size() - 1);
    *frameTime = t - prevDurationSoFar;
}


/**
 * @brief Returns the total duration of the loop segment of the animation.
 *
 * @return The duration.
 */
float Animation::getLoopDuration() {
    float duration = 0.0f;
    for(size_t f = loopFrame; f < frames.size(); f++) {
        duration += frames[f].duration;
    }
    return duration;
}


/**
 * @brief Returns the total time since the animation start, when given a frame
 * and the current time in the current frame.
 *
 * @param frameIdx Current frame index.
 * @param frameTime Time in the current frame.
 * @return The time.
 */
float Animation::getTime(size_t frameIdx, float frameTime) {
    if(frameIdx == INVALID) {
        return 0.0f;
    }
    if(frameIdx >= frames.size()) {
        return getDuration();
    }
    
    float curTime = 0.0f;
    for(size_t f = 0; f < frameIdx; f++) {
        curTime += frames[f].duration;
    }
    curTime += frameTime;
    return curTime;
}


/**
 * @brief Constructs a new animation database object.
 *
 * @param a List of animations.
 * @param s List of sprites.
 * @param b List of body parts.
 */
AnimationDatabase::AnimationDatabase(
    const vector<Animation*> &a, const vector<Sprite*> &s,
    const vector<BodyPart*> &b
) :
    animations(a),
    sprites(s),
    bodyParts(b) {
    
}


/**
 * @brief Calculates the maximum distance that any of its hitbox can reach,
 * and stores it in the hitboxSpan variable.
 */
void AnimationDatabase::calculateHitboxSpan() {
    hitboxSpan = 0.0f;
    for(size_t s = 0; s < sprites.size(); s++) {
        Sprite* s_ptr = sprites[s];
        for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
            Hitbox* hPtr = &s_ptr->hitboxes[h];
            
            float d = Distance(Point(0.0f), hPtr->pos).toFloat();
            d += hPtr->radius;
            hitboxSpan = std::max(hitboxSpan, d);
        }
    }
}


/**
 * @brief Enemies and such have a regular list of animations.
 * The only way to change these animations is through the script.
 * So animation control is done entirely through game data.
 * However, the animations for Pikmin, leaders, etc. are pre-named.
 * e.g.: The game wants there to be an "idle" animation,
 * a "walk" animation, etc.
 * Because we are NOT looking up with strings, if we want more than 20FPS,
 * we need a way to convert from a numeric index
 * (one that stands for walking, one for idling, etc.)
 * into the corresponding index on the animation file.
 * This is where this comes in.
 *
 * @param conversions A vector of size_t and strings.
 * The size_t is the hardcoded index (probably in some constant or enum).
 * The string is the name of the animation in the animation file.
 * @param file File from where these animations were loaded. Used to
 * report errors.
 */
void AnimationDatabase::createConversions(
    const vector<std::pair<size_t, string> > &conversions,
    const DataNode* file
) {
    preNamedConversions.clear();
    
    if(conversions.empty()) return;
    
    //First, find the highest index.
    size_t highest = conversions[0].first;
    for(size_t c = 1; c < conversions.size(); c++) {
        highest = std::max(highest, conversions[c].first);
    }
    
    preNamedConversions.assign(highest + 1, INVALID);
    
    for(size_t c = 0; c < conversions.size(); c++) {
        size_t aPos = findAnimation(conversions[c].second);
        preNamedConversions[conversions[c].first] = aPos;
        if(aPos == INVALID) {
            game.errors.report(
                "Animation \"" + conversions[c].second + "\" is required "
                "by the engine, but does not exist!", file
            );
        }
    }
}


/**
 * @brief Destroys an animation database and all of its content.
 */
void AnimationDatabase::destroy() {
    resetMetadata();
    for(size_t a = 0; a < animations.size(); a++) {
        delete animations[a];
    }
    for(size_t s = 0; s < sprites.size(); s++) {
        delete sprites[s];
    }
    for(size_t b = 0; b < bodyParts.size(); b++) {
        delete bodyParts[b];
    }
    animations.clear();
    sprites.clear();
    bodyParts.clear();
}


/**
 * @brief Deletes a sprite, adjusting any animations that use it.
 *
 * @param idx Sprite index.
 */
void AnimationDatabase::deleteSprite(size_t idx) {
    for(size_t a = 0; a < animations.size(); a++) {
        Animation* aPtr = animations[a];
        
        for(size_t f = 0; f < aPtr->frames.size();) {
            if(aPtr->frames[f].spriteIdx == idx) {
                aPtr->deleteFrame(f);
            } else {
                f++;
            }
        }
    }
    
    sprites.erase(sprites.begin() + idx);
    
    for(size_t a = 0; a < animations.size(); a++) {
        Animation* aPtr = animations[a];
        for(size_t f = 0; f < aPtr->frames.size(); f++) {
            Frame* fPtr = &(aPtr->frames[f]);
            fPtr->spriteIdx = findSprite(fPtr->spriteName);
        }
    }
}


/**
 * @brief Fills each frame's sound index cache variable, where applicable.
 *
 * @param mtPtr Mob type with the sound data.
 */
void AnimationDatabase::fillSoundIdxCaches(MobType* mtPtr) {
    for(size_t a = 0; a < animations.size(); a++) {
        Animation* aPtr = animations[a];
        for(size_t f = 0; f < aPtr->frames.size(); f++) {
            Frame* fPtr = &aPtr->frames[f];
            fPtr->soundIdx = INVALID;
            if(fPtr->sound.empty()) continue;
            
            for(size_t s = 0; s < mtPtr->sounds.size(); s++) {
                if(mtPtr->sounds[s].name == fPtr->sound) {
                    fPtr->soundIdx = s;
                }
            }
        }
    }
}


/**
 * @brief Returns the index of the specified animation.
 *
 * @param name Name of the animation to search for.
 * @return The index, or INVALID if not found.
 */
size_t AnimationDatabase::findAnimation(const string &name) const {
    for(size_t a = 0; a < animations.size(); a++) {
        if(animations[a]->name == name) return a;
    }
    return INVALID;
}


/**
 * @brief Returns the index of the specified body part.
 *
 * @param name Name of the body part to search for.
 * @return The index, or INVALID if not found.
 */
size_t AnimationDatabase::findBodyPart(const string &name) const {
    for(size_t b = 0; b < bodyParts.size(); b++) {
        if(bodyParts[b]->name == name) return b;
    }
    return INVALID;
}


/**
 * @brief Returns the index of the specified sprite.
 *
 * @param name Name of the sprite to search for.
 * @return The index, or INVALID if not found.
 */
size_t AnimationDatabase::findSprite(const string &name) const {
    for(size_t s = 0; s < sprites.size(); s++) {
        if(sprites[s]->name == name) return s;
    }
    return INVALID;
}


/**
 * @brief Fixes the pointers for body parts.
 */
void AnimationDatabase::fixBodyPartPointers() {
    for(size_t s = 0; s < sprites.size(); s++) {
        Sprite* sPtr = sprites[s];
        for(size_t h = 0; h < sPtr->hitboxes.size(); h++) {
            Hitbox* hPtr = &sPtr->hitboxes[h];
            
            for(size_t b = 0; b < bodyParts.size(); b++) {
                BodyPart* bPtr = bodyParts[b];
                if(bPtr->name == hPtr->bodyPartName) {
                    hPtr->bodyPartIdx = b;
                    hPtr->bodyPartPtr = bPtr;
                    break;
                }
            }
        }
    }
}


/**
 * @brief Loads animation database data from a data node.
 *
 * @param node Data node to load from.
 */
void AnimationDatabase::loadFromDataNode(DataNode* node) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Body parts.
    DataNode* bodyPartsNode = node->getChildByName("body_parts");
    size_t nBodyParts = bodyPartsNode->getNrOfChildren();
    for(size_t b = 0; b < nBodyParts; b++) {
    
        DataNode* bodyPartNode = bodyPartsNode->getChild(b);
        
        BodyPart* curBodyPart = new BodyPart(bodyPartNode->name);
        bodyParts.push_back(curBodyPart);
    }
    
    //Sprites.
    DataNode* spritesNode = node->getChildByName("sprites");
    size_t nSprites = spritesNode->getNrOfChildren();
    for(size_t s = 0; s < nSprites; s++) {
    
        DataNode* spriteNode = spritesNode->getChild(s);
        ReaderSetter sRS(spriteNode);
        Sprite* newSprite = new Sprite(spriteNode->name);
        
        DataNode* bmpNameNode = nullptr;
        
        sRS.set("file_pos", newSprite->bmpPos);
        sRS.set("file_size", newSprite->bmpSize);
        sRS.set("offset", newSprite->offset);
        sRS.set("scale", newSprite->scale);
        sRS.set("angle", newSprite->angle);
        sRS.set("tint", newSprite->tint);
        sRS.set("file", newSprite->bmpName, &bmpNameNode);
        sRS.set("top_visible", newSprite->topVisible);
        sRS.set("top_pos", newSprite->topPos);
        sRS.set("top_size", newSprite->topSize);
        sRS.set("top_angle", newSprite->topAngle);
        
        //Hitboxes.
        DataNode* hitboxesNode = spriteNode->getChildByName("hitboxes");
        size_t nHitboxes = hitboxesNode->getNrOfChildren();
        for(size_t h = 0; h < nHitboxes; h++) {
            DataNode* hitboxNode = hitboxesNode->getChild(h);
            ReaderSetter hRS(hitboxNode);
            Hitbox newHitbox;
            
            string coordsStr;
            int hitboxTypeInt = HITBOX_TYPE_NORMAL;
            string hazardStr;
            DataNode* hazardNode = nullptr;
            
            hRS.set("coords", coordsStr);
            hRS.set("height", newHitbox.height);
            hRS.set("radius", newHitbox.radius);
            hRS.set("type", hitboxTypeInt);
            hRS.set("value", newHitbox.value);
            hRS.set("can_pikmin_latch", newHitbox.canPikminLatch);
            hRS.set("knockback_outward", newHitbox.knockbackOutward);
            hRS.set("knockback_angle", newHitbox.knockbackAngle);
            hRS.set("knockback", newHitbox.knockback);
            hRS.set("wither_chance", newHitbox.witherChance);
            hRS.set("hazard", hazardStr, &hazardNode);
            
            newHitbox.bodyPartName = hitboxNode->name;
            newHitbox.pos = s2p(coordsStr, &newHitbox.z);
            newHitbox.type = (HITBOX_TYPE) hitboxTypeInt;
            if(!hazardStr.empty()) {
                auto hazardIt = game.content.hazards.list.find(hazardStr);
                if(hazardIt != game.content.hazards.list.end()) {
                    newHitbox.hazard = &(hazardIt->second);
                } else {
                    game.errors.report(
                        "Unknown hazard \"" + hazardStr + "\"!",
                        hazardNode
                    );
                }
            }
            
            newSprite->hitboxes.push_back(newHitbox);
            
        }
        
        newSprite->setBitmap(
            newSprite->bmpName, newSprite->bmpPos, newSprite->bmpSize,
            bmpNameNode
        );
        
        sprites.push_back(newSprite);
    }
    
    //Animations.
    DataNode* animsNode = node->getChildByName("animations");
    size_t nAnims = animsNode->getNrOfChildren();
    for(size_t a = 0; a < nAnims; a++) {
        DataNode* animNode = animsNode->getChild(a);
        ReaderSetter aRS(animNode);
        Animation* newAnim = new Animation(animNode->name);
        
        aRS.set("loop_frame", newAnim->loopFrame);
        aRS.set("hit_rate", newAnim->hitRate);
        
        newAnim->name = animNode->name;
        
        //Frames.
        DataNode* framesNode = animNode->getChildByName("frames");
        size_t nFrames = framesNode->getNrOfChildren();
        for(size_t f = 0; f < nFrames; f++) {
            DataNode* frameNode = framesNode->getChild(f);
            ReaderSetter fRS(frameNode);
            Frame newFrame;
            
            string signalStr;
            
            fRS.set("signal", signalStr);
            fRS.set("duration", newFrame.duration);
            fRS.set("interpolate", newFrame.interpolate);
            fRS.set("sound", newFrame.sound);
            fRS.set("sound", newFrame.sound);
            
            newFrame.spriteName = frameNode->name;
            newFrame.spriteIdx = findSprite(frameNode->name);
            newFrame.spritePtr =
                newFrame.spriteIdx == INVALID ?
                nullptr :
                sprites[newFrame.spriteIdx];
            newFrame.signal = signalStr.empty() ? INVALID : s2i(signalStr);
            
            newAnim->frames.push_back(newFrame);
        }
        
        animations.push_back(newAnim);
    }
    
    //Finish up.
    fixBodyPartPointers();
    calculateHitboxSpan();
}


/**
 * @brief Saves the animation database data to a data node.
 *
 * @param node Data node to save to.
 * @param saveTopData Whether to save the Pikmin top's data.
 */
void AnimationDatabase::saveToDataNode(
    DataNode* node, bool saveTopData
) {
    //Content metadata.
    saveMetadataToDataNode(node);
    
    //Animations.
    DataNode* animationsNode = node->addNew("animations");
    for(size_t a = 0; a < animations.size(); a++) {
    
        //Animation.
        Animation* animPtr = animations[a];
        DataNode* animNode = animationsNode->addNew(animPtr->name);
        GetterWriter aGW(animNode);
        
        if(animPtr->loopFrame > 0) {
            aGW.write("loop_frame", animPtr->loopFrame);
        }
        if(animPtr->hitRate != 100) {
            aGW.write("hit_rate", animPtr->hitRate);
        }
        
        //Frames.
        DataNode* framesNode = animNode->addNew("frames");
        for(size_t f = 0; f < animPtr->frames.size(); f++) {
        
            //Frame.
            Frame* framePtr = &animPtr->frames[f];
            DataNode* frameNode = framesNode->addNew(framePtr->spriteName);
            GetterWriter fGW(frameNode);
            
            fGW.write("duration", framePtr->duration);
            if(framePtr->interpolate) {
                fGW.write("interpolate", framePtr->interpolate);
            }
            if(framePtr->signal != INVALID) {
                fGW.write("signal", framePtr->signal);
            }
            if(!framePtr->sound.empty() && framePtr->sound != NONE_OPTION) {
                fGW.write("sound", framePtr->sound);
            }
        }
    }
    
    //Sprites.
    DataNode* spritesNode = node->addNew("sprites");
    for(size_t s = 0; s < sprites.size(); s++) {
    
        //Sprite.
        Sprite* spritePtr = sprites[s];
        DataNode* spriteNode = spritesNode->addNew(sprites[s]->name);
        GetterWriter sGW(spriteNode);
        
        sGW.write("file", spritePtr->bmpName);
        sGW.write("file_pos", spritePtr->bmpPos);
        sGW.write("file_size", spritePtr->bmpSize);
        if(spritePtr->offset.x != 0.0 || spritePtr->offset.y != 0.0) {
            sGW.write("offset", spritePtr->offset);
        }
        if(spritePtr->scale.x != 1.0 || spritePtr->scale.y != 1.0) {
            sGW.write("scale", spritePtr->scale);
        }
        if(spritePtr->angle != 0.0) {
            sGW.write("angle", spritePtr->angle);
        }
        if(spritePtr->tint != COLOR_WHITE) {
            sGW.write("tint", spritePtr->tint);
        }
        
        if(saveTopData) {
            sGW.write("top_visible", spritePtr->topVisible);
            sGW.write("top_pos", spritePtr->topPos);
            sGW.write("top_size", spritePtr->topSize);
            sGW.write("top_angle", spritePtr->topAngle);
        }
        
        if(!spritePtr->hitboxes.empty()) {
        
            //Hitboxes.
            DataNode* hitboxesNode = spriteNode->addNew("hitboxes");
            for(size_t h = 0; h < spritePtr->hitboxes.size(); h++) {
            
                //Hitbox.
                Hitbox* hitboxPtr = &spritePtr->hitboxes[h];
                DataNode* hitboxNode =
                    hitboxesNode->addNew(hitboxPtr->bodyPartName);
                GetterWriter hGW(hitboxNode);
                
                hGW.write("coords", p2s(hitboxPtr->pos, &hitboxPtr->z));
                hGW.write("height", hitboxPtr->height);
                hGW.write("radius", hitboxPtr->radius);
                hGW.write("type", hitboxPtr->type);
                hGW.write("value", hitboxPtr->value);
                if(
                    hitboxPtr->type == HITBOX_TYPE_NORMAL &&
                    hitboxPtr->canPikminLatch
                ) {
                    hGW.write("can_pikmin_latch", hitboxPtr->canPikminLatch);
                }
                if(hitboxPtr->hazard) {
                    hGW.write(
                        "hazard", hitboxPtr->hazard->manifest->internalName
                    );
                }
                if(
                    hitboxPtr->type == HITBOX_TYPE_ATTACK &&
                    hitboxPtr->knockbackOutward
                ) {
                    hGW.write("knockback_outward", hitboxPtr->knockbackOutward);
                }
                if(
                    hitboxPtr->type == HITBOX_TYPE_ATTACK &&
                    hitboxPtr->knockbackAngle != 0
                ) {
                    hGW.write("knockback_angle", hitboxPtr->knockbackAngle);
                }
                if(
                    hitboxPtr->type == HITBOX_TYPE_ATTACK &&
                    hitboxPtr->knockback != 0
                ) {
                    hGW.write("knockback", hitboxPtr->knockback);
                }
                if(
                    hitboxPtr->type == HITBOX_TYPE_ATTACK &&
                    hitboxPtr->witherChance > 0
                ) {
                    hGW.write("wither_chance", hitboxPtr->witherChance);
                }
            }
        }
    }
    
    //Body parts.
    DataNode* bodyPartsNode = node->addNew("body_parts");
    for(size_t b = 0; b < bodyParts.size(); b++) {
    
        //Body part.
        BodyPart* bodyPartPtr = bodyParts[b];
        bodyPartsNode->addNew(bodyPartPtr->name);
        
    }
}


/**
 * @brief Sorts all animations and sprites alphabetically,
 * making them more organized.
 */
void AnimationDatabase::sortAlphabetically() {
    sort(
        animations.begin(), animations.end(),
    [] (const Animation * a1, const Animation * a2) {
        return a1->name < a2->name;
    }
    );
    sort(
        sprites.begin(), sprites.end(),
    [] (const Sprite * s1, const Sprite * s2) {
        return s1->name < s2->name;
    }
    );
    
    for(size_t a = 0; a < animations.size(); a++) {
        Animation* aPtr = animations[a];
        for(size_t f = 0; f < aPtr->frames.size(); f++) {
            Frame* fPtr = &(aPtr->frames[f]);
            
            fPtr->spriteIdx = findSprite(fPtr->spriteName);
        }
    }
}


/**
 * @brief Constructs a new animation instance object.
 *
 * @param animDb The animation database. Used when changing animations.
 */
AnimationInstance::AnimationInstance(AnimationDatabase* animDb) :
    curAnim(nullptr),
    animDb(animDb) {
    
}


/**
 * @brief Constructs a new animation instance object.
 *
 * @param ai2 The other animation instance.
 */
AnimationInstance::AnimationInstance(const AnimationInstance &ai2) :
    curAnim(ai2.curAnim),
    animDb(ai2.animDb) {
    
    toStart();
}


/**
 * @brief Copies data from another animation instance.
 *
 * @param ai2 The other animation instance.
 * @return The current object.
 */
AnimationInstance &AnimationInstance::operator=(
    const AnimationInstance &ai2
) {
    if(this != &ai2) {
        curAnim = ai2.curAnim;
        animDb = ai2.animDb;
    }
    
    toStart();
    
    return *this;
}


/**
 * @brief Returns the sprite of the current frame of animation.
 *
 * @param outCurSpritePtr If not nullptr, the current sprite is
 * returned here.
 * @param outNextSpritePtr If not nullptr, the next sprite in the animation
 * is returned here.
 * @param outInterpolationFactor If not nullptr, the interpolation factor
 * (0 to 1) between the current sprite and the next one is returned here.
 */
void AnimationInstance::getSpriteData(
    Sprite** outCurSpritePtr, Sprite** outNextSpritePtr,
    float* outInterpolationFactor
) const {
    if(!validFrame()) {
        if(outCurSpritePtr) *outCurSpritePtr = nullptr;
        if(outNextSpritePtr) *outNextSpritePtr = nullptr;
        if(outInterpolationFactor) *outInterpolationFactor = 0.0f;
        return;
    }
    
    Frame* curFramePtr = &curAnim->frames[curFrameIdx];
    //First, the basics -- the current sprite.
    if(outCurSpritePtr) {
        *outCurSpritePtr = curFramePtr->spritePtr;
    }
    
    //Now only bother with interpolation data if we actually need it.
    if(!outNextSpritePtr && !outInterpolationFactor) return;
    
    if(!curFramePtr->interpolate) {
        //This frame doesn't even interpolate.
        if(outNextSpritePtr) *outNextSpritePtr = curFramePtr->spritePtr;
        if(outInterpolationFactor) *outInterpolationFactor = 0.0f;
        return;
    }
    
    //Get the next sprite.
    size_t nextFrameIdx = getNextFrameIdx();
    Frame* nextFramePtr = &curAnim->frames[nextFrameIdx];
    
    if(outNextSpritePtr) *outNextSpritePtr = nextFramePtr->spritePtr;
    
    //Get the interpolation factor.
    if(outInterpolationFactor) {
        if(curFramePtr->duration == 0.0f) {
            *outInterpolationFactor = 0.0f;
        } else {
            *outInterpolationFactor =
                curFrameTime /
                curFramePtr->duration;
        }
    }
}


/**
 * @brief Returns the index of the next frame of animation, the one after
 * the current one.
 *
 * @param outReachedEnd If not nullptr, true is returned here if we've reached
 * the end and the next frame loops back to the beginning.
 * @return The index, or INVALID on error.
 */
size_t AnimationInstance::getNextFrameIdx(bool* outReachedEnd) const {
    if(outReachedEnd) *outReachedEnd = false;
    if(!curAnim) return INVALID;
    
    if(curFrameIdx < curAnim->frames.size() - 1) {
        return curFrameIdx + 1;
    } else {
        if(outReachedEnd) *outReachedEnd = true;
        if(curAnim->loopFrame < curAnim->frames.size()) {
            return curAnim->loopFrame;
        } else {
            return 0;
        }
    }
}


/**
 * @brief Initializes the instance by setting its database to the given one,
 * its animation to the first one in the database, and setting the time
 * to the beginning.
 *
 * @param db Pointer to the animation database.
 */
void AnimationInstance::initToFirstAnim(AnimationDatabase* db) {
    animDb = db;
    if(db && !animDb->animations.empty()) {
        curAnim = animDb->animations[0];
    }
    toStart();
}


/**
 * @brief Skips the current animation instance ahead in time for a
 * random amount of time.
 *
 * The time is anywhere between 0 and the total duration of the
 * animation. Frame signals will be ignored.
 */
void AnimationInstance::skipAheadRandomly() {
    if(!curAnim) return;
    //First, find how long the animation lasts for.
    
    float totalDuration = 0;
    for(size_t f = 0; f < curAnim->frames.size(); f++) {
        totalDuration += curAnim->frames[f].duration;
    }
    
    tick(game.rng.f(0, totalDuration));
}


/**
 * @brief Clears everything.
 */
void AnimationInstance::clear() {
    curAnim = nullptr;
    animDb = nullptr;
    curFrameTime = 0;
    curFrameIdx = INVALID;
}


/**
 * @brief Ticks the animation time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @param signals Any frame that sends a signal adds it here.
 * @param sounds Any frame that should play a sound adds it here.
 * @return Whether or not the animation ended its final frame.
 */
bool AnimationInstance::tick(
    float deltaT, vector<size_t>* signals,
    vector<size_t>* sounds
) {
    if(!curAnim) return false;
    size_t nFrames = curAnim->frames.size();
    if(nFrames == 0) return false;
    Frame* curFramePtr = &curAnim->frames[curFrameIdx];
    if(curFramePtr->duration == 0) {
        return true;
    }
    
    curFrameTime += deltaT;
    
    bool reachedEnd = false;
    
    //This is a while instead of an if because if the framerate is too low
    //and the next frame's duration is too short, it could be that a tick
    //goes over an entire frame, and lands 2 or more frames ahead.
    while(curFrameTime > curFramePtr->duration && curFramePtr->duration != 0) {
        curFrameTime = curFrameTime - curFramePtr->duration;
        bool reachedEndNow = false;
        curFrameIdx = getNextFrameIdx(&reachedEndNow);
        reachedEnd |= reachedEndNow;
        curFramePtr = &curAnim->frames[curFrameIdx];
        if(curFramePtr->signal != INVALID && signals) {
            signals->push_back(curFramePtr->signal);
        }
        if(curFramePtr->soundIdx != INVALID && sounds) {
            sounds->push_back(curFramePtr->soundIdx);
        }
    }
    
    return reachedEnd;
}


/**
 * @brief Sets the animation state to the beginning.
 * It's called automatically when the animation is first set.
 */
void AnimationInstance::toStart() {
    curFrameTime = 0;
    curFrameIdx = 0;
}


/**
 * @brief Returns whether or not the animation instance is in a state where
 * it can show a valid frame.
 *
 * @return Whether it's in a valid state.
 */
bool AnimationInstance::validFrame() const {
    if(!curAnim) return false;
    if(curFrameIdx >= curAnim->frames.size()) return false;
    return true;
}


/**
 * @brief Constructs a new frame object.
 *
 * @param sn Name of the sprite.
 * @param si Index of the sprite in the animation database.
 * @param sp Pointer to the sprite.
 * @param d Duration.
 * @param in Whether to interpolate between this frame's transformation data
 * and the next's.
 * @param snd Sound name.
 * @param s Signal.
 */
Frame::Frame(
    const string &sn, size_t si, Sprite* sp, float d,
    bool in, const string &snd, size_t s
) :
    spriteName(sn),
    spriteIdx(si),
    spritePtr(sp),
    duration(d),
    interpolate(in),
    sound(snd),
    signal(s) {
    
}


/**
 * @brief Constructs a new sprite object.
 *
 * @param name Internal name; should be unique.
 * @param b Bitmap.
 * @param h List of hitboxes.
 */
Sprite::Sprite(
    const string &name, ALLEGRO_BITMAP* const b, const vector<Hitbox> &h
) :
    name(name),
    bitmap(b),
    hitboxes(h) {
    
}


/**
 * @brief Constructs a new sprite object.
 *
 * @param name Internal name, should be unique.
 * @param b Parent bitmap.
 * @param bPos X and Y of the top-left corner of the sprite, in the
 * parent's bitmap.
 * @param bSize Width and height of the sprite, in the parent's bitmap.
 * @param h List of hitboxes.
 */
Sprite::Sprite(
    const string &name, ALLEGRO_BITMAP* const b, const Point &bPos,
    const Point &bSize, const vector<Hitbox> &h
) :
    name(name),
    parentBmp(b),
    bmpPos(bPos),
    bmpSize(bSize),
    bitmap(
        b ?
        al_create_sub_bitmap(b, bPos.x, bPos.y, bSize.x, bSize.y) :
        nullptr
    ),
    hitboxes(h) {
    
}


/**
 * @brief Constructs a new sprite object.
 *
 * @param s2 The other sprite.
 */
Sprite::Sprite(const Sprite &s2) :
    name(s2.name),
    parentBmp(nullptr),
    bmpName(s2.bmpName),
    bmpPos(s2.bmpPos),
    bmpSize(s2.bmpSize),
    offset(s2.offset),
    scale(s2.scale),
    angle(s2.angle),
    tint(s2.tint),
    topPos(s2.topPos),
    topSize(s2.topSize),
    topAngle(s2.topAngle),
    topVisible(s2.topVisible),
    bitmap(nullptr),
    hitboxes(s2.hitboxes) {
    
    setBitmap(bmpName, bmpPos, bmpSize);
}


/**
 * @brief Destroys the sprite object.
 */
Sprite::~Sprite() {
    setBitmap("", Point(), Point());
}


/**
 * @brief Creates the hitboxes, based on the body parts.
 *
 * @param adb The animation database the sprites and body parts belong to.
 * @param height The hitboxes's starting height.
 * @param radius The hitboxes's starting radius.
 */
void Sprite::createHitboxes(
    AnimationDatabase* const adb, float height, float radius
) {
    hitboxes.clear();
    for(size_t b = 0; b < adb->bodyParts.size(); b++) {
        hitboxes.push_back(
            Hitbox(
                adb->bodyParts[b]->name,
                b,
                adb->bodyParts[b],
                Point(), 0, height, radius
            )
        );
    }
}


/**
 * @brief Copies data from another sprite.
 *
 * @param s2 The other sprite.
 * @return The current object.
 */
Sprite &Sprite::operator=(const Sprite &s2) {
    if(this != &s2) {
        name = s2.name;
        parentBmp = nullptr;
        bmpPos = s2.bmpPos;
        bmpSize = s2.bmpSize;
        offset = s2.offset;
        scale = s2.scale;
        angle = s2.angle;
        tint = s2.tint;
        topPos = s2.topPos;
        topSize = s2.topSize;
        topAngle = s2.topAngle;
        topVisible = s2.topVisible;
        bitmap = nullptr;
        hitboxes = s2.hitboxes;
        setBitmap(s2.bmpName, bmpPos, bmpSize);
    }
    
    return *this;
}


/**
 * @brief Sets the bitmap and parent bitmap, according to the given information.
 * This automatically manages bitmap un/loading and such.
 * If the file name string is empty, sets to a nullptr bitmap
 * (and still unloads the old bitmap).
 *
 * @param newBmpName Internal name of the bitmap.
 * @param newBmpPos Top-left coordinates of the sub-bitmap inside the bitmap.
 * @param newBmpSize Dimensions of the sub-bitmap.
 * @param node If not nullptr, this will be used to report an error with,
 * in case something happens.
 */
void Sprite::setBitmap(
    const string &newBmpName,
    const Point &newBmpPos, const Point &newBmpSize,
    DataNode* node
) {
    if(bitmap) {
        al_destroy_bitmap(bitmap);
        bitmap = nullptr;
    }
    if(newBmpName != bmpName && parentBmp) {
        game.content.bitmaps.list.free(bmpName);
        parentBmp = nullptr;
    }
    
    if(newBmpName.empty()) {
        bmpName.clear();
        bmpSize = Point();
        bmpPos = Point();
        return;
    }
    
    if(newBmpName != bmpName || !parentBmp) {
        parentBmp =
            game.content.bitmaps.list.get(newBmpName, node, node != nullptr);
    }
    
    Point parentSize = getBitmapDimensions(parentBmp);
    
    bmpName = newBmpName;
    bmpPos = newBmpPos;
    bmpSize = newBmpSize;
    bmpPos.x = std::clamp(newBmpPos.x, 0.0f, parentSize.x - 1);
    bmpPos.y = std::clamp(newBmpPos.y, 0.0f, parentSize.y - 1);
    bmpSize.x = std::clamp(newBmpSize.x, 0.0f, parentSize.x - bmpPos.x);
    bmpSize.y = std::clamp(newBmpSize.y, 0.0f, parentSize.y - bmpPos.y);
    
    if(parentBmp) {
        bitmap =
            al_create_sub_bitmap(
                parentBmp, bmpPos.x, bmpPos.y,
                bmpSize.x, bmpSize.y
            );
    }
}


/**
 * @brief Returns the final transformation data for a "basic" sprite effect.
 * i.e. the translation, angle, scale, and tint. This makes use of
 * interpolation between two frames if applicable.
 *
 * @param basePos Base position of the translation.
 * @param baseAngle Base angle of the rotation.
 * @param baseAngleCosCache If you have a cached value for the base angle's
 * cosine, write it here. Otherwise use LARGE_FLOAT.
 * @param baseAngleSinCache If you have a cached value for the base angle's
 * sine, write it here. Otherwise use LARGE_FLOAT.
 * @param curSpritePtr The current sprite.
 * @param nextSpritePtr The next sprite, if any.
 * @param interpolationFactor Amount to interpolate the two sprites by, if any.
 * Ranges from 0 to 1.
 * @param outEffTrans If not nullptr, the final translation is
 * returned here.
 * @param outEffAngle If not nullptr, the final rotation angle is
 * returned here.
 * @param outEffScale If not nullptr, the final scale is
 * returned here.
 * @param outEffTint If not nullptr, the final tint color is
 * returned here.
 */
void getSpriteBasicEffects(
    const Point &basePos, float baseAngle,
    float baseAngleCosCache, float baseAngleSinCache,
    Sprite* curSpritePtr, Sprite* nextSpritePtr, float interpolationFactor,
    Point* outEffTrans, float* outEffAngle,
    Point* outEffScale, ALLEGRO_COLOR* outEffTint
) {
    if(baseAngleCosCache == LARGE_FLOAT) {
        baseAngleCosCache = cos(baseAngle);
    }
    if(baseAngleSinCache == LARGE_FLOAT) {
        baseAngleSinCache = sin(baseAngle);
    }
    
    if(outEffTrans) {
        outEffTrans->x =
            basePos.x +
            baseAngleCosCache * curSpritePtr->offset.x -
            baseAngleSinCache * curSpritePtr->offset.y;
        outEffTrans->y =
            basePos.y +
            baseAngleSinCache * curSpritePtr->offset.x +
            baseAngleCosCache * curSpritePtr->offset.y;
    }
    if(outEffAngle) {
        *outEffAngle = baseAngle + curSpritePtr->angle;
    }
    if(outEffScale) {
        *outEffScale = curSpritePtr->scale;
    }
    if(outEffTint) {
        *outEffTint = curSpritePtr->tint;
    }
    
    if(nextSpritePtr && interpolationFactor > 0.0f) {
        Point nextTrans(
            basePos.x +
            baseAngleCosCache * nextSpritePtr->offset.x -
            baseAngleSinCache * nextSpritePtr->offset.y,
            basePos.y +
            baseAngleSinCache * nextSpritePtr->offset.x +
            baseAngleCosCache * nextSpritePtr->offset.y
        );
        float nextAngle = baseAngle + nextSpritePtr->angle;
        Point nextScale = nextSpritePtr->scale;
        ALLEGRO_COLOR nextTint = nextSpritePtr->tint;
        
        if(outEffTrans) {
            *outEffTrans =
                interpolatePoint(
                    interpolationFactor, 0.0f, 1.0f,
                    *outEffTrans, nextTrans
                );
        }
        if(outEffAngle) {
            *outEffAngle =
                interpolateAngle(
                    interpolationFactor, 0.0f, 1.0f,
                    *outEffAngle, nextAngle
                );
        }
        if(outEffScale) {
            *outEffScale =
                interpolatePoint(
                    interpolationFactor, 0.0f, 1.0f,
                    *outEffScale, nextScale
                );
        }
        if(outEffTint) {
            *outEffTint =
                interpolateColor(
                    interpolationFactor, 0.0f, 1.0f,
                    *outEffTint, nextTint
                );
        }
    }
}


/**
 * @brief Returns the final transformation data for a Pikmin top's "basic"
 * sprite effect. i.e. the translation, angle, scale, and tint.
 * This makes use of interpolation between two frames if applicable.
 *
 * @param curSpritePtr The current sprite.
 * @param nextSpritePtr The next sprite, if any.
 * @param interpolationFactor Amount to interpolate the two sprites by, if any.
 * Ranges from 0 to 1.
 * @param outEffTrans If not nullptr, the top's final translation is
 * returned here.
 * @param outEffAngle If not nullptr, the top's final rotation angle is
 * returned here.
 * @param outEffSize If not nullptr, the top's final size is
 * returned here.
 */
void getSpriteBasicTopEffects(
    Sprite* curSpritePtr, Sprite* nextSpritePtr, float interpolationFactor,
    Point* outEffTrans, float* outEffAngle,
    Point* outEffSize
) {
    if(outEffTrans) {
        *outEffTrans = curSpritePtr->topPos;
    }
    if(outEffAngle) {
        *outEffAngle = curSpritePtr->topAngle;
    }
    if(outEffSize) {
        *outEffSize = curSpritePtr->topSize;
    }
    
    if(nextSpritePtr && interpolationFactor > 0.0f) {
        Point nextTrans = nextSpritePtr->topPos;
        float nextAngle = nextSpritePtr->topAngle;
        Point nextSize = nextSpritePtr->topSize;
        
        if(outEffTrans) {
            *outEffTrans =
                interpolatePoint(
                    interpolationFactor, 0.0f, 1.0f,
                    *outEffTrans, nextTrans
                );
        }
        if(outEffAngle) {
            *outEffAngle =
                interpolateAngle(
                    interpolationFactor, 0.0f, 1.0f,
                    *outEffAngle, nextAngle
                );
        }
        if(outEffSize) {
            *outEffSize =
                interpolatePoint(
                    interpolationFactor, 0.0f, 1.0f,
                    *outEffSize, nextSize
                );
        }
    }
}
