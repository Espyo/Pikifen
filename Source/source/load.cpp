/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Data loading and unloading functions.
 */

#include <algorithm>

#include <allegro5/allegro_ttf.h>

#include "load.h"

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "init.h"
#include "spike_damage.h"
#include "utils/allegro_utils.h"
#include "utils/general_utils.h"
#include "utils/string_utils.h"


using std::set;


/**
 * @brief Loads an area into memory.
 *
 * @param requested_area_folder_name Name of the folder where the
 * area's data is.
 * @param requested_area_type Type of area this is.
 * What folder it loads from depends on this value.
 * @param load_for_editor If true, skips loading some things that the
 * area editor won't need.
 * @param from_backup If true, load from a backup, if any.
 */
void load_area(
    const string &requested_area_folder_name,
    const AREA_TYPE requested_area_type,
    const bool load_for_editor, const bool from_backup
) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Data");
    }
    
    game.cur_area_data.clear();
    
    string geometry_file_name;
    string data_file_name;
    
    if(from_backup) {
        string base_folder =
            get_base_area_folder_path(requested_area_type, false) +
            "/" + requested_area_folder_name;
        geometry_file_name = base_folder + "/" + AREA_GEOMETRY_BACKUP_FILE_NAME;
        data_file_name = base_folder + "/" + AREA_DATA_BACKUP_FILE_NAME;
    } else {
        string base_folder =
            get_base_area_folder_path(requested_area_type, true) +
            "/" + requested_area_folder_name;
        geometry_file_name = base_folder + "/" + AREA_GEOMETRY_FILE_NAME;
        data_file_name = base_folder + "/" + AREA_DATA_FILE_NAME;
    }
    
    game.cur_area_data.folder_name = requested_area_folder_name;
    game.cur_area_data.path =
        get_base_area_folder_path(requested_area_type, true) +
        "/" + requested_area_folder_name;
    game.cur_area_data.type = requested_area_type;
    
    data_node data_file(data_file_name);
    reader_setter rs(&data_file);
    
    //Content metadata.
    game.cur_area_data.load_from_data_node(&data_file);
    
    //Area configuration data.
    data_node* weather_node = nullptr;
    data_node* song_node = nullptr;
    
    rs.set("subtitle", game.cur_area_data.subtitle);
    rs.set("difficulty", game.cur_area_data.difficulty);
    rs.set("spray_amounts", game.cur_area_data.spray_amounts);
    rs.set("song", game.cur_area_data.song_name, &song_node);
    rs.set("weather", game.cur_area_data.weather_name, &weather_node);
    rs.set("day_time_start", game.cur_area_data.day_time_start);
    rs.set("day_time_speed", game.cur_area_data.day_time_speed);
    rs.set("bg_bmp", game.cur_area_data.bg_bmp_file_name);
    rs.set("bg_color", game.cur_area_data.bg_color);
    rs.set("bg_dist", game.cur_area_data.bg_dist);
    rs.set("bg_zoom", game.cur_area_data.bg_bmp_zoom);
    
    load_area_mission_data(&data_file, game.cur_area_data.mission);
    
    if(game.loading_text_bmp) al_destroy_bitmap(game.loading_text_bmp);
    if(game.loading_subtext_bmp) al_destroy_bitmap(game.loading_subtext_bmp);
    game.loading_text_bmp = nullptr;
    game.loading_subtext_bmp = nullptr;
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    draw_loading_screen(
        game.cur_area_data.name,
        get_subtitle_or_mission_goal(
            game.cur_area_data.subtitle,
            game.cur_area_data.type,
            game.cur_area_data.mission.goal
        ),
        1.0f
    );
    al_flip_display();
    
    if(!load_for_editor) {
    
        if(game.perf_mon) {
            game.perf_mon->start_measurement("Area -- Initial assets");
        }
        
        if(game.cur_area_data.weather_name.empty()) {
            game.cur_area_data.weather_condition = weather();
            
        } else if(
            game.content.weather_conditions.find(game.cur_area_data.weather_name) ==
            game.content.weather_conditions.end()
        ) {
            game.errors.report(
                "Area \"" + requested_area_folder_name +
                "\" refers to an unknown weather condition, \"" +
                game.cur_area_data.weather_name + "\"!",
                weather_node
            );
            game.cur_area_data.weather_condition = weather();
            
        } else {
            game.cur_area_data.weather_condition =
                game.content.weather_conditions[game.cur_area_data.weather_name];
                
        }
        
        if(
            !game.cur_area_data.song_name.empty() &&
            game.audio.songs.find(game.cur_area_data.song_name) ==
            game.audio.songs.end()
        ) {
            game.errors.report(
                "Area \"" + requested_area_folder_name +
                "\" refers to an unknown song, \"" +
                game.cur_area_data.song_name + "\"!",
                song_node
            );
        }
    }
    
    if(!load_for_editor && !game.cur_area_data.bg_bmp_file_name.empty()) {
        game.cur_area_data.bg_bmp =
            game.textures.get(game.cur_area_data.bg_bmp_file_name, &data_file);
    }
    
    if(!load_for_editor && game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Time to load the geometry.
    data_node geometry_file = load_data_file(geometry_file_name);
    
    //Vertexes.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Vertexes");
    }
    
    size_t n_vertexes =
        geometry_file.get_child_by_name(
            "vertexes"
        )->get_nr_of_children_by_name("v");
    for(size_t v = 0; v < n_vertexes; ++v) {
        data_node* vertex_data =
            geometry_file.get_child_by_name(
                "vertexes"
            )->get_child_by_name("v", v);
        vector<string> words = split(vertex_data->value);
        if(words.size() == 2) {
            game.cur_area_data.vertexes.push_back(
                new vertex(s2f(words[0]), s2f(words[1]))
            );
        }
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Edges.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Edges");
    }
    
    size_t n_edges =
        geometry_file.get_child_by_name(
            "edges"
        )->get_nr_of_children_by_name("e");
    for(size_t e = 0; e < n_edges; ++e) {
        data_node* edge_data =
            geometry_file.get_child_by_name(
                "edges"
            )->get_child_by_name("e", e);
        edge* new_edge = new edge();
        
        vector<string> s_idxs = split(edge_data->get_child_by_name("s")->value);
        if(s_idxs.size() < 2) s_idxs.insert(s_idxs.end(), 2, "-1");
        for(size_t s = 0; s < 2; ++s) {
            if(s_idxs[s] == "-1") new_edge->sector_idxs[s] = INVALID;
            else new_edge->sector_idxs[s] = s2i(s_idxs[s]);
        }
        
        vector<string> v_idxs = split(edge_data->get_child_by_name("v")->value);
        if(v_idxs.size() < 2) v_idxs.insert(v_idxs.end(), 2, "0");
        
        new_edge->vertex_idxs[0] = s2i(v_idxs[0]);
        new_edge->vertex_idxs[1] = s2i(v_idxs[1]);
        
        data_node* shadow_length =
            edge_data->get_child_by_name("shadow_length");
        if(!shadow_length->value.empty()) {
            new_edge->wall_shadow_length =
                s2f(shadow_length->value);
        }
        
        data_node* shadow_color =
            edge_data->get_child_by_name("shadow_color");
        if(!shadow_color->value.empty()) {
            new_edge->wall_shadow_color =
                s2c(shadow_color->value);
        }
        
        data_node* smoothing_length =
            edge_data->get_child_by_name("smoothing_length");
        if(!smoothing_length->value.empty()) {
            new_edge->ledge_smoothing_length =
                s2f(smoothing_length->value);
        }
        
        data_node* smoothing_color =
            edge_data->get_child_by_name("smoothing_color");
        if(!smoothing_color->value.empty()) {
            new_edge->ledge_smoothing_color =
                s2c(smoothing_color->value);
        }
        
        game.cur_area_data.edges.push_back(new_edge);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Sectors.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Sectors");
    }
    
    size_t n_sectors =
        geometry_file.get_child_by_name(
            "sectors"
        )->get_nr_of_children_by_name("s");
    for(size_t s = 0; s < n_sectors; ++s) {
        data_node* sector_data =
            geometry_file.get_child_by_name(
                "sectors"
            )->get_child_by_name("s", s);
        sector* new_sector = new sector();
        
        size_t new_type =
            game.sector_types.get_idx(
                sector_data->get_child_by_name("type")->value
            );
        if(new_type == INVALID) {
            new_type = SECTOR_TYPE_NORMAL;
        }
        new_sector->type = (SECTOR_TYPE) new_type;
        new_sector->is_bottomless_pit =
            s2b(
                sector_data->get_child_by_name(
                    "is_bottomless_pit"
                )->get_value_or_default("false")
            );
        new_sector->brightness =
            s2f(
                sector_data->get_child_by_name(
                    "brightness"
                )->get_value_or_default(i2s(GEOMETRY::DEF_SECTOR_BRIGHTNESS))
            );
        new_sector->tag = sector_data->get_child_by_name("tag")->value;
        new_sector->z = s2f(sector_data->get_child_by_name("z")->value);
        new_sector->fade = s2b(sector_data->get_child_by_name("fade")->value);
        
        new_sector->texture_info.file_name =
            sector_data->get_child_by_name("texture")->value;
        new_sector->texture_info.rot =
            s2f(sector_data->get_child_by_name("texture_rotate")->value);
            
        vector<string> scales =
            split(sector_data->get_child_by_name("texture_scale")->value);
        if(scales.size() >= 2) {
            new_sector->texture_info.scale.x = s2f(scales[0]);
            new_sector->texture_info.scale.y = s2f(scales[1]);
        }
        vector<string> translations =
            split(sector_data->get_child_by_name("texture_trans")->value);
        if(translations.size() >= 2) {
            new_sector->texture_info.translation.x = s2f(translations[0]);
            new_sector->texture_info.translation.y = s2f(translations[1]);
        }
        new_sector->texture_info.tint =
            s2c(
                sector_data->get_child_by_name("texture_tint")->
                get_value_or_default("255 255 255")
            );
            
        if(!new_sector->fade && !new_sector->is_bottomless_pit) {
            new_sector->texture_info.bitmap =
                game.textures.get(new_sector->texture_info.file_name, nullptr);
        }
        
        data_node* hazards_node = sector_data->get_child_by_name("hazards");
        vector<string> hazards_strs =
            semicolon_list_to_vector(hazards_node->value);
        for(size_t h = 0; h < hazards_strs.size(); ++h) {
            string hazard_name = hazards_strs[h];
            if(game.content.hazards.find(hazard_name) == game.content.hazards.end()) {
                game.errors.report(
                    "Unknown hazard \"" + hazard_name +
                    "\"!", hazards_node
                );
            } else {
                new_sector->hazards.push_back(&(game.content.hazards[hazard_name]));
            }
        }
        new_sector->hazards_str = hazards_node->value;
        new_sector->hazard_floor =
            s2b(
                sector_data->get_child_by_name(
                    "hazards_floor"
                )->get_value_or_default("true")
            );
            
        game.cur_area_data.sectors.push_back(new_sector);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Mobs.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Object generators");
    }
    
    vector<std::pair<size_t, size_t> > mob_links_buffer;
    size_t n_mobs =
        geometry_file.get_child_by_name("mobs")->get_nr_of_children();
        
    for(size_t m = 0; m < n_mobs; ++m) {
    
        data_node* mob_node =
            geometry_file.get_child_by_name("mobs")->get_child(m);
            
        mob_gen* mob_ptr = new mob_gen();
        
        mob_ptr->pos = s2p(mob_node->get_child_by_name("p")->value);
        mob_ptr->angle =
            s2f(
                mob_node->get_child_by_name("angle")->get_value_or_default("0")
            );
        mob_ptr->vars = mob_node->get_child_by_name("vars")->value;
        
        string category_name = mob_node->name;
        string type_name;
        mob_category* category =
            game.mob_categories.get_from_name(category_name);
        if(category) {
            type_name = mob_node->get_child_by_name("type")->value;
            mob_ptr->type = category->get_type(type_name);
        } else {
            category = game.mob_categories.get(MOB_CATEGORY_NONE);
            mob_ptr->type = nullptr;
        }
        
        vector<string> link_strs =
            split(mob_node->get_child_by_name("links")->value);
        for(size_t l = 0; l < link_strs.size(); ++l) {
            mob_links_buffer.push_back(std::make_pair(m, s2i(link_strs[l])));
        }
        
        data_node* stored_inside_node =
            mob_node->get_child_by_name("stored_inside");
        if(!stored_inside_node->value.empty()) {
            mob_ptr->stored_inside = s2i(stored_inside_node->value);
        }
        
        bool valid =
            category && category->id != MOB_CATEGORY_NONE &&
            mob_ptr->type;
            
        if(!valid) {
            //Error.
            mob_ptr->type = nullptr;
            if(!load_for_editor) {
                game.errors.report(
                    "Unknown mob type \"" + type_name + "\" of category \"" +
                    mob_node->name + "\"!",
                    mob_node
                );
            }
        }
        
        game.cur_area_data.mob_generators.push_back(mob_ptr);
    }
    
    for(size_t l = 0; l < mob_links_buffer.size(); ++l) {
        size_t f = mob_links_buffer[l].first;
        size_t s = mob_links_buffer[l].second;
        game.cur_area_data.mob_generators[f]->links.push_back(
            game.cur_area_data.mob_generators[s]
        );
        game.cur_area_data.mob_generators[f]->link_idxs.push_back(s);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Paths.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Paths");
    }
    
    size_t n_stops =
        geometry_file.get_child_by_name("path_stops")->get_nr_of_children();
    for(size_t s = 0; s < n_stops; ++s) {
    
        data_node* path_stop_node =
            geometry_file.get_child_by_name("path_stops")->get_child(s);
            
        path_stop* s_ptr = new path_stop();
        
        s_ptr->pos = s2p(path_stop_node->get_child_by_name("pos")->value);
        s_ptr->radius = s2f(path_stop_node->get_child_by_name("radius")->value);
        s_ptr->flags = s2i(path_stop_node->get_child_by_name("flags")->value);
        s_ptr->label = path_stop_node->get_child_by_name("label")->value;
        data_node* links_node = path_stop_node->get_child_by_name("links");
        size_t n_links = links_node->get_nr_of_children();
        
        for(size_t l = 0; l < n_links; ++l) {
        
            string link_data = links_node->get_child(l)->value;
            vector<string> link_data_parts = split(link_data);
            
            path_link* l_struct =
                new path_link(
                s_ptr, nullptr, s2i(link_data_parts[0])
            );
            if(link_data_parts.size() >= 2) {
                l_struct->type = (PATH_LINK_TYPE) s2i(link_data_parts[1]);
            }
            
            s_ptr->links.push_back(l_struct);
            
        }
        
        s_ptr->radius = std::max(s_ptr->radius, PATHS::MIN_STOP_RADIUS);
        
        game.cur_area_data.path_stops.push_back(s_ptr);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Tree shadows.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Tree shadows");
    }
    
    size_t n_shadows =
        geometry_file.get_child_by_name("tree_shadows")->get_nr_of_children();
    for(size_t s = 0; s < n_shadows; ++s) {
    
        data_node* shadow_node =
            geometry_file.get_child_by_name("tree_shadows")->get_child(s);
            
        tree_shadow* s_ptr = new tree_shadow();
        
        vector<string> words =
            split(shadow_node->get_child_by_name("pos")->value);
        s_ptr->center.x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->center.y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        words = split(shadow_node->get_child_by_name("size")->value);
        s_ptr->size.x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->size.y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        s_ptr->angle =
            s2f(
                shadow_node->get_child_by_name(
                    "angle"
                )->get_value_or_default("0")
            );
        s_ptr->alpha =
            s2i(
                shadow_node->get_child_by_name(
                    "alpha"
                )->get_value_or_default("255")
            );
        s_ptr->file_name = shadow_node->get_child_by_name("file")->value;
        s_ptr->bitmap = game.textures.get(s_ptr->file_name, nullptr);
        
        words = split(shadow_node->get_child_by_name("sway")->value);
        s_ptr->sway.x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->sway.y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        if(s_ptr->bitmap == game.bmp_error && !load_for_editor) {
            game.errors.report(
                "Unknown tree shadow texture \"" + s_ptr->file_name + "\"!",
                shadow_node
            );
        }
        
        game.cur_area_data.tree_shadows.push_back(s_ptr);
        
    }
    
    //Thumbnail image.
    string thumbnail_path =
        get_base_area_folder_path(requested_area_type, !from_backup) +
        "/" + requested_area_folder_name +
        (from_backup ? "/Thumbnail_backup.png" : "/Thumbnail.png");
    game.cur_area_data.load_thumbnail(thumbnail_path);
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Set up stuff.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Geometry calculations");
    }
    
    for(size_t e = 0; e < game.cur_area_data.edges.size(); ++e) {
        game.cur_area_data.fix_edge_pointers(
            game.cur_area_data.edges[e]
        );
    }
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        game.cur_area_data.connect_sector_edges(
            game.cur_area_data.sectors[s]
        );
    }
    for(size_t v = 0; v < game.cur_area_data.vertexes.size(); ++v) {
        game.cur_area_data.connect_vertex_edges(
            game.cur_area_data.vertexes[v]
        );
    }
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        game.cur_area_data.fix_path_stop_pointers(
            game.cur_area_data.path_stops[s]
        );
    }
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        game.cur_area_data.path_stops[s]->calculate_dists();
    }
    if(!load_for_editor) {
        //Fade sectors that also fade brightness should be
        //at midway between the two neighbors.
        for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
            sector* s_ptr = game.cur_area_data.sectors[s];
            if(s_ptr->fade) {
                sector* n1 = nullptr;
                sector* n2 = nullptr;
                s_ptr->get_texture_merge_sectors(&n1, &n2);
                if(n1 && n2) {
                    s_ptr->brightness = (n1->brightness + n2->brightness) / 2;
                }
            }
        }
    }
    
    
    //Triangulate everything and save bounding boxes.
    set<edge*> lone_edges;
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        s_ptr->triangles.clear();
        TRIANGULATION_ERROR res =
            triangulate_sector(s_ptr, &lone_edges, false);
            
        if(res != TRIANGULATION_ERROR_NONE && load_for_editor) {
            game.cur_area_data.problems.non_simples[s_ptr] = res;
            game.cur_area_data.problems.lone_edges.insert(
                lone_edges.begin(), lone_edges.end()
            );
        }
        
        s_ptr->calculate_bounding_box();
    }
    
    if(!load_for_editor) game.cur_area_data.generate_blockmap();
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/**
 * @brief Loads an area's mission data.
 *
 * @param node Data node to load from.
 * @param data Data object to fill.
 */
void load_area_mission_data(data_node* node, mission_data &data) {
    data.fail_hud_primary_cond = INVALID;
    data.fail_hud_secondary_cond = INVALID;
    
    reader_setter rs(node);
    string goal_str;
    string required_mobs_str;
    int mission_grading_mode_int = MISSION_GRADING_MODE_GOAL;
    
    rs.set("mission_goal", goal_str);
    rs.set("mission_goal_amount", data.goal_amount);
    rs.set("mission_goal_all_mobs", data.goal_all_mobs);
    rs.set("mission_required_mobs", required_mobs_str);
    rs.set("mission_goal_exit_center", data.goal_exit_center);
    rs.set("mission_goal_exit_size", data.goal_exit_size);
    rs.set("mission_fail_conditions", data.fail_conditions);
    rs.set("mission_fail_too_few_pik_amount", data.fail_too_few_pik_amount);
    rs.set("mission_fail_too_many_pik_amount", data.fail_too_many_pik_amount);
    rs.set("mission_fail_pik_killed", data.fail_pik_killed);
    rs.set("mission_fail_leaders_kod", data.fail_leaders_kod);
    rs.set("mission_fail_enemies_killed", data.fail_enemies_killed);
    rs.set("mission_fail_time_limit", data.fail_time_limit);
    rs.set("mission_fail_hud_primary_cond", data.fail_hud_primary_cond);
    rs.set("mission_fail_hud_secondary_cond", data.fail_hud_secondary_cond);
    rs.set("mission_grading_mode", mission_grading_mode_int);
    rs.set("mission_points_per_pikmin_born", data.points_per_pikmin_born);
    rs.set("mission_points_per_pikmin_death", data.points_per_pikmin_death);
    rs.set("mission_points_per_sec_left", data.points_per_sec_left);
    rs.set("mission_points_per_sec_passed", data.points_per_sec_passed);
    rs.set("mission_points_per_treasure_point", data.points_per_treasure_point);
    rs.set("mission_points_per_enemy_point", data.points_per_enemy_point);
    rs.set("mission_point_loss_data", data.point_loss_data);
    rs.set("mission_point_hud_data", data.point_hud_data);
    rs.set("mission_starting_points", data.starting_points);
    rs.set("mission_bronze_req", data.bronze_req);
    rs.set("mission_silver_req", data.silver_req);
    rs.set("mission_gold_req", data.gold_req);
    rs.set("mission_platinum_req", data.platinum_req);
    
    data.goal = MISSION_GOAL_END_MANUALLY;
    for(size_t g = 0; g < game.mission_goals.size(); ++g) {
        if(game.mission_goals[g]->get_name() == goal_str) {
            data.goal = (MISSION_GOAL) g;
            break;
        }
    }
    vector<string> mission_required_mobs_strs =
        split(required_mobs_str, ";");
    data.goal_mob_idxs.reserve(
        mission_required_mobs_strs.size()
    );
    for(size_t m = 0; m < mission_required_mobs_strs.size(); ++m) {
        data.goal_mob_idxs.insert(
            s2i(mission_required_mobs_strs[m])
        );
    }
    data.grading_mode = (MISSION_GRADING_MODE) mission_grading_mode_int;
    
    //Automatically turn the pause menu fail condition on/off for convenience.
    if(data.goal == MISSION_GOAL_END_MANUALLY) {
        disable_flag(
            data.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
    } else {
        enable_flag(
            data.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
    }
    
    //Automatically turn off the seconds left score criterion for convenience.
    if(
        !has_flag(
            data.fail_conditions,
            get_idx_bitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        data.points_per_sec_left = 0;
        disable_flag(
            data.point_hud_data,
            get_idx_bitmask(MISSION_SCORE_CRITERIA_SEC_LEFT)
        );
        disable_flag(
            data.point_loss_data,
            get_idx_bitmask(MISSION_SCORE_CRITERIA_SEC_LEFT)
        );
    }
}


/**
 * @brief Loads a mission's record.
 *
 * @param file File data node to load from.
 * @param area_name Name of the area.
 * @param area_subtitle Area subtitle, or mission goal if none.
 * @param area_maker Area maker.
 * @param area_version Area version.
 * @param record Record object to fill.
 */
void load_area_mission_record(
    data_node* file,
    const string &area_name, const string &area_subtitle,
    const string &area_maker, const string &area_version,
    mission_record &record
) {
    string mission_record_entry_name =
        area_name + ";" +
        area_subtitle + ";" +
        area_maker + ";" +
        area_version;
        
    vector<string> record_parts =
        split(
            file->get_child_by_name(
                mission_record_entry_name
            )->value,
            ";"
        );
        
    if(record_parts.size() == 3) {
        record.clear = record_parts[0] == "1";
        record.score = s2i(record_parts[1]);
        record.date = record_parts[2];
    }
}


/**
 * @brief Loads asset file names.
 */
void load_asset_file_names() {
    data_node file(SYSTEM_ASSET_FILE_NAMES_FILE_PATH);
    
    game.asset_file_names.load(&file);
}


/**
 * @brief Loads an audio stream from the game's content.
 *
 * @param file_name Name of the file to load.
 * @param node If not nullptr, blame this data node if the file
 * doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The stream.
 */
ALLEGRO_AUDIO_STREAM* load_audio_stream(
    const string &file_name, data_node* node, bool report_errors
) {
    ALLEGRO_AUDIO_STREAM* stream =
        al_load_audio_stream(
            (AUDIO_TRACK_FOLDER_PATH + "/" + file_name).c_str(),
            4, 2048
        );
        
    if(!stream && report_errors) {
        game.errors.report(
            "Could not open audio stream file \"" + file_name + "\"!",
            node
        );
    }
    
    return stream;
}


/**
 * @brief Loads a bitmap from the game's content.
 *
 * @param file_name File name of the bitmap.
 * @param node If present, it will be used to report errors, if any.
 * @param report_error If false, omits error reporting.
 * @param error_bmp_on_error If true, returns the error bitmap in the case of an
 * error. Otherwise, returns nullptr.
 * @param error_bmp_on_empty If true, returns the error bitmap in the case of an
 * empty file name. Otherwise, returns nullptr.
 * @param path_from_root Normally, files are fetched from the images folder.
 * If this parameter is true, the path starts from the game's root.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* load_bmp(
    const string &file_name, data_node* node,
    const bool report_error, const bool error_bmp_on_error,
    const bool error_bmp_on_empty, const bool path_from_root
) {
    if(file_name.empty()) {
        if(error_bmp_on_empty) {
            return game.bmp_error;
        } else {
            return nullptr;
        }
    }
    
    string base_dir = (path_from_root ? "" : (GRAPHICS_FOLDER_PATH + "/"));
    ALLEGRO_BITMAP* b =
        al_load_bitmap((base_dir + file_name).c_str());
        
    if(!b) {
        if(report_error) {
            game.errors.report(
                "Could not open image \"" + file_name + "\"!",
                node
            );
        }
        if(error_bmp_on_error) {
            b = game.bmp_error;
        }
    }
    
    return b;
}


/**
 * @brief Loads a data file from the game's content.
 *
 * @param file_path Path to the file, relative to the program root folder.
 */
data_node load_data_file(const string &file_path) {
    data_node n = data_node(file_path);
    if(!n.file_was_opened) {
        game.errors.report(
            "Could not open data file \"" + file_path + "\"!"
        );
    }
    
    return n;
}


/**
 * @brief Loads a font from the disk. If it's a bitmap it'll load it from
 * the bitmap and map the characters according to the ranges provided.
 * If it's a font file, it'll just load it directly.
 *
 * @param file_name Name of the file in the graphics folder.
 * @param n Number of Unicode ranges in the bitmap, if it's a bitmap.
 * @param ranges "n" pairs of first and last Unicode point to map glyphs to
 * for each range, if it's a bitmap.
 * @param size Font size, if it's a font file.
 */
ALLEGRO_FONT* load_font(
    const string &file_name, int n, const int ranges[], int size
) {
    string full_path = GRAPHICS_FOLDER_PATH + "/" + file_name;
    ALLEGRO_FONT* result = nullptr;
    
    //First, try to load it as a TTF font.
    result =
        al_load_ttf_font(full_path.c_str(), size, ALLEGRO_TTF_NO_KERNING);
        
    if(result) return result;
    
    //Now try as a bitmap.
    ALLEGRO_BITMAP* bmp = load_bmp(file_name);
    result = al_grab_font_from_bitmap(bmp, n, ranges);
    al_destroy_bitmap(bmp);
    
    return result;
}


/**
 * @brief Loads the game's fonts.
 */
void load_fonts() {
    const int STANDARD_FONT_RANGES_SIZE = 2;
    const int standard_font_ranges[STANDARD_FONT_RANGES_SIZE] = {
        0x0020, 0x007E, //ASCII
        /*0x00A0, 0x00A1, //Non-breaking space and inverted !
        0x00BF, 0x00FF, //Inverted ? and European vowels and such*/
    };
    
    const int COUNTER_FONT_RANGES_SIZE = 6;
    const int counter_font_ranges[COUNTER_FONT_RANGES_SIZE] = {
        0x002D, 0x0039, //Dash, dot, slash, numbers
        0x003A, 0x003A, //Colon
        0x0078, 0x0078, //Lowercase x
    };
    
    const int JUST_NUMBERS_FONT_RANGES_SIZE = 2;
    const int just_numbers_font_ranges[JUST_NUMBERS_FONT_RANGES_SIZE] = {
        0x0030, 0x0039, //0 to 9
    };
    
    const int VALUE_FONT_RANGES_SIZE = 6;
    const int value_font_ranges[VALUE_FONT_RANGES_SIZE] = {
        0x0024, 0x0024, //Dollar sign
        0x002D, 0x002D, //Dash
        0x0030, 0x0039, //Numbers
    };
    
    //We can't load the fonts directly because we want to set the ranges.
    //So we load them into bitmaps first.
    
    //Area name font.
    game.sys_assets.fnt_area_name =
        load_font(
            game.asset_file_names.fnt_area_name,
            STANDARD_FONT_RANGES_SIZE / 2, standard_font_ranges,
            34
        );
        
    //Built-in font.
    game.sys_assets.fnt_builtin = al_create_builtin_font();
    
    //Counter font.
    game.sys_assets.fnt_counter =
        load_font(
            game.asset_file_names.fnt_counter,
            COUNTER_FONT_RANGES_SIZE / 2, counter_font_ranges,
            32
        );
        
    //Cursor counter font.
    game.sys_assets.fnt_cursor_counter =
        load_font(
            game.asset_file_names.fnt_cursor_counter,
            JUST_NUMBERS_FONT_RANGES_SIZE / 2, just_numbers_font_ranges,
            16
        );
        
    //Slim font.
    game.sys_assets.fnt_slim =
        load_font(
            game.asset_file_names.fnt_slim,
            STANDARD_FONT_RANGES_SIZE / 2, standard_font_ranges,
            22
        );
        
    //Standard font.
    game.sys_assets.fnt_standard =
        load_font(
            game.asset_file_names.fnt_standard,
            STANDARD_FONT_RANGES_SIZE / 2, standard_font_ranges,
            22
        );
        
    //Value font.
    game.sys_assets.fnt_value =
        load_font(
            game.asset_file_names.fnt_value,
            VALUE_FONT_RANGES_SIZE / 2, value_font_ranges,
            16
        );
}


/**
 * @brief Loads the game's configuration file.
 */
void load_game_config() {
    data_node file = load_data_file(CONFIG_FILE);
    
    game.config.load(&file);
    
    al_set_window_title(
        game.display,
        game.config.name.empty() ? "Pikifen" : game.config.name.c_str()
    );
}


/**
 * @brief Loads the maker tools from the tool config file.
 */
void load_maker_tools() {
    data_node file(MAKER_TOOLS_FILE_PATH);
    
    if(!file.file_was_opened) return;
    
    game.maker_tools.enabled = s2b(file.get_child_by_name("enabled")->value);
    
    for(unsigned char k = 0; k < 20; k++) {
        string tool_name;
        if(k < 10) {
            //The first ten indexes are the F2 - F11 keys.
            tool_name = file.get_child_by_name("f" + i2s(k + 2))->value;
        } else {
            //The second ten indexes are the 0 - 9 keys.
            tool_name = file.get_child_by_name(i2s(k - 10))->value;
        }
        
        for(size_t t = 0; t < N_MAKER_TOOLS; ++t) {
            if(tool_name == MAKER_TOOLS::NAMES[t]) {
                game.maker_tools.keys[k] = (MAKER_TOOL_TYPE) t;
            }
        }
    }
    
    reader_setter rs(&file);
    
    data_node* mob_hurting_percentage_node = nullptr;
    
    rs.set("area_image_mobs", game.maker_tools.area_image_mobs);
    rs.set("area_image_padding", game.maker_tools.area_image_padding);
    rs.set("area_image_shadows", game.maker_tools.area_image_shadows);
    rs.set("area_image_size", game.maker_tools.area_image_size);
    rs.set("change_speed_multiplier", game.maker_tools.change_speed_mult);
    rs.set(
        "mob_hurting_percentage", game.maker_tools.mob_hurting_ratio,
        &mob_hurting_percentage_node
    );
    rs.set("auto_start_option", game.maker_tools.auto_start_option);
    rs.set("auto_start_mode", game.maker_tools.auto_start_mode);
    rs.set("performance_monitor", game.maker_tools.use_perf_mon);
    
    if(mob_hurting_percentage_node) {
        game.maker_tools.mob_hurting_ratio /= 100.0;
    }
}


/**
 * @brief Loads miscellaneous fixed graphics.
 */
void load_misc_graphics() {
    //Icon.
    game.sys_assets.bmp_icon = game.bitmaps.get(game.asset_file_names.bmp_icon);
    al_set_display_icon(game.display, game.sys_assets.bmp_icon);
    
    //Graphics.
    game.sys_assets.bmp_menu_icons =
        game.bitmaps.get(game.asset_file_names.bmp_menu_icons);
    game.sys_assets.bmp_bright_circle =
        game.bitmaps.get(game.asset_file_names.bmp_bright_circle);
    game.sys_assets.bmp_bright_ring =
        game.bitmaps.get(game.asset_file_names.bmp_bright_ring);
    game.sys_assets.bmp_bubble_box =
        game.bitmaps.get(game.asset_file_names.bmp_bubble_box);
    game.sys_assets.bmp_button_box =
        game.bitmaps.get(game.asset_file_names.bmp_button_box);
    game.sys_assets.bmp_checkbox_check =
        game.bitmaps.get(game.asset_file_names.bmp_checkbox_check);
    game.sys_assets.bmp_checkbox_no_check =
        game.bitmaps.get(game.asset_file_names.bmp_checkbox_no_check);
    game.sys_assets.bmp_cursor =
        game.bitmaps.get(game.asset_file_names.bmp_cursor);
    game.sys_assets.bmp_enemy_spirit =
        game.bitmaps.get(game.asset_file_names.bmp_enemy_spirit);
    game.sys_assets.bmp_focus_box =
        game.bitmaps.get(game.asset_file_names.bmp_focus_box);
    game.sys_assets.bmp_frame_box =
        game.bitmaps.get(game.asset_file_names.bmp_frame_box);
    game.sys_assets.bmp_hard_bubble =
        game.bitmaps.get(game.asset_file_names.bmp_hard_bubble);
    game.sys_assets.bmp_idle_glow =
        game.bitmaps.get(game.asset_file_names.bmp_idle_glow);
    game.sys_assets.bmp_key_box =
        game.bitmaps.get(game.asset_file_names.bmp_key_box);
    game.sys_assets.bmp_leader_silhouette_side =
        game.bitmaps.get(game.asset_file_names.bmp_leader_silhouette_side);
    game.sys_assets.bmp_leader_silhouette_top =
        game.bitmaps.get(game.asset_file_names.bmp_leader_silhouette_top);
    game.sys_assets.bmp_medal_bronze =
        game.bitmaps.get(game.asset_file_names.bmp_medal_bronze);
    game.sys_assets.bmp_medal_gold =
        game.bitmaps.get(game.asset_file_names.bmp_medal_gold);
    game.sys_assets.bmp_medal_none =
        game.bitmaps.get(game.asset_file_names.bmp_medal_none);
    game.sys_assets.bmp_medal_platinum =
        game.bitmaps.get(game.asset_file_names.bmp_medal_platinum);
    game.sys_assets.bmp_medal_silver =
        game.bitmaps.get(game.asset_file_names.bmp_medal_silver);
    game.sys_assets.bmp_menu_icons =
        game.bitmaps.get(game.asset_file_names.bmp_menu_icons);
    game.sys_assets.bmp_mission_clear =
        game.bitmaps.get(game.asset_file_names.bmp_mission_clear);
    game.sys_assets.bmp_mission_fail =
        game.bitmaps.get(game.asset_file_names.bmp_mission_fail);
    game.sys_assets.bmp_more =
        game.bitmaps.get(game.asset_file_names.bmp_more);
    game.sys_assets.bmp_mouse_cursor =
        game.bitmaps.get(game.asset_file_names.bmp_mouse_cursor);
    game.sys_assets.bmp_notification =
        game.bitmaps.get(game.asset_file_names.bmp_notification);
    game.sys_assets.bmp_pikmin_spirit =
        game.bitmaps.get(game.asset_file_names.bmp_pikmin_spirit);
    game.sys_assets.bmp_player_input_icons =
        game.bitmaps.get(game.asset_file_names.bmp_player_input_icons);
    game.sys_assets.bmp_random =
        game.bitmaps.get(game.asset_file_names.bmp_random);
    game.sys_assets.bmp_rock =
        game.bitmaps.get(game.asset_file_names.bmp_rock);
    game.sys_assets.bmp_shadow =
        game.bitmaps.get(game.asset_file_names.bmp_shadow);
    game.sys_assets.bmp_shadow_square =
        game.bitmaps.get(game.asset_file_names.bmp_shadow_square);
    game.sys_assets.bmp_smack =
        game.bitmaps.get(game.asset_file_names.bmp_smack);
    game.sys_assets.bmp_smoke =
        game.bitmaps.get(game.asset_file_names.bmp_smoke);
    game.sys_assets.bmp_sparkle =
        game.bitmaps.get(game.asset_file_names.bmp_sparkle);
    game.sys_assets.bmp_spotlight =
        game.bitmaps.get(game.asset_file_names.bmp_spotlight);
    game.sys_assets.bmp_swarm_arrow =
        game.bitmaps.get(game.asset_file_names.bmp_swarm_arrow);
    game.sys_assets.bmp_throw_invalid =
        game.bitmaps.get(game.asset_file_names.bmp_throw_invalid);
    game.sys_assets.bmp_throw_preview =
        game.bitmaps.get(game.asset_file_names.bmp_throw_preview);
    game.sys_assets.bmp_throw_preview_dashed =
        game.bitmaps.get(game.asset_file_names.bmp_throw_preview_dashed);
    game.sys_assets.bmp_wave_ring =
        game.bitmaps.get(game.asset_file_names.bmp_wave_ring);
}


/**
 * @brief Loads miscellaneous fixed sound effects.
 */
void load_misc_sounds() {
    game.audio.init(
        game.options.master_volume,
        game.options.world_sfx_volume,
        game.options.music_volume,
        game.options.ambiance_volume,
        game.options.ui_sfx_volume
    );
    
    //Sound effects.
    game.sys_assets.sfx_attack =
        game.audio.samples.get(game.asset_file_names.sfx_attack);
    game.sys_assets.sfx_camera =
        game.audio.samples.get(game.asset_file_names.sfx_camera);
    game.sys_assets.sfx_menu_activate =
        game.audio.samples.get(game.asset_file_names.sfx_menu_activate);
    game.sys_assets.sfx_menu_back =
        game.audio.samples.get(game.asset_file_names.sfx_menu_back);
    game.sys_assets.sfx_menu_select =
        game.audio.samples.get(game.asset_file_names.sfx_menu_select);
    game.sys_assets.sfx_pluck =
        game.audio.samples.get(game.asset_file_names.sfx_pluck);
    game.sys_assets.sfx_throw =
        game.audio.samples.get(game.asset_file_names.sfx_throw);
    game.sys_assets.sfx_spray =
        game.audio.samples.get(game.asset_file_names.sfx_spray);
    game.sys_assets.sfx_switch_pikmin =
        game.audio.samples.get(game.asset_file_names.sfx_switch_pikmin);
}


/**
 * @brief Loads the player's options.
 */
void load_options() {
    data_node file = data_node(OPTIONS_FILE_PATH);
    if(!file.file_was_opened) return;
    
    //Init game controllers.
    game.controller_numbers.clear();
    int n_joysticks = al_get_num_joysticks();
    for(int j = 0; j < n_joysticks; ++j) {
        game.controller_numbers[al_get_joystick(j)] = j;
    }
    
    //Read the main options.
    game.options.load(&file);
    
    game.win_fullscreen = game.options.intended_win_fullscreen;
    game.win_w = game.options.intended_win_w;
    game.win_h = game.options.intended_win_h;
    
    //Set up the editor histories.
    reader_setter rs(&file);
    
    game.states.animation_ed->history.clear();
    for(size_t h = 0; h < game.states.animation_ed->get_history_size(); ++h) {
        game.states.animation_ed->history.push_back("");
        rs.set(
            game.states.animation_ed->get_history_option_prefix() +
            i2s(h + 1),
            game.states.animation_ed->history[h]
        );
    }
    game.states.area_ed->history.clear();
    for(size_t h = 0; h < game.states.area_ed->get_history_size(); ++h) {
        game.states.area_ed->history.push_back("");
        rs.set(
            game.states.area_ed->get_history_option_prefix() +
            i2s(h + 1),
            game.states.area_ed->history[h]
        );
    }
    game.states.gui_ed->history.clear();
    for(size_t h = 0; h < game.states.gui_ed->get_history_size(); ++h) {
        game.states.gui_ed->history.push_back("");
        rs.set(
            game.states.gui_ed->get_history_option_prefix() +
            i2s(h + 1),
            game.states.gui_ed->history[h]
        );
    }
    
    //Final setup.
    controls_manager_options controls_mgr_options;
    controls_mgr_options.stick_min_deadzone =
        game.options.joystick_min_deadzone;
    controls_mgr_options.stick_max_deadzone =
        game.options.joystick_max_deadzone;
    game.controls.set_options(controls_mgr_options);
}


/**
 * @brief Loads an audio sample from the game's content.
 *
 * @param file_name Name of the file to load.
 * @param node If not nullptr, blame this data node if the file
 * doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The sample.
 */
ALLEGRO_SAMPLE* load_sample(
    const string &file_name, data_node* node, bool report_errors
) {
    ALLEGRO_SAMPLE* sample =
        al_load_sample((AUDIO_SOUNDS_FOLDER_PATH + "/" + file_name).c_str());
        
    if(!sample && report_errors) {
        game.errors.report(
            "Could not open audio file \"" + file_name + "\"!",
            node
        );
    }
    
    return sample;
}


/**
 * @brief Loads the songs.
 */
void load_songs() {
    vector<string> song_files =
        folder_to_vector(AUDIO_SONG_FOLDER_PATH, false);
        
    for(size_t s = 0; s < song_files.size(); ++s) {
        string path = AUDIO_SONG_FOLDER_PATH + "/" + song_files[s];
        data_node file = load_data_file(path);
        if(!file.file_was_opened) continue;
        
        song new_song;
        new_song.path = path;
        new_song.load_from_data_node(&file);
        game.audio.songs[new_song.name] = new_song;
    }
}


/**
 * @brief Loads the engine's lifetime statistics.
 */
void load_statistics() {
    data_node stats_file;
    stats_file.load_file(STATISTICS_FILE_PATH, true, false, true);
    if(!stats_file.file_was_opened) return;
    
    statistics_t &s = game.statistics;
    
    reader_setter rs(&stats_file);
    rs.set("startups",               s.startups);
    rs.set("runtime",                s.runtime);
    rs.set("gameplay_time",          s.gameplay_time);
    rs.set("area_entries",           s.area_entries);
    rs.set("pikmin_births",          s.pikmin_births);
    rs.set("pikmin_deaths",          s.pikmin_deaths);
    rs.set("pikmin_eaten",           s.pikmin_eaten);
    rs.set("pikmin_hazard_deaths",   s.pikmin_hazard_deaths);
    rs.set("pikmin_blooms",          s.pikmin_blooms);
    rs.set("pikmin_saved",           s.pikmin_saved);
    rs.set("enemy_deaths",           s.enemy_deaths);
    rs.set("pikmin_thrown",          s.pikmin_thrown);
    rs.set("whistle_uses",           s.whistle_uses);
    rs.set("distance_walked",        s.distance_walked);
    rs.set("leader_damage_suffered", s.leader_damage_suffered);
    rs.set("punch_damage_caused",    s.punch_damage_caused);
    rs.set("leader_kos",             s.leader_kos);
    rs.set("sprays_used",            s.sprays_used);
}


/**
 * @brief Loads the animations that are used system-wide.
 */
void load_system_animations() {
    data_node system_animations_file =
        load_data_file(SYSTEM_ANIMATIONS_FILE_PATH);
        
    init_single_animation(
        &system_animations_file,
        "leader_damage_sparks", game.sys_assets.spark_animation
    );
}


/**
 * @brief Unloads the loaded area from memory.
 */
void unload_area() {
    game.cur_area_data.clear();
}


/**
 * @brief Unloads miscellaneous graphics, sounds, and other resources.
 */
void unload_misc_resources() {
    //Graphics.
    game.bitmaps.free(game.sys_assets.bmp_bright_circle);
    game.bitmaps.free(game.sys_assets.bmp_bright_ring);
    game.bitmaps.free(game.sys_assets.bmp_bubble_box);
    game.bitmaps.free(game.sys_assets.bmp_button_box);
    game.bitmaps.free(game.sys_assets.bmp_checkbox_check);
    game.bitmaps.free(game.sys_assets.bmp_checkbox_no_check);
    game.bitmaps.free(game.sys_assets.bmp_cursor);
    game.bitmaps.free(game.sys_assets.bmp_enemy_spirit);
    game.bitmaps.free(game.sys_assets.bmp_focus_box);
    game.bitmaps.free(game.sys_assets.bmp_frame_box);
    game.bitmaps.free(game.sys_assets.bmp_hard_bubble);
    game.bitmaps.free(game.sys_assets.bmp_icon);
    game.bitmaps.free(game.sys_assets.bmp_idle_glow);
    game.bitmaps.free(game.sys_assets.bmp_key_box);
    game.bitmaps.free(game.sys_assets.bmp_leader_silhouette_side);
    game.bitmaps.free(game.sys_assets.bmp_leader_silhouette_top);
    game.bitmaps.free(game.sys_assets.bmp_medal_bronze);
    game.bitmaps.free(game.sys_assets.bmp_medal_gold);
    game.bitmaps.free(game.sys_assets.bmp_medal_none);
    game.bitmaps.free(game.sys_assets.bmp_medal_platinum);
    game.bitmaps.free(game.sys_assets.bmp_medal_silver);
    game.bitmaps.free(game.sys_assets.bmp_menu_icons);
    game.bitmaps.free(game.sys_assets.bmp_mission_clear);
    game.bitmaps.free(game.sys_assets.bmp_mission_fail);
    game.bitmaps.free(game.sys_assets.bmp_more);
    game.bitmaps.free(game.sys_assets.bmp_mouse_cursor);
    game.bitmaps.free(game.sys_assets.bmp_notification);
    game.bitmaps.free(game.sys_assets.bmp_pikmin_spirit);
    game.bitmaps.free(game.sys_assets.bmp_player_input_icons);
    game.bitmaps.free(game.sys_assets.bmp_random);
    game.bitmaps.free(game.sys_assets.bmp_rock);
    game.bitmaps.free(game.sys_assets.bmp_shadow);
    game.bitmaps.free(game.sys_assets.bmp_shadow_square);
    game.bitmaps.free(game.sys_assets.bmp_smack);
    game.bitmaps.free(game.sys_assets.bmp_smoke);
    game.bitmaps.free(game.sys_assets.bmp_sparkle);
    game.bitmaps.free(game.sys_assets.bmp_spotlight);
    game.bitmaps.free(game.sys_assets.bmp_swarm_arrow);
    game.bitmaps.free(game.sys_assets.bmp_throw_invalid);
    game.bitmaps.free(game.sys_assets.bmp_throw_preview);
    game.bitmaps.free(game.sys_assets.bmp_throw_preview_dashed);
    game.bitmaps.free(game.sys_assets.bmp_wave_ring);
    
    //Fonts.
    al_destroy_font(game.sys_assets.fnt_area_name);
    al_destroy_font(game.sys_assets.fnt_counter);
    al_destroy_font(game.sys_assets.fnt_cursor_counter);
    al_destroy_font(game.sys_assets.fnt_slim);
    al_destroy_font(game.sys_assets.fnt_standard);
    al_destroy_font(game.sys_assets.fnt_value);
    
    //Sounds effects.
    game.audio.samples.free(game.sys_assets.sfx_attack);
    game.audio.samples.free(game.sys_assets.sfx_camera);
    game.audio.samples.free(game.sys_assets.sfx_menu_activate);
    game.audio.samples.free(game.sys_assets.sfx_menu_back);
    game.audio.samples.free(game.sys_assets.sfx_menu_select);
    game.audio.samples.free(game.sys_assets.sfx_spray);
    game.audio.samples.free(game.sys_assets.sfx_switch_pikmin);
    game.audio.samples.free(game.sys_assets.sfx_throw);
}


/**
 * @brief Unloads loaded songs from memory.
 */
void unload_songs() {
    for(auto &s : game.audio.songs) {
        game.audio.streams.free(s.second.main_track);
        for(auto &t : s.second.mix_tracks) {
            game.audio.streams.free(t.second);
        }
    }
    game.audio.songs.clear();
}
