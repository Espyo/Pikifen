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
    bool is_shift_pressed = false;
    bool is_ctrl_pressed = false;
    getShiftCtrlAltState(
        &is_shift_pressed, &is_ctrl_pressed, nullptr
    );
    return
        is_shift_pressed ? 1 :
        is_ctrl_pressed ? 2 :
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
    bool is_gameplay_tool_action =
        game.controls.getPlayerActionType(action.actionTypeId).category ==
        PLAYER_ACTION_CAT_GAMEPLAY_MAKER_TOOLS;
    if(!is_gameplay_tool_action) return false;
    if(!enabled) return true;
    if(action.value < 0.5f) return false;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_AREA_IMAGE: {

        unsigned char setting_idx = getMakerToolSettingIdx();
        ALLEGRO_BITMAP* bmp =
            game.states.gameplay->drawToBitmap(
                game.makerTools.areaImageSettings[setting_idx]
            );
        string file_name =
            FOLDER_PATHS_FROM_ROOT::USER_DATA + "/area_" +
            sanitizeFileName(game.curAreaData->name) +
            "_" + getCurrentTime(true) + ".png";
            
        if(!al_save_bitmap(file_name.c_str(), bmp)) {
            game.errors.report(
                "Could not save the area onto an image,"
                " with the name \"" + file_name + "\"!"
            );
        }
        
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_CHANGE_SPEED: {

        unsigned char setting_idx =
            getMakerToolSettingIdx();
        bool final_state = false;
        if(!game.makerTools.changeSpeed) {
            final_state = true;
        } else {
            if(game.makerTools.changeSpeedSettingIdx != setting_idx) {
                final_state = true;
            }
        }
        
        if(final_state) {
            game.makerTools.changeSpeedSettingIdx = setting_idx;
        }
        game.makerTools.changeSpeed = final_state;
        
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

        unsigned char setting_idx = getMakerToolSettingIdx();
        Mob* m = getClosestMobToCursor(true);
        if(m) {
            m->setHealth(
                true, true,
                -game.makerTools.mobHurtingSettings[setting_idx]
            );
        }
        game.makerTools.usedHelpingTools = true;
        break;
        
    } case PLAYER_ACTION_TYPE_MT_MOB_INFO: {

        bool is_shift_pressed;
        bool is_ctrl_pressed;
        getShiftCtrlAltState(
            &is_shift_pressed, &is_ctrl_pressed, nullptr
        );
        
        Mob* prev_lock_mob = game.makerTools.infoLock;
        Mob* m;
        if(is_shift_pressed) {
            m = getNextMobNearCursor(prev_lock_mob, false);
        } else if(is_ctrl_pressed) {
            m = nullptr;
        } else {
            m = getClosestMobToCursor(false);
        }
        
        game.makerTools.infoLock = prev_lock_mob == m ? nullptr : m;
        if(
            prev_lock_mob != nullptr &&
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
            bool is_shift_pressed;
            bool is_ctrl_pressed;
            getShiftCtrlAltState(
                &is_shift_pressed, &is_ctrl_pressed, nullptr
            );
            
            bool must_use_last_type =
                (is_shift_pressed && game.makerTools.lastPikminType);
            PikminType* new_pikmin_type = nullptr;
            
            if(must_use_last_type) {
                new_pikmin_type = game.makerTools.lastPikminType;
            } else {
                new_pikmin_type =
                    game.content.mobTypes.list.pikmin.begin()->second;
                    
                auto p = game.content.mobTypes.list.pikmin.begin();
                for(; p != game.content.mobTypes.list.pikmin.end(); ++p) {
                    if(p->second == game.makerTools.lastPikminType) {
                        ++p;
                        if(p != game.content.mobTypes.list.pikmin.end()) {
                            new_pikmin_type = p->second;
                        }
                        break;
                    }
                }
                game.makerTools.lastPikminType = new_pikmin_type;
            }
            
            createMob(
                game.mobCategories.get(MOB_CATEGORY_PIKMIN),
                game.mouseCursor.wPos, new_pikmin_type, 0,
                is_ctrl_pressed ? "maturity=0" : "maturity=2"
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

        bool is_shift_pressed;
        getShiftCtrlAltState(&is_shift_pressed, nullptr, nullptr);
        
        Mob* mob_to_teleport =
            (is_shift_pressed && game.makerTools.infoLock) ?
            game.makerTools.infoLock :
            game.states.gameplay->curLeaderPtr;
            
        Sector* mouse_sector =
            getSector(game.mouseCursor.wPos, nullptr, true);
        if(mouse_sector && mob_to_teleport) {
            mob_to_teleport->chase(
                game.mouseCursor.wPos, mouse_sector->z,
                CHASE_FLAG_TELEPORT
            );
            game.cam.setPos(game.mouseCursor.wPos);
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
    bool is_global_tool_action =
        game.controls.getPlayerActionType(action.actionTypeId).category ==
        PLAYER_ACTION_CAT_GLOBAL_MAKER_TOOLS;
    if(!is_global_tool_action) return false;
    if(!enabled) return true;
    if(action.value < 0.5f) return false;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_AUTO_START: {

        string cur_state_name = game.getCurStateName();
        if(cur_state_name == game.states.animationEd->getName()) {
            autoStartState = "animation_editor";
            autoStartOption =
                game.states.animationEd->getOpenedContentPath();
        } else if(cur_state_name == game.states.areaEd->getName()) {
            autoStartState = "area_editor";
            autoStartOption =
                game.states.areaEd->getOpenedContentPath();
        } else if(cur_state_name == game.states.guiEd->getName()) {
            autoStartState = "gui_editor";
            autoStartOption =
                game.states.guiEd->getOpenedContentPath();
        } else if(cur_state_name == game.states.particleEd->getName()) {
            autoStartState = "particle_editor";
            autoStartOption =
                game.states.particleEd->getOpenedContentPath();
        } else if(cur_state_name == game.states.gameplay->getName()) {
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
        DataNode* controls_node = node->getChildByName("controls");
        game.controls.loadBindsFromDataNode(controls_node, 0);
    }
    
    //Area image.
    {
        DataNode* area_image_node = node->getChildByName("area_image");
        DataNode* settings_nodes[3] {
            area_image_node->getChildByName("main_settings"),
            area_image_node->getChildByName("shift_settings"),
            area_image_node->getChildByName("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter rs(settings_nodes[s]);
            rs.set("size", areaImageSettings[s].size);
            rs.set("padding", areaImageSettings[s].padding);
            rs.set("mobs", areaImageSettings[s].mobs);
            rs.set("shadows", areaImageSettings[s].shadows);
        }
    }
    
    //Auto start.
    {
        DataNode* auto_start_node = node->getChildByName("auto_start");
        ReaderSetter rs(auto_start_node);
        rs.set("state", autoStartState);
        rs.set("option", autoStartOption);
    }
    
    //Change speed.
    {
        DataNode* change_speed_node = node->getChildByName("change_speed");
        DataNode* settings_nodes[3] {
            change_speed_node->getChildByName("main_settings"),
            change_speed_node->getChildByName("shift_settings"),
            change_speed_node->getChildByName("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter rs(settings_nodes[s]);
            rs.set("multiplier", changeSpeedSettings[s]);
        }
    }
    
    //Hurt mob.
    {
        DataNode* hurt_mob_node = node->getChildByName("hurt_mob");
        DataNode* settings_nodes[3] {
            hurt_mob_node->getChildByName("main_settings"),
            hurt_mob_node->getChildByName("shift_settings"),
            hurt_mob_node->getChildByName("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            ReaderSetter rs(settings_nodes[s]);
            DataNode* n;
            rs.set("percentage", mobHurtingSettings[s], &n);
            if(n) {
                mobHurtingSettings[s] /= 100.0f;
            }
        }
    }
    
    //Performance monitor.
    {
        DataNode* perf_mon_node = node->getChildByName("performance_monitor");
        ReaderSetter rs(perf_mon_node);
        rs.set("enabled", usePerfMon);
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
    GetterWriter gw(node);
    
    //General.
    gw.get("enabled", enabled);
    
    //Area image.
    {
        DataNode* area_image_node = node->addNew("area_image");
        DataNode* settings_nodes[3] {
            area_image_node->addNew("main_settings"),
            area_image_node->addNew("shift_settings"),
            area_image_node->addNew("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            GetterWriter sgw(settings_nodes[s]);
            sgw.get("size", areaImageSettings[s].size);
            sgw.get("padding", areaImageSettings[s].padding);
            sgw.get("mobs", areaImageSettings[s].mobs);
            sgw.get("shadows", areaImageSettings[s].shadows);
        }
    }
    
    //Auto start.
    {
        DataNode* auto_start_node = node->addNew("auto_start");
        GetterWriter agw(auto_start_node);
        agw.get("state", autoStartState);
        agw.get("option", autoStartOption);
    }
    
    //Change speed.
    {
        DataNode* change_speed_node = node->addNew("change_speed");
        DataNode* settings_nodes[3] {
            change_speed_node->addNew("main_settings"),
            change_speed_node->addNew("shift_settings"),
            change_speed_node->addNew("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            GetterWriter sgw(settings_nodes[s]);
            sgw.get("multiplier", changeSpeedSettings[s]);
        }
    }
    
    //Hurt mob.
    {
        DataNode* hurt_mob_node = node->addNew("hurt_mob");
        DataNode* settings_nodes[3] {
            hurt_mob_node->addNew("main_settings"),
            hurt_mob_node->addNew("shift_settings"),
            hurt_mob_node->addNew("ctrl_settings")
        };
        for(unsigned char s = 0; s < 3; s++) {
            GetterWriter sgw(settings_nodes[s]);
            sgw.get("percentage", mobHurtingSettings[s] * 100.0f);
        }
    }
    
    //Performance monitor.
    {
        DataNode* perf_mon_node = node->addNew("performance_monitor");
        GetterWriter pgw(perf_mon_node);
        pgw.get("enabled", usePerfMon);
    }
}
