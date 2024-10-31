/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Content type manager classes and related functions.
 */

#include <algorithm>

#include "content_type_manager.h"

#include "load.h"
#include "functions.h"
#include "game.h"
#include "init.h"
#include "utils/allegro_utils.h"
#include "utils/string_utils.h"


/**
 * @brief Clears the manifests.
 */
void area_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void area_content_manager::fill_manifests() {
    for(size_t t = 0; t < N_AREA_TYPES; t++) {
        manifests.push_back(map<string, content_manifest>());
        fill_manifests_map(
            manifests[t],
            t == AREA_TYPE_SIMPLE ?
            FOLDER_PATHS_FROM_PKG::SIMPLE_AREAS :
            FOLDER_PATHS_FROM_PKG::MISSION_AREAS,
            true
        );
    }
}


/**
 * @brief Returns the manifest matching the specified area, or nullptr if none
 * was found.
 * 
 * @param area_name Name of the area.
 * @param package Package it belongs to.
 * @param type Area type.
 * @return The manifest, or nullptr.
 */
content_manifest* area_content_manager::find_manifest(
    const string& area_name, const string& package, AREA_TYPE type
) {
    for(auto& m : manifests[type]) {
        if(m.first == area_name && m.second.package == package) {
            return &m.second;
        }
    }
    return nullptr;
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string area_content_manager::get_name() const {
    return "area";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string area_content_manager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void area_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(size_t t = 0; t < N_AREA_TYPES; t++) {
        list.push_back(vector<area_data*>());
        for(auto &a : manifests[t]) {
            load_area_into_vector(&a.second, (AREA_TYPE) t, false);
        }
    }
}


/**
 * @brief Loads an area.
 *
 * @param area_ptr Object to load into.
 * @param manifest Manifest of the area.
 * @param type Type of area this is.
 * @param level Level to load at.
 * @param from_backup If true, load from a backup, if any.
 */
void area_content_manager::load_area(
    area_data* area_ptr, content_manifest* manifest, AREA_TYPE type,
    CONTENT_LOAD_LEVEL level, bool from_backup
) {
    //Setup.
    string user_data_path =
        FOLDER_PATHS_FROM_ROOT::AREA_USER_DATA + "/" +
        manifest->package + "/" +
        (
            type == AREA_TYPE_SIMPLE ?
            FOLDER_NAMES::SIMPLE_AREAS :
            FOLDER_NAMES::MISSION_AREAS
        );
    string base_folder_path = from_backup ? user_data_path : manifest->path;
    
    string data_file_path = base_folder_path + "/" + FILE_NAMES::AREA_MAIN_DATA;
    data_node data_file = load_data_file(data_file_path);
    if(!data_file.file_was_opened) return;

    string geometry_file_path = base_folder_path + "/" + FILE_NAMES::AREA_GEOMETRY;
    data_node geometry_file = load_data_file(geometry_file_path);
    if(!geometry_file.file_was_opened) return;
    
    area_ptr->manifest = manifest;
    area_ptr->type = type;
    area_ptr->user_data_path = user_data_path;
    
    //Main data.
    if(game.perf_mon) game.perf_mon->start_measurement("Area -- Data");
    area_ptr->load_main_data_from_data_node(&data_file, level);
    area_ptr->load_mission_data_from_data_node(&data_file);
    if(game.perf_mon) game.perf_mon->finish_measurement();
    
    //Loading screen.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        if(game.loading_text_bmp) al_destroy_bitmap(game.loading_text_bmp);
        if(game.loading_subtext_bmp) al_destroy_bitmap(game.loading_subtext_bmp);
        game.loading_text_bmp = nullptr;
        game.loading_subtext_bmp = nullptr;
        draw_loading_screen(
            area_ptr->name,
            get_subtitle_or_mission_goal(
                area_ptr->subtitle,
                area_ptr->type,
                area_ptr->mission.goal
            ),
            1.0f
        );
        al_flip_display();
    }
    
    //Thumbnail image.
    string thumbnail_path = base_folder_path + FILE_NAMES::AREA_THUMBNAIL;
    area_ptr->load_thumbnail(thumbnail_path);
    
    //Geometry.
    if(level >= CONTENT_LOAD_LEVEL_EDITOR) {
        if(game.perf_mon) game.perf_mon->start_measurement("Area -- Geometry");
        area_ptr->load_geometry_from_data_node(&geometry_file, level);
        if(game.perf_mon) game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads an area into the vector of areas. This does not load it as the
 * "current" area.
 *
 * @param manifest Manifest of the area.
 * @param type Type of area this is.
 * @param from_backup If true, load from a backup, if any.
 */
void area_content_manager::load_area_into_vector(
    content_manifest* manifest, AREA_TYPE type,
    bool from_backup
) {
    area_data* new_area = new area_data();
    list[type].push_back(new_area);
    load_area(
        new_area, manifest, type,
        CONTENT_LOAD_LEVEL_BASIC, from_backup
    );
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void area_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(size_t t = 0; t < list.size(); t++) {
        for(size_t a = 0; a < list[t].size(); a++) {
            delete list[t][a];
        }
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void bitmap_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void bitmap_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::GRAPHICS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string bitmap_content_manager::get_name() const {
    return "bitmap";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string bitmap_content_manager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void bitmap_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void bitmap_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Fills in a given manifests map.
 *
 * @param manifests Manifests map to fill.
 * @param content_rel_path Path to the content, relative to the start
 * of the package.
 * @param folders True if the content is folders, false if it's files.
 */
void content_type_manager::fill_manifests_map(
    map<string, content_manifest> &manifests, const string &content_rel_path, bool folders
) {
    fill_manifests_map_from_pkg(manifests, FOLDER_NAMES::BASE_PKG, content_rel_path, folders);
}


/**
 * @brief Fills in a given manifests map from within a package folder.
 *
 * @param manifests Manifests map to fill.
 * @param package_name Name of the package folder.
 * @param content_rel_path Path to the content, relative to the start
 * of the package.
 * @param folders True if the content is folders, false if it's files.
 */
void content_type_manager::fill_manifests_map_from_pkg(
    map<string, content_manifest> &manifests, const string &package_name,
    const string &content_rel_path, bool folders
) {
    const string folder_path =
        FOLDER_PATHS_FROM_ROOT::GAME_DATA + "/" + package_name +
        "/" + content_rel_path;
        
    vector<string> items =
        folder_to_vector_recursively(folder_path, folders);
        
    for(size_t i = 0; i < items.size(); i++) {
        string internal_name = remove_extension(items[i]);
        manifests[internal_name] = content_manifest(internal_name, folder_path + "/" + items[i], package_name);
    }
}


/**
 * @brief Clears the manifests.
 */
void custom_particle_gen_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void custom_particle_gen_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::PARTICLE_GENERATORS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string custom_particle_gen_content_manager::get_name() const {
    return "particle generator";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string custom_particle_gen_content_manager::get_perf_mon_measurement_name() const {
    return "Custom particle generators";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void custom_particle_gen_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &g : manifests) {
        load_generator(&g.second, level);
    }
}


/**
 * @brief Loads a user-made particle generator.
 *
 * @param manifest Manifest of the particle generator.
 * @param level Level to load at.
 */
void custom_particle_gen_content_manager::load_generator(
    content_manifest* manifest, CONTENT_LOAD_LEVEL level
) {
    data_node file = load_data_file(manifest->path);
    if(!file.file_was_opened) return;
    
    particle_generator new_pg;
    new_pg.manifest = manifest;
    new_pg.load_from_data_node(&file, level);
    list[manifest->internal_name] = new_pg;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void custom_particle_gen_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(auto g = list.begin(); g != list.end(); ++g) {
        game.content.bitmaps.list.free(g->second.base_particle.bitmap);
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void global_anim_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void global_anim_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::GLOBAL_ANIMATIONS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string global_anim_content_manager::get_name() const {
    return "global animation";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string global_anim_content_manager::get_perf_mon_measurement_name() const {
    return "Global animations";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void global_anim_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &a : manifests) {
        load_global_animation(&a.second, level);
    }
}


/**
 * @brief Loads a global animation.
 *
 * @param manifest Manifest of the animation.
 * @param level Level to load at.
 */
void global_anim_content_manager::load_global_animation(content_manifest* manifest, CONTENT_LOAD_LEVEL level) {
    data_node file(manifest->path);
    single_animation_suite anim;
    anim.database.manifest = manifest;
    anim.database.load_from_data_node(&file);
    anim.instance.cur_anim = anim.database.animations[0];
    anim.instance.to_start();
    list[manifest->internal_name] = anim;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void global_anim_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void gui_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void gui_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::GUI, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string gui_content_manager::get_name() const {
    return "GUI definition";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string gui_content_manager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void gui_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(const auto &g : manifests) {
        list[g.first] = load_data_file(g.second.path);
    }
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void gui_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void hazard_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void hazard_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::HAZARDS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string hazard_content_manager::get_name() const {
    return "hazard";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string hazard_content_manager::get_perf_mon_measurement_name() const {
    return "Hazards";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void hazard_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &h : manifests) {
        load_hazard(&h.second, level);
    }
}


/**
 * @brief Loads a hazard.
 *
 * @param manifest Manifest of the hazard.
 * @param level Level to load at.
 */
void hazard_content_manager::load_hazard(
    content_manifest* manifest, CONTENT_LOAD_LEVEL level
) {
    data_node file = load_data_file(manifest->path);
    if(!file.file_was_opened) return;
    
    hazard new_h;
    new_h.manifest = manifest;
    new_h.load_from_data_node(&file);
    list[manifest->internal_name] = new_h;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void hazard_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void liquid_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void liquid_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::LIQUIDS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string liquid_content_manager::get_name() const {
    return "liquid";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string liquid_content_manager::get_perf_mon_measurement_name() const {
    return "Liquids";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void liquid_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &l : manifests) {
        load_liquid(&l.second, level);
    }
}


/**
 * @brief Loads a liquid.
 *
 * @param manifest Manifest of the liquid.
 * @param level Level to load at.
 */
void liquid_content_manager::load_liquid(
    content_manifest* manifest, CONTENT_LOAD_LEVEL level
) {
    data_node file = load_data_file(manifest->path);
    if(!file.file_was_opened) return;
    
    liquid* new_l = new liquid();
    new_l->manifest = manifest;
    new_l->load_from_data_node(&file, level);
    list[manifest->internal_name] = new_l;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void liquid_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(const auto& l : list) {
        delete l.second;
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void misc_config_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void misc_config_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::MISC, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string misc_config_content_manager::get_name() const {
    return "misc. config";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string misc_config_content_manager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void misc_config_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    //Game config.
    data_node game_config_file =
        load_data_file(manifests[FILE_NAMES::GAME_CONFIG].path);
    game.config.load(&game_config_file);
    al_set_window_title(
        game.display,
        game.config.name.empty() ? "Pikifen" : game.config.name.c_str()
    );
    
    //System animations.
    data_node system_animations_file =
        load_data_file(manifests[FILE_NAMES::SYSTEM_ANIMS].path);
    init_single_animation(
        &system_animations_file, "leader_damage_sparks",
        game.sys_assets.spark_animation
    );
    
    //System asset file names.
    data_node system_asset_fn_file =
        load_data_file(manifests[FILE_NAMES::SYSTEM_ASSET_FILE_NAMES].path);
    game.asset_file_names.load(&system_asset_fn_file);
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void misc_config_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Clears the manifests.
 */
void mob_type_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void mob_type_content_manager::fill_manifests() {
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        manifests.push_back(map<string, content_manifest>());
        fill_manifests_map(
            manifests[c],
            FOLDER_PATHS_FROM_PKG::MOB_TYPES + "/" +
            game.mob_categories.get((MOB_CATEGORY) c)->folder_name,
            true
        );
    }
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string mob_type_content_manager::get_name() const {
    return "mob type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string mob_type_content_manager::get_perf_mon_measurement_name() const {
    return "Mob types";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void mob_type_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    //Load the categorized mob types.
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        if(c == MOB_CATEGORY_NONE) {
            continue;
        }
        
        mob_category* category = game.mob_categories.get((MOB_CATEGORY) c);
        if(game.perf_mon) {
            game.perf_mon->start_measurement(
                "Object types -- " + category->name
            );
        }
        
        load_mob_types_of_category(category, level);
        
        if(game.perf_mon) {
            game.perf_mon->finish_measurement();
        }
    }
    
    //Pikmin type order.
    vector<string> missing_pikmin_order_types;
    for(auto &p : list.pikmin) {
        if(
            std::find(
                game.config.pikmin_order_strings.begin(),
                game.config.pikmin_order_strings.end(),
                p.first
            ) == game.config.pikmin_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_pikmin_order_types.push_back(p.first);
        }
    }
    if(!missing_pikmin_order_types.empty()) {
        std::sort(
            missing_pikmin_order_types.begin(),
            missing_pikmin_order_types.end()
        );
        game.config.pikmin_order_strings.insert(
            game.config.pikmin_order_strings.end(),
            missing_pikmin_order_types.begin(),
            missing_pikmin_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.pikmin_order_strings.size(); o++) {
        string s = game.config.pikmin_order_strings[o];
        if(list.pikmin.find(s) != list.pikmin.end()) {
            game.config.pikmin_order.push_back(list.pikmin[s]);
        } else {
            game.errors.report(
                "Unknown Pikmin type \"" + s + "\" found "
                "in the Pikmin order list in the config file!"
            );
        }
    }
    
    //Leader type order.
    vector<string> missing_leader_order_types;
    for(auto &l : list.leader) {
        if(
            find(
                game.config.leader_order_strings.begin(),
                game.config.leader_order_strings.end(),
                l.first
            ) == game.config.leader_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_leader_order_types.push_back(l.first);
        }
    }
    if(!missing_leader_order_types.empty()) {
        std::sort(
            missing_leader_order_types.begin(),
            missing_leader_order_types.end()
        );
        game.config.leader_order_strings.insert(
            game.config.leader_order_strings.end(),
            missing_leader_order_types.begin(),
            missing_leader_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.leader_order_strings.size(); o++) {
        string s = game.config.leader_order_strings[o];
        if(list.leader.find(s) != list.leader.end()) {
            game.config.leader_order.push_back(list.leader[s]);
        } else {
            game.errors.report(
                "Unknown leader type \"" + s + "\" found "
                "in the leader order list in the config file!"
            );
        }
    }
    
    //Create the special mob types.
    create_special_mob_types();
}


/**
 * @brief Loads the mob types from a category's folder.
 *
 * @param category Pointer to the mob category.
 * @param level Level to load at.
 */
void mob_type_content_manager::load_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level) {
    if(category->folder_name.empty()) return;
    
    map<string, content_manifest> &man = manifests[category->id];
    for(auto &t : man) {
        data_node file(t.second.path + "/data.txt");
        if(!file.file_was_opened) continue;
        
        mob_type* mt;
        mt = category->create_type();
        mt->load_from_data_node(&file, level, t.second.path);
        category->register_type(t.first, mt);
        mt->manifest = &t.second;
        
    }
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void mob_type_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    game.config.leader_order.clear();
    game.config.pikmin_order.clear();
    
    for(size_t c = 0; c < N_MOB_CATEGORIES; c++) {
        mob_category* category = game.mob_categories.get((MOB_CATEGORY) c);
        unload_mob_types_of_category(category, level);
    }
}


/**
 * @brief Unloads a type of mob.
 *
 * @param mt Mob type to unload.
 * @param level Should match the level at which the content got loaded.
 */
void mob_type_content_manager::unload_mob_type(mob_type* mt, CONTENT_LOAD_LEVEL level) {
    for(size_t s = 0; s < mt->sounds.size(); s++) {
        ALLEGRO_SAMPLE* s_ptr = mt->sounds[s].sample;
        if(!s) continue;
        game.content.samples.list.free(s_ptr);
    }
    if(level >= CONTENT_LOAD_LEVEL_FULL) {
        mt->anims.destroy();
        unload_script(mt);
        
        mt->unload_resources();
    }
}


/**
 * @brief Unloads all loaded types of mob from a category.
 *
 * @param category Pointer to the mob category.
 * @param level Should match the level at which the content got loaded.
 */
void mob_type_content_manager::unload_mob_types_of_category(mob_category* category, CONTENT_LOAD_LEVEL level) {

    vector<string> type_names;
    category->get_type_names(type_names);
    
    for(size_t t = 0; t < type_names.size(); t++) {
        mob_type* mt = category->get_type(type_names[t]);
        unload_mob_type(mt, level);
    }
    
    category->clear_types();
}


/**
 * @brief Clears the manifests.
 */
void sample_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void sample_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::SOUNDS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string sample_content_manager::get_name() const {
    return "audio sample";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string sample_content_manager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void sample_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void sample_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Clears the manifests.
 */
void song_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void song_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::SONGS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string song_content_manager::get_name() const {
    return "song";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string song_content_manager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void song_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        load_song(&s.second, level);
    }
}


/**
 * @brief Loads a song.
 *
 * @param manifest Manifest of the song.
 * @param level Level to load at.
 */
void song_content_manager::load_song(content_manifest* manifest, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(manifest->path);
    if(!file.file_was_opened) return;
    
    song new_song;
    new_song.manifest = manifest;
    new_song.load_from_data_node(&file);
    list[manifest->internal_name] = new_song;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void song_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(auto &s : list) {
        s.second.unload();
    }
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void song_track_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void song_track_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::SONG_TRACKS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string song_track_content_manager::get_name() const {
    return "song track";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string song_track_content_manager::get_perf_mon_measurement_name() const {
    return "";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void song_track_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void song_track_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    
}


/**
 * @brief Clears the manifests.
 */
void spike_damage_type_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void spike_damage_type_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::SPIKE_DAMAGES_TYPES, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string spike_damage_type_content_manager::get_name() const {
    return "spike damage type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string spike_damage_type_content_manager::get_perf_mon_measurement_name() const {
    return "Spike damage types";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void spike_damage_type_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        load_spike_damage_type(&s.second, level);
    }
}


/**
 * @brief Loads a spike damage type.
 *
 * @param manifest Manifest of the spike damage type.
 * @param level Level to load at.
 */
void spike_damage_type_content_manager::load_spike_damage_type(content_manifest* manifest, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(manifest->path);
    if(!file.file_was_opened) return;
    
    spike_damage_type new_t;
    new_t.manifest = manifest;
    new_t.load_from_data_node(&file);
    list[manifest->internal_name] = new_t;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void spike_damage_type_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void spray_type_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void spray_type_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::SPRAYS, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string spray_type_content_manager::get_name() const {
    return "spray type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string spray_type_content_manager::get_perf_mon_measurement_name() const {
    return "Spray types";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void spray_type_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &s : manifests) {
        load_spray_type(&s.second, level);
    }

    //Spray type order.
    vector<string> missing_spray_order_types;
    for(auto &s : list) {
        if(
            find(
                game.config.spray_order_strings.begin(),
                game.config.spray_order_strings.end(),
                s.first
            ) == game.config.spray_order_strings.end()
        ) {
            //Missing from the list? Add it to the "missing" pile.
            missing_spray_order_types.push_back(s.first);
        }
    }
    if(!missing_spray_order_types.empty()) {
        std::sort(
            missing_spray_order_types.begin(),
            missing_spray_order_types.end()
        );
        game.config.spray_order_strings.insert(
            game.config.spray_order_strings.end(),
            missing_spray_order_types.begin(),
            missing_spray_order_types.end()
        );
    }
    for(size_t o = 0; o < game.config.spray_order_strings.size(); o++) {
        string s = game.config.spray_order_strings[o];
        if(list.find(s) != list.end()) {
            game.config.spray_order.push_back(&list[s]);
        } else {
            game.errors.report(
                "Unknown spray type \"" + s + "\" found "
                "in the spray order list in the config file!"
            );
        }
    }
}


/**
 * @brief Loads a spray type.
 *
 * @param manifest Manifest of the spray type.
 * @param level Level to load at.
 */
void spray_type_content_manager::load_spray_type(content_manifest* manifest, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(manifest->path);
    if(!file.file_was_opened) return;
    
    spray_type new_t;
    new_t.manifest = manifest;
    new_t.load_from_data_node(&file, level);
    list[manifest->internal_name] = new_t;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void spray_type_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    for(const auto &s : list) {
        game.content.bitmaps.list.free(s.second.bmp_spray);
    }
    game.config.spray_order.clear();
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void status_type_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void status_type_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::STATUSES, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string status_type_content_manager::get_name() const {
    return "status type";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string status_type_content_manager::get_perf_mon_measurement_name() const {
    return "Status types";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void status_type_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    vector<status_type*> types_with_replacements;
    vector<string> types_with_replacements_names;

    for(auto &s : manifests) {
        load_status_type(&s.second, level);
    }
    
    for(auto &s : list) {
        if(!s.second->replacement_on_timeout_str.empty()) {
            types_with_replacements.push_back(s.second);
            types_with_replacements_names.push_back(
                s.second->replacement_on_timeout_str
            );
        }
    }
    
    for(size_t s = 0; s < types_with_replacements.size(); s++) {
        string rn = types_with_replacements_names[s];
        bool found = false;
        for(auto &s2 : list) {
            if(s2.first == rn) {
                types_with_replacements[s]->replacement_on_timeout =
                    s2.second;
                found = true;
                break;
            }
        }
        if(found) continue;
        
        game.errors.report(
            "The status effect type \"" +
            types_with_replacements[s]->name +
            "\" has a replacement effect called \"" + rn + "\", but there is "
            "no status effect with that name!"
        );
    }
}


/**
 * @brief Loads a status type.
 *
 * @param manifest Manifest of the status type.
 * @param level Level to load at.
 */
void status_type_content_manager::load_status_type(content_manifest* manifest, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(manifest->path);
    if(!file.file_was_opened) return;
    
    status_type* new_t = new status_type();
    new_t->manifest = manifest;
    new_t->load_from_data_node(&file, level);
    list[manifest->internal_name] = new_t;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void status_type_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}


/**
 * @brief Clears the manifests.
 */
void weather_condition_content_manager::clear_manifests() {
    manifests.clear();
}


/**
 * @brief Fills in the manifests.
 */
void weather_condition_content_manager::fill_manifests() {
    fill_manifests_map(manifests, FOLDER_PATHS_FROM_PKG::WEATHER, false);
}


/**
 * @brief Returns the content type's name.
 * 
 * @return The name.
 */
string weather_condition_content_manager::get_name() const {
    return "weather condition";
}


/**
 * @brief Returns the name to use for the performance monitor, if any.
 * 
 * @return The name.
 */
string weather_condition_content_manager::get_perf_mon_measurement_name() const {
    return "Weather conditions";
}


/**
 * @brief Loads all content in the manifests.
 * 
 * @param level Level to load at.
 */
void weather_condition_content_manager::load_all(CONTENT_LOAD_LEVEL level) {
    for(auto &w : manifests) {
        load_weather_condition(&w.second, level);
    }
}


/**
 * @brief Loads a status type.
 *
 * @param manifest Manifest of the weather condition.
 * @param level Level to load at.
 */
void weather_condition_content_manager::load_weather_condition(content_manifest* manifest, CONTENT_LOAD_LEVEL level) {
    data_node file = load_data_file(manifest->path);
    if(!file.file_was_opened) return;
    
    weather new_w;
    new_w.manifest = manifest;
    new_w.load_from_data_node(&file);
    list[manifest->internal_name] = new_w;
}


/**
 * @brief Unloads all loaded content.
 * 
 * @param level Load level. Should match the level used to load the content.
 */
void weather_condition_content_manager::unload_all(CONTENT_LOAD_LEVEL level) {
    list.clear();
}
