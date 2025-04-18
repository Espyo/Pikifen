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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
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
 * @param loop_frame Loop frame index.
 * @param hit_rate If this has an attack, this is the chance of hitting.
 * 0 - 100.
 */
Animation::Animation(
    const string &name, const vector<Frame> &frames,
    size_t loop_frame, unsigned char hit_rate
) :
    name(name),
    frames(frames),
    loopFrame(loop_frame),
    hitRate(hit_rate) {
    
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
 * @param frame_idx The frame index is returned here.
 * @param frame_time The time within the frame is returned here.
 */
void Animation::getFrameAndTime(
    float t, size_t* frame_idx, float* frame_time
) {
    *frame_idx = 0;
    *frame_time = 0.0f;
    
    if(frames.empty() || t <= 0.0f) {
        return;
    }
    
    float duration_so_far = 0.0f;
    float prev_duration_so_far = 0.0f;
    size_t f = 0;
    for(f = 0; f < frames.size(); f++) {
        prev_duration_so_far = duration_so_far;
        duration_so_far += frames[f].duration;
        
        if(duration_so_far > t) {
            break;
        }
    }
    
    *frame_idx = std::clamp(f, (size_t) 0, frames.size() - 1);
    *frame_time = t - prev_duration_so_far;
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
 * @param frame_idx Current frame index.
 * @param frame_time Time in the current frame.
 * @return The time.
 */
float Animation::getTime(size_t frame_idx, float frame_time) {
    if(frame_idx == INVALID) {
        return 0.0f;
    }
    if(frame_idx >= frames.size()) {
        return getDuration();
    }
    
    float cur_time = 0.0f;
    for(size_t f = 0; f < frame_idx; f++) {
        cur_time += frames[f].duration;
    }
    cur_time += frame_time;
    return cur_time;
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
            Hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            float d = Distance(Point(0.0f), h_ptr->pos).toFloat();
            d += h_ptr->radius;
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
        size_t a_pos = findAnimation(conversions[c].second);
        preNamedConversions[conversions[c].first] = a_pos;
        if(a_pos == INVALID) {
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
        Animation* a_ptr = animations[a];
        
        for(size_t f = 0; f < a_ptr->frames.size();) {
            if(a_ptr->frames[f].spriteIdx == idx) {
                a_ptr->deleteFrame(f);
            } else {
                f++;
            }
        }
    }
    
    sprites.erase(sprites.begin() + idx);
    
    for(size_t a = 0; a < animations.size(); a++) {
        Animation* a_ptr = animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
            Frame* f_ptr = &(a_ptr->frames[f]);
            f_ptr->spriteIdx = findSprite(f_ptr->spriteName);
        }
    }
}


/**
 * @brief Fills each frame's sound index cache variable, where applicable.
 *
 * @param mt_ptr Mob type with the sound data.
 */
void AnimationDatabase::fillSoundIdxCaches(MobType* mt_ptr) {
    for(size_t a = 0; a < animations.size(); a++) {
        Animation* a_ptr = animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
            Frame* f_ptr = &a_ptr->frames[f];
            f_ptr->soundIdx = INVALID;
            if(f_ptr->sound.empty()) continue;
            
            for(size_t s = 0; s < mt_ptr->sounds.size(); s++) {
                if(mt_ptr->sounds[s].name == f_ptr->sound) {
                    f_ptr->soundIdx = s;
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
        Sprite* s_ptr = sprites[s];
        for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
            Hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            for(size_t b = 0; b < bodyParts.size(); b++) {
                BodyPart* b_ptr = bodyParts[b];
                if(b_ptr->name == h_ptr->bodyPartName) {
                    h_ptr->bodyPartIdx = b;
                    h_ptr->bodyPartPtr = b_ptr;
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
    DataNode* body_parts_node = node->getChildByName("body_parts");
    size_t n_body_parts = body_parts_node->getNrOfChildren();
    for(size_t b = 0; b < n_body_parts; b++) {
    
        DataNode* body_part_node = body_parts_node->getChild(b);
        
        BodyPart* cur_body_part = new BodyPart(body_part_node->name);
        bodyParts.push_back(cur_body_part);
    }
    
    //Sprites.
    DataNode* sprites_node = node->getChildByName("sprites");
    size_t n_sprites = sprites_node->getNrOfChildren();
    for(size_t s = 0; s < n_sprites; s++) {
    
        DataNode* sprite_node = sprites_node->getChild(s);
        ReaderSetter srs(sprite_node);
        Sprite* new_s = new Sprite(sprite_node->name);
        DataNode* file_node = nullptr;
        
        srs.set("file_pos", new_s->bmpPos);
        srs.set("file_size", new_s->bmpSize);
        srs.set("offset", new_s->offset);
        srs.set("scale", new_s->scale);
        srs.set("angle", new_s->angle);
        srs.set("tint", new_s->tint);
        srs.set("file", new_s->bmpName, &file_node);
        srs.set("top_visible", new_s->topVisible);
        srs.set("top_pos", new_s->topPos);
        srs.set("top_size", new_s->topSize);
        srs.set("top_angle", new_s->topAngle);
        
        //Hitboxes.
        DataNode* hitboxes_node = sprite_node->getChildByName("hitboxes");
        size_t n_hitboxes = hitboxes_node->getNrOfChildren();
        for(size_t h = 0; h < n_hitboxes; h++) {
            DataNode* hitbox_node = hitboxes_node->getChild(h);
            ReaderSetter hrs(hitbox_node);
            Hitbox new_hitbox;
            string coords_str;
            int hitbox_type_int;
            string hazard_str;
            DataNode* hazard_node = nullptr;
            
            hrs.set("coords", coords_str);
            hrs.set("height", new_hitbox.height);
            hrs.set("radius", new_hitbox.radius);
            hrs.set("type", hitbox_type_int);
            hrs.set("value", new_hitbox.value);
            hrs.set("can_pikmin_latch", new_hitbox.canPikminLatch);
            hrs.set("knockback_outward", new_hitbox.knockbackOutward);
            hrs.set("knockback_angle", new_hitbox.knockbackAngle);
            hrs.set("knockback", new_hitbox.knockback);
            hrs.set("wither_chance", new_hitbox.witherChance);
            hrs.set("hazard", hazard_str, &hazard_node);
            
            new_hitbox.bodyPartName = hitbox_node->name;
            new_hitbox.pos = s2p(coords_str, &new_hitbox.z);
            new_hitbox.type = (HITBOX_TYPE) hitbox_type_int;
            if(!hazard_str.empty()) {
                auto hazard_it = game.content.hazards.list.find(hazard_str);
                if(hazard_it != game.content.hazards.list.end()) {
                    new_hitbox.hazard = &(hazard_it->second);
                } else {
                    game.errors.report(
                        "Unknown hazard \"" + hazard_str + "\"!",
                        hazard_node
                    );
                }
            }
            
            new_s->hitboxes.push_back(new_hitbox);
            
        }
        
        new_s->setBitmap(
            new_s->bmpName, new_s->bmpPos, new_s->bmpSize,
            file_node
        );
        
        sprites.push_back(new_s);
    }
    
    //Animations.
    DataNode* anims_node = node->getChildByName("animations");
    size_t n_anims = anims_node->getNrOfChildren();
    for(size_t a = 0; a < n_anims; a++) {
    
        DataNode* anim_node = anims_node->getChild(a);
        ReaderSetter ars(anim_node);
        Animation* new_a = new Animation(anim_node->name);
        
        ars.set("loop_frame", new_a->loopFrame);
        ars.set("hit_rate", new_a->hitRate);
        
        new_a->name = anim_node->name;
        
        //Frames.
        DataNode* frames_node = anim_node->getChildByName("frames");
        size_t n_frames = frames_node->getNrOfChildren();
        for(size_t f = 0; f < n_frames; f++) {
            DataNode* frame_node = frames_node->getChild(f);
            ReaderSetter frs(frame_node);
            Frame new_frame;
            string signal_str;
            
            frs.set("signal", signal_str);
            frs.set("duration", new_frame.duration);
            frs.set("interpolate", new_frame.interpolate);
            frs.set("sound", new_frame.sound);
            frs.set("sound", new_frame.sound);
            
            new_frame.spriteName = frame_node->name;
            new_frame.spriteIdx = findSprite(frame_node->name);
            new_frame.spritePtr =
                new_frame.spriteIdx == INVALID ?
                nullptr :
                sprites[new_frame.spriteIdx];
            new_frame.signal = signal_str.empty() ? INVALID : s2i(signal_str);
            
            new_a->frames.push_back(new_frame);
        }
        
        animations.push_back(new_a);
    }
    
    //Finish up.
    fixBodyPartPointers();
    calculateHitboxSpan();
}


/**
 * @brief Saves the animation database data to a data node.
 *
 * @param node Data node to save to.
 * @param save_top_data Whether to save the Pikmin top's data.
 */
void AnimationDatabase::saveToDataNode(
    DataNode* node, bool save_top_data
) {
    //Content metadata.
    saveMetadataToDataNode(node);
    
    //Animations.
    DataNode* animations_node = node->addNew("animations");
    for(size_t a = 0; a < animations.size(); a++) {
    
        //Animation.
        Animation* a_ptr = animations[a];
        DataNode* anim_node = animations_node->addNew(a_ptr->name);
        GetterWriter agw(anim_node);
        
        if(a_ptr->loopFrame > 0) {
            agw.write("loop_frame", a_ptr->loopFrame);
        }
        if(a_ptr->hitRate != 100) {
            agw.write("hit_rate", a_ptr->hitRate);
        }
        
        //Frames.
        DataNode* frames_node = anim_node->addNew("frames");
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
        
            //Frame.
            Frame* f_ptr = &a_ptr->frames[f];
            DataNode* frame_node = frames_node->addNew(f_ptr->spriteName);
            GetterWriter fgw(frame_node);
            
            fgw.write("duration", f_ptr->duration);
            if(f_ptr->interpolate) {
                fgw.write("interpolate", f_ptr->interpolate);
            }
            if(f_ptr->signal != INVALID) {
                fgw.write("signal", f_ptr->signal);
            }
            if(!f_ptr->sound.empty() && f_ptr->sound != NONE_OPTION) {
                fgw.write("sound", f_ptr->sound);
            }
        }
    }
    
    //Sprites.
    DataNode* sprites_node = node->addNew("sprites");
    for(size_t s = 0; s < sprites.size(); s++) {
    
        //Sprite.
        Sprite* s_ptr = sprites[s];
        DataNode* sprite_node = sprites_node->addNew(sprites[s]->name);
        GetterWriter sgw(sprite_node);
        
        sgw.write("file", s_ptr->bmpName);
        sgw.write("file_pos", s_ptr->bmpPos);
        sgw.write("file_size", s_ptr->bmpSize);
        if(s_ptr->offset.x != 0.0 || s_ptr->offset.y != 0.0) {
            sgw.write("offset", s_ptr->offset);
        }
        if(s_ptr->scale.x != 1.0 || s_ptr->scale.y != 1.0) {
            sgw.write("scale", s_ptr->scale);
        }
        if(s_ptr->angle != 0.0) {
            sgw.write("angle", s_ptr->angle);
        }
        if(s_ptr->tint != COLOR_WHITE) {
            sgw.write("tint", s_ptr->tint);
        }
        
        if(save_top_data) {
            sgw.write("top_visible", s_ptr->topVisible);
            sgw.write("top_pos", s_ptr->topPos);
            sgw.write("top_size", s_ptr->topSize);
            sgw.write("top_angle", s_ptr->topAngle);
        }
        
        if(!s_ptr->hitboxes.empty()) {
        
            //Hitboxes.
            DataNode* hitboxes_node = sprite_node->addNew("hitboxes");
            for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
            
                //Hitbox.
                Hitbox* h_ptr = &s_ptr->hitboxes[h];
                DataNode* hitbox_node = hitboxes_node->addNew(h_ptr->bodyPartName);
                GetterWriter hgw(hitbox_node);
                
                hgw.write("coords", p2s(h_ptr->pos, &h_ptr->z));
                hgw.write("height", h_ptr->height);
                hgw.write("radius", h_ptr->radius);
                hgw.write("type", h_ptr->type);
                hgw.write("value", h_ptr->value);
                if(
                    h_ptr->type == HITBOX_TYPE_NORMAL &&
                    h_ptr->canPikminLatch
                ) {
                    hgw.write("can_pikmin_latch", h_ptr->canPikminLatch);
                }
                if(h_ptr->hazard) {
                    hgw.write("hazard", h_ptr->hazard->manifest->internalName);
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockbackOutward
                ) {
                    hgw.write("knockback_outward", h_ptr->knockbackOutward);
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockbackAngle != 0
                ) {
                    hgw.write("knockback_angle", h_ptr->knockbackAngle);
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback != 0
                ) {
                    hgw.write("knockback", h_ptr->knockback);
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->witherChance > 0
                ) {
                    hgw.write("wither_chance", h_ptr->witherChance);
                }
            }
        }
    }
    
    //Body parts.
    DataNode* body_parts_node = node->addNew("body_parts");
    for(size_t b = 0; b < bodyParts.size(); b++) {
    
        //Body part.
        BodyPart* b_ptr = bodyParts[b];
        body_parts_node->addNew(b_ptr->name);
        
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
        Animation* a_ptr = animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
            Frame* f_ptr = &(a_ptr->frames[f]);
            
            f_ptr->spriteIdx = findSprite(f_ptr->spriteName);
        }
    }
}


/**
 * @brief Constructs a new animation instance object.
 *
 * @param anim_db The animation database. Used when changing animations.
 */
AnimationInstance::AnimationInstance(AnimationDatabase* anim_db) :
    curAnim(nullptr),
    animDb(anim_db) {
    
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
 * @param out_cur_sprite_ptr If not nullptr, the current sprite is
 * returned here.
 * @param out_next_sprite_ptr If not nullptr, the next sprite in the animation
 * is returned here.
 * @param out_interpolation_factor If not nullptr, the interpolation factor
 * (0 to 1) between the current sprite and the next one is returned here.
 */
void AnimationInstance::getSpriteData(
    Sprite** out_cur_sprite_ptr, Sprite** out_next_sprite_ptr,
    float* out_interpolation_factor
) const {
    if(!validFrame()) {
        if(out_cur_sprite_ptr) *out_cur_sprite_ptr = nullptr;
        if(out_next_sprite_ptr) *out_next_sprite_ptr = nullptr;
        if(out_interpolation_factor) *out_interpolation_factor = 0.0f;
        return;
    }
    
    Frame* cur_frame_ptr = &curAnim->frames[curFrameIdx];
    //First, the basics -- the current sprite.
    if(out_cur_sprite_ptr) {
        *out_cur_sprite_ptr = cur_frame_ptr->spritePtr;
    }
    
    //Now only bother with interpolation data if we actually need it.
    if(!out_next_sprite_ptr && !out_interpolation_factor) return;
    
    if(!cur_frame_ptr->interpolate) {
        //This frame doesn't even interpolate.
        if(out_next_sprite_ptr) *out_next_sprite_ptr = cur_frame_ptr->spritePtr;
        if(out_interpolation_factor) *out_interpolation_factor = 0.0f;
        return;
    }
    
    //Get the next sprite.
    size_t next_frame_idx = getNextFrameIdx();
    Frame* next_frame_ptr = &curAnim->frames[next_frame_idx];
    
    if(out_next_sprite_ptr) *out_next_sprite_ptr = next_frame_ptr->spritePtr;
    
    //Get the interpolation factor.
    if(out_interpolation_factor) {
        if(cur_frame_ptr->duration == 0.0f) {
            *out_interpolation_factor = 0.0f;
        } else {
            *out_interpolation_factor =
                curFrameTime /
                cur_frame_ptr->duration;
        }
    }
}


/**
 * @brief Returns the index of the next frame of animation, the one after
 * the current one.
 *
 * @param out_reached_end If not nullptr, true is returned here if we've reached
 * the end and the next frame loops back to the beginning.
 * @return The index, or INVALID on error.
 */
size_t AnimationInstance::getNextFrameIdx(bool* out_reached_end) const {
    if(out_reached_end) *out_reached_end = false;
    if(!curAnim) return INVALID;
    
    if(curFrameIdx < curAnim->frames.size() - 1) {
        return curFrameIdx + 1;
    } else {
        if(out_reached_end) *out_reached_end = true;
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
    
    float total_duration = 0;
    for(size_t f = 0; f < curAnim->frames.size(); f++) {
        total_duration += curAnim->frames[f].duration;
    }
    
    tick(game.rng.f(0, total_duration));
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
 * @param delta_t How long the frame's tick is, in seconds.
 * @param signals Any frame that sends a signal adds it here.
 * @param sounds Any frame that should play a sound adds it here.
 * @return Whether or not the animation ended its final frame.
 */
bool AnimationInstance::tick(
    float delta_t, vector<size_t>* signals,
    vector<size_t>* sounds
) {
    if(!curAnim) return false;
    size_t n_frames = curAnim->frames.size();
    if(n_frames == 0) return false;
    Frame* cur_frame = &curAnim->frames[curFrameIdx];
    if(cur_frame->duration == 0) {
        return true;
    }
    
    curFrameTime += delta_t;
    
    bool reached_end = false;
    
    //This is a while instead of an if because if the framerate is too low
    //and the next frame's duration is too short, it could be that a tick
    //goes over an entire frame, and lands 2 or more frames ahead.
    while(curFrameTime > cur_frame->duration && cur_frame->duration != 0) {
        curFrameTime = curFrameTime - cur_frame->duration;
        bool reached_end_now = false;
        curFrameIdx = getNextFrameIdx(&reached_end_now);
        reached_end |= reached_end_now;
        cur_frame = &curAnim->frames[curFrameIdx];
        if(cur_frame->signal != INVALID && signals) {
            signals->push_back(cur_frame->signal);
        }
        if(cur_frame->soundIdx != INVALID && sounds) {
            sounds->push_back(cur_frame->soundIdx);
        }
    }
    
    return reached_end;
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
 * @param b_pos X and Y of the top-left corner of the sprite, in the
 * parent's bitmap.
 * @param b_size Width and height of the sprite, in the parent's bitmap.
 * @param h List of hitboxes.
 */
Sprite::Sprite(
    const string &name, ALLEGRO_BITMAP* const b, const Point &b_pos,
    const Point &b_size, const vector<Hitbox> &h
) :
    name(name),
    parentBmp(b),
    bmpPos(b_pos),
    bmpSize(b_size),
    bitmap(
        b ?
        al_create_sub_bitmap(b, b_pos.x, b_pos.y, b_size.x, b_size.y) :
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
 * @param new_bmp_name Internal name of the bitmap.
 * @param new_bmp_pos Top-left coordinates of the sub-bitmap inside the bitmap.
 * @param new_bmp_size Dimensions of the sub-bitmap.
 * @param node If not nullptr, this will be used to report an error with,
 * in case something happens.
 */
void Sprite::setBitmap(
    const string &new_bmp_name,
    const Point &new_bmp_pos, const Point &new_bmp_size,
    DataNode* node
) {
    if(bitmap) {
        al_destroy_bitmap(bitmap);
        bitmap = nullptr;
    }
    if(new_bmp_name != bmpName && parentBmp) {
        game.content.bitmaps.list.free(bmpName);
        parentBmp = nullptr;
    }
    
    if(new_bmp_name.empty()) {
        bmpName.clear();
        bmpSize = Point();
        bmpPos = Point();
        return;
    }
    
    if(new_bmp_name != bmpName || !parentBmp) {
        parentBmp = game.content.bitmaps.list.get(new_bmp_name, node, node != nullptr);
    }
    
    Point parent_size = getBitmapDimensions(parentBmp);
    
    bmpName = new_bmp_name;
    bmpPos = new_bmp_pos;
    bmpSize = new_bmp_size;
    bmpPos.x = std::clamp(new_bmp_pos.x, 0.0f, parent_size.x - 1);
    bmpPos.y = std::clamp(new_bmp_pos.y, 0.0f, parent_size.y - 1);
    bmpSize.x = std::clamp(new_bmp_size.x, 0.0f, parent_size.x - bmpPos.x);
    bmpSize.y = std::clamp(new_bmp_size.y, 0.0f, parent_size.y - bmpPos.y);
    
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
 * @param base_pos Base position of the translation.
 * @param base_angle Base angle of the rotation.
 * @param base_angle_cos_cache If you have a cached value for the base angle's
 * cosine, write it here. Otherwise use LARGE_FLOAT.
 * @param base_angle_sin_cache If you have a cached value for the base angle's
 * sine, write it here. Otherwise use LARGE_FLOAT.
 * @param cur_s_ptr The current sprite.
 * @param next_s_ptr The next sprite, if any.
 * @param interpolation_factor Amount to interpolate the two sprites by, if any.
 * Ranges from 0 to 1.
 * @param out_eff_trans If not nullptr, the final translation is
 * returned here.
 * @param out_eff_angle If not nullptr, the final rotation angle is
 * returned here.
 * @param out_eff_scale If not nullptr, the final scale is
 * returned here.
 * @param out_eff_tint If not nullptr, the final tint color is
 * returned here.
 */
void getSpriteBasicEffects(
    const Point &base_pos, float base_angle,
    float base_angle_cos_cache, float base_angle_sin_cache,
    Sprite* cur_s_ptr, Sprite* next_s_ptr, float interpolation_factor,
    Point* out_eff_trans, float* out_eff_angle,
    Point* out_eff_scale, ALLEGRO_COLOR* out_eff_tint
) {
    if(base_angle_cos_cache == LARGE_FLOAT) {
        base_angle_cos_cache = cos(base_angle);
    }
    if(base_angle_sin_cache == LARGE_FLOAT) {
        base_angle_sin_cache = sin(base_angle);
    }
    
    if(out_eff_trans) {
        out_eff_trans->x =
            base_pos.x +
            base_angle_cos_cache * cur_s_ptr->offset.x -
            base_angle_sin_cache * cur_s_ptr->offset.y;
        out_eff_trans->y =
            base_pos.y +
            base_angle_sin_cache * cur_s_ptr->offset.x +
            base_angle_cos_cache * cur_s_ptr->offset.y;
    }
    if(out_eff_angle) {
        *out_eff_angle = base_angle + cur_s_ptr->angle;
    }
    if(out_eff_scale) {
        *out_eff_scale = cur_s_ptr->scale;
    }
    if(out_eff_tint) {
        *out_eff_tint = cur_s_ptr->tint;
    }
    
    if(next_s_ptr && interpolation_factor > 0.0f) {
        Point next_trans(
            base_pos.x +
            base_angle_cos_cache * next_s_ptr->offset.x -
            base_angle_sin_cache * next_s_ptr->offset.y,
            base_pos.y +
            base_angle_sin_cache * next_s_ptr->offset.x +
            base_angle_cos_cache * next_s_ptr->offset.y
        );
        float next_angle = base_angle + next_s_ptr->angle;
        Point next_scale = next_s_ptr->scale;
        ALLEGRO_COLOR next_tint = next_s_ptr->tint;
        
        if(out_eff_trans) {
            *out_eff_trans =
                interpolatePoint(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_trans, next_trans
                );
        }
        if(out_eff_angle) {
            *out_eff_angle =
                interpolateAngle(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_angle, next_angle
                );
        }
        if(out_eff_scale) {
            *out_eff_scale =
                interpolatePoint(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_scale, next_scale
                );
        }
        if(out_eff_tint) {
            *out_eff_tint =
                interpolateColor(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_tint, next_tint
                );
        }
    }
}


/**
 * @brief Returns the final transformation data for a Pikmin top's "basic"
 * sprite effect. i.e. the translation, angle, scale, and tint.
 * This makes use of interpolation between two frames if applicable.
 *
 * @param cur_s_ptr The current sprite.
 * @param next_s_ptr The next sprite, if any.
 * @param interpolation_factor Amount to interpolate the two sprites by, if any.
 * Ranges from 0 to 1.
 * @param out_eff_trans If not nullptr, the top's final translation is
 * returned here.
 * @param out_eff_angle If not nullptr, the top's final rotation angle is
 * returned here.
 * @param out_eff_size If not nullptr, the top's final size is
 * returned here.
 */
void getSpriteBasicTopEffects(
    Sprite* cur_s_ptr, Sprite* next_s_ptr, float interpolation_factor,
    Point* out_eff_trans, float* out_eff_angle,
    Point* out_eff_size
) {
    if(out_eff_trans) {
        *out_eff_trans = cur_s_ptr->topPos;
    }
    if(out_eff_angle) {
        *out_eff_angle = cur_s_ptr->topAngle;
    }
    if(out_eff_size) {
        *out_eff_size = cur_s_ptr->topSize;
    }
    
    if(next_s_ptr && interpolation_factor > 0.0f) {
        Point next_trans = next_s_ptr->topPos;
        float next_angle = next_s_ptr->topAngle;
        Point next_size = next_s_ptr->topSize;
        
        if(out_eff_trans) {
            *out_eff_trans =
                interpolatePoint(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_trans, next_trans
                );
        }
        if(out_eff_angle) {
            *out_eff_angle =
                interpolateAngle(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_angle, next_angle
                );
        }
        if(out_eff_size) {
            *out_eff_size =
                interpolatePoint(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_size, next_size
                );
        }
    }
}
