/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
#include "vars.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * Creates a sprite, with a pre-existing bitmap.
 * name:   Internal name, should be unique.
 * b:      Bitmap.
 * gw, gh: In-game width and height of the sprite.
 * h:      List of hitboxes.
 */
sprite::sprite(
    const string &name, ALLEGRO_BITMAP* const b,
    const float gw, const float gh, const vector<hitbox> &h
) :
    name(name),
    bitmap(b),
    game_w(gw),
    game_h(gh),
    hitboxes(h),
    file_x(0),
    file_y(0),
    file_w(0),
    file_h(0),
    offs_x(0),
    offs_y(0),
    top_visible(true),
    top_x(0),
    top_y(0),
    top_angle(0),
    top_w(32),
    top_h(32),
    parent_bmp(nullptr) {
    
    calculate_hitbox_span();
}


/* ----------------------------------------------------------------------------
 * Creates a sprite using a parent bitmap and the coordinates.
 * name:   Internal name, should be unique.
 * b:      Parent bitmap.
 * bx, by: X and Y of the top-left corner of the sprite, in the parent's bitmap.
 * bw, bh: Width and height of the sprite, in the parent's bitmap.
 * gw, gh: In-game width and height of the sprite.
 * h:      List of hitboxes.
 */
sprite::sprite(
    const string &name, ALLEGRO_BITMAP* const b, const int bx, const int by,
    const int bw, const int bh, const float gw, const float gh,
    const vector<hitbox> &h
) :
    name(name),
    parent_bmp(b),
    bitmap(b ? al_create_sub_bitmap(b, bx, by, bw, bh) : nullptr),
    game_w(gw),
    game_h(gh),
    hitboxes(h),
    file_x(bx),
    file_y(by),
    file_w(bw),
    file_h(bh),
    offs_x(0),
    offs_y(0),
    top_visible(true),
    top_x(0),
    top_y(0),
    top_angle(0),
    top_w(32),
    top_h(32) {
    
    calculate_hitbox_span();
}


/* ----------------------------------------------------------------------------
 * Creates a sprite by copying info from another sprite.
 */
sprite::sprite(const sprite &s2) :
    name(s2.name),
    parent_bmp(s2.parent_bmp),
    bitmap(s2.bitmap),
    hitboxes(s2.hitboxes),
    game_w(s2.game_w),
    game_h(s2.game_h),
    file(s2.file),
    file_x(s2.file_x),
    file_y(s2.file_y),
    file_w(s2.file_w),
    file_h(s2.file_h),
    offs_x(s2.offs_x),
    offs_y(s2.offs_y),
    top_visible(s2.top_visible),
    top_x(s2.top_x),
    top_y(s2.top_y),
    top_w(s2.top_w),
    top_h(s2.top_h),
    top_angle(s2.top_angle),
    hitbox_span(s2.hitbox_span) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a sprite and its bitmaps.
 */
sprite::~sprite() {
    if(parent_bmp) bitmaps.detach(file);
    if(bitmap) al_destroy_bitmap(bitmap);
}


/* ----------------------------------------------------------------------------
 * Calculates the span of the hitboxes.
 */
void sprite::calculate_hitbox_span() {
    hitbox_span = 0;
    for(size_t h = 0; h < hitboxes.size(); ++h) {
        hitbox* h_ptr = &hitboxes[h];
        
        float d = dist(0, 0, h_ptr->x, h_ptr->y).to_float();
        d += h_ptr->radius;
        hitbox_span = max(hitbox_span, d);
    }
}


/* ----------------------------------------------------------------------------
 * Creates the hitboxes, based on the body parts.
 */
void sprite::create_hitboxes(animation_database* const adb) {
    hitboxes.clear();
    for(size_t b = 0; b < adb->body_parts.size(); ++b) {
        hitboxes.push_back(
            hitbox(
                adb->body_parts[b]->name,
                b,
                adb->body_parts[b]
            )
        );
    }
    calculate_hitbox_span();
}


/* ----------------------------------------------------------------------------
 * Creates a frame of animation.
 * sn: Name of the sprite.
 * si: Index of the sprite in the animation database.
 * sp: Pointer to the sprite.
 * d:  Duration.
 */
frame::frame(
    const string &sn, const size_t si, sprite* sp, const float d
) :
    sprite_name(sn),
    sprite_index(si),
    sprite_ptr(sp),
    duration(d) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an animation.
 * name:       Name, should be unique.
 * frames:     List of frame instances.
 * loop_frame: Loop frame number.
 */
animation::animation(
    const string &name, vector<frame> frames,
    const size_t loop_frame
) :
    name(name),
    frames(frames),
    loop_frame(loop_frame) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an animation by copying info from another animation.
 */
animation::animation(const animation &a2) :
    name(a2.name),
    frames(a2.frames),
    loop_frame(a2.loop_frame) {
}


/* ----------------------------------------------------------------------------
 * Creates an animation instance.
 * anim_db: The animation database. Used when changing animations.
 */
animation_instance::animation_instance(animation_database* anim_db) :
    anim_db(anim_db),
    cur_anim(nullptr),
    cur_frame_time(0),
    cur_frame_index(0),
    done_once(false) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an animation instance by copying info from another.
 */
animation_instance::animation_instance(const animation_instance &ai2) :
    cur_anim(ai2.cur_anim),
    anim_db(ai2.anim_db) {
    
    start();
}


/* ----------------------------------------------------------------------------
 * Starts or restarts the animation.
 * It's called automatically when the animation is set.
 */
void animation_instance::start() {
    cur_frame_time = 0;
    cur_frame_index = 0;
    done_once = false;
}


/* ----------------------------------------------------------------------------
 * Ticks the animation with the given amount of time.
 * Returns whether or not the animation ended its final frame.
 */
bool animation_instance::tick(const float time) {
    if(!cur_anim) return false;
    size_t n_frames = cur_anim->frames.size();
    if(n_frames == 0) return false;
    frame* cur_frame = &cur_anim->frames[cur_frame_index];
    if(cur_frame->duration == 0) {
        done_once = true;
        return true;
    }
    
    cur_frame_time += time;
    
    //This is a while instead of an if because if the framerate is too low
    //and the next frame's duration is too short, it could be that a tick
    //goes over an entire frame, and lands 2 or more frames ahead.
    while(cur_frame_time > cur_frame->duration && cur_frame->duration != 0) {
        cur_frame_time = cur_frame_time - cur_frame->duration;
        cur_frame_index++;
        if(cur_frame_index >= n_frames) {
            done_once = true;
            cur_frame_index =
                (cur_anim->loop_frame >= n_frames) ? 0 : cur_anim->loop_frame;
        }
        cur_frame = &cur_anim->frames[cur_frame_index];
    }
    
    return done_once;
}


/* ----------------------------------------------------------------------------
 * Returns the sprite of the current frame of animation.
 */
sprite* animation_instance::get_cur_sprite() {
    if(!cur_anim) return NULL;
    if(cur_anim->frames.empty()) return NULL;
    return cur_anim->frames[cur_frame_index].sprite_ptr;
}


/* ----------------------------------------------------------------------------
 * Creates an animation database.
 */
animation_database::animation_database(
    vector<animation*> a, vector<sprite*> s, vector<body_part*> b
) :
    animations(a),
    sprites(s),
    body_parts(b) {
    
}


/* ----------------------------------------------------------------------------
 * Returns the index of the specified animation.
 * Returns INVALID if not found.
 */
size_t animation_database::find_animation(string name) {
    for(size_t a = 0; a < animations.size(); ++a) {
        if(animations[a]->name == name) return a;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Returns the index of the specified sprite.
 * Returns INVALID if not found.
 */
size_t animation_database::find_sprite(string name) {
    for(size_t s = 0; s < sprites.size(); ++s) {
        if(sprites[s]->name == name) return s;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Returns the index of the specified body part.
 * Returns INVALID if not found.
 */
size_t animation_database::find_body_part(string name) {
    for(size_t b = 0; b < body_parts.size(); ++b) {
        if(body_parts[b]->name == name) return b;
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
 * conversions: A vector of size_t and strings.
   * The size_t is the hardcoded ID (probably in some constant or enum).
   * The string is the name of the animation in the animation file.
 */
void animation_database::create_conversions(
    vector<pair<size_t, string> > conversions
) {
    pre_named_conversions.clear();
    
    if(conversions.empty()) return;
    
    //First, find the highest number.
    size_t highest = conversions[0].first;
    for(size_t c = 1; c < conversions.size(); ++c) {
        highest = max(highest, conversions[c].first);
    }
    
    pre_named_conversions.assign(highest + 1, INVALID);
    
    for(size_t c = 0; c < conversions.size(); ++c) {
        size_t a_pos = find_animation(conversions[c].second);
        pre_named_conversions[conversions[c].first] = a_pos;
    }
}


/* ----------------------------------------------------------------------------
 * Destroys an animation database and all of its content.
 */
void animation_database::destroy() {
    for(auto a = animations.begin(); a != animations.end(); ++a) {
        delete *a;
    }
    for(auto s = sprites.begin(); s != sprites.end(); ++s) {
        delete *s;
    }
    for(auto b = body_parts.begin(); b != body_parts.end(); ++b) {
        delete *b;
    }
    animations.clear();
    sprites.clear();
    body_parts.clear();
}


/* ----------------------------------------------------------------------------
 * Loads the animations from a file.
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
            
            vector<string> coords =
                split(hitbox_node->get_child_by_name("coords")->value);
            if(coords.size() >= 3) {
                cur_hitbox.x = s2f(coords[0]);
                cur_hitbox.y = s2f(coords[1]);
                cur_hitbox.z = s2f(coords[2]);
            }
            cur_hitbox.height =
                s2f(hitbox_node->get_child_by_name("height")->value);
            cur_hitbox.radius =
                s2f(hitbox_node->get_child_by_name("radius")->value);
            cur_hitbox.body_part_name =
                hitbox_node->name;
            cur_hitbox.type =
                s2i(hitbox_node->get_child_by_name("type")->value);
            cur_hitbox.multiplier =
                s2f(
                    hitbox_node->get_child_by_name("multiplier")->value
                );
            cur_hitbox.can_pikmin_latch =
                s2b(
                    hitbox_node->get_child_by_name(
                        "can_pikmin_latch"
                    )->value
                );
            cur_hitbox.knockback_outward =
                s2b(hitbox_node->get_child_by_name("outward")->value);
            cur_hitbox.knockback_angle =
                s2f(hitbox_node->get_child_by_name("angle")->value);
            cur_hitbox.knockback =
                s2f(
                    hitbox_node->get_child_by_name(
                        "knockback"
                    )->value
                );
                
            data_node* hazards_node =
                hitbox_node->get_child_by_name("hazards");
            cur_hitbox.hazards_str = hazards_node->value;
            vector<string> hazards_strs =
                semicolon_list_to_vector(cur_hitbox.hazards_str);
            for(size_t h = 0; h < hazards_strs.size(); ++h) {
                string hazard_name = hazards_strs[h];
                if(hazards.find(hazard_name) == hazards.end()) {
                    log_error(
                        "Unknown hazard \"" + hazard_name + "\"!",
                        hazards_node
                    );
                } else {
                    cur_hitbox.hazards.push_back(
                        &(hazards[hazard_name])
                    );
                }
            }
            
            
            hitboxes.push_back(cur_hitbox);
            
        }
        
        ALLEGRO_BITMAP* parent =
            bitmaps.get(
                sprite_node->get_child_by_name("file")->value,
                sprite_node->get_child_by_name("file")
            );
        sprite* new_s =
            new sprite(
            sprite_node->name,
            parent,
            s2i(sprite_node->get_child_by_name("file_x")->value),
            s2i(sprite_node->get_child_by_name("file_y")->value),
            s2i(sprite_node->get_child_by_name("file_w")->value),
            s2i(sprite_node->get_child_by_name("file_h")->value),
            s2f(sprite_node->get_child_by_name("game_w")->value),
            s2f(sprite_node->get_child_by_name("game_h")->value),
            hitboxes
        );
        adb.sprites.push_back(new_s);
        
        new_s->file = sprite_node->get_child_by_name("file")->value;
        new_s->parent_bmp = parent;
        new_s->offs_x = s2f(sprite_node->get_child_by_name("offs_x")->value);
        new_s->offs_y = s2f(sprite_node->get_child_by_name("offs_y")->value);
        new_s->top_visible =
            s2b(
                sprite_node->get_child_by_name("top_visible")->value
            );
        new_s->top_x = s2f(sprite_node->get_child_by_name("top_x")->value);
        new_s->top_y = s2f(sprite_node->get_child_by_name("top_y")->value);
        new_s->top_w = s2f(sprite_node->get_child_by_name("top_w")->value);
        new_s->top_h = s2f(sprite_node->get_child_by_name("top_h")->value);
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
            frames.push_back(
                frame(
                    frame_node->name,
                    s_pos,
                    (s_pos == INVALID) ? NULL : adb.sprites[s_pos],
                    s2f(frame_node->get_child_by_name("duration")->value)
                )
            );
        }
        
        adb.animations.push_back(
            new animation(
                anim_node->name,
                frames,
                s2i(anim_node->get_child_by_name("loop_frame")->value)
            )
        );
    }
    
    return adb;
}
