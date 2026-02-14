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


namespace MAKER_TOOLS {

//Time the player has to confirm a maker tool usage in normal play.
const float PLAY_CONFIRMATION_TIMER = 1.0f;

}


/**
 * @brief Checks whether maker tools are allowed, and if not, sets up
 * code to warn the player and let them allow it, if applicable.
 *
 * @param inputValue Input value.
 * @return Whether they are allowed.
 */
bool MakerTools::checkMakerToolsAllowed(float inputValue) {
    if(!enabled) return false;
    bool isInPlay =
        game.states.gameplay->loaded &&
        game.quickPlay.areaPath.empty();
        
    if(
        !game.options.misc.makerToolsInPlay && isInPlay && !allowedInPlayNow &&
        inputValue >= 0.5f
    ) {
        if(playConfirmationPresses == 0) {
            game.console.write(
                "You've tried to use a maker tool, which isn't fit for "
                "normal gameplay.\n"
                "Are you sure? Quickly press a maker tool button three times "
                "to confirm.",
                15
            );
            playConfirmationTimer = MAKER_TOOLS::PLAY_CONFIRMATION_TIMER;
            playConfirmationPresses++;
            return false;
        } else if(playConfirmationPresses == 2) {
            game.console.write(
                "Maker tools are now allowed till you leave the area.\n"
                "Check the options to always allow maker tools in normal "
                "gameplay.",
                15
            );
            allowedInPlayNow = true;
            return false;
        } else {
            playConfirmationPresses++;
            return false;
        }
    }
    return true;
}


/**
 * @brief Returns which setting index to use for a settings-based maker tool,
 * depending on the modifier inputs that are currently pressed.
 *
 * @return The index.
 */
unsigned char MakerTools::getMakerToolSettingIdx() const {
    return mod1 ? 1 : mod2 ? 2 : 0;
}


/**
 * @brief Handles a player action and performs an input tool if possible,
 * for the tools that take place during gameplay only.
 *
 * @param action The action.
 * @return Whether it got handled.
 */
bool MakerTools::handleGameplayPlayerAction(const Inpution::Action& action) {
    bool isGameplayToolAction =
        game.controls.getActionTypeById(
            (PLAYER_ACTION_TYPE) action.actionTypeId
        ).category ==
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS;
    if(!isGameplayToolAction) return false;
    if(!checkMakerToolsAllowed(action.value)) return true;
    if(action.value < 0.5f) return false;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_AREA_IMAGE: {

        unsigned char settingIdx = getMakerToolSettingIdx();
        ALLEGRO_BITMAP* bmp =
            game.states.gameplay->drawToBitmap(
                areaImageSettings[settingIdx]
            );
        string fileName =
            FOLDER_PATHS_FROM_ROOT::USER_DATA + "/area_" +
            game.curAreaData->manifest->internalName +
            "_" + getCurrentTime(true) + ".png";
            
        if(!al_save_bitmap(fileName.c_str(), bmp)) {
            game.errors.report(
                "Could not save the area onto an image,"
                " with the name \"" + fileName + "\"!"
            );
        } else {
            game.console.write("Saved area image \"" + fileName + "\".");
        }
        
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_CHANGE_SPEED: {

        if(frameAdvanceMode) {
            frameAdvanceMode = false;
            mustAdvanceOneFrame = false;
        } else {
            unsigned char settingIdx =
                getMakerToolSettingIdx();
            bool finalState = false;
            if(!changeSpeed) {
                finalState = true;
            } else {
                if(changeSpeedSettingIdx != settingIdx) {
                    finalState = true;
                }
            }
            
            if(finalState) {
                changeSpeedSettingIdx = settingIdx;
            }
            changeSpeed = finalState;
        }
        
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_FRAME_ADVANCE: {

        if(mod1) {
            frameAdvanceMode = false;
            mustAdvanceOneFrame = false;
        } else {
            if(!frameAdvanceMode) {
                frameAdvanceMode = true;
            } else {
                mustAdvanceOneFrame = true;
            }
        }
        
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO: {

        geometryInfo =
            !geometryInfo;
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_HUD: {

        hud = !hud;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_HURT_MOB: {

        unsigned char settingIdx = getMakerToolSettingIdx();
        Mob* m =
            getClosestMobToMouseCursor(
                game.states.gameplay->players[0].view, true
            );
        if(m) {
            m->setHealth(
                true, true,
                -mobHurtingSettings[settingIdx]
            );
        }
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_MOB_INFO: {

        Mob* prevLockMob = infoLock;
        Mob* m;
        if(mod1) {
            m =
                getNextMobNearCursor(
                    game.states.gameplay->players[0].view, prevLockMob, false
                );
        } else if(mod2) {
            m = nullptr;
        } else {
            m =
                getClosestMobToMouseCursor(
                    game.states.gameplay->players[0].view, false
                );
        }
        
        infoLock = prevLockMob == m ? nullptr : m;
        if(
            prevLockMob != nullptr &&
            infoLock == nullptr
        ) {
            game.console.write("Mob: None.", 2.0f, 2.0f);
        }
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_NEW_PIKMIN: {

        if(
            game.states.gameplay->mobs.pikmin.size() <
            game.curAreaData->getMaxPikminInField()
        ) {
            bool mustUseLastType = (mod1 && lastPikminType);
            PikminType* newPikminType = nullptr;
            
            if(mustUseLastType) {
                newPikminType = lastPikminType;
            } else {
                newPikminType =
                    game.content.mobTypes.list.pikmin.begin()->second;
                    
                auto p = game.content.mobTypes.list.pikmin.begin();
                for(; p != game.content.mobTypes.list.pikmin.end(); ++p) {
                    if(p->second == lastPikminType) {
                        ++p;
                        if(p != game.content.mobTypes.list.pikmin.end()) {
                            newPikminType = p->second;
                        }
                        break;
                    }
                }
                lastPikminType = newPikminType;
            }
            
            createMob(
                game.mobCategories.get(MOB_CATEGORY_PIKMIN),
                game.states.gameplay->players[0].view.mouseCursorWorldPos,
                newPikminType, 0,
                mod2 ? "maturity=0" : "maturity=2"
            );
            usedHelpingTools = true;
        }
        break;
        
    } case PLAYER_ACTION_TYPE_MT_PATH_INFO: {

        pathInfo = !pathInfo;
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SHOW_COLLISION: {

        collision =
            !collision;
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES: {

        hitboxes =
            !hitboxes;
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SHOW_REACHES: {

        reaches = !reaches;
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_TELEPORT: {

        Mob* mobToTeleport =
            (mod1 && infoLock) ?
            infoLock :
            game.states.gameplay->players[0].leaderPtr;
            
        Sector* mouseSector =
            getSector(
                game.states.gameplay->players[0].view.mouseCursorWorldPos,
                nullptr, true
            );
        if(mouseSector && mobToTeleport) {
            mobToTeleport->chase(
                game.states.gameplay->players[0].view.mouseCursorWorldPos,
                mouseSector->z, CHASE_FLAG_TELEPORT
            );
            if(mobToTeleport == game.states.gameplay->players[0].leaderPtr) {
                game.states.gameplay->players[0].view.cam.setPos(
                    game.states.gameplay->players[0].view.mouseCursorWorldPos
                );
            } else {
                //Tick it once so it can run its teleportation code.
                //This is useful if the player teleports it far away,
                //where it'd be marked as inactive. It's slightly hacky,
                //but it's just a maker tool, so no sweat.
                mobToTeleport->tick(FLT_MIN);
            }
        }
        usedHelpingTools = true;
        break;
    }
    }
    
    return true;
}


/**
 * @brief Handles a player action and performs an input tool if possible,
 * for the tools that take place globally, as well as for the modifiers.
 *
 * @param action The action.
 * @return Whether it got handled.
 */
bool MakerTools::handleGeneralPlayerAction(const Inpution::Action& action) {
    bool isGeneralToolAction =
        game.controls.getActionTypeById(
            (PLAYER_ACTION_TYPE) action.actionTypeId
        ).category ==
        PLAYER_ACTION_CAT_GENERAL_MAKER_TOOLS;
    if(!isGeneralToolAction) return false;
    if(!checkMakerToolsAllowed(action.value)) return true;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_AUTO_START: {

        if(action.value < 0.5f) return false;
        
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
        if(autoStartState.empty()) {
            game.console.write("Reset Pikifen's auto-start.");
        } else {
            string msg =
                "Set Pikifen to auto-start in the \"" + autoStartState +
                "\" state";
            if(autoStartOption.empty()) {
                msg += ".";
            } else {
                msg += ", option \"" + autoStartOption + "\".";
            }
            game.console.write(msg);
        }
        
        usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP: {

        if(action.value < 0.5f) return false;
        
        game.audio.setSongPosNearLoop();
        break;
        
    } case PLAYER_ACTION_TYPE_MT_MOD_1: {

        mod1 = action.value >= 0.5f;
        
        break;
        
    } case PLAYER_ACTION_TYPE_MT_MOD_2: {

        mod2 = action.value >= 0.5f;
        
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
            areaImageNode->getChildByName("normal_settings"),
            areaImageNode->getChildByName("mod_1_settings"),
            areaImageNode->getChildByName("mod_2_settings")
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
            changeSpeedNode->getChildByName("normal_settings"),
            changeSpeedNode->getChildByName("mod_1_settings"),
            changeSpeedNode->getChildByName("mod_2_settings")
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
            hurtMobNode->getChildByName("normal_settings"),
            hurtMobNode->getChildByName("mod_1_settings"),
            hurtMobNode->getChildByName("mod_2_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter hRS(settingsNodes[s]);
            
            DataNode* percentageNode = nullptr;
            hRS.set("percentage", mobHurtingSettings[s], &percentageNode);
            if(percentageNode) {
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
    frameAdvanceMode = false;
    collision = false;
    geometryInfo = false;
    hitboxes = false;
    hud = true;
    infoLock = nullptr;
    lastPikminType = nullptr;
    pathInfo = false;
    reaches = false;
    
    usedHelpingTools = false;
    allowedInPlayNow = false;
    playConfirmationPresses = 0;
    playConfirmationTimer = 0.0f;
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
            areaImageNode->addNew("normal_settings"),
            areaImageNode->addNew("mod_1_settings"),
            areaImageNode->addNew("mod_2_settings")
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
            changeSpeedNode->addNew("normal_settings"),
            changeSpeedNode->addNew("mod_1_settings"),
            changeSpeedNode->addNew("mod_2_settings")
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
            hurtMobNode->addNew("normal_settings"),
            hurtMobNode->addNew("mod_1_settings"),
            hurtMobNode->addNew("mod_2_settings")
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


/**
 * @brief Ticks one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void MakerTools::tick(float deltaT) {
    if(playConfirmationTimer > 0.0f) {
        playConfirmationTimer -= deltaT;
        if(playConfirmationTimer <= 0.0f) {
            playConfirmationTimer = 0.0f;
            playConfirmationPresses = 0;
        }
    }
}
