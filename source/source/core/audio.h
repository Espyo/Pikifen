/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for audio-related things.
 *
 * The audio manager is the main engine behind everything audio in Pikifen.
 * Sound effects can only come from sound sources. This way, it's possible
 * for a sound source to set its position in the game world every frame for
 * the sake of panning and volume, for it to keep emitting a certain sound until
 * told to stop, etc.
 * So when something in the engine needs to play a sound effect, it asks the
 * audio manager to create a sound source. Sometime later, either by the
 * audio manager itself or by manual request from whatever saved the source,
 * the source can then be destroyed (via audio manager).
 * The existence of a centralized audio manager helps ensure we don't have the
 * same sound effect play back too many times in a jarring way, helps makes
 * sounds fade out smoothly, helps make panning and volume simpler, and more.
 * As for music, only one song can be the current song at a time, though
 * multiple songs can be technically playing at once, and each song can have
 * more than one audio file (known as tracks here). When the current song is
 * set, all other songs that are currently playing are faded out to stop.
 * Most of the structures in this system are designed to be plain old data, with
 * the manager being in charge of logic.
 */

#pragma once

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "../content/content.h"
#include "../lib/data_file/data_file.h"
#include "../util/geometry_utils.h"
#include "const.h"
#include "misc_structs.h"


using std::map;
using std::string;


class AudioManager;
class Mob;


#pragma region Constants


namespace AUDIO {
extern const float DEF_STACK_MIN_POS;
extern const float INTERLUDE_GAIN_SPEED;
extern const float MIX_TRACK_GAIN_SPEED;
extern const float PLAYBACK_GAIN_SPEED;
extern const float PLAYBACK_PAN_SPEED;
extern const float PLAYBACK_PAUSE_GAIN_SPEED;
extern const float PLAYBACK_RANGE_CLOSE;
extern const float PLAYBACK_RANGE_FAR_GAIN;
extern const float PLAYBACK_RANGE_FAR_PAN;
extern const float PLAYBACK_STOP_GAIN_SPEED;
extern const float SONG_GAIN_SPEED;
extern const float SONG_SOFTENED_VOLUME;
}


//Types of sound effects.
enum SOUND_TYPE {

    //In-world gameplay sound played globally, like a chime or name call.
    SOUND_TYPE_GAMEPLAY_GLOBAL,
    
    //In-world gameplay sound from a specific position, like a Pikmin yawning.
    SOUND_TYPE_GAMEPLAY_POS,
    
    //In-world ambient sound played globally, like birds chirping.
    SOUND_TYPE_AMBIANCE_GLOBAL,
    
    //In-world ambient sound effect from a specific position, like a waterfall.
    SOUND_TYPE_AMBIANCE_POS,
    
    //UI sound effect, that persists through pausing the gameplay.
    SOUND_TYPE_UI,
    
};


//Ways to handle sound effect playback stacking.
enum SOUND_STACK_MODE {

    //Stack like normal. Maybe with a minimum time threshold.
    SOUND_STACK_MODE_NORMAL,
    
    //Any new playback overrides any existing one, forcing them to stop.
    SOUND_STACK_MODE_OVERRIDE,
    
    //New playback is forbidden if other playbacks exist.
    SOUND_STACK_MODE_NEVER,
    
};


//Flags for sound effects.
enum SOUND_FLAG {

    //Normally, sources are destroyed when playback ends. This keeps them.
    SOUND_FLAG_KEEP_ON_PLAYBACK_END = 1 << 0,
    
    //Normally, playbacks stop when the source is destroyed. This keeps them.
    SOUND_FLAG_KEEP_PLAYBACK_ON_DESTROY = 1 << 1,
    
    //Normally, creating a sound source emits a playback. This prevents that.
    SOUND_FLAG_DONT_EMIT_ON_CREATION = 1 << 2,
    
    //Loops. You probably want one of the other "keep" flags too.
    SOUND_FLAG_LOOP = 1 << 3,
    
};


//Possible states for a playback.
enum SOUND_PLAYBACK_STATE {

    //Playing like normal.
    SOUND_PLAYBACK_STATE_PLAYING,
    
    //In the process of fading out to pause.
    SOUND_PLAYBACK_STATE_PAUSING,
    
    //Paused.
    SOUND_PLAYBACK_STATE_PAUSED,
    
    //In the process of fading in to unpause.
    SOUND_PLAYBACK_STATE_UNPAUSING,
    
    //In the process of fading out to stop.
    SOUND_PLAYBACK_STATE_STOPPING,
    
    //Finished playing and needs to be destroyed.
    SOUND_PLAYBACK_STATE_DESTROYED,
    
};


//Possible states for a song.
enum SONG_STATE {

    //In the process of fading in as it starts.
    SONG_STATE_STARTING,
    
    //Playing like normal.
    SONG_STATE_PLAYING,
    
    //In the process of lowering its volume due to a game pause.
    SONG_STATE_SOFTENING,
    
    //Volume lowered due to a game pause.
    SONG_STATE_SOFTENED,
    
    //In the process of raising its volume due to a game unpause.
    SONG_STATE_UNSOFTENING,
    
    //In the process of fading out to stop.
    SONG_STATE_STOPPING,
    
    //Not playing.
    SONG_STATE_STOPPED,
    
};


//Ways for a song track to be a part of the mix.
enum MIX_TRACK_TYPE {

    //Enemy nearby.
    MIX_TRACK_TYPE_ENEMY,
    
    //Total number.
    N_MIX_TRACK_TYPES,
    
};


#pragma endregion
#pragma region Classes


/**
 * @brief Configuration about a given sound effect source.
 */
struct SoundSourceConfig {

    //--- Members ---
    
    //Flags. Use SOUND_FLAG.
    Bitmask8 flags = 0;
    
    //How it should stack with other playbacks.
    SOUND_STACK_MODE stackMode = SOUND_STACK_MODE_NORMAL;
    
    //Minimum time of other playbacks before stacking. Avoid 0 (always stack).
    float stackMinPos = AUDIO::DEF_STACK_MIN_POS;
    
    //Volume at which it plays. [0 - 1].
    float volume = 1.0f;
    
    //Speed at which it plays. Also affects pitch.
    float speed = 1.0f;
    
    //Randomness to the volume every time it emits the sound. 0 for none.
    float volumeDeviation = 0.0f;
    
    //Randomness to the speed every time it emits the sound. 0 for none.
    float speedDeviation = 0.0f;
    
    //X% chance for the sound to actually play. [0 - 100].
    //100 for normal chance.
    unsigned char randomChance = 100;
    
    //Randomly delay the emission between 0 and this amount. 0 for none.
    float randomDelay = 0.0f;
    
    //Interval between emissions of the sound. 0 means it plays once.
    float interval = 0.0f;
    
};


/**
 * @brief Something in the game that emits sound effects.
 *
 * Typically this is tied to a mob, but it could also just be an
 * abstract source. All sound playback needs to come from a source.
 */
struct SoundSource {

    //--- Members ---
    
    //Allegro sound sample that it plays.
    ALLEGRO_SAMPLE* sample = nullptr;
    
    //Type of sound effect.
    SOUND_TYPE type = SOUND_TYPE_GAMEPLAY_GLOBAL;
    
    //Configuration.
    SoundSourceConfig config;
    
    //Position in the game world, if applicable.
    Point pos;
    
    //Time left until the next emission.
    float emitTimeLeft = 0.0f;
    
    //Does it need to be deleted?
    bool destroyed = false;
    
};


/**
 * @brief An instance of a sound effect's playback.
 * This needs to be emitted from a sound source.
 */
struct SoundPlayback {

    //--- Members ---
    
    //The source of the sound effect.
    size_t sourceId = 0;
    
    //Its Allegro sample instance.
    ALLEGRO_SAMPLE_INSTANCE* allegroSampleInstance = nullptr;
    
    //State.
    SOUND_PLAYBACK_STATE state = SOUND_PLAYBACK_STATE_PLAYING;
    
    //Current volume.
    float volume = 1.0f;
    
    //Volume that it wants to be at.
    float targetVolume = 1.0f;
    
    //Base volume, before any position or fade-based operations.
    float baseVolume = 1.0f;
    
    //Current pan.
    float pan = 0.0f;
    
    //Pan that it wants to be at.
    float targetPan = 0.0f;
    
    //Multiply the volume by this much, due to the playback's state.
    float stateVolumeMult = 1.0f;
    
    //Position before pausing.
    unsigned int prePausePos = 0;
    
};


/**
 * @brief Info about an entire piece of music, including all of
 * the tracks that go into mixing the final thing.
 *
 * Every frame, all of the mix tracks are initially set to a status of false,
 * meaning they're not meant to play. Something in the game's tick logic is
 * responsible for setting the right statuses to true. Then the audio manager
 * is responsible for fading them in and out as necessary.
 */
struct Song : public Content {

    //--- Members ---
    
    //The main track. Other tracks can be mixed on top of this if applicable.
    ALLEGRO_AUDIO_STREAM* mainTrack = nullptr;
    
    //Other tracks to mix in with the main track.
    map<MIX_TRACK_TYPE, ALLEGRO_AUDIO_STREAM*> mixTracks;
    
    //Current volume.
    float volume = 0.0f;
    
    //Point it was at when it stopped, if any.
    float stopPoint = 0.0f;
    
    //State.
    SONG_STATE state = SONG_STATE_STOPPED;
    
    //Loop region start point, in seconds.
    double loopStart = 0;
    
    //Loop region end point, in seconds. 0 = end of song.
    double loopEnd = 0;
    
    
    //--- Function declarations ---
    
    void loadFromDataNode(DataNode* node);
    void unload();
    
};


/**
 * @brief Manages everything about the game's audio.
 */
class AudioManager {

public:

    //--- Members ---
    
    //Base master mixer volume.
    float baseMasterMixerVolume = 1.0f;
    
    //Base gameplay sound mixer volume.
    float baseGameplaySoundMixerVolume = 1.0f;
    
    //Base music mixer volume.
    float baseMusicMixerVolume = 1.0f;
    
    //Base ambiance sound mixer volume.
    float baseAmbianceSoundMixerVolume = 1.0f;
    
    //Base UI sound mixer volume.
    float baseUiSoundMixerVolume = 1.0f;
    
    //Callback for when a song ends, if any.
    std::function<void(const string& name)> onSongFinished = nullptr;
    
    
    //--- Function declarations ---
    
    size_t createMobSoundSource(
        ALLEGRO_SAMPLE* sample, Mob* mPtr, bool ambiance = false,
        const SoundSourceConfig& config = SoundSourceConfig()
    );
    size_t createGlobalSoundSource(
        ALLEGRO_SAMPLE* sample, bool ambiance = false,
        const SoundSourceConfig& config = SoundSourceConfig()
    );
    size_t createPosSoundSource(
        ALLEGRO_SAMPLE* sample, const Point& pos, bool ambiance = false,
        const SoundSourceConfig& config = SoundSourceConfig()
    );
    size_t createUiSoundSource(
        ALLEGRO_SAMPLE* sample,
        const SoundSourceConfig& config = SoundSourceConfig()
    );
    bool destroySoundSource(size_t sourceId);
    void destroy();
    bool emit(size_t sourceId);
    void handleInterludeEnd(bool instant);
    void handleInterludeStart(bool instant);
    void handleMobDeletion(const Mob* mPtr);
    void handleStreamFinished(ALLEGRO_AUDIO_STREAM* stream);
    void handleWorldPause();
    void handleWorldUnpause();
    void init();
    void markMixTrackStatus(MIX_TRACK_TYPE trackType);
    bool rewindSong(const string& name);
    bool scheduleEmission(size_t sourceId, bool first);
    void setCameraPos(const Point& camTL, const Point& camBR);
    bool setCurrentSong(
        const string& name, bool fromStart = true, bool fadeIn = true,
        bool loop = true
    );
    void setSongPosNearLoop();
    bool setSoundSourcePos(size_t sourceId, const Point& pos);
    void stopAllPlaybacks(const ALLEGRO_SAMPLE* filter = nullptr);
    void tick(float deltaT);
    void updateMixerVolumes();
    
private:

    //--- Members ---
    
    //Master mixer.
    ALLEGRO_MIXER* masterMixer = nullptr;
    
    //In-world gameplay sound effect mixer.
    ALLEGRO_MIXER* gameplaySoundMixer = nullptr;
    
    //Music mixer.
    ALLEGRO_MIXER* musicMixer = nullptr;
    
    //In-world ambiance sound effect mixer.
    ALLEGRO_MIXER* ambianceSoundMixer = nullptr;
    
    //UI sound effect mixer.
    ALLEGRO_MIXER* uiSoundMixer = nullptr;
    
    //Allegro voice from which the sound effects play.
    ALLEGRO_VOICE* voice = nullptr;
    
    //Incremental ID, used for the next source to create.
    size_t nextSoundSourceId = 1;
    
    //Mob-specific sound effect sources.
    map<size_t, Mob*> mobSources;
    
    //All sound effect sources, including the mob-specific ones.
    map<size_t, SoundSource> sources;
    
    //All sound effects being played right now.
    vector<SoundPlayback> playbacks;
    
    //Status for things that affect mix tracks this frame.
    vector<bool> mixStatuses;
    
    //Current volume of each mix track type.
    vector<float> mixVolumes;
    
    //Are we currently in an interlude?
    bool inInterlude = false;
    
    //Current sound effect mixer volume multiplier,
    //based on whether we're in an interlude, [0 - 1].
    float interludeVolume = 0.0f;
    
    //Top-left camera coordinates.
    Point camTL;
    
    //Bottom-right camera coordinates.
    Point camBR;
    
    
    //--- Function declarations ---
    
    size_t createSoundSource(
        ALLEGRO_SAMPLE* sample,
        SOUND_TYPE type,
        const SoundSourceConfig& config,
        const Point& pos
    );
    bool destroySoundPlayback(size_t playbackIdx);
    SoundSource* getSource(size_t sourceId);
    void startSongTrack(
        Song* songPtr, ALLEGRO_AUDIO_STREAM* stream,
        bool fromStart, bool fadeIn, bool loop
    );
    bool stopSoundPlayback(size_t playbackIdx);
    void updatePlaybackVolumeAndPan(size_t playbackIdx);
    void updatePlaybackTargetVolAndPan(size_t playbackIdx);
    
};

    
/**
 * @brief Info on a sound that can be emitted, from a data node.
 */
struct DataNodeSound {

    //--- Members ---
    
    //The loaded sample.
    ALLEGRO_SAMPLE* sample = nullptr;
    
    //Type of sound.
    SOUND_TYPE type = SOUND_TYPE_GAMEPLAY_POS;
    
    //Configuration.
    SoundSourceConfig config;


    //--- Function declarations ---

    void loadFromDataNode(DataNode* node);
    
};


#pragma endregion
