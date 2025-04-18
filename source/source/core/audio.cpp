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

#include "../util/general_utils.h"
#include "game.h"
#include "load.h"
#include "misc_functions.h"


namespace AUDIO {

//Default min stack pos. Let's use a value higher than 0, since if for any
//reason the same sound plays multiple times at once, they are actually
//stopped under the SOUND_STACK_MODE_NORMAL mode,
//thus perventing a super-loud sound.
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
 * @brief Creates an in-world global sound effect source and returns its ID.
 *
 * This is basically how you can get the engine to produce a sound that doesn't
 * involve a position in the game world.
 *
 * @param sample Sound sample that this source will emit.
 * @param ambiance Whether it's an ambiance sound or a gameplay sound.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t AudioManager::createGlobalSoundSource(
    ALLEGRO_SAMPLE* sample, bool ambiance,
    const SoundSourceConfig &config
) {
    return
        createSoundSource(
            sample,
            ambiance ? SOUND_TYPE_AMBIANCE_GLOBAL : SOUND_TYPE_GAMEPLAY_GLOBAL,
            config, Point()
        );
}


/**
 * @brief Creates an in-world mob sound effect source and returns its ID.
 *
 * This is like createPosSoundSource, but ties the source to the mob,
 * meaning the audio manager is responsible for updating the source's position
 * every frame to match the mob's.
 *
 * @param sample Sound sample that this source will emit.
 * @param m_ptr Pointer to the mob.
 * @param ambiance Whether it's an ambiance sound or a gameplay sound.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t AudioManager::createMobSoundSource(
    ALLEGRO_SAMPLE* sample,
    Mob* m_ptr, bool ambiance,
    const SoundSourceConfig &config
) {
    size_t source_id =
        createSoundSource(
            sample,
            ambiance ? SOUND_TYPE_AMBIANCE_POS : SOUND_TYPE_GAMEPLAY_POS,
            config, m_ptr->pos
        );
    mobSources[source_id] = m_ptr;
    return source_id;
}


/**
 * @brief Creates an in-world positional sound effect source and returns its ID.
 *
 * This is basically how you can get the engine to produce a sound that
 * involves a position in the game world.
 *
 * @param sample Sound sample that this source will emit.
 * @param pos Starting position in the game world.
 * @param ambiance Whether it's an ambiance sound or a gameplay sound.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t AudioManager::createPosSoundSource(
    ALLEGRO_SAMPLE* sample, const Point &pos, bool ambiance,
    const SoundSourceConfig &config
) {
    return
        createSoundSource(
            sample,
            ambiance ? SOUND_TYPE_AMBIANCE_POS : SOUND_TYPE_GAMEPLAY_POS,
            config, pos
        );
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
size_t AudioManager::createSoundSource(
    ALLEGRO_SAMPLE* sample,
    SOUND_TYPE type,
    const SoundSourceConfig &config,
    const Point &pos
) {
    if(!sample) return 0;
    
    size_t id = nextSoundSourceId;
    
    sources[id] = SoundSource();
    sources[id].sample = sample;
    sources[id].type = type;
    sources[id].config = config;
    sources[id].pos = pos;
    
    if(!hasFlag(config.flags, SOUND_FLAG_DONT_EMIT_ON_CREATION)) {
        scheduleEmission(id, true);
        if(sources[id].emitTimeLeft <= 0.0f) {
            emit(id);
            scheduleEmission(id, false);
        }
    }
    
    nextSoundSourceId++; //Hopefully there will be no collisions.
    
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
size_t AudioManager::createUiSoundsource(
    ALLEGRO_SAMPLE* sample,
    const SoundSourceConfig &config
) {
    return createSoundSource(sample, SOUND_TYPE_UI, config, Point());
}


/**
 * @brief Destroys the audio manager.
 */
void AudioManager::destroy() {
    al_detach_voice(voice);
    al_destroy_mixer(gameplaySoundMixer);
    al_destroy_mixer(musicMixer);
    al_destroy_mixer(ambianceSoundMixer);
    al_destroy_mixer(uiSoundMixer);
    al_destroy_mixer(masterMixer);
    al_destroy_voice(voice);
}


/**
 * @brief Destroys a playback object directly.
 * The "stopping" state is not relevant here.
 *
 * @param playback_idx Index of the playback in the list of playbacks.
 * @return Whether it succeeded.
 */
bool AudioManager::destroySoundPlayback(size_t playback_idx) {
    SoundPlayback* playback_ptr = &playbacks[playback_idx];
    if(playback_ptr->state == SOUND_PLAYBACK_STATE_DESTROYED) return false;
    playback_ptr->state = SOUND_PLAYBACK_STATE_DESTROYED;
    
    SoundSource* source_ptr = getSource(playback_ptr->sourceId);
    
    //Destroy the source, if applicable.
    if(source_ptr) {
        if(!hasFlag(source_ptr->config.flags, SOUND_FLAG_KEEP_ON_PLAYBACK_END)) {
            destroySoundSource(playback_ptr->sourceId);
        }
    }
    
    //Destroy the Allegro sample instance.
    ALLEGRO_SAMPLE_INSTANCE* instance =
        playback_ptr->allegroSampleInstance;
    if(instance) {
        al_set_sample_instance_playing(instance, false);
        al_detach_sample_instance(instance);
        if(instance) al_destroy_sample_instance(instance);
        playback_ptr->allegroSampleInstance = nullptr;
    }
    
    return true;
}


/**
 * @brief Destroys a sound source.
 *
 * @param source_id ID of the sound source to destroy.
 * @return Whether it succeeded.
 */
bool AudioManager::destroySoundSource(size_t source_id) {
    SoundSource* source_ptr = getSource(source_id);
    if(!source_ptr) return false;
    
    if(source_ptr->destroyed) return false;
    source_ptr->destroyed = true;
    
    //Check if we must stop playbacks.
    if(
        !hasFlag(
            source_ptr->config.flags,
            SOUND_FLAG_KEEP_PLAYBACK_ON_DESTROY
        )
    ) {
        for(size_t p = 0; p < playbacks.size(); p++) {
            if(playbacks[p].sourceId == source_id) {
                stopSoundPlayback(p);
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
bool AudioManager::emit(size_t source_id) {
    //Setup.
    SoundSource* source_ptr = getSource(source_id);
    if(!source_ptr) return false;
    
    ALLEGRO_SAMPLE* sample = source_ptr->sample;
    if(!sample) return false;
    
    //Check if other playbacks exist to prevent stacking.
    float lowest_stacking_playback_pos = FLT_MAX;
    if(
        source_ptr->config.stackMinPos > 0.0f ||
        source_ptr->config.stackMode == SOUND_STACK_MODE_NEVER
    ) {
        for(size_t p = 0; p < playbacks.size(); p++) {
            SoundPlayback* playback = &playbacks[p];
            SoundSource* p_source_ptr = getSource(playback->sourceId);
            if(!p_source_ptr || p_source_ptr->sample != sample) continue;
            
            float playback_pos =
                al_get_sample_instance_position(
                    playback->allegroSampleInstance
                ) /
                (float) al_get_sample_frequency(p_source_ptr->sample);
            lowest_stacking_playback_pos =
                std::min(lowest_stacking_playback_pos, playback_pos);
        }
        
        if(
            source_ptr->config.stackMinPos > 0.0f &&
            lowest_stacking_playback_pos < source_ptr->config.stackMinPos
        ) {
            //Can't emit. This would stack the sounds, and there are other
            //playbacks that haven't reached the minimum stack threshold yet.
            return false;
        }
        if(
            source_ptr->config.stackMode == SOUND_STACK_MODE_NEVER &&
            lowest_stacking_playback_pos < FLT_MAX
        ) {
            //Can't emit. This would stack the sounds.
            return false;
        }
    }
    
    //Check if other playbacks exist and if we need to stop them.
    if(source_ptr->config.stackMode == SOUND_STACK_MODE_OVERRIDE) {
        for(size_t p = 0; p < playbacks.size(); p++) {
            SoundPlayback* playback = &playbacks[p];
            SoundSource* p_source_ptr = getSource(playback->sourceId);
            if(!p_source_ptr || p_source_ptr->sample != sample) {
                continue;
            }
            stopSoundPlayback(p);
        }
    }
    
    //Create the playback.
    playbacks.push_back(SoundPlayback());
    SoundPlayback* playback_ptr = &playbacks.back();
    playback_ptr->sourceId = source_id;
    playback_ptr->allegroSampleInstance = al_create_sample_instance(sample);
    if(!playback_ptr->allegroSampleInstance) return false;
    
    playback_ptr->baseGain = source_ptr->config.gain;
    if(source_ptr->config.gainDeviation != 0.0f) {
        playback_ptr->baseGain +=
            game.rng.f(
                -source_ptr->config.gainDeviation,
                source_ptr->config.gainDeviation
            );
        playback_ptr->baseGain =
            std::clamp(playback_ptr->baseGain, 0.0f, 1.0f);
    }
    
    //Play.
    updatePlaybackTargetGainAndPan(playbacks.size() - 1);
    playback_ptr->gain = playback_ptr->targetGain;
    playback_ptr->pan = playback_ptr->targetPan;
    
    ALLEGRO_MIXER* mixer = nullptr;
    switch(source_ptr->type) {
    case SOUND_TYPE_GAMEPLAY_GLOBAL:
    case SOUND_TYPE_GAMEPLAY_POS: {
        mixer = gameplaySoundMixer;
        break;
    } case SOUND_TYPE_AMBIANCE_GLOBAL: {
    } case SOUND_TYPE_AMBIANCE_POS: {
        mixer = ambianceSoundMixer;
        break;
    } case SOUND_TYPE_UI: {
        mixer = uiSoundMixer;
        break;
    }
    }
    
    al_attach_sample_instance_to_mixer(
        playback_ptr->allegroSampleInstance, mixer
    );
    
    al_set_sample_instance_playmode(
        playback_ptr->allegroSampleInstance,
        hasFlag(source_ptr->config.flags, SOUND_FLAG_LOOP) ?
        ALLEGRO_PLAYMODE_LOOP :
        ALLEGRO_PLAYMODE_ONCE
    );
    float speed = source_ptr->config.speed;
    if(source_ptr->config.speedDeviation != 0.0f) {
        speed +=
            game.rng.f(
                -source_ptr->config.speedDeviation,
                source_ptr->config.speedDeviation
            );
    }
    speed = std::max(0.0f, speed);
    al_set_sample_instance_speed(playback_ptr->allegroSampleInstance, speed);
    updatePlaybackGainAndPan(playbacks.size() - 1);
    
    al_set_sample_instance_position(
        playback_ptr->allegroSampleInstance,
        0.0f
    );
    al_set_sample_instance_playing(
        playback_ptr->allegroSampleInstance,
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
SoundSource* AudioManager::getSource(size_t source_id) {
    auto source_it = sources.find(source_id);
    if(source_it == sources.end()) return nullptr;
    return &source_it->second;
}


/**
 * @brief Handles a mob being deleted.
 *
 * @param m_ptr Mob that got deleted.
 */
void AudioManager::handleMobDeletion(const Mob* m_ptr) {
    for(auto s = mobSources.begin(); s != mobSources.end();) {
        if(s->second == m_ptr) {
            s = mobSources.erase(s);
        } else {
            ++s;
        }
    }
}


/**
 * @brief Handles a non-looping Allegro audio stream being finished.
 *
 * @param stream Stream that finished.
 */
void AudioManager::handleStreamFinished(ALLEGRO_AUDIO_STREAM* stream) {
    for(const auto &s : game.content.songs.list) {
        if(s.second.mainTrack == stream) {
            if(onSongFinished) onSongFinished(s.first);
        }
    }
}


/**
 * @brief Handles the gameplay of the game world being paused.
 */
void AudioManager::handleWorldPause() {
    //Pause playbacks.
    for(size_t p = 0; p < playbacks.size(); p++) {
        SoundPlayback* playback_ptr = &playbacks[p];
        if(playback_ptr->state == SOUND_PLAYBACK_STATE_DESTROYED) {
            continue;
        }
        
        SoundSource* source_ptr = getSource(playback_ptr->sourceId);
        if(!source_ptr) continue;
        
        if(
            source_ptr->type == SOUND_TYPE_GAMEPLAY_GLOBAL ||
            source_ptr->type == SOUND_TYPE_GAMEPLAY_POS ||
            source_ptr->type == SOUND_TYPE_AMBIANCE_GLOBAL ||
            source_ptr->type == SOUND_TYPE_AMBIANCE_POS
        ) {
            playback_ptr->state = SOUND_PLAYBACK_STATE_PAUSING;
        }
    }
    
    //Soften songs.
    for(auto &s : game.content.songs.list) {
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
void AudioManager::handleWorldUnpause() {
    //Unpause playbacks.
    for(size_t p = 0; p < playbacks.size(); p++) {
        SoundPlayback* playback_ptr = &playbacks[p];
        if(playback_ptr->state == SOUND_PLAYBACK_STATE_DESTROYED) {
            continue;
        }
        
        SoundSource* source_ptr = getSource(playback_ptr->sourceId);
        if(!source_ptr) continue;
        
        if(
            source_ptr->type == SOUND_TYPE_GAMEPLAY_GLOBAL ||
            source_ptr->type == SOUND_TYPE_GAMEPLAY_POS ||
            source_ptr->type == SOUND_TYPE_AMBIANCE_GLOBAL ||
            source_ptr->type == SOUND_TYPE_AMBIANCE_POS
        ) {
            playback_ptr->state = SOUND_PLAYBACK_STATE_UNPAUSING;
            al_set_sample_instance_playing(
                playback_ptr->allegroSampleInstance,
                true
            );
            al_set_sample_instance_position(
                playback_ptr->allegroSampleInstance,
                playback_ptr->prePausePos
            );
        }
    }
    
    //Unsoften songs.
    for(auto &s : game.content.songs.list) {
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
 * @param gameplay_sound_volume Volume of the gameplay sound effects mixer.
 * @param music_volume Volume of the music mixer.
 * @param ambiance_sound_volume Volume of the ambiance sounds mixer.
 * @param ui_sound_volume Volume of the UI sound effects mixer.
 */
void AudioManager::init(
    float master_volume, float gameplay_sound_volume, float music_volume,
    float ambiance_sound_volume, float ui_sound_volume
) {
    //Main voice.
    voice =
        al_create_voice(
            44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2
        );
        
    //Master mixer.
    masterMixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_voice(masterMixer, voice);
    
    //Gameplay sound effects mixer.
    gameplaySoundMixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_mixer(gameplaySoundMixer, masterMixer);
    
    //Music mixer.
    musicMixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_mixer(musicMixer, masterMixer);
    
    //Ambiance sounds mixer.
    ambianceSoundMixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_mixer(ambianceSoundMixer, masterMixer);
    
    //UI sound effects mixer.
    uiSoundMixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_mixer(uiSoundMixer, masterMixer);
    
    //Set all of the mixer volumes.
    updateVolumes(
        master_volume,
        gameplay_sound_volume,
        music_volume,
        ambiance_sound_volume,
        ui_sound_volume
    );
    
    //Initialization of every mix track type.
    for(size_t m = 0; m < N_MIX_TRACK_TYPES; m++) {
        mixStatuses.push_back(false);
        mixVolumes.push_back(0.0f);
    }
}


/**
 * @brief Marks a mix track type's status to true for this frame.
 *
 * @param track_type Track type to mark.
 */
void AudioManager::markMixTrackStatus(MIX_TRACK_TYPE track_type) {
    mixStatuses[track_type] = true;
}


/**
 * @brief Sets a song's position to the beginning.
 *
 * @param name Name of the song in the list of loaded songs.
 * @return Whether it succeeded.
 */
bool AudioManager::rewindSong(const string &name) {
    auto song_it = game.content.songs.list.find(name);
    if(song_it == game.content.songs.list.end()) return false;
    Song* song_ptr = &song_it->second;
    
    song_ptr->stopPoint = 0.0f;
    al_rewind_audio_stream(song_ptr->mainTrack);
    for(auto const &m : song_ptr->mixTracks) {
        al_rewind_audio_stream(m.second);
    }
    
    return true;
}


/**
 * @brief Schedules a sound effect source's emission. This includes things
 * like randomly delaying it if configured to do so.
 *
 * @param source_id ID of the sound source.
 * @param first True if this is the first emission of the source.
 * @return Whether it succeeded.
 */
bool AudioManager::scheduleEmission(size_t source_id, bool first) {
    SoundSource* source_ptr = getSource(source_id);
    if(!source_ptr) return false;
    
    source_ptr->emitTimeLeft = first ? 0.0f : source_ptr->config.interval;
    if(first || source_ptr->config.interval > 0.0f) {
        source_ptr->emitTimeLeft +=
            game.rng.f(0, source_ptr->config.randomDelay);
    }
    
    return true;
}


/**
 * @brief Sets the camera's position.
 *
 * @param cam_tl Current coordinates of the camera's top-left corner.
 * @param cam_br Current coordinates of the camera's bottom-right corner.
 */
void AudioManager::setCameraPos(const Point &cam_tl, const Point &cam_br) {
    this->camTL = cam_tl;
    this->camBR = cam_br;
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
 * @param fade_in If true, the new song fades in like normal.
 * @param loop Whether it loops.
 * @return Whether it succeeded.
 */
bool AudioManager::setCurrentSong(
    const string &name, bool from_start, bool fade_in, bool loop
) {

    //Stop all other songs first.
    for(auto &s : game.content.songs.list) {
        Song* song_ptr = &s.second;
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
    }
    
    //Get the new song to play, if applicable.
    if(name.empty()) {
        //If the name's empty, we just wanted to stop all songs.
        //Meaning we're done here.
        return true;
    }
    
    auto song_it = game.content.songs.list.find(name);
    if(song_it == game.content.songs.list.end()) return false;
    Song* song_ptr = &song_it->second;
    
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
            startSongTrack(
                song_ptr, song_ptr->mainTrack, from_start, fade_in, loop
            );
            for(auto const &m : song_ptr->mixTracks) {
                startSongTrack(song_ptr, m.second, from_start, fade_in, loop);
            }
        }
        song_ptr->gain = fade_in ? 0.0f : 1.0f;
        song_ptr->state = fade_in ? SONG_STATE_STARTING : SONG_STATE_PLAYING;
    }
    }
    
    return true;
}


/**
 * @brief Sets the current position of all songs to be near the loop point.
 * This is helpful for when you want to test said loop point.
 */
void AudioManager::setSongPosNearLoop() {
    for(auto s : game.content.songs.list) {
        double pos = std::max(0.0, s.second.loopEnd - 4.0f);
        al_seek_audio_stream_secs(s.second.mainTrack, pos);
        for(auto const &m : s.second.mixTracks) {
            al_seek_audio_stream_secs(m.second, pos);
        }
    }
}


/**
 * @brief Sets the position of a positional sound effect source.
 *
 * @param source_id ID of the sound effect source.
 * @param pos New position.
 * @return Whether it succeeded.
 */
bool AudioManager::setSoundSourcePos(size_t source_id, const Point &pos) {
    SoundSource* source_ptr = getSource(source_id);
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
 * @param fade_in If true, the song starts fading in like normal.
 * @param loop Whether it loops.
 */
void AudioManager::startSongTrack(
    Song* song_ptr, ALLEGRO_AUDIO_STREAM* stream,
    bool from_start, bool fade_in, bool loop
) {
    if(!stream) return;
    al_set_audio_stream_gain(stream, fade_in ? 0.0f : 1.0f);
    al_seek_audio_stream_secs(stream, from_start ? 0.0f : song_ptr->stopPoint);
    al_set_audio_stream_loop_secs(
        stream, song_ptr->loopStart, song_ptr->loopEnd
    );
    al_set_audio_stream_playmode(
        stream, loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE
    );
    
    al_attach_audio_stream_to_mixer(stream, musicMixer);
    al_set_audio_stream_playing(stream, true);
}


/**
 * @brief Stops all playbacks. Alternatively, stops all playbacks of
 * a given sound sample.
 *
 * @param filter Sound sample to filter by, or nullptr to stop all playbacks.
 */
void AudioManager::stopAllPlaybacks(const ALLEGRO_SAMPLE* filter) {
    for(size_t p = 0; p < playbacks.size(); p++) {
        bool to_stop = false;
        
        if(!filter) {
            to_stop = true;
        } else {
            SoundPlayback* playback_ptr = &playbacks[p];
            SoundSource* source_ptr =
                getSource(playback_ptr->sourceId);
            if(source_ptr && source_ptr->sample == filter) {
                to_stop = true;
            }
        }
        
        if(to_stop) {
            stopSoundPlayback(p);
        }
    }
}


/**
 * @brief Stops a playback, putting it in the "stopping" state.
 *
 * @param playback_idx Index of the playback in the list of playbacks.
 * @return Whether it succeeded.
 */
bool AudioManager::stopSoundPlayback(size_t playback_idx) {
    SoundPlayback* playback_ptr = &playbacks[playback_idx];
    if(playback_ptr->state == SOUND_PLAYBACK_STATE_STOPPING) return false;
    if(playback_ptr->state == SOUND_PLAYBACK_STATE_DESTROYED) return false;
    playback_ptr->state = SOUND_PLAYBACK_STATE_STOPPING;
    return true;
}


/**
 * @brief Ticks the audio manager by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void AudioManager::tick(float delta_t) {
    //Clear deleted mob sources.
    for(auto s = mobSources.begin(); s != mobSources.end();) {
        Mob* mob_ptr = s->second;
        if(!mob_ptr || mob_ptr->toDelete) {
            s = mobSources.erase(s);
        } else {
            ++s;
        }
    }
    
    //Update the position of sources tied to mobs.
    for(auto &s : sources) {
        if(s.second.destroyed) continue;
        auto mob_source_it = mobSources.find(s.first);
        if(mob_source_it == mobSources.end()) continue;
        Mob* mob_ptr = mob_source_it->second;
        if(!mob_ptr || mob_ptr->toDelete) continue;
        s.second.pos = mob_ptr->pos;
    }
    
    //Emit playbacks from sources that want to emit.
    for(auto &s : sources) {
        if(s.second.destroyed) continue;
        if(s.second.emitTimeLeft == 0.0f) continue;
        
        s.second.emitTimeLeft -= delta_t;
        if(s.second.emitTimeLeft <= 0.0f) {
            emit(s.first);
            scheduleEmission(s.first, false);
        }
    }
    
    //Update playbacks.
    for(size_t p = 0; p < playbacks.size(); p++) {
        SoundPlayback* playback_ptr = &playbacks[p];
        if(playback_ptr->state == SOUND_PLAYBACK_STATE_DESTROYED) continue;
        
        if(
            !al_get_sample_instance_playing(
                playback_ptr->allegroSampleInstance
            ) &&
            playback_ptr->state != SOUND_PLAYBACK_STATE_PAUSED
        ) {
            //Finished playing entirely.
            destroySoundPlayback(p);
            
        } else {
            //Update target gain and pan, based on in-world position,
            //if applicable.
            updatePlaybackTargetGainAndPan(p);
            
            //Inch the gain and pan to the target values.
            playback_ptr->gain =
                inchTowards(
                    playback_ptr->gain,
                    playback_ptr->targetGain,
                    AUDIO::PLAYBACK_GAIN_SPEED * delta_t
                );
            playback_ptr->pan =
                inchTowards(
                    playback_ptr->pan,
                    playback_ptr->targetPan,
                    AUDIO::PLAYBACK_PAN_SPEED * delta_t
                );
                
            //Pausing and unpausing.
            if(playback_ptr->state == SOUND_PLAYBACK_STATE_PAUSING) {
                playback_ptr->stateGainMult -=
                    AUDIO::PLAYBACK_PAUSE_GAIN_SPEED * delta_t;
                if(playback_ptr->stateGainMult <= 0.0f) {
                    playback_ptr->stateGainMult = 0.0f;
                    playback_ptr->state = SOUND_PLAYBACK_STATE_PAUSED;
                    playback_ptr->prePausePos =
                        al_get_sample_instance_position(
                            playback_ptr->allegroSampleInstance
                        );
                    al_set_sample_instance_playing(
                        playback_ptr->allegroSampleInstance,
                        false
                    );
                }
            } else if(playback_ptr->state == SOUND_PLAYBACK_STATE_UNPAUSING) {
                playback_ptr->stateGainMult +=
                    AUDIO::PLAYBACK_PAUSE_GAIN_SPEED * delta_t;
                if(playback_ptr->stateGainMult >= 1.0f) {
                    playback_ptr->stateGainMult = 1.0f;
                    playback_ptr->state = SOUND_PLAYBACK_STATE_PLAYING;
                }
            }
            
            //Stopping.
            if(playback_ptr->state == SOUND_PLAYBACK_STATE_STOPPING) {
                playback_ptr->stateGainMult -=
                    AUDIO::PLAYBACK_STOP_GAIN_SPEED * delta_t;
                if(playback_ptr->stateGainMult <= 0.0f) {
                    destroySoundPlayback(p);
                }
            }
            
            //Update the final gain and pan values.
            updatePlaybackGainAndPan(p);
        }
    }
    
    //Delete destroyed playbacks.
    for(size_t p = 0; p < playbacks.size();) {
        if(playbacks[p].state == SOUND_PLAYBACK_STATE_DESTROYED) {
            playbacks.erase(playbacks.begin() + p);
        } else {
            p++;
        }
    }
    
    //Delete destroyed sources.
    for(auto s = sources.begin(); s != sources.end();) {
        if(s->second.destroyed) {
            auto mob_source_it = mobSources.find(s->first);
            if(mob_source_it != mobSources.end()) {
                mobSources.erase(mob_source_it);
            }
            s = sources.erase(s);
        } else {
            ++s;
        }
    }
    
    //Update the volume of songs depending on their state.
    for(auto &s : game.content.songs.list) {
        Song* song_ptr = &s.second;
        
        switch(song_ptr->state) {
        case SONG_STATE_STARTING: {
            song_ptr->gain =
                inchTowards(
                    song_ptr->gain,
                    1.0f,
                    AUDIO::SONG_GAIN_SPEED * delta_t
                );
            al_set_audio_stream_gain(song_ptr->mainTrack, song_ptr->gain);
            if(song_ptr->gain == 1.0f) {
                song_ptr->state = SONG_STATE_PLAYING;
            }
            break;
        } case SONG_STATE_PLAYING: {
            //Nothing to do.
            break;
        } case SONG_STATE_SOFTENING: {
            song_ptr->gain =
                inchTowards(
                    song_ptr->gain,
                    AUDIO::SONG_SOFTENED_GAIN,
                    AUDIO::SONG_GAIN_SPEED * delta_t
                );
            al_set_audio_stream_gain(song_ptr->mainTrack, song_ptr->gain);
            if(song_ptr->gain == AUDIO::SONG_SOFTENED_GAIN) {
                song_ptr->state = SONG_STATE_SOFTENED;
            }
            break;
        } case SONG_STATE_SOFTENED: {
            //Nothing to do.
            break;
        } case SONG_STATE_UNSOFTENING: {
            song_ptr->gain =
                inchTowards(
                    song_ptr->gain,
                    1.0f,
                    AUDIO::SONG_GAIN_SPEED * delta_t
                );
            al_set_audio_stream_gain(song_ptr->mainTrack, song_ptr->gain);
            if(song_ptr->gain == 1.0f) {
                song_ptr->state = SONG_STATE_PLAYING;
            }
            break;
        } case SONG_STATE_STOPPING: {
            song_ptr->gain =
                inchTowards(
                    song_ptr->gain,
                    0.0f,
                    AUDIO::SONG_GAIN_SPEED * delta_t
                );
            al_set_audio_stream_gain(song_ptr->mainTrack, song_ptr->gain);
            if(song_ptr->gain == 0.0f) {
                al_set_audio_stream_playing(song_ptr->mainTrack, false);
                al_detach_audio_stream(song_ptr->mainTrack);
                for(auto &m : song_ptr->mixTracks) {
                    al_set_audio_stream_playing(m.second, false);
                    al_detach_audio_stream(m.second);
                }
                song_ptr->stopPoint =
                    al_get_audio_stream_position_secs(song_ptr->mainTrack);
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
    for(size_t m = 0; m < N_MIX_TRACK_TYPES; m++) {
        mixVolumes[m] =
            inchTowards(
                mixVolumes[m],
                mixStatuses[m] ? 1.0f : 0.0f,
                AUDIO::MIX_TRACK_GAIN_SPEED * delta_t
            );
            
        for(auto &s : game.content.songs.list) {
            Song* song_ptr = &s.second;
            if(song_ptr->state == SONG_STATE_STOPPED) {
                continue;
            }
            
            auto track_it = song_ptr->mixTracks.find((MIX_TRACK_TYPE) m);
            if(track_it == song_ptr->mixTracks.end()) continue;
            
            al_set_audio_stream_gain(
                track_it->second, mixVolumes[m] * song_ptr->gain
            );
        }
        
    }
    
    //Prepare the statuses for the next frame.
    for(size_t s = 0; s < N_MIX_TRACK_TYPES; s++) {
        mixStatuses[s] = false;
    }
}


/**
 * @brief Instantly updates a playback's current gain and pan, using its member
 * variables. This also clamps the variables if needed.
 *
 * @param playback_idx Index of the playback in the list.
 */
void AudioManager::updatePlaybackGainAndPan(size_t playback_idx) {
    if(playback_idx >= playbacks.size()) return;
    SoundPlayback* playback_ptr = &playbacks[playback_idx];
    if(playback_ptr->state == SOUND_PLAYBACK_STATE_DESTROYED) return;
    
    playback_ptr->gain = std::clamp(playback_ptr->gain, 0.0f, 1.0f);
    float final_gain = playback_ptr->gain * playback_ptr->stateGainMult;
    final_gain *= playback_ptr->baseGain;
    final_gain = std::clamp(final_gain, 0.0f, 1.0f);
    al_set_sample_instance_gain(
        playback_ptr->allegroSampleInstance,
        final_gain
    );
    
    playback_ptr->pan = std::clamp(playback_ptr->pan, -1.0f, 1.0f);
    
    al_set_sample_instance_pan(
        playback_ptr->allegroSampleInstance,
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
void AudioManager::updatePlaybackTargetGainAndPan(size_t playback_idx) {
    if(playback_idx >= playbacks.size()) return;
    SoundPlayback* playback_ptr = &playbacks[playback_idx];
    if(playback_ptr->state == SOUND_PLAYBACK_STATE_DESTROYED) return;
    
    SoundSource* source_ptr = getSource(playback_ptr->sourceId);
    if(!source_ptr) return;
    
    bool is_positional =
        source_ptr->type == SOUND_TYPE_GAMEPLAY_POS ||
        source_ptr->type == SOUND_TYPE_AMBIANCE_POS;
    if(!is_positional) return;
    
    //Calculate camera things.
    Point cam_size = camBR - camTL;
    if(cam_size.x == 0.0f || cam_size.y == 0.0f) return;
    
    Point cam_center = (camTL + camBR) / 2.0f;
    float d = Distance(cam_center, source_ptr->pos).toFloat();
    Point delta = source_ptr->pos - cam_center;
    
    //Set the gain.
    float gain =
        interpolateNumber(
            fabs(d),
            AUDIO::PLAYBACK_RANGE_CLOSE, AUDIO::PLAYBACK_RANGE_FAR_GAIN,
            1.0f, 0.0f
        );
    gain = std::clamp(gain, 0.0f, 1.0f);
    playback_ptr->targetGain = gain;
    
    //Set the pan.
    float pan_abs =
        interpolateNumber(
            fabs(delta.x),
            AUDIO::PLAYBACK_RANGE_CLOSE, AUDIO::PLAYBACK_RANGE_FAR_PAN,
            0.0f, 1.0f
        );
    pan_abs = std::clamp(pan_abs, 0.0f, 1.0f);
    float pan = delta.x > 0.0f ? pan_abs : -pan_abs;
    playback_ptr->targetPan = pan;
}


/**
 * @brief Updates the volumes of all mixers.
 *
 * @param master_volume Volume of the master mixer.
 * @param gameplay_sound_volume Volume of the gameplay sound effects mixer.
 * @param music_volume Volume of the music mixer.
 * @param ambiance_sound_volume Volume of the ambiance sounds mixer.
 * @param ui_sound_volume Volume of the UI sound effects mixer.
 */
void AudioManager::updateVolumes(
    float master_volume, float gameplay_sound_volume, float music_volume,
    float ambiance_sound_volume, float ui_sound_volume
) {
    master_volume = std::clamp(master_volume, 0.0f, 1.0f);
    al_set_mixer_gain(masterMixer, master_volume);
    
    gameplay_sound_volume = std::clamp(gameplay_sound_volume, 0.0f, 1.0f);
    al_set_mixer_gain(gameplaySoundMixer, gameplay_sound_volume);
    
    music_volume = std::clamp(music_volume, 0.0f, 1.0f);
    al_set_mixer_gain(musicMixer, music_volume);
    
    ambiance_sound_volume = std::clamp(ambiance_sound_volume, 0.0f, 1.0f);
    al_set_mixer_gain(ambianceSoundMixer, ambiance_sound_volume);
    
    ui_sound_volume = std::clamp(ui_sound_volume, 0.0f, 1.0f);
    al_set_mixer_gain(uiSoundMixer, ui_sound_volume);
}


/**
 * @brief Loads song data from a data node.
 *
 * @param node Data node to load from.
 */
void Song::loadFromDataNode(DataNode* node) {
    //Content metadata.
    loadMetadataFromDataNode(node);
    
    //Standard data.
    ReaderSetter rs(node);
    
    string main_track_str;
    DataNode* main_track_node = nullptr;
    
    rs.set("main_track", main_track_str, &main_track_node);
    rs.set("loop_start", loopStart);
    rs.set("loop_end", loopEnd);
    rs.set("name", name);
    
    mainTrack =
        game.content.songTracks.list.get(main_track_str, main_track_node);
        
    DataNode* mix_tracks_node = node->getChildByName("mix_tracks");
    size_t n_mix_tracks = mix_tracks_node->getNrOfChildren();
    
    for(size_t m = 0; m < n_mix_tracks; m++) {
        DataNode* mix_track_node = mix_tracks_node->getChild(m);
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
        
        mixTracks[trigger] =
            game.content.songTracks.list.get(mix_track_node->value, mix_track_node);
    }
    
    if(loopEnd == 0.0f) {
        loopEnd = al_get_audio_stream_length_secs(mainTrack);
    }
    if(loopEnd < loopStart) {
        loopStart = 0.0f;
    }
}


/**
 * @brief Unloads the song.
 */
void Song::unload() {
    game.content.songTracks.list.free(mainTrack);
    for(auto &t : mixTracks) {
        game.content.songTracks.list.free(t.second);
    }
}
