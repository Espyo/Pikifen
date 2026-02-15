/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pause menu classes and functions.
 */

#include <algorithm>

#include "gameplay.h"

#include "../../content/mob/resource.h"
#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace PAUSE_MENU {

//Control lockout time after entering the menu.
const float ENTRY_LOCKOUT_TIME = 0.15f;

//Interval between calculations of the Go Here path.
const float GO_HERE_CALC_INTERVAL = 0.15f;

//Name of the GUI definition file.
const string GUI_FILE_NAME = "pause_menu_system";

//Name of the mission page GUI definition file.
const string MISSION_GUI_FILE_NAME = "pause_menu_mission";

//Width and height of the mission mob marker.
const float MISSION_MOB_MARKER_SIZE = 24.0f;

//Multiply time by this much to animate the mission mob marker.
const float MISSION_MOB_MARKER_TIME_MULT = 3.0f;

//Name of the radar page GUI definition file.
const string RADAR_GUI_FILE_NAME = "pause_menu_radar";

//Maximum radar zoom level.
const float RADAR_MAX_ZOOM = 4.0f;

//Minimum radar zoom level.
const float RADAR_MIN_ZOOM = 0.03f;

//How long an Onion waits before fading to the next color.
const float RADAR_ONION_COLOR_FADE_CYCLE_DUR = 1.0f;

//How long an Onion fades between two colors.
const float RADAR_ONION_COLOR_FADE_DUR = 0.2f;

//Max radar pan speed when not using mouse, in pixels per second.
const float RADAR_PAN_SPEED = 600.0f;

//Max radar zoom speed when not using mouse, in amount per second.
const float RADAR_ZOOM_SPEED = 2.5f;

//Name of the status page GUI definition file.
const string STATUS_GUI_FILE_NAME = "pause_menu_status";

}


/**
 * @brief Constructs a new pause menu struct object.
 *
 * @param startOnRadar True if the page to start on should be the radar,
 * false if it should be the system page.
 */
PauseMenu::PauseMenu(bool startOnRadar) {
    pages.push_back(PAUSE_MENU_PAGE_SYSTEM);
    pages.push_back(PAUSE_MENU_PAGE_RADAR);
    pages.push_back(PAUSE_MENU_PAGE_STATUS);
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
        pages.push_back(PAUSE_MENU_PAGE_MISSION);
    }
    
    initMainPauseMenu();
    initRadarPage();
    initStatusPage();
    initMissionPage();
    
    //Initialize some radar things.
    bool foundValidSector = false;
    lowestSectorZ = FLT_MAX;
    highestSectorZ = -FLT_MAX;
    
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* sPtr = game.curAreaData->sectors[s];
        if(sPtr->type == SECTOR_TYPE_BLOCKING) continue;
        lowestSectorZ = std::min(lowestSectorZ, sPtr->z);
        highestSectorZ = std::max(highestSectorZ, sPtr->z);
        foundValidSector = true;
    }
    
    if(!foundValidSector || lowestSectorZ == highestSectorZ) {
        lowestSectorZ = -32.0f;
        highestSectorZ = 32.0f;
    }
    
    bool foundValidEdge = false;
    radarMinCoords = Point(FLT_MAX);
    radarMaxCoords = Point(-FLT_MAX);
    
    for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
        Edge* ePtr = game.curAreaData->edges[e];
        if(!ePtr->sectors[0] || !ePtr->sectors[1]) continue;
        if(
            ePtr->sectors[0]->type == SECTOR_TYPE_BLOCKING &&
            ePtr->sectors[1]->type == SECTOR_TYPE_BLOCKING
        ) {
            continue;
        }
        foundValidEdge = true;
        updateMinMaxCoords(
            radarMinCoords, radarMaxCoords,
            v2p(ePtr->vertexes[0])
        );
        updateMinMaxCoords(
            radarMinCoords, radarMaxCoords,
            v2p(ePtr->vertexes[1])
        );
    }
    
    if(!foundValidEdge) {
        radarMinCoords = Point();
        radarMaxCoords = Point();
    }
    radarMinCoords = radarMinCoords - 16.0f;
    radarMaxCoords = radarMaxCoords + 16.0f;
    
    radarSelectedLeader = game.states.gameplay->players[0].leaderPtr;
    
    if(radarSelectedLeader) {
        radarView.cam.setPos(radarSelectedLeader->pos);
    }
    radarView.cam.setZoom(game.states.gameplay->players[0].radarZoom);
    
    //Start the process.
    openingLockoutTimer = PAUSE_MENU::ENTRY_LOCKOUT_TIME;
    GuiManager* firstGui = startOnRadar ? &radarGui : &gui;
    firstGui->responsive = true;
    firstGui->startAnimation(
        GUI_MANAGER_ANIM_UP_TO_CENTER, GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
}


/**
 * @brief Destroys the pause menu struct object.
 */
PauseMenu::~PauseMenu() {
    gui.destroy();
    radarGui.destroy();
    statusGui.destroy();
    missionGui.destroy();
    
    if(secondaryMenu) {
        secondaryMenu->unload();
        delete secondaryMenu;
        secondaryMenu = nullptr;
    }
    
    game.content.bitmaps.list.free(bmpRadarCursor);
    game.content.bitmaps.list.free(bmpRadarPikmin);
    game.content.bitmaps.list.free(bmpRadarTreasure);
    game.content.bitmaps.list.free(bmpRadarEnemyAlive);
    game.content.bitmaps.list.free(bmpRadarEnemyDead);
    game.content.bitmaps.list.free(bmpRadarLeaderBubble);
    game.content.bitmaps.list.free(bmpRadarLeaderX);
    game.content.bitmaps.list.free(bmpRadarObstacle);
    game.content.bitmaps.list.free(bmpRadarOnionSkeleton);
    game.content.bitmaps.list.free(bmpRadarOnionBulb);
    game.content.bitmaps.list.free(bmpRadarShip);
    game.content.bitmaps.list.free(bmpRadarPath);
    bmpRadarCursor = nullptr;
    bmpRadarPikmin = nullptr;
    bmpRadarTreasure = nullptr;
    bmpRadarEnemyAlive = nullptr;
    bmpRadarEnemyDead = nullptr;
    bmpRadarLeaderBubble = nullptr;
    bmpRadarLeaderX = nullptr;
    bmpRadarObstacle = nullptr;
    bmpRadarOnionSkeleton = nullptr;
    bmpRadarOnionBulb = nullptr;
    bmpRadarShip = nullptr;
    bmpRadarPath = nullptr;
}


/**
 * @brief Adds a new bullet point to either the fail condition list, or the
 * grading explanation list.
 *
 * @param list List to add to.
 * @param text Text.
 * @param color Text color.
 */
void PauseMenu::addBullet(
    ListGuiItem* list, const string& text,
    const ALLEGRO_COLOR& color
) {
    size_t bulletIdx = list->children.size();
    const float BULLET_HEIGHT = 0.18f;
    const float BULLET_PADDING = 0.01f;
    const float BULLETS_OFFSET = 0.01f;
    const float bulletCenterY =
        (BULLETS_OFFSET + BULLET_HEIGHT / 2.0f) +
        ((BULLET_HEIGHT + BULLET_PADDING) * bulletIdx);
        
    BulletGuiItem* bullet =
        new BulletGuiItem(
        text, game.sysContent.fntStandard, color
    );
    bullet->ratioCenter = Point(0.50f, bulletCenterY);
    bullet->ratioSize = Point(0.96f, BULLET_HEIGHT);
    list->addChild(bullet);
    missionGui.addItem(bullet);
}


/**
 * @brief Adds a new line to one of the Pikmin status boxes.
 *
 * @param list List to add to.
 * @param pikType Relevant Pikmin type, if applicable.
 * @param groupText Text to display on the "group" cell.
 * @param idleText Text to display on the "idle" cell.
 * @param fieldText Text to display on the "field" cell.
 * @param onionText Text to display on the "Onion" cell.
 * @param totalText Text to display on the "total" cell.
 * @param newText Text to display on the "new" cell.
 * @param lostText Text to display on the "lost" cell.
 * @param isSingle True if this is a box with a single row.
 * @param isTotals True if this is the totals box.
 */
void PauseMenu::addPikminStatusLine(
    ListGuiItem* list,
    PikminType* pikType,
    const string& groupText,
    const string& idleText,
    const string& fieldText,
    const string& onionText,
    const string& totalText,
    const string& newText,
    const string& lostText,
    bool isSingle, bool isTotals
) {

    const float x1 = 0.00f;
    const float x2 = 1.00f;
    const float workingWidth = x2 - x1;
    const float itemXInterval = workingWidth / 8.0f;
    const float firstX = x1 + itemXInterval / 2.0f;
    const float itemWidth = itemXInterval - 0.02f;
    
    const float y1 = isSingle ? 0.0f : list->getChildrenSpan();
    const float itemHeight = isSingle ? 1.0f : 0.17f;
    const float numberItemHeight = isSingle ? itemHeight : itemHeight * 0.60f;
    const float itemYSpacing = isSingle ? 0.0f : 0.03f;
    const float itemY = y1 + itemHeight / 2.0f + itemYSpacing;
    
    ALLEGRO_FONT* font =
        (isSingle && !isTotals) ?
        game.sysContent.fntStandard :
        game.sysContent.fntCounter;
    string tooltipStart =
        pikType ?
        "Number of " + pikType->name + " " :
        "Total number of Pikmin ";
    bool canFocus = pikType || isTotals;
    
    if(pikType) {
    
        //Pikmin type.
        GuiItem* typeItem = new GuiItem();
        typeItem->onDraw =
        [pikType] (const DrawInfo & draw) {
            drawBitmapInBox(
                pikType->bmpIcon, draw.center, draw.size, true, 0.0f, draw.tint
            );
        };
        typeItem->ratioCenter =
            Point(
                firstX + itemXInterval * 0,
                itemY
            );
        typeItem->ratioSize = Point(itemWidth, itemHeight);
        list->addChild(typeItem);
        statusGui.addItem(typeItem);
        
    } else if(isTotals) {
    
        //Totals header.
        TextGuiItem* totalsHeaderItem =
            new TextGuiItem("Total", game.sysContent.fntAreaName);
        totalsHeaderItem->ratioCenter =
            Point(
                firstX + itemXInterval * 0,
                itemY
            );
        totalsHeaderItem->ratioSize = Point(itemWidth, itemHeight);
        list->addChild(totalsHeaderItem);
        statusGui.addItem(totalsHeaderItem);
        
    }
    
    //Group Pikmin.
    TextGuiItem* groupTextItem =
        new TextGuiItem(groupText, font);
    groupTextItem->focusable = canFocus;
    groupTextItem->ratioCenter =
        Point(
            firstX + itemXInterval * 1,
            itemY
        );
    groupTextItem->ratioSize = Point(itemWidth, numberItemHeight);
    if(canFocus) {
        groupTextItem->onGetTooltip =
        [tooltipStart] () {
            return tooltipStart + "in your active leader's group.";
        };
    }
    if(groupText == "0") {
        groupTextItem->color = changeAlpha(groupTextItem->color, 64);
    }
    list->addChild(groupTextItem);
    statusGui.addItem(groupTextItem);
    
    //Idle Pikmin.
    TextGuiItem* idleTextItem =
        new TextGuiItem(idleText, font);
    idleTextItem->focusable = canFocus;
    idleTextItem->ratioCenter =
        Point(
            firstX + itemXInterval * 2,
            itemY
        );
    idleTextItem->ratioSize = Point(itemWidth, numberItemHeight);
    if(canFocus) {
        idleTextItem->onGetTooltip =
        [tooltipStart] () {
            return tooltipStart + "idling on the field.";
        };
    }
    if(idleText == "0") {
        idleTextItem->color = changeAlpha(idleTextItem->color, 64);
    }
    list->addChild(idleTextItem);
    statusGui.addItem(idleTextItem);
    
    //Field Pikmin.
    TextGuiItem* fieldTextItem =
        new TextGuiItem(fieldText, font);
    fieldTextItem->focusable = canFocus;
    fieldTextItem->ratioCenter =
        Point(
            firstX + itemXInterval * 3,
            itemY
        );
    fieldTextItem->ratioSize = Point(itemWidth, numberItemHeight);
    if(canFocus) {
        fieldTextItem->onGetTooltip =
        [tooltipStart] () {
            return tooltipStart + "out on the field.";
        };
    }
    if(fieldText == "0") {
        fieldTextItem->color = changeAlpha(fieldTextItem->color, 64);
    }
    list->addChild(fieldTextItem);
    statusGui.addItem(fieldTextItem);
    
    //Onion Pikmin.
    TextGuiItem* onionTextItem =
        new TextGuiItem(onionText, font);
    onionTextItem->focusable = canFocus;
    onionTextItem->ratioCenter =
        Point(
            firstX + itemXInterval * 4,
            itemY
        );
    onionTextItem->ratioSize = Point(itemWidth, numberItemHeight);
    if(canFocus) {
        onionTextItem->onGetTooltip =
        [tooltipStart] () {
            return tooltipStart + "inside Onions.";
        };
    }
    if(onionText == "0") {
        onionTextItem->color = changeAlpha(onionTextItem->color, 64);
    }
    list->addChild(onionTextItem);
    statusGui.addItem(onionTextItem);
    
    //Total Pikmin.
    TextGuiItem* totalTextItem =
        new TextGuiItem(totalText, font, game.config.guiColors.gold);
    totalTextItem->focusable = canFocus;
    totalTextItem->ratioCenter =
        Point(
            firstX + itemXInterval * 5,
            itemY
        );
    totalTextItem->ratioSize = Point(itemWidth, numberItemHeight);
    if(canFocus) {
        totalTextItem->onGetTooltip =
        [tooltipStart] () {
            return tooltipStart + "you have.";
        };
    }
    if(totalText == "0") {
        totalTextItem->color = changeAlpha(totalTextItem->color, 64);
    }
    list->addChild(totalTextItem);
    statusGui.addItem(totalTextItem);
    
    //Separator.
    GuiItem* separatorItem = new GuiItem();
    separatorItem->ratioCenter =
        Point(firstX + itemXInterval * 5.5f, itemY);
    separatorItem->ratioSize = Point(1.0f, itemHeight);
    separatorItem->onDraw =
    [] (const DrawInfo & draw) {
        al_draw_line(
            draw.center.x, draw.center.y - draw.size.y / 2.0f,
            draw.center.x, draw.center.y + draw.size.y / 2.0f,
            tintColor(COLOR_TRANSPARENT_WHITE, draw.tint), 5.0f
        );
    };
    list->addChild(separatorItem);
    statusGui.addItem(separatorItem);
    
    //New Pikmin.
    TextGuiItem* newTextItem =
        new TextGuiItem(
        newText, font, game.config.guiColors.good
    );
    newTextItem->focusable = canFocus;
    newTextItem->ratioCenter =
        Point(
            firstX + itemXInterval * 6,
            itemY
        );
    newTextItem->ratioSize = Point(itemWidth, numberItemHeight);
    if(canFocus) {
        newTextItem->onGetTooltip =
        [tooltipStart] () {
            return tooltipStart + "born today.";
        };
    }
    if(newText == "0") {
        newTextItem->color = changeAlpha(newTextItem->color, 64);
    }
    list->addChild(newTextItem);
    statusGui.addItem(newTextItem);
    
    //Lost Pikmin.
    TextGuiItem* lostTextItem =
        new TextGuiItem(
        lostText, font, game.config.guiColors.bad
    );
    lostTextItem->focusable = canFocus;
    lostTextItem->ratioCenter =
        Point(
            firstX + itemXInterval * 7,
            itemY
        );
    lostTextItem->ratioSize = Point(itemWidth, numberItemHeight);
    if(canFocus) {
        lostTextItem->onGetTooltip =
        [tooltipStart] () {
            return tooltipStart + "lost today.";
        };
    }
    if(lostText == "0") {
        lostTextItem->color = changeAlpha(lostTextItem->color, 64);
    }
    list->addChild(lostTextItem);
    statusGui.addItem(lostTextItem);
}


/**
 * @brief Calculates the Go Here path from the selected leader to the radar
 * cursor, if applicable, and stores the results in goHerePath and
 * goHerePathResult.
 */
void PauseMenu::calculateGoHerePath() {
    radarCursorLeader = nullptr;
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); l++) {
        Leader* lPtr = game.states.gameplay->mobs.leaders[l];
        if(
            lPtr->health > 0 &&
            Distance(lPtr->pos, radarCursor) <= 24.0f / radarView.cam.zoom
        ) {
            radarCursorLeader = lPtr;
            break;
        }
    }
    
    if(
        !radarSelectedLeader ||
        radarCursorLeader ||
        Distance(radarSelectedLeader->pos, radarCursor) < 128.0f
    ) {
        goHerePath.clear();
        goHerePathResult = PATH_RESULT_ERROR;
        return;
    }
    
    if(!radarSelectedLeader->fsm.getEvent(LEADER_EV_GO_HERE)) {
        goHerePath.clear();
        goHerePathResult = PATH_RESULT_ERROR;
        return;
    }
    
    Sector* cursorSector = getSector(radarCursor, nullptr, true);
    
    if(!cursorSector || cursorSector->type == SECTOR_TYPE_BLOCKING) {
        goHerePath.clear();
        goHerePathResult = PATH_RESULT_ERROR;
        return;
    }
    
    PathFollowSettings settings;
    settings.flags =
        PATH_FOLLOW_FLAG_CAN_CONTINUE | PATH_FOLLOW_FLAG_LIGHT_LOAD;
    settings.invulnerabilities =
        radarSelectedLeader->group->getGroupInvulnerabilities(
            radarSelectedLeader
        );
        
    goHerePathResult =
        getPath(
            radarSelectedLeader->pos,
            radarCursor,
            settings,
            goHerePath, nullptr, nullptr, nullptr
        );
}


/**
 * @brief Either asks the player to confirm if they wish to leave, or leaves
 * outright, based on the player's confirmation question preferences.
 *
 */
void PauseMenu::confirmOrLeave() {
    bool doConfirmation = false;
    string confirmationExplanation;
    switch(game.options.misc.leavingConfMode) {
    case LEAVING_CONF_MODE_NEVER: {
        doConfirmation = false;
        break;
    } case LEAVING_CONF_MODE_1_MIN: {
        doConfirmation =
            game.states.gameplay->gameplayTimePassed >= 60.0f;
        break;
    } case LEAVING_CONF_MODE_ALWAYS: {
        doConfirmation = true;
        break;
    } case N_LEAVING_CONF_MODES: {
        break;
    }
    }
    
    if(doConfirmation) {
        switch(leaveTarget) {
        case GAMEPLAY_LEAVE_TARGET_RETRY: {
            confirmationExplanation =
                "If you retry, you will LOSE all of your progress "
                "and start over. Are you sure you want to retry?";
            break;
        } case GAMEPLAY_LEAVE_TARGET_END: {
            confirmationExplanation =
                "If you end now, you will stop playing and will go to the "
                "results menu.";
            if(game.curAreaData->type == AREA_TYPE_MISSION) {
                if(
                    game.curAreaData->missionOld.goal ==
                    MISSION_GOAL_END_MANUALLY
                ) {
                    confirmationExplanation +=
                        " The goal of this mission is to end through here, so "
                        "make sure you've done everything you need first.";
                } else {
                    confirmationExplanation +=
                        " This will end the mission as a fail, "
                        "even though you may still get a medal from it.";
                    if(
                        game.curAreaData->missionOld.gradingMode ==
                        MISSION_GRADING_MODE_POINTS
                    ) {
                        confirmationExplanation +=
                            " Note that since you fail the mission, you may "
                            "lose out on some points. You should check the "
                            "pause menu's mission page for more information.";
                    }
                    
                }
            }
            confirmationExplanation +=
                " Are you sure you want to end?";
            break;
        } case GAMEPLAY_LEAVE_TARGET_AREA_SELECT: {
            confirmationExplanation =
                "If you quit, you will LOSE all of your progress and instantly "
                "stop playing. Are you sure you want to quit?";
            break;
        }
        }
        
        game.modal.reset();
        game.modal.title = "Are you sure?";
        game.modal.prompt =
            confirmationExplanation + "\n\n"
            "(You can customize this confirmation question in the "
            "options menu.)";
        game.modal.extraButtons.push_back(
        ModalGuiManager::Button {
            .text = "Confirm",
            .tooltip = "Yes, I'm sure.",
            .color = game.config.guiColors.bad,
            .onActivate = [this] (const Point&) {
                startLeavingGameplay();
            }
        }
        );
        game.modal.defaultFocusButtonIdx = 1;
        game.modal.updateItems();
        game.modal.open();
        
    } else {
        startLeavingGameplay();
    }
}


/**
 * @brief Creates a button meant for changing to a page either to the left or
 * to the right of the current one.
 *
 * @param targetPage Which page this button leads to.
 * @param left True if this page is to the left of the current,
 * false if to the right.
 * @param curGui Pointer to the current page's GUI manager.
 * @return The button.
 */
ButtonGuiItem* PauseMenu::createPageButton(
    PAUSE_MENU_PAGE targetPage, bool left,
    GuiManager* curGui
) {
    string pageName;
    string tooltipName;
    switch(targetPage) {
    case PAUSE_MENU_PAGE_SYSTEM: {
        pageName = "System";
        tooltipName = "system";
        break;
    } case PAUSE_MENU_PAGE_RADAR: {
        pageName = "Radar";
        tooltipName = "radar";
        break;
    } case PAUSE_MENU_PAGE_STATUS: {
        pageName = "Status";
        tooltipName = "status";
        break;
    } case PAUSE_MENU_PAGE_MISSION: {
        pageName = "Mission";
        tooltipName = "mission";
        break;
    }
    }
    
    ButtonGuiItem* newButton =
        new ButtonGuiItem(
        left ?
        "< " + pageName :
        pageName + " >",
        game.sysContent.fntStandard,
        game.config.guiColors.pageChange
    );
    newButton->onActivate =
    [this, curGui, targetPage, left] (const Point&) {
        switchPage(curGui, targetPage, left);
    };
    newButton->onGetTooltip =
    [tooltipName] () {
        return "Go to the pause menu's " + tooltipName + " page.";
    };
    
    return newButton;
}


/**
 * @brief Creates the buttons and input GUI items that allow switching pages.
 *
 * @param curPage Page that these creations belong to.
 * @param curGui Pointer to the current page's GUI manager.
 */
void PauseMenu::createPageButtons(
    PAUSE_MENU_PAGE curPage, GuiManager* curGui
) {
    size_t curPageIdx =
        std::distance(
            pages.begin(),
            std::find(pages.begin(), pages.end(), curPage)
        );
    size_t leftPageIdx = sumAndWrap((int) curPageIdx, -1, (int) pages.size());
    size_t rightPageIdx = sumAndWrap((int) curPageIdx, 1, (int) pages.size());
    
    //Left page button.
    ButtonGuiItem* leftPageButton =
        createPageButton(pages[leftPageIdx], true, curGui);
    curGui->addItem(leftPageButton, "left_page");
    
    //Left page input icon.
    GuiItem* leftPageInput = new GuiItem();
    leftPageInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showGuiInputIcons) return;
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_MENU_PAGE_LEFT, draw.center, draw.size,
            true, game.sysContent.fntSlim, draw.tint
        );
    };
    curGui->addItem(leftPageInput, "left_page_input");
    
    //Right page button.
    ButtonGuiItem* rightPageButton =
        createPageButton(pages[rightPageIdx], false, curGui);
    curGui->addItem(rightPageButton, "right_page");
    
    //Right page input icon.
    GuiItem* rightPageInput = new GuiItem();
    rightPageInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showGuiInputIcons) return;
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT, draw.center, draw.size,
            true, game.sysContent.fntSlim, draw.tint
        );
    };
    curGui->addItem(rightPageInput, "right_page_input");
    
    leftPageButtons[curGui] = leftPageButton;
    rightPageButtons[curGui] = rightPageButton;
}


/**
 * @brief Draws the pause menu.
 */
void PauseMenu::draw() {
    gui.draw();
    radarGui.draw();
    statusGui.draw();
    missionGui.draw();
    if(secondaryMenu) secondaryMenu->draw();
    game.modal.draw();
}


/**
 * @brief Draws a segment of the Go Here path.
 *
 * @param start Starting point.
 * @param end Ending point.
 * @param color Color of the segment.
 * @param texturePoint Pointer to a variable keeping track of what point of
 * the texture we've drawn so far for this path, so that the effect is seamless
 * between segments.
 */
void PauseMenu::drawGoHereSegment(
    const Point& start, const Point& end,
    const ALLEGRO_COLOR& color, float* texturePoint
) {
    const float PATH_SEGMENT_THICKNESS = 12.0f / radarView.cam.zoom;
    const float PATH_SEGMENT_TIME_MULT = 10.0f;
    
    ALLEGRO_VERTEX av[4];
    for(unsigned char a = 0; a < 4; a++) {
        av[a].color = color;
        av[a].z = 0.0f;
    }
    int bmpH = al_get_bitmap_height(bmpRadarPath);
    float textureScale = bmpH / PATH_SEGMENT_THICKNESS / radarView.cam.zoom;
    float angle = getAngle(start, end);
    float distance = Distance(start, end).toFloat() * radarView.cam.zoom;
    float textureOffset = game.timePassed * PATH_SEGMENT_TIME_MULT;
    float textureStart = *texturePoint;
    float textureEnd = textureStart + distance;
    Point rotOffset = rotatePoint(Point(0, PATH_SEGMENT_THICKNESS), angle);
    
    av[0].x = start.x - rotOffset.x;
    av[0].y = start.y - rotOffset.y;
    av[1].x = start.x + rotOffset.x;
    av[1].y = start.y + rotOffset.y;
    av[2].x = end.x - rotOffset.x;
    av[2].y = end.y - rotOffset.y;
    av[3].x = end.x + rotOffset.x;
    av[3].y = end.y + rotOffset.y;
    
    av[0].u = (textureStart - textureOffset) * textureScale;
    av[0].v = 0.0f;
    av[1].u = (textureStart - textureOffset) * textureScale;
    av[1].v = bmpH;
    av[2].u = (textureEnd - textureOffset) * textureScale;
    av[2].v = 0.0f;
    av[3].u = (textureEnd - textureOffset) * textureScale;
    av[3].v = bmpH;
    
    al_draw_prim(
        av, nullptr, bmpRadarPath, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP
    );
    
    *texturePoint = textureEnd;
}


/**
 * @brief Draws the radar itself.
 *
 * @param center Center coordinates of the radar on-window.
 * @param size Width and height of the radar on-window.
 */
void PauseMenu::drawRadar(
    const Point& center, const Point& size
) {
    //Setup.
    ALLEGRO_TRANSFORM oldTransform;
    int oldCrX = 0;
    int oldCrY = 0;
    int oldCrW = 0;
    int oldCrH = 0;
    al_copy_transform(&oldTransform, al_get_current_transform());
    al_get_clipping_rectangle(&oldCrX, &oldCrY, &oldCrW, &oldCrH);
    
    al_use_transform(&radarView.worldToWindowTransform);
    al_set_clipping_rectangle(
        center.x - size.x / 2.0f,
        center.y - size.y / 2.0f,
        size.x,
        size.y
    );
    
    //Background fill.
    al_clear_to_color(game.config.aestheticRadar.backgroundColor);
    
    //Draw each sector.
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* sPtr = game.curAreaData->sectors[s];
        
        if(sPtr->type == SECTOR_TYPE_BLOCKING) continue;
        ALLEGRO_COLOR color =
            interpolateColor(
                sPtr->z, lowestSectorZ, highestSectorZ,
                game.config.aestheticRadar.lowestColor,
                game.config.aestheticRadar.highestColor
            );
            
        if(sPtr->hazard && sPtr->hazard->associatedLiquid) {
            color =
                interpolateColor(
                    0.80f, 0.0f, 1.0f,
                    color, sPtr->hazard->associatedLiquid->radarColor
                );
        }
        
        for(size_t t = 0; t < sPtr->triangles.size(); t++) {
            ALLEGRO_VERTEX av[3];
            for(size_t v = 0; v < 3; v++) {
                av[v].u = 0;
                av[v].v = 0;
                av[v].x = sPtr->triangles[t].points[v]->x;
                av[v].y = sPtr->triangles[t].points[v]->y;
                av[v].z = 0;
                av[v].color = color;
            }
            
            al_draw_prim(
                av, nullptr, nullptr,
                0, 3, ALLEGRO_PRIM_TRIANGLE_LIST
            );
        }
    }
    
    //Draw each edge.
    for(size_t e = 0; e < game.curAreaData->edges.size(); e++) {
        Edge* ePtr = game.curAreaData->edges[e];
        
        if(!ePtr->sectors[0] || !ePtr->sectors[1]) {
            //The other side is already the void, so no need for an edge.
            continue;
        }
        
        if(
            fabs(ePtr->sectors[0]->z - ePtr->sectors[1]->z) <=
            GEOMETRY::STEP_HEIGHT
        ) {
            //Step.
            continue;
        }
        
        al_draw_line(
            ePtr->vertexes[0]->x,
            ePtr->vertexes[0]->y,
            ePtr->vertexes[1]->x,
            ePtr->vertexes[1]->y,
            game.config.aestheticRadar.edgeColor,
            1.5f / radarView.cam.zoom
        );
    }
    
    //Mission exit region.
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->missionOld.goal == MISSION_GOAL_GET_TO_EXIT
    ) {
        drawHighlightedRectRegion(
            game.curAreaData->missionOld.goalExitCenter,
            game.curAreaData->missionOld.goalExitSize,
            changeAlpha(game.config.guiColors.gold, 192), game.timePassed
        );
    }
    
    //Onion icons.
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); o++) {
        Onion* oPtr =
            game.states.gameplay->mobs.onions[o];
        vector<PikminType*>* pikTypesPtr =
            &oPtr->nest->nestType->pikTypes;
            
        size_t nrPikTypes = pikTypesPtr->size();
        if(nrPikTypes > 0) {
            float fadeCyclePos =
                std::min(
                    (float) fmod(
                        game.timePassed,
                        PAUSE_MENU::RADAR_ONION_COLOR_FADE_CYCLE_DUR
                    ),
                    PAUSE_MENU::RADAR_ONION_COLOR_FADE_DUR
                );
                
            size_t pikTypeIdxTarget =
                (int) (
                    game.timePassed /
                    PAUSE_MENU::RADAR_ONION_COLOR_FADE_CYCLE_DUR
                ) % nrPikTypes;
            size_t pikTypeIdxPrev =
                (pikTypeIdxTarget + nrPikTypes - 1) % nrPikTypes;
                
            ALLEGRO_COLOR targetColor =
                interpolateColor(
                    fadeCyclePos, 0.0f,
                    PAUSE_MENU::RADAR_ONION_COLOR_FADE_DUR,
                    pikTypesPtr->at(pikTypeIdxPrev)->mainColor,
                    pikTypesPtr->at(pikTypeIdxTarget)->mainColor
                );
                
            drawBitmap(
                bmpRadarOnionBulb, oPtr->pos,
                Point(24.0f / radarView.cam.zoom),
                0.0f,
                targetColor
            );
        }
        drawBitmap(
            bmpRadarOnionSkeleton, oPtr->pos,
            Point(24.0f / radarView.cam.zoom)
        );
    }
    
    //Ship icons.
    for(size_t s = 0; s < game.states.gameplay->mobs.ships.size(); s++) {
        Ship* sPtr = game.states.gameplay->mobs.ships[s];
        
        drawBitmap(
            bmpRadarShip, sPtr->pos,
            Point(24.0f / radarView.cam.zoom)
        );
    }
    
    //Enemy icons.
    for(size_t e = 0; e < game.states.gameplay->mobs.enemies.size(); e++) {
        Enemy* ePtr = game.states.gameplay->mobs.enemies[e];
        if(ePtr->parent) continue;
        
        drawBitmap(
            ePtr->health > 0 ? bmpRadarEnemyAlive : bmpRadarEnemyDead,
            ePtr->pos,
            Point(24.0f / radarView.cam.zoom),
            ePtr->health > 0 ? game.timePassed : 0.0f
        );
    }
    
    //Leader icons.
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); l++) {
        Leader* lPtr = game.states.gameplay->mobs.leaders[l];
        
        drawBitmap(
            lPtr->leaType->bmpIcon, lPtr->pos,
            Point(40.0f / radarView.cam.zoom)
        );
        drawBitmap(
            bmpRadarLeaderBubble, lPtr->pos,
            Point(48.0f / radarView.cam.zoom),
            0.0f,
            radarSelectedLeader == lPtr ?
            al_map_rgb(0, 255, 255) :
            COLOR_WHITE
        );
        drawFilledEquilateralTriangle(
            lPtr->pos +
            rotatePoint(Point(24.5f / radarView.cam.zoom, 0.0f), lPtr->angle),
            6.0f / radarView.cam.zoom,
            lPtr->angle,
            radarSelectedLeader == lPtr ?
            al_map_rgb(0, 255, 255) :
            lPtr->health > 0 ?
            COLOR_WHITE :
            al_map_rgb(128, 128, 128)
        );
        if(lPtr->health <= 0) {
            drawBitmap(
                bmpRadarLeaderX, lPtr->pos,
                Point(36.0f / radarView.cam.zoom)
            );
        }
    }
    
    //Treasure icons.
    for(size_t t = 0; t < game.states.gameplay->mobs.treasures.size(); t++) {
        Treasure* tPtr = game.states.gameplay->mobs.treasures[t];
        
        drawBitmap(
            bmpRadarTreasure, tPtr->pos,
            Point(32.0f / radarView.cam.zoom),
            sin(game.timePassed * 2.0f) * (TAU * 0.05f)
        );
    }
    for(size_t r = 0; r < game.states.gameplay->mobs.resources.size(); r++) {
        Resource* rPtr = game.states.gameplay->mobs.resources[r];
        if(
            rPtr->resType->deliveryResult !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        
        drawBitmap(
            bmpRadarTreasure, rPtr->pos,
            Point(32.0f / radarView.cam.zoom),
            sin(game.timePassed * 2.0f) * (TAU * 0.05f)
        );
    }
    for(size_t p = 0; p < game.states.gameplay->mobs.piles.size(); p++) {
        Pile* pPtr = game.states.gameplay->mobs.piles[p];
        if(
            !pPtr->pilType->contents ||
            pPtr->amount == 0 ||
            pPtr->pilType->contents->deliveryResult !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        
        drawBitmap(
            bmpRadarTreasure, pPtr->pos,
            Point(32.0f / radarView.cam.zoom),
            sin(game.timePassed * 2.0f) * (TAU * 0.05f)
        );
    }
    
    //Pikmin icons.
    for(size_t p = 0; p < game.states.gameplay->mobs.pikmin.size(); p++) {
        Pikmin* pPtr = game.states.gameplay->mobs.pikmin[p];
        
        drawBitmap(
            bmpRadarPikmin, pPtr->pos,
            Point(16.0f / radarView.cam.zoom),
            0.0f,
            pPtr->pikType->mainColor
        );
    }
    
    //Obstacle icons.
    unordered_set<Mob*> obstacles;
    for(const auto& o : game.states.gameplay->pathMgr.obstructions) {
        obstacles.insert(o.second.begin(), o.second.end());
    }
    for(const auto& o : obstacles) {
        drawBitmap(
            bmpRadarObstacle, o->pos,
            Point(40.0f / radarView.cam.zoom),
            o->angle
        );
    }
    
    //Mission mob markers.
    if(!game.states.gameplay->missionRemainingMobIds.empty()) {
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            Mob* mPtr = game.states.gameplay->mobs.all[m];
            if(
                !isInContainer(
                    game.states.gameplay->missionRemainingMobIds, mPtr->id
                )
            ) continue;
            
            float alpha =
                (
                    sin(
                        game.timePassed *
                        PAUSE_MENU::MISSION_MOB_MARKER_TIME_MULT
                    )
                ) + 0.5f;
            alpha = std::clamp(alpha, 0.0f, 1.0f);
            drawBitmap(
                game.sysContent.bmpMissionMob, mPtr->pos,
                Point(PAUSE_MENU::MISSION_MOB_MARKER_SIZE) / radarView.cam.zoom,
                0.0f, multAlpha(game.config.guiColors.gold, alpha)
            );
        };
    }
    
    //Currently-active Go Here paths.
    for(size_t l = 0; l < game.states.gameplay->mobs.leaders.size(); l++) {
        Leader* lPtr = game.states.gameplay->mobs.leaders[l];
        if(!lPtr->midGoHere) continue;
        
        float pathTexturePoint = 0.0f;
        ALLEGRO_COLOR color = al_map_rgba(120, 140, 170, 192);
        
        switch(lPtr->pathInfo->result) {
        case PATH_RESULT_DIRECT:
        case PATH_RESULT_DIRECT_NO_STOPS:
        case PATH_RESULT_DIRECT_NO_ACCESSIBLE_STOPS: {
            //Go directly from A to B.
            
            drawGoHereSegment(
                lPtr->pos,
                lPtr->pathInfo->settings.targetPoint,
                color, &pathTexturePoint
            );
            
            break;
            
        } case PATH_RESULT_NORMAL_PATH:
        case PATH_RESULT_PATH_WITH_SINGLE_STOP:
        case PATH_RESULT_PATH_WITH_OBSTACLES: {
    
            size_t firstStop = lPtr->pathInfo->curPathStopIdx;
            if(firstStop >= lPtr->pathInfo->path.size()) continue;
            
            drawGoHereSegment(
                lPtr->pos,
                lPtr->pathInfo->path[firstStop]->pos,
                color, &pathTexturePoint
            );
            for(
                size_t s = firstStop + 1;
                s < lPtr->pathInfo->path.size();
                s++
            ) {
                drawGoHereSegment(
                    lPtr->pathInfo->path[s - 1]->pos,
                    lPtr->pathInfo->path[s]->pos,
                    color, &pathTexturePoint
                );
            }
            drawGoHereSegment(
                lPtr->pathInfo->path.back()->pos,
                lPtr->pathInfo->settings.targetPoint,
                color, &pathTexturePoint
            );
            
            break;
            
        } default: {
    
            break;
        }
        }
    }
    
    //Go Here choice path.
    float pathTexturePoint = 0.0f;
    switch(goHerePathResult) {
    case PATH_RESULT_DIRECT:
    case PATH_RESULT_DIRECT_NO_STOPS:
    case PATH_RESULT_DIRECT_NO_ACCESSIBLE_STOPS: {
        //Go directly from A to B.
        
        drawGoHereSegment(
            radarSelectedLeader->pos,
            radarCursor,
            al_map_rgb(64, 200, 240), &pathTexturePoint
        );
        
        break;
        
    } case PATH_RESULT_NORMAL_PATH:
    case PATH_RESULT_PATH_WITH_SINGLE_STOP:
    case PATH_RESULT_PATH_WITH_OBSTACLES: {
        //Regular path.
        ALLEGRO_COLOR color;
        if(goHerePathResult == PATH_RESULT_PATH_WITH_OBSTACLES) {
            color = al_map_rgb(200, 64, 64);
        } else {
            color = al_map_rgb(64, 200, 240);
        }
        
        if(!goHerePath.empty()) {
            drawGoHereSegment(
                radarSelectedLeader->pos,
                goHerePath[0]->pos,
                color, &pathTexturePoint
            );
            for(size_t s = 1; s < goHerePath.size(); s++) {
                drawGoHereSegment(
                    goHerePath[s - 1]->pos,
                    goHerePath[s]->pos,
                    color, &pathTexturePoint
                );
            }
            drawGoHereSegment(
                goHerePath.back()->pos,
                radarCursor,
                color, &pathTexturePoint
            );
        }
        
        break;
        
    } default: {

        break;
    }
    }
    
    //Radar cursor.
    drawBitmap(
        bmpRadarCursor, radarCursor,
        Point(48.0f / radarView.cam.zoom),
        game.timePassed * TAU * 0.3f
    );
    
    //Debugging feature -- show area active cells.
    if(game.debug.showAreaActiveCells) {
        for(
            size_t cellX = 0;
            cellX < game.states.gameplay->areaActiveCells.size();
            cellX++
        ) {
            for(
                size_t cellY = 0;
                cellY < game.states.gameplay->areaActiveCells[cellX].size();
                cellY++
            ) {
                float startX =
                    game.curAreaData->bmap.topLeftCorner.x +
                    cellX * GEOMETRY::AREA_CELL_SIZE;
                float startY =
                    game.curAreaData->bmap.topLeftCorner.y +
                    cellY * GEOMETRY::AREA_CELL_SIZE;
                al_draw_rectangle(
                    startX + (1.0f / radarView.cam.zoom),
                    startY + (1.0f / radarView.cam.zoom),
                    startX + GEOMETRY::AREA_CELL_SIZE -
                    (1.0f / radarView.cam.zoom),
                    startY + GEOMETRY::AREA_CELL_SIZE -
                    (1.0f / radarView.cam.zoom),
                    game.states.gameplay->areaActiveCells[cellX][cellY] ?
                    al_map_rgb(32, 192, 32) :
                    al_map_rgb(192, 32, 32),
                    1.0f / radarView.cam.zoom
                );
            }
        }
    }
    
    //Return to normal drawing.
    al_use_transform(&oldTransform);
    al_set_clipping_rectangle(oldCrX, oldCrY, oldCrW, oldCrH);
    
    float decoPadding = std::min(size.x * 0.02f, size.y * 0.02f);
    
    //North indicator.
    float northIndSize = std::min(size.x * 0.08f, size.y * 0.08f);
    Point northIndCenter(
        center.x - size.x / 2.0f + northIndSize / 2.0f + decoPadding,
        center.y - size.y / 2.0f + northIndSize / 2.0f + decoPadding
    );
    al_draw_filled_circle(
        northIndCenter.x, northIndCenter.y,
        northIndSize / 2.0f, game.config.aestheticRadar.backgroundColor
    );
    drawText(
        "N", game.sysContent.fntSlim,
        Point(northIndCenter.x, northIndCenter.y + 1.0f),
        Point(northIndSize * 0.40f),
        game.config.aestheticRadar.highestColor
    );
    al_draw_filled_triangle(
        northIndCenter.x,
        northIndCenter.y - northIndSize * 0.40f,
        northIndCenter.x - northIndSize * 0.20f,
        northIndCenter.y - northIndSize * 0.20f,
        northIndCenter.x + northIndSize * 0.20f,
        northIndCenter.y - northIndSize * 0.20f,
        game.config.aestheticRadar.highestColor
    );
    
    //Area name.
    Point areaNameSize(size.x * 0.40f, size.y * 0.08f);
    Point areaNameCenter(
        center.x + size.x / 2.0f - areaNameSize.x / 2.0f - decoPadding,
        center.y - size.y / 2.0f + areaNameSize.y / 2.0f + decoPadding
    );
    drawFilledRoundedRatioRectangle(
        areaNameCenter, areaNameSize,
        0.4f, game.config.aestheticRadar.backgroundColor
    );
    drawText(
        game.curAreaData->name, game.sysContent.fntStandard,
        areaNameCenter, areaNameSize * 0.60f,
        game.config.aestheticRadar.highestColor
    );
    
    //Draw some scan lines.
    float scanLineY = center.y - size.y / 2.0f;
    while(scanLineY < center.y + size.y / 2.0f) {
        al_draw_line(
            center.x - size.x / 2.0f,
            scanLineY,
            center.x + size.x / 2.0f,
            scanLineY,
            mapAlpha(8), 2.0f
        );
        scanLineY += 16.0f;
    }
    float scanLineX = center.x - size.x / 2.0f;
    while(scanLineX < center.x + size.x / 2.0f) {
        al_draw_line(
            scanLineX,
            center.y - size.y / 2.0f,
            scanLineX,
            center.y + size.y / 2.0f,
            mapAlpha(8), 2.0f
        );
        scanLineX += 16.0f;
    }
    
    //Draw a rectangle all around.
    drawTexturedBox(
        center, size, game.sysContent.bmpFrameBox,
        COLOR_TRANSPARENT_WHITE
    );
}


/**
 * @brief Fills the list of mission fail conditions.
 *
 * @param list List item to fill.
 */
void PauseMenu::fillMissionFailList(ListGuiItem* list) {
    for(size_t f = 0; f < game.missionFailConds.size(); f++) {
        if(
            hasFlag(
                game.curAreaData->missionOld.failConditions,
                getIdxBitmask(f)
            )
        ) {
            MissionFail* cond = game.missionFailConds[f];
            
            string description =
                cond->getPlayerDescription(&game.curAreaData->missionOld);
            addBullet(list, description, game.config.guiColors.bad);
            
            float percentage = 0.0f;
            int cur =
                cond->getCurAmount(game.states.gameplay);
            int req =
                cond->getReqAmount(game.states.gameplay);
            if(req != 0.0f) {
                percentage = cur / (float) req;
            }
            percentage *= 100;
            string status = cond->getStatus(cur, req, percentage);
            
            if(status.empty()) continue;
            addBullet(list, "    " + status);
        }
    }
    
    if(game.curAreaData->missionOld.failConditions == 0) {
        addBullet(list, "(None)");
    }
}


/**
 * @brief Fills the list of mission grading information.
 *
 * @param list List item to fill.
 */
void PauseMenu::fillMissionGradingList(ListGuiItem* list) {
    switch(game.curAreaData->missionOld.gradingMode) {
    case MISSION_GRADING_MODE_POINTS: {
        addBullet(
            list,
            "Your medal depends on your score:"
        );
        addBullet(
            list,
            "    Platinum: " +
            i2s(game.curAreaData->missionOld.platinumReq) + "+ points.",
            game.config.guiColors.gold
        );
        addBullet(
            list,
            "    Gold: " +
            i2s(game.curAreaData->missionOld.goldReq) + "+ points.",
            game.config.guiColors.gold
        );
        addBullet(
            list,
            "    Silver: " +
            i2s(game.curAreaData->missionOld.silverReq) + "+ points.",
            game.config.guiColors.gold
        );
        addBullet(
            list,
            "    Bronze: " +
            i2s(game.curAreaData->missionOld.bronzeReq) + "+ points.",
            game.config.guiColors.gold
        );
        
        vector<string> scoreNotes;
        for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
            MissionScoreCriterionOld* cPtr =
                game.missionScoreCriteria[c];
            int mult = cPtr->getMultiplier(&game.curAreaData->missionOld);
            if(mult != 0) {
                scoreNotes.push_back(
                    "    " + cPtr->getName() + " x " + i2s(mult) + "."
                );
            }
        }
        if(!scoreNotes.empty()) {
            addBullet(
                list,
                "Your score is calculated like so:"
            );
            for(size_t s = 0; s < scoreNotes.size(); s++) {
                addBullet(list, scoreNotes[s]);
            }
        } else {
            addBullet(
                list,
                "In this mission, your score will always be 0."
            );
        }
        
        vector<string> lossNotes;
        for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
            MissionScoreCriterionOld* cPtr =
                game.missionScoreCriteria[c];
            if(
                hasFlag(
                    game.curAreaData->missionOld.pointLossData,
                    getIdxBitmask(c)
                )
            ) {
                lossNotes.push_back("    " + cPtr->getName());
            }
        }
        if(!lossNotes.empty()) {
            addBullet(
                list,
                "If you fail, you'll lose your score for:"
            );
            for(size_t l = 0; l < lossNotes.size(); l++) {
                addBullet(list, lossNotes[l]);
            }
        }
        break;
    } case MISSION_GRADING_MODE_GOAL: {
        addBullet(
            list,
            "You get a platinum medal if you clear the goal."
        );
        addBullet(
            list,
            "You get no medal if you fail."
        );
        break;
    } case MISSION_GRADING_MODE_PARTICIPATION: {
        addBullet(
            list,
            "You get a platinum medal just by playing the mission."
        );
        break;
    }
    }
}


/**
 * @brief Returns a string representing the player's status towards the
 * mission goal.
 *
 * @return The status.
 */
string PauseMenu::getMissionGoalStatus() {
    float percentage = 0.0f;
    int cur =
        game.missionGoals[game.curAreaData->missionOld.goal]->
        getCurAmount(game.states.gameplay);
    int req =
        game.missionGoals[game.curAreaData->missionOld.goal]->
        getReqAmount(game.states.gameplay);
    if(req != 0.0f) {
        percentage = cur / (float) req;
    }
    percentage *= 100;
    return
        game.missionGoals[game.curAreaData->missionOld.goal]->
        getStatus(cur, req, percentage);
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void PauseMenu::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    if(!game.modal.isActive()) {
    
        radarView.updateMouseCursor(game.mouseCursor.winPos);
        
        gui.handleAllegroEvent(ev);
        radarGui.handleAllegroEvent(ev);
        statusGui.handleAllegroEvent(ev);
        missionGui.handleAllegroEvent(ev);
        if(secondaryMenu) secondaryMenu->handleAllegroEvent(ev);
        
        //Handle some radar logic.
        DrawInfo radarDraw;
        radarGui.getItemDrawInfo(radarItem, &radarDraw);
        bool mouseInRadar =
            radarGui.responsive &&
            isPointInRectangle(
                game.mouseCursor.winPos,
                radarDraw.center, radarDraw.size
            );
            
        if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if(mouseInRadar) {
                radarMouseDown = true;
                radarMouseDownPoint = game.mouseCursor.winPos;
            }
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            if(mouseInRadar && !radarMouseDragging) {
                //Clicked somewhere.
                radarConfirm();
            }
            
            radarMouseDown = false;
            radarMouseDragging = false;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(
                radarMouseDown &&
                (
                    fabs(game.mouseCursor.winPos.x - radarMouseDownPoint.x) >
                    4.0f ||
                    fabs(game.mouseCursor.winPos.y - radarMouseDownPoint.y) >
                    4.0f
                )
            ) {
                //Consider the mouse down as part of a mouse drag, not a click.
                radarMouseDragging = true;
            }
            
            if(
                radarMouseDragging &&
                (ev.mouse.dx != 0.0f || ev.mouse.dy != 0.0f)
            ) {
                //Pan the radar around.
                panRadar(Point(-ev.mouse.dx, -ev.mouse.dy));
                
            } else if(
                mouseInRadar && ev.mouse.dz != 0.0f
            ) {
                //Zoom in or out, using the radar/mouse cursor as the anchor.
                zoomRadarWithMouse(
                    ev.mouse.dz * 0.1f, radarDraw.center, radarDraw.size
                );
                
            }
        }
        
    }
    
    game.modal.handleAllegroEvent(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void PauseMenu::handlePlayerAction(const Inpution::Action& action) {
    if(openingLockoutTimer > 0.0f) {
        //Don't accept inputs shortly after the menu opens.
        //This helps errant inputs from before the menu bleeding into the menu
        //immediately after it opens, like the "radar toggle" action.
        return;
    }
    if(closing) return;
    
    if(!game.modal.isActive()) {
    
        bool handledByRadar = false;
        
        if(radarGui.responsive) {
            switch(action.actionTypeId) {
            case PLAYER_ACTION_TYPE_RADAR: {
                if(action.value >= 0.5f) {
                    game.audio.createUiSoundSource(
                        game.sysContent.sndMenuBack, { .volume = 0.75f }
                    );
                    startClosing(&radarGui);
                    handledByRadar = true;
                }
                break;
            } case PLAYER_ACTION_TYPE_RADAR_RIGHT: {
                radarPan.right = action.value;
                handledByRadar = true;
                break;
            } case PLAYER_ACTION_TYPE_RADAR_DOWN: {
                radarPan.down = action.value;
                handledByRadar = true;
                break;
            } case PLAYER_ACTION_TYPE_RADAR_LEFT: {
                radarPan.left = action.value;
                handledByRadar = true;
                break;
            } case PLAYER_ACTION_TYPE_RADAR_UP: {
                radarPan.up = action.value;
                handledByRadar = true;
                break;
            } case PLAYER_ACTION_TYPE_RADAR_ZOOM_IN: {
                radarZoom.up = action.value;
                handledByRadar = true;
                break;
            } case PLAYER_ACTION_TYPE_RADAR_ZOOM_OUT: {
                radarZoom.down = action.value;
                handledByRadar = true;
                break;
            } case PLAYER_ACTION_TYPE_MENU_OK: {
                radarConfirm();
                handledByRadar = true;
                break;
            } case PLAYER_ACTION_TYPE_MENU_RIGHT: {
                if(
                    game.controls.actionQueueContains(
                        PLAYER_ACTION_TYPE_RADAR_RIGHT
                    )
                ) {
                    handledByRadar = true;
                } else if(
                    hasFlag(action.flags, Inpution::ACTION_FLAG_REPEAT) &&
                    radarPan.right > 0.0f
                ) {
                    handledByRadar = true;
                }
                break;
            } case PLAYER_ACTION_TYPE_MENU_DOWN: {
                if(
                    game.controls.actionQueueContains(
                        PLAYER_ACTION_TYPE_RADAR_DOWN
                    )
                ) {
                    handledByRadar = true;
                } else if(
                    hasFlag(action.flags, Inpution::ACTION_FLAG_REPEAT) &&
                    radarPan.down > 0.0f
                ) {
                    handledByRadar = true;
                }
                break;
            } case PLAYER_ACTION_TYPE_MENU_LEFT: {
                if(
                    game.controls.actionQueueContains(
                        PLAYER_ACTION_TYPE_RADAR_LEFT
                    )
                ) {
                    handledByRadar = true;
                } else if(
                    hasFlag(action.flags, Inpution::ACTION_FLAG_REPEAT) &&
                    radarPan.left > 0.0f
                ) {
                    handledByRadar = true;
                }
                break;
            } case PLAYER_ACTION_TYPE_MENU_UP: {
                if(
                    game.controls.actionQueueContains(
                        PLAYER_ACTION_TYPE_RADAR_UP
                    )
                ) {
                    handledByRadar = true;
                } else if(
                    hasFlag(action.flags, Inpution::ACTION_FLAG_REPEAT) &&
                    radarPan.up > 0.0f
                ) {
                    handledByRadar = true;
                }
                break;
            }
            }
        }
        
        if(!handledByRadar) {
            //Only let the GUIs handle it if the radar didn't need it, otherwise
            //we could see the GUI item focus move around or such because
            //radar and menus actions share binds.
            gui.handlePlayerAction(action);
            radarGui.handlePlayerAction(action);
            statusGui.handlePlayerAction(action);
            missionGui.handlePlayerAction(action);
            if(secondaryMenu) secondaryMenu->handlePlayerAction(action);
            
            switch(action.actionTypeId) {
            case PLAYER_ACTION_TYPE_MENU_PAGE_LEFT:
            case PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT: {
                if(action.value >= 0.5f) {
                    GuiManager* curGui = &gui;
                    if(radarGui.responsive) {
                        curGui = &radarGui;
                    } else if(statusGui.responsive) {
                        curGui = &statusGui;
                    } else if(missionGui.responsive) {
                        curGui = &missionGui;
                    }
                    
                    if(curGui->responsive) {
                        map<GuiManager*, ButtonGuiItem*>* m =
                            action.actionTypeId ==
                            PLAYER_ACTION_TYPE_MENU_PAGE_LEFT ?
                            &leftPageButtons :
                            &rightPageButtons;
                        (*m)[curGui]->activate();
                    }
                }
                break;
            }
            }
        }
        
    }
    
    game.modal.handlePlayerAction(action);
    
}


/**
 * @brief Initializes the pause menu's main menu.
 */
void PauseMenu::initMainPauseMenu() {
    //Menu items.
    DataNode* guiFile = &game.content.guiDefs.list[PAUSE_MENU::GUI_FILE_NAME];
    gui.registerCoords("left_page",        12,    5, 20,  6);
    gui.registerCoords("left_page_input",   3,    7,  4,  4);
    gui.registerCoords("right_page",       88,    5, 20,  6);
    gui.registerCoords("right_page_input", 97,    7,  4,  4);
    gui.registerCoords("area_name",        50,   20, 96,  8);
    gui.registerCoords("area_subtitle",    50, 28.5, 88,  9);
    gui.registerCoords("continue",         13,   88, 22,  8);
    gui.registerCoords("continue_input",    3,   91,  4,  4);
    gui.registerCoords("retry",            28,   44, 38, 12);
    gui.registerCoords("end",              72,   44, 38, 12);
    gui.registerCoords("help",             19,   65, 26, 10);
    gui.registerCoords("options",          50,   65, 26, 10);
    gui.registerCoords("stats",            81,   65, 26, 10);
    gui.registerCoords("quit",             87,   88, 22,  8);
    gui.registerCoords("tooltip",          50,   96, 96,  4);
    gui.readDataFile(guiFile);
    
    //Page buttons and inputs.
    createPageButtons(PAUSE_MENU_PAGE_SYSTEM, &gui);
    
    //Area name.
    TextGuiItem* areaNameText =
        new TextGuiItem(
        game.curAreaData->name, game.sysContent.fntAreaName,
        changeAlpha(game.config.guiColors.gold, 192)
    );
    gui.addItem(areaNameText, "area_name");
    
    //Area subtitle.
    TextGuiItem* areaSubtitleText =
        new TextGuiItem(
        calculateAreaSubtitle(
            game.curAreaData->subtitle, game.curAreaData->type,
            game.curAreaData->mission.preset
        ),
        game.sysContent.fntAreaName,
        changeAlpha(COLOR_WHITE, 192)
    );
    gui.addItem(areaSubtitleText, "area_subtitle");
    
    //Continue button.
    gui.backItem =
        new ButtonGuiItem(
        "Continue", game.sysContent.fntStandard, game.config.guiColors.back
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
        startClosing(&gui);
    };
    gui.backItem->onGetTooltip =
    [] () { return "Unpause and continue playing."; };
    gui.addItem(gui.backItem, "continue");
    
    //Continue input icon.
    guiAddBackInputIcon(&gui, "continue_input");
    
    //Retry button.
    ButtonGuiItem* retryButton =
        new ButtonGuiItem(
        game.curAreaData->type == AREA_TYPE_SIMPLE ?
        "Restart exploration" :
        "Retry mission",
        game.sysContent.fntStandard
    );
    retryButton->onActivate =
    [this] (const Point&) {
        leaveTarget = GAMEPLAY_LEAVE_TARGET_RETRY;
        confirmOrLeave();
    };
    retryButton->onGetTooltip =
    [] () {
        return
            game.curAreaData->type == AREA_TYPE_SIMPLE ?
            "Restart this area's exploration." :
            "Retry the mission from the start.";
    };
    gui.addItem(retryButton, "retry");
    
    //End button.
    ButtonGuiItem* endButton =
        new ButtonGuiItem(
        game.curAreaData->type == AREA_TYPE_SIMPLE ?
        "End exploration" :
        "End mission",
        game.sysContent.fntStandard
    );
    endButton->onActivate =
    [this] (const Point&) {
        leaveTarget = GAMEPLAY_LEAVE_TARGET_END;
        confirmOrLeave();
    };
    endButton->onGetTooltip =
    [] () {
        bool asFail =
            hasFlag(
                game.curAreaData->missionOld.failConditions,
                getIdxBitmask(MISSION_FAIL_COND_PAUSE_MENU)
            );
        return
            game.curAreaData->type == AREA_TYPE_SIMPLE ?
            "End this area's exploration." :
            asFail ?
            "End this mission as a fail." :
            "End this mission successfully.";
    };
    gui.addItem(endButton, "end");
    
    //Help button.
    ButtonGuiItem* helpButton =
        new ButtonGuiItem("Help", game.sysContent.fntStandard);
    helpButton->onActivate =
    [this] (const Point&) {
        HelpMenu* helpMenu = new HelpMenu();
        helpMenu->leaveCallback = [this, helpMenu] () {
            helpMenu->unloadTimer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
            transitionGuis(
                helpMenu->gui, gui, GUI_MANAGER_ANIM_CENTER_TO_DOWN,
                GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
            );
        };
        helpMenu->load();
        helpMenu->enter();
        secondaryMenu = helpMenu;
        transitionGuis(
            gui, helpMenu->gui, GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    helpButton->onGetTooltip =
    [] () {
        return
            "Quick help and tips about how to play. "
            "You can also find this in the title screen.";
    };
    gui.addItem(helpButton, "help");
    
    //Options button.
    ButtonGuiItem* optionsButton =
        new ButtonGuiItem("Options", game.sysContent.fntStandard);
    optionsButton->onActivate =
    [this] (const Point&) {
        OptionsMenu* optionsMenu = new OptionsMenu();
        optionsMenu->leaveCallback = [this, optionsMenu] () {
            optionsMenu->unloadTimer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
            transitionGuis(
                optionsMenu->topGui, gui, GUI_MANAGER_ANIM_CENTER_TO_DOWN,
                GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
            );
        };
        optionsMenu->load();
        optionsMenu->enter();
        secondaryMenu = optionsMenu;
        transitionGuis(
            gui, optionsMenu->topGui, GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    optionsButton->onGetTooltip =
    [] () {
        return
            "Customize your playing experience. "
            "You can also find this in the title screen.";
    };
    gui.addItem(optionsButton, "options");
    
    //Statistics button.
    ButtonGuiItem* statsButton =
        new ButtonGuiItem("Statistics", game.sysContent.fntStandard);
    statsButton->onActivate =
    [this] (const Point&) {
        StatsMenu* statsMenu = new StatsMenu();
        statsMenu->leaveCallback = [this, statsMenu] () {
            statsMenu->unloadTimer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
            transitionGuis(
                statsMenu->gui, gui, GUI_MANAGER_ANIM_CENTER_TO_DOWN,
                GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
            );
        };
        statsMenu->load();
        statsMenu->enter();
        secondaryMenu = statsMenu;
        transitionGuis(
            gui, statsMenu->gui, GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    statsButton->onGetTooltip =
    [] () {
        return
            "Check out some fun lifetime statistics. "
            "You can also find this in the title screen.";
    };
    gui.addItem(statsButton, "stats");
    
    //Quit button.
    ButtonGuiItem* quitButton =
        new ButtonGuiItem(
        game.quickPlay.areaPath.empty() ?
        "Quit" :
        "Back to editor",
        game.sysContent.fntStandard, game.config.guiColors.bad
    );
    quitButton->onActivate =
    [this] (const Point&) {
        leaveTarget = GAMEPLAY_LEAVE_TARGET_AREA_SELECT;
        confirmOrLeave();
    };
    quitButton->onGetTooltip =
    [] () {
        return
            "Lose your progress and return to the " +
            string(
                game.quickPlay.areaPath.empty() ?
                "area selection menu" :
                "editor"
            ) + ".";
    };
    gui.addItem(quitButton, "quit");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    gui.setFocusedItem(gui.backItem, true);
    gui.responsive = false;
    gui.hideItems();
}


/**
 * @brief Initializes the mission page.
 */
void PauseMenu::initMissionPage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[PAUSE_MENU::MISSION_GUI_FILE_NAME];
        
    //Menu items.
    missionGui.registerCoords("left_page",        12,  5, 20,  6);
    missionGui.registerCoords("left_page_input",   3,  7,  4,  4);
    missionGui.registerCoords("right_page",       88,  5, 20,  6);
    missionGui.registerCoords("right_page_input", 97,  7,  4,  4);
    missionGui.registerCoords("continue",         10, 16, 16,  4);
    missionGui.registerCoords("continue_input",    3, 17,  4,  4);
    missionGui.registerCoords("goal_header",      50, 16, 60,  4);
    missionGui.registerCoords("goal",             50, 22, 96,  4);
    missionGui.registerCoords("goal_status",      50, 26, 96,  4);
    missionGui.registerCoords("fail_header",      50, 32, 96,  4);
    missionGui.registerCoords("fail_list",        48, 48, 92, 24);
    missionGui.registerCoords("fail_scroll",      97, 48,  2, 24);
    missionGui.registerCoords("grading_header",   50, 64, 96,  4);
    missionGui.registerCoords("grading_list",     48, 80, 92, 24);
    missionGui.registerCoords("grading_scroll",   97, 80,  2, 24);
    missionGui.registerCoords("tooltip",          50, 96, 96,  4);
    missionGui.readDataFile(guiFile);
    
    //Page buttons and inputs.
    createPageButtons(PAUSE_MENU_PAGE_MISSION, &missionGui);
    
    //Continue button.
    missionGui.backItem =
        new ButtonGuiItem(
        "Continue", game.sysContent.fntStandard, game.config.guiColors.back
    );
    missionGui.backItem->onActivate =
    [this] (const Point&) {
        startClosing(&missionGui);
    };
    missionGui.backItem->onGetTooltip =
    [] () { return "Unpause and continue playing."; };
    missionGui.addItem(missionGui.backItem, "continue");
    
    //Continue input icon.
    guiAddBackInputIcon(&missionGui, "continue_input");
    
    //Goal header text.
    TextGuiItem* goalHeaderText =
        new TextGuiItem(
        "Goal", game.sysContent.fntAreaName,
        game.config.guiColors.smallHeader
    );
    missionGui.addItem(goalHeaderText, "goal_header");
    
    //Goal explanation text.
    TextGuiItem* goalText =
        new TextGuiItem(
        game.missionGoals[game.curAreaData->missionOld.goal]->
        getPlayerDescription(&game.curAreaData->missionOld),
        game.sysContent.fntStandard,
        game.config.guiColors.gold
    );
    missionGui.addItem(goalText, "goal");
    
    //Goal status text.
    TextGuiItem* goalStatusText =
        new TextGuiItem(
        getMissionGoalStatus(),
        game.sysContent.fntStandard
    );
    missionGui.addItem(goalStatusText, "goal_status");
    
    //Fail conditions header text.
    TextGuiItem* failHeaderText =
        new TextGuiItem(
        "Fail conditions", game.sysContent.fntAreaName,
        game.config.guiColors.smallHeader
    );
    missionGui.addItem(failHeaderText, "fail_header");
    
    //Fail condition explanation list.
    ListGuiItem* missionFailList = new ListGuiItem();
    missionGui.addItem(missionFailList, "fail_list");
    fillMissionFailList(missionFailList);
    
    //Fail condition explanation scrollbar.
    ScrollGuiItem* failScroll = new ScrollGuiItem();
    failScroll->listItem = missionFailList;
    missionGui.addItem(failScroll, "fail_scroll");
    
    //Grading header text.
    TextGuiItem* gradingHeaderText =
        new TextGuiItem(
        "Grading", game.sysContent.fntAreaName,
        game.config.guiColors.smallHeader
    );
    missionGui.addItem(gradingHeaderText, "grading_header");
    
    //Grading explanation list.
    ListGuiItem* missionGradingList = new ListGuiItem();
    missionGui.addItem(missionGradingList, "grading_list");
    fillMissionGradingList(missionGradingList);
    
    //Grading explanation scrollbar.
    ScrollGuiItem* gradingScroll = new ScrollGuiItem();
    gradingScroll->listItem = missionGradingList;
    missionGui.addItem(gradingScroll, "grading_scroll");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&missionGui);
    missionGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    missionGui.setFocusedItem(missionGui.backItem, true);
    missionGui.responsive = false;
    missionGui.hideItems();
}


/**
 * @brief Initializes the radar page.
 */
void PauseMenu::initRadarPage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[PAUSE_MENU::RADAR_GUI_FILE_NAME];
        
    //Assets.
    DataNode* bitmapsNode = guiFile->getChildByName("bitmaps");
    
#define loader(var, name) \
    var = \
    game.content.bitmaps.list.get( \
        bitmapsNode->getChildByName(name)->value, \
        bitmapsNode->getChildByName(name) \
    );
    
    loader(bmpRadarCursor,        "cursor");
    loader(bmpRadarPikmin,        "pikmin");
    loader(bmpRadarTreasure,      "treasure");
    loader(bmpRadarEnemyAlive,    "enemy_alive");
    loader(bmpRadarEnemyDead,     "enemy_dead");
    loader(bmpRadarLeaderBubble,  "leader_bubble");
    loader(bmpRadarLeaderX,       "leader_x");
    loader(bmpRadarObstacle,      "obstacle");
    loader(bmpRadarOnionSkeleton, "onion_skeleton");
    loader(bmpRadarOnionBulb,     "onion_bulb");
    loader(bmpRadarShip,          "ship");
    loader(bmpRadarPath,          "path");
    
#undef loader
    
    //Menu items.
    radarGui.registerCoords("left_page",           12,     5,    20,    6);
    radarGui.registerCoords("left_page_input",      3,     7,     4,    4);
    radarGui.registerCoords("right_page",          88,     5,    20,    6);
    radarGui.registerCoords("right_page_input",    97,     7,     4,    4);
    radarGui.registerCoords("continue",            10,    16,    16,    4);
    radarGui.registerCoords("continue_input",       3,    17,     4,    4);
    radarGui.registerCoords("radar",               37.5,  56.25, 70,   72.5);
    radarGui.registerCoords("group_pikmin_label",  86.25, 77.5,  22.5,  5);
    radarGui.registerCoords("group_pikmin_number", 86.25, 85,    22.5,  5);
    radarGui.registerCoords("idle_pikmin_label",   86.25, 62.5,  22.5,  5);
    radarGui.registerCoords("idle_pikmin_number",  86.25, 70,    22.5,  5);
    radarGui.registerCoords("field_pikmin_label",  86.25, 47.5,  22.5,  5);
    radarGui.registerCoords("field_pikmin_number", 86.25, 55,    22.5,  5);
    radarGui.registerCoords("cursor_info",         86.25, 33.75, 22.5, 17.5);
    radarGui.registerCoords("instructions",        58.75, 16,    77.5,  4);
    radarGui.registerCoords("tooltip",             50,    96,    96,    4);
    radarGui.readDataFile(guiFile);
    
    //Page buttons and inputs.
    createPageButtons(PAUSE_MENU_PAGE_RADAR, &radarGui);
    
    //Continue button.
    radarGui.backItem =
        new ButtonGuiItem(
        "Continue", game.sysContent.fntStandard, game.config.guiColors.back
    );
    radarGui.backItem->onActivate =
    [this] (const Point&) {
        startClosing(&radarGui);
    };
    radarGui.backItem->onGetTooltip =
    [] () { return "Unpause and continue playing."; };
    radarGui.addItem(radarGui.backItem, "continue");
    
    //Continue input icon.
    guiAddBackInputIcon(&radarGui, "continue_input");
    
    //Radar item.
    radarItem = new GuiItem();
    radarItem->onDraw =
    [this] (const DrawInfo & draw) {
        drawRadar(draw.center, draw.size);
    };
    radarGui.addItem(radarItem, "radar");
    
    //Group Pikmin label text.
    TextGuiItem* groupPikLabelText =
        new TextGuiItem(
        "Group Pikmin:", game.sysContent.fntStandard,
        COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    radarGui.addItem(groupPikLabelText, "group_pikmin_label");
    
    //Group Pikmin number text.
    TextGuiItem* groupPikNrText =
        new TextGuiItem(
        i2s(
            game.states.gameplay->getAmountOfGroupPikmin(
                &game.states.gameplay->players[0]
            )
        ),
        game.sysContent.fntCounter,
        COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radarGui.addItem(groupPikNrText, "group_pikmin_number");
    
    //Idle Pikmin label text.
    TextGuiItem* idlePikLabelText =
        new TextGuiItem(
        "Idle Pikmin:", game.sysContent.fntStandard,
        COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    radarGui.addItem(idlePikLabelText, "idle_pikmin_label");
    
    //Idle Pikmin number text.
    TextGuiItem* idlePikNrText =
        new TextGuiItem(
        i2s(game.states.gameplay->getAmountOfIdlePikmin()),
        game.sysContent.fntCounter,
        COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radarGui.addItem(idlePikNrText, "idle_pikmin_number");
    
    //Field Pikmin label text.
    TextGuiItem* fieldPikLabelText =
        new TextGuiItem(
        "Field Pikmin:", game.sysContent.fntStandard,
        COLOR_WHITE, ALLEGRO_ALIGN_LEFT
    );
    radarGui.addItem(fieldPikLabelText, "field_pikmin_label");
    
    //Field Pikmin number text.
    TextGuiItem* fieldPikNrText =
        new TextGuiItem(
        i2s(game.states.gameplay->getAmountOfFieldPikmin()),
        game.sysContent.fntCounter, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    radarGui.addItem(fieldPikNrText, "field_pikmin_number");
    
    //Cursor info text.
    TextGuiItem* cursorInfoText =
        new TextGuiItem("", game.sysContent.fntStandard);
    cursorInfoText->lineWrap = true;
    cursorInfoText->onDraw =
    [this, cursorInfoText] (const DrawInfo & draw) {
        if(cursorInfoText->text.empty()) return;
        
        //Draw the text.
        int lineHeight = al_get_font_line_height(cursorInfoText->font);
        vector<StringToken> tokens = tokenizeString(cursorInfoText->text);
        setStringTokenWidths(
            tokens, game.sysContent.fntStandard,
            game.sysContent.fntSlim, lineHeight, false
        );
        vector<vector<StringToken> > tokensPerLine =
            splitLongStringWithTokens(tokens, draw.size.x);
        float textH = tokensPerLine.size() * lineHeight;
        
        for(size_t l = 0; l < tokensPerLine.size(); l++) {
            drawStringTokens(
                tokensPerLine[l], game.sysContent.fntStandard,
                game.sysContent.fntSlim,
                false,
                Point(
                    draw.center.x,
                    draw.center.y - textH / 2.0f + l * lineHeight
                ),
                cursorInfoText->flags,
                Point(draw.size.x, lineHeight), Point(1.0f), draw.tint
            );
        }
        
        //Draw a box around it.
        drawTexturedBox(
            draw.center, draw.size, game.sysContent.bmpFrameBox,
            tintColor(COLOR_TRANSPARENT_WHITE, draw.tint)
        );
        
        //Draw a connection from here to the radar cursor.
        Point lineAnchor(
            draw.center.x - draw.size.x / 2.0f - 16.0f, draw.center.y
        );
        Point cursorWindowPos = radarCursor;
        al_transform_coordinates(
            &radarView.worldToWindowTransform,
            &cursorWindowPos.x, &cursorWindowPos.y
        );
        
        al_draw_line(
            draw.center.x - draw.size.x / 2.0f, draw.center.y,
            lineAnchor.x, lineAnchor.y,
            COLOR_TRANSPARENT_WHITE, 2.0f
        );
        
        cursorWindowPos =
            cursorWindowPos +
            rotatePoint(
                Point(24.0f, 0.0f),
                getAngle(cursorWindowPos, lineAnchor)
            );
        al_draw_line(
            lineAnchor.x, lineAnchor.y,
            cursorWindowPos.x, cursorWindowPos.y,
            COLOR_TRANSPARENT_WHITE, 2.0f
        );
    };
    cursorInfoText->onTick =
    [this, cursorInfoText] (float deltaT) {
        if(radarCursorLeader) {
            cursorInfoText->text =
                (
                    radarCursorLeader == radarSelectedLeader ?
                    "" :
                    "\\k menu_ok \\k "
                ) + radarCursorLeader->type->name;
        } else if(
            radarSelectedLeader &&
            !radarSelectedLeader->fsm.getEvent(LEADER_EV_GO_HERE)
        ) {
            cursorInfoText->text =
                "Can't go here... Leader is busy!";
            cursorInfoText->color = COLOR_WHITE;
        } else {
            switch(goHerePathResult) {
            case PATH_RESULT_DIRECT:
            case PATH_RESULT_DIRECT_NO_STOPS:
            case PATH_RESULT_DIRECT_NO_ACCESSIBLE_STOPS:
            case PATH_RESULT_NORMAL_PATH:
            case PATH_RESULT_PATH_WITH_SINGLE_STOP: {
                cursorInfoText->text = "\\k menu_ok \\k Go here!";
                cursorInfoText->color = game.config.guiColors.gold;
                break;
            } case PATH_RESULT_PATH_WITH_OBSTACLES: {
                cursorInfoText->text = "Can't go here... Path blocked!";
                cursorInfoText->color = COLOR_WHITE;
                break;
            } case PATH_RESULT_END_STOP_UNREACHABLE: {
                cursorInfoText->text = "Can't go here...";
                cursorInfoText->color = COLOR_WHITE;
                break;
            } default: {
                cursorInfoText->text.clear();
                cursorInfoText->color = COLOR_WHITE;
                break;
            }
            }
        }
    };
    radarGui.addItem(cursorInfoText, "cursor_info");
    
    //Instructions text.
    TextGuiItem* instructionsText =
        new TextGuiItem(
        "\\k menu_radar_up \\k"
        "\\k menu_radar_left \\k"
        "\\k menu_radar_down \\k"
        "\\k menu_radar_right \\k Pan   "
        "\\k menu_radar_zoom_in \\k"
        "\\k menu_radar_zoom_out \\k Zoom",
        game.sysContent.fntSlim,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    instructionsText->lineWrap = true;
    radarGui.addItem(instructionsText, "instructions");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&radarGui);
    radarGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    radarGui.setFocusedItem(nullptr);
    radarGui.responsive = false;
    radarGui.hideItems();
}


/**
 * @brief Initializes the status page.
 */
void PauseMenu::initStatusPage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[PAUSE_MENU::STATUS_GUI_FILE_NAME];
        
    //Menu items.
    statusGui.registerCoords("left_page",        12,     5,   20,    6);
    statusGui.registerCoords("left_page_input",   3,     7,    4,    4);
    statusGui.registerCoords("right_page",       88,     5,   20,    6);
    statusGui.registerCoords("right_page_input", 97,     7,    4,    4);
    statusGui.registerCoords("continue",         10,    16,   16,    4);
    statusGui.registerCoords("continue_input",    3,    17,    4,    4);
    statusGui.registerCoords("list_header",      50,    23.5, 88,    7);
    statusGui.registerCoords("list",             50,    56,   88,   56);
    statusGui.registerCoords("list_scroll",      97,    56,    2,   56);
    statusGui.registerCoords("totals",           50,    89,   88,    8);
    statusGui.registerCoords("tooltip",          50,    96,   96,    4);
    statusGui.readDataFile(guiFile);
    
    //Page buttons and inputs.
    createPageButtons(PAUSE_MENU_PAGE_STATUS, &statusGui);
    
    //Continue button.
    statusGui.backItem =
        new ButtonGuiItem(
        "Continue", game.sysContent.fntStandard, game.config.guiColors.back
    );
    statusGui.backItem->onActivate =
    [this] (const Point&) {
        startClosing(&statusGui);
    };
    statusGui.backItem->onGetTooltip =
    [] () { return "Unpause and continue playing."; };
    statusGui.addItem(statusGui.backItem, "continue");
    
    //Continue input icon.
    guiAddBackInputIcon(&statusGui, "continue_input");
    
    //Pikmin list header box.
    ListGuiItem* listHeader = new ListGuiItem();
    listHeader->onDraw =
    [] (const DrawInfo&) {};
    statusGui.addItem(listHeader, "list_header");
    
    //Pikmin list box.
    pikminList = new ListGuiItem();
    statusGui.addItem(pikminList, "list");
    
    //Pikmin list scrollbar.
    ScrollGuiItem* listScroll = new ScrollGuiItem();
    listScroll->listItem = pikminList;
    statusGui.addItem(listScroll, "list_scroll");
    
    //Pikmin totals box.
    ListGuiItem* totals = new ListGuiItem();
    totals->onDraw =
    [] (const DrawInfo&) {};
    statusGui.addItem(totals, "totals");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&statusGui);
    statusGui.addItem(tooltipText, "tooltip");
    
    //Setup the list header.
    addPikminStatusLine(
        listHeader,
        nullptr,
        "Group",
        "Idle",
        "Field",
        "Onion",
        "Total",
        "New",
        "Lost",
        true, false
    );
    
    size_t totalInGroup = 0;
    size_t totalIdling = 0;
    size_t totalOnField = 0;
    long totalInOnion = 0;
    long grandTotal = 0;
    long totalNew = 0;
    long totalLost = 0;
    
    //Setup the list rows.
    for(size_t p = 0; p < game.config.pikmin.order.size(); p++) {
        PikminType* ptPtr = game.config.pikmin.order[p];
        
        size_t inGroup =
            game.states.gameplay->getAmountOfGroupPikmin(
                &game.states.gameplay->players[0], ptPtr
            );
        size_t idling =
            game.states.gameplay->getAmountOfIdlePikmin(ptPtr);
        size_t onField =
            game.states.gameplay->getAmountOfFieldPikmin(ptPtr);
        long inOnion =
            game.states.gameplay->getAmountOfOnionPikmin(ptPtr);
        long total = (long) onField + inOnion;
        
        long newPiks = 0;
        auto newIt =
            game.states.gameplay->pikminBornPerType.find(ptPtr);
        if(newIt != game.states.gameplay->pikminBornPerType.end()) {
            newPiks = newIt->second;
        }
        long lost = 0;
        auto lostIt =
            game.states.gameplay->pikminDeathsPerType.find(ptPtr);
        if(lostIt != game.states.gameplay->pikminDeathsPerType.end()) {
            lost = lostIt->second;
        }
        
        if(total + newPiks + lost > 0) {
            addPikminStatusLine(
                pikminList,
                ptPtr,
                i2s(inGroup),
                i2s(idling),
                i2s(onField),
                i2s(inOnion),
                i2s(total),
                i2s(newPiks),
                i2s(lost),
                false, false
            );
        }
        
        totalInGroup += inGroup;
        totalIdling += idling;
        totalOnField += onField;
        totalInOnion += inOnion;
        grandTotal += total;
        totalNew += newPiks;
        totalLost += lost;
    }
    
    //Setup the list totals.
    addPikminStatusLine(
        totals,
        nullptr,
        i2s(totalInGroup),
        i2s(totalIdling),
        i2s(totalOnField),
        i2s(totalInOnion),
        i2s(grandTotal),
        i2s(totalNew),
        i2s(totalLost),
        true, true
    );
    
    //Finishing touches.
    statusGui.setFocusedItem(statusGui.backItem, true);
    statusGui.responsive = false;
    statusGui.hideItems();
}


/**
 * @brief Pans the radar by an amount.
 *
 * @param amount How much to pan by.
 */
void PauseMenu::panRadar(Point amount) {
    Point delta = amount / radarView.cam.zoom;
    radarView.cam.pos += delta;
    radarView.cam.pos.x =
        std::clamp(radarView.cam.pos.x, radarMinCoords.x, radarMaxCoords.x);
    radarView.cam.pos.y =
        std::clamp(radarView.cam.pos.y, radarMinCoords.y, radarMaxCoords.y);
}


/**
 * @brief When the player confirms their action in the radar.
 */
void PauseMenu::radarConfirm() {
    calculateGoHerePath();
    
    if(radarCursorLeader) {
        //Select a leader.
        radarSelectedLeader = radarCursorLeader;
        
    } else if(
        goHerePathResult == PATH_RESULT_DIRECT ||
        goHerePathResult == PATH_RESULT_DIRECT_NO_STOPS ||
        goHerePathResult == PATH_RESULT_DIRECT_NO_ACCESSIBLE_STOPS ||
        goHerePathResult == PATH_RESULT_NORMAL_PATH ||
        goHerePathResult == PATH_RESULT_PATH_WITH_SINGLE_STOP
    ) {
        //Start Go Here.
        radarSelectedLeader->fsm.runEvent(
            LEADER_EV_GO_HERE, (void*) &radarCursor
        );
        startClosing(&radarGui);
        
    }
}


/**
 * @brief Starts the closing process.
 *
 * @param curGui The currently active GUI manager.
 */
void PauseMenu::startClosing(GuiManager* curGui) {
    curGui->responsive = false;
    curGui->startAnimation(
        GUI_MANAGER_ANIM_CENTER_TO_UP,
        GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
    game.states.gameplay->players[0].hud->gui.startAnimation(
        GUI_MANAGER_ANIM_OUT_TO_IN,
        GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
    closing = true;
    closingTimer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
    
    game.states.gameplay->players[0].radarZoom = radarView.cam.zoom;
}


/**
 * @brief Starts the process of leaving the gameplay state.
 */
void PauseMenu::startLeavingGameplay() {
    if(
        leaveTarget == GAMEPLAY_LEAVE_TARGET_END &&
        game.curAreaData->type == AREA_TYPE_MISSION
    ) {
        bool missionEndsInClear = false;
        for(size_t e = 0; e < game.curAreaData->mission.events.size(); e++) {
            MissionEvent* ePtr = &game.curAreaData->mission.events[e];
            if(ePtr->type != MISSION_EV_PAUSE_MENU_END) continue;
            if(ePtr->actionType == MISSION_ACTION_END_CLEAR) {
                missionEndsInClear = true;
                break;
            } else if(ePtr->actionType == MISSION_ACTION_END_FAIL) {
                missionEndsInClear = false;
                break;
            } else {
                MissionActionType* actionType =
                    game.missionActionTypes[ePtr->actionType];
                actionType->run(ePtr, game.states.gameplay);
            }
        }
        game.states.gameplay->missionWasCleared = missionEndsInClear;
    }
    game.states.gameplay->startLeaving(leaveTarget);
}


/**
 * @brief Switches pages in the pause menu.
 *
 * @param curGui Pointer to the current page's GUI manager.
 * @param newPage The new page to switch to.
 * @param left Is the new page to the left of the current one, or the right?
 */
void PauseMenu::switchPage(
    GuiManager* curGui, PAUSE_MENU_PAGE newPage, bool left
) {
    GuiManager* newGui = nullptr;
    switch(newPage) {
    case PAUSE_MENU_PAGE_SYSTEM: {
        newGui = &gui;
        break;
    } case PAUSE_MENU_PAGE_RADAR: {
        newGui = &radarGui;
        break;
    } case PAUSE_MENU_PAGE_STATUS: {
        newGui = &statusGui;
        break;
    } case PAUSE_MENU_PAGE_MISSION: {
        newGui = &missionGui;
        break;
    }
    }
    
    transitionGuis(
        *curGui, *newGui,
        left ?
        GUI_MANAGER_ANIM_CENTER_TO_RIGHT :
        GUI_MANAGER_ANIM_CENTER_TO_LEFT,
        GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void PauseMenu::tick(float deltaT) {
    //Tick the GUI.
    gui.tick(deltaT);
    radarGui.tick(deltaT);
    statusGui.tick(deltaT);
    missionGui.tick(deltaT);
    
    if(secondaryMenu) {
        if(secondaryMenu->loaded) {
            secondaryMenu->tick(game.deltaT);
        }
        if(!secondaryMenu->loaded) {
            delete secondaryMenu;
            secondaryMenu = nullptr;
        }
    }
    
    game.modal.tick(deltaT);
    
    //Tick the background.
    const float bgAlphaMultSpeed =
        1.0f / GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME;
    const float diff =
        closing ? -bgAlphaMultSpeed : bgAlphaMultSpeed;
    bgAlphaMult = std::clamp(bgAlphaMult + diff * deltaT, 0.0f, 1.0f);
    
    //Tick the menu opening and closing.
    if(openingLockoutTimer > 0.0f) {
        openingLockoutTimer -= deltaT;
    }
    if(closing) {
        closingTimer -= deltaT;
        if(closingTimer <= 0.0f) {
            toDelete = true;
        }
    }
    
    //Tick radar things.
    DrawInfo radarDraw;
    radarGui.getItemDrawInfo(radarItem, &radarDraw);
    radarView.center = radarDraw.center;
    radarView.size = radarDraw.size;
    radarView.updateTransformations();
    
    if(radarGui.responsive) {
    
        Point radarMovCoords;
        float dummyAngle;
        float dummyMagnitude;
        radarPan.getInfo(&radarMovCoords, &dummyAngle, &dummyMagnitude);
        if(radarMovCoords.x != 0.0f || radarMovCoords.y != 0.0f) {
            panRadar(radarMovCoords * PAUSE_MENU::RADAR_PAN_SPEED * deltaT);
        }
        
        radarZoom.getInfo(&radarMovCoords, &dummyAngle, &dummyMagnitude);
        if(radarMovCoords.y != 0.0f) {
            zoomRadar(
                (-radarMovCoords.y) * PAUSE_MENU::RADAR_ZOOM_SPEED * deltaT
            );
        }
        
        bool mouseInRadar =
            isPointInRectangle(
                game.mouseCursor.winPos,
                radarDraw.center, radarDraw.size
            );
            
        if(mouseInRadar) {
            radarCursor = radarView.mouseCursorWorldPos;
        } else {
            radarCursor = radarView.cam.pos;
        }
        
        goHereCalcTime -= deltaT;
        if(goHereCalcTime <= 0.0f) {
            goHereCalcTime = PAUSE_MENU::GO_HERE_CALC_INTERVAL;
            
            calculateGoHerePath();
        }
        
    }
    
}


/**
 * @brief Zooms the radar by an amount.
 *
 * @param amount How much to zoom by.
 */
void PauseMenu::zoomRadar(float amount) {
    float delta = amount * radarView.cam.zoom;
    radarView.cam.zoom += delta;
    radarView.cam.zoom =
        std::clamp(
            radarView.cam.zoom,
            PAUSE_MENU::RADAR_MIN_ZOOM, PAUSE_MENU::RADAR_MAX_ZOOM
        );
}


/**
 * @brief Zooms the radar by an amount, anchored on the radar cursor.
 *
 * @param amount How much to zoom by.
 * @param radarCenter Coordinates of the radar's center.
 * @param radarSize Dimensions of the radar.
 */
void PauseMenu::zoomRadarWithMouse(
    float amount, const Point& radarCenter, const Point& radarSize
) {
    //Keep a backup of the old radar cursor coordinates.
    Point oldCursorPos = radarCursor;
    
    //Do the zoom.
    zoomRadar(amount);
    radarView.updateTransformations();
    
    //Figure out where the cursor will be after the zoom.
    radarCursor = game.mouseCursor.winPos;
    al_transform_coordinates(
        &radarView.windowToWorldTransform,
        &radarCursor.x, &radarCursor.y
    );
    
    //Readjust the transformation by shifting the camera
    //so that the cursor ends up where it was before.
    panRadar(
        Point(
            (oldCursorPos.x - radarCursor.x) * radarView.cam.zoom,
            (oldCursorPos.y - radarCursor.y) * radarView.cam.zoom
        )
    );
    
    //Update the cursor coordinates again.
    radarView.updateTransformations();
    radarCursor = game.mouseCursor.winPos;
    al_transform_coordinates(
        &radarView.windowToWorldTransform,
        &radarCursor.x, &radarCursor.y
    );
}
