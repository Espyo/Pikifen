/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the options menu struct and related functions.
 */

#pragma once

#include <functional>
#include <map>
#include <string>

#include "../content/other/gui.h"
#include "../core/options.h"
#include "packs_menu.h"

using std::map;
using std::string;


namespace OPTIONS_MENU {
extern const string AUDIO_GUI_FILE_NAME;
extern const float BIND_BUTTON_HEIGHT;
extern const float BIND_BUTTON_PADDING;
extern const string CONTROL_BINDS_GUI_FILE_NAME;
extern const string CONTROLS_GUI_FILE_NAME;
extern const string GRAPHICS_GUI_FILE_NAME;
extern const float HUD_MOVE_TIME;
extern const float INPUT_CAPTURE_TIMEOUT_DURATION;
extern const string MISC_GUI_FILE_NAME;
extern const string TOP_GUI_FILE_NAME;
}


//Possible control binds menu types.
enum CONTROL_BINDS_MENU_TYPE {

    //Normal controls.
    CONTROL_BINDS_MENU_NORMAL,
    
    //Special controls.
    CONTROL_BINDS_MENU_SPECIAL,
    
};


/**
 * @brief Info on how a picker GUI item in the options menu should work.
 *
 * @tparam t The type of value the picker controls.
 */
template<typename t>
class OptionsMenuPickerGuiItem : public PickerGuiItem {

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
    OptionsMenuPickerGuiItem(
        const string &base_text, t* cur_value, const t &def_value,
        const vector<t> &preset_values, const vector<string> &preset_names,
        const string &tooltip = ""
    ) :
        PickerGuiItem(base_text, ""),
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
        for(size_t p = 0; p < preset_values.size(); p++) {
            if(*cur_value == preset_values[p]) {
                cur_option_idx = p;
                break;
            }
        }
        option = getCurOptionName();
        nr_options = preset_values.size();
        
        on_previous = [this] () { changeOption(-1); };
        on_next = [this] () { changeOption(1); };
        on_get_tooltip = [this] () {
            size_t def_idx = 0;
            string full_tooltip;
            for(; def_idx < this->preset_values.size(); def_idx++) {
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
    string getCurOptionName() {
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
    void changeOption(int step) {
        if(cur_option_idx == INVALID) {
            cur_option_idx = 0;
        } else {
            cur_option_idx =
                sumAndWrap(
                    (int) cur_option_idx, step, (int) preset_values.size()
                );
        }
        
        *cur_value = preset_values[cur_option_idx];
        option = getCurOptionName();
        startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        if(after_change) after_change();
    }
    
};


/**
 * @brief Info about the options menu currently being presented to
 * the player.
 */
class OptionsMenu : public Menu {
public:

    //--- Members ---
    
    //GUI for the top-level page.
    GuiManager top_gui;
    
    //GUI for the controls options page.
    GuiManager controls_gui;
    
    //GUI for the control binds options page.
    GuiManager binds_gui;
    
    //GUI for the graphics options page.
    GuiManager graphics_gui;
    
    //GUI for the audio options page.
    GuiManager audio_gui;
    
    //GUI for the misc. options page.
    GuiManager misc_gui;
    
    
    //--- Function declarations ---
    
    void draw() override;
    void load() override;
    void handleAllegroEvent(const ALLEGRO_EVENT &ev) override;
    void handlePlayerAction(const PlayerAction &action) override;
    void unload() override;
    void tick(float delta_t);
    
    
private:

    //--- Members ---
    
    //Known good resolution presets.
    vector<std::pair<int, int> > resolution_presets;
    
    //Currently selected resolution.
    std::pair<int, int> cur_resolution_option;
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmp_menu_bg = nullptr;
    
    //Information about the current pack management menu, if any.
    PacksMenu* packs_menu = nullptr;
    
    //Auto-throw picker widget.
    OptionsMenuPickerGuiItem<AUTO_THROW_MODE>* auto_throw_picker = nullptr;
    
    //Resolution picker widget.
    OptionsMenuPickerGuiItem<std::pair<int, int> >* resolution_picker = nullptr;
    
    //Cursor speed picker widget.
    OptionsMenuPickerGuiItem<float>* cursor_speed_picker = nullptr;
    
    //Cursor camera weight picker widget.
    OptionsMenuPickerGuiItem<float>* cursor_cam_weight_picker = nullptr;
    
    //Leaving confirmation picker widget.
    OptionsMenuPickerGuiItem<LEAVING_CONF_MODE>*
    leaving_confirmation_picker = nullptr;
    
    //Master volume picker widget.
    OptionsMenuPickerGuiItem<float>* master_vol_picker = nullptr;
    
    //Gameplay sound effects volume picker widget.
    OptionsMenuPickerGuiItem<float>* gameplay_sound_vol_picker = nullptr;
    
    //Music volume picker widget.
    OptionsMenuPickerGuiItem<float>* music_vol_picker = nullptr;
    
    //Ambiance sound effects volume picker widget.
    OptionsMenuPickerGuiItem<float>* ambiance_sound_vol_picker = nullptr;
    
    //UI sound effects volume picker widget.
    OptionsMenuPickerGuiItem<float>* ui_sound_vol_picker = nullptr;
    
    //Restart warning text widget.
    TextGuiItem* warning_text = nullptr;
    
    //Type of control binds to show.
    CONTROL_BINDS_MENU_TYPE binds_menu_type = CONTROL_BINDS_MENU_NORMAL;
    
    //GUI for the "more..." options of an action type in the binds menu.
    GuiManager binds_more_gui;
    
    //Control binds list GUI item.
    ListGuiItem* binds_list_box = nullptr;
    
    //Is it currently capturing bind input? 0: No. 1: Capturing. 2: Finishing.
    unsigned char capturing_input = 0;
    
    //Time left before the input capturing times out.
    float capturing_input_timeout = 0.0f;
    
    //Is it showing an action type's "more..." menu in the binds menu?
    bool showing_binds_more = false;
    
    //List of binds per player action type.
    vector<vector<ControlBind> > binds_per_action_type;
    
    //Current player action type.
    PLAYER_ACTION_TYPE cur_action_type = PLAYER_ACTION_TYPE_NONE;
    
    //Current global bind index we're working with.
    size_t cur_bind_idx = 0;
    
    //Whether we need to populate the binds.
    bool must_populate_binds = true;
    
    
    //--- Function declarations ---
    
    void initGuiAudioPage();
    void initGuiControlsPage();
    void initGuiControlBindsPage();
    void initGuiGraphicsPage();
    void initGuiMiscPage();
    void initGuiTopPage();
    void triggerRestartWarning();
    void chooseInput(
        const PLAYER_ACTION_TYPE action_type, size_t bind_idx
    );
    void deleteBind(
        const PLAYER_ACTION_TYPE action_type, size_t bind_idx
    );
    void populateBinds();
    void restoreDefaultBinds(const PLAYER_ACTION_TYPE action_type);
    
};
