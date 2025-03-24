/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the maker tool structures and functions.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>

#include "../content/mob/mob.h"
#include "../lib/data_file/data_file.h"
#include "controls.h"


using std::vector;


//List of maker tools.
enum MAKER_TOOL_TYPE {

    //None.
    MAKER_TOOL_TYPE_NONE,
    
    //Create an image of the whole area.
    MAKER_TOOL_TYPE_AREA_IMAGE,
    
    //Change gameplay speed.
    MAKER_TOOL_TYPE_CHANGE_SPEED,
    
    //Show collision box.
    MAKER_TOOL_TYPE_COLLISION,
    
    //Geometry info beneath mouse cursor.
    MAKER_TOOL_TYPE_GEOMETRY_INFO,
    
    //Show hitboxes.
    MAKER_TOOL_TYPE_HITBOXES,
    
    //Toggle HUD visibility.
    MAKER_TOOL_TYPE_HUD,
    
    //Hurt mob beneath mouse cursor.
    MAKER_TOOL_TYPE_HURT_MOB,
    
    //Get info on the mob beneath mouse cursor.
    MAKER_TOOL_TYPE_MOB_INFO,
    
    //Create a new Pikmin beneath mouse cursor.
    MAKER_TOOL_TYPE_NEW_PIKMIN,
    
    //Show path info.
    MAKER_TOOL_TYPE_PATH_INFO,
    
    //Set song position near loop.
    MAKER_TOOL_TYPE_SET_SONG_POS_NEAR_LOOP,
    
    //Teleport to mouse cursor.
    MAKER_TOOL_TYPE_TELEPORT,
    
    //Total amount of maker tools.
    N_MAKER_TOOLS,
    
};


namespace MAKER_TOOLS {
extern const string NAMES[N_MAKER_TOOLS];
}


/**
 * @brief Info about all of the maker tools.
 */
struct MakerTools {

    //--- Misc. definitions ---

    struct AreaImageSettings {

        //Padding around the area in the area image tool.
        float padding = 32.0f;
        
        //Show tree shadows in the area image tool?
        bool shadows = true;
        
        //Maximum width or height of the area image.
        int size = 2048;
        
        //Show mobs in the area image?
        bool mobs = true;

    };


    //--- Members ---
    
    //Are the tools enabled?
    bool enabled = true;

    //Different area image settings.
    AreaImageSettings area_image_settings[3];
    
    //Automatically pick this from the list of the selected auto-entry mode.
    string auto_start_option;
    
    //Automatically enter this game mode when the game boots.
    string auto_start_state;
    
    //Are we currently changing the game speed?
    bool change_speed = false;
    
    //Which game speed change setting to use.
    unsigned char change_speed_setting_idx = 0;

    //Different game speed change settings. These are multipliers to change by.
    float change_speed_settings[3] = { 2.0f, 0.5f, 1.0f };

    //Are collision boxes visible in-game?
    bool collision = false;
    
    //Is the geometry information tool enabled?
    bool geometry_info = false;
    
    //Are hitboxes visible in-game?
    bool hitboxes = false;
    
    //Is the HUD visible?
    bool hud = true;
    
    //Mob currently locked-on to for the mob information tool. nullptr if off.
    Mob* info_lock = nullptr;
    
    //If any maker info is being printed, this is how long it lasts on-screen.
    float info_print_duration = 5.0f;
    
    //If any maker info is being printed, this is how long its fade lasts.
    float info_print_fade_duration = 3.0f;
    
    //If any maker info is being printed, this is its text.
    string info_print_text;
    
    //If any maker info is being printed, this represents its time to live.
    Timer info_print_timer;

    //For each key (F2 - F11, 0 - 9), what tool is bound to it?
    MAKER_TOOL_TYPE keys[20];
    
    //When we last spawned a Pikmin, what was its type?
    PikminType* last_pikmin_type = nullptr;
    
    //Different mob hurting settings. When used, dock this much of its max HP.
    float mob_hurting_settings[3] = { 0.75f, 1.0f, -1.0f };
    
    //Show path info?
    bool path_info = false;
    
    //Use the performance monitor?
    bool use_perf_mon = false;
    
    //Has the player made use of any tools that could help them play?
    bool used_helping_tools = false;
    
    
    //--- Function declarations ---
    
    MakerTools();
    void handle_player_action(const PlayerAction& action);
    void load_from_data_node(DataNode* node);
    void reset_for_gameplay();
    void save_to_data_node(DataNode* node);


private:

    //--- Function declarations ---

    unsigned char get_maker_tool_setting_idx() const;
    
};
