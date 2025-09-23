/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion menu class and related functions.
 */

#pragma once

#include <string>

#include "../../content/mob/mob_utils.h"
#include "../../content/other/gui.h"


using std::size_t;
using std::string;


class PikminType;

namespace ONION_MENU {
extern const string GUI_FILE_PATH;
extern const size_t NR_TYPES_VISIBLE;
extern const float RED_TEXT_DURATION;
extern const string TYPE_GUI_FILE_PATH;
}


//Results for trying to transfer Pikmin up or down on the menu.
enum ONION_TRANSFER_RESULT {

    //OK.
    ONION_TRANSFER_RESULT_OK,
    
    //No more Pikmin inside the Onion to take out.
    ONION_TRANSFER_RESULT_NONE_IN_ONION,
    
    //No more Pikmin in the group to store.
    ONION_TRANSFER_RESULT_NONE_IN_GROUP,
    
    //No more space in the field.
    ONION_TRANSFER_RESULT_FIELD_FULL,
    
};


/**
 * @brief Info about a given Pikmin type in an Onion menu.
 */
struct OnionMenuPikminType {

    //--- Members ---
    
    //The player wants to add/subtract these many from the group.
    int delta = 0;
    
    //Index of this type in the Onion's list. Cache for convenience.
    size_t typeIdx = INVALID;
    
    //Pikmin type associated. Cache for convenience.
    PikminType* pikType = nullptr;
    
    
    //--- Function declarations ---
    
    OnionMenuPikminType(size_t idx, PikminType* pikType);
    
};


/**
 * @brief Info about the Onion menu currently being presented to
 * the player.
 */
struct OnionMenu {

    public:
    
    //--- Members ---
    
    //Pointer to the struct with nest information.
    PikminNest* nestPtr = nullptr;
    
    //Pointer to the leader responsible.
    Leader* leaderPtr = nullptr;
    
    //Information on every type's management.
    vector<OnionMenuPikminType> types;
    
    //GUI manager.
    GuiManager gui;
    
    //Is "select all" currently on?
    bool selectAll = false;
    
    //Is "change ten" currently on?
    bool changeTen = false;
    
    //Which GUI items are in red right now, if any, and how much time left.
    map<GuiItem*, float> redItems;
    
    //List of GUI items for the Onion icons. Cache for convenience.
    vector<GuiItem*> onionIconItems;
    
    //List of GUI items for the Onion buttons. Cache for convenience.
    vector<GuiItem*> onionButtonItems;
    
    //List of GUI items for the Onion amounts. Cache for convenience.
    vector<GuiItem*> onionAmountItems;
    
    //List of GUI items for the group icons. Cache for convenience.
    vector<GuiItem*> groupIconItems;
    
    //List of GUI items for the group buttons. Cache for convenience.
    vector<GuiItem*> groupButtonItems;
    
    //List of GUI items for the group amounts. Cache for convenience.
    vector<GuiItem*> groupAmountItems;
    
    //List of GUI items for the full type controls. Cache for convenience.
    vector<GuiItem*> fullTypeItems;
    
    //The button that controls all Onions. Cache for convenience.
    GuiItem* onionAllButton = nullptr;
    
    //The button that controls all groups. Cache for convenience.
    GuiItem* groupAllButton = nullptr;
    
    //The item that controls all Pikmin types at once. Cache for convenience.
    GuiItem* fullTypeAllItem = nullptr;
    
    //Field amount text. Cache for convenience.
    GuiItem* fieldAmountText = nullptr;
    
    //GUI item for the list of types. Cache for convenience.
    ListGuiItem* listItem = nullptr;
    
    //"Change 10" mode toggle button.
    ButtonGuiItem* changeTenButton = nullptr;
    
    //"Select all" mode toggle button.
    ButtonGuiItem* selectAllButton = nullptr;
    
    //Dummy item used to help with padding the list. Cache for convenience.
    GuiItem* listPaddingDummyItem = nullptr;
    
    //Multiply the background alpha by this much.
    float bgAlphaMult = 0.0f;
    
    //Time left until the menu finishes closing.
    float closingTimer = 0.0f;
    
    //Is the struct meant to be deleted?
    bool toDelete = false;
    
    
    //--- Function declarations ---
    
    OnionMenu(PikminNest* nPtr, Leader* lPtr);
    ~OnionMenu();
    ONION_TRANSFER_RESULT transfer(bool toGroup, size_t typeIdx);
    ONION_TRANSFER_RESULT canAddToGroup(size_t typeIdx);
    ONION_TRANSFER_RESULT canAddToOnion(size_t typeIdx);
    void confirm();
    void growButtons();
    void handleAllegroEvent(const ALLEGRO_EVENT& ev);
    void handlePlayerAction(const Inpution::Action& action);
    void startClosing();
    void tick(float deltaT);
    bool toggleChangeTen();
    bool toggleSelectAll();
    
    private:
    
    //--- Members ---
    
    //Is it currently closing?
    bool closing = false;
    
    
    //--- Function declarations ---
    
    bool doButtonDirLogic(int playerActionId, size_t typeIdx);
    void doButtonLogic(bool toGroup, size_t typeIdx, bool fromDirection);
    string getTransferAmountStr();
    void makeGuiItemRed(GuiItem* item);
    void update();
    
};
