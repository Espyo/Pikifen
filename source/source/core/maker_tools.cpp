/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Maker tool structures and functions.
 */


#include "maker_tools.h"

#include "game.h"
#include "misc_functions.h"


/**
 * @brief Constructs a new maker tools info object.
 */
MakerTools::MakerTools() {
    infoPrintTimer = Timer(1.0f, [this] () { infoPrintText.clear(); });
}


/**
 * @brief Returns which setting index to use for a settings-based maker tool,
 * depending on the keys that are currently pressed.
 *
 * @return The index.
 */
unsigned char MakerTools::getMakerToolSettingIdx() const {
    bool isShiftPressed = false;
    bool isCtrlPressed = false;
    getShiftCtrlAltState(
        &isShiftPressed, &isCtrlPressed, nullptr
    );
    return
        isShiftPressed ? 1 :
        isCtrlPressed ? 2 :
        0;
}


/**
 * @brief Handles a player action and performs an input tool if possible,
 * for the tools that take place during gameplay only.
 *
 * @param action The action.
 * @return Whether it got handled.
 */
bool MakerTools::handleGameplayPlayerAction(const PlayerAction &action) {
    bool isGameplayToolAction =
        game.controls.getPlayerActionType(action.actionTypeId).category ==
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS;
    if(!isGameplayToolAction) return false;
    if(!enabled) return true;
    if(action.value < 0.5f) return false;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_AREA_IMAGE: {

        unsigned char settingIdx = getMakerToolSettingIdx();
        ALLEGRO_BITMAP* bmp =
            game.states.gameplay->drawToBitmap(
                game.makerTools.areaImageSettings[settingIdx]
            );
        string fileName =
            FOLDER_PATHS_FROM_ROOT::USER_DATA + "/area_" +
            sanitizeFileName(game.curAreaData->name) +
            "_" + getCurrentTime(true) + ".png";
            
        if(!al_save_bitmap(fileName.c_str(), bmp)) {
            game.errors.report(
                "Could not save the area onto an image,"
                " with the name \"" + fileName + "\"!"
            );
        }
        
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_CHANGE_SPEED: {

        unsigned char settingIdx =
            getMakerToolSettingIdx();
        bool finalState = false;
        if(!game.makerTools.changeSpeed) {
            finalState = true;
        } else {
            if(game.makerTools.changeSpeedSettingIdx != settingIdx) {
                finalState = true;
            }
        }
        
        if(finalState) {
            game.makerTools.changeSpeedSettingIdx = settingIdx;
        }
        game.makerTools.changeSpeed = finalState;
        
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO: {

        game.makerTools.geometryInfo =
            !game.makerTools.geometryInfo;
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_HUD: {

        game.makerTools.hud = !game.makerTools.hud;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_HURT_MOB: {

        unsigned char settingIdx = getMakerToolSettingIdx();
        Mob* m = getClosestMobToCursor(true);
        if(m) {
            m->setHealth(
                true, true,
                -game.makerTools.mobHurtingSettings[settingIdx]
            );
        }
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_MOB_INFO: {

        bool isShiftPressed;
        bool isCtrlPressed;
        getShiftCtrlAltState(
            &isShiftPressed, &isCtrlPressed, nullptr
        );
        
        Mob* prevLockMob = game.makerTools.infoLock;
        Mob* m;
        if(isShiftPressed) {
            m = getNextMobNearCursor(prevLockMob, false);
        } else if(isCtrlPressed) {
            m = nullptr;
        } else {
            m = getClosestMobToCursor(false);
        }
        
        game.makerTools.infoLock = prevLockMob == m ? nullptr : m;
        if(
            prevLockMob != nullptr &&
            game.makerTools.infoLock == nullptr
        ) {
            printInfo("Mob: None.", 2.0f, 2.0f);
        }
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_NEW_PIKMIN: {

        if(
            game.states.gameplay->mobs.pikmin.size() <
            game.config.rules.maxPikminInField
        ) {
            bool isShiftPressed;
            bool isCtrlPressed;
            getShiftCtrlAltState(
                &isShiftPressed, &isCtrlPressed, nullptr
            );
            
            bool mustUseLastType =
                (isShiftPressed && game.makerTools.lastPikminType);
            PikminType* newPikminType = nullptr;
            
            if(mustUseLastType) {
                newPikminType = game.makerTools.lastPikminType;
            } else {
                newPikminType =
                    game.content.mobTypes.list.pikmin.begin()->second;
                    
                auto p = game.content.mobTypes.list.pikmin.begin();
                for(; p != game.content.mobTypes.list.pikmin.end(); ++p) {
                    if(p->second == game.makerTools.lastPikminType) {
                        ++p;
                        if(p != game.content.mobTypes.list.pikmin.end()) {
                            newPikminType = p->second;
                        }
                        break;
                    }
                }
                game.makerTools.lastPikminType = newPikminType;
            }
            
            createMob(
                game.mobCategories.get(MOB_CATEGORY_PIKMIN),
                game.view.cursorWorldPos, newPikminType, 0,
                isCtrlPressed ? "maturity=0" : "maturity=2"
            );
            game.makerTools.usedHelpingTools = true;
        }
        break;
        
    } case PLAYER_ACTION_TYPE_MT_PATH_INFO: {

        game.makerTools.pathInfo = !game.makerTools.pathInfo;
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SHOW_COLLISION: {

        game.makerTools.collision =
            !game.makerTools.collision;
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES: {

        game.makerTools.hitboxes =
            !game.makerTools.hitboxes;
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_TELEPORT: {

        bool isShiftPressed;
        getShiftCtrlAltState(&isShiftPressed, nullptr, nullptr);
        
        Mob* mobToTeleport =
            (isShiftPressed && game.makerTools.infoLock) ?
            game.makerTools.infoLock :
            game.states.gameplay->curLeaderPtr;
            
        Sector* mouseSector =
            getSector(game.view.cursorWorldPos, nullptr, true);
        if(mouseSector && mobToTeleport) {
            mobToTeleport->chase(
                game.view.cursorWorldPos, mouseSector->z,
                CHASE_FLAG_TELEPORT
            );
            game.view.cam.setPos(game.view.cursorWorldPos);
        }
        game.makerTools.usedHelpingTools = true;
        break;
    }
    }
    
    return true;
}


/**
 * @brief Handles a player action and performs an input tool if possible,
 * for the tools that take place globally.
 *
 * @param action The action.
 * @return Whether it got handled.
 */
bool MakerTools::handleGlobalPlayerAction(const PlayerAction &action) {
    bool isGlobalToolAction =
        game.controls.getPlayerActionType(action.actionTypeId).category ==
        PLAYER_ACTION_CAT_GLOBAL_MAKER_TOOLS;
    if(!isGlobalToolAction) return false;
    if(!enabled) return true;
    if(action.value < 0.5f) return false;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_AUTO_START: {

        string curStateName = game.getCurStateName();
        if(curStateName == game.states.animationEd->getName()) {
            autoStartState = "animation_editor";
            autoStartOption =
                game.states.animationEd->getOpenedContentPath();
        } else if(curStateName == game.states.areaEd->getName()) {
            autoStartState = "area_editor";
            autoStartOption =
                game.states.areaEd->getOpenedContentPath();
        } else if(curStateName == game.states.guiEd->getName()) {
            autoStartState = "gui_editor";
            autoStartOption =
                game.states.guiEd->getOpenedContentPath();
        } else if(curStateName == game.states.particleEd->getName()) {
            autoStartState = "particle_editor";
            autoStartOption =
                game.states.particleEd->getOpenedContentPath();
        } else if(curStateName == game.states.gameplay->getName()) {
            autoStartState = "play";
            autoStartOption =
                game.states.gameplay->pathOfAreaToLoad;
        } else {
            autoStartState.clear();
            autoStartOption.clear();
        }
        saveMakerTools();
        
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP: {

        game.audio.setSongPosNearLoop();
        break;
        
    }
    }
    
    return true;
}


/**
 * @brief Loads all the settings from a data node.
 *
 * @param node The node.
 */
void MakerTools::loadFromDataNode(DataNode* node) {
    //Whether maker tools are enabled.
    enabled = s2b(node->getChildByName("enabled")->value);
    
    //Controls.
    {
        DataNode* controlsNode = node->getChildByName("controls");
        game.controls.loadBindsFromDataNode(controlsNode, 0);
    }
    
    //Area image.
    {
        DataNode* areaImageNode = node->getChildByName("area_image");
        DataNode* settingsNodes[3] {
            areaImageNode->getChildByName("main_settings"),
            areaImageNode->getChildByName("shift_settings"),
            areaImageNode->getChildByName("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter aRS(settingsNodes[s]);
            aRS.set("size", areaImageSettings[s].size);
            aRS.set("padding", areaImageSettings[s].padding);
            aRS.set("mobs", areaImageSettings[s].mobs);
            aRS.set("shadows", areaImageSettings[s].shadows);
        }
    }
    
    //Auto start.
    {
        DataNode* autoStartNode = node->getChildByName("auto_start");
        ReaderSetter aRS(autoStartNode);
        aRS.set("state", autoStartState);
        aRS.set("option", autoStartOption);
    }
    
    //Change speed.
    {
        DataNode* changeSpeedNode = node->getChildByName("change_speed");
        DataNode* settingsNodes[3] {
            changeSpeedNode->getChildByName("main_settings"),
            changeSpeedNode->getChildByName("shift_settings"),
            changeSpeedNode->getChildByName("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter sRS(settingsNodes[s]);
            sRS.set("multiplier", changeSpeedSettings[s]);
        }
    }
    
    //Hurt mob.
    {
        DataNode* hurtMobNode = node->getChildByName("hurt_mob");
        DataNode* settingsNodes[3] {
            hurtMobNode->getChildByName("main_settings"),
            hurtMobNode->getChildByName("shift_settings"),
            hurtMobNode->getChildByName("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter hRS(settingsNodes[s]);
            DataNode* n;
            hRS.set("percentage", mobHurtingSettings[s], &n);
            if(n) {
                mobHurtingSettings[s] /= 100.0f;
            }
        }
    }
    
    //Performance monitor.
    {
        DataNode* perfMonNode = node->getChildByName("performance_monitor");
        ReaderSetter pRS(perfMonNode);
        pRS.set("enabled", usePerfMon);
    }
}


/**
 * @brief Resets the states of the tools so that players can play without any
 * tool affecting the experience.
 */
void MakerTools::resetForGameplay() {
    changeSpeed = false;
    collision = false;
    geometryInfo = false;
    hitboxes = false;
    hud = true;
    infoLock = nullptr;
    lastPikminType = nullptr;
    pathInfo = false;
    usedHelpingTools = false;
}


/**
 * @brief Saves all the settings to a data node.
 *
 * @param node The node.
 */
void MakerTools::saveToDataNode(DataNode* node) {
    GetterWriter mGW(node);
    
    //General.
    mGW.write("enabled", enabled);
    
    //Area image.
    {
        DataNode* areaImageNode = node->addNew("area_image");
        DataNode* settingsNodes[3] {
            areaImageNode->addNew("main_settings"),
            areaImageNode->addNew("shift_settings"),
            areaImageNode->addNew("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            GetterWriter sGW(settingsNodes[s]);
            sGW.write("size", areaImageSettings[s].size);
            sGW.write("padding", areaImageSettings[s].padding);
            sGW.write("mobs", areaImageSettings[s].mobs);
            sGW.write("shadows", areaImageSettings[s].shadows);
        }
    }
    
    //Auto start.
    {
        DataNode* autoStartNode = node->addNew("auto_start");
        GetterWriter aGW(autoStartNode);
        aGW.write("state", autoStartState);
        aGW.write("option", autoStartOption);
    }
    
    //Change speed.
    {
        DataNode* changeSpeedNode = node->addNew("change_speed");
        DataNode* settingsNodes[3] {
            changeSpeedNode->addNew("main_settings"),
            changeSpeedNode->addNew("shift_settings"),
            changeSpeedNode->addNew("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            GetterWriter sGW(settingsNodes[s]);
            sGW.write("multiplier", changeSpeedSettings[s]);
        }
    }
    
    //Hurt mob.
    {
        DataNode* hurtMobNode = node->addNew("hurt_mob");
        DataNode* settingsNodes[3] {
            hurtMobNode->addNew("main_settings"),
            hurtMobNode->addNew("shift_settings"),
            hurtMobNode->addNew("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            GetterWriter sGW(settingsNodes[s]);
            sGW.write("percentage", mobHurtingSettings[s] * 100.0f);
        }
    }
    
    //Performance monitor.
    {
        DataNode* perfMonNode = node->addNew("performance_monitor");
        GetterWriter pGW(perfMonNode);
        pGW.write("enabled", usePerfMon);
    }
}
