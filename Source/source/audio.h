/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for audio-related things.
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


//Types of sounds.
enum SOUND_TYPE {
    //Global sound, like a chime or UI sound.
    SOUND_TYPE_GLOBAL,
    //Sound emitted from a specific position in the game world.
    SOUND_TYPE_POSITIONAL,
};


//Flags for sounds.
enum SOUND_FLAGS {
    //Should the sound source be destroyed when sound playback ends?
    SOUND_FLAG_DESTROY_ON_PLAYBACK_END = 0x01,
    //Should all playbacks stop when the source is destroyed?
    SOUND_FLAG_STOP_PLAYBACK_ON_DESTROY = 0x02,
    //Normally, creating a sound source emits on creation. This prevents that.
    SOUND_FLAG_DONT_EMIT_ON_CREATION = 0x04,
    //If true, only emit a sound if no other playback (of the sound) exists.
    SOUND_FLAG_NEVER_STACK = 0x08,
    //Should all playbacks be destroyed if a new playback needs to start?
    SOUND_FLAG_STOP_OTHER_PLAYBACKS = 0x10,
};


/* ----------------------------------------------------------------------------
 * Something in the game that emits sound effects.
 * Typically this is a mob, but it could also just be an abstract source.
 * All sound playback needs to come from a source.
 */
struct sound_source_struct {
    //Sample that it plays.
    ALLEGRO_SAMPLE* sample;
    //Type of sound.
    SOUND_TYPE type;
    //Flags.
    uint8_t flags;
    //Minimum time spent on other playbacks before stacking. 0 = always stack.
    float stack_min_pos;
    //Gain at which it plays.
    float gain;
    //Pan with which it plays.
    float pan;
    //Speed at which it plays. Also affects pitch.
    float speed;
    //Randomness to the gain every time it emits the sound. 0 for none.
    float gain_deviation;
    //Randomness to the pitch every time it emits the sound. 0 for none.
    float pan_deviation;
    //Randomness to the pan every time it emits the sound. 0 for none.
    float speed_deviation;
    //Position in the game world, if applicable.
    point pos;
    //Interval between emissions of the sound. 0 means it plays once.
    float interval;
    //Time left until the next emission.
    float emit_time_left;
    
    sound_source_struct();
    
    private:
    
};


/* ----------------------------------------------------------------------------
 * An instance of a sound sample's playback.
 * This needs to be emitted from a sound source.
 */
struct sound_playback_struct {
    //The source of the sound.
    size_t source_id;
    //Its Allegro sample instance.
    ALLEGRO_SAMPLE_INSTANCE* instance;
    
    sound_playback_struct();
};


/* ----------------------------------------------------------------------------
 * Sample manager.
 * See bitmap manager.
 */
struct sound_sample_manager {
    public:
    explicit sound_sample_manager(const string &base_dir);
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
    //Global audio mixer.
    ALLEGRO_MIXER* mixer;
    //Allegro voice from which the sound effects play.
    ALLEGRO_VOICE* voice;
    //Manager of samples.
    sound_sample_manager samples;
    
    size_t create_sound_source(
        ALLEGRO_SAMPLE* sample,
        SOUND_TYPE type = SOUND_TYPE_GLOBAL,
        uint8_t flags = 0,
        float stack_min_pos = 0.0f,
        float gain = 1.0f
    );
    bool destroy_sound_source(size_t source_id);
    void destroy();
    bool emit(size_t source_id);
    void init();
    void stop_all_playbacks(ALLEGRO_SAMPLE* filter);
    void tick(float delta_t, const point &cam_tl, const point &cam_br);
    audio_manager();
    
private:
    size_t next_sound_source_id;
    //All sound sources.
    map<size_t, sound_source_struct> sources;
    //All sounds being played right now.
    vector<sound_playback_struct> playbacks;
    //Top-left camera coordinates.
    point cam_tl;
    //Bottom-right camera coordinates.
    point cam_br;
    
    bool destroy_playback(size_t playback_idx);
    sound_source_struct* get_source(size_t source_id);
};


#endif //ifndef AUDIO_INCLUDED
