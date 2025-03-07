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


namespace AUDIO {
extern const float DEF_STACK_MIN_POS;
extern const float MIX_TRACK_GAIN_SPEED;
extern const float PLAYBACK_GAIN_SPEED;
extern const float PLAYBACK_PAN_SPEED;
extern const float PLAYBACK_PAUSE_GAIN_SPEED;
extern const float PLAYBACK_RANGE_CLOSE;
extern const float PLAYBACK_RANGE_FAR_GAIN;
extern const float PLAYBACK_RANGE_FAR_PAN;
extern const float PLAYBACK_STOP_GAIN_SPEED;
extern const float SONG_GAIN_SPEED;
extern const float SONG_SOFTENED_GAIN;
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


/**
 * @brief Configuration about a given sound effect source.
 */
struct SoundSourceConfig {

    //--- Members ---
    
    //Flags. Use SOUND_FLAG.
    bitmask_8_t flags = 0;
    
    //How it should stack with other playbacks.
    SOUND_STACK_MODE stack_mode = SOUND_STACK_MODE_NORMAL;
    
    //Minimum time of other playbacks before stacking. Avoid 0 (always stack).
    float stack_min_pos = AUDIO::DEF_STACK_MIN_POS;
    
    //Gain at which it plays. 0 to 1.
    float gain = 1.0f;
    
    //Speed at which it plays. Also affects pitch.
    float speed = 1.0f;
    
    //Randomness to the gain every time it emits the sound. 0 for none.
    float gain_deviation = 0.0f;
    
    //Randomness to the speed every time it emits the sound. 0 for none.
    float speed_deviation = 0.0f;
    
    //Randomly delay the emission between 0 and this amount. 0 for none.
    float random_delay = 0.0f;
    
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
    float emit_time_left = 0.0f;
    
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
    size_t source_id = 0;
    
    //Its Allegro sample instance.
    ALLEGRO_SAMPLE_INSTANCE* allegro_sample_instance = nullptr;
    
    //State.
    SOUND_PLAYBACK_STATE state = SOUND_PLAYBACK_STATE_PLAYING;
    
    //Current gain.
    float gain = 1.0f;
    
    //Gain that it wants to be at.
    float target_gain = 1.0f;
    
    //Base gain, before any position or fade-based operations.
    float base_gain = 1.0f;
    
    //Current pan.
    float pan = 0.0f;
    
    //Pan that it wants to be at.
    float target_pan = 0.0f;
    
    //Multiply the gain by this much, due to the playback's state.
    float state_gain_mult = 1.0f;
    
    //Position before pausing.
    unsigned int pre_pause_pos = 0;
    
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
    ALLEGRO_AUDIO_STREAM* main_track = nullptr;
    
    //Other tracks to mix in with the main track.
    map<MIX_TRACK_TYPE, ALLEGRO_AUDIO_STREAM*> mix_tracks;
    
    //Current gain.
    float gain = 0.0f;
    
    //Point it was at when it stopped, if any.
    float stop_point = 0.0f;
    
    //State.
    SONG_STATE state = SONG_STATE_STOPPED;
    
    //Loop region start point, in seconds.
    double loop_start = 0;
    
    //Loop region end point, in seconds. 0 = end of song.
    double loop_end = 0;
    
    
    //--- Function declarations ---
    
    void load_from_data_node(DataNode* node);
    void unload();
    
};


/**
 * @brief Manages everything about the game's audio.
 */
class AudioManager {

public:

    //--- Members ---
    
    //Callback for when a song ends, if any.
    std::function<void(const string &name)> on_song_finished = nullptr;
    
    
    //--- Function declarations ---
    
    size_t create_mob_sound_source(
        ALLEGRO_SAMPLE* sample, Mob* m_ptr, bool ambiance = false,
        const SoundSourceConfig &config = SoundSourceConfig()
    );
    size_t create_global_sound_source(
        ALLEGRO_SAMPLE* sample, bool ambiance = false,
        const SoundSourceConfig &config = SoundSourceConfig()
    );
    size_t create_pos_sound_source(
        ALLEGRO_SAMPLE* sample, const Point &pos, bool ambiance = false,
        const SoundSourceConfig &config = SoundSourceConfig()
    );
    size_t create_ui_sound_source(
        ALLEGRO_SAMPLE* sample,
        const SoundSourceConfig &config = SoundSourceConfig()
    );
    bool destroy_sound_source(size_t source_id);
    void destroy();
    bool emit(size_t source_id);
    void handle_mob_deletion(const Mob* m_ptr);
    void handle_stream_finished(ALLEGRO_AUDIO_STREAM* stream);
    void handle_world_pause();
    void handle_world_unpause();
    void init(
        float master_volume, float gameplay_sound_volume, float music_volume,
        float ambiance_sound_volume, float ui_sound_volume
    );
    void mark_mix_track_status(MIX_TRACK_TYPE track_type);
    bool rewind_song(const string &name);
    bool schedule_emission(size_t source_id, bool first);
    void set_camera_pos(const Point &cam_tl, const Point &cam_br);
    bool set_current_song(
        const string &name, bool from_start = true, bool fade_in = true,
        bool loop = true
    );
    void set_song_pos_near_loop();
    bool set_sound_source_pos(size_t source_id, const Point &pos);
    void stop_all_playbacks(const ALLEGRO_SAMPLE* filter = nullptr);
    void tick(float delta_t);
    void update_volumes(
        float master_volume, float gameplay_sound_volume, float music_volume,
        float ambiance_sound_volume, float ui_sound_volume
    );
    
private:

    //--- Members ---
    
    //Master mixer.
    ALLEGRO_MIXER* master_mixer = nullptr;
    
    //In-world gameplay sound effect mixer.
    ALLEGRO_MIXER* gameplay_sound_mixer = nullptr;
    
    //Music mixer.
    ALLEGRO_MIXER* music_mixer = nullptr;
    
    //In-world ambiance sound effect mixer.
    ALLEGRO_MIXER* ambiance_sound_mixer = nullptr;
    
    //UI sound effect mixer.
    ALLEGRO_MIXER* ui_sound_mixer = nullptr;
    
    //Allegro voice from which the sound effects play.
    ALLEGRO_VOICE* voice = nullptr;
    
    //Incremental ID, used for the next source to create.
    size_t next_sound_source_id = 1;
    
    //Mob-specific sound effect sources.
    map<size_t, Mob*> mob_sources;
    
    //All sound effect sources.
    map<size_t, SoundSource> sources;
    
    //All sound effects being played right now.
    vector<SoundPlayback> playbacks;
    
    //Status for things that affect mix tracks this frame.
    vector<bool> mix_statuses;
    
    //Current volume of each mix track type.
    vector<float> mix_volumes;
    
    //Top-left camera coordinates.
    Point cam_tl;
    
    //Bottom-right camera coordinates.
    Point cam_br;
    
    
    //--- Function declarations ---
    
    size_t create_sound_source(
        ALLEGRO_SAMPLE* sample,
        SOUND_TYPE type,
        const SoundSourceConfig &config,
        const Point &pos
    );
    bool destroy_sound_playback(size_t playback_idx);
    SoundSource* get_source(size_t source_id);
    void start_song_track(
        Song* song_ptr, ALLEGRO_AUDIO_STREAM* stream,
        bool from_start, bool fade_in, bool loop
    );
    bool stop_sound_playback(size_t playback_idx);
    void update_playback_gain_and_pan(size_t playback_idx);
    void update_playback_target_gain_and_pan(size_t playback_idx);
    
};
