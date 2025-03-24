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
    info_print_timer = Timer(1.0f, [this] () { info_print_text.clear(); });
    for(size_t k = 0; k < 20; k++) {
        keys[k] = MAKER_TOOL_TYPE_NONE;
    }
    
    //Some defaults that are convenient to have on.
    keys[10] = MAKER_TOOL_TYPE_AREA_IMAGE;
    keys[11] = MAKER_TOOL_TYPE_CHANGE_SPEED;
    keys[12] = MAKER_TOOL_TYPE_TELEPORT;
    keys[13] = MAKER_TOOL_TYPE_HURT_MOB;
    keys[14] = MAKER_TOOL_TYPE_NEW_PIKMIN;
    keys[15] = MAKER_TOOL_TYPE_MOB_INFO;
    keys[16] = MAKER_TOOL_TYPE_GEOMETRY_INFO;
    keys[17] = MAKER_TOOL_TYPE_HITBOXES;
    keys[18] = MAKER_TOOL_TYPE_COLLISION;
    keys[19] = MAKER_TOOL_TYPE_HUD;
}


/**
 * @brief Returns which setting index to use for a settings-based maker tool,
 * depending on the keys that are currently pressed.
 *
 * @return The index.
 */
unsigned char MakerTools::get_maker_tool_setting_idx() const {
    bool is_shift_pressed = false;
    bool is_ctrl_pressed = false;
    get_shift_ctrl_alt_state(
        &is_shift_pressed, &is_ctrl_pressed, nullptr
    );
    return
        is_shift_pressed ? 1 :
        is_ctrl_pressed ? 2 :
        0;
}


/**
 * @brief Handles a player action and performs an input tool if possible.
 *
 * @param action The action.
 */
void MakerTools::handle_player_action(const PlayerAction &action) {
    if(!enabled) return;
    if(action.value < 0.5f) return;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MT_AREA_IMAGE: {

        if(game.states.gameplay->loaded) {
            unsigned char setting_idx = get_maker_tool_setting_idx();
            ALLEGRO_BITMAP* bmp =
                game.states.gameplay->draw_to_bitmap(
                    game.maker_tools.area_image_settings[setting_idx]
                );
            string file_name =
                FOLDER_PATHS_FROM_ROOT::USER_DATA + "/area_" +
                sanitize_file_name(game.cur_area_data->name) +
                "_" + get_current_time(true) + ".png";
                
            if(!al_save_bitmap(file_name.c_str(), bmp)) {
                game.errors.report(
                    "Could not save the area onto an image,"
                    " with the name \"" + file_name + "\"!"
                );
            }
            
            game.maker_tools.used_helping_tools = true;
        }
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_CHANGE_SPEED: {

        unsigned char setting_idx =
            get_maker_tool_setting_idx();
        bool final_state = false;
        if(!game.maker_tools.change_speed) {
            final_state = true;
        } else {
            if(game.maker_tools.change_speed_setting_idx != setting_idx) {
                final_state = true;
            }
        }
        
        if(final_state) {
            game.maker_tools.change_speed_setting_idx = setting_idx;
        }
        game.maker_tools.change_speed = final_state;
        
        game.maker_tools.used_helping_tools = true;
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_GEOMETRY_INFO: {

        game.maker_tools.geometry_info =
            !game.maker_tools.geometry_info;
        game.maker_tools.used_helping_tools = true;
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_HUD: {

        game.maker_tools.hud = !game.maker_tools.hud;
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_HURT_MOB: {

        unsigned char setting_idx = get_maker_tool_setting_idx();
        Mob* m = get_closest_mob_to_cursor(true);
        if(m) {
            m->set_health(
                true, true,
                -game.maker_tools.mob_hurting_settings[setting_idx]
            );
        }
        game.maker_tools.used_helping_tools = true;
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_MOB_INFO: {

        if(game.states.gameplay->loaded) {
            bool is_shift_pressed;
            bool is_ctrl_pressed;
            get_shift_ctrl_alt_state(
                &is_shift_pressed, &is_ctrl_pressed, nullptr
            );
            
            Mob* prev_lock_mob = game.maker_tools.info_lock;
            Mob* m;
            if(is_shift_pressed) {
                m = get_next_mob_near_cursor(prev_lock_mob, false);
            } else if(is_ctrl_pressed) {
                m = nullptr;
            } else {
                m = get_closest_mob_to_cursor(false);
            }
            
            game.maker_tools.info_lock = prev_lock_mob == m ? nullptr : m;
            if(
                prev_lock_mob != nullptr &&
                game.maker_tools.info_lock == nullptr
            ) {
                print_info("Mob: None.", 2.0f, 2.0f);
            }
            game.maker_tools.used_helping_tools = true;
        }
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_NEW_PIKMIN: {

        if(
            game.states.gameplay->loaded &&
            game.states.gameplay->mobs.pikmin_list.size() <
            game.config.max_pikmin_in_field
        ) {
            bool is_shift_pressed;
            bool is_ctrl_pressed;
            get_shift_ctrl_alt_state(
                &is_shift_pressed, &is_ctrl_pressed, nullptr
            );
            
            bool must_use_last_type =
                (is_shift_pressed && game.maker_tools.last_pikmin_type);
            PikminType* new_pikmin_type = nullptr;
            
            if(must_use_last_type) {
                new_pikmin_type = game.maker_tools.last_pikmin_type;
            } else {
                new_pikmin_type =
                    game.content.mob_types.list.pikmin.begin()->second;
                    
                auto p = game.content.mob_types.list.pikmin.begin();
                for(; p != game.content.mob_types.list.pikmin.end(); ++p) {
                    if(p->second == game.maker_tools.last_pikmin_type) {
                        ++p;
                        if(p != game.content.mob_types.list.pikmin.end()) {
                            new_pikmin_type = p->second;
                        }
                        break;
                    }
                }
                game.maker_tools.last_pikmin_type = new_pikmin_type;
            }
            
            create_mob(
                game.mob_categories.get(MOB_CATEGORY_PIKMIN),
                game.mouse_cursor.w_pos, new_pikmin_type, 0,
                is_ctrl_pressed ? "maturity=0" : "maturity=2"
            );
            game.maker_tools.used_helping_tools = true;
        }
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_PATH_INFO: {

        game.maker_tools.path_info = !game.maker_tools.path_info;
        game.maker_tools.used_helping_tools = true;
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_SET_SONG_POS_NEAR_LOOP: {

        game.audio.set_song_pos_near_loop();
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_SHOW_COLLISION: {

        game.maker_tools.collision =
            !game.maker_tools.collision;
        game.maker_tools.used_helping_tools = true;
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_SHOW_HITBOXES: {

        game.maker_tools.hitboxes =
            !game.maker_tools.hitboxes;
        game.maker_tools.used_helping_tools = true;
        break;
        
    }
    case PLAYER_ACTION_TYPE_MT_TELEPORT: {

        if(game.states.gameplay->loaded) {
            bool is_shift_pressed;
            get_shift_ctrl_alt_state(&is_shift_pressed, nullptr, nullptr);
            
            Mob* mob_to_teleport =
                (is_shift_pressed && game.maker_tools.info_lock) ?
                game.maker_tools.info_lock :
                game.states.gameplay->cur_leader_ptr;
                
            Sector* mouse_sector =
                get_sector(game.mouse_cursor.w_pos, nullptr, true);
            if(mouse_sector && mob_to_teleport) {
                mob_to_teleport->chase(
                    game.mouse_cursor.w_pos, mouse_sector->z,
                    CHASE_FLAG_TELEPORT
                );
                game.cam.set_pos(game.mouse_cursor.w_pos);
            }
            game.maker_tools.used_helping_tools = true;
        }
        break;
    }
    }
}


/**
 * @brief Loads all the settings from a data node.
 *
 * @param node The node.
 */
void MakerTools::load_from_data_node(DataNode* node) {
    //Whether maker tools are enabled.
    enabled = s2b(node->getChildByName("enabled")->value);
    
    //Controls.
    {
        DataNode* controls_node = node->getChildByName("controls");
        game.controls.load_binds_from_data_node(controls_node, 0);
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
            rs.set("size", area_image_settings[s].size);
            rs.set("padding", area_image_settings[s].padding);
            rs.set("mobs", area_image_settings[s].mobs);
            rs.set("shadows", area_image_settings[s].shadows);
        }
    }
    
    //Auto start.
    {
        DataNode* auto_start_node = node->getChildByName("auto_start");
        ReaderSetter rs(auto_start_node);
        rs.set("state", auto_start_state);
        rs.set("option", auto_start_option);
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
            rs.set("multiplier", change_speed_settings[s]);
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
            rs.set("percentage", mob_hurting_settings[s], &n);
            if(n) {
                mob_hurting_settings[s] /= 100.0f;
            }
        }
    }
    
    //Performance monitor.
    {
        DataNode* perf_mon_node = node->getChildByName("performance_monitor");
        ReaderSetter rs(perf_mon_node);
        rs.set("enabled", use_perf_mon);
    }
}


/**
 * @brief Resets the states of the tools so that players can play without any
 * tool affecting the experience.
 */
void MakerTools::reset_for_gameplay() {
    change_speed = false;
    collision = false;
    geometry_info = false;
    hitboxes = false;
    hud = true;
    info_lock = nullptr;
    last_pikmin_type = nullptr;
    path_info = false;
    used_helping_tools = false;
}


/**
 * @brief Saves all the settings to a data node.
 *
 * @param node The node.
 */
void MakerTools::save_to_data_node(DataNode* node) {
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
            sgw.get("size", area_image_settings[s].size);
            sgw.get("padding", area_image_settings[s].padding);
            sgw.get("mobs", area_image_settings[s].mobs);
            sgw.get("shadows", area_image_settings[s].shadows);
        }
    }
    
    //Auto start.
    {
        DataNode* auto_start_node = node->addNew("auto_start");
        GetterWriter agw(auto_start_node);
        agw.get("state", auto_start_state);
        agw.get("option", auto_start_option);
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
            sgw.get("multiplier", change_speed_settings[s]);
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
            sgw.get("percentage", mob_hurting_settings[s] * 100.0f);
        }
    }
    
    //Performance monitor.
    {
        DataNode* perf_mon_node = node->addNew("performance_monitor");
        GetterWriter pgw(perf_mon_node);
        pgw.get("enabled", use_perf_mon);
    }
}
