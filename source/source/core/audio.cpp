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
//thus preventing a super-loud sound.
const float DEF_STACK_MIN_POS = 0.1f;

//Change speed for interlude volume changes, measured in amount per second.
const float INTERLUDE_GAIN_SPEED = 2.0f;

//Change speed for a mix track's volume, measured in amount per second.
const float MIX_TRACK_GAIN_SPEED = 1.0f;

//Change speed for a playback's volume, measured in amount per second.
const float PLAYBACK_GAIN_SPEED = 3.0f;

//Change speed for a playback's pan, measured in amount per second.
const float PLAYBACK_PAN_SPEED = 8.0f;

//Change speed of playback volume when un/pausing,
//measured in amount per second.
const float PLAYBACK_PAUSE_GAIN_SPEED = 5.0f;

//Distance to an audio source where it'll be considered close, i.e. it will
//play at full volume and no pan.
const float PLAYBACK_RANGE_CLOSE = 100.0f;

//Distance after which an audio source's volume will be 0.
const float PLAYBACK_RANGE_FAR_GAIN = 450.0f;

//Horizontal distance after which an audio source's pan will be
//fully left/right.
const float PLAYBACK_RANGE_FAR_PAN = 300.0f;

//Change speed of playback volume when stopping, measured in amount per second.
const float PLAYBACK_STOP_GAIN_SPEED = 8.0f;

//Change speed for a song's volume, measured in amount per second.
const float SONG_GAIN_SPEED = 1.0f;

//Volume for when a song is softened, due to a game pause.
const float SONG_SOFTENED_VOLUME = 0.4f;

}


#pragma region Audio manager


/**
 * @brief Creates an in-world global sound effect source,
 * adds it to the list, and returns its ID.
 *
 * This is basically how you can get the engine to produce a sound that doesn't
 * involve a position in the game world.
 *
 * @param sample Sound sample that this source will emit.
 * @param ambiance Whether it's an ambiance sound or a gameplay sound.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t AudioManager::addNewGlobalSoundSource(
    ALLEGRO_SAMPLE* sample, bool ambiance,
    const SoundSourceConfig& config
) {
    return
        addNewSoundSource(
            sample,
            ambiance ? SOUND_TYPE_AMBIANCE_GLOBAL : SOUND_TYPE_GAMEPLAY_GLOBAL,
            config, Point()
        );
}


/**
 * @brief Creates an in-world mob sound effect source,
 * adds it to the list, and returns its ID.
 *
 * This is like addNewPosSoundSource, but ties the source to the mob,
 * meaning the audio manager is responsible for updating the source's position
 * every frame to match the mob's.
 *
 * @param sample Sound sample that this source will emit.
 * @param mPtr Pointer to the mob.
 * @param ambiance Whether it's an ambiance sound or a gameplay sound.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t AudioManager::addNewMobSoundSource(
    ALLEGRO_SAMPLE* sample,
    Mob* mPtr, bool ambiance,
    const SoundSourceConfig& config
) {
    size_t sourceId =
        addNewSoundSource(
            sample,
            ambiance ? SOUND_TYPE_AMBIANCE_POS : SOUND_TYPE_GAMEPLAY_POS,
            config, mPtr->pos
        );
    mobSources[sourceId] = mPtr;
    return sourceId;
}


/**
 * @brief Creates an in-world positional sound effect source,
 * adds it to the list, and returns its ID.
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
size_t AudioManager::addNewPosSoundSource(
    ALLEGRO_SAMPLE* sample, const Point& pos, bool ambiance,
    const SoundSourceConfig& config
) {
    return
        addNewSoundSource(
            sample,
            ambiance ? SOUND_TYPE_AMBIANCE_POS : SOUND_TYPE_GAMEPLAY_POS,
            config, pos
        );
}


/**
 * @brief Creates a sound effect source,
 * adds it to the list, and returns its ID.
 *
 * @param sample Sound sample that this source will emit.
 * @param type Sound type.
 * @param config Configuration.
 * @param pos Position in the game world, if applicable.
 * @return The ID, or 0 on failure.
 */
size_t AudioManager::addNewSoundSource(
    ALLEGRO_SAMPLE* sample,
    SOUND_TYPE type,
    const SoundSourceConfig& config,
    const Point& pos
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
            if(emit(id)) {
                scheduleEmission(id, false);
            } else {
                destroySoundSource(id);
            }
        }
    }
    
    nextSoundSourceId++; //Hopefully there will be no collisions.
    
    return id;
}


/**
 * @brief Creates a global UI sound effect source,
 * adds it to the list, and returns its ID.
 *
 * This is basically how you can get the engine to produce a UI sound.
 *
 * @param sample Sound sample that this source will emit.
 * @param config Configuration.
 * @return The ID, or 0 on failure.
 */
size_t AudioManager::addNewUiSoundSource(
    ALLEGRO_SAMPLE* sample,
    const SoundSourceConfig& config
) {
    return addNewSoundSource(sample, SOUND_TYPE_UI, config, Point());
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
 * @param playbackIdx Index of the playback in the list of playbacks.
 * @return Whether it succeeded.
 */
bool AudioManager::destroySoundPlayback(size_t playbackIdx) {
    SoundPlayback* playbackPtr = &playbacks[playbackIdx];
    if(playbackPtr->state == SOUND_PLAYBACK_STATE_DESTROYED) return false;
    playbackPtr->state = SOUND_PLAYBACK_STATE_DESTROYED;
    
    SoundSource* sourcePtr = getSource(playbackPtr->sourceId);
    
    //Destroy the source, if applicable.
    if(sourcePtr) {
        if(!hasFlag(sourcePtr->config.flags, SOUND_FLAG_KEEP_ON_PLAYBACK_END)) {
            destroySoundSource(playbackPtr->sourceId);
        }
    }
    
    //Destroy the Allegro sample instance.
    ALLEGRO_SAMPLE_INSTANCE* instance =
        playbackPtr->allegroSampleInstance;
    if(instance) {
        al_set_sample_instance_playing(instance, false);
        al_detach_sample_instance(instance);
        if(instance) al_destroy_sample_instance(instance);
        playbackPtr->allegroSampleInstance = nullptr;
    }
    
    return true;
}


/**
 * @brief Destroys a sound source.
 *
 * @param sourceId ID of the sound source to destroy.
 * @return Whether it succeeded.
 */
bool AudioManager::destroySoundSource(size_t sourceId) {
    SoundSource* sourcePtr = getSource(sourceId);
    if(!sourcePtr) return false;
    
    if(sourcePtr->destroyed) return false;
    sourcePtr->destroyed = true;
    
    //Check if we must stop playbacks.
    if(
        !hasFlag(
            sourcePtr->config.flags,
            SOUND_FLAG_KEEP_PLAYBACK_ON_DESTROY
        )
    ) {
        for(size_t p = 0; p < playbacks.size(); p++) {
            if(playbacks[p].sourceId == sourceId) {
                stopSoundPlayback(p);
            }
        }
    }
    
    return true;
}


/**
 * @brief Emits a sound from a sound source now, if possible.
 *
 * @param sourceId ID of the source to emit sound from.
 * @return Whether it succeeded.
 */
bool AudioManager::emit(size_t sourceId) {
    //Setup.
    SoundSource* sourcePtr = getSource(sourceId);
    if(!sourcePtr) return false;
    
    ALLEGRO_SAMPLE* sample = sourcePtr->sample;
    if(!sample) return false;
    
    //Check if other playbacks exist to prevent stacking.
    float lowestStackingPlaybackPos = FLT_MAX;
    if(
        sourcePtr->config.stackMinPos > 0.0f ||
        sourcePtr->config.stackMode == SOUND_STACK_MODE_NEVER
    ) {
        for(size_t p = 0; p < playbacks.size(); p++) {
            SoundPlayback* playback = &playbacks[p];
            SoundSource* pSourcePtr = getSource(playback->sourceId);
            if(!pSourcePtr || pSourcePtr->sample != sample) continue;
            
            float playbackPos =
                al_get_sample_instance_position(
                    playback->allegroSampleInstance
                ) /
                (float) al_get_sample_frequency(pSourcePtr->sample);
            lowestStackingPlaybackPos =
                std::min(lowestStackingPlaybackPos, playbackPos);
        }
        
        if(
            sourcePtr->config.stackMinPos > 0.0f &&
            lowestStackingPlaybackPos < sourcePtr->config.stackMinPos
        ) {
            //Can't emit. This would stack the sounds, and there are other
            //playbacks that haven't reached the minimum stack threshold yet.
            return false;
        }
        if(
            sourcePtr->config.stackMode == SOUND_STACK_MODE_NEVER &&
            lowestStackingPlaybackPos < FLT_MAX
        ) {
            //Can't emit. This would stack the sounds.
            return false;
        }
    }
    
    //Check if there's a random chance for this to not play.
    if(sourcePtr->config.randomChance < 100) {
        unsigned char roll = game.rng.i(1, 100);
        if(roll > sourcePtr->config.randomChance) {
            //Can't emit. Random chance failed.
            return false;
        }
    }
    
    //Check if other playbacks exist and if we need to stop them.
    if(sourcePtr->config.stackMode == SOUND_STACK_MODE_OVERRIDE) {
        for(size_t p = 0; p < playbacks.size(); p++) {
            SoundPlayback* playback = &playbacks[p];
            SoundSource* pSourcePtr = getSource(playback->sourceId);
            if(!pSourcePtr || pSourcePtr->sample != sample) {
                continue;
            }
            stopSoundPlayback(p);
        }
    }
    
    //Create the playback.
    playbacks.push_back(SoundPlayback());
    SoundPlayback* playbackPtr = &playbacks.back();
    playbackPtr->sourceId = sourceId;
    playbackPtr->allegroSampleInstance = al_create_sample_instance(sample);
    if(!playbackPtr->allegroSampleInstance) return false;
    
    playbackPtr->baseVolume = sourcePtr->config.volume;
    if(sourcePtr->config.volumeDeviation != 0.0f) {
        playbackPtr->baseVolume +=
            game.rng.f(
                -sourcePtr->config.volumeDeviation,
                sourcePtr->config.volumeDeviation
            );
        playbackPtr->baseVolume =
            std::clamp(playbackPtr->baseVolume, 0.0f, 1.0f);
    }
    
    //Play.
    updatePlaybackTargetVolAndPan(playbacks.size() - 1);
    playbackPtr->volume = playbackPtr->targetVolume;
    playbackPtr->pan = playbackPtr->targetPan;
    
    ALLEGRO_MIXER* mixer = nullptr;
    switch(sourcePtr->type) {
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
        playbackPtr->allegroSampleInstance, mixer
    );
    
    al_set_sample_instance_playmode(
        playbackPtr->allegroSampleInstance,
        hasFlag(sourcePtr->config.flags, SOUND_FLAG_LOOP) ?
        ALLEGRO_PLAYMODE_LOOP :
        ALLEGRO_PLAYMODE_ONCE
    );
    float speed = sourcePtr->config.speed;
    if(sourcePtr->config.speedDeviation != 0.0f) {
        speed +=
            game.rng.f(
                -sourcePtr->config.speedDeviation,
                sourcePtr->config.speedDeviation
            );
    }
    speed = std::max(0.0f, speed);
    al_set_sample_instance_speed(playbackPtr->allegroSampleInstance, speed);
    updatePlaybackVolumeAndPan(playbacks.size() - 1);
    
    al_set_sample_instance_position(
        playbackPtr->allegroSampleInstance,
        0.0f
    );
    al_set_sample_instance_playing(
        playbackPtr->allegroSampleInstance,
        true
    );
    
    return true;
}


/**
 * @brief Returns a source's pointer from a source in the list.
 *
 * @param sourceId ID of the sound source.
 * @return The source, or nullptr if invalid.
 */
SoundSource* AudioManager::getSource(size_t sourceId) {
    auto sourceIt = sources.find(sourceId);
    if(sourceIt == sources.end()) return nullptr;
    return &sourceIt->second;
}


/**
 * @brief Handles an interlude ending, so the sound effect mixers can
 * stop lowering their volume.
 *
 * @param instant Whether the change in volume happens instantly.
 */
void AudioManager::handleInterludeEnd(bool instant) {
    inInterlude = false;
    if(instant) {
        interludeVolume = 1.0f;
        updateMixerVolumes();
    }
}


/**
 * @brief Handles an interlude starting, so the sound effect mixers can
 * lower their volume.
 *
 * @param instant Whether the change in volume happens instantly.
 */
void AudioManager::handleInterludeStart(bool instant) {
    inInterlude = true;
    if(instant) {
        interludeVolume = 0.0f;
        updateMixerVolumes();
    }
}


/**
 * @brief Handles a mob being deleted.
 *
 * @param mPtr Mob that got deleted.
 */
void AudioManager::handleMobDeletion(const Mob* mPtr) {
    for(auto s = mobSources.begin(); s != mobSources.end();) {
        if(s->second == mPtr) {
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
    for(const auto& s : game.content.songs.list) {
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
        SoundPlayback* playbackPtr = &playbacks[p];
        if(playbackPtr->state == SOUND_PLAYBACK_STATE_DESTROYED) {
            continue;
        }
        
        SoundSource* sourcePtr = getSource(playbackPtr->sourceId);
        if(!sourcePtr) continue;
        
        if(
            sourcePtr->type == SOUND_TYPE_GAMEPLAY_GLOBAL ||
            sourcePtr->type == SOUND_TYPE_GAMEPLAY_POS ||
            sourcePtr->type == SOUND_TYPE_AMBIANCE_GLOBAL ||
            sourcePtr->type == SOUND_TYPE_AMBIANCE_POS
        ) {
            playbackPtr->state = SOUND_PLAYBACK_STATE_PAUSING;
        }
    }
    
    //Soften songs.
    for(auto& s : game.content.songs.list) {
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
        SoundPlayback* playbackPtr = &playbacks[p];
        if(playbackPtr->state == SOUND_PLAYBACK_STATE_DESTROYED) {
            continue;
        }
        
        SoundSource* sourcePtr = getSource(playbackPtr->sourceId);
        if(!sourcePtr) continue;
        
        if(
            sourcePtr->type == SOUND_TYPE_GAMEPLAY_GLOBAL ||
            sourcePtr->type == SOUND_TYPE_GAMEPLAY_POS ||
            sourcePtr->type == SOUND_TYPE_AMBIANCE_GLOBAL ||
            sourcePtr->type == SOUND_TYPE_AMBIANCE_POS
        ) {
            playbackPtr->state = SOUND_PLAYBACK_STATE_UNPAUSING;
            al_set_sample_instance_playing(
                playbackPtr->allegroSampleInstance,
                true
            );
            al_set_sample_instance_position(
                playbackPtr->allegroSampleInstance,
                playbackPtr->prePausePos
            );
        }
    }
    
    //Unsoften songs.
    for(auto& s : game.content.songs.list) {
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
 */
void AudioManager::init() {
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
    updateMixerVolumes();
    
    //Initialization of every mix track type.
    for(size_t m = 0; m < N_MIX_TRACK_TYPES; m++) {
        mixStatuses.push_back(false);
        mixVolumes.push_back(0.0f);
    }
}


/**
 * @brief Marks a mix track type's status to true for this frame.
 *
 * @param trackType Track type to mark.
 */
void AudioManager::markMixTrackStatus(MIX_TRACK_TYPE trackType) {
    mixStatuses[trackType] = true;
}


/**
 * @brief Sets a song's position to the beginning.
 *
 * @param name Name of the song in the list of loaded songs.
 * @return Whether it succeeded.
 */
bool AudioManager::rewindSong(const string& name) {
    auto songIt = game.content.songs.list.find(name);
    if(songIt == game.content.songs.list.end()) return false;
    Song* songPtr = &songIt->second;
    
    songPtr->stopPoint = 0.0f;
    al_rewind_audio_stream(songPtr->mainTrack);
    for(auto const& m : songPtr->mixTracks) {
        al_rewind_audio_stream(m.second);
    }
    
    return true;
}


/**
 * @brief Schedules a sound effect source's emission. This includes things
 * like randomly delaying it if configured to do so.
 *
 * @param sourceId ID of the sound source.
 * @param first True if this is the first emission of the source.
 * @return Whether it succeeded.
 */
bool AudioManager::scheduleEmission(size_t sourceId, bool first) {
    SoundSource* sourcePtr = getSource(sourceId);
    if(!sourcePtr) return false;
    
    sourcePtr->emitTimeLeft = first ? 0.0f : sourcePtr->config.interval;
    if(first || sourcePtr->config.interval > 0.0f) {
        sourcePtr->emitTimeLeft +=
            game.rng.f(0, sourcePtr->config.randomDelay);
    }
    
    return true;
}


/**
 * @brief Sets the camera's position.
 *
 * @param camTL Current coordinates of the camera's top-left corner.
 * @param camBR Current coordinates of the camera's bottom-right corner.
 */
void AudioManager::setCameraPos(const Point& camTL, const Point& camBR) {
    this->camTL = camTL;
    this->camBR = camBR;
}


/**
 * @brief Sets what the current song should be.
 *
 * If it's different from the song that's currently playing,
 * then that one fades out as this one fades in.
 * To stop playing songs, send an empty string as the song name argument.
 *
 * @param name Name of the song in the list of loaded songs.
 * @param fromStart If true, the song starts from the beginning,
 * otherwise it starts from where it left off.
 * This argument only applies if the song was stopped.
 * @param fadeIn If true, the new song fades in like normal.
 * @param loop Whether it loops.
 * @return Whether it succeeded.
 */
bool AudioManager::setCurrentSong(
    const string& name, bool fromStart, bool fadeIn, bool loop
) {

    //Stop all other songs first.
    for(auto& s : game.content.songs.list) {
        Song* songPtr = &s.second;
        if(songPtr->name == name) {
            //This is the song we want to play. Let's not handle it here.
            continue;
        }
        switch(songPtr->state) {
        case SONG_STATE_STARTING:
        case SONG_STATE_PLAYING:
        case SONG_STATE_SOFTENING:
        case SONG_STATE_SOFTENED:
        case SONG_STATE_UNSOFTENING: {
            songPtr->state = SONG_STATE_STOPPING;
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
    
    auto songIt = game.content.songs.list.find(name);
    if(songIt == game.content.songs.list.end()) return false;
    Song* songPtr = &songIt->second;
    
    //Play it.
    switch(songPtr->state) {
    case SONG_STATE_STARTING:
    case SONG_STATE_PLAYING:
    case SONG_STATE_SOFTENING:
    case SONG_STATE_SOFTENED:
    case SONG_STATE_UNSOFTENING: {
        //Already playing.
        break;
    } case SONG_STATE_STOPPING: {
        //We need it to go back, not stop.
        songPtr->state = SONG_STATE_STARTING;
        break;
    } case SONG_STATE_STOPPED: {
        //Start it.
        if(songPtr->state == SONG_STATE_STOPPED) {
            startSongTrack(
                songPtr, songPtr->mainTrack, fromStart, fadeIn, loop
            );
            for(auto const& m : songPtr->mixTracks) {
                startSongTrack(songPtr, m.second, fromStart, fadeIn, loop);
            }
        }
        songPtr->volume = fadeIn ? 0.0f : 1.0f;
        songPtr->state = fadeIn ? SONG_STATE_STARTING : SONG_STATE_PLAYING;
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
        for(auto const& m : s.second.mixTracks) {
            al_seek_audio_stream_secs(m.second, pos);
        }
    }
}


/**
 * @brief Sets the position of a positional sound effect source.
 *
 * @param sourceId ID of the sound effect source.
 * @param pos New position.
 * @return Whether it succeeded.
 */
bool AudioManager::setSoundSourcePos(size_t sourceId, const Point& pos) {
    SoundSource* sourcePtr = getSource(sourceId);
    if(!sourcePtr) return false;
    
    sourcePtr->pos = pos;
    
    return true;
}


/**
 * @brief Starts playing a song's track from scratch.
 *
 * @param songPtr The song.
 * @param stream Audio stream of the track.
 * @param fromStart If true, the song starts from the beginning,
 * otherwise it starts from where it left off.
 * @param fadeIn If true, the song starts fading in like normal.
 * @param loop Whether it loops.
 */
void AudioManager::startSongTrack(
    Song* songPtr, ALLEGRO_AUDIO_STREAM* stream,
    bool fromStart, bool fadeIn, bool loop
) {
    if(!stream) return;
    al_set_audio_stream_gain(stream, fadeIn ? 0.0f : 1.0f);
    al_seek_audio_stream_secs(stream, fromStart ? 0.0f : songPtr->stopPoint);
    al_set_audio_stream_loop_secs(
        stream, songPtr->loopStart, songPtr->loopEnd
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
        bool toStop = false;
        
        if(!filter) {
            toStop = true;
        } else {
            SoundPlayback* playbackPtr = &playbacks[p];
            SoundSource* sourcePtr =
                getSource(playbackPtr->sourceId);
            if(sourcePtr && sourcePtr->sample == filter) {
                toStop = true;
            }
        }
        
        if(toStop) {
            stopSoundPlayback(p);
        }
    }
}


/**
 * @brief Stops a playback, putting it in the "stopping" state.
 *
 * @param playbackIdx Index of the playback in the list of playbacks.
 * @return Whether it succeeded.
 */
bool AudioManager::stopSoundPlayback(size_t playbackIdx) {
    SoundPlayback* playbackPtr = &playbacks[playbackIdx];
    if(playbackPtr->state == SOUND_PLAYBACK_STATE_STOPPING) return false;
    if(playbackPtr->state == SOUND_PLAYBACK_STATE_DESTROYED) return false;
    playbackPtr->state = SOUND_PLAYBACK_STATE_STOPPING;
    return true;
}


/**
 * @brief Ticks the audio manager by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void AudioManager::tick(float deltaT) {
    //Clear deleted mob sources.
    for(auto s = mobSources.begin(); s != mobSources.end();) {
        Mob* mobPtr = s->second;
        if(!mobPtr || mobPtr->toDelete) {
            s = mobSources.erase(s);
        } else {
            ++s;
        }
    }
    
    //Update the position of sources tied to mobs.
    for(auto& s : sources) {
        if(s.second.destroyed) continue;
        auto mobSourceIt = mobSources.find(s.first);
        if(mobSourceIt == mobSources.end()) continue;
        Mob* mobPtr = mobSourceIt->second;
        if(!mobPtr || mobPtr->toDelete) continue;
        s.second.pos = mobPtr->pos;
    }
    
    //Emit playbacks from sources that want to emit.
    for(auto& s : sources) {
        if(s.second.destroyed) continue;
        if(s.second.emitTimeLeft == 0.0f) continue;
        
        s.second.emitTimeLeft -= deltaT;
        if(s.second.emitTimeLeft <= 0.0f) {
            if(emit(s.first)) {
                scheduleEmission(s.first, false);
            } else {
                destroySoundSource(s.first);
            }
        }
    }
    
    //Update playbacks.
    for(size_t p = 0; p < playbacks.size(); p++) {
        SoundPlayback* playbackPtr = &playbacks[p];
        if(playbackPtr->state == SOUND_PLAYBACK_STATE_DESTROYED) continue;
        
        if(
            !al_get_sample_instance_playing(
                playbackPtr->allegroSampleInstance
            ) &&
            playbackPtr->state != SOUND_PLAYBACK_STATE_PAUSED
        ) {
            //Finished playing entirely.
            destroySoundPlayback(p);
            
        } else {
            //Update target volume and pan, based on in-world position,
            //if applicable.
            updatePlaybackTargetVolAndPan(p);
            
            //Inch the volume and pan to the target values.
            playbackPtr->volume =
                inchTowards(
                    playbackPtr->volume,
                    playbackPtr->targetVolume,
                    AUDIO::PLAYBACK_GAIN_SPEED * deltaT
                );
            playbackPtr->pan =
                inchTowards(
                    playbackPtr->pan,
                    playbackPtr->targetPan,
                    AUDIO::PLAYBACK_PAN_SPEED * deltaT
                );
                
            //Pausing and unpausing.
            if(playbackPtr->state == SOUND_PLAYBACK_STATE_PAUSING) {
                playbackPtr->stateVolumeMult -=
                    AUDIO::PLAYBACK_PAUSE_GAIN_SPEED * deltaT;
                if(playbackPtr->stateVolumeMult <= 0.0f) {
                    playbackPtr->stateVolumeMult = 0.0f;
                    playbackPtr->state = SOUND_PLAYBACK_STATE_PAUSED;
                    playbackPtr->prePausePos =
                        al_get_sample_instance_position(
                            playbackPtr->allegroSampleInstance
                        );
                    al_set_sample_instance_playing(
                        playbackPtr->allegroSampleInstance,
                        false
                    );
                }
            } else if(playbackPtr->state == SOUND_PLAYBACK_STATE_UNPAUSING) {
                playbackPtr->stateVolumeMult +=
                    AUDIO::PLAYBACK_PAUSE_GAIN_SPEED * deltaT;
                if(playbackPtr->stateVolumeMult >= 1.0f) {
                    playbackPtr->stateVolumeMult = 1.0f;
                    playbackPtr->state = SOUND_PLAYBACK_STATE_PLAYING;
                }
            }
            
            //Stopping.
            if(playbackPtr->state == SOUND_PLAYBACK_STATE_STOPPING) {
                playbackPtr->stateVolumeMult -=
                    AUDIO::PLAYBACK_STOP_GAIN_SPEED * deltaT;
                if(playbackPtr->stateVolumeMult <= 0.0f) {
                    destroySoundPlayback(p);
                }
            }
            
            //Update the final volume and pan values.
            updatePlaybackVolumeAndPan(p);
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
            auto mobSourceIt = mobSources.find(s->first);
            if(mobSourceIt != mobSources.end()) {
                mobSources.erase(mobSourceIt);
            }
            s = sources.erase(s);
        } else {
            ++s;
        }
    }
    
    //Update the volume of songs depending on their state.
    for(auto& s : game.content.songs.list) {
        Song* songPtr = &s.second;
        
        switch(songPtr->state) {
        case SONG_STATE_STARTING: {
            songPtr->volume =
                inchTowards(
                    songPtr->volume,
                    1.0f,
                    AUDIO::SONG_GAIN_SPEED * deltaT
                );
            al_set_audio_stream_gain(songPtr->mainTrack, songPtr->volume);
            if(songPtr->volume == 1.0f) {
                songPtr->state = SONG_STATE_PLAYING;
            }
            break;
        } case SONG_STATE_PLAYING: {
            //Nothing to do.
            break;
        } case SONG_STATE_SOFTENING: {
            songPtr->volume =
                inchTowards(
                    songPtr->volume,
                    AUDIO::SONG_SOFTENED_VOLUME,
                    AUDIO::SONG_GAIN_SPEED * deltaT
                );
            al_set_audio_stream_gain(songPtr->mainTrack, songPtr->volume);
            if(songPtr->volume == AUDIO::SONG_SOFTENED_VOLUME) {
                songPtr->state = SONG_STATE_SOFTENED;
            }
            break;
        } case SONG_STATE_SOFTENED: {
            //Nothing to do.
            break;
        } case SONG_STATE_UNSOFTENING: {
            songPtr->volume =
                inchTowards(
                    songPtr->volume,
                    1.0f,
                    AUDIO::SONG_GAIN_SPEED * deltaT
                );
            al_set_audio_stream_gain(songPtr->mainTrack, songPtr->volume);
            if(songPtr->volume == 1.0f) {
                songPtr->state = SONG_STATE_PLAYING;
            }
            break;
        } case SONG_STATE_STOPPING: {
            songPtr->volume =
                inchTowards(
                    songPtr->volume,
                    0.0f,
                    AUDIO::SONG_GAIN_SPEED * deltaT
                );
            al_set_audio_stream_gain(songPtr->mainTrack, songPtr->volume);
            if(songPtr->volume == 0.0f) {
                al_set_audio_stream_playing(songPtr->mainTrack, false);
                al_detach_audio_stream(songPtr->mainTrack);
                for(auto& m : songPtr->mixTracks) {
                    al_set_audio_stream_playing(m.second, false);
                    al_detach_audio_stream(m.second);
                }
                songPtr->stopPoint =
                    al_get_audio_stream_position_secs(songPtr->mainTrack);
                songPtr->state = SONG_STATE_STOPPED;
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
                AUDIO::MIX_TRACK_GAIN_SPEED * deltaT
            );
            
        for(auto& s : game.content.songs.list) {
            Song* songPtr = &s.second;
            if(songPtr->state == SONG_STATE_STOPPED) {
                continue;
            }
            
            auto trackIt = songPtr->mixTracks.find((MIX_TRACK_TYPE) m);
            if(trackIt == songPtr->mixTracks.end()) continue;
            
            al_set_audio_stream_gain(
                trackIt->second, mixVolumes[m] * songPtr->volume
            );
        }
        
    }
    
    //Prepare the statuses for the next frame.
    for(size_t s = 0; s < N_MIX_TRACK_TYPES; s++) {
        mixStatuses[s] = false;
    }
    
    //Update the volume of the sound effect mixers, depending on the interlude
    //status.
    if(inInterlude && interludeVolume > 0.0f) {
        interludeVolume =
            inchTowards(
                interludeVolume, 0.0f, -AUDIO::INTERLUDE_GAIN_SPEED * deltaT
            );
        updateMixerVolumes();
    } else if(!inInterlude && interludeVolume < 1.0f) {
        interludeVolume =
            inchTowards(
                interludeVolume, 1.0f, AUDIO::INTERLUDE_GAIN_SPEED * deltaT
            );
        updateMixerVolumes();
    }
}


/**
 * @brief Updates the volumes of all mixers, based on the values of the
 * various variables in charge.
 */
void AudioManager::updateMixerVolumes() {
    baseMasterMixerVolume =
        std::clamp(baseMasterMixerVolume, 0.0f, 1.0f);
    baseGameplaySoundMixerVolume =
        std::clamp(baseGameplaySoundMixerVolume, 0.0f, 1.0f);
    baseMusicMixerVolume =
        std::clamp(baseMusicMixerVolume, 0.0f, 1.0f);
    baseAmbianceSoundMixerVolume =
        std::clamp(baseAmbianceSoundMixerVolume, 0.0f, 1.0f);
    baseUiSoundMixerVolume =
        std::clamp(baseUiSoundMixerVolume, 0.0f, 1.0f);
    interludeVolume = std::clamp(interludeVolume, 0.0f, 1.0f);
    
    al_set_mixer_gain(
        masterMixer, baseMasterMixerVolume
    );
    al_set_mixer_gain(
        gameplaySoundMixer, baseGameplaySoundMixerVolume * interludeVolume
    );
    al_set_mixer_gain(
        musicMixer, baseMusicMixerVolume
    );
    al_set_mixer_gain(
        ambianceSoundMixer, baseAmbianceSoundMixerVolume * interludeVolume
    );
    al_set_mixer_gain(
        uiSoundMixer, baseUiSoundMixerVolume
    );
}


/**
 * @brief Updates a playback's target volume and target pan, based on distance
 * from the camera.
 *
 * This won't update the volume and pan yet, but each audio
 * manager tick will be responsible for bringing the volume and pan to these
 * values smoothly over time.
 *
 * @param playbackIdx Index of the playback in the list.
 */
void AudioManager::updatePlaybackTargetVolAndPan(size_t playbackIdx) {
    if(playbackIdx >= playbacks.size()) return;
    SoundPlayback* playbackPtr = &playbacks[playbackIdx];
    if(playbackPtr->state == SOUND_PLAYBACK_STATE_DESTROYED) return;
    
    SoundSource* sourcePtr = getSource(playbackPtr->sourceId);
    if(!sourcePtr) return;
    
    bool isPositional =
        sourcePtr->type == SOUND_TYPE_GAMEPLAY_POS ||
        sourcePtr->type == SOUND_TYPE_AMBIANCE_POS;
    if(!isPositional) return;
    
    //Calculate camera things.
    Point camSize = camBR - camTL;
    if(camSize.x == 0.0f || camSize.y == 0.0f) return;
    
    Point camCenter = (camTL + camBR) / 2.0f;
    float d = Distance(camCenter, sourcePtr->pos).toFloat();
    Point delta = sourcePtr->pos - camCenter;
    
    //Set the volume.
    float volume =
        interpolateNumber(
            fabs(d),
            AUDIO::PLAYBACK_RANGE_CLOSE, AUDIO::PLAYBACK_RANGE_FAR_GAIN,
            1.0f, 0.0f
        );
    volume = std::clamp(volume, 0.0f, 1.0f);
    playbackPtr->targetVolume = volume;
    
    //Set the pan.
    float panAbs =
        interpolateNumber(
            fabs(delta.x),
            AUDIO::PLAYBACK_RANGE_CLOSE, AUDIO::PLAYBACK_RANGE_FAR_PAN,
            0.0f, 1.0f
        );
    panAbs = std::clamp(panAbs, 0.0f, 1.0f);
    float pan = delta.x > 0.0f ? panAbs : -panAbs;
    playbackPtr->targetPan = pan;
}


/**
 * @brief Instantly updates a playback's current volume and pan, using its
 * member variables. This also clamps the variables if needed.
 *
 * @param playbackIdx Index of the playback in the list.
 */
void AudioManager::updatePlaybackVolumeAndPan(size_t playbackIdx) {
    if(playbackIdx >= playbacks.size()) return;
    SoundPlayback* playbackPtr = &playbacks[playbackIdx];
    if(playbackPtr->state == SOUND_PLAYBACK_STATE_DESTROYED) return;
    
    playbackPtr->volume = std::clamp(playbackPtr->volume, 0.0f, 1.0f);
    float finalVolume = playbackPtr->volume * playbackPtr->stateVolumeMult;
    finalVolume *= playbackPtr->baseVolume;
    finalVolume = std::clamp(finalVolume, 0.0f, 1.0f);
    al_set_sample_instance_gain(
        playbackPtr->allegroSampleInstance,
        finalVolume
    );
    
    playbackPtr->pan = std::clamp(playbackPtr->pan, -1.0f, 1.0f);
    
    al_set_sample_instance_pan(
        playbackPtr->allegroSampleInstance,
        playbackPtr->pan
    );
}


#pragma endregion
#pragma region Others


/**
 * @brief Loads data from a data node.
 * 
 * @param node The node.
 */
void DataNodeSound::loadFromDataNode(DataNode* node) {
    ReaderSetter sRS(node);
    string soundINameStr;
    DataNode* soundINameNode = nullptr;
    string typeStr;
    DataNode* typeNode = nullptr;
    string stackModeStr;
    DataNode* stackModeNode = nullptr;
    float volumeFloat = 100.0f;
    float speedFloat = 100.0f;
    bool loopBool = false;
    
    sRS.set("sound", soundINameStr, &soundINameNode);
    sRS.set("type", typeStr, &typeNode);
    sRS.set("stack_mode", stackModeStr, &stackModeNode);
    sRS.set("stack_min_pos", config.stackMinPos);
    sRS.set("loop", loopBool);
    sRS.set("volume", volumeFloat);
    sRS.set("speed", speedFloat);
    sRS.set("volume_deviation", config.volumeDeviation);
    sRS.set("speed_deviation", config.speedDeviation);
    sRS.set("random_chance", config.randomChance);
    sRS.set("random_delay", config.randomDelay);
    
    sample =
        game.content.sounds.list.get(soundINameStr, soundINameNode);
        
    if(typeNode) {
        readEnumProp(
            soundTypeINames, typeStr, &type,
            "sound effect type", typeNode
        );
    }
    
    if(stackModeNode) {
        readEnumProp(
            soundStackModeINames, stackModeStr, &config.stackMode,
            "sound effect stack mode", stackModeNode
        );
    }
    
    if(loopBool) {
        enableFlag(config.flags, SOUND_FLAG_LOOP);
    }
    
    config.volume = volumeFloat / 100.0f;
    config.volume = std::clamp(config.volume, 0.0f, 1.0f);
    
    config.speed = speedFloat / 100.0f;
    config.speed = std::max(0.0f, config.speed);
    
    config.volumeDeviation /= 100.0f;
    config.speedDeviation /= 100.0f;
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
    ReaderSetter sRS(node);
    
    string mainTrackStr;
    DataNode* mainTrackNode = nullptr;
    
    sRS.set("main_track", mainTrackStr, &mainTrackNode);
    sRS.set("loop_start", loopStart);
    sRS.set("loop_end", loopEnd);
    sRS.set("name", name);
    
    mainTrack =
        game.content.songTracks.list.get(mainTrackStr, mainTrackNode);
        
    DataNode* mixTracksNode = node->getChildByName("mix_tracks");
    size_t nMixTracks = mixTracksNode->getNrOfChildren();
    
    for(size_t m = 0; m < nMixTracks; m++) {
        DataNode* mixTrackNode = mixTracksNode->getChild(m);
        MIX_TRACK_TYPE trigger = N_MIX_TRACK_TYPES;
        
        if(mixTrackNode->name == "enemy") {
            trigger = MIX_TRACK_TYPE_ENEMY;
        } else {
            game.errors.report(
                "Unknown mix track trigger \"" +
                mixTrackNode->name +
                "\"!", mixTrackNode
            );
            continue;
        }
        
        mixTracks[trigger] =
            game.content.songTracks.list.get(mixTrackNode->value, mixTrackNode);
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
    for(auto& t : mixTracks) {
        game.content.songTracks.list.free(t.second);
    }
}


#pragma endregion
