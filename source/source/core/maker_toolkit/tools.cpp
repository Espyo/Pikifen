/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Maker tool structures and functions.
 */


#include "tools.hpp"

#include "../../util/os_utils.hpp"
#include "../game.hpp"
#include "../misc_functions.hpp"


namespace MAKER_TOOLS {

//Time the player has to confirm a maker tool usage in normal play.
const float PLAY_CONFIRMATION_TIMER = 1.0f;

}


#pragma region Tool runner functions


/**
 * @brief Code for the clear terminal output maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::clear(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    game.console.clearTerminal();
    
    return true;
}


/**
 * @brief Code for the control camera maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::controlCam(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& actionArg = args[0];
    
    //Main logic.
    bool actionFound = false;
    MAKER_TOOL_CONTROL_CAM_ACTION action =
        enumGetValue(makerToolControlCamActionINames, actionArg, &actionFound);
        
    if(!actionFound) {
        game.console.write(
            LOG_LEVEL_ERROR,
            "Unknown action type \"" + actionArg + "\"!"
        );
        return false;
    }
    
    bool wasControllingLeader = !mgr.inControlCam || mgr.controlCamFrozen;
    
    switch(action) {
    case MAKER_TOOL_CONTROL_CAM_ACTION_MODE: {
        mgr.inControlCam = !mgr.inControlCam;
        mgr.controlCamFrozen = false;
        break;
    } case MAKER_TOOL_CONTROL_CAM_ACTION_FREEZE: {
        if(!mgr.inControlCam) mgr.inControlCam = true;
        mgr.controlCamFrozen = !mgr.controlCamFrozen;
        break;
    }
    }
    
    bool isControllingLeader = !mgr.inControlCam || mgr.controlCamFrozen;
    
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
 * @brief Code for the create reminder maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::createReminder(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
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
        game.console.write(LOG_LEVEL_INFO, "Canceled reminder.", 5.0f);
    };
    game.modal.extraButtons.push_back(
    ModalGuiManager::Button {
        .text = "Create",
        .tooltip = "Create the reminder.",
        .onActivate = [mgr] (const Point&) {
            AreaMakerReminder newReminder;
            newReminder.center = mgr.toolStartCursor;
            newReminder.text = game.modal.textInput.getString();
            game.curArea->reminders.push_back(newReminder);
            game.content.areas.saveAreaReminders(game.curArea);
            game.modal.close();
            game.controls.ignoreMenuCloseActions();
            game.console.write(LOG_LEVEL_INFO, "Created reminder!", 5.0f);
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
 * @brief Code for the delete mob maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::deleteMob(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
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
 * @return Whether it succeeded.
 */
bool MakerToolRunners::fillInventory(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& amountArg = args[0];
    
    //Main logic.
    size_t newAmount = std::clamp(s2i(amountArg), 0, 9999);
    
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
 * @return Whether it succeeded.
 */
bool MakerToolRunners::frameAdvance(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& actionArg = args[0];
    
    //Main logic.
    bool actionFound = false;
    MAKER_TOOL_FRAME_ADVANCE_ACTION action =
        enumGetValue(
            makerToolFrameAdvanceActionINames, actionArg, &actionFound
        );
        
    if(!actionFound) {
        game.console.write(
            LOG_LEVEL_ERROR,
            "Unknown action type \"" + actionArg + "\"!"
        );
        return false;
    }
    
    switch(action) {
    case MAKER_TOOL_FRAME_ADVANCE_ACTION_ADVANCE: {
        if(!mgr.inFrameAdvanceMode) {
            mgr.inFrameAdvanceMode = true;
        } else {
            mgr.mustAdvanceOneFrame = true;
        }
        break;
    } case MAKER_TOOL_FRAME_ADVANCE_ACTION_RESUME: {
        mgr.inFrameAdvanceMode = false;
        mgr.mustAdvanceOneFrame = false;
        break;
    }
    }
    
    return true;
}


/**
 * @brief Code for the help maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::help(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    string list;
    forIdx(t, game.makerTools.types) {
        MakerToolType* tPtr = &game.makerTools.types[t];
        if(tPtr->type == MAKER_TOOL_TYPE_NONE) continue;
        if(!list.empty()) list += ", ";
        list += tPtr->name;
    }
    string text =
        "List of maker tools:\n" +
        list +
        "\nFor more information, see the included manual.";
    game.console.write(LOG_LEVEL_INFO, text);
    
    return true;
}


/**
 * @brief Code for the hide HUD maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::hideHud(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    mgr.showingHud = !mgr.showingHud;
    
    return true;
}


/**
 * @brief Code for the hurt mob maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::hurtMob(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& ratioArg = args[0];
    
    //Main logic.
    Mob* m =
        getClosestMobToMouseCursor(
            game.states.gameplay->players[0].view, true
        );
        
    if(!m) return false;
    
    m->setHealth(true, true, -s2f(ratioArg));
    
    return true;
}


/**
 * @brief Code for the inspect area maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::inspectArea(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    mgr.inspectingArea = !mgr.inspectingArea;
    if(!mgr.inspectingArea) {
        game.console.write(LOG_LEVEL_INFO, "No longer inspecting area.", 5.0f);
    }
    
    return true;
}


/**
 * @brief Code for the inspect mob maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::inspectMob(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& actionArg = args[0];
    
    //Main logic.
    bool actionFound = false;
    MAKER_TOOL_INSPECT_MOB_ACTION action =
        enumGetValue(
            makerToolInspectMobActionINames, actionArg, &actionFound
        );
        
    if(!actionFound) {
        game.console.write(
            LOG_LEVEL_ERROR,
            "Unknown action type \"" + actionArg + "\"!"
        );
        return false;
    }
    
    Mob* prevInspectedMob = mgr.inspectedMob;
    Mob* m = nullptr;
    
    switch(action) {
    case MAKER_TOOL_INSPECT_MOB_ACTION_GET_CLOSEST: {
        m =
            getClosestMobToMouseCursor(
                game.states.gameplay->players[0].view, false
            );
        break;
    } case MAKER_TOOL_INSPECT_MOB_ACTION_ITERATE: {
        m =
            getNextMobNearCursor(
                game.states.gameplay->players[0].view,
                prevInspectedMob, false
            );
        break;
    } case MAKER_TOOL_INSPECT_MOB_ACTION_STOP: {
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
 * @brief Code for the save area image maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::saveAreaImage(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& sizeArg = args[0];
    const string& paddingArg = args[1];
    const string& shadowsArg = args[2];
    
    //Main logic.
    ALLEGRO_BITMAP* bmp =
        game.states.gameplay->drawToBitmap(
            s2i(sizeArg), s2i(paddingArg), s2b(shadowsArg)
        );
    string fileName =
        FOLDER_PATHS_FROM_ROOT::USER_DATA + "/area_" +
        game.curArea->manifest->internalName +
        "_" + getCurrentTime(true) + ".png";
        
    if(!bmp || !al_save_bitmap(fileName.c_str(), bmp)) {
        game.log.write(
            LOG_LEVEL_ERROR,
            "Could not save the area onto an image,"
            " with the name \"" + fileName + "\"!"
        );
    } else {
        game.console.write(
            LOG_LEVEL_INFO,
            "Saved area image \"" + fileName + "\".", 5.0f
        );
    }
    
    return true;
}


/**
 * @brief Code for the set auto-start maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::setAutoStart(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
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
        game.console.write(LOG_LEVEL_INFO, "Reset Pikifen's auto-start.", 5.0f);
    } else {
        string msg =
            "Set Pikifen to auto-start in the \"" + mgr.autoStartState +
            "\" state";
        if(mgr.autoStartOption.empty()) {
            msg += ".";
        } else {
            msg += ", option \"" + mgr.autoStartOption + "\".";
        }
        game.console.write(LOG_LEVEL_INFO, msg, 5.0f);
    }
    
    mgr.usedHelpingTools = true;
    
    return true;
}


/**
 * @brief Code for the set song position near loop maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::setSongPosNearLoop(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& secondsArg = args[0];
    
    //Main logic.
    float seconds = std::clamp((float) s2f(secondsArg), 0.0f, 9999.0f);
    game.audio.setSongPosNearLoop(seconds);
    
    return true;
}


/**
 * @brief Code for the set speed maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::setSpeed(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& multArg = args[0];
    
    //Main logic.
    float newMultiplier = std::clamp((float) s2f(multArg), 0.001f, 100.0f);
    
    if(mgr.inFrameAdvanceMode) {
        mgr.inFrameAdvanceMode = false;
        mgr.mustAdvanceOneFrame = false;
    } else {
        bool finalState = false;
        if(!mgr.changingSpeed) {
            finalState = true;
        } else {
            if(mgr.changedSpeedMultiplier != newMultiplier) {
                finalState = true;
            }
        }
        
        if(finalState) {
            mgr.changedSpeedMultiplier = newMultiplier;
        }
        if(mgr.changedSpeedMultiplier == 1.0f) finalState = false;
        mgr.changingSpeed = finalState;
    }
    
    return true;
}


/**
 * @brief Code for the show collision maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::showCollision(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    mgr.showingCollision = !mgr.showingCollision;
    
    return true;
}


/**
 * @brief Code for the show geometry info maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::showGeometryInfo(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    mgr.showingGeometryInfo = !mgr.showingGeometryInfo;
    if(mgr.showingGeometryInfo) {
        mgr.toolStartCursor =
            game.states.gameplay->players[0].view.mouseCursorWorldPos;
    }
    
    return true;
}


/**
 * @brief Code for the show hitboxes maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::showHitboxes(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    mgr.showingHitboxes = !mgr.showingHitboxes;
    
    return true;
}


/**
 * @brief Code for the show path info maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::showPathInfo(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    mgr.showingPathInfo = !mgr.showingPathInfo;
    
    return true;
}


/**
 * @brief Code for the show reaches maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::showReaches(
    MakerTools& mgr, const vector<string>& args
) {
    //Main logic.
    mgr.showingReaches = !mgr.showingReaches;
    
    return true;
}


/**
 * @brief Code for the spawn mob maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::spawnMob(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& typeArg = args[0];
    const string& amountArg = args[1];
    
    //Main logic.
    size_t amount = std::clamp(s2i(amountArg), 1, 100);
    
    for(size_t m = 0; m < amount; m++) {
        MobType* newMobType = game.mobCategories.findMobType(typeArg);
        
        if(!newMobType) {
            game.console.write(
                LOG_LEVEL_ERROR,
                "Unknown mob type \"" + typeArg + "\"!"
            );
            return false;
        }
        
        createMob(
            newMobType->category,
            game.states.gameplay->players[0].view.mouseCursorWorldPos,
            newMobType, 0.0f, ""
        );
        
    }
    
    return true;
}


/**
 * @brief Code for the spawn Pikmin maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::spawnPikmin(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& typeArg = args[0];
    const string& amountArg = args[1];
    const string& maturityArg = args[2];
    
    //Main logic.
    size_t amount = std::clamp(s2i(amountArg), 1, 100);
    bool maturityFound = false;
    MATURITY maturity =
        enumGetValue(maturityINames, maturityArg, &maturityFound);
        
    if(!maturityFound) {
        game.console.write(
            LOG_LEVEL_ERROR,
            "Unknown maturity \"" + maturityArg + "\"!"
        );
        return false;
    }
    
    for(size_t p = 0; p < amount; p++) {
        if(
            game.states.gameplay->mobs.pikmin.size() >=
            game.curArea->getMaxPikminInField()
        ) {
            return true;
        }
        
        const auto getNextType =
        [&mgr] () {
            PikminType* result =
                game.content.mobTypes.list.pikmin.begin()->second;
            auto p = game.content.mobTypes.list.pikmin.begin();
            for(; p != game.content.mobTypes.list.pikmin.end(); ++p) {
                if(p->second == mgr.lastPikminType) {
                    ++p;
                    if(p != game.content.mobTypes.list.pikmin.end()) {
                        result = p->second;
                    }
                    break;
                }
            }
            return result;
        };
        
        PikminType* newPikminType = nullptr;
        
        if(typeArg == "same") {
            if(mgr.lastPikminType) {
                newPikminType = mgr.lastPikminType;
            } else {
                newPikminType = getNextType();
            }
        } else if(typeArg == "next") {
            newPikminType = getNextType();
        } else {
            const auto it = game.content.mobTypes.list.pikmin.find(typeArg);
            if(it != game.content.mobTypes.list.pikmin.end()) {
                newPikminType = it->second;
            } else {
                game.console.write(
                    LOG_LEVEL_ERROR,
                    "Unknown Pikmin type \"" + typeArg + "\"!"
                );
                return false;
            }
        }
        
        mgr.lastPikminType = newPikminType;
        
        createMob(
            game.mobCategories.get(MOB_CATEGORY_PIKMIN),
            game.states.gameplay->players[0].view.mouseCursorWorldPos,
            newPikminType, 0,
            "maturity=" + i2s(maturity)
        );
        
    }
    
    return true;
}


/**
 * @brief Code for the teleport maker tool command.
 *
 * @param mgr Maker tool manager.
 * @param args Arguments passed to the command.
 * @return Whether it succeeded.
 */
bool MakerToolRunners::teleport(
    MakerTools& mgr, const vector<string>& args
) {
    //Get the arguments.
    const string& inspectedArg = args[0];
    const string& leaveGroupArg = args[1];
    
    //Main logic.
    Mob* mobToTeleport =
        (s2b(inspectedArg) && mgr.inspectedMob) ?
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
        if(!s2b(leaveGroupArg)) {
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
 * @brief Returns whether the current context is valid for the given
 * context flags.
 *
 * @param flags The flags.
 * @return Whether it is valid.
 */
bool MakerTools::checkContextFlags(Bitmask8 flags) const {
    int flagBitmask = getIdxBitmask(getContext());
    return hasFlag(flags, flagBitmask);
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
            string info =
                "You've tried to use a maker tool, which isn't fit for "
                "normal gameplay.\n"
                "Are you sure? Try again three times in a row to confirm.";
            game.console.write(LOG_LEVEL_INFO, info);
            playConfirmationTimer = MAKER_TOOLS::PLAY_CONFIRMATION_TIMER;
            playConfirmationPresses++;
            return false;
            
        } else if(playConfirmationPresses == 2) {
            string info =
                "Maker tools are now allowed till you next enter an area.\n"
                "Check the options to always allow maker tools in normal "
                "gameplay.";
            game.console.write(LOG_LEVEL_INFO, info);
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
 * @brief Returns what the current context is.
 *
 * @return The context.
 */
MAKER_TOOL_CONTEXT MakerTools::getContext() const {
    return
        game.getCurStateName() == game.states.gameplay->getName() ?
        MAKER_TOOL_CONTEXT_GAMEPLAY :
        MAKER_TOOL_CONTEXT_OUT_OF_GAMEPLAY;
}


/**
 * @brief Returns the name of the current context, in lowercase.
 *
 * @return The name.
 */
string MakerTools::getContextName() const {
    switch(getContext()) {
    case MAKER_TOOL_CONTEXT_OUT_OF_GAMEPLAY: {
        return "out-of-gameplay";
        break;
    } case MAKER_TOOL_CONTEXT_GAMEPLAY: {
        return "gameplay";
        break;
    }
    }
    return "";
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
    case PLAYER_ACTION_TYPE_MT_CREATE_REMINDER: {
        toolToRun = MAKER_TOOL_TYPE_CREATE_REMINDER;
        break;
    } case PLAYER_ACTION_TYPE_MT_CONTROL_CAM: {
        toolToRun = MAKER_TOOL_TYPE_CONTROL_CAM;
        args.push_back(
            mod1 ?
            enumGetName(
                makerToolControlCamActionINames,
                MAKER_TOOL_CONTROL_CAM_ACTION_FREEZE
            ) :
            enumGetName(
                makerToolControlCamActionINames,
                MAKER_TOOL_CONTROL_CAM_ACTION_MODE
            )
        );
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
    } case PLAYER_ACTION_TYPE_MT_HIDE_HUD: {
        toolToRun = MAKER_TOOL_TYPE_HIDE_HUD;
        break;
    } case PLAYER_ACTION_TYPE_MT_HURT_MOB: {
        toolToRun = MAKER_TOOL_TYPE_HURT_MOB;
        unsigned char settingIdx = getMakerToolSettingIdx();
        args.push_back(f2s(hurtMobSettings[settingIdx]));
        break;
    } case PLAYER_ACTION_TYPE_MT_INSPECT_AREA: {
        toolToRun = MAKER_TOOL_TYPE_INSPECT_AREA;
        break;
    } case PLAYER_ACTION_TYPE_MT_INSPECT_MOB: {
        toolToRun = MAKER_TOOL_TYPE_INSPECT_MOB;
        args.push_back(
            mod1 ?
            enumGetName(
                makerToolInspectMobActionINames,
                MAKER_TOOL_INSPECT_MOB_ACTION_ITERATE
            ) :
            mod2 ?
            enumGetName(
                makerToolInspectMobActionINames,
                MAKER_TOOL_INSPECT_MOB_ACTION_STOP
            ) :
            enumGetName(
                makerToolInspectMobActionINames,
                MAKER_TOOL_INSPECT_MOB_ACTION_GET_CLOSEST
            )
        );
        break;
    } case PLAYER_ACTION_TYPE_MT_SAVE_AREA_IMAGE: {
        toolToRun = MAKER_TOOL_TYPE_SAVE_AREA_IMAGE;
        unsigned char settingIdx = getMakerToolSettingIdx();
        args.push_back(i2s(saveAreaImageSettings[settingIdx].size));
        args.push_back(i2s(saveAreaImageSettings[settingIdx].padding));
        args.push_back(b2s(saveAreaImageSettings[settingIdx].shadows));
        break;
    } case PLAYER_ACTION_TYPE_MT_SET_SPEED: {
        toolToRun = MAKER_TOOL_TYPE_SET_SPEED;
        unsigned char settingIdx = getMakerToolSettingIdx();
        args.push_back(f2s(setSpeedSettings[settingIdx]));
        break;
    } case PLAYER_ACTION_TYPE_MT_SHOW_COLLISION: {
        toolToRun = MAKER_TOOL_TYPE_SHOW_COLLISION;
        break;
    } case PLAYER_ACTION_TYPE_MT_SHOW_GEOMETRY_INFO: {
        toolToRun = MAKER_TOOL_TYPE_SHOW_GEOMETRY_INFO;
        break;
    } case PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES: {
        toolToRun = MAKER_TOOL_TYPE_SHOW_HITBOXES;
        break;
    } case PLAYER_ACTION_TYPE_MT_SHOW_PATH_INFO: {
        toolToRun = MAKER_TOOL_TYPE_SHOW_PATH_INFO;
        break;
    } case PLAYER_ACTION_TYPE_MT_SHOW_REACHES: {
        toolToRun = MAKER_TOOL_TYPE_SHOW_REACHES;
        break;
    } case PLAYER_ACTION_TYPE_MT_SPAWN_PIKMIN: {
        toolToRun = MAKER_TOOL_TYPE_SPAWN_PIKMIN;
        args.push_back(mod1 ? "same" : "next");
        args.push_back("1");
        args.push_back(
            mod2 ?
            enumGetName(maturityINames, MATURITY_LEAF) :
            enumGetName(maturityINames, MATURITY_FLOWER)
        );
        break;
    } case PLAYER_ACTION_TYPE_MT_TELEPORT: {
        toolToRun = MAKER_TOOL_TYPE_TELEPORT;
        args.push_back(b2s(mod1));
        args.push_back(b2s(mod2));
        break;
    }
    }
    
    if(toolToRun != MAKER_TOOL_TYPE_NONE) {
        runTool(toolToRun, args);
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
    if(isPlayerWritingText()) return false;
    if(!checkMakerToolsAllowed(action.value)) return true;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_CONSOLE: {

        if(action.value < 0.5f) return false;
        
        game.console.toggleTerminal();
        break;
        
    } case PLAYER_ACTION_TYPE_MT_SET_AUTO_START: {

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
    
    //Save area image.
    {
        DataNode* areaImageNode = node->getChildByName("area_image");
        DataNode* settingsNodes[3] {
            areaImageNode->getChildByName("normal_settings"),
            areaImageNode->getChildByName("mod_1_settings"),
            areaImageNode->getChildByName("mod_2_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter aRS(settingsNodes[s]);
            
            aRS.set("size", saveAreaImageSettings[s].size);
            aRS.set("padding", saveAreaImageSettings[s].padding);
            aRS.set("shadows", saveAreaImageSettings[s].shadows);
        }
    }
    
    //Set auto-start.
    {
        DataNode* autoStartNode = node->getChildByName("auto_start");
        ReaderSetter aRS(autoStartNode);
        
        aRS.set("state", autoStartState);
        aRS.set("option", autoStartOption);
    }
    
    //Set speed.
    {
        DataNode* changeSpeedNode = node->getChildByName("change_speed");
        DataNode* settingsNodes[3] {
            changeSpeedNode->getChildByName("normal_settings"),
            changeSpeedNode->getChildByName("mod_1_settings"),
            changeSpeedNode->getChildByName("mod_2_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter sRS(settingsNodes[s]);
            
            sRS.set("multiplier", setSpeedSettings[s]);
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
            hRS.set("percentage", hurtMobSettings[s], &percentageNode);
            if(percentageNode) {
                hurtMobSettings[s] /= 100.0f;
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
    changingSpeed = false;
    inFrameAdvanceMode = false;
    showingCollision = false;
    showingGeometryInfo = false;
    showingHitboxes = false;
    showingHud = true;
    inspectingArea = false;
    inspectedMob = nullptr;
    lastPikminType = nullptr;
    showingPathInfo = false;
    showingReaches = false;
    
    usedHelpingTools = false;
    allowedInPlayNow = false;
    playConfirmationPresses = 0;
    playConfirmationTimer = 0.0f;
}


/**
 * @brief Runs a specific tool, given its name.
 *
 * @param typeName Internal name of the tool type.
 * @param args Arguments.
 * @return Whether it succeeded.
 */
bool MakerTools::runTool(const string& typeName, const vector<string>& args) {
    MAKER_TOOL_TYPE typeToRun = MAKER_TOOL_TYPE_NONE;
    
    forIdx(t, types) {
        const MakerToolType& type = game.makerTools.types[t];
        if(type.type == MAKER_TOOL_TYPE_NONE) continue;
        if(type.name == typeName) {
            typeToRun = type.type;
            break;
        }
    }
    
    //Check if it is recognized.
    if(typeToRun == MAKER_TOOL_TYPE_NONE) {
        game.console.write(
            LOG_LEVEL_ERROR, "Unknown maker tool name \"" + typeName + "\"!"
        );
        return false;
    }
    
    return runTool(typeToRun, args);
}


/**
 * @brief Runs a specific tool.
 *
 * @param type The tool type.
 * @param inputArgs Arguments.
 * @return Whether it succeeded.
 */
bool MakerTools::runTool(
    MAKER_TOOL_TYPE type, const vector<string>& inputArgs
) {
    MakerToolType* typePtr = &types[type];
    
    //Check if this action is allowed in the current context.
    if(!checkContextFlags(typePtr->contexts)) {
        game.log.write(
            LOG_LEVEL_ERROR,
            "The maker tool \"" + typePtr->name + "\" can't be used in " +
            aAn(getContextName()) + " context!"
        );
        return false;
    }
    
    vector<string> args;
    vector<bool> dummyArgIsVar;
    if(
        !compileCommandArgs(
            typePtr->parameters, inputArgs,
            "The \"" + typePtr->name + "\" maker tool", false,
            &args, &dummyArgIsVar
        )
    ) {
        return false;
    }
    
    bool success = types[type].code(*this, args);
    
    if(types[type].helpful) {
        usedHelpingTools = true;
    }
    
    return success;
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
    
    //Save area image.
    {
        DataNode* areaImageNode = node->addNew("area_image");
        DataNode* settingsNodes[3] {
            areaImageNode->addNew("normal_settings"),
            areaImageNode->addNew("mod_1_settings"),
            areaImageNode->addNew("mod_2_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            GetterWriter sGW(settingsNodes[s]);
            
            sGW.write("size", saveAreaImageSettings[s].size);
            sGW.write("padding", saveAreaImageSettings[s].padding);
            sGW.write("shadows", saveAreaImageSettings[s].shadows);
        }
    }
    
    //Set auto-start.
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
            
            sGW.write("multiplier", setSpeedSettings[s]);
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
            
            sGW.write("percentage", hurtMobSettings[s] * 100.0f);
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
