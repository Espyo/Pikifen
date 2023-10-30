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
}


/* ----------------------------------------------------------------------------
 * Constructs the audio manager.
 */
audio_manager::audio_manager() :
    samples(""),
    mixer(nullptr),
    voice(nullptr),
    next_sfx_source_id(0) {
}


/* ----------------------------------------------------------------------------
 * Creates a sound source and returns its ID. This is basically how you can
 * get the engine to produce a sound.
 * Returns 0 on failure.
 * sample:
 *   Sound sample that this source will emit.
 * type:
 *   Type of sound.
 * config:
 *   Configuration.
 */
size_t audio_manager::create_sfx_source(
    ALLEGRO_SAMPLE* sample,
    SFX_TYPE type,
    const sfx_source_config_struct &config
) {
    if(!sample) return 0;
    
    next_sfx_source_id++; //Hopefully there will be no collisions.
    sources[next_sfx_source_id] = sfx_source_struct();
    sources[next_sfx_source_id].sample = sample;
    sources[next_sfx_source_id].config = config;
    emit(next_sfx_source_id);
    
    return next_sfx_source_id;
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
bool audio_manager::destroy_playback(size_t playback_idx) {
    ALLEGRO_SAMPLE_INSTANCE* instance =
        playbacks[playback_idx].allegro_sample_instance;
    if(instance) {
        al_set_sample_instance_playing(instance, false);
        al_detach_sample_instance(instance);
        if(instance) al_destroy_sample_instance(instance);
    }
    
    playbacks.erase(playbacks.begin() + playback_idx);
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Destroys a sound source.
 * Returns whether it succeeded.
 * source_id:
 *   ID of the sound source to destroy.
 */
bool audio_manager::destroy_sfx_source(size_t source_id) {
    auto source_it = sources.find(source_id);
    if(source_it == sources.end()) return false;
    
    //Check if we must stop playbacks.
    if(
        has_flag(
            source_it->second.config.flags,
            SFX_FLAG_STOP_PLAYBACK_ON_DESTROY
        )
    ) {
        for(size_t p = 0; p < playbacks.size();) {
            if(playbacks[p].source_id == source_id) {
                //TODO prevent infinite recursion if
                //SFX_FLAG_KEEP_ON_PLAYBACK_END is not set.
                destroy_playback(p);
            } else {
                ++p;
            }
        }
    }
    
    //Finally, destroy it.
    sources.erase(source_it);
    
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
        for(size_t p = 0; p < playbacks.size();) {
            sfx_playback_struct* playback = &playbacks[p];
            sfx_source_struct* p_source_ptr = get_source(playback->source_id);
            if(!p_source_ptr || p_source_ptr->sample != sample) {
                ++p;
                continue;
            }
            destroy_playback(p);
        }
    }
    
    //Create the playback.
    sfx_playback_struct playback;
    playback.source_id = source_id;
    playback.allegro_sample_instance = al_create_sample_instance(sample);
    if(!playback.allegro_sample_instance) return false;
    
    playbacks.push_back(playback);
    
    //Play.
    al_attach_sample_instance_to_mixer(playback.allegro_sample_instance, mixer);
    
    al_set_sample_instance_playmode(
        playback.allegro_sample_instance, ALLEGRO_PLAYMODE_ONCE
    );
    al_set_sample_instance_gain(
        playback.allegro_sample_instance,
        clamp(source_ptr->config.gain, 0.0f, 1.0f)
    );
    al_set_sample_instance_pan(
        playback.allegro_sample_instance,
        clamp(source_ptr->config.pan, -1.0f, 1.0f)
    );
    al_set_sample_instance_speed(
        playback.allegro_sample_instance,
        std::max(0.0f, source_ptr->config.speed)
    );
    
    al_set_sample_instance_position(
        playback.allegro_sample_instance,
        0.0f
    );
    al_set_sample_instance_playing(
        playback.allegro_sample_instance,
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
 * Stops all playbacks. Alternatively, stops all playbacks of a given sound
 * sample.
 * filter:
 *   Sound sample to filter by, or NULL to stop all playbacks.
 */
void audio_manager::stop_all_playbacks(ALLEGRO_SAMPLE* filter) {
    for(size_t p = 0; p < playbacks.size();) {
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
            destroy_playback(p);
        } else {
            ++p;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Ticks the audio manager by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 * cam_tl:
 *   Current coordinates of the camera's top-left corner.
 * cam_br:
 *   Current coordinates of the camera's bottom-right corner.
 */
void audio_manager::tick(
    float delta_t, const point  &cam_tl, const point  &cam_br
) {
    this->cam_tl = cam_tl;
    this->cam_br = cam_br;
    
    for(size_t p = 0; p < playbacks.size();) {
        sfx_playback_struct* playback = &playbacks[p];
        
        if(!al_get_sample_instance_playing(playback->allegro_sample_instance)) {
            //Finished playing.
            uint8_t flags = sources[playback->source_id].config.flags;
            if(!has_flag(flags, SFX_FLAG_KEEP_ON_PLAYBACK_END)) {
                destroy_sfx_source(playback->source_id);
            }
            destroy_playback(p);
        } else {
            ++p;
        }
        
    }
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
