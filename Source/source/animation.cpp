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

#include "functions.h"
#include "game.h"
#include "utils/string_utils.h"


using std::size_t;
using std::string;
using std::vector;

/* ----------------------------------------------------------------------------
 * Creates an animation.
 * name:
 *   Name, should be unique.
 * frames:
 *   List of frames.
 * loop_frame:
 *   Loop frame number.
 * hit_rate:
 *   If this has an attack, this is the chance of hitting. 0 - 100.
 */
animation::animation(
    const string &name, const vector<frame> &frames,
    const size_t loop_frame, const unsigned char hit_rate
) :
    name(name),
    frames(frames),
    loop_frame(loop_frame),
    hit_rate(hit_rate) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an animation by copying info from another animation.
 * a2:
 *   The other animation.
 */
animation::animation(const animation &a2) :
    name(a2.name),
    frames(a2.frames),
    loop_frame(a2.loop_frame),
    hit_rate(a2.hit_rate) {
}


/* ----------------------------------------------------------------------------
 * Creates an animation by copying info from another animation.
 * a2:
 *   The other animation.
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


/* ----------------------------------------------------------------------------
 * Returns the total duration of the animation.
 */
float animation::get_duration() {
    float duration = 0.0f;
    for(size_t f = 0; f < frames.size(); ++f) {
        duration += frames[f].duration;
    }
    return duration;
}


/* ----------------------------------------------------------------------------
 * Returns the frame number, and time within that frame, that matches the
 * specified time.
 * t:
 *   Time to check.
 * frame_nr:
 *   The frame number is returned here.
 * frame_time:
 *   The time within the frame is returned here.
 */
void animation::get_frame_and_time(
    const float t, size_t* frame_nr, float* frame_time
) {
    *frame_nr = 0;
    *frame_time = 0.0f;
    
    if(frames.empty() || t <= 0.0f) {
        return;
    }
    
    float duration_so_far = 0.0f;
    float prev_duration_so_far = 0.0f;
    size_t f = 0;
    for(f = 0; f < frames.size(); ++f) {
        prev_duration_so_far = duration_so_far;
        duration_so_far += frames[f].duration;
        
        if(duration_so_far > t) {
            break;
        }
    }
    
    *frame_nr = clamp(f, 0, frames.size() - 1);
    *frame_time = t - prev_duration_so_far;
}


/* ----------------------------------------------------------------------------
 * Returns the total time since the animation start, when given a frame
 * and the current time in the current frame.
 * frame_nr:
 *   Current frame number.
 * frame_time:
 *   Time in the current frame.
 */
float animation::get_time(const size_t frame_nr, const float frame_time) {
    if(frame_nr == INVALID) {
        return 0.0f;
    }
    if(frame_nr >= frames.size()) {
        return get_duration();
    }
    
    float cur_time = 0.0f;
    for(size_t f = 0; f < frame_nr; ++f) {
        cur_time += frames[f].duration;
    }
    cur_time += frame_time;
    return cur_time;
}


/* ----------------------------------------------------------------------------
 * Creates an animation database.
 * a:
 *   List of animations.
 * s:
 *   List of sprites.
 * b:
 *   List of body parts.
 */
animation_database::animation_database(
    const vector<animation*> &a, const vector<sprite*> &s,
    const vector<body_part*> &b
) :
    animations(a),
    sprites(s),
    body_parts(b),
    max_span(0.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Calculates the maximum distance that any of its hitbox can reach.,
 * and stores it in the max_span variable.
 */
void animation_database::calculate_max_span() {
    max_span = 0.0f;
    for(size_t s = 0; s < sprites.size(); ++s) {
        sprite* s_ptr = sprites[s];
        for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
            hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            float d = dist(point(0, 0), h_ptr->pos).to_float();
            d += h_ptr->radius;
            max_span = std::max(max_span, d);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Enemies and such have a regular list of animations.
 * The only way to change these animations is through the script.
 * So animation control is done entirely through game data.
 * However, the animations for Pikmin, leaders, etc. are pre-named.
 * e.g.: The game wants there to be an "idle" animation,
 * a "walk" animation, etc.
 * Because we are NOT looking up with strings, if we want more than 20FPS,
 * we need a way to convert from a numeric ID
 * (one that stands for walking, one for idling, etc.)
 * into the corresponding number on the animation file.
 * This is where this comes in.
 *
 * conversions:
 *   A vector of size_t and strings.
 *   The size_t is the hardcoded ID (probably in some constant or enum).
 *   The string is the name of the animation in the animation file.
 * file:
 *   File from where these animations were loaded. Used to
 *   report errors.
 */
void animation_database::create_conversions(
    vector<std::pair<size_t, string> > conversions, data_node* file
) {
    pre_named_conversions.clear();
    
    if(conversions.empty()) return;
    
    //First, find the highest number.
    size_t highest = conversions[0].first;
    for(size_t c = 1; c < conversions.size(); ++c) {
        highest = std::max(highest, conversions[c].first);
    }
    
    pre_named_conversions.assign(highest + 1, INVALID);
    
    for(size_t c = 0; c < conversions.size(); ++c) {
        size_t a_pos = find_animation(conversions[c].second);
        pre_named_conversions[conversions[c].first] = a_pos;
        if(a_pos == INVALID) {
            log_error(
                "Animation \"" + conversions[c].second + "\" is required "
                "by the engine, but does not exist!", file
            );
        }
    }
}


/* ----------------------------------------------------------------------------
 * Destroys an animation database and all of its content.
 */
void animation_database::destroy() {
    for(size_t a = 0; a < animations.size(); ++a) {
        delete animations[a];
    }
    for(size_t s = 0; s < sprites.size(); ++s) {
        delete sprites[s];
    }
    for(size_t b = 0; b < body_parts.size(); ++b) {
        delete body_parts[b];
    }
    animations.clear();
    sprites.clear();
    body_parts.clear();
}


/* ----------------------------------------------------------------------------
 * Returns the index of the specified animation.
 * Returns INVALID if not found.
 * name:
 *   Name of the animation to search for.
 */
size_t animation_database::find_animation(const string &name) const {
    for(size_t a = 0; a < animations.size(); ++a) {
        if(animations[a]->name == name) return a;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Returns the index of the specified body part.
 * Returns INVALID if not found.
 * name:
 *   Name of the body part to search for.
 */
size_t animation_database::find_body_part(const string &name) const {
    for(size_t b = 0; b < body_parts.size(); ++b) {
        if(body_parts[b]->name == name) return b;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Returns the index of the specified sprite.
 * Returns INVALID if not found.
 * name:
 *   Name of the sprite to search for.
 */
size_t animation_database::find_sprite(const string &name) const {
    for(size_t s = 0; s < sprites.size(); ++s) {
        if(sprites[s]->name == name) return s;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Fixes the pointers for body parts.
 */
void animation_database::fix_body_part_pointers() {
    for(size_t s = 0; s < sprites.size(); ++s) {
        sprite* s_ptr = sprites[s];
        for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
            hitbox* h_ptr = &s_ptr->hitboxes[h];
            
            for(size_t b = 0; b < body_parts.size(); ++b) {
                body_part* b_ptr = body_parts[b];
                if(b_ptr->name == h_ptr->body_part_name) {
                    h_ptr->body_part_index = b;
                    h_ptr->body_part_ptr = b_ptr;
                    break;
                }
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Sorts all animations and sprites alphabetically, making them more organized.
 */
void animation_database::sort_alphabetically() {
    sort(
        animations.begin(), animations.end(),
    [] (animation * a1, animation * a2) {
        return a1->name < a2->name;
    }
    );
    sort(
        sprites.begin(), sprites.end(),
    [] (sprite * s1, sprite * s2) {
        return s1->name < s2->name;
    }
    );
    
    for(size_t a = 0; a < animations.size(); ++a) {
        animation* a_ptr = animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); ++f) {
            frame* f_ptr = &(a_ptr->frames[f]);
            
            f_ptr->sprite_index = find_sprite(f_ptr->sprite_name);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates an animation instance.
 * anim_db:
 *   The animation database. Used when changing animations.
 */
animation_instance::animation_instance(animation_database* anim_db) :
    cur_anim(nullptr),
    anim_db(anim_db),
    cur_frame_time(0),
    cur_frame_index(0) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an animation instance by copying info from another.
 * ai2:
 *   The other animation instance.
 */
animation_instance::animation_instance(const animation_instance &ai2) :
    cur_anim(ai2.cur_anim),
    anim_db(ai2.anim_db) {
    
    start();
}


/* ----------------------------------------------------------------------------
 * Copies data from another animation instance.
 * ai2:
 *   The other animation instance.
 */
animation_instance &animation_instance::operator=(
    const animation_instance &ai2
) {
    if(this != &ai2) {
        cur_anim = ai2.cur_anim;
        anim_db = ai2.anim_db;
    }
    
    start();
    
    return *this;
}


/* ----------------------------------------------------------------------------
 * Returns the sprite of the current frame of animation.
 */
sprite* animation_instance::get_cur_sprite() const {
    if(!cur_anim) return NULL;
    if(cur_frame_index == INVALID) return NULL;
    return cur_anim->frames[cur_frame_index].sprite_ptr;
}


/* ----------------------------------------------------------------------------
 * Skips the current animation instance ahead in time for a random amount of
 * time. The time is anywhere between 0 and the total duration of the
 * animation. Frame signals will be ignored.
 */
void animation_instance::skip_ahead_randomly() {
    if(!cur_anim) return;
    //First, find how long the animation lasts for.
    
    float total_duration = 0;
    for(size_t f = 0; f < cur_anim->frames.size(); ++f) {
        total_duration += cur_anim->frames[f].duration;
    }
    
    tick(randomf(0, total_duration));
}


/* ----------------------------------------------------------------------------
 * Starts or restarts the animation.
 * It's called automatically when the animation is set.
 */
void animation_instance::start() {
    cur_frame_time = 0;
    cur_frame_index = 0;
}


/* ----------------------------------------------------------------------------
 * Ticks the animation time by one frame of logic.
 * Returns whether or not the animation ended its final frame.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 * signals:
 *   Any frame that sends a signal adds it here.
 */
bool animation_instance::tick(const float delta_t, vector<size_t>* signals) {
    if(!cur_anim) return false;
    size_t n_frames = cur_anim->frames.size();
    if(n_frames == 0) return false;
    frame* cur_frame = &cur_anim->frames[cur_frame_index];
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
        cur_frame_index++;
        if(cur_frame_index >= n_frames) {
            reached_end = true;
            cur_frame_index =
                (cur_anim->loop_frame >= n_frames) ? 0 : cur_anim->loop_frame;
        }
        cur_frame = &cur_anim->frames[cur_frame_index];
        if(cur_frame->signal != INVALID && signals) {
            signals->push_back(cur_frame->signal);
        }
    }
    
    return reached_end;
}


/* ----------------------------------------------------------------------------
 * Creates a frame of animation.
 * sn:
 *   Name of the sprite.
 * si:
 *   Index of the sprite in the animation database.
 * sp:
 *   Pointer to the sprite.
 * d:
 *   Duration.
 * s:
 *   Signal.
 */
frame::frame(
    const string &sn, const size_t si, sprite* sp, const float d, const size_t s
) :
    sprite_name(sn),
    sprite_index(si),
    sprite_ptr(sp),
    duration(d),
    signal(s) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a sprite, with a pre-existing bitmap.
 * name:
 *   Internal name; should be unique.
 * b:
 *   Bitmap.
 * h:
 *   List of hitboxes.
 */
sprite::sprite(
    const string &name, ALLEGRO_BITMAP* const b, const vector<hitbox> &h
) :
    name(name),
    parent_bmp(nullptr),
    scale(point(1.0, 1.0)),
    angle(0),
    top_size(5.5, 10),
    top_angle(0),
    top_visible(true),
    bitmap(b),
    hitboxes(h) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a sprite using a parent bitmap and the coordinates.
 * name:
 *   Internal name, should be unique.
 * b:
 *   Parent bitmap.
 * b_pos:
 *   X and Y of the top-left corner of the sprite, in the parent's bitmap.
 * b_size:
 *   Width and height of the sprite, in the parent's bitmap.
 * h:
 *   List of hitboxes.
 */
sprite::sprite(
    const string &name, ALLEGRO_BITMAP* const b, const point &b_pos,
    const point &b_size, const vector<hitbox> &h
) :
    name(name),
    parent_bmp(b),
    file_pos(b_pos),
    file_size(b_size),
    scale(point(1.0, 1.0)),
    angle(0),
    top_angle(0),
    top_visible(true),
    bitmap(
        b ?
        al_create_sub_bitmap(b, b_pos.x, b_pos.y, b_size.x, b_size.y) :
        nullptr
    ),
    hitboxes(h) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a sprite by copying info from another sprite.
 * s2:
 *   The other sprite.
 */
sprite::sprite(const sprite &s2) :
    name(s2.name),
    parent_bmp(NULL),
    file(s2.file),
    file_pos(s2.file_pos),
    file_size(s2.file_size),
    offset(s2.offset),
    scale(s2.scale),
    angle(s2.angle),
    top_pos(s2.top_pos),
    top_size(s2.top_size),
    top_angle(s2.top_angle),
    top_visible(s2.top_visible),
    bitmap(NULL),
    hitboxes(s2.hitboxes) {
    
    set_bitmap(file, file_pos, file_size);
}


/* ----------------------------------------------------------------------------
 * Destroys a sprite and its bitmaps.
 */
sprite::~sprite() {
    set_bitmap("", point(), point());
}


/* ----------------------------------------------------------------------------
 * Creates the hitboxes, based on the body parts.
 * adb:
 *   The animation database the sprites and body parts belong to.
 * height:
 *   The hitboxes's starting height.
 * radius:
 *   The hitboxes's starting radius.
 */
void sprite::create_hitboxes(
    animation_database* const adb, const float height, const float radius
) {
    hitboxes.clear();
    for(size_t b = 0; b < adb->body_parts.size(); ++b) {
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


/* ----------------------------------------------------------------------------
 * Copies data from another sprite.
 * s2:
 *   The other sprite.
 */
sprite &sprite::operator=(const sprite &s2) {
    if(this != &s2) {
        name = s2.name;
        parent_bmp = NULL;
        file_pos = s2.file_pos;
        file_size = s2.file_size;
        offset = s2.offset;
        scale = s2.scale;
        angle = s2.angle;
        top_pos = s2.top_pos;
        top_size = s2.top_size;
        top_angle = s2.top_angle;
        top_visible = s2.top_visible;
        bitmap = NULL;
        hitboxes = s2.hitboxes;
        set_bitmap(s2.file, file_pos, file_size);
    }
    
    return *this;
}


/* ----------------------------------------------------------------------------
 * Sets the bitmap and parent bitmap, according to the given information.
 * This automatically manages bitmap un/loading and such.
 * If the file name string is empty, sets to a NULL bitmap
 * (and still unloads the old bitmap).
 * file_name:
 *   File name of the bitmap.
 * file_pos:
 *   Top-left coordinates of the sub-bitmap inside the bitmap.
 * file_size:
 *   Dimensions of the sub-bitmap.
 * node:
 *   If not NULL, this will be used to report an error with, in case
 *   something happens.
 */
void sprite::set_bitmap(
    const string &file_name, const point &file_pos, const point &file_size,
    data_node* node
) {
    if(bitmap) {
        al_destroy_bitmap(bitmap);
        bitmap = NULL;
    }
    if(file_name != file && parent_bmp) {
        game.bitmaps.detach(file);
        parent_bmp = NULL;
    }
    
    if(file_name.empty()) {
        file.clear();
        this->file_size = point();
        this->file_pos = point();
        return;
    }
    
    if(file_name != file || !parent_bmp) {
        parent_bmp = game.bitmaps.get(file_name, node, node != NULL);
    }
    
    int parent_w = al_get_bitmap_width(parent_bmp);
    int parent_h = al_get_bitmap_height(parent_bmp);
    
    file = file_name;
    this->file_pos = file_pos;
    this->file_size = file_size;
    this->file_pos.x = clamp(file_pos.x, 0, parent_w - 1);
    this->file_pos.y = clamp(file_pos.y, 0, parent_h - 1);
    this->file_size.x = clamp(file_size.x, 0, parent_w - this->file_pos.x);
    this->file_size.y = clamp(file_size.y, 0, parent_h - this->file_pos.y);
    
    if(parent_bmp) {
        bitmap =
            al_create_sub_bitmap(
                parent_bmp, this->file_pos.x, this->file_pos.y,
                this->file_size.x, this->file_size.y
            );
    }
}



/* ----------------------------------------------------------------------------
 * Loads the animations from a file.
 * file_node:
 *   File to load from.
 */
animation_database load_animation_database_from_file(data_node* file_node) {
    animation_database adb;
    
    //Body parts.
    data_node* body_parts_node = file_node->get_child_by_name("body_parts");
    size_t n_body_parts = body_parts_node->get_nr_of_children();
    for(size_t b = 0; b < n_body_parts; ++b) {
    
        data_node* body_part_node = body_parts_node->get_child(b);
        
        body_part* cur_body_part = new body_part(body_part_node->name);
        adb.body_parts.push_back(cur_body_part);
    }
    
    //Sprites.
    data_node* sprites_node = file_node->get_child_by_name("sprites");
    size_t n_sprites = sprites_node->get_nr_of_children();
    for(size_t s = 0; s < n_sprites; ++s) {
    
        data_node* sprite_node = sprites_node->get_child(s);
        vector<hitbox> hitboxes;
        
        data_node* hitboxes_node =
            sprite_node->get_child_by_name("hitboxes");
        size_t n_hitboxes = hitboxes_node->get_nr_of_children();
        
        for(size_t h = 0; h < n_hitboxes; ++h) {
        
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
                (HITBOX_TYPES)
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
            for(size_t hs = 0; hs < hazards_strs.size(); ++hs) {
                string hazard_name = hazards_strs[hs];
                if(game.hazards.find(hazard_name) == game.hazards.end()) {
                    log_error(
                        "Unknown hazard \"" + hazard_name + "\"!",
                        hazards_node
                    );
                } else {
                    cur_hitbox.hazards.push_back(
                        &(game.hazards[hazard_name])
                    );
                }
            }
            
            
            hitboxes.push_back(cur_hitbox);
            
        }
        
        sprite* new_s =
            new sprite(
            sprite_node->name,
            NULL,
            s2p(sprite_node->get_child_by_name("file_pos")->value),
            s2p(sprite_node->get_child_by_name("file_size")->value),
            hitboxes
        );
        adb.sprites.push_back(new_s);
        
        new_s->offset = s2p(sprite_node->get_child_by_name("offset")->value);
        new_s->scale =
            s2p(
                sprite_node->get_child_by_name(
                    "scale"
                )->get_value_or_default("1 1")
            );
        new_s->angle = s2f(sprite_node->get_child_by_name("angle")->value);
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
    data_node* anims_node = file_node->get_child_by_name("animations");
    size_t n_anims = anims_node->get_nr_of_children();
    for(size_t a = 0; a < n_anims; ++a) {
    
        data_node* anim_node = anims_node->get_child(a);
        vector<frame> frames;
        
        data_node* frames_node =
            anim_node->get_child_by_name("frames");
        size_t n_frames =
            frames_node->get_nr_of_children();
            
        for(size_t f = 0; f < n_frames; ++f) {
            data_node* frame_node = frames_node->get_child(f);
            size_t s_pos = adb.find_sprite(frame_node->name);
            string signal_str =
                frame_node->get_child_by_name("signal")->value;
            frames.push_back(
                frame(
                    frame_node->name,
                    s_pos,
                    (s_pos == INVALID) ? NULL : adb.sprites[s_pos],
                    s2f(frame_node->get_child_by_name("duration")->value),
                    (signal_str.empty() ? INVALID : s2i(signal_str))
                )
            );
        }
        
        adb.animations.push_back(
            new animation(
                anim_node->name,
                frames,
                s2i(anim_node->get_child_by_name("loop_frame")->value),
                s2i(anim_node->get_child_by_name(
                        "hit_rate"
                    )->get_value_or_default("100"))
            )
        );
    }
    
    adb.engine_version = file_node->get_child_by_name("engine_version")->value;
    
    adb.calculate_max_span();
    return adb;
}
