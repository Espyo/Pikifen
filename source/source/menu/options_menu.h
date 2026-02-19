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
extern const float SHORTCUT_BUTTON_HEIGHT;
extern const float SHORTCUT_BUTTON_PADDING;
extern const string SHORTCUTS_GUI_FILE_NAME;
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
 * @tparam ValueT The type of value the picker controls.
 */
template<typename ValueT>
class OptionsMenuPickerGuiItem : public PickerGuiItem {

public:

    //--- Public members ---
    
    //Points to the current value.
    ValueT* curValue = nullptr;
    
    //Default value.
    const ValueT defValue = ValueT();
    
    //Tooltip, sans default. Used if the presets don't have their own tooltips.
    string tooltip;
    
    //Value of each preset.
    vector<ValueT> presetValues;
    
    //Name of each preset.
    vector<string> presetNames;
    
    //Tooltip for each preset. If empty, "tooltip" is used instead.
    vector<string> presetDescriptions;
    
    //Code to run after a value is changed, if any.
    std::function<void()> afterChange = nullptr;
    
    //Converts a value to a string. Used in the tooltip's default, if necessary.
    std::function<string(ValueT)> valueToString = nullptr;
    
    
    //--- Public function definitions ---
    
    /**
     * @brief Constructs a new options menu picker GUI item object.
     *
     * @param baseText Base text.
     * @param curValue Current value.
     * @param defValue Default value.
     * @param presetValues Value of each preset.
     * @param presetNames Name of each preset.
     * @param tooltip Base tooltip.
     */
    OptionsMenuPickerGuiItem(
        const string& baseText, ValueT* curValue, const ValueT& defValue,
        const vector<ValueT>& presetValues, const vector<string>& presetNames,
        const string& tooltip = ""
    ) :
        PickerGuiItem(baseText, ""),
        curValue(curValue),
        defValue(defValue),
        tooltip(tooltip),
        presetValues(presetValues),
        presetNames(presetNames) {
    }
    
    
    /**
     * @brief Initializes the picker. This needs to be called after setting
     * all of its properties, since it relies on them existing and having their
     * final values. Without this function, the picker won't behave as
     * expected.
     */
    void init() {
        curOptionIdx = INVALID;
        for(size_t p = 0; p < presetValues.size(); p++) {
            if(*curValue == presetValues[p]) {
                curOptionIdx = p;
                break;
            }
        }
        option = getCurOptionName();
        nrOptions = presetValues.size();
        
        onPrevious = [this] () { changeOption(-1); };
        onNext = [this] () { changeOption(1); };
        onGetTooltip = [this] () {
            size_t defIdx = 0;
            string fullTooltip;
            for(; defIdx < this->presetValues.size(); defIdx++) {
                if(this->presetValues[defIdx] == this->defValue) {
                    break;
                }
            }
            if(presetDescriptions.empty()) {
                fullTooltip = this->tooltip;
            } else {
                if(curOptionIdx == INVALID) {
                    fullTooltip = "Using a custom value.";
                } else {
                    fullTooltip = presetDescriptions[curOptionIdx];
                }
            }
            fullTooltip += " Default: " + this->presetNames[defIdx] + ".";
            return fullTooltip;
        };
    }
    
    
    /**
     * @brief Returns the name of the current option.
     *
     * @return The name.
     */
    string getCurOptionName() {
        if(curOptionIdx == INVALID) {
            if(valueToString) {
                return valueToString(*curValue) + " (custom)";
            } else {
                return "Custom";
            }
        } else {
            return presetNames[curOptionIdx];
        }
    }
    
    
    /**
     * @brief Changes to the next or to the previous option.
     *
     * @param step What direction to change to. +1 is next, -1 is previous.
     */
    void changeOption(int step) {
        if(curOptionIdx == INVALID) {
            curOptionIdx = 0;
        } else {
            curOptionIdx =
                sumAndWrap(
                    (int) curOptionIdx, step, (int) presetValues.size()
                );
        }
        
        *curValue = presetValues[curOptionIdx];
        option = getCurOptionName();
        startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        if(afterChange) afterChange();
    }
    
};


/**
 * @brief Info about the options menu currently being presented to
 * the player.
 */
class OptionsMenu : public Menu {

public:

    //--- Public members ---
    
    //GUI for the top-level page.
    GuiManager topGui;
    
    //GUI for the controls options page.
    GuiManager controlsGui;
    
    //GUI for the control binds options page.
    GuiManager bindsGui;
    
    //GUI for the inventory shortcuts options page.
    GuiManager shortcutsGui;
    
    //GUI for the graphics options page.
    GuiManager graphicsGui;
    
    //GUI for the audio options page.
    GuiManager audioGui;
    
    //GUI for the misc. options page.
    GuiManager miscGui;
    
    
    //--- Public function declarations ---
    
    void draw() override;
    void load() override;
    void handleAllegroEvent(const ALLEGRO_EVENT& ev) override;
    bool handlePlayerAction(const Inpution::Action& action) override;
    void unload() override;
    void tick(float deltaT) override;
    
    
private:

    //--- Private members ---
    
    //Known good resolution presets.
    vector<std::pair<int, int> > resolutionPresets;
    
    //Currently selected resolution.
    std::pair<int, int> curResolutionOption;
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmpMenuBg = nullptr;
    
    //Information about the current pack management menu, if any.
    PacksMenu* packsMenu = nullptr;
    
    //Auto-throw picker item.
    OptionsMenuPickerGuiItem<AUTO_THROW_MODE>* autoThrowPicker = nullptr;
    
    //Resolution picker item.
    OptionsMenuPickerGuiItem<std::pair<int, int> >* resolutionPicker = nullptr;
    
    //Leader cursor with mouse item.
    CheckGuiItem* leaderCursorMouseCheck = nullptr;
    
    //Leader cursor speed picker item.
    OptionsMenuPickerGuiItem<float>* leaderCursorSpeedPicker = nullptr;
    
    //Leader cursor camera weight picker item.
    OptionsMenuPickerGuiItem<float>* leaderCursorCamWeightPicker = nullptr;
    
    //Fast inventory picker check item.
    CheckGuiItem* fastInventoryCheck = nullptr;
    
    //Leaving confirmation picker item.
    OptionsMenuPickerGuiItem<LEAVING_CONF_MODE>*
    leavingConfirmationPicker = nullptr;
    
    //Pikmin bump picker item.
    OptionsMenuPickerGuiItem<float>*
    pikminBumpPicker = nullptr;
    
    //Master volume picker item.
    OptionsMenuPickerGuiItem<float>* masterVolPicker = nullptr;
    
    //Gameplay sound effects volume picker item.
    OptionsMenuPickerGuiItem<float>* gameplaySoundVolPicker = nullptr;
    
    //Music volume picker item.
    OptionsMenuPickerGuiItem<float>* musicVolPicker = nullptr;
    
    //Ambiance sound effects volume picker item.
    OptionsMenuPickerGuiItem<float>* ambianceSoundVolPicker = nullptr;
    
    //UI sound effects volume picker item.
    OptionsMenuPickerGuiItem<float>* uiSoundVolPicker = nullptr;
    
    //Restart warning text item.
    TextGuiItem* warningText = nullptr;
    
    //Type of control binds to show.
    CONTROL_BINDS_MENU_TYPE bindsMenuType = CONTROL_BINDS_MENU_NORMAL;
    
    //GUI for the "more..." options of an action type in the binds menu.
    GuiManager bindsMoreGui;
    
    //Control binds list GUI item.
    ListGuiItem* bindsListBox = nullptr;
    
    //Is it currently capturing bind input? 0: No. 1: Capturing. 2: Finishing.
    unsigned char capturingInput = 0;
    
    //Time left before the input capturing times out.
    float capturingInputTimeout = 0.0f;
    
    //Is it showing an action type's "more..." menu in the binds menu?
    bool showingBindsMore = false;
    
    //List of binds per player action type.
    vector<vector<Inpution::Bind> > bindsPerActionType;
    
    //Current player action type.
    PLAYER_ACTION_TYPE curActionType = PLAYER_ACTION_TYPE_NONE;
    
    //Current global bind index we're working with.
    size_t curBindIdx = 0;
    
    //Whether we need to populate the binds.
    bool mustPopulateBinds = true;
    
    //Inventory shortcut index that's currently being edited. -1 for none.
    signed char curShortcutIdx = -1;
    
    //Each shortcut button, in order.
    vector<ButtonGuiItem*> shortcutButtons;
    
    //Shortcut items list explanation text GUI item.
    TextGuiItem* shortcutItemsListExplanation = nullptr;
    
    //Shortcut items list GUI item.
    ListGuiItem* shortcutItemsListBox = nullptr;
    
    //Scrollbar for the shortcut items list GUI item.
    ScrollGuiItem* shortcutItemsListScroll = nullptr;
    
    //Warning button.
    ButtonGuiItem* bindsWarningButton = nullptr;
    
    //List of actions that are recommended to have at least one bind, but don't.
    vector<PLAYER_ACTION_TYPE> unboundRecommendedActions;
    
    
    //--- Private function declarations ---
    
    void addNewBindEntryItems(
        const PlayerActionType& actionType, bool addSectionHeader,
        GuiItem** itemToFocus
    );
    void addNewOrUpdateBindFromInput(const Inpution::Input& input);
    void addNewShortcutItemItems(
        const string& name, const string& internalName, GuiItem** itemToFocus,
        const ALLEGRO_COLOR& textColor = COLOR_WHITE
    );
    void addNewShortcutItems(unsigned char index);
    void drawShortcutName(const GuiItem::DrawInfo& draw, unsigned char index);
    void initGuiAudioPage();
    void initGuiControlsPage();
    void initGuiControlBindsPage();
    void initGuiGraphicsPage();
    void initGuiMiscPage();
    void initGuiShortcutsPage();
    void initGuiTopPage();
    void triggerRestartWarning();
    void chooseInput(
        const PLAYER_ACTION_TYPE actionType, size_t bindIdx
    );
    void chooseShortcut();
    void deleteBind(
        const PLAYER_ACTION_TYPE actionType, size_t bindIdx
    );
    void populateBinds();
    void populateShortcutItems();
    void restoreDefaultBinds(const PLAYER_ACTION_TYPE actionType);
    void updateBindWarnings();
    void updateControlsPage();
    void updateShortcutsPage();
    
};
