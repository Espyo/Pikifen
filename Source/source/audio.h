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
 */

#ifndef AUDIO_INCLUDED
#define AUDIO_INCLUDED

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#include "libs/data_file.h"
#include "utils/geometry_utils.h"


using std::map;
using std::string;


class audio_manager;
class mob;


namespace AUDIO {
extern const float DEF_STACK_MIN_POS;
extern const float GAIN_CHANGE_SPEED;
extern const float PAN_CHANGE_SPEED;
extern const float PLAYBACK_PAUSE_GAIN_SPEED;
extern const float PLAYBACK_STOP_GAIN_SPEED;
}


//Types of sound effects.
enum SFX_TYPE {
    //In-world global sound effect, like a chime or name call.
    SFX_TYPE_WORLD_GLOBAL,
    //In-world sound effect from a specific position in the game world.
    SFX_TYPE_WORLD_POS,
    //In-world ambient sound effect.
    SFX_TYPE_WORLD_AMBIANCE,
    //UI sound effect, that persists through pausing the gameplay.
    SFX_TYPE_UI,
};


//Ways to handle sound effect playback stacking.
enum SFX_STACK_MODES {
    //Stack like normal. Maybe with a minimum time threshold.
    SFX_STACK_NORMAL,
    //Any new playback overrides any existing one, forcing them to stop.
    SFX_STACK_OVERRIDE,
    //New playback is forbidden if other playbacks exist.
    SFX_STACK_NEVER,
};


//Flags for sound effects.
enum SFX_FLAGS {
    //Normally, sources are destroyed when playback ends. This keeps them.
    SFX_FLAG_KEEP_ON_PLAYBACK_END = 0x01,
    //Normally, playbacks stop when the source is destroyed. This keeps them.
    SFX_FLAG_KEEP_PLAYBACK_ON_DESTROY = 0x02,
    //Normally, creating a sound source emits a playback. This prevents that.
    SFX_FLAG_DONT_EMIT_ON_CREATION = 0x04,
};


//Possible states for a playback.
enum SFX_PLAYBACK_STATES {
    //Playing like normal.
    SFX_PLAYBACK_PLAYING,
    //In the process of fading out to pause.
    SFX_PLAYBACK_PAUSING,
    //Paused.
    SFX_PLAYBACK_PAUSED,
    //In the process of fading in to unpause.
    SFX_PLAYBACK_UNPAUSING,
    //In the process of fading out to stop.
    SFX_PLAYBACK_STOPPING,
    //Finished playing and needs to be destroyed.
    SFX_PLAYBACK_DESTROYED,
};


/* ----------------------------------------------------------------------------
 * Configuration about a given sound effect source.
 */
struct sfx_source_config_struct {
    //Flags. Use SFX_FLAGS.
    uint8_t flags = 0;
    //How it should stack with other playbacks.
    SFX_STACK_MODES stack_mode = SFX_STACK_NORMAL;
    //Minimum time of other playbacks before stacking. Avoid 0 (always stack).
    float stack_min_pos = AUDIO::DEF_STACK_MIN_POS;
    //Gain at which it plays. 0 to 1.
    float gain = 1.0f;
    //Pan with which it plays. -1 (full left) to 0 (normal) to 1 (full right).
    float pan = 0.0f;
    //Speed at which it plays. Also affects pitch.
    float speed = 1.0f;
    //Randomness to the gain every time it emits the sound. 0 for none.
    float gain_deviation = 0.0f;
    //Randomness to the pitch every time it emits the sound. 0 for none.
    float pan_deviation = 0.0f;
    //Randomness to the pan every time it emits the sound. 0 for none.
    float speed_deviation = 0.0f;
    //Randomly delay the emission between 0 and this amount. 0 for none.
    float random_delay = 0.0f;
    //Interval between emissions of the sound. 0 means it plays once.
    float interval = 0.0f;
};


/* ----------------------------------------------------------------------------
 * Something in the game that emits sound effects.
 * Typically this is tied to a mob, but it could also just be an
 * abstract source. All sound playback needs to come from a source.
 */
struct sfx_source_struct {
    //Allegro sound sample that it plays.
    ALLEGRO_SAMPLE* sample = nullptr;
    //Type of sound effect.
    SFX_TYPE type = SFX_TYPE_WORLD_GLOBAL;
    //Configuration.
    sfx_source_config_struct config;
    //Position in the game world, if applicable.
    point pos;
    //Time left until the next emission.
    float emit_time_left = 0.0f;
    //Does it need to be deleted?
    bool destroyed = false;
};


/* ----------------------------------------------------------------------------
 * An instance of a sound effect's playback.
 * This needs to be emitted from a sound source.
 */
struct sfx_playback_struct {
    //The source of the sound effect.
    size_t source_id = 0;
    //Its Allegro sample instance.
    ALLEGRO_SAMPLE_INSTANCE* allegro_sample_instance = NULL;
    //State.
    SFX_PLAYBACK_STATES state = SFX_PLAYBACK_PLAYING;
    //Current gain.
    float gain = 1.0f;
    //Gain that it wants to be at.
    float target_gain = 1.0f;
    //Current pan.
    float pan = 0.0f;
    //Pan that it wants to be at.
    float target_pan = 0.0f;
    //Multiply the gain by this much, due to the playback's state.
    float state_gain_mult = 1.0f;
    //Position before pausing.
    unsigned int pre_pause_pos = 0;
};


/* ----------------------------------------------------------------------------
 * Sample manager.
 * See bitmap manager.
 */
struct sfx_sample_manager {
    public:
    explicit sfx_sample_manager(const string &base_dir);
    ALLEGRO_SAMPLE* get(
        const string &name, data_node* node = NULL,
        const bool report_errors = true
    );
    void detach(const ALLEGRO_SAMPLE* s);
    void detach(const string &name);
    void clear();
    
    long get_total_calls() const;
    size_t get_list_size() const;
    
    private:
    struct sample_info {
        //Sample pointer.
        ALLEGRO_SAMPLE* s;
        //How many calls it has.
        size_t calls;
        
        sample_info(ALLEGRO_SAMPLE* s = NULL);
    };
    //Base directory that this manager works on.
    string base_dir;
    //List of loaded samples.
    map<string, sample_info> list;
    //Total sum of calls. Useful for debugging.
    long total_calls;
    
    void detach(map<string, sample_info>::iterator it);
    
};


/* ----------------------------------------------------------------------------
 * Manages everything about the game's audio.
 */
class audio_manager {
public:
    //Manager of samples.
    sfx_sample_manager samples;
    
    size_t create_ui_sfx_source(
        ALLEGRO_SAMPLE* sample,
        const sfx_source_config_struct &config = sfx_source_config_struct()
    );
    size_t create_mob_sfx_source(
        ALLEGRO_SAMPLE* sample,
        mob* m_ptr,
        const sfx_source_config_struct &config = sfx_source_config_struct()
    );
    size_t create_world_ambiance_sfx_source(
        ALLEGRO_SAMPLE* sample,
        const sfx_source_config_struct &config = sfx_source_config_struct()
    );
    size_t create_world_global_sfx_source(
        ALLEGRO_SAMPLE* sample,
        const sfx_source_config_struct &config = sfx_source_config_struct()
    );
    size_t create_world_pos_sfx_source(
        ALLEGRO_SAMPLE* sample,
        const point &pos,
        const sfx_source_config_struct &config = sfx_source_config_struct()
    );
    bool destroy_sfx_source(size_t source_id);
    void destroy();
    bool emit(size_t source_id);
    void handle_mob_deletion(mob* m_ptr);
    void handle_world_pause();
    void handle_world_unpause();
    void init(
        float master_volume, float world_sfx_volume, float music_volume,
        float ambiance_volume, float ui_sfx_volume
    );
    bool schedule_emission(size_t source_id, bool first);
    void set_camera_pos(const point &cam_tl, const point &cam_br);
    bool set_sfx_source_pos(size_t source_id, const point &pos);
    void stop_all_playbacks(ALLEGRO_SAMPLE* filter);
    void tick(float delta_t);
    void update_volumes(
        float master_volume, float world_sfx_volume, float music_volume,
        float ambiance_volume, float ui_sfx_volume
    );
    audio_manager();
    
private:
    //Master mixer.
    ALLEGRO_MIXER* master_mixer;
    //General in-world sound effect mixer.
    ALLEGRO_MIXER* world_sfx_mixer;
    //In-world ambiance sound effect mixer.
    ALLEGRO_MIXER* world_ambiance_sfx_mixer;
    //UI sound effect mixer.
    ALLEGRO_MIXER* ui_sfx_mixer;
    //Allegro voice from which the sound effects play.
    ALLEGRO_VOICE* voice;
    //Incremental ID, used for the next source to create.
    size_t next_sfx_source_id;
    //Mob-specific sound effect sources.
    map<size_t, mob*> mob_sources;
    //All sound effect sources.
    map<size_t, sfx_source_struct> sources;
    //All sound effects being played right now.
    vector<sfx_playback_struct> playbacks;
    //Top-left camera coordinates.
    point cam_tl;
    //Bottom-right camera coordinates.
    point cam_br;
    
    size_t create_sfx_source(
        ALLEGRO_SAMPLE* sample,
        SFX_TYPE type,
        const sfx_source_config_struct &config,
        const point &pos
    );
    bool destroy_sfx_playback(size_t playback_idx);
    sfx_source_struct* get_source(size_t source_id);
    bool stop_sfx_playback(size_t playback_idx);
    void update_playback_gain_and_pan(size_t playback_idx);
    void update_playback_target_gain_and_pan(size_t playback_idx);
};


#endif //ifndef AUDIO_INCLUDED
