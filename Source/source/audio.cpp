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
#include "game.h"
#include "load.h"
#include "utils/general_utils.h"


namespace AUDIO {

//Default min stack pos. Let's use a value higher than 0, since if for any
//reason the same sound plays multiple times at once, they are actually
//stopped under the SFX_STACK_MODE_NORMAL mode, thus perventing a super-loud sound.
const float DEF_STACK_MIN_POS = 0.1f;

//Change speed for a mix track's gain, measured in amount per second.
const float MIX_TRACK_GAIN_SPEED = 1.0f;

//Change speed for a playback's gain, measured in amount per second.
const float PLAYBACK_GAIN_SPEED = 3.0f;

//Change speed for a playback's pan, measured in amount per second.
const float PLAYBACK_PAN_SPEED = 8.0f;

//Change speed of playback gain when un/pausing, measured in amount per second.
const float PLAYBACK_PAUSE_GAIN_SPEED = 5.0f;

//Distance to an audio source where it'll be considered close, i.e. it will
//play at full volume and no pan.
const float PLAYBACK_RANGE_CLOSE = 100.0f;

//Distance after which an audio source's volume will be 0.
const float PLAYBACK_RANGE_FAR_GAIN = 450.0f;

//Horizontal distance after which an audio source's pan will be
//fully left/right.
const float PLAYBACK_RANGE_FAR_PAN = 300.0f;

//Change speed of playback gain when stopping, measured in amount per second.
const float PLAYBACK_STOP_GAIN_SPEED = 8.0f;

//Change speed for a song's gain, measured in amount per second.
const float SONG_GAIN_SPEED = 1.0f;

//Gain for when a song is softened, due to a game pause.
const float SONG_SOFTENED_GAIN = 0.4f;

}


/**
 * @brief Constructs a new audio manager object.
 */
audio_manager::audio_manager() :
    samples(""),
    streams("") {
}


/**
 * @brief Creates a mob sound effect source and returns its ID.
 *
 * This is like create_world_pos_sfx_source, but ties the source to the mob,
 * meaning the audio manager is responsible for updating the source's position
 * every frame to match the mob's.
 *
 * @param sample Sound sample that this source will emit.
 * @param m_ptr Pointer to the mob.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t audio_manager::create_mob_sfx_source(
    ALLEGRO_SAMPLE* sample,
    mob* m_ptr,
    const sfx_source_config_t &config
) {
    size_t source_id =
        create_sfx_source(sample, SFX_TYPE_WORLD_POS, config, m_ptr->pos);
    mob_sources[source_id] = m_ptr;
    return source_id;
}


/**
 * @brief Creates a sound effect source and returns its ID.
 *
 * @param sample Sound sample that this source will emit.
 * @param type Sound type.
 * @param config Configuration.
 * @param pos Position in the game world, if applicable.
 * @return The ID, or 0 on failure.
 */
size_t audio_manager::create_sfx_source(
    ALLEGRO_SAMPLE* sample,
    SFX_TYPE type,
    const sfx_source_config_t &config,
    const point &pos
) {
    if(!sample) return 0;
    
    size_t id = next_sfx_source_id;
    
    sources[id] = sfx_source_t();
    sources[id].sample = sample;
    sources[id].type = type;
    sources[id].config = config;
    sources[id].pos = pos;
    
    if(!has_flag(config.flags, SFX_FLAG_DONT_EMIT_ON_CREATION)) {
        schedule_emission(id, true);
        if(sources[id].emit_time_left <= 0.0f) {
            emit(id);
            schedule_emission(id, false);
        }
    }
    
    next_sfx_source_id++; //Hopefully there will be no collisions.
    
    return id;
}


/**
 * @brief Creates a global UI sound effect source and returns its ID.
 *
 * This is basically how you can get the engine to produce a UI sound.
 *
 * @param sample Sound sample that this source will emit.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t audio_manager::create_ui_sfx_source(
    ALLEGRO_SAMPLE* sample,
    const sfx_source_config_t &config
) {
    return create_sfx_source(sample, SFX_TYPE_UI, config, point());
}


/**
 * @brief Creates an in-world ambiance sound effect source and returns its ID.
 *
 * This is basically how you can get the engine to produce a sound that doesn't
 * involve a position in the game world, and is just decorative ambiance.
 *
 * @param sample Sound sample that this source will emit.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t audio_manager::create_world_ambiance_sfx_source(
    ALLEGRO_SAMPLE* sample,
    const sfx_source_config_t &config
) {
    return create_sfx_source(sample, SFX_TYPE_WORLD_AMBIANCE, config, point());
}


/**
 * @brief Creates an in-world global sound effect source and returns its ID.
 *
 * This is basically how you can get the engine to produce a sound that doesn't
 * involve a position in the game world.
 *
 * @param sample Sound sample that this source will emit.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t audio_manager::create_world_global_sfx_source(
    ALLEGRO_SAMPLE* sample,
    const sfx_source_config_t &config
) {
    return create_sfx_source(sample, SFX_TYPE_WORLD_GLOBAL, config, point());
}


/**
 * @brief Creates an in-world positional sound effect source and returns its ID.
 *
 * This is basically how you can get the engine to produce a sound that
 * involves a position in the game world.
 *
 * @param sample Sound sample that this source will emit.
 * @param pos Starting position in the game world.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t audio_manager::create_world_pos_sfx_source(
    ALLEGRO_SAMPLE* sample,
    const point &pos,
    const sfx_source_config_t &config
) {
    return create_sfx_source(sample, SFX_TYPE_WORLD_POS, config, pos);
}


/**
 * @brief Destroys the audio manager.
 */
void audio_manager::destroy() {
    al_detach_voice(voice);
    al_destroy_mixer(world_sfx_mixer);
    al_destroy_mixer(music_mixer);
    al_destroy_mixer(world_ambiance_sfx_mixer);
    al_destroy_mixer(ui_sfx_mixer);
    al_destroy_mixer(master_mixer);
    al_destroy_voice(voice);
}


/**
 * @brief Destroys a playback object directly.
 * The "stopping" state is not relevant here.
 *
 * @param playback_idx Index of the playback in the list of playbacks.
 * @return Whether it succeeded.
 */
bool audio_manager::destroy_sfx_playback(size_t playback_idx) {
    sfx_playback_t* playback_ptr = &playbacks[playback_idx];
    if(playback_ptr->state == SFX_PLAYBACK_STATE_DESTROYED) return false;
    playback_ptr->state = SFX_PLAYBACK_STATE_DESTROYED;
    
    sfx_source_t* source_ptr = get_source(playback_ptr->source_id);
    
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


/**
 * @brief Destroys a sound source.
 *
 * @param source_id ID of the sound source to destroy.
 * @return Whether it succeeded.
 */
bool audio_manager::destroy_sfx_source(size_t source_id) {
    sfx_source_t* source_ptr = get_source(source_id);
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
                stop_sfx_playback(p);
            }
        }
    }
    
    return true;
}


/**
 * @brief Emits a sound from a sound source now, if possible.
 *
 * @param source_id ID of the source to emit sound from.
 * @return Whether it succeeded.
 */
bool audio_manager::emit(size_t source_id) {
    //Setup.
    sfx_source_t* source_ptr = get_source(source_id);
    if(!source_ptr) return false;
    
    ALLEGRO_SAMPLE* sample = source_ptr->sample;
    if(!sample) return false;
    
    //Check if other playbacks exist to prevent stacking.
    float lowest_stacking_playback_pos = FLT_MAX;
    if(
        source_ptr->config.stack_min_pos > 0.0f ||
        source_ptr->config.stack_mode == SFX_STACK_MODE_NEVER
    ) {
        for(size_t p = 0; p < playbacks.size(); ++p) {
            sfx_playback_t* playback = &playbacks[p];
            sfx_source_t* p_source_ptr = get_source(playback->source_id);
            if(!p_source_ptr || p_source_ptr->sample != sample) continue;
            
            float playback_pos =
                al_get_sample_instance_position(
                    playback->allegro_sample_instance
                ) /
                (float) al_get_sample_frequency(p_source_ptr->sample);
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
            source_ptr->config.stack_mode == SFX_STACK_MODE_NEVER &&
            lowest_stacking_playback_pos < FLT_MAX
        ) {
            //Can't emit. This would stack the sounds.
            return false;
        }
    }
    
    //Check if other playbacks exist and if we need to stop them.
    if(source_ptr->config.stack_mode == SFX_STACK_MODE_OVERRIDE) {
        for(size_t p = 0; p < playbacks.size(); ++p) {
            sfx_playback_t* playback = &playbacks[p];
            sfx_source_t* p_source_ptr = get_source(playback->source_id);
            if(!p_source_ptr || p_source_ptr->sample != sample) {
                continue;
            }
            stop_sfx_playback(p);
        }
    }
    
    //Create the playback.
    playbacks.push_back(sfx_playback_t());
    sfx_playback_t* playback_ptr = &playbacks.back();
    playback_ptr->source_id = source_id;
    playback_ptr->allegro_sample_instance = al_create_sample_instance(sample);
    if(!playback_ptr->allegro_sample_instance) return false;
    
    playback_ptr->base_gain = source_ptr->config.gain;
    if(source_ptr->config.gain_deviation != 0.0f) {
        playback_ptr->base_gain +=
            randomf(
                -source_ptr->config.gain_deviation,
                source_ptr->config.gain_deviation
            );
        playback_ptr->base_gain =
            clamp(playback_ptr->base_gain, 0.0f, 1.0f);
    }
    
    //Play.
    update_playback_target_gain_and_pan(playbacks.size() - 1);
    playback_ptr->gain = playback_ptr->target_gain;
    playback_ptr->pan = playback_ptr->target_pan;
    
    ALLEGRO_MIXER* mixer = nullptr;
    switch(source_ptr->type) {
    case SFX_TYPE_WORLD_GLOBAL:
    case SFX_TYPE_WORLD_POS: {
        mixer = world_sfx_mixer;
        break;
    } case SFX_TYPE_WORLD_AMBIANCE: {
        mixer = world_ambiance_sfx_mixer;
        break;
    } case SFX_TYPE_UI: {
        mixer = ui_sfx_mixer;
        break;
    }
    }
    
    al_attach_sample_instance_to_mixer(
        playback_ptr->allegro_sample_instance, mixer
    );
    
    al_set_sample_instance_playmode(
        playback_ptr->allegro_sample_instance,
        has_flag(source_ptr->config.flags, SFX_FLAG_LOOP) ?
        ALLEGRO_PLAYMODE_LOOP :
        ALLEGRO_PLAYMODE_ONCE
    );
    float speed = source_ptr->config.speed;
    if(source_ptr->config.speed_deviation != 0.0f) {
        speed +=
            randomf(
                -source_ptr->config.speed_deviation,
                source_ptr->config.speed_deviation
            );
    }
    speed = std::max(0.0f, speed);
    al_set_sample_instance_speed(playback_ptr->allegro_sample_instance, speed);
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


/**
 * @brief Returns a source's pointer from a source in the list.
 *
 * @param source_id ID of the sound source.
 * @return The source, or nullptr if invalid.
 */
sfx_source_t* audio_manager::get_source(size_t source_id) {
    auto source_it = sources.find(source_id);
    if(source_it == sources.end()) return nullptr;
    return &source_it->second;
}


/**
 * @brief Handles a mob being deleted.
 *
 * @param m_ptr Mob that got deleted.
 */
void audio_manager::handle_mob_deletion(const mob* m_ptr) {
    for(auto s = mob_sources.begin(); s != mob_sources.end();) {
        if(s->second == m_ptr) {
            s = mob_sources.erase(s);
        } else {
            ++s;
        }
    }
}


/**
 * @brief Handles the gameplay of the game world being paused.
 */
void audio_manager::handle_world_pause() {
    //Pause playbacks.
    for(size_t p = 0; p < playbacks.size(); ++p) {
        sfx_playback_t* playback_ptr = &playbacks[p];
        if(playback_ptr->state == SFX_PLAYBACK_STATE_DESTROYED) {
            continue;
        }
        
        sfx_source_t* source_ptr = get_source(playback_ptr->source_id);
        if(!source_ptr) continue;
        
        if(
            source_ptr->type == SFX_TYPE_WORLD_GLOBAL ||
            source_ptr->type == SFX_TYPE_WORLD_POS ||
            source_ptr->type == SFX_TYPE_WORLD_AMBIANCE
        ) {
            playback_ptr->state = SFX_PLAYBACK_STATE_PAUSING;
        }
    }
    
    //Soften songs.
    for(auto &s : songs) {
        if(
            s.second.state == SONG_STATE_STOPPING ||
            s.second.state == SONG_STATE_STOPPED
        ) {
            continue;
        }
        s.second.state = SONG_STATE_SOFTENING;
    }
}


/**
 * @brief Handles the gameplay of the game world being unpaused.
 */
void audio_manager::handle_world_unpause() {
    //Unpause playbacks.
    for(size_t p = 0; p < playbacks.size(); ++p) {
        sfx_playback_t* playback_ptr = &playbacks[p];
        if(playback_ptr->state == SFX_PLAYBACK_STATE_DESTROYED) {
            continue;
        }
        
        sfx_source_t* source_ptr = get_source(playback_ptr->source_id);
        if(!source_ptr) continue;
        
        if(
            source_ptr->type == SFX_TYPE_WORLD_GLOBAL ||
            source_ptr->type == SFX_TYPE_WORLD_POS ||
            source_ptr->type == SFX_TYPE_WORLD_AMBIANCE
        ) {
            playback_ptr->state = SFX_PLAYBACK_STATE_UNPAUSING;
            al_set_sample_instance_playing(
                playback_ptr->allegro_sample_instance,
                true
            );
            al_set_sample_instance_position(
                playback_ptr->allegro_sample_instance,
                playback_ptr->pre_pause_pos
            );
        }
    }
    
    //Unsoften songs.
    for(auto &s : songs) {
        if(
            s.second.state == SONG_STATE_STOPPING ||
            s.second.state == SONG_STATE_STOPPED
        ) {
            continue;
        }
        s.second.state = SONG_STATE_UNSOFTENING;
    }
}


/**
 * @brief Initializes the audio manager.
 *
 * @param master_volume Volume of the master mixer.
 * @param world_sfx_volume Volume of the in-world sound effects mixer.
 * @param music_volume Volume of the music mixer.
 * @param ambiance_volume Volume of the ambiance sounds mixer.
 * @param ui_sfx_volume Volume of the UI sound effects mixer.
 */
void audio_manager::init(
    float master_volume, float world_sfx_volume, float music_volume,
    float ambiance_volume, float ui_sfx_volume
) {
    //Main voice.
    voice =
        al_create_voice(
            44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2
        );
        
    //Master mixer.
    master_mixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_voice(master_mixer, voice);
    
    //World sound effects mixer.
    world_sfx_mixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_mixer(world_sfx_mixer, master_mixer);
    
    //Music mixer.
    music_mixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_mixer(music_mixer, master_mixer);
    
    //World ambiance sounds mixer.
    world_ambiance_sfx_mixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_mixer(world_ambiance_sfx_mixer, master_mixer);
    
    //UI sound effects mixer.
    ui_sfx_mixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_mixer(ui_sfx_mixer, master_mixer);
    
    //Set all of the mixer volumes.
    update_volumes(
        master_volume,
        world_sfx_volume,
        music_volume,
        ambiance_volume,
        ui_sfx_volume
    );
    
    //Initialization of every mix track type.
    for(size_t m = 0; m < N_MIX_TRACK_TYPES; ++m) {
        mix_statuses.push_back(false);
        mix_volumes.push_back(0.0f);
    }
}


/**
 * @brief Marks a mix track type's status to true for this frame.
 *
 * @param track_type Track type to mark.
 */
void audio_manager::mark_mix_track_status(MIX_TRACK_TYPE track_type) {
    mix_statuses[track_type] = true;
}


/**
 * @brief Schedules a sound effect source's emission. This includes things
 * like randomly delaying it if configured to do so.
 *
 * @param source_id ID of the sound source.
 * @param first True if this is the first emission of the source.
 * @return Whether it succeeded.
 */
bool audio_manager::schedule_emission(size_t source_id, bool first) {
    sfx_source_t* source_ptr = get_source(source_id);
    if(!source_ptr) return false;
    
    source_ptr->emit_time_left = first ? 0.0f : source_ptr->config.interval;
    if(first || source_ptr->config.interval > 0.0f) {
        source_ptr->emit_time_left +=
            randomf(0, source_ptr->config.random_delay);
    }
    
    return true;
}


/**
 * @brief Sets the camera's position.
 *
 * @param cam_tl Current coordinates of the camera's top-left corner.
 * @param cam_br Current coordinates of the camera's bottom-right corner.
 */
void audio_manager::set_camera_pos(const point &cam_tl, const point &cam_br) {
    this->cam_tl = cam_tl;
    this->cam_br = cam_br;
}


/**
 * @brief Sets what the current song should be.
 *
 * If it's different from the song that's currently playing,
 * then that one fades out as this one fades in.
 * To stop playing songs, send an empty string as the song name argument.
 *
 * @param name Name of the song in the list of loaded songs.
 * @param from_start If true, the song starts from the beginning,
 * otherwise it starts from where it left off.
 * This argument only applies if the song was stopped.
 * @return Whether it succeeded.
 */
bool audio_manager::set_current_song(const string &name, bool from_start) {

    //Stop all other songs first.
    for(auto &s : songs) {
        song* song_ptr = &s.second;
        if(song_ptr->name == name) {
            //This is the song we want to play. Let's not handle it here.
            continue;
        }
        switch(song_ptr->state) {
        case SONG_STATE_STARTING:
        case SONG_STATE_PLAYING:
        case SONG_STATE_SOFTENING:
        case SONG_STATE_SOFTENED:
        case SONG_STATE_UNSOFTENING: {
            song_ptr->state = SONG_STATE_STOPPING;
            break;
        } case SONG_STATE_STOPPING:
        case SONG_STATE_STOPPED: {
            //Already stopped, or stopping.
            break;
        }
        }
        //TODO
    }
    
    //Get the new song to play, if applicable.
    if(name.empty()) {
        //If the name's empty, we just wanted to stop all songs.
        //Meaning we're done here.
        return true;
    }
    
    auto song_it = songs.find(name);
    if(song_it == songs.end()) return false;
    song* song_ptr = &song_it->second;
    
    //Play it.
    switch(song_ptr->state) {
    case SONG_STATE_STARTING:
    case SONG_STATE_PLAYING:
    case SONG_STATE_SOFTENING:
    case SONG_STATE_SOFTENED:
    case SONG_STATE_UNSOFTENING: {
        //Already playing.
        break;
    } case SONG_STATE_STOPPING: {
        //We need it to go back, not stop.
        song_ptr->state = SONG_STATE_STARTING;
        break;
    } case SONG_STATE_STOPPED: {
        //Start it.
        if(song_ptr->state == SONG_STATE_STOPPED) {
            start_song_track(song_ptr, song_ptr->main_track, from_start);
            for(auto const &m : song_ptr->mix_tracks) {
                start_song_track(song_ptr, m.second, from_start);
            }
        }
        song_ptr->state = SONG_STATE_STARTING;
    }
    }
    
    return true;
}


/**
 * @brief Sets the position of a positional sound effect source.
 *
 * @param source_id ID of the sound effect source.
 * @param pos New position.
 * @return Whether it succeeded.
 */
bool audio_manager::set_sfx_source_pos(size_t source_id, const point &pos) {
    sfx_source_t* source_ptr = get_source(source_id);
    if(!source_ptr) return false;
    
    source_ptr->pos = pos;
    
    return true;
}


/**
 * @brief Starts playing a song's track from scratch.
 *
 * @param song_ptr The song.
 * @param stream Audio stream of the track.
 * @param from_start If true, the song starts from the beginning,
 * otherwise it starts from where it left off.
 */
void audio_manager::start_song_track(
    song* song_ptr, ALLEGRO_AUDIO_STREAM* stream, bool from_start
) {
    if(!stream) return;
    al_set_audio_stream_gain(stream, 0.0f);
    al_seek_audio_stream_secs(stream, from_start ? 0.0f : song_ptr->stop_point);
    al_set_audio_stream_loop_secs(
        stream, song_ptr->loop_start, song_ptr->loop_end
    );
    al_set_audio_stream_playmode(stream, ALLEGRO_PLAYMODE_LOOP);
    
    al_attach_audio_stream_to_mixer(stream, music_mixer);
    al_set_audio_stream_playing(stream, true);
}


/**
 * @brief Stops all playbacks. Alternatively, stops all playbacks of
 * a given sound sample.
 *
 * @param filter Sound sample to filter by, or nullptr to stop all playbacks.
 */
void audio_manager::stop_all_playbacks(const ALLEGRO_SAMPLE* filter) {
    for(size_t p = 0; p < playbacks.size(); ++p) {
        bool to_stop = false;
        
        if(!filter) {
            to_stop = true;
        } else {
            sfx_playback_t* playback_ptr = &playbacks[p];
            sfx_source_t* source_ptr =
                get_source(playback_ptr->source_id);
            if(source_ptr && source_ptr->sample == filter) {
                to_stop = true;
            }
        }
        
        if(to_stop) {
            stop_sfx_playback(p);
        }
    }
}


/**
 * @brief Stops a playback, putting it in the "stopping" state.
 *
 * @param playback_idx Index of the playback in the list of playbacks.
 * @return Whether it succeeded.
 */
bool audio_manager::stop_sfx_playback(size_t playback_idx) {
    sfx_playback_t* playback_ptr = &playbacks[playback_idx];
    if(playback_ptr->state == SFX_PLAYBACK_STATE_STOPPING) return false;
    if(playback_ptr->state == SFX_PLAYBACK_STATE_DESTROYED) return false;
    playback_ptr->state = SFX_PLAYBACK_STATE_STOPPING;
    return true;
}


/**
 * @brief Ticks the audio manager by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
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
    
    //Emit playbacks from sources that want to emit.
    for(auto &s : sources) {
        if(s.second.destroyed) continue;
        if(s.second.emit_time_left == 0.0f) continue;
        
        s.second.emit_time_left -= delta_t;
        if(s.second.emit_time_left <= 0.0f) {
            emit(s.first);
            schedule_emission(s.first, false);
        }
    }
    
    //Update playbacks.
    for(size_t p = 0; p < playbacks.size(); ++p) {
        sfx_playback_t* playback_ptr = &playbacks[p];
        if(playback_ptr->state == SFX_PLAYBACK_STATE_DESTROYED) continue;
        
        if(
            !al_get_sample_instance_playing(
                playback_ptr->allegro_sample_instance
            ) &&
            playback_ptr->state != SFX_PLAYBACK_STATE_PAUSED
        ) {
            //Finished playing entirely.
            destroy_sfx_playback(p);
            
        } else {
            //Update target gain and pan, based on in-world position,
            //if applicable.
            update_playback_target_gain_and_pan(p);
            
            //Inch the gain and pan to the target values.
            playback_ptr->gain =
                inch_towards(
                    playback_ptr->gain,
                    playback_ptr->target_gain,
                    AUDIO::PLAYBACK_GAIN_SPEED * delta_t
                );
            playback_ptr->pan =
                inch_towards(
                    playback_ptr->pan,
                    playback_ptr->target_pan,
                    AUDIO::PLAYBACK_PAN_SPEED * delta_t
                );
                
            //Pausing and unpausing.
            if(playback_ptr->state == SFX_PLAYBACK_STATE_PAUSING) {
                playback_ptr->state_gain_mult -=
                    AUDIO::PLAYBACK_PAUSE_GAIN_SPEED * delta_t;
                if(playback_ptr->state_gain_mult <= 0.0f) {
                    playback_ptr->state_gain_mult = 0.0f;
                    playback_ptr->state = SFX_PLAYBACK_STATE_PAUSED;
                    playback_ptr->pre_pause_pos =
                        al_get_sample_instance_position(
                            playback_ptr->allegro_sample_instance
                        );
                    al_set_sample_instance_playing(
                        playback_ptr->allegro_sample_instance,
                        false
                    );
                }
            } else if(playback_ptr->state == SFX_PLAYBACK_STATE_UNPAUSING) {
                playback_ptr->state_gain_mult +=
                    AUDIO::PLAYBACK_PAUSE_GAIN_SPEED * delta_t;
                if(playback_ptr->state_gain_mult >= 1.0f) {
                    playback_ptr->state_gain_mult = 1.0f;
                    playback_ptr->state = SFX_PLAYBACK_STATE_PLAYING;
                }
            }
            
            //Stopping.
            if(playback_ptr->state == SFX_PLAYBACK_STATE_STOPPING) {
                playback_ptr->state_gain_mult -=
                    AUDIO::PLAYBACK_STOP_GAIN_SPEED * delta_t;
                if(playback_ptr->state_gain_mult <= 0.0f) {
                    destroy_sfx_playback(p);
                }
            }
            
            //Update the final gain and pan values.
            update_playback_gain_and_pan(p);
        }
    }
    
    //Delete destroyed playbacks.
    for(size_t p = 0; p < playbacks.size();) {
        if(playbacks[p].state == SFX_PLAYBACK_STATE_DESTROYED) {
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
    
    //Update the volume of songs depending on their state.
    for(auto &s : songs) {
        song* song_ptr = &s.second;
        
        switch(song_ptr->state) {
        case SONG_STATE_STARTING: {
            song_ptr->gain =
                inch_towards(
                    song_ptr->gain,
                    1.0f,
                    AUDIO::SONG_GAIN_SPEED * delta_t
                );
            al_set_audio_stream_gain(song_ptr->main_track, song_ptr->gain);
            if(song_ptr->gain == 1.0f) {
                song_ptr->state = SONG_STATE_PLAYING;
            }
            break;
        } case SONG_STATE_PLAYING: {
            //Nothing to do.
            break;
        } case SONG_STATE_SOFTENING: {
            song_ptr->gain =
                inch_towards(
                    song_ptr->gain,
                    AUDIO::SONG_SOFTENED_GAIN,
                    AUDIO::SONG_GAIN_SPEED * delta_t
                );
            al_set_audio_stream_gain(song_ptr->main_track, song_ptr->gain);
            if(song_ptr->gain == AUDIO::SONG_SOFTENED_GAIN) {
                song_ptr->state = SONG_STATE_SOFTENED;
            }
            break;
        } case SONG_STATE_SOFTENED: {
            //Nothing to do.
            break;
        } case SONG_STATE_UNSOFTENING: {
            song_ptr->gain =
                inch_towards(
                    song_ptr->gain,
                    1.0f,
                    AUDIO::SONG_GAIN_SPEED * delta_t
                );
            al_set_audio_stream_gain(song_ptr->main_track, song_ptr->gain);
            if(song_ptr->gain == 1.0f) {
                song_ptr->state = SONG_STATE_PLAYING;
            }
            break;
        } case SONG_STATE_STOPPING: {
            song_ptr->gain =
                inch_towards(
                    song_ptr->gain,
                    0.0f,
                    AUDIO::SONG_GAIN_SPEED * delta_t
                );
            al_set_audio_stream_gain(song_ptr->main_track, song_ptr->gain);
            if(song_ptr->gain == 0.0f) {
                al_set_audio_stream_playing(song_ptr->main_track, false);
                al_detach_audio_stream(song_ptr->main_track);
                for(auto &m : song_ptr->mix_tracks) {
                    al_set_audio_stream_playing(m.second, false);
                    al_detach_audio_stream(m.second);
                }
                song_ptr->stop_point =
                    al_get_audio_stream_position_secs(song_ptr->main_track);
                song_ptr->state = SONG_STATE_STOPPED;
            }
            break;
        } case SONG_STATE_STOPPED: {
            //Nothing to do.
            break;
        }
        }
    }
    
    //Update the status of mix track types, and their volumes.
    for(size_t m = 0; m < N_MIX_TRACK_TYPES; ++m) {
        mix_volumes[m] =
            inch_towards(
                mix_volumes[m],
                mix_statuses[m] ? 1.0f : 0.0f,
                AUDIO::MIX_TRACK_GAIN_SPEED * delta_t
            );
            
        for(auto &s : songs) {
            song* song_ptr = &s.second;
            if(song_ptr->state == SONG_STATE_STOPPED) {
                continue;
            }
            
            auto track_it = song_ptr->mix_tracks.find((MIX_TRACK_TYPE) m);
            if(track_it == song_ptr->mix_tracks.end()) continue;
            
            al_set_audio_stream_gain(
                track_it->second, mix_volumes[m] * song_ptr->gain
            );
        }
        
    }
    
    //Prepare the statuses for the next frame.
    for(size_t s = 0; s < N_MIX_TRACK_TYPES; ++s) {
        mix_statuses[s] = false;
    }
}


/**
 * @brief Instantly updates a playback's current gain and pan, using its member
 * variables. This also clamps the variables if needed.
 *
 * @param playback_idx Index of the playback in the list.
 */
void audio_manager::update_playback_gain_and_pan(size_t playback_idx) {
    if(playback_idx >= playbacks.size()) return;
    sfx_playback_t* playback_ptr = &playbacks[playback_idx];
    
    playback_ptr->gain = clamp(playback_ptr->gain, 0.0f, 1.0f);
    float final_gain = playback_ptr->gain * playback_ptr->state_gain_mult;
    final_gain *= playback_ptr->base_gain;
    final_gain = clamp(final_gain, 0.0f, 1.0f);
    al_set_sample_instance_gain(
        playback_ptr->allegro_sample_instance,
        final_gain
    );
    
    playback_ptr->pan = clamp(playback_ptr->pan, -1.0f, 1.0f);
    
    al_set_sample_instance_pan(
        playback_ptr->allegro_sample_instance,
        playback_ptr->pan
    );
}


/**
 * @brief Updates a playback's target gain and target pan, based on distance
 * from the camera.
 *
 * This won't update the gain and pan yet, but each audio
 * manager tick will be responsible for bringing the gain and pan to these
 * values smoothly over time.
 *
 * @param playback_idx Index of the playback in the list.
 */
void audio_manager::update_playback_target_gain_and_pan(size_t playback_idx) {
    if(playback_idx >= playbacks.size()) return;
    sfx_playback_t* playback_ptr = &playbacks[playback_idx];
    
    sfx_source_t* source_ptr = get_source(playback_ptr->source_id);
    if(!source_ptr || source_ptr->type != SFX_TYPE_WORLD_POS) return;
    
    //Calculate screen and camera things.
    point screen_size = cam_br - cam_tl;
    if(screen_size.x == 0.0f || screen_size.y == 0.0f) return;
    
    point cam_center = (cam_tl + cam_br) / 2.0f;
    float d = dist(cam_center, source_ptr->pos).to_float();
    point delta = source_ptr->pos - cam_center;
    
    //Set the gain.
    float gain =
        interpolate_number(
            fabs(d),
            AUDIO::PLAYBACK_RANGE_CLOSE, AUDIO::PLAYBACK_RANGE_FAR_GAIN,
            1.0f, 0.0f
        );
    gain = clamp(gain, 0.0f, 1.0f);
    playback_ptr->target_gain = gain;
    
    //Set the pan.
    float pan_abs =
        interpolate_number(
            fabs(delta.x),
            AUDIO::PLAYBACK_RANGE_CLOSE, AUDIO::PLAYBACK_RANGE_FAR_PAN,
            0.0f, 1.0f
        );
    pan_abs = clamp(pan_abs, 0.0f, 1.0f);
    float pan = delta.x > 0.0f ? pan_abs : -pan_abs;
    playback_ptr->target_pan = pan;
}


/**
 * @brief Updates the volumes of all mixers.
 *
 * @param master_volume Volume of the master mixer.
 * @param world_sfx_volume Volume of the in-world sound effects mixer.
 * @param music_volume Volume of the music mixer.
 * @param ambiance_volume Volume of the ambiance sounds mixer.
 * @param ui_sfx_volume Volume of the UI sound effects mixer.
 */
void audio_manager::update_volumes(
    float master_volume, float world_sfx_volume, float music_volume,
    float ambiance_volume, float ui_sfx_volume
) {
    master_volume = clamp(master_volume, 0.0f, 1.0f);
    al_set_mixer_gain(master_mixer, master_volume);
    
    world_sfx_volume = clamp(world_sfx_volume, 0.0f, 1.0f);
    al_set_mixer_gain(world_sfx_mixer, world_sfx_volume);
    
    music_volume = clamp(music_volume, 0.0f, 1.0f);
    al_set_mixer_gain(music_mixer, music_volume);
    
    ambiance_volume = clamp(ambiance_volume, 0.0f, 1.0f);
    al_set_mixer_gain(world_ambiance_sfx_mixer, ambiance_volume);
    
    ui_sfx_volume = clamp(ui_sfx_volume, 0.0f, 1.0f);
    al_set_mixer_gain(ui_sfx_mixer, ui_sfx_volume);
}


/**
 * @brief Loads song data from a data node.
 * 
 * @param node Data node to load from.
 */
void song::load_from_data_node(data_node* node) {
    //Content metadata.
    load_metadata_from_data_node(node);

    //Standard data.
    reader_setter rs(node);

    string main_track_str;
    data_node* main_track_node = nullptr;
    
    rs.set("main_track", main_track_str, &main_track_node);
    rs.set("loop_start", loop_start);
    rs.set("loop_end", loop_end);
    rs.set("title", title);
    
    main_track =
        game.audio.streams.get(main_track_str, main_track_node);
        
    data_node* mix_tracks_node = node->get_child_by_name("mix_tracks");
    size_t n_mix_tracks = mix_tracks_node->get_nr_of_children();
    
    for(size_t m = 0; m < n_mix_tracks; ++m) {
        data_node* mix_track_node = mix_tracks_node->get_child(m);
        MIX_TRACK_TYPE trigger = N_MIX_TRACK_TYPES;
        
        if(mix_track_node->name == "enemy") {
            trigger = MIX_TRACK_TYPE_ENEMY;
        } else {
            game.errors.report(
                "Unknown mix track trigger \"" +
                mix_track_node->name +
                "\"!", mix_track_node
            );
            continue;
        }
        
        mix_tracks[trigger] =
            game.audio.streams.get(mix_track_node->value, mix_track_node);
    }
    
    if(loop_end < loop_start) {
        loop_start = 0.0f;
    }
}
