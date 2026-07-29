/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Maker tool structures and functions.
 */


#include "maker_tools.hpp"

#include "../util/os_utils.hpp"
#include "game.hpp"
#include "misc_functions.hpp"


namespace MAKER_TOOLS {

//Time the player has to confirm a maker tool usage in normal play.
const float PLAY_CONFIRMATION_TIMER = 1.0f;

}


#pragma region Tool runner functions


/**
 * @brief Code for the area image maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::areaImage(
    MakerTools& mgr, const vector<string>& args
) {
    int size = s2i(args[0]);
    int padding = s2i(args[1]);
    bool treeShadows = s2b(args[2]);
    
    ALLEGRO_BITMAP* bmp =
        game.states.gameplay->drawToBitmap(
            size, padding, treeShadows
        );
    string fileName =
        FOLDER_PATHS_FROM_ROOT::USER_DATA + "/area_" +
        game.curArea->manifest->internalName +
        "_" + getCurrentTime(true) + ".png";
        
    if(!al_save_bitmap(fileName.c_str(), bmp)) {
        game.errors.report(
            "Could not save the area onto an image,"
            " with the name \"" + fileName + "\"!"
        );
    } else {
        game.console.write(
            "Saved area image \"" + fileName + "\".", false, 5.0f
        );
    }
    
    return true;
}


/**
 * @brief Code for the area inspector maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::areaInspector(
    MakerTools& mgr, const vector<string>& args
) {
    mgr.inspectingArea = !mgr.inspectingArea;
    if(!mgr.inspectingArea) {
        game.makerDisplay.write("No longer inspecting area.", 5.0f);
    }
    
    return true;
}


/**
 * @brief Code for the change speed maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::changeSpeed(
    MakerTools& mgr, const vector<string>& args
) {
    float newMultiplier = s2f(args[0]);
    
    if(mgr.frameAdvanceMode) {
        mgr.frameAdvanceMode = false;
        mgr.mustAdvanceOneFrame = false;
    } else {
        bool finalState = false;
        if(!mgr.changeSpeed) {
            finalState = true;
        } else {
            if(mgr.changeSpeedMultiplier != newMultiplier) {
                finalState = true;
            }
        }
        
        if(finalState) {
            mgr.changeSpeedMultiplier = newMultiplier;
        }
        mgr.changeSpeed = finalState;
    }
    
    return true;
}


/**
 * @brief Code for the delete mob maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::deleteMob(
    MakerTools& mgr, const vector<string>& args
) {
    if(mgr.inspectedMob) {
        mgr.inspectedMob->toDelete = true;
    } else {
        game.makerDisplay.write("No mob is being inspected.", 5.0f);
        return false;
    }
    
    return true;
}


/**
 * @brief Code for the fill inventory maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::fillInventory(
    MakerTools& mgr, const vector<string>& args
) {
    size_t newAmount = s2i(args[0]);
    
    for(size_t t = 0; t < MAX_PLAYER_TEAMS; t++) {
        PlayerTeam* tPtr = &game.states.gameplay->playerTeams[t];
        forIdx(s, tPtr->sprayStats) {
            tPtr->sprayStats[s].nrSprays = newAmount;
        }
    }
    
    return true;
}


/**
 * @brief Code for the frame advance maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::frameAdvance(
    MakerTools& mgr, const vector<string>& args
) {
    MAKER_TOOL_FRAME_ADVANCE_ACTION action =
        enumGetValue(makerToolFrameAdvanceActionINames, args[0]);
        
    switch(action) {
    case MAKER_TOOL_FRAME_ADVANCE_ACTION_ADVANCE: {
        if(!mgr.frameAdvanceMode) {
            mgr.frameAdvanceMode = true;
        } else {
            mgr.mustAdvanceOneFrame = true;
        }
        break;
    } case MAKER_TOOL_FRAME_ADVANCE_ACTION_RESUME: {
        mgr.frameAdvanceMode = false;
        mgr.mustAdvanceOneFrame = false;
        break;
    }
    }
    
    return true;
}


/**
 * @brief Code for the free camera maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::freeCam(
    MakerTools& mgr, const vector<string>& args
) {
    MAKER_TOOL_FREE_CAM_ACTION action =
        enumGetValue(makerToolFreeCamActionINames, args[0]);
        
    bool wasControllingLeader = !mgr.freeCamMode || mgr.freeCamFrozen;
    
    switch(action) {
    case MAKER_TOOL_FREE_CAM_ACTION_MODE: {
        mgr.freeCamMode = !mgr.freeCamMode;
        mgr.freeCamFrozen = false;
        break;
    } case MAKER_TOOL_FREE_CAM_ACTION_FREEZE: {
        if(!mgr.freeCamMode) mgr.freeCamMode = true;
        mgr.freeCamFrozen = !mgr.freeCamFrozen;
        break;
    }
    }
    
    bool isControllingLeader = !mgr.freeCamMode || mgr.freeCamFrozen;
    
    if(wasControllingLeader && !isControllingLeader) {
        //Stop the leaders, otherwise they'll keep moving out of control.
        game.states.gameplay->stopAllLeaders();
    } else if(!wasControllingLeader && isControllingLeader) {
        //Stop the camera, otherwise it'll keep moving out of control.
        game.states.gameplay->players[0].view.cam.centerSpeed = Point(0.0f);
    }
    
    return true;
}


/**
 * @brief Code for the geometry info maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::geometryInfo(
    MakerTools& mgr, const vector<string>& args
) {
    mgr.geometryInfo = !mgr.geometryInfo;
    if(mgr.geometryInfo) {
        mgr.toolStartCursor =
            game.states.gameplay->players[0].view.mouseCursorWorldPos;
    }
    
    return true;
}


/**
 * @brief Code for the hide HUD maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::hideHud(
    MakerTools& mgr, const vector<string>& args
) {
    mgr.hud = !mgr.hud;
    
    return true;
}


/**
 * @brief Code for the hurt mob maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::hurtMob(
    MakerTools& mgr, const vector<string>& args
) {
    float ratio = s2f(args[0]);
    
    Mob* m =
        getClosestMobToMouseCursor(
            game.states.gameplay->players[0].view, true
        );
        
    if(!m) return false;
    
    m->setHealth(true, true, -ratio);
    
    return true;
}


/**
 * @brief Code for the mob inspector maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::mobInspector(
    MakerTools& mgr, const vector<string>& args
) {
    MAKER_TOOL_MOB_INSPECTOR_ACTION action =
        enumGetValue(makerToolMobInspectorActionINames, args[0]);
        
    Mob* prevInspectedMob = mgr.inspectedMob;
    Mob* m = nullptr;
    
    switch(action) {
    case MAKER_TOOL_MOB_INSPECTOR_ACTION_GET_CLOSEST: {
        m =
            getClosestMobToMouseCursor(
                game.states.gameplay->players[0].view, false
            );
        break;
    } case MAKER_TOOL_MOB_INSPECTOR_ACTION_ITERATE: {
        m =
            getNextMobNearCursor(
                game.states.gameplay->players[0].view,
                prevInspectedMob, false
            );
        break;
    } case MAKER_TOOL_MOB_INSPECTOR_ACTION_STOP: {
        break;
    }
    }
    
    mgr.inspectedMob = prevInspectedMob == m ? nullptr : m;
    if(
        prevInspectedMob != nullptr &&
        mgr.inspectedMob == nullptr
    ) {
        game.makerDisplay.write("No longer inspecting mob.", 5.0f);
    }
    
    return true;
}


/**
 * @brief Code for the new Pikmin maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::newPikmin(
    MakerTools& mgr, const vector<string>& args
) {
    string type = args[0];
    MATURITY maturity = enumGetValue(maturityINames, args[1]);
    
    if(
        game.states.gameplay->mobs.pikmin.size() >=
        game.curArea->getMaxPikminInField()
    ) {
        return false;
    }
    
    PikminType* newPikminType = nullptr;
    
    if(type == "same" && mgr.lastPikminType) {
        newPikminType = mgr.lastPikminType;
    } else if(type == "next") {
        newPikminType =
            game.content.mobTypes.list.pikmin.begin()->second;
            
        auto p = game.content.mobTypes.list.pikmin.begin();
        for(; p != game.content.mobTypes.list.pikmin.end(); ++p) {
            if(p->second == mgr.lastPikminType) {
                ++p;
                if(p != game.content.mobTypes.list.pikmin.end()) {
                    newPikminType = p->second;
                }
                break;
            }
        }
        mgr.lastPikminType = newPikminType;
    }
    
    createMob(
        game.mobCategories.get(MOB_CATEGORY_PIKMIN),
        game.states.gameplay->players[0].view.mouseCursorWorldPos,
        newPikminType, 0,
        "maturity=" + i2s(maturity)
    );
    
    return true;
}


/**
 * @brief Code for the new reminder maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::newReminder(
    MakerTools& mgr, const vector<string>& args
) {
    if(game.states.gameplay->isPaused()) return false;
    
    mgr.toolStartCursor =
        game.states.gameplay->players[0].view.mouseCursorWorldPos;
    game.modal.reset();
    game.modal.title = "New reminder";
    game.modal.prompt =
        "Creating a new reminder for the area maker at " +
        i2s(mgr.toolStartCursor.x) + "," + i2s(mgr.toolStartCursor.y) +
        ".\n"
        "What should the reminder say?\n"
        "\n\n\n\n\n\n";
    game.modal.back = "Cancel";
    game.modal.onBack =
    [] () {
        game.controls.ignoreMenuCloseActions();
        game.console.write("Cancelled reminder.", false, 5.0f);
    };
    game.modal.extraButtons.push_back(
    ModalGuiManager::Button {
        .text = "Create",
        .tooltip = "Create the reminder.",
        .onActivate = [mgr] (const Point&) {
            AreaMakerReminder newReminder;
            newReminder.center = mgr.toolStartCursor;
            newReminder.text = game.modal.textInput;
            game.curArea->reminders.push_back(newReminder);
            game.content.areas.saveAreaReminders(game.curArea);
            game.modal.close();
            game.controls.ignoreMenuCloseActions();
            game.console.write("Created reminder!", false, 5.0f);
        }
    }
    );
    game.modal.defaultFocusButtonIdx = 1;
    game.modal.textInputEnterButtonIdx = 1;
    game.modal.useTextInput = true;
    game.modal.updateItems();
    game.modal.open();
    
    return true;
}


/**
 * @brief Code for the path info maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::pathInfo(
    MakerTools& mgr, const vector<string>& args
) {
    mgr.pathInfo = !mgr.pathInfo;
    
    return true;
}


/**
 * @brief Code for the set auto start maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::setAutoStart(
    MakerTools& mgr, const vector<string>& args
) {
    string curStateName = game.getCurStateName();
    
    if(curStateName == game.states.animationEd->getName()) {
        mgr.autoStartState = "animation_editor";
        mgr.autoStartOption =
            game.states.animationEd->getOpenedContentPath();
    } else if(curStateName == game.states.areaEd->getName()) {
        mgr.autoStartState = "area_editor";
        mgr.autoStartOption =
            game.states.areaEd->getOpenedContentPath();
    } else if(curStateName == game.states.guiEd->getName()) {
        mgr.autoStartState = "gui_editor";
        mgr.autoStartOption =
            game.states.guiEd->getOpenedContentPath();
    } else if(curStateName == game.states.particleEd->getName()) {
        mgr.autoStartState = "particle_editor";
        mgr.autoStartOption =
            game.states.particleEd->getOpenedContentPath();
    } else if(curStateName == game.states.gameplay->getName()) {
        mgr.autoStartState = "play";
        mgr.autoStartOption =
            game.states.gameplay->pathOfAreaToLoad;
    } else {
        mgr.autoStartState.clear();
        mgr.autoStartOption.clear();
    }
    
    saveMakerTools();
    if(mgr.autoStartState.empty()) {
        game.console.write("Reset Pikifen's auto-start.", false, 5.0f);
    } else {
        string msg =
            "Set Pikifen to auto-start in the \"" + mgr.autoStartState +
            "\" state";
        if(mgr.autoStartOption.empty()) {
            msg += ".";
        } else {
            msg += ", option \"" + mgr.autoStartOption + "\".";
        }
        game.console.write(msg, false, 5.0f);
    }
    
    mgr.usedHelpingTools = true;
    
    return true;
}


/**
 * @brief Code for the set song near loop maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::setSongPosNearLoop(
    MakerTools& mgr, const vector<string>& args
) {
    game.audio.setSongPosNearLoop();
    
    return true;
}


/**
 * @brief Code for the show collision maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::showCollision(
    MakerTools& mgr, const vector<string>& args
) {
    mgr.collision = !mgr.collision;
    
    return true;
}


/**
 * @brief Code for the show hitboxes maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::showHitboxes(
    MakerTools& mgr, const vector<string>& args
) {
    mgr.hitboxes = !mgr.hitboxes;
    
    return true;
}


/**
 * @brief Code for the show reaches maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::showReaches(
    MakerTools& mgr, const vector<string>& args
) {
    mgr.reaches = !mgr.reaches;
    
    return true;
}


/**
 * @brief Code for the teleport maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 */
bool MakerToolRunners::teleport(
    MakerTools& mgr, const vector<string>& args
) {
    bool useInspectedMob = s2b(args[0]);
    bool leaveGroupMembersBehind = s2b(args[1]);
    
    Mob* mobToTeleport =
        (useInspectedMob && mgr.inspectedMob) ?
        mgr.inspectedMob :
        game.states.gameplay->players[0].leaderPtr;
        
    Sector* mouseSector =
        getSector(
            game.states.gameplay->players[0].view.mouseCursorWorldPos,
            nullptr, true
        );
        
    if(!mouseSector || !mobToTeleport) return false;
    
    mobToTeleport->chase(
        game.states.gameplay->players[0].view.mouseCursorWorldPos,
        mouseSector->floorZ, CHASE_FLAG_TELEPORT
    );
    if(mobToTeleport == game.states.gameplay->players[0].leaderPtr) {
        game.states.gameplay->players[0].view.cam.setPos(
            game.states.gameplay->players[0].view.mouseCursorWorldPos
        );
        if(!leaveGroupMembersBehind) {
            forIdx(p, mobToTeleport->group->members) {
                mobToTeleport->group->members[p]->chase(
                    game.states.gameplay->
                    players[0].view.mouseCursorWorldPos,
                    mouseSector->floorZ, CHASE_FLAG_TELEPORT
                );
            }
        }
    } else {
        //Tick it once so it can run its teleportation code.
        //This is useful if the player teleports it far away,
        //where it'd be marked as inactive. It's slightly hacky,
        //but it's just a maker tool, so no sweat.
        mobToTeleport->tick(FLT_MIN);
    }
    
    return true;
}


#pragma endregion


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
            string info =
                "You've tried to use a maker tool, which isn't fit for "
                "normal gameplay.\n"
                "Are you sure? Quickly press a maker tool button three times "
                "to confirm.";
            game.console.write(info, false);
            playConfirmationTimer = MAKER_TOOLS::PLAY_CONFIRMATION_TIMER;
            playConfirmationPresses++;
            return false;
            
        } else if(playConfirmationPresses == 2) {
            string info =
                "Maker tools are now allowed till you next enter an area.\n"
                "Check the options to always allow maker tools in normal "
                "gameplay.";
            game.console.write(info, false);
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
    
    MAKER_TOOL_TYPE toolToRun = MAKER_TOOL_TYPE_NONE;
    vector<string> args;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_AREA_IMAGE: {
        toolToRun = MAKER_TOOL_TYPE_AREA_IMAGE;
        unsigned char settingIdx = getMakerToolSettingIdx();
        args.push_back(i2s(areaImageSettings[settingIdx].size));
        args.push_back(i2s(areaImageSettings[settingIdx].padding));
        args.push_back(b2s(areaImageSettings[settingIdx].shadows));
        break;
    } case PLAYER_ACTION_TYPE_MT_AREA_INSPECTOR: {
        toolToRun = MAKER_TOOL_TYPE_AREA_INSPECTOR;
        break;
    } case PLAYER_ACTION_TYPE_MT_CHANGE_SPEED: {
        toolToRun = MAKER_TOOL_TYPE_CHANGE_SPEED;
        unsigned char settingIdx = getMakerToolSettingIdx();
        args.push_back(f2s(changeSpeedSettings[settingIdx]));
        break;
    } case PLAYER_ACTION_TYPE_MT_DELETE_MOB: {
        toolToRun = MAKER_TOOL_TYPE_DELETE_MOB;
        break;
    } case PLAYER_ACTION_TYPE_MT_FILL_INVENTORY: {
        toolToRun = MAKER_TOOL_TYPE_FILL_INVENTORY;
        args.push_back(mod1 ? "0" : "99");
        break;
    } case PLAYER_ACTION_TYPE_MT_FRAME_ADVANCE: {
        toolToRun = MAKER_TOOL_TYPE_FRAME_ADVANCE;
        args.push_back(
            mod1 ?
            enumGetName(
                makerToolFrameAdvanceActionINames,
                MAKER_TOOL_FRAME_ADVANCE_ACTION_RESUME
            ) :
            enumGetName(
                makerToolFrameAdvanceActionINames,
                MAKER_TOOL_FRAME_ADVANCE_ACTION_ADVANCE
            )
        );
        break;
    } case PLAYER_ACTION_TYPE_MT_FREE_CAM: {
        toolToRun = MAKER_TOOL_TYPE_FREE_CAM;
        args.push_back(
            mod1 ?
            enumGetName(
                makerToolFreeCamActionINames,
                MAKER_TOOL_FREE_CAM_ACTION_FREEZE
            ) :
            enumGetName(
                makerToolFreeCamActionINames,
                MAKER_TOOL_FREE_CAM_ACTION_MODE
            )
        );
        break;
    } case PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO: {
        toolToRun = MAKER_TOOL_TYPE_GEOMETRY_INFO;
        break;
    } case PLAYER_ACTION_TYPE_MT_HIDE_HUD: {
        toolToRun = MAKER_TOOL_TYPE_HIDE_HUD;
        break;
    } case PLAYER_ACTION_TYPE_MT_HURT_MOB: {
        toolToRun = MAKER_TOOL_TYPE_HURT_MOB;
        unsigned char settingIdx = getMakerToolSettingIdx();
        args.push_back(f2s(mobHurtingSettings[settingIdx]));
        break;
    } case PLAYER_ACTION_TYPE_MT_MOB_INSPECTOR: {
        toolToRun = MAKER_TOOL_TYPE_MOB_INSPECTOR;
        args.push_back(
            mod1 ?
            enumGetName(
                makerToolMobInspectorActionINames,
                MAKER_TOOL_MOB_INSPECTOR_ACTION_ITERATE
            ) :
            mod2 ?
            enumGetName(
                makerToolMobInspectorActionINames,
                MAKER_TOOL_MOB_INSPECTOR_ACTION_STOP
            ) :
            enumGetName(
                makerToolMobInspectorActionINames,
                MAKER_TOOL_MOB_INSPECTOR_ACTION_GET_CLOSEST
            )
        );
        break;
    } case PLAYER_ACTION_TYPE_MT_NEW_PIKMIN: {
        toolToRun = MAKER_TOOL_TYPE_NEW_PIKMIN;
        args.push_back(mod1 ? "same" : "next");
        args.push_back(
            mod2 ?
            enumGetName(maturityINames, MATURITY_LEAF) :
            enumGetName(maturityINames, MATURITY_FLOWER)
        );
        break;
    } case PLAYER_ACTION_TYPE_MT_NEW_REMINDER: {
        toolToRun = MAKER_TOOL_TYPE_NEW_REMINDER;
        break;
    } case PLAYER_ACTION_TYPE_MT_PATH_INFO: {
        toolToRun = MAKER_TOOL_TYPE_PATH_INFO;
        break;
    } case PLAYER_ACTION_TYPE_MT_SHOW_COLLISION: {
        toolToRun = MAKER_TOOL_TYPE_SHOW_COLLISION;
        break;
    } case PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES: {
        toolToRun = MAKER_TOOL_TYPE_SHOW_HITBOXES;
        break;
    } case PLAYER_ACTION_TYPE_MT_SHOW_REACHES: {
        toolToRun = MAKER_TOOL_TYPE_SHOW_REACHES;
        break;
    } case PLAYER_ACTION_TYPE_MT_TELEPORT: {
        toolToRun = MAKER_TOOL_TYPE_TELEPORT;
        args.push_back(b2s(mod1));
        args.push_back(b2s(mod2));
        break;
    }
    }
    
    if(toolToRun != MAKER_TOOL_TYPE_NONE) {
        types[toolToRun].code(*this, args);
        if(types[toolToRun].helpful) {
            usedHelpingTools = true;
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
    case PLAYER_ACTION_TYPE_MT_CONSOLE: {

        if(action.value < 0.5f) return false;
        
        game.console.toggleTerminal();
        break;
        
    } case PLAYER_ACTION_TYPE_MT_AUTO_START: {

        if(action.value < 0.5f) return false;
        
        types[MAKER_TOOL_TYPE_SET_AUTO_START].code(*this, {});
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP: {

        if(action.value < 0.5f) return false;
        
        types[MAKER_TOOL_TYPE_SET_SONG_POS_NEAR_LOOP].code(*this, {});
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
    inspectingArea = false;
    inspectedMob = nullptr;
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
