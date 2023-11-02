/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Audio-related things.
 */

#include <algorithm>

#include "audio.h"

#include "functions.h"
#include "load.h"


namespace AUDIO {
//Default min stack pos. Let's use a value higher than 0, since if for any
//reason the same sound plays multiple times at once, they are actually
//stopped under the SFX_STACK_NORMAL mode, thus perventing a super-loud sound.
const float DEF_STACK_MIN_POS = 0.02f;
//Maximum change speed for a playback's gain, measured in amount per second.
const float GAIN_CHANGE_SPEED = 3.0f;
//Maximum change speed for a playback's pan, measured in amount per second.
const float PAN_CHANGE_SPEED = 8.0f;
}


/* ----------------------------------------------------------------------------
 * Constructs the audio manager.
 */
audio_manager::audio_manager() :
    samples(""),
    mixer(nullptr),
    voice(nullptr),
    next_sfx_source_id(1) {
}


/* ----------------------------------------------------------------------------
 * Creates a global sound effect source and returns its ID.
 * This is basically how you can get the engine to produce a sound that doesn't
 * involve a position in the game world.
 * Returns 0 on failure.
 * sample:
 *   Sound sample that this source will emit.
 * config:
 *   Configuration.
 */
size_t audio_manager::create_global_sfx_source(
    ALLEGRO_SAMPLE* sample,
    const sfx_source_config_struct &config
) {
    return create_sfx_source(sample, SFX_TYPE_GLOBAL, config, point());
}


/* ----------------------------------------------------------------------------
 * Creates a mob sound effect source and returns its ID.
 * This is like create_pos_sfx_source, but ties the source to the mob, meaning
 * the audio manager is responsible for updating the source's position
 * every frame to match the mob's.
 * Returns 0 on failure.
 * sample:
 *   Sound sample that this source will emit.
 * m_ptr:
 *   Pointer to the mob.
 * config:
 *   Configuration.
 */
size_t audio_manager::create_mob_sfx_source(
    ALLEGRO_SAMPLE* sample,
    mob* m_ptr,
    const sfx_source_config_struct &config
) {
    size_t source_id =
        create_sfx_source(sample, SFX_TYPE_POSITIONAL, config, m_ptr->pos);
    mob_sources[source_id] = m_ptr;
    return source_id;
}


/* ----------------------------------------------------------------------------
 * Creates a positional sound effect source and returns its ID.
 * This is basically how you can get the engine to produce a sound that
 * involves a position in the game world.
 * Returns 0 on failure.
 * sample:
 *   Sound sample that this source will emit.
 * pos:
 *   Starting position in the game world.
 * config:
 *   Configuration.
 */
size_t audio_manager::create_pos_sfx_source(
    ALLEGRO_SAMPLE* sample,
    const point &pos,
    const sfx_source_config_struct &config
) {
    return create_sfx_source(sample, SFX_TYPE_POSITIONAL, config, pos);
}


/* ----------------------------------------------------------------------------
 * Creates a sound effect source and returns its ID.
 * Returns 0 on failure.
 * sample:
 *   Sound sample that this source will emit.
 * type:
 *   Sound type.
 * config:
 *   Configuration.
 * pos:
 *   Position in the game world, if applicable.
 */
size_t audio_manager::create_sfx_source(
    ALLEGRO_SAMPLE* sample,
    SFX_TYPE type,
    const sfx_source_config_struct &config,
    const point &pos
) {
    if(!sample) return 0;
    
    size_t id = next_sfx_source_id;
    
    sources[id] = sfx_source_struct();
    sources[id].sample = sample;
    sources[id].type = type;
    sources[id].config = config;
    sources[id].pos = pos;
    emit(id);
    
    next_sfx_source_id++; //Hopefully there will be no collisions.
    
    return id;
}


/* ----------------------------------------------------------------------------
 * Destroys the audio manager.
 */
void audio_manager::destroy() {
    al_detach_voice(voice);
    al_destroy_mixer(mixer);
    al_destroy_voice(voice);
}


/* ----------------------------------------------------------------------------
 * Destroys a playback object.
 * Returns whether it succeeded.
 * playback_idx:
 *   Index of the playback in the list of playbacks.
 */
bool audio_manager::destroy_sfx_playback(size_t playback_idx) {
    sfx_playback_struct* playback_ptr = &playbacks[playback_idx];
    if(playback_ptr->state == SFX_PLAYBACK_DESTROYED) return false;
    playback_ptr->state = SFX_PLAYBACK_DESTROYED;
    
    sfx_source_struct* source_ptr = get_source(playback_ptr->source_id);
    
    //Destroy the source, if applicable.
    if(source_ptr) {
        if(!has_flag(source_ptr->config.flags, SFX_FLAG_KEEP_ON_PLAYBACK_END)) {
            destroy_sfx_source(playback_ptr->source_id);
        }
    }
    
    //Destroy the Allegro sample instance.
    ALLEGRO_SAMPLE_INSTANCE* instance =
        playback_ptr->allegro_sample_instance;
    if(instance) {
        al_set_sample_instance_playing(instance, false);
        al_detach_sample_instance(instance);
        if(instance) al_destroy_sample_instance(instance);
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Destroys a sound source.
 * Returns whether it succeeded.
 * source_id:
 *   ID of the sound source to destroy.
 */
bool audio_manager::destroy_sfx_source(size_t source_id) {
    sfx_source_struct* source_ptr = get_source(source_id);
    if(!source_ptr) return false;
    
    if(source_ptr->destroyed) return false;
    source_ptr->destroyed = true;
    
    //Check if we must stop playbacks.
    if(
        !has_flag(
            source_ptr->config.flags,
            SFX_FLAG_KEEP_PLAYBACK_ON_DESTROY
        )
    ) {
        for(size_t p = 0; p < playbacks.size(); ++p) {
            if(playbacks[p].source_id == source_id) {
                destroy_sfx_playback(p);
            }
        }
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Emits a sound from a sound source now, if possible.
 * Returns whether it succeeded.
 * source_id:
 *   ID of the source to emit sound from.
 */
bool audio_manager::emit(size_t source_id) {
    //Setup.
    sfx_source_struct* source_ptr = get_source(source_id);
    if(!source_ptr) return false;
    
    ALLEGRO_SAMPLE* sample = source_ptr->sample;
    if(!sample) return false;
    
    //Check if other playbacks exist to prevent stacking.
    float lowest_stacking_playback_pos = FLT_MAX;
    if(
        source_ptr->config.stack_min_pos > 0.0f ||
        source_ptr->config.stack_mode == SFX_STACK_NEVER
    ) {
        for(size_t p = 0; p < playbacks.size(); ++p) {
            sfx_playback_struct* playback = &playbacks[p];
            sfx_source_struct* p_source_ptr = get_source(playback->source_id);
            if(!p_source_ptr || p_source_ptr->sample != sample) continue;
            
            float playback_pos =
                al_get_sample_instance_position(
                    playback->allegro_sample_instance
                ) / 44100.0f;
            lowest_stacking_playback_pos =
                std::min(lowest_stacking_playback_pos, playback_pos);
        }
        
        if(
            source_ptr->config.stack_min_pos > 0.0f &&
            lowest_stacking_playback_pos < source_ptr->config.stack_min_pos
        ) {
            //Can't emit. This would stack the sounds, and there are other
            //playbacks that haven't reached the minimum stack threshold yet.
            return false;
        }
        if(
            source_ptr->config.stack_mode == SFX_STACK_NEVER &&
            lowest_stacking_playback_pos < FLT_MAX
        ) {
            //Can't emit. This would stack the sounds.
            return false;
        }
    }
    
    //Check if other playbacks exist and if we need to stop them.
    if(source_ptr->config.stack_mode == SFX_STACK_OVERRIDE) {
        for(size_t p = 0; p < playbacks.size(); ++p) {
            sfx_playback_struct* playback = &playbacks[p];
            sfx_source_struct* p_source_ptr = get_source(playback->source_id);
            if(!p_source_ptr || p_source_ptr->sample != sample) {
                continue;
            }
            destroy_sfx_playback(p);
        }
    }
    
    //Create the playback.
    playbacks.push_back(sfx_playback_struct());
    sfx_playback_struct* playback_ptr = &playbacks.back();
    playback_ptr->source_id = source_id;
    playback_ptr->allegro_sample_instance = al_create_sample_instance(sample);
    if(!playback_ptr->allegro_sample_instance) return false;
    
    //Play.
    update_playback_target_gain_and_pan(playbacks.size() - 1);
    playback_ptr->gain = playback_ptr->target_gain;
    playback_ptr->pan = playback_ptr->target_pan;
    
    al_attach_sample_instance_to_mixer(
        playback_ptr->allegro_sample_instance, mixer
    );
    
    al_set_sample_instance_playmode(
        playback_ptr->allegro_sample_instance, ALLEGRO_PLAYMODE_ONCE
    );
    al_set_sample_instance_speed(
        playback_ptr->allegro_sample_instance,
        std::max(0.0f, source_ptr->config.speed)
    );
    update_playback_gain_and_pan(playbacks.size() - 1);
    
    al_set_sample_instance_position(
        playback_ptr->allegro_sample_instance,
        0.0f
    );
    al_set_sample_instance_playing(
        playback_ptr->allegro_sample_instance,
        true
    );
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns a source's pointer from a source in the list, or NULL if invalid.
 * source_id:
 *   ID of the sound source.
 */
sfx_source_struct* audio_manager::get_source(size_t source_id) {
    auto source_it = sources.find(source_id);
    if(source_it == sources.end()) return NULL;
    return &source_it->second;
}


/* ----------------------------------------------------------------------------
 * Handles a mob being deleted.
 * m_ptr:
 *   Mob that got deleted.
 */
void audio_manager::handle_mob_deletion(mob* m_ptr) {
    for(auto s = mob_sources.begin(); s != mob_sources.end();) {
        if(s->second == m_ptr) {
            s = mob_sources.erase(s);
        } else {
            ++s;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Initializes the audio manager.
 */
void audio_manager::init() {
    voice =
        al_create_voice(
            44100, ALLEGRO_AUDIO_DEPTH_INT16,   ALLEGRO_CHANNEL_CONF_2
        );
    mixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_voice(mixer, voice);
}


/* ----------------------------------------------------------------------------
 * Sets the camera's position.
 * cam_tl:
 *   Current coordinates of the camera's top-left corner.
 * cam_br:
 *   Current coordinates of the camera's bottom-right corner.
 */
void audio_manager::set_camera_pos(const point &cam_tl, const point &cam_br) {
    this->cam_tl = cam_tl;
    this->cam_br = cam_br;
}


/* ----------------------------------------------------------------------------
 * Sets the position of a positional sound effect source.
 * source_id:
 *   ID of the sound effect source.
 * pos:
 *   New position.
 */
bool audio_manager::set_sfx_source_pos(size_t source_id, const point &pos) {
    sfx_source_struct* source_ptr = get_source(source_id);
    if(!source_ptr) return false;
    
    source_ptr->pos = pos;
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Stops all playbacks. Alternatively, stops all playbacks of a given sound
 * sample.
 * filter:
 *   Sound sample to filter by, or NULL to stop all playbacks.
 */
void audio_manager::stop_all_playbacks(ALLEGRO_SAMPLE* filter) {
    for(size_t p = 0; p < playbacks.size(); ++p) {
        bool to_stop = false;
        
        if(!filter) {
            to_stop = true;
        } else {
            sfx_playback_struct* playback_ptr = &playbacks[p];
            sfx_source_struct* source_ptr =
                get_source(playback_ptr->source_id);
            if(source_ptr && source_ptr->sample == filter) {
                to_stop = true;
            }
        }
        
        if(to_stop) {
            destroy_sfx_playback(p);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Ticks the audio manager by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void audio_manager::tick(float delta_t) {
    //Clear deleted mob sources.
    for(auto s = mob_sources.begin(); s != mob_sources.end();) {
        mob* mob_ptr = s->second;
        if(!mob_ptr || mob_ptr->to_delete) {
            s = mob_sources.erase(s);
        } else {
            ++s;
        }
    }
    
    //Update the position of sources tied to mobs.
    for(auto &s : sources) {
        if(s.second.destroyed) continue;
        auto mob_source_it = mob_sources.find(s.first);
        if(mob_source_it == mob_sources.end()) continue;
        mob* mob_ptr = mob_source_it->second;
        if(!mob_ptr || mob_ptr->to_delete) continue;
        s.second.pos = mob_ptr->pos;
    }
    
    //Update playbacks.
    for(size_t p = 0; p < playbacks.size(); ++p) {
        sfx_playback_struct* playback_ptr = &playbacks[p];
        if(playback_ptr->state == SFX_PLAYBACK_DESTROYED) continue;
        
        if(
            !al_get_sample_instance_playing(
                playback_ptr->allegro_sample_instance
            )
        ) {
            //Finished playing.
            destroy_sfx_playback(p);
            
        } else {
            update_playback_target_gain_and_pan(p);
            
            playback_ptr->gain =
                inch_towards(
                    playback_ptr->gain,
                    playback_ptr->target_gain,
                    AUDIO::GAIN_CHANGE_SPEED * delta_t
                );
            playback_ptr->pan =
                inch_towards(
                    playback_ptr->pan,
                    playback_ptr->target_pan,
                    AUDIO::PAN_CHANGE_SPEED * delta_t
                );
                
            update_playback_gain_and_pan(p);
        }
    }
    
    //Delete destroyed playbacks.
    for(size_t p = 0; p < playbacks.size();) {
        if(playbacks[p].state == SFX_PLAYBACK_DESTROYED) {
            playbacks.erase(playbacks.begin() + p);
        } else {
            ++p;
        }
    }
    
    //Delete destroyed sources.
    for(auto s = sources.begin(); s != sources.end();) {
        if(s->second.destroyed) {
            auto mob_source_it = mob_sources.find(s->first);
            if(mob_source_it != mob_sources.end()) {
                mob_sources.erase(mob_source_it);
            }
            s = sources.erase(s);
        } else {
            ++s;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Instantly updates a playback's current gain and pan, using its member
 * variables. This also clamps the variables if needed.
 * playback_idx:
 *   Index of the playback in the list.
 */
void audio_manager::update_playback_gain_and_pan(size_t playback_idx) {
    if(playback_idx >= playbacks.size()) return;
    sfx_playback_struct* playback_ptr = &playbacks[playback_idx];
    
    playback_ptr->gain = clamp(playback_ptr->gain, 0.0f, 1.0f);
    al_set_sample_instance_gain(
        playback_ptr->allegro_sample_instance,
        playback_ptr->gain
    );
    
    playback_ptr->pan = clamp(playback_ptr->pan, -1.0f, 1.0f);
    al_set_sample_instance_pan(
        playback_ptr->allegro_sample_instance,
        playback_ptr->pan
    );
}


/* ----------------------------------------------------------------------------
 * Updates a playback's target gain and target pan, based on distance
 * from the camera. This won't update the gain and pan yet, but each audio
 * manager tick will be responsible for bringing the gain and pan to these
 * values smoothly over time.
 * playback_idx:
 *   Index of the playback in the list.
 */
void audio_manager::update_playback_target_gain_and_pan(size_t playback_idx) {
    if(playback_idx >= playbacks.size()) return;
    sfx_playback_struct* playback_ptr = &playbacks[playback_idx];
    
    sfx_source_struct* source_ptr = get_source(playback_ptr->source_id);
    if(!source_ptr || source_ptr->type != SFX_TYPE_POSITIONAL) return;
    
    //Calculate screen and camera things.
    point screen_size = cam_br - cam_tl;
    if(screen_size.x == 0.0f || screen_size.y == 0.0f) return;
    
    point cam_center = (cam_tl + cam_br) / 2.0f;
    
    //Get how many screens of distance it is from the center, for X and Y.
    point delta = (source_ptr->pos - cam_center) / screen_size;
    
    float max_delta = std::max(fabs(delta.x), fabs(delta.y));
    
    //Set the gain.
    float gain =
        interpolate_number(
            fabs(max_delta),
            0.2f, 0.8f,
            1.0f, 0.0f
        );
    playback_ptr->target_gain = gain;
    
    //Set the pan.
    float pan_abs =
        interpolate_number(
            fabs(delta.x),
            0.2f, 0.6f,
            0.0f, 1.0f
        );
    pan_abs = clamp(pan_abs, 0.0f, 1.0f);
    float pan = delta.x > 0.0f ? pan_abs : -pan_abs;
    playback_ptr->target_pan = pan;
}



/* ----------------------------------------------------------------------------
 * Creates a sound sample manager.
 * base_dir:
 *   Base directory its files belong to.
 */
sfx_sample_manager::sfx_sample_manager(const string &base_dir) :
    base_dir(base_dir),
    total_calls(0) {
    
}


/* ----------------------------------------------------------------------------
 * Deletes all samples loaded and clears the list.
 */
void sfx_sample_manager::clear() {
    for(auto &s : list) {
        al_destroy_sample(s.second.s);
    }
    list.clear();
    total_calls = 0;
}


/* ----------------------------------------------------------------------------
 * Marks a sound sample to have one less call.
 * If it has 0 calls, it's automatically cleared.
 * it:
 *   Iterator from the map for the sample.
 */
void sfx_sample_manager::detach(map<string, sample_info>::iterator it) {
    if(it == list.end()) return;
    
    it->second.calls--;
    total_calls--;
    if(it->second.calls == 0) {
        al_destroy_sample(it->second.s);
    }
    list.erase(it);
}


/* ----------------------------------------------------------------------------
 * Marks a sound sample to have one less call.
 * If it has 0 calls, it's automatically cleared.
 * name:
 *   Sound sample's file name.
 */
void sfx_sample_manager::detach(const string &name) {
    if(name.empty()) return;
    detach(list.find(name));
}


/* ----------------------------------------------------------------------------
 * Marks a sound sample to have one less call.
 * If it has 0 calls, it's automatically cleared.
 * s:
 *   Sample to detach.
 */
void sfx_sample_manager::detach(const ALLEGRO_SAMPLE* s) {
    if(!s) return;
    
    auto it = list.begin();
    for(; it != list.end(); ++it) {
        if(it->second.s == s) break;
    }
    
    detach(it);
}


/* ----------------------------------------------------------------------------
 * Returns the specified sound sample, by name.
 * name:
 *   Name of the sound sample to get.
 * node:
 *   If not NULL, blame this data node if the file doesn't exist.
 * report_errors:
 *   Only issues errors if this is true.
 */
ALLEGRO_SAMPLE* sfx_sample_manager::get(
    const string &name, data_node* node,
    const bool report_errors
) {
    if(name.empty()) return load_sample("", node, report_errors);
    
    if(list.find(name) == list.end()) {
        ALLEGRO_SAMPLE* s =
            load_sample(base_dir + "/" + name, node, report_errors);
        list[name] = sample_info(s);
        total_calls++;
        return s;
    } else {
        list[name].calls++;
        total_calls++;
        return list[name].s;
    }
};


/* ----------------------------------------------------------------------------
 * Returns the size of the list. Used for debugging.
 */
size_t sfx_sample_manager::get_list_size() const {
    return list.size();
}



/* ----------------------------------------------------------------------------
 * Returns the total number of calls. Used for debugging.
 */
long sfx_sample_manager::get_total_calls() const {
    return total_calls;
}


/* ----------------------------------------------------------------------------
 * Creates a structure with information about a sound sample, for the manager.
 * s:
 *   The sample.
 */
sfx_sample_manager::sample_info::sample_info(ALLEGRO_SAMPLE* s) :
    s(s),
    calls(1) {
    
}
