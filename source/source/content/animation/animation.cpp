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
animation::animation(
    const string &name, const vector<frame> &frames,
    size_t loop_frame, unsigned char hit_rate
) :
    name(name),
    frames(frames),
    loop_frame(loop_frame),
    hit_rate(hit_rate) {
    
}


/**
 * @brief Constructs a new animation object.
 *
 * @param a2 The other animation.
 */
animation::animation(const animation &a2) :
    name(a2.name),
    frames(a2.frames),
    loop_frame(a2.loop_frame),
    hit_rate(a2.hit_rate) {
}


/**
 * @brief Creates an animation by copying info from another animation.
 *
 * @param a2 The other animation.
 * @return The current object.
 */
animation &animation::operator=(const animation &a2) {
    if(this != &a2) {
        name = a2.name;
        frames = a2.frames;
        loop_frame = a2.loop_frame;
        hit_rate = a2.hit_rate;
    }
    
    return *this;
}


/**
 * @brief Deletes one of the animation's frames.
 *
 * @param idx Frame index.
 */
void animation::delete_frame(size_t idx) {
    if(idx == INVALID) return;
    
    if(idx < loop_frame) {
        //Let the loop frame stay the same.
        loop_frame--;
    }
    if(
        idx == loop_frame &&
        loop_frame == frames.size() - 1
    ) {
        //Stop the loop frame from going out of bounds.
        loop_frame--;
    }
    frames.erase(frames.begin() + idx);
}


/**
 * @brief Returns the total duration of the animation.
 *
 * @return The duration.
 */
float animation::get_duration() {
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
void animation::get_frame_and_time(
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
    
    *frame_idx = clamp(f, 0, frames.size() - 1);
    *frame_time = t - prev_duration_so_far;
}


/**
 * @brief Returns the total duration of the loop segment of the animation.
 *
 * @return The duration.
 */
float animation::get_loop_duration() {
    float duration = 0.0f;
    for(size_t f = loop_frame; f < frames.size(); f++) {
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
float animation::get_time(size_t frame_idx, float frame_time) {
    if(frame_idx == INVALID) {
        return 0.0f;
    }
    if(frame_idx >= frames.size()) {
        return get_duration();
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
animation_database::animation_database(
    const vector<animation*> &a, const vector<sprite*> &s,
    const vector<body_part*> &b
) :
    animations(a),
    sprites(s),
    body_parts(b) {
    
}


/**
 * @brief Calculates the maximum distance that any of its hitbox can reach,
 * and stores it in the hitbox_span variable.
 */
void animation_database::calculate_hitbox_span() {
    hitbox_span = 0.0f;
    for(size_t s = 0; s < sprites.size(); s++) {
        sprite* s_ptr = sprites[s];
        for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
            hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            float d = dist(point(0.0f), h_ptr->pos).to_float();
            d += h_ptr->radius;
            hitbox_span = std::max(hitbox_span, d);
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
void animation_database::create_conversions(
    const vector<std::pair<size_t, string> > &conversions,
    const data_node* file
) {
    pre_named_conversions.clear();
    
    if(conversions.empty()) return;
    
    //First, find the highest index.
    size_t highest = conversions[0].first;
    for(size_t c = 1; c < conversions.size(); c++) {
        highest = std::max(highest, conversions[c].first);
    }
    
    pre_named_conversions.assign(highest + 1, INVALID);
    
    for(size_t c = 0; c < conversions.size(); c++) {
        size_t a_pos = find_animation(conversions[c].second);
        pre_named_conversions[conversions[c].first] = a_pos;
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
void animation_database::destroy() {
    reset_metadata();
    for(size_t a = 0; a < animations.size(); a++) {
        delete animations[a];
    }
    for(size_t s = 0; s < sprites.size(); s++) {
        delete sprites[s];
    }
    for(size_t b = 0; b < body_parts.size(); b++) {
        delete body_parts[b];
    }
    animations.clear();
    sprites.clear();
    body_parts.clear();
}


/**
 * @brief Deletes a sprite, adjusting any animations that use it.
 *
 * @param idx Sprite index.
 */
void animation_database::delete_sprite(size_t idx) {
    for(size_t a = 0; a < animations.size(); a++) {
        animation* a_ptr = animations[a];
        
        for(size_t f = 0; f < a_ptr->frames.size();) {
            if(a_ptr->frames[f].sprite_idx == idx) {
                a_ptr->delete_frame(f);
            } else {
                f++;
            }
        }
    }
    
    sprites.erase(sprites.begin() + idx);
    
    for(size_t a = 0; a < animations.size(); a++) {
        animation* a_ptr = animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
            frame* f_ptr = &(a_ptr->frames[f]);
            f_ptr->sprite_idx = find_sprite(f_ptr->sprite_name);
        }
    }
}


/**
 * @brief Fills each frame's sound index cache variable, where applicable.
 *
 * @param mt_ptr Mob type with the sound data.
 */
void animation_database::fill_sound_idx_caches(mob_type* mt_ptr) {
    for(size_t a = 0; a < animations.size(); a++) {
        animation* a_ptr = animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
            frame* f_ptr = &a_ptr->frames[f];
            f_ptr->sound_idx = INVALID;
            if(f_ptr->sound.empty()) continue;
            
            for(size_t s = 0; s < mt_ptr->sounds.size(); s++) {
                if(mt_ptr->sounds[s].name == f_ptr->sound) {
                    f_ptr->sound_idx = s;
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
size_t animation_database::find_animation(const string &name) const {
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
size_t animation_database::find_body_part(const string &name) const {
    for(size_t b = 0; b < body_parts.size(); b++) {
        if(body_parts[b]->name == name) return b;
    }
    return INVALID;
}


/**
 * @brief Returns the index of the specified sprite.
 *
 * @param name Name of the sprite to search for.
 * @return The index, or INVALID if not found.
 */
size_t animation_database::find_sprite(const string &name) const {
    for(size_t s = 0; s < sprites.size(); s++) {
        if(sprites[s]->name == name) return s;
    }
    return INVALID;
}


/**
 * @brief Fixes the pointers for body parts.
 */
void animation_database::fix_body_part_pointers() {
    for(size_t s = 0; s < sprites.size(); s++) {
        sprite* s_ptr = sprites[s];
        for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
            hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            for(size_t b = 0; b < body_parts.size(); b++) {
                body_part* b_ptr = body_parts[b];
                if(b_ptr->name == h_ptr->body_part_name) {
                    h_ptr->body_part_idx = b;
                    h_ptr->body_part_ptr = b_ptr;
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
void animation_database::load_from_data_node(data_node* node) {
    //Content metadata.
    load_metadata_from_data_node(node);
    
    //Body parts.
    data_node* body_parts_node = node->get_child_by_name("body_parts");
    size_t n_body_parts = body_parts_node->get_nr_of_children();
    for(size_t b = 0; b < n_body_parts; b++) {
    
        data_node* body_part_node = body_parts_node->get_child(b);
        
        body_part* cur_body_part = new body_part(body_part_node->name);
        body_parts.push_back(cur_body_part);
    }
    
    //Sprites.
    data_node* sprites_node = node->get_child_by_name("sprites");
    size_t n_sprites = sprites_node->get_nr_of_children();
    for(size_t s = 0; s < n_sprites; s++) {
    
        data_node* sprite_node = sprites_node->get_child(s);
        vector<hitbox> hitboxes;
        
        data_node* hitboxes_node =
            sprite_node->get_child_by_name("hitboxes");
        size_t n_hitboxes = hitboxes_node->get_nr_of_children();
        
        for(size_t h = 0; h < n_hitboxes; h++) {
        
            data_node* hitbox_node =
                hitboxes_node->get_child(h);
            hitbox cur_hitbox = hitbox();
            
            cur_hitbox.pos =
                s2p(
                    hitbox_node->get_child_by_name("coords")->value,
                    &cur_hitbox.z
                );
            cur_hitbox.height =
                s2f(hitbox_node->get_child_by_name("height")->value);
            cur_hitbox.radius =
                s2f(hitbox_node->get_child_by_name("radius")->value);
            cur_hitbox.body_part_name =
                hitbox_node->name;
            cur_hitbox.type =
                (HITBOX_TYPE)
                s2i(hitbox_node->get_child_by_name("type")->value);
            cur_hitbox.value =
                s2f(
                    hitbox_node->get_child_by_name("value")->value
                );
            cur_hitbox.can_pikmin_latch =
                s2b(
                    hitbox_node->get_child_by_name(
                        "can_pikmin_latch"
                    )->get_value_or_default("false")
                );
            cur_hitbox.knockback_outward =
                s2b(
                    hitbox_node->get_child_by_name(
                        "knockback_outward"
                    )->get_value_or_default("false")
                );
            cur_hitbox.knockback_angle =
                s2f(hitbox_node->get_child_by_name("knockback_angle")->value);
            cur_hitbox.knockback =
                s2f(
                    hitbox_node->get_child_by_name(
                        "knockback"
                    )->get_value_or_default("0")
                );
            cur_hitbox.wither_chance =
                s2i(
                    hitbox_node->get_child_by_name("wither_chance")->value
                );
                
            data_node* hazards_node =
                hitbox_node->get_child_by_name("hazards");
            cur_hitbox.hazards_str = hazards_node->value;
            vector<string> hazards_strs =
                semicolon_list_to_vector(cur_hitbox.hazards_str);
            for(size_t hs = 0; hs < hazards_strs.size(); hs++) {
                const string &hazard_name = hazards_strs[hs];
                if(game.content.hazards.list.find(hazard_name) == game.content.hazards.list.end()) {
                    game.errors.report(
                        "Unknown hazard \"" + hazard_name + "\"!",
                        hazards_node
                    );
                } else {
                    cur_hitbox.hazards.push_back(
                        &(game.content.hazards.list[hazard_name])
                    );
                }
            }
            
            
            hitboxes.push_back(cur_hitbox);
            
        }
        
        sprite* new_s =
            new sprite(
            sprite_node->name,
            nullptr,
            s2p(sprite_node->get_child_by_name("file_pos")->value),
            s2p(sprite_node->get_child_by_name("file_size")->value),
            hitboxes
        );
        sprites.push_back(new_s);
        
        new_s->offset = s2p(sprite_node->get_child_by_name("offset")->value);
        new_s->scale =
            s2p(
                sprite_node->get_child_by_name(
                    "scale"
                )->get_value_or_default("1 1")
            );
        new_s->angle = s2f(sprite_node->get_child_by_name("angle")->value);
        new_s->tint =
            s2c(
                sprite_node->get_child_by_name("tint")->get_value_or_default(
                    "255 255 255 255"
                )
            );
        new_s->file = sprite_node->get_child_by_name("file")->value;
        new_s->set_bitmap(
            new_s->file, new_s->file_pos, new_s->file_size,
            sprite_node->get_child_by_name("file")
        );
        new_s->top_visible =
            s2b(
                sprite_node->get_child_by_name("top_visible")->value
            );
        new_s->top_pos =
            s2p(sprite_node->get_child_by_name("top_pos")->value);
        new_s->top_size =
            s2p(sprite_node->get_child_by_name("top_size")->value);
        new_s->top_angle =
            s2f(
                sprite_node->get_child_by_name("top_angle")->value
            );
    }
    
    //Animations.
    data_node* anims_node = node->get_child_by_name("animations");
    size_t n_anims = anims_node->get_nr_of_children();
    for(size_t a = 0; a < n_anims; a++) {
    
        data_node* anim_node = anims_node->get_child(a);
        vector<frame> frames;
        
        data_node* frames_node =
            anim_node->get_child_by_name("frames");
        size_t n_frames =
            frames_node->get_nr_of_children();
            
        for(size_t f = 0; f < n_frames; f++) {
            data_node* frame_node = frames_node->get_child(f);
            size_t s_pos = find_sprite(frame_node->name);
            string signal_str =
                frame_node->get_child_by_name("signal")->value;
            frames.push_back(
                frame(
                    frame_node->name,
                    s_pos,
                    (s_pos == INVALID) ? nullptr : sprites[s_pos],
                    s2f(frame_node->get_child_by_name("duration")->value),
                    s2b(frame_node->get_child_by_name("interpolate")->value),
                    frame_node->get_child_by_name("sound")->value,
                    (signal_str.empty() ? INVALID : s2i(signal_str))
                )
            );
        }
        
        animations.push_back(
            new animation(
                anim_node->name,
                frames,
                s2i(anim_node->get_child_by_name("loop_frame")->value),
                s2i(
                    anim_node->get_child_by_name(
                        "hit_rate"
                    )->get_value_or_default("100")
                )
            )
        );
    }
    
    //Finish up.
    fix_body_part_pointers();
    calculate_hitbox_span();
}


/**
 * @brief Saves the animation database data to a data node.
 *
 * @param node Data node to save to.
 * @param save_top_data Whether to save the Pikmin top's data.
 */
void animation_database::save_to_data_node(
    data_node* node, bool save_top_data
) {
    //Content metadata.
    save_metadata_to_data_node(node);
    
    //Animations.
    data_node* animations_node = new data_node("animations", "");
    node->add(animations_node);
    
    for(size_t a = 0; a < animations.size(); a++) {
        data_node* anim_node = new data_node(animations[a]->name, "");
        animations_node->add(anim_node);
        
        if(animations[a]->loop_frame > 0) {
            anim_node->add(
                new data_node(
                    "loop_frame", i2s(animations[a]->loop_frame)
                )
            );
        }
        if(animations[a]->hit_rate != 100) {
            anim_node->add(
                new data_node("hit_rate", i2s(animations[a]->hit_rate))
            );
        }
        data_node* frames_node = new data_node("frames", "");
        anim_node->add(frames_node);
        
        for(size_t f = 0; f < animations[a]->frames.size(); f++) {
            frame* f_ptr = &animations[a]->frames[f];
            
            data_node* frame_node =
                new data_node(f_ptr->sprite_name, "");
            frames_node->add(frame_node);
            
            frame_node->add(
                new data_node("duration", f2s(f_ptr->duration))
            );
            if(f_ptr->interpolate) {
                frame_node->add(
                    new data_node("interpolate", b2s(f_ptr->interpolate))
                );
            }
            if(f_ptr->signal != INVALID) {
                frame_node->add(
                    new data_node("signal", i2s(f_ptr->signal))
                );
            }
            if(!f_ptr->sound.empty() && f_ptr->sound != NONE_OPTION) {
                frame_node->add(
                    new data_node("sound", f_ptr->sound)
                );
            }
        }
    }
    
    //Sprites.
    data_node* sprites_node = new data_node("sprites", "");
    node->add(sprites_node);
    
    for(size_t s = 0; s < sprites.size(); s++) {
        sprite* s_ptr = sprites[s];
        data_node* sprite_node = new data_node(sprites[s]->name, "");
        sprites_node->add(sprite_node);
        
        sprite_node->add(new data_node("file",      s_ptr->file));
        sprite_node->add(new data_node("file_pos",  p2s(s_ptr->file_pos)));
        sprite_node->add(new data_node("file_size", p2s(s_ptr->file_size)));
        if(s_ptr->offset.x != 0.0 || s_ptr->offset.y != 0.0) {
            sprite_node->add(new data_node("offset", p2s(s_ptr->offset)));
        }
        if(s_ptr->scale.x != 1.0 || s_ptr->scale.y != 1.0) {
            sprite_node->add(new data_node("scale", p2s(s_ptr->scale)));
        }
        if(s_ptr->angle != 0.0) {
            sprite_node->add(new data_node("angle", f2s(s_ptr->angle)));
        }
        if(s_ptr->tint != COLOR_WHITE) {
            sprite_node->add(new data_node("tint", c2s(s_ptr->tint)));
        }
        
        if(save_top_data) {
            sprite_node->add(
                new data_node("top_visible", b2s(s_ptr->top_visible))
            );
            sprite_node->add(
                new data_node("top_pos", p2s(s_ptr->top_pos))
            );
            sprite_node->add(
                new data_node("top_size", p2s(s_ptr->top_size))
            );
            sprite_node->add(
                new data_node("top_angle", f2s(s_ptr->top_angle))
            );
        }
        
        if(!s_ptr->hitboxes.empty()) {
            data_node* hitboxes_node =
                new data_node("hitboxes", "");
            sprite_node->add(hitboxes_node);
            
            for(size_t h = 0; h < s_ptr->hitboxes.size(); h++) {
                hitbox* h_ptr = &s_ptr->hitboxes[h];
                
                data_node* hitbox_node =
                    new data_node(h_ptr->body_part_name, "");
                hitboxes_node->add(hitbox_node);
                
                hitbox_node->add(
                    new data_node("coords", p2s(h_ptr->pos, &h_ptr->z))
                );
                hitbox_node->add(
                    new data_node("height", f2s(h_ptr->height))
                );
                hitbox_node->add(
                    new data_node("radius", f2s(h_ptr->radius))
                );
                hitbox_node->add(
                    new data_node("type", i2s(h_ptr->type))
                );
                hitbox_node->add(
                    new data_node("value", f2s(h_ptr->value))
                );
                if(
                    h_ptr->type == HITBOX_TYPE_NORMAL &&
                    h_ptr->can_pikmin_latch
                ) {
                    hitbox_node->add(
                        new data_node(
                            "can_pikmin_latch", b2s(h_ptr->can_pikmin_latch)
                        )
                    );
                }
                if(!h_ptr->hazards_str.empty()) {
                    hitbox_node->add(
                        new data_node("hazards", h_ptr->hazards_str)
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback_outward
                ) {
                    hitbox_node->add(
                        new data_node(
                            "knockback_outward",
                            b2s(h_ptr->knockback_outward)
                        )
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback_angle != 0
                ) {
                    hitbox_node->add(
                        new data_node(
                            "knockback_angle", f2s(h_ptr->knockback_angle)
                        )
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback != 0
                ) {
                    hitbox_node->add(
                        new data_node("knockback", f2s(h_ptr->knockback))
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->wither_chance > 0
                ) {
                    hitbox_node->add(
                        new data_node(
                            "wither_chance", i2s(h_ptr->wither_chance)
                        )
                    );
                }
            }
        }
    }
    
    //Body parts.
    data_node* body_parts_node = new data_node("body_parts", "");
    node->add(body_parts_node);
    
    for(size_t b = 0; b < body_parts.size(); b++) {
        data_node* body_part_node =
            new data_node(body_parts[b]->name, "");
        body_parts_node->add(body_part_node);
        
    }
}


/**
 * @brief Sorts all animations and sprites alphabetically,
 * making them more organized.
 */
void animation_database::sort_alphabetically() {
    sort(
        animations.begin(), animations.end(),
    [] (const animation * a1, const animation * a2) {
        return a1->name < a2->name;
    }
    );
    sort(
        sprites.begin(), sprites.end(),
    [] (const sprite * s1, const sprite * s2) {
        return s1->name < s2->name;
    }
    );
    
    for(size_t a = 0; a < animations.size(); a++) {
        animation* a_ptr = animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); f++) {
            frame* f_ptr = &(a_ptr->frames[f]);
            
            f_ptr->sprite_idx = find_sprite(f_ptr->sprite_name);
        }
    }
}


/**
 * @brief Constructs a new animation instance object.
 *
 * @param anim_db The animation database. Used when changing animations.
 */
animation_instance::animation_instance(animation_database* anim_db) :
    cur_anim(nullptr),
    anim_db(anim_db) {
    
}


/**
 * @brief Constructs a new animation instance object.
 *
 * @param ai2 The other animation instance.
 */
animation_instance::animation_instance(const animation_instance &ai2) :
    cur_anim(ai2.cur_anim),
    anim_db(ai2.anim_db) {
    
    to_start();
}


/**
 * @brief Copies data from another animation instance.
 *
 * @param ai2 The other animation instance.
 * @return The current object.
 */
animation_instance &animation_instance::operator=(
    const animation_instance &ai2
) {
    if(this != &ai2) {
        cur_anim = ai2.cur_anim;
        anim_db = ai2.anim_db;
    }
    
    to_start();
    
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
void animation_instance::get_sprite_data(
    sprite** out_cur_sprite_ptr, sprite** out_next_sprite_ptr,
    float* out_interpolation_factor
) const {
    if(!valid_frame()) {
        if(out_cur_sprite_ptr) *out_cur_sprite_ptr = nullptr;
        if(out_next_sprite_ptr) *out_next_sprite_ptr = nullptr;
        if(out_interpolation_factor) *out_interpolation_factor = 0.0f;
        return;
    }
    
    frame* cur_frame_ptr = &cur_anim->frames[cur_frame_idx];
    //First, the basics -- the current sprite.
    if(out_cur_sprite_ptr) {
        *out_cur_sprite_ptr = cur_frame_ptr->sprite_ptr;
    }
    
    //Now only bother with interpolation data if we actually need it.
    if(!out_next_sprite_ptr && !out_interpolation_factor) return;
    
    if(!cur_frame_ptr->interpolate) {
        //This frame doesn't even interpolate.
        if(out_next_sprite_ptr) *out_next_sprite_ptr = cur_frame_ptr->sprite_ptr;
        if(out_interpolation_factor) *out_interpolation_factor = 0.0f;
        return;
    }
    
    //Get the next sprite.
    size_t next_frame_idx = get_next_frame_idx();
    frame* next_frame_ptr = &cur_anim->frames[next_frame_idx];
    
    if(out_next_sprite_ptr) *out_next_sprite_ptr = next_frame_ptr->sprite_ptr;
    
    //Get the interpolation factor.
    if(out_interpolation_factor) {
        if(cur_frame_ptr->duration == 0.0f) {
            *out_interpolation_factor = 0.0f;
        } else {
            *out_interpolation_factor =
                cur_frame_time /
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
size_t animation_instance::get_next_frame_idx(bool* out_reached_end) const {
    if(out_reached_end) *out_reached_end = false;
    if(!cur_anim) return INVALID;
    
    if(cur_frame_idx < cur_anim->frames.size() - 1) {
        return cur_frame_idx + 1;
    } else {
        if(out_reached_end) *out_reached_end = true;
        if(cur_anim->loop_frame < cur_anim->frames.size()) {
            return cur_anim->loop_frame;
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
void animation_instance::init_to_first_anim(animation_database* db) {
    anim_db = db;
    if(db && !anim_db->animations.empty()) {
        cur_anim = anim_db->animations[0];
    }
    to_start();
}


/**
 * @brief Skips the current animation instance ahead in time for a
 * random amount of time.
 *
 * The time is anywhere between 0 and the total duration of the
 * animation. Frame signals will be ignored.
 */
void animation_instance::skip_ahead_randomly() {
    if(!cur_anim) return;
    //First, find how long the animation lasts for.
    
    float total_duration = 0;
    for(size_t f = 0; f < cur_anim->frames.size(); f++) {
        total_duration += cur_anim->frames[f].duration;
    }
    
    tick(game.rng.f(0, total_duration));
}


/**
 * @brief Clears everything.
 */
void animation_instance::clear() {
    cur_anim = nullptr;
    anim_db = nullptr;
    cur_frame_time = 0;
    cur_frame_idx = INVALID;
}


/**
 * @brief Ticks the animation time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 * @param signals Any frame that sends a signal adds it here.
 * @param sounds Any frame that should play a sound adds it here.
 * @return Whether or not the animation ended its final frame.
 */
bool animation_instance::tick(
    float delta_t, vector<size_t>* signals,
    vector<size_t>* sounds
) {
    if(!cur_anim) return false;
    size_t n_frames = cur_anim->frames.size();
    if(n_frames == 0) return false;
    frame* cur_frame = &cur_anim->frames[cur_frame_idx];
    if(cur_frame->duration == 0) {
        return true;
    }
    
    cur_frame_time += delta_t;
    
    bool reached_end = false;
    
    //This is a while instead of an if because if the framerate is too low
    //and the next frame's duration is too short, it could be that a tick
    //goes over an entire frame, and lands 2 or more frames ahead.
    while(cur_frame_time > cur_frame->duration && cur_frame->duration != 0) {
        cur_frame_time = cur_frame_time - cur_frame->duration;
        bool reached_end_now = false;
        cur_frame_idx = get_next_frame_idx(&reached_end_now);
        reached_end |= reached_end_now;
        cur_frame = &cur_anim->frames[cur_frame_idx];
        if(cur_frame->signal != INVALID && signals) {
            signals->push_back(cur_frame->signal);
        }
        if(cur_frame->sound_idx != INVALID && sounds) {
            sounds->push_back(cur_frame->sound_idx);
        }
    }
    
    return reached_end;
}


/**
 * @brief Sets the animation state to the beginning.
 * It's called automatically when the animation is first set.
 */
void animation_instance::to_start() {
    cur_frame_time = 0;
    cur_frame_idx = 0;
}


/**
 * @brief Returns whether or not the animation instance is in a state where
 * it can show a valid frame.
 *
 * @return Whether it's in a valid state.
 */
bool animation_instance::valid_frame() const {
    if(!cur_anim) return false;
    if(cur_frame_idx >= cur_anim->frames.size()) return false;
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
frame::frame(
    const string &sn, size_t si, sprite* sp, float d,
    bool in, const string &snd, size_t s
) :
    sprite_name(sn),
    sprite_idx(si),
    sprite_ptr(sp),
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
sprite::sprite(
    const string &name, ALLEGRO_BITMAP* const b, const vector<hitbox> &h
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
sprite::sprite(
    const string &name, ALLEGRO_BITMAP* const b, const point &b_pos,
    const point &b_size, const vector<hitbox> &h
) :
    name(name),
    parent_bmp(b),
    file_pos(b_pos),
    file_size(b_size),
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
sprite::sprite(const sprite &s2) :
    name(s2.name),
    parent_bmp(nullptr),
    file(s2.file),
    file_pos(s2.file_pos),
    file_size(s2.file_size),
    offset(s2.offset),
    scale(s2.scale),
    angle(s2.angle),
    tint(s2.tint),
    top_pos(s2.top_pos),
    top_size(s2.top_size),
    top_angle(s2.top_angle),
    top_visible(s2.top_visible),
    bitmap(nullptr),
    hitboxes(s2.hitboxes) {
    
    set_bitmap(file, file_pos, file_size);
}


/**
 * @brief Destroys the sprite object.
 */
sprite::~sprite() {
    set_bitmap("", point(), point());
}


/**
 * @brief Creates the hitboxes, based on the body parts.
 *
 * @param adb The animation database the sprites and body parts belong to.
 * @param height The hitboxes's starting height.
 * @param radius The hitboxes's starting radius.
 */
void sprite::create_hitboxes(
    animation_database* const adb, float height, float radius
) {
    hitboxes.clear();
    for(size_t b = 0; b < adb->body_parts.size(); b++) {
        hitboxes.push_back(
            hitbox(
                adb->body_parts[b]->name,
                b,
                adb->body_parts[b],
                point(), 0, height, radius
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
sprite &sprite::operator=(const sprite &s2) {
    if(this != &s2) {
        name = s2.name;
        parent_bmp = nullptr;
        file_pos = s2.file_pos;
        file_size = s2.file_size;
        offset = s2.offset;
        scale = s2.scale;
        angle = s2.angle;
        tint = s2.tint;
        top_pos = s2.top_pos;
        top_size = s2.top_size;
        top_angle = s2.top_angle;
        top_visible = s2.top_visible;
        bitmap = nullptr;
        hitboxes = s2.hitboxes;
        set_bitmap(s2.file, file_pos, file_size);
    }
    
    return *this;
}


/**
 * @brief Sets the bitmap and parent bitmap, according to the given information.
 * This automatically manages bitmap un/loading and such.
 * If the file name string is empty, sets to a nullptr bitmap
 * (and still unloads the old bitmap).
 *
 * @param new_file_name File name of the bitmap.
 * @param new_file_pos Top-left coordinates of the sub-bitmap inside the bitmap.
 * @param new_file_size Dimensions of the sub-bitmap.
 * @param node If not nullptr, this will be used to report an error with,
 * in case something happens.
 */
void sprite::set_bitmap(
    const string &new_file_name,
    const point &new_file_pos, const point &new_file_size,
    data_node* node
) {
    if(bitmap) {
        al_destroy_bitmap(bitmap);
        bitmap = nullptr;
    }
    if(new_file_name != file && parent_bmp) {
        game.content.bitmaps.list.free(file);
        parent_bmp = nullptr;
    }
    
    if(new_file_name.empty()) {
        file.clear();
        file_size = point();
        file_pos = point();
        return;
    }
    
    if(new_file_name != file || !parent_bmp) {
        parent_bmp = game.content.bitmaps.list.get(new_file_name, node, node != nullptr);
    }
    
    point parent_size = get_bitmap_dimensions(parent_bmp);
    
    file = new_file_name;
    file_pos = new_file_pos;
    file_size = new_file_size;
    file_pos.x = clamp(new_file_pos.x, 0, parent_size.x - 1);
    file_pos.y = clamp(new_file_pos.y, 0, parent_size.y - 1);
    file_size.x = clamp(new_file_size.x, 0, parent_size.x - file_pos.x);
    file_size.y = clamp(new_file_size.y, 0, parent_size.y - file_pos.y);
    
    if(parent_bmp) {
        bitmap =
            al_create_sub_bitmap(
                parent_bmp, file_pos.x, file_pos.y,
                file_size.x, file_size.y
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
void get_sprite_basic_effects(
    const point &base_pos, float base_angle,
    float base_angle_cos_cache, float base_angle_sin_cache,
    sprite* cur_s_ptr, sprite* next_s_ptr, float interpolation_factor,
    point* out_eff_trans, float* out_eff_angle,
    point* out_eff_scale, ALLEGRO_COLOR* out_eff_tint
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
        point next_trans(
            base_pos.x +
            base_angle_cos_cache * next_s_ptr->offset.x -
            base_angle_sin_cache * next_s_ptr->offset.y,
            base_pos.y +
            base_angle_sin_cache * next_s_ptr->offset.x +
            base_angle_cos_cache * next_s_ptr->offset.y
        );
        float next_angle = base_angle + next_s_ptr->angle;
        point next_scale = next_s_ptr->scale;
        ALLEGRO_COLOR next_tint = next_s_ptr->tint;
        
        if(out_eff_trans) {
            *out_eff_trans =
                interpolate_point(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_trans, next_trans
                );
        }
        if(out_eff_angle) {
            *out_eff_angle =
                interpolate_angle(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_angle, next_angle
                );
        }
        if(out_eff_scale) {
            *out_eff_scale =
                interpolate_point(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_scale, next_scale
                );
        }
        if(out_eff_tint) {
            *out_eff_tint =
                interpolate_color(
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
void get_sprite_basic_top_effects(
    sprite* cur_s_ptr, sprite* next_s_ptr, float interpolation_factor,
    point* out_eff_trans, float* out_eff_angle,
    point* out_eff_size
) {
    if(out_eff_trans) {
        *out_eff_trans = cur_s_ptr->top_pos;
    }
    if(out_eff_angle) {
        *out_eff_angle = cur_s_ptr->top_angle;
    }
    if(out_eff_size) {
        *out_eff_size = cur_s_ptr->top_size;
    }
    
    if(next_s_ptr && interpolation_factor > 0.0f) {
        point next_trans = next_s_ptr->top_pos;
        float next_angle = next_s_ptr->top_angle;
        point next_size = next_s_ptr->top_size;
        
        if(out_eff_trans) {
            *out_eff_trans =
                interpolate_point(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_trans, next_trans
                );
        }
        if(out_eff_angle) {
            *out_eff_angle =
                interpolate_angle(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_angle, next_angle
                );
        }
        if(out_eff_size) {
            *out_eff_size =
                interpolate_point(
                    interpolation_factor, 0.0f, 1.0f,
                    *out_eff_size, next_size
                );
        }
    }
}
