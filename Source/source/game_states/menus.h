/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the menus.
 */

#ifndef MENUS_INCLUDED
#define MENUS_INCLUDED

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
extern const string SPECS_GUI_FILE_PATH;
}


namespace CONTROLS_MENU {
extern const string GUI_FILE_PATH;
}


namespace MAIN_MENU {
extern const string GUI_FILE_PATH;
extern const float HUD_MOVE_TIME;
extern const string MAKE_GUI_FILE_PATH;
extern const string PLAY_GUI_FILE_PATH;
extern const string TUTORIAL_GUI_FILE_PATH;
}


namespace OPTIONS_MENU {
extern const string GUI_FILE_PATH;
}


namespace RESULTS {
extern const string GUI_FILE_PATH;
}


namespace STATS_MENU {
extern const string GUI_FILE_PATH;
}


//Pages of the main menu.
enum MAIN_MENU_PAGES {
    //Main page.
    MAIN_MENU_PAGE_MAIN,
    //Play page.
    MAIN_MENU_PAGE_PLAY,
    //Make page.
    MAIN_MENU_PAGE_MAKE,
};


/* ----------------------------------------------------------------------------
 * Information about the main menu.
 */
class main_menu_state : public game_state {
public:
    main_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
    MAIN_MENU_PAGES page_to_load;
    
private:
    struct logo_pik {
        //Position.
        point pos;
        //Current angle.
        float angle;
        //Forward movement speed.
        float speed;
        //Its destination.
        point destination;
        //Speed at which it sways.
        float sway_speed;
        //Variable that controls its swaying.
        float sway_var;
        //Image that represents this Pikmin's top.
        ALLEGRO_BITMAP* top;
        //Has it reached its destination?
        bool reached_destination;
    };
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg;
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
    point logo_min_screen_limit;
    //Bottom-right coordinates of the logo, in screen percentage.
    point logo_max_screen_limit;
    //Maximum speed a logo Pikmin can move at.
    float logo_pikmin_max_speed;
    //Minimum speed a logo Pikmin can move at.
    float logo_pikmin_min_speed;
    //How much to smooth a logo Pikmin's speed by.
    float logo_pikmin_speed_smoothness;
    //How much to sway a logo Pikmin by.
    float logo_pikmin_sway_amount;
    //Maximum speed at which a logo Pikmin can sway.
    float logo_pikmin_sway_max_speed;
    //Minimum speed at which a logo Pikmin can sway.
    float logo_pikmin_sway_min_speed;
    //Width and height of a logo Pikmin.
    point logo_pikmin_size;
    //Map of what characters represent what Pikmin top bitmaps.
    map<unsigned char, ALLEGRO_BITMAP*> logo_type_bitmaps;
    
    void init_main_page();
    void init_make_page();
    void init_play_page();
    void init_tutorial_page();
};


/* ----------------------------------------------------------------------------
 * Holds information on how a picker GUI item in the options menu should work.
 */
template<typename t>
class options_menu_picker_gui_item : public picker_gui_item {
public:
    //Points to the current value.
    t* cur_value;
    //Default value.
    const t def_value;
    //Tooltip, sans default. Used if the presets don't have their own tooltips.
    string tooltip;
    //Value of each preset.
    vector<t> preset_values;
    //Name of each preset.
    vector<string> preset_names;
    //Tooltip for each preset. If empty, "tooltip" is used instead.
    vector<string> preset_descriptions;
    //Code to run after a value is changed, if any.
    std::function<void()> after_change;
    //Converts a value to a string. Used in the tooltip's default, if necessary.
    std::function<string(t)> value_to_string;
    
    
    /* -------------------------------------------------------------------------
    * Creates an options menu picker GUI item instance.
    */
    options_menu_picker_gui_item(
        const string &base_text, t* cur_value, const t &def_value,
        const vector<t> preset_values, const vector<string> preset_names,
        const string &tooltip = ""
    ) :
        picker_gui_item(base_text, ""),
        cur_value(cur_value),
        def_value(def_value),
        tooltip(tooltip),
        preset_values(preset_values),
        preset_names(preset_names),
        after_change(nullptr),
        value_to_string(nullptr) {
    }
    
    
    /* -------------------------------------------------------------------------
    * Initializes the picker. This needs to be called after setting all of
    * its properties, since it relies on them existing and having their
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
    
    
    /* -------------------------------------------------------------------------
    * Returns the name of the current option.
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
    
    
    /* -------------------------------------------------------------------------
    * Changes to the next or to the previous option.
    * step:
    *   What direction to change to. +1 is next, -1 is previous.
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
        cur_option_idx = cur_option_idx;
        start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        if(after_change) after_change();
    }
    
};


/* ----------------------------------------------------------------------------
 * Information about the options menu.
 */
class options_menu_state : public game_state {
public:
    options_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    //Known good resolution presets.
    vector<std::pair<int, int> > resolution_presets;
    //Currently selected resolution.
    std::pair<int, int> cur_resolution_option;
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg;
    //GUI.
    gui_manager gui;
    //Auto-throw picker widget.
    options_menu_picker_gui_item<AUTO_THROW_MODES>* auto_throw_picker;
    //Resolution picker widget.
    options_menu_picker_gui_item<std::pair<int, int> >* resolution_picker;
    //Cursor speed picker widget.
    options_menu_picker_gui_item<float>* cursor_speed_picker;
    //Cursor camera weight picker widget.
    options_menu_picker_gui_item<float>* cursor_cam_weight_picker;
    //Leaving confirmation picker widget.
    options_menu_picker_gui_item<LEAVING_CONFIRMATION_MODES>*
    leaving_confirmation_picker;
    //Restart warning text widget.
    text_gui_item* warning_text;
    
    void go_to_controls();
    void leave();
    void trigger_restart_warning();
};


/* ----------------------------------------------------------------------------
 * Information about the controls menu.
 */
class controls_menu_state : public game_state {
public:
    controls_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    //Bitmap of the menu's background.
    ALLEGRO_BITMAP* bmp_menu_bg;
    //GUI.
    gui_manager gui;
    //Control list widget.
    list_gui_item* list_box;
    //Is it currently capturing input?
    bool capturing_input;
    //If it's capturing input, this is the index of the control to capture for.
    size_t input_capture_control_nr;
    
    void add_control();
    void add_control_gui_items(const size_t index, const bool focus);
    void choose_button(const size_t index);
    void choose_next_action(const size_t index);
    void choose_prev_action(const size_t index);
    void delete_control(const size_t index);
    void delete_control_gui_items();
    void leave();
    
};


/* ----------------------------------------------------------------------------
 * Information about the area selection menu.
 */
class area_menu_state : public game_state {
public:
    //Type of area that the menu is dealing with.
    AREA_TYPES area_type;
    
    area_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    //Bitmap of the menu background,
    ALLEGRO_BITMAP* bmp_menu_bg;
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
    //Record clear of each area available.
    vector<bool> area_record_clears;
    //Record scores of each area available.
    vector<int> area_record_scores;
    //Record dates of each area available.
    vector<string> area_record_dates;
    //Main GUI.
    gui_manager gui;
    //Area info GUI item.
    gui_item* info_box;
    //Mission specs GUI item.
    gui_item* specs_box;
    //Currently selected area, or INVALID for none.
    size_t cur_area_idx;
    //Area list box item.
    list_gui_item* list_box;
    //Button of the first area available, if any.
    button_gui_item* first_area_button;
    //Name text item, in the info page.
    text_gui_item* info_name_text;
    //Name text item, in the specs page.
    text_gui_item* specs_name_text;
    //Subtitle text item.
    text_gui_item* subtitle_text;
    //Thumbnail of the currently selected area.
    ALLEGRO_BITMAP* cur_thumb;
    //Description text item.
    text_gui_item* description_text;
    //Difficulty text item.
    text_gui_item* difficulty_text;
    //Tags text item.
    text_gui_item* tags_text;
    //Maker text item.
    text_gui_item* maker_text;
    //Version text item.
    text_gui_item* version_text;
    //Record info text item.
    text_gui_item* record_info_text;
    //Record stamp of the currently selected area.
    ALLEGRO_BITMAP* cur_stamp;
    //Record medal of the currently selected area.
    ALLEGRO_BITMAP* cur_medal;
    //Record date text item.
    text_gui_item* record_date_text;
    //Goal text item.
    text_gui_item* goal_text;
    //Fail explanation list item.
    list_gui_item* fail_list;
    //Grading explanation list item.
    list_gui_item* grading_list;
    //Show the mission specs?
    bool show_mission_specs;
    
    void add_bullet(list_gui_item* list, const string &text);
    void animate_info_and_specs();
    void change_info(const size_t area_idx);
    void init_gui_main();
    void init_gui_info_page();
    void init_gui_specs_page();
    void leave();
    
};


/* ----------------------------------------------------------------------------
 * Information about the statistics menu.
 */
class stats_menu_state : public game_state {
public:
    stats_menu_state();
    void load() override;
    void unload() override;
    void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    void do_logic() override;
    void do_drawing() override;
    string get_name() const override;
    
private:
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg;
    //Statistics list item.
    list_gui_item* stats_list;
    //Runtime stat text item.
    text_gui_item* runtime_value_text;
    //GUI.
    gui_manager gui;
    
    void add_header(const string &label);
    text_gui_item* add_stat(
        const string &label, const string &value, const string &description
    );
    void leave();
    void populate_stats_list();
    void update_runtime_value_text();
};


#endif //ifndef MENUS_INCLUDED
