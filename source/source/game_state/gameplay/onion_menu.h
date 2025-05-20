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
extern const float RED_TEXT_DURATION;
extern const size_t TYPES_PER_PAGE;
}


/**
 * @brief Info about a given Pikmin type in an Onion menu.
 */
struct OnionMenuPikminType {

    //--- Members ---
    
    //The player wants to add/subtract these many from the group.
    int delta = 0;
    
    //Index of this type in the Onion's list. Cache for convenience.
    size_t typeIdx = INVALID;
    
    //Index in the on-window list, or INVALID. Cache for convenience.
    size_t onWindowIdx = INVALID;
    
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
    
    //If it manages more than 5, this is the Pikmin type page index.
    size_t page = 0;
    
    //Which GUI items are in red right now, if any, and how much time left.
    map<GuiItem*, float> redItems;
    
    //Total page amount. Cache for convenience.
    size_t nrPages = 0;
    
    //Pikmin types currently on-window. Cache for convenience.
    vector<OnionMenuPikminType*> onWindowTypes;
    
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
    
    //The button that controls all Onions. Cache for convenience.
    GuiItem* onionAllButton = nullptr;
    
    //The button that controls all groups. Cache for convenience.
    GuiItem* groupAllButton = nullptr;
    
    //Left Onion "more..." icon. Cache for convenience.
    GuiItem* onionMoreLIcon = nullptr;
    
    //Right Onion "more..." icon. Cache for convenience.
    GuiItem* onionMoreRIcon = nullptr;
    
    //Left group "more..." icon. Cache for convenience.
    GuiItem* groupMoreLIcon = nullptr;
    
    //Right group "more..." icon. Cache for convenience.
    GuiItem* groupMoreRIcon = nullptr;
    
    //Previous page button. Cache for convenience.
    GuiItem* prevPageButton = nullptr;
    
    //Next page button. Cache for convenience.
    GuiItem* nextPageButton = nullptr;
    
    //Field amount text. Cache for convenience.
    GuiItem* fieldAmountText = nullptr;
    
    //Multiply the background alpha by this much.
    float bgAlphaMult = 0.0f;
    
    //Time left until the menu finishes closing.
    float closingTimer = 0.0f;
    
    //Is the struct meant to be deleted?
    bool toDelete = false;
    
    
    //--- Function declarations ---
    
    OnionMenu(PikminNest* nPtr, Leader* lPtr);
    ~OnionMenu();
    void addAllToGroup();
    void addAllToOnion();
    void addToGroup(size_t typeIdx);
    void addToOnion(size_t typeIdx);
    void confirm();
    void goToPage(size_t page);
    void growButtons();
    void handleAllegroEvent(const ALLEGRO_EVENT& ev);
    void handlePlayerAction(const PlayerAction& action);
    void startClosing();
    void tick(float deltaT);
    void toggleSelectAll();
    
    private:
    
    //--- Members ---
    
    //Is it currently closing?
    bool closing = false;
    
    
    //--- Function declarations ---
    
    void makeGuiItemRed(GuiItem* item);
    void update();
    
};
