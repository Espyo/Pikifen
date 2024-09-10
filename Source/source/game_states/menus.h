/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the menus.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>

#include "game_state.h"
#include "../gui.h"
#include "../options.h"
#include "../misc_structs.h"


using std::map;
using std::size_t;
using std::vector;


namespace AREA_MENU {
extern const string GUI_FILE_PATH;
extern const string INFO_GUI_FILE_PATH;
extern const float PAGE_SWAP_DURATION;
extern const string SONG_NAME;
extern const string SPECS_GUI_FILE_PATH;
}


namespace CONTROL_BINDS_MENU {
extern const float BIND_BUTTON_HEIGHT;
extern const float BIND_BUTTON_PADDING;
extern const float CAPTURE_TIMEOUT_DURATION;
extern const string GUI_FILE_PATH;
extern const string SONG_NAME;
}


namespace MAIN_MENU {
extern const string GUI_FILE_PATH;
extern const float HUD_MOVE_TIME;
extern const string MAKE_GUI_FILE_PATH;
extern const string PLAY_GUI_FILE_PATH;
extern const string SONG_NAME;
extern const string TUTORIAL_GUI_FILE_PATH;
}


namespace OPTIONS_MENU {
extern const string AUDIO_GUI_FILE_PATH;
extern const string CONTROLS_GUI_FILE_PATH;
extern const string GRAPHICS_GUI_FILE_PATH;
extern const float HUD_MOVE_TIME;
extern const string MISC_GUI_FILE_PATH;
extern const string SONG_NAME;
extern const string TOP_GUI_FILE_PATH;
}


namespace RESULTS {
extern const string GUI_FILE_PATH;
extern const string SONG_NAME;
}


namespace STATS_MENU {
extern const string GUI_FILE_PATH;
extern const string SONG_NAME;
}


//Pages of the main menu.
enum MAIN_MENU_PAGE {

    //Main page.
    MAIN_MENU_PAGE_MAIN,
    
    //Play page.
    MAIN_MENU_PAGE_PLAY,
    
    //Make page.
    MAIN_MENU_PAGE_MAKE,
    
};


//Pages of the options menu.
enum OPTIONS_MENU_PAGE {

    //Top-level page.
    OPTIONS_MENU_PAGE_TOP,
    
    //Controls page.
    OPTIONS_MENU_PAGE_CONTROLS,
    
};


/**
 * @brief Info about the main menu.
 */
class main_menu_state : public game_state {

public:

    //--- Members ---
    
    //What page to load when it is created.
    MAIN_MENU_PAGE page_to_load = MAIN_MENU_PAGE_MAIN;
    
    
    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:

    //--- Misc. declarations ---
    
    /**
     * @brief Represents a Pikmin in the logo.
     */
    struct logo_pik {
    
        //--- Members ---
        
        //Position.
        point pos;
        
        //Current angle.
        float angle = 0.0f;
        
        //Forward movement speed.
        float speed = 0.0f;
        
        //Its destination.
        point destination;
        
        //Speed at which it sways.
        float sway_speed = 0.0f;
        
        //Variable that controls its swaying.
        float sway_var = 0.0f;
        
        //Image that represents this Pikmin's top.
        ALLEGRO_BITMAP* top = nullptr;
        
        //Has it reached its destination?
        bool reached_destination = false;
        
    };
    
    
    //--- Members ---
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    //List of Pikmin that make up the logo.
    vector<logo_pik> logo_pikmin;
    
    //GUI for the main page.
    gui_manager main_gui;
    
    //GUI for the play page.
    gui_manager play_gui;
    
    //GUI for the make page.
    gui_manager make_gui;
    
    //GUI for the tutorial question page.
    gui_manager tutorial_gui;
    
    //Top-left coordinates of the logo, in screen percentage.
    point logo_min_screen_limit = point(10.0f, 10.0f);
    
    //Bottom-right coordinates of the logo, in screen percentage.
    point logo_max_screen_limit = point(90.0f, 50.0f);
    
    //Maximum speed a logo Pikmin can move at.
    float logo_pikmin_max_speed = 800.0f;
    
    //Minimum speed a logo Pikmin can move at.
    float logo_pikmin_min_speed = 600.0f;
    
    //How much to smooth a logo Pikmin's speed by.
    float logo_pikmin_speed_smoothness = 0.08f;
    
    //How much to sway a logo Pikmin by.
    float logo_pikmin_sway_amount = 3.0f;
    
    //Maximum speed at which a logo Pikmin can sway.
    float logo_pikmin_sway_max_speed = 5.5f;
    
    //Minimum speed at which a logo Pikmin can sway.
    float logo_pikmin_sway_min_speed = 2.5f;
    
    //Width and height of a logo Pikmin.
    point logo_pikmin_size = point(3.5f, 3.5f);
    
    //Map of what characters represent what Pikmin top bitmaps.
    map<unsigned char, ALLEGRO_BITMAP*> logo_type_bitmaps;
    
    
    //--- Function declarations ---
    
    void init_gui_main_page();
    void init_gui_make_page();
    void init_gui_play_page();
    void init_gui_tutorial_page();
    
};


/**
 * @brief Info on how a picker GUI item in the options menu should work.
 *
 * @tparam t The type of value the picker controls.
 */
template<typename t>
class options_menu_picker_gui_item : public picker_gui_item {

public:

    //--- Members ---
    
    //Points to the current value.
    t* cur_value = nullptr;
    
    //Default value.
    const t def_value = t();
    
    //Tooltip, sans default. Used if the presets don't have their own tooltips.
    string tooltip;
    
    //Value of each preset.
    vector<t> preset_values;
    
    //Name of each preset.
    vector<string> preset_names;
    
    //Tooltip for each preset. If empty, "tooltip" is used instead.
    vector<string> preset_descriptions;
    
    //Code to run after a value is changed, if any.
    std::function<void()> after_change = nullptr;
    
    //Converts a value to a string. Used in the tooltip's default, if necessary.
    std::function<string(t)> value_to_string = nullptr;
    
    
    //--- Function definitions ---
    
    /**
     * @brief Constructs a new options menu picker GUI item object.
     *
     * @param base_text Base text.
     * @param cur_value Current value.
     * @param def_value Default value.
     * @param preset_values Value of each preset.
     * @param preset_names Name of each preset.
     * @param tooltip Base tooltip.
     */
    options_menu_picker_gui_item(
        const string &base_text, t* cur_value, const t &def_value,
        const vector<t> &preset_values, const vector<string> &preset_names,
        const string &tooltip = ""
    ) :
        picker_gui_item(base_text, ""),
        cur_value(cur_value),
        def_value(def_value),
        tooltip(tooltip),
        preset_values(preset_values),
        preset_names(preset_names) {
    }
    
    
    /**
     * @brief Initializes the picker. This needs to be called after setting
     * all of its properties, since it relies on them existing and having their
     * final values. Without this function, the picker won't behave as
     * expected.
     */
    void init() {
        cur_option_idx = INVALID;
        for(size_t p = 0; p < preset_values.size(); ++p) {
            if(*cur_value == preset_values[p]) {
                cur_option_idx = p;
                break;
            }
        }
        option = get_cur_option_name();
        nr_options = preset_values.size();
        
        on_previous = [this] () { change_option(-1); };
        on_next = [this] () { change_option(1); };
        on_get_tooltip = [this] () {
            size_t def_idx = 0;
            string full_tooltip;
            for(; def_idx < this->preset_values.size(); ++def_idx) {
                if(this->preset_values[def_idx] == this->def_value) {
                    break;
                }
            }
            if(preset_descriptions.empty()) {
                full_tooltip = this->tooltip;
            } else {
                if(cur_option_idx == INVALID) {
                    full_tooltip = "Using a custom value.";
                } else {
                    full_tooltip = preset_descriptions[cur_option_idx];
                }
            }
            full_tooltip += " Default: " + this->preset_names[def_idx] + ".";
            return full_tooltip;
        };
    }
    
    
    /**
     * @brief Returns the name of the current option.
     *
     * @return The name.
     */
    string get_cur_option_name() {
        if(cur_option_idx == INVALID) {
            if(value_to_string) {
                return value_to_string(*cur_value) + " (custom)";
            } else {
                return "Custom";
            }
        } else {
            return preset_names[cur_option_idx];
        }
    }
    
    
    /**
     * @brief Changes to the next or to the previous option.
     *
     * @param step What direction to change to. +1 is next, -1 is previous.
     */
    void change_option(int step) {
        if(cur_option_idx == INVALID) {
            cur_option_idx = 0;
        } else {
            cur_option_idx =
                sum_and_wrap(
                    (int) cur_option_idx, step, (int) preset_values.size()
                );
        }
        
        *cur_value = preset_values[cur_option_idx];
        option = get_cur_option_name();
        start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        if(after_change) after_change();
    }
    
};


/**
 * @brief Info about the options menu.
 */
class options_menu_state : public game_state {

public:

    //--- Members ---
    
    //What page to load when it is created.
    OPTIONS_MENU_PAGE page_to_load = OPTIONS_MENU_PAGE_TOP;
    
    
    //--- Function declarations ---
    
    options_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:

    //--- Members ---
    
    //Known good resolution presets.
    vector<std::pair<int, int> > resolution_presets;
    
    //Currently selected resolution.
    std::pair<int, int> cur_resolution_option;
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    //GUI for the top-level page.
    gui_manager top_gui;
    
    //GUI for the controls options page.
    gui_manager controls_gui;
    
    //GUI for the graphics options page.
    gui_manager graphics_gui;
    
    //GUI for the audio options page.
    gui_manager audio_gui;
    
    //GUI for the misc. options page.
    gui_manager misc_gui;
    
    //Auto-throw picker widget.
    options_menu_picker_gui_item<AUTO_THROW_MODE>* auto_throw_picker = nullptr;
    
    //Resolution picker widget.
    options_menu_picker_gui_item<std::pair<int, int> >* resolution_picker = nullptr;
    
    //Cursor speed picker widget.
    options_menu_picker_gui_item<float>* cursor_speed_picker = nullptr;
    
    //Cursor camera weight picker widget.
    options_menu_picker_gui_item<float>* cursor_cam_weight_picker = nullptr;
    
    //Leaving confirmation picker widget.
    options_menu_picker_gui_item<LEAVING_CONFIRMATION_MODE>*
    leaving_confirmation_picker = nullptr;
    
    //Master volume picker widget.
    options_menu_picker_gui_item<float>* master_vol_picker = nullptr;
    
    //World sound effects volume picker widget.
    options_menu_picker_gui_item<float>* world_sfx_vol_picker = nullptr;
    
    //Music volume picker widget.
    options_menu_picker_gui_item<float>* music_vol_picker = nullptr;
    
    //Ambiance sound volume picker widget.
    options_menu_picker_gui_item<float>* ambiance_vol_picker = nullptr;
    
    //UI sound effects volume picker widget.
    options_menu_picker_gui_item<float>* ui_sfx_vol_picker = nullptr;
    
    //Restart warning text widget.
    text_gui_item* warning_text = nullptr;
    
    
    //--- Function declarations ---
    
    void go_to_control_binds();
    void init_gui_audio_page();
    void init_gui_controls_page();
    void init_gui_graphics_page();
    void init_gui_misc_page();
    void init_gui_top_page();
    void leave();
    void trigger_restart_warning();
    
};


/**
 * @brief Info about the controls menu.
 */
class control_binds_menu_state : public game_state {

public:

    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:

    //--- Members ---
    
    //Bitmap of the menu's background.
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    //GUI.
    gui_manager gui;
    
    //GUI for the "more..." options of an action type.
    gui_manager more_gui;
    
    //Control list GUI item.
    list_gui_item* list_box = nullptr;
    
    //Is it currently capturing input? 0: No. 1: Capturing. 2: Finishing.
    unsigned char capturing_input = 0;
    
    //Time left before the input capturing times out.
    float capturing_input_timeout = 0.0f;
    
    //Is it showing an action type's "more..." menu?
    bool showing_more = false;
    
    //List of binds per player action type.
    vector<vector<control_bind> > binds_per_action_type;
    
    //Current player action type.
    PLAYER_ACTION_TYPE cur_action_type = PLAYER_ACTION_TYPE_NONE;
    
    //Current global bind index we're working with.
    size_t cur_bind_idx = 0;
    
    
    //--- Function declarations ---
    
    void choose_input(
        const PLAYER_ACTION_TYPE action_type, const size_t bind_idx
    );
    void delete_bind(
        const PLAYER_ACTION_TYPE action_type, const size_t bind_idx
    );
    void populate_binds();
    void restore_defaults(const PLAYER_ACTION_TYPE action_type);
    void leave();
    
};


/**
 * @brief Info about the area selection menu.
 */
class area_menu_state : public game_state {

public:

    //--- Members ---
    
    //Type of area that the menu is dealing with.
    AREA_TYPE area_type = AREA_TYPE_SIMPLE;
    
    
    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:

    //--- Members ---
    
    //Bitmap of the menu background,
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    //Folder name of each area available.
    vector<string> areas_to_pick;
    
    //Button for each area available.
    vector<gui_item*> area_buttons;
    
    //Display name of each area available.
    vector<string> area_names;
    
    //Subtitle of each area available.
    vector<string> area_subtitles;
    
    //Description of each area available.
    vector<string> area_descriptions;
    
    //Difficulty of each area available.
    vector<unsigned char> area_difficulties;
    
    //Tags of each area available.
    vector<string> area_tags;
    
    //Maker of each area available.
    vector<string> area_makers;
    
    //Version of each area available.
    vector<string> area_versions;
    
    //Thumbnail of each area available.
    vector<ALLEGRO_BITMAP*> area_thumbs;
    
    //Mission data of each area available.
    vector<mission_data> area_mission_data;
    
    //Records of each area available.
    vector<mission_record> area_records;
    
    //Main GUI.
    gui_manager gui;
    
    //Area info GUI item.
    gui_item* info_box = nullptr;
    
    //Mission specs GUI item.
    gui_item* specs_box = nullptr;
    
    //Currently selected area, or INVALID for none.
    size_t cur_area_idx = INVALID;
    
    //Area list box item.
    list_gui_item* list_box = nullptr;
    
    //Button of the first area available, if any.
    button_gui_item* first_area_button = nullptr;
    
    //Name text item, in the info page.
    text_gui_item* info_name_text = nullptr;
    
    //Name text item, in the specs page.
    text_gui_item* specs_name_text = nullptr;
    
    //Subtitle text item.
    text_gui_item* subtitle_text = nullptr;
    
    //Thumbnail of the currently selected area.
    ALLEGRO_BITMAP* cur_thumb = nullptr;
    
    //Description text item.
    text_gui_item* description_text = nullptr;
    
    //Difficulty text item.
    text_gui_item* difficulty_text = nullptr;
    
    //Tags text item.
    text_gui_item* tags_text = nullptr;
    
    //Maker text item.
    text_gui_item* maker_text = nullptr;
    
    //Version text item.
    text_gui_item* version_text = nullptr;
    
    //Record info text item.
    text_gui_item* record_info_text = nullptr;
    
    //Record stamp of the currently selected area.
    ALLEGRO_BITMAP* cur_stamp = nullptr;
    
    //Record medal of the currently selected area.
    ALLEGRO_BITMAP* cur_medal = nullptr;
    
    //Record date text item.
    text_gui_item* record_date_text = nullptr;
    
    //Goal text item.
    text_gui_item* goal_text = nullptr;
    
    //Fail explanation list item.
    list_gui_item* fail_list = nullptr;
    
    //Grading explanation list item.
    list_gui_item* grading_list = nullptr;
    
    //Show the mission specs?
    bool show_mission_specs = false;
    
    
    //--- Function declarations ---
    
    void add_bullet(list_gui_item* list, const string &text);
    void animate_info_and_specs();
    void change_info(const size_t area_idx);
    void init_gui_main();
    void init_gui_info_page();
    void init_gui_specs_page();
    void leave();
    
};


/**
 * @brief Info about the statistics menu.
 */
class stats_menu_state : public game_state {

public:

    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:

    //--- Members ---
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    //Statistics list item.
    list_gui_item* stats_list = nullptr;
    
    //Runtime stat text item.
    text_gui_item* runtime_value_text = nullptr;
    
    //GUI.
    gui_manager gui;
    
    
    //--- Function declarations ---
    
    void add_header(const string &label);
    text_gui_item* add_stat(
        const string &label, const string &value, const string &description
    );
    void leave();
    void populate_stats_list();
    void update_runtime_value_text();
    
};
