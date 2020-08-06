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

#include "load.h"

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "init.h"
#include "utils/string_utils.h"

using std::set;


/* ----------------------------------------------------------------------------
 * Loads an area into memory.
 * name:            Name of the area's folder.
 * load_for_editor: If true, skips loading some things that the area editor
 *   won't need.
 * from_backup:     If true, load from a backup, if any.
 */
void load_area(
    const string &name, const bool load_for_editor, const bool from_backup
) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Data");
    }
    
    game.cur_area_data.clear();
    
    string geometry_file_name;
    string data_file_name;
    string sanitized_area_name = sanitize_file_name(name);
    if(from_backup) {
        geometry_file_name =
            USER_AREA_DATA_FOLDER_PATH + "/" + sanitized_area_name +
            "/Geometry_backup.txt";
        data_file_name =
            USER_AREA_DATA_FOLDER_PATH + "/" + sanitized_area_name +
            "/Data_backup.txt";
    } else {
        geometry_file_name =
            AREAS_FOLDER_PATH + "/" + sanitized_area_name + "/Geometry.txt";
        data_file_name =
            AREAS_FOLDER_PATH + "/" + sanitized_area_name + "/Data.txt";
    }
    
    //First, load the area's configuration data.
    data_node data_file(data_file_name);
    reader_setter rs(&data_file);
    
    data_node* weather_node = NULL;
    
    rs.set("name", game.cur_area_data.name);
    rs.set("subtitle", game.cur_area_data.subtitle);
    rs.set("creator", game.cur_area_data.creator);
    rs.set("version", game.cur_area_data.version);
    rs.set("notes", game.cur_area_data.notes);
    rs.set("spray_amounts", game.cur_area_data.spray_amounts);
    rs.set("weather", game.cur_area_data.weather_name, &weather_node);
    rs.set("bg_bmp", game.cur_area_data.bg_bmp_file_name);
    rs.set("bg_color", game.cur_area_data.bg_color);
    rs.set("bg_dist", game.cur_area_data.bg_dist);
    rs.set("bg_zoom", game.cur_area_data.bg_bmp_zoom);
    
    if(game.loading_text_bmp) al_destroy_bitmap(game.loading_text_bmp);
    if(game.loading_subtext_bmp) al_destroy_bitmap(game.loading_subtext_bmp);
    game.loading_text_bmp = NULL;
    game.loading_subtext_bmp = NULL;
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    draw_loading_screen(
        game.cur_area_data.name, game.cur_area_data.subtitle, 1.0
    );
    al_flip_display();
    
    if(!load_for_editor) {
    
        if(game.perf_mon) {
            game.perf_mon->start_measurement("Area -- Initial assets");
        }
        
        if(game.cur_area_data.weather_name.empty()) {
            game.cur_area_data.weather_condition = weather();
            
        } else if(
            game.weather_conditions.find(game.cur_area_data.weather_name) ==
            game.weather_conditions.end()
        ) {
            log_error(
                "Area " + name +
                " refers to an unknown weather condition, \"" +
                game.cur_area_data.weather_name + "\"!",
                weather_node
            );
            game.cur_area_data.weather_condition = weather();
            
        } else {
            game.cur_area_data.weather_condition =
                game.weather_conditions[game.cur_area_data.weather_name];
                
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
        
        vector<string> s_nrs = split(edge_data->get_child_by_name("s")->value);
        if(s_nrs.size() < 2) s_nrs.insert(s_nrs.end(), 2, "-1");
        for(size_t s = 0; s < 2; ++s) {
            if(s_nrs[s] == "-1") new_edge->sector_nrs[s] = INVALID;
            else new_edge->sector_nrs[s] = s2i(s_nrs[s]);
        }
        
        vector<string> v_nrs = split(edge_data->get_child_by_name("v")->value);
        if(v_nrs.size() < 2) v_nrs.insert(v_nrs.end(), 2, "0");
        
        new_edge->vertex_nrs[0] = s2i(v_nrs[0]);
        new_edge->vertex_nrs[1] = s2i(v_nrs[1]);
        
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
        
        new_sector->type =
            game.sector_types.get_nr(sector_data->get_child_by_name("type")->value);
        if(new_sector->type == 255) new_sector->type = SECTOR_TYPE_NORMAL;
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
                )->get_value_or_default(i2s(DEF_SECTOR_BRIGHTNESS))
            );
        new_sector->tag = sector_data->get_child_by_name("tag")->value;
        new_sector->z = s2f(sector_data->get_child_by_name("z")->value);
        new_sector->fade = s2b(sector_data->get_child_by_name("fade")->value);
        new_sector->always_cast_shadow =
            s2b(
                sector_data->get_child_by_name("always_cast_shadow")->value
            );
            
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
                game.textures.get(new_sector->texture_info.file_name, NULL);
        }
        
        data_node* hazards_node = sector_data->get_child_by_name("hazards");
        vector<string> hazards_strs =
            semicolon_list_to_vector(hazards_node->value);
        for(size_t h = 0; h < hazards_strs.size(); ++h) {
            string hazard_name = hazards_strs[h];
            if(game.hazards.find(hazard_name) == game.hazards.end()) {
                log_error(
                    "Unknown hazard \"" + hazard_name +
                    "\"!", hazards_node
                );
            } else {
                new_sector->hazards.push_back(&(game.hazards[hazard_name]));
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
        
        mob_ptr->category = game.mob_categories.get_from_name(mob_node->name);
        if(!mob_ptr->category) continue;
        
        string mt = mob_node->get_child_by_name("type")->value;
        mob_ptr->type = mob_ptr->category->get_type(mt);
        
        vector<string> link_strs =
            split(mob_node->get_child_by_name("links")->value);
        for(size_t l = 0; l < link_strs.size(); ++l) {
            mob_links_buffer.push_back(std::make_pair(m, s2i(link_strs[l])));
        }
        
        bool problem = false;
        
        if(!mob_ptr->type && !load_for_editor) {
            //Error.
            log_error(
                "Unknown \"" + mob_ptr->category->name +
                "\" mob type \"" +
                mt + "\"!",
                mob_node
            );
            problem = true;
        }
        
        if(
            (
                mob_ptr->category->id == MOB_CATEGORY_NONE ||
                mob_ptr->category->id == INVALID
            ) && !load_for_editor
        ) {
        
            log_error(
                "Unknown mob category \"" + mob_node->name + "\"!", mob_node
            );
            mob_ptr->category = game.mob_categories.get(MOB_CATEGORY_NONE);
            problem = true;
            
        }
        
        if(!problem) {
            game.cur_area_data.mob_generators.push_back(mob_ptr);
        } else {
            delete mob_ptr;
        }
    }
    
    for(size_t l = 0; l < mob_links_buffer.size(); ++l) {
        size_t f = mob_links_buffer[l].first;
        size_t s = mob_links_buffer[l].second;
        game.cur_area_data.mob_generators[f]->links.push_back(
            game.cur_area_data.mob_generators[s]
        );
        game.cur_area_data.mob_generators[f]->link_nrs.push_back(s);
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    
    //Path stops.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Area -- Paths");
    }
    
    size_t n_stops =
        geometry_file.get_child_by_name("path_stops")->get_nr_of_children();
    for(size_t s = 0; s < n_stops; ++s) {
    
        data_node* path_stop_node =
            geometry_file.get_child_by_name("path_stops")->get_child(s);
            
        path_stop* s_ptr = new path_stop();
        
        vector<string> words =
            split(path_stop_node->get_child_by_name("pos")->value);
        s_ptr->pos.x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->pos.y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        data_node* links_node = path_stop_node->get_child_by_name("links");
        size_t n_links = links_node->get_nr_of_children();
        
        for(size_t l = 0; l < n_links; ++l) {
        
            data_node* link_node = links_node->get_child(l);
            path_link l_struct(NULL, INVALID);
            
            l_struct.end_nr = s2i(link_node->value);
            
            s_ptr->links.push_back(l_struct);
            
        }
        
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
        s_ptr->bitmap = game.textures.get(s_ptr->file_name, NULL);
        
        words = split(shadow_node->get_child_by_name("sway")->value);
        s_ptr->sway.x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->sway.y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        if(s_ptr->bitmap == game.bmp_error && !load_for_editor) {
            log_error(
                "Unknown tree shadow texture \"" + s_ptr->file_name + "\"!",
                shadow_node
            );
        }
        
        game.cur_area_data.tree_shadows.push_back(s_ptr);
        
    }
    
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
                sector* n1 = NULL;
                sector* n2 = NULL;
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
        TRIANGULATION_ERRORS res =
            triangulate(s_ptr, &lone_edges, load_for_editor, false);
            
        if(res != TRIANGULATION_NO_ERROR && load_for_editor) {
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


/* ----------------------------------------------------------------------------
 * Loads asset file names.
 */
void load_asset_file_names() {
    data_node file(SYSTEM_ASSET_FILE_NAMES_FILE_PATH);
    
    game.asset_file_names.load(&file);
}


/* ----------------------------------------------------------------------------
 * Loads a bitmap from the game's content.
 * file_name:          File name of the bitmap.
 * node:               If present, it will be used to report errors, if any.
 * report_error:       If false, omits error reporting.
 * error_bmp_on_error: If true, returns the error bitmap in the case of an
 *   error. Otherwise, returns NULL.
 * error_bmp_on_empty: If true, returns the error bitmap in the case of an
 *   empty file name. Otherwise, returns NULL.
 * path_from_root:     Normally, files are fetched from the images folder.
 *   If this parameter is true, the path starts from the game's root.
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
            return NULL;
        }
    }
    
    string base_dir = (path_from_root ? "" : (GRAPHICS_FOLDER_PATH + "/"));
    ALLEGRO_BITMAP* b =
        al_load_bitmap((base_dir + file_name).c_str());
        
    if(!b) {
        if(report_error) {
            log_error("Could not open image " + file_name + "!", node);
        }
        if(error_bmp_on_error) {
            b = game.bmp_error;
        }
    }
    
    return b;
}


/* ----------------------------------------------------------------------------
 * Loads the creator tools from the tool config file.
 */
void load_creator_tools() {
    data_node file(CREATOR_TOOLS_FILE_PATH);
    
    if(!file.file_was_opened) return;
    
    game.creator_tools.enabled = s2b(file.get_child_by_name("enabled")->value);
    
    for(unsigned char k = 0; k < 20; k++) {
        string tool_name;
        if(k < 10) {
            //The first ten indexes are the F2 - F11 keys.
            tool_name = file.get_child_by_name("f" + i2s(k + 2))->value;
        } else {
            //The second ten indexes are the 0 - 9 keys.
            tool_name = file.get_child_by_name(i2s(k - 10))->value;
        }
        
        for(size_t t = 0; t < N_CREATOR_TOOLS; ++t) {
            if(tool_name == CREATOR_TOOL_NAMES[t]) {
                game.creator_tools.keys[k] = t;
            }
        }
    }
    
    reader_setter rs(&file);
    
    data_node* mob_hurting_percentage_node = NULL;
    
    rs.set("area_image_mobs", game.creator_tools.area_image_mobs);
    rs.set("area_image_shadows", game.creator_tools.area_image_shadows);
    rs.set("area_image_size", game.creator_tools.area_image_size);
    rs.set("change_speed_multiplier", game.creator_tools.change_speed_mult);
    rs.set(
        "mob_hurting_percentage", game.creator_tools.mob_hurting_ratio,
        &mob_hurting_percentage_node
    );
    rs.set("auto_start_option", game.creator_tools.auto_start_option);
    rs.set("auto_start_mode", game.creator_tools.auto_start_mode);
    rs.set("performance_monitor", game.creator_tools.use_perf_mon);
    
    if(mob_hurting_percentage_node) {
        game.creator_tools.mob_hurting_ratio /= 100.0;
    }
}


/* ----------------------------------------------------------------------------
 * Loads the user-made particle generators.
 */
void load_custom_particle_generators(const bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Custom particle generators");
    }
    
    vector<string> generator_files =
        folder_to_vector(PARTICLE_GENERATORS_FOLDER_PATH, false);
        
    for(size_t g = 0; g < generator_files.size(); ++g) {
        data_node file =
            load_data_file(
                PARTICLE_GENERATORS_FOLDER_PATH + "/" + generator_files[g]
            );
        if(!file.file_was_opened) continue;
        
        data_node* p_node = file.get_child_by_name("base");
        reader_setter grs(&file);
        reader_setter prs(p_node);
        
        string name_str;
        float emission_interval_float = 0.0f;
        size_t number_int = 1;
        string bitmap_str;
        data_node* bitmap_node = NULL;
        
        particle base_p;
        
        grs.set("name", name_str);
        grs.set("emission_interval", emission_interval_float);
        grs.set("number", number_int);
        
        prs.set("bitmap",          bitmap_str, &bitmap_node);
        prs.set("duration",        base_p.duration);
        prs.set("friction",        base_p.friction);
        prs.set("gravity",         base_p.gravity);
        prs.set("size_grow_speed", base_p.size_grow_speed);
        prs.set("size",            base_p.size);
        prs.set("speed",           base_p.speed);
        prs.set("color",           base_p.color);
        
        if(bitmap_node) {
            if(load_resources) {
                base_p.bitmap =
                    game.bitmaps.get(
                        bitmap_str, bitmap_node
                    );
            }
            base_p.type = PARTICLE_TYPE_BITMAP;
        } else {
            base_p.type = PARTICLE_TYPE_CIRCLE;
        }
        
        base_p.time = base_p.duration;
        base_p.priority = PARTICLE_PRIORITY_MEDIUM;
        
        particle_generator new_pg(emission_interval_float, base_p, number_int);
        
        grs.set("number_deviation",      new_pg.number_deviation);
        grs.set("duration_deviation",    new_pg.duration_deviation);
        grs.set("friction_deviation",    new_pg.friction_deviation);
        grs.set("gravity_deviation",     new_pg.gravity_deviation);
        grs.set("size_deviation",        new_pg.size_deviation);
        grs.set("pos_deviation",         new_pg.pos_deviation);
        grs.set("speed_deviation",       new_pg.speed_deviation);
        grs.set("angle",                 new_pg.angle);
        grs.set("angle_deviation",       new_pg.angle_deviation);
        grs.set("total_speed",           new_pg.total_speed);
        grs.set("total_speed_deviation", new_pg.total_speed_deviation);
        
        new_pg.angle = deg_to_rad(new_pg.angle);
        new_pg.angle_deviation = deg_to_rad(new_pg.angle_deviation);
        
        new_pg.id =
            MOB_PARTICLE_GENERATOR_STATUS +
            game.custom_particle_generators.size();
            
        game.custom_particle_generators[name_str] = new_pg;
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Loads a data file from the game's content.
 */
data_node load_data_file(const string &file_name) {
    data_node n = data_node(file_name);
    if(!n.file_was_opened) {
        log_error("Could not open data file " + file_name + "!");
    }
    
    return n;
}


/* ----------------------------------------------------------------------------
 * Loads the game's fonts.
 */
void load_fonts() {
    const int STANDARD_FONT_RANGES_SIZE = 2;
    int standard_font_ranges[STANDARD_FONT_RANGES_SIZE] = {
        0x0020, 0x007E, //ASCII
        /*0x00A0, 0x00A1, //Non-breaking space and inverted !
        0x00BF, 0x00FF, //Inverted ? and European vowels and such*/
    };
    
    const int COUNTER_FONT_RANGES_SIZE = 6;
    int counter_font_ranges[COUNTER_FONT_RANGES_SIZE] = {
        0x002D, 0x002D, //Dash
        0x002F, 0x0039, //Slash and numbers
        0x0078, 0x0078, //x
    };
    
    const int VALUE_FONT_RANGES_SIZE = 6;
    int value_font_ranges[VALUE_FONT_RANGES_SIZE] = {
        0x0024, 0x0024, //Dollar sign
        0x002D, 0x002D, //Dash
        0x0030, 0x0039, //Numbers
    };
    
    //We can't load the fonts directly because we want to set the ranges.
    //So we load them into bitmaps first.
    
    //Main font.
    ALLEGRO_BITMAP* temp_font_bmp = load_bmp(game.asset_file_names.main_font);
    if(temp_font_bmp) {
        game.fonts.main =
            al_grab_font_from_bitmap(
                temp_font_bmp,
                STANDARD_FONT_RANGES_SIZE / 2, standard_font_ranges
            );
    }
    al_destroy_bitmap(temp_font_bmp);
    
    //Area name font.
    temp_font_bmp = load_bmp(game.asset_file_names.area_name_font);
    if(temp_font_bmp) {
        game.fonts.area_name =
            al_grab_font_from_bitmap(
                temp_font_bmp,
                STANDARD_FONT_RANGES_SIZE / 2, standard_font_ranges
            );
    }
    al_destroy_bitmap(temp_font_bmp);
    
    //Counter font.
    temp_font_bmp = load_bmp(game.asset_file_names.counter_font);
    if(temp_font_bmp) {
        game.fonts.counter =
            al_grab_font_from_bitmap(
                temp_font_bmp,
                COUNTER_FONT_RANGES_SIZE / 2, counter_font_ranges
            );
    }
    al_destroy_bitmap(temp_font_bmp);
    
    //Value font.
    temp_font_bmp = load_bmp(game.asset_file_names.value_font);
    if(temp_font_bmp) {
        game.fonts.value =
            al_grab_font_from_bitmap(
                temp_font_bmp,
                VALUE_FONT_RANGES_SIZE / 2, value_font_ranges
            );
    }
    al_destroy_bitmap(temp_font_bmp);
    
    game.fonts.builtin = al_create_builtin_font();
}


/* ----------------------------------------------------------------------------
 * Loads the game's configuration file.
 */
void load_game_config() {
    data_node file = load_data_file(CONFIG_FILE);
    
    game.config.load(&file);
    
    al_set_window_title(game.display, game.config.name.c_str());
}


/* ----------------------------------------------------------------------------
 * Loads the hazards from the game data.
 */
void load_hazards() {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Hazards");
    }
    
    vector<string> hazard_files =
        folder_to_vector(HAZARDS_FOLDER_PATH, false);
        
    for(size_t h = 0; h < hazard_files.size(); ++h) {
        data_node file =
            load_data_file(HAZARDS_FOLDER_PATH + "/" + hazard_files[h]);
        if(!file.file_was_opened) continue;
        
        hazard new_h;
        reader_setter rs(&file);
        
        string effects_str;
        string liquid_str;
        data_node* effects_node = NULL;
        data_node* liquid_node = NULL;
        
        rs.set("name", new_h.name);
        rs.set("color", new_h.main_color);
        rs.set("effects", effects_str, &effects_node);
        rs.set("liquid", liquid_str, &liquid_node);
        
        if(effects_node) {
            vector<string> effects_strs = semicolon_list_to_vector(effects_str);
            for(size_t e = 0; e < effects_strs.size(); ++e) {
                string effect_name = effects_strs[e];
                if(
                    game.status_types.find(effect_name) ==
                    game.status_types.end()
                ) {
                    log_error(
                        "Unknown status effect \"" + effect_name + "\"!",
                        effects_node
                    );
                } else {
                    new_h.effects.push_back(
                        game.status_types[effect_name]
                    );
                }
            }
        }
        
        if(liquid_node) {
            if(game.liquids.find(liquid_str) == game.liquids.end()) {
                log_error(
                    "Unknown liquid \"" + liquid_str + "\"!",
                    liquid_node
                );
            } else {
                new_h.associated_liquid = game.liquids[liquid_str];
            }
        }
        
        game.hazards[new_h.name] = new_h;
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the liquids from the game data.
 */
void load_liquids(const bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Liquid types");
    }
    
    vector<string> liquid_files =
        folder_to_vector(LIQUIDS_FOLDER_PATH, false);
        
    for(size_t l = 0; l < liquid_files.size(); ++l) {
        data_node file =
            load_data_file(LIQUIDS_FOLDER_PATH + "/" + liquid_files[l]);
        if(!file.file_was_opened) continue;
        
        liquid* new_l = new liquid();
        reader_setter rs(&file);
        string animation_str;
        
        rs.set("name", new_l->name);
        rs.set("animation", animation_str);
        rs.set("color", new_l->main_color);
        rs.set("surface_1_speed", new_l->surface_speed[0]);
        rs.set("surface_2_speed", new_l->surface_speed[0]);
        rs.set("surface_alpha", new_l->surface_alpha);
        
        if(load_resources) {
            data_node anim_file =
                load_data_file(ANIMATIONS_FOLDER_PATH + "/" + animation_str);
                
            new_l->anim_db =
                load_animation_database_from_file(&anim_file);
            if(!new_l->anim_db.animations.empty()) {
                new_l->anim_instance =
                    animation_instance(&new_l->anim_db);
                new_l->anim_instance.cur_anim =
                    new_l->anim_db.animations[0];
                new_l->anim_instance.start();
            }
        }
        
        game.liquids[new_l->name] = new_l;
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Loads miscellaneous fixed graphics.
 */
void load_misc_graphics() {
    //Icon.
    game.sys_assets.bmp_icon = load_bmp(game.asset_file_names.icon);
    al_set_display_icon(game.display, game.sys_assets.bmp_icon);
    
    //Graphics.
    game.sys_assets.bmp_checkbox_check = load_bmp(   game.asset_file_names.checkbox_check);
    game.sys_assets.bmp_cursor = load_bmp(           game.asset_file_names.cursor);
    game.sys_assets.bmp_cursor_invalid = load_bmp(   game.asset_file_names.cursor_invalid);
    game.sys_assets.bmp_enemy_spirit = load_bmp(     game.asset_file_names.enemy_spirit);
    game.sys_assets.bmp_idle_glow = load_bmp(        game.asset_file_names.idle_glow);
    game.sys_assets.bmp_mouse_cursor = load_bmp(     game.asset_file_names.mouse_cursor);
    game.sys_assets.bmp_mouse_wd_icon = load_bmp(    game.asset_file_names.mouse_wd_icon);
    game.sys_assets.bmp_mouse_wu_icon = load_bmp(    game.asset_file_names.mouse_wu_icon);
    game.sys_assets.bmp_notification = load_bmp(     game.asset_file_names.notification);
    game.sys_assets.bmp_pikmin_silhouette = load_bmp(game.asset_file_names.pikmin_silhouette);
    game.sys_assets.bmp_pikmin_spirit = load_bmp(    game.asset_file_names.pikmin_spirit);
    game.sys_assets.bmp_rock = load_bmp(             game.asset_file_names.rock);
    game.sys_assets.bmp_shadow = load_bmp(           game.asset_file_names.shadow);
    game.sys_assets.bmp_smack = load_bmp(            game.asset_file_names.smack);
    game.sys_assets.bmp_smoke = load_bmp(            game.asset_file_names.smoke);
    game.sys_assets.bmp_sparkle = load_bmp(          game.asset_file_names.sparkle);
    game.sys_assets.bmp_spotlight = load_bmp(        game.asset_file_names.spotlight);
    game.sys_assets.bmp_swarm_arrow = load_bmp(      game.asset_file_names.swarm_arrow);
    game.sys_assets.bmp_wave_ring = load_bmp(        game.asset_file_names.wave_ring);
    for(unsigned char i = 0; i < 3; ++i) {
        game.sys_assets.bmp_mouse_button_icon[i] =
            load_bmp(game.asset_file_names.mouse_button_icon[i]);
    }
}


/* ----------------------------------------------------------------------------
 * Loads miscellaneous fixed sound effects.
 */
void load_misc_sounds() {
    //Sound effects.
    game.voice =
        al_create_voice(
            44100, ALLEGRO_AUDIO_DEPTH_INT16,   ALLEGRO_CHANNEL_CONF_2
        );
    game.mixer =
        al_create_mixer(
            44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2
        );
    al_attach_mixer_to_voice(game.mixer, game.voice);
    
    game.sys_assets.sfx_attack = load_sample(              "Attack.ogg");
    game.sys_assets.sfx_pikmin_attack = load_sample(       "Pikmin_attack.ogg");
    game.sys_assets.sfx_pikmin_carrying = load_sample(     "Pikmin_carrying.ogg");
    game.sys_assets.sfx_pikmin_carrying_grab = load_sample("Pikmin_carrying_grab.ogg");
    game.sys_assets.sfx_pikmin_caught = load_sample(       "Pikmin_caught.ogg");
    game.sys_assets.sfx_pikmin_dying = load_sample(        "Pikmin_dying.ogg");
    game.sys_assets.sfx_pikmin_held = load_sample(         "Pikmin_held.ogg");
    game.sys_assets.sfx_pikmin_idle = load_sample(         "Pikmin_idle.ogg");
    game.sys_assets.sfx_pikmin_thrown = load_sample(       "Pikmin_thrown.ogg");
    game.sys_assets.sfx_pikmin_plucked = load_sample(      "Pikmin_plucked.ogg");
    game.sys_assets.sfx_pikmin_called = load_sample(       "Pikmin_called.ogg");
    game.sys_assets.sfx_pluck = load_sample(               "Pluck.ogg");
    game.sys_assets.sfx_throw = load_sample(               "Throw.ogg");
    game.sys_assets.sfx_switch_pikmin = load_sample(       "Switch_Pikmin.ogg");
    game.sys_assets.sfx_camera = load_sample(              "Camera.ogg");
}


/* ----------------------------------------------------------------------------
 * Loads the player's options.
 */
void load_options() {
    data_node file = data_node(OPTIONS_FILE_PATH);
    if(!file.file_was_opened) return;
    
    //Init joysticks.
    game.joystick_numbers.clear();
    int n_joysticks = al_get_num_joysticks();
    for(int j = 0; j < n_joysticks; ++j) {
        game.joystick_numbers[al_get_joystick(j)] = j;
    }
    
    //Read the main options.
    game.options.load(&file);
    
    game.win_fullscreen = game.options.intended_win_fullscreen;
    game.win_w = game.options.intended_win_w;
    game.win_h = game.options.intended_win_h;
    
    //Set up the animation editor history.
    reader_setter rs(&file);
    
    game.states.animation_editor_st->history.clear();
    for(size_t h = 0; h < animation_editor::HISTORY_SIZE; ++h) {
        game.states.animation_editor_st->history.push_back("");
        rs.set(
            "animation_editor_history_" + i2s(h + 1),
            game.states.animation_editor_st->history[h]
        );
    }
}


/* ----------------------------------------------------------------------------
 * Loads an audio sample from the game's content.
 */
sample_struct load_sample(const string &file_name) {
    ALLEGRO_SAMPLE* sample =
        al_load_sample((AUDIO_FOLDER_PATH + "/" + file_name).c_str());
    if(!sample) {
        log_error("Could not open audio sample " + file_name + "!");
    }
    
    return sample_struct(sample, game.mixer);
}


/* ----------------------------------------------------------------------------
 * Loads the spike damage types available.
 */
void load_spike_damage_types() {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Spike damage types");
    }
    
    vector<string> type_files =
        folder_to_vector(SPIKE_DAMAGES_FOLDER_PATH, false);
        
    for(size_t t = 0; t < type_files.size(); ++t) {
        data_node file =
            load_data_file(SPIKE_DAMAGES_FOLDER_PATH + "/" + type_files[t]);
        if(!file.file_was_opened) continue;
        
        spike_damage_type new_t;
        reader_setter rs(&file);
        
        string particle_generator_name;
        data_node* damage_node = NULL;
        data_node* particle_generator_node = NULL;
        
        rs.set("name", new_t.name);
        rs.set("damage", new_t.damage, &damage_node);
        rs.set("ingestion_only", new_t.ingestion_only);
        rs.set("is_damage_ratio", new_t.is_damage_ratio);
        rs.set(
            "particle_generator", particle_generator_name,
            &particle_generator_node
        );
        
        if(particle_generator_node) {
            if(
                game.custom_particle_generators.find(particle_generator_name) ==
                game.custom_particle_generators.end()
            ) {
                log_error(
                    "Unknown particle generator \"" +
                    particle_generator_name + "\"!", particle_generator_node
                );
            } else {
                new_t.particle_gen =
                    &game.custom_particle_generators[particle_generator_name];
                new_t.particle_offset_pos =
                    s2p(
                        file.get_child_by_name("particle_offset")->value,
                        &new_t.particle_offset_z
                    );
            }
        }
        
        if(new_t.damage == 0) {
            log_error(
                "Spike damage type \"" + new_t.name +
                "\" needs a damage number!",
                (damage_node ? damage_node : &file)
            );
        }
        
        game.spike_damage_types[new_t.name] = new_t;
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Loads spray types from the game data.
 */
void load_spray_types(const bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Spray types");
    }
    
    vector<string> type_files =
        folder_to_vector(SPRAYS_FOLDER_PATH, false);
        
    vector<spray_type> temp_types;
    
    for(size_t t = 0; t < type_files.size(); ++t) {
        data_node file =
            load_data_file(SPRAYS_FOLDER_PATH + "/" + type_files[t]);
        if(!file.file_was_opened) continue;
        
        spray_type new_t;
        reader_setter rs(&file);
        
        string effects_str;
        string icon_str;
        data_node* effects_node = NULL;
        data_node* icon_node = NULL;
        
        rs.set("name", new_t.name);
        rs.set("effects", effects_str, &effects_node);
        rs.set("icon", icon_str, &icon_node);
        rs.set("group", new_t.group);
        rs.set("angle", new_t.angle);
        rs.set("distance_range", new_t.distance_range);
        rs.set("angle_range", new_t.angle_range);
        rs.set("color", new_t.main_color);
        rs.set("ingredients_needed", new_t.ingredients_needed);
        rs.set("buries_pikmin", new_t.buries_pikmin);
        
        if(effects_node) {
            vector<string> effects_strs =
                semicolon_list_to_vector(effects_node->value);
            for(size_t e = 0; e < effects_strs.size(); ++e) {
                string effect_name = effects_strs[e];
                if(
                    game.status_types.find(effect_name) ==
                    game.status_types.end()
                ) {
                    log_error(
                        "Unknown status effect \"" + effect_name + "\"!",
                        effects_node
                    );
                } else {
                    new_t.effects.push_back(game.status_types[effect_name]);
                }
            }
        }
        
        new_t.angle = deg_to_rad(new_t.angle);
        new_t.angle_range = deg_to_rad(new_t.angle_range);
        
        if(load_resources) {
            new_t.bmp_spray = game.bitmaps.get(icon_str, icon_node);
        }
        
        temp_types.push_back(new_t);
    }
    
    //Check the registered order and sort.
    for(size_t t = 0; t < temp_types.size(); ++t) {
        if(
            find(
                game.config.spray_order_strings.begin(),
                game.config.spray_order_strings.end(),
                temp_types[t].name
            ) == game.config.spray_order_strings.end()
        ) {
            log_error(
                "Spray type \"" + temp_types[t].name + "\" was not found "
                "in the spray order list in the config file!"
            );
            game.config.spray_order_strings.push_back(temp_types[t].name);
        }
    }
    for(size_t o = 0; o < game.config.spray_order_strings.size(); ++o) {
        string s = game.config.spray_order_strings[o];
        bool found = false;
        for(size_t t = 0; t < temp_types.size(); ++t) {
            if(temp_types[t].name == s) {
                game.spray_types.push_back(temp_types[t]);
                found = true;
            }
        }
        
        if(!found) {
            log_error(
                "Unknown spray type \"" + s + "\" found "
                "in the spray order list in the config file!"
            );
        }
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Loads status effect types from the game data.
 */
void load_status_types(const bool load_resources) {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Status types");
    }
    
    vector<string> type_files =
        folder_to_vector(STATUSES_FOLDER_PATH, false);
        
    for(size_t t = 0; t < type_files.size(); ++t) {
        data_node file =
            load_data_file(STATUSES_FOLDER_PATH + "/" + type_files[t]);
        if(!file.file_was_opened) continue;
        
        status_type* new_t = new status_type();
        reader_setter rs(&file);
        
        bool affects_pikmin_bool = false;
        bool affects_leaders_bool = false;
        bool affects_enemies_bool = false;
        bool affects_others_bool = false;
        string particle_offset_str;
        string particle_gen_str;
        data_node* particle_gen_node = NULL;
        
        rs.set("name",                    new_t->name);
        rs.set("color",                   new_t->color);
        rs.set("tint",                    new_t->tint);
        rs.set("glow",                    new_t->glow);
        rs.set("affects_pikmin",          affects_pikmin_bool);
        rs.set("affects_leaders",         affects_leaders_bool);
        rs.set("affects_enemies",         affects_enemies_bool);
        rs.set("affects_others",          affects_others_bool);
        rs.set("removable_with_whistle",  new_t->removable_with_whistle);
        rs.set("auto_remove_time",        new_t->auto_remove_time);
        rs.set("health_change_ratio",     new_t->health_change_ratio);
        rs.set("causes_disable",          new_t->causes_disable);
        rs.set("causes_flailing",         new_t->causes_flailing);
        rs.set("causes_panic",            new_t->causes_panic);
        rs.set("disabled_state_inedible", new_t->disabled_state_inedible);
        rs.set("speed_multiplier",        new_t->speed_multiplier);
        rs.set("attack_multiplier",       new_t->attack_multiplier);
        rs.set("defense_multiplier",      new_t->defense_multiplier);
        rs.set("maturity_change_amount",  new_t->maturity_change_amount);
        rs.set("disables_attack",         new_t->disables_attack);
        rs.set("turns_invisible",         new_t->turns_invisible);
        rs.set("anim_speed_multiplier",   new_t->anim_speed_multiplier);
        rs.set("animation",               new_t->animation_name);
        rs.set("animation_mob_scale",     new_t->animation_mob_scale);
        rs.set("particle_generator",      particle_gen_str, &particle_gen_node);
        rs.set("particle_offset",         particle_offset_str);
        
        new_t->affects = 0;
        if(affects_pikmin_bool) {
            new_t->affects |= STATUS_AFFECTS_PIKMIN;
        }
        if(affects_leaders_bool) {
            new_t->affects |= STATUS_AFFECTS_LEADERS;
        }
        if(affects_enemies_bool) {
            new_t->affects |= STATUS_AFFECTS_ENEMIES;
        }
        if(affects_others_bool) {
            new_t->affects |= STATUS_AFFECTS_OTHERS;
        }
        
        if(particle_gen_node) {
            if(
                game.custom_particle_generators.find(particle_gen_str) ==
                game.custom_particle_generators.end()
            ) {
                log_error(
                    "Unknown particle generator \"" +
                    particle_gen_str + "\"!", particle_gen_node
                );
            } else {
                new_t->generates_particles =
                    true;
                new_t->particle_gen =
                    &game.custom_particle_generators[particle_gen_str];
                new_t->particle_offset_pos =
                    s2p(particle_offset_str, &new_t->particle_offset_z);
            }
        }
        
        if(load_resources) {
            if(!new_t->animation_name.empty()) {
                data_node anim_file =
                    load_data_file(
                        ANIMATIONS_FOLDER_PATH + "/" + new_t->animation_name
                    );
                new_t->anim_db = load_animation_database_from_file(&anim_file);
                if(!new_t->anim_db.animations.empty()) {
                    new_t->anim_instance =
                        animation_instance(&new_t->anim_db);
                    new_t->anim_instance.cur_anim =
                        new_t->anim_db.animations[0];
                    new_t->anim_instance.start();
                }
            }
        }
        
        game.status_types[new_t->name] = new_t;
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Loads the animations that are used system-wide.
 */
void load_system_animations() {
    data_node system_animations_file =
        load_data_file(SYSTEM_ANIMATIONS_FILE_PATH);
        
    init_single_animation(
        &system_animations_file, "leader_damage_sparks", game.sys_assets.spark_animation
    );
}


/* ----------------------------------------------------------------------------
 * Loads the weather conditions available.
 */
void load_weather() {
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Weather");
    }
    
    vector<string> weather_files =
        folder_to_vector(WEATHER_FOLDER_PATH, false);
        
    for(size_t w = 0; w < weather_files.size(); ++w) {
        data_node file =
            load_data_file(WEATHER_FOLDER_PATH + "/" + weather_files[w]);
        if(!file.file_was_opened) continue;
        
        weather new_w;
        reader_setter rs(&file);
        
        //General.
        rs.set("name", new_w.name);
        rs.set("fog_near", new_w.fog_near);
        rs.set("fog_far", new_w.fog_far);
        
        new_w.fog_near = std::max(new_w.fog_near, 0.0f);
        new_w.fog_far = std::max(new_w.fog_far, new_w.fog_near);
        
        //Lighting.
        vector<std::pair<size_t, string> > lighting_table =
            get_weather_table(file.get_child_by_name("lighting"));
            
        for(size_t p = 0; p < lighting_table.size(); ++p) {
            new_w.daylight.push_back(
                std::make_pair(
                    lighting_table[p].first,
                    s2c(lighting_table[p].second)
                )
            );
        }
        
        //Sun's strength.
        vector<std::pair<size_t, string> > sun_strength_table =
            get_weather_table(file.get_child_by_name("sun_strength"));
            
        for(size_t p = 0; p < sun_strength_table.size(); ++p) {
            new_w.sun_strength.push_back(
                std::make_pair(
                    sun_strength_table[p].first,
                    s2i(sun_strength_table[p].second)
                )
            );
        }
        
        //Blackout effect's strength.
        vector<std::pair<size_t, string> > blackout_strength_table =
            get_weather_table(
                file.get_child_by_name("blackout_strength")
            );
            
        for(size_t p = 0; p < blackout_strength_table.size(); ++p) {
            new_w.blackout_strength.push_back(
                std::make_pair(
                    blackout_strength_table[p].first,
                    s2i(blackout_strength_table[p].second)
                )
            );
        }
        
        //Fog.
        vector<std::pair<size_t, string> > fog_color_table =
            get_weather_table(
                file.get_child_by_name("fog_color")
            );
        for(size_t p = 0; p < fog_color_table.size(); ++p) {
            new_w.fog_color.push_back(
                std::make_pair(
                    fog_color_table[p].first,
                    s2c(fog_color_table[p].second)
                )
            );
        }
        
        //Save it in the map.
        game.weather_conditions[new_w.name] = new_w;
    }
    
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Unloads the loaded area from memory.
 */
void unload_area() {
    game.cur_area_data.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads custom particle generators loaded from memory.
 */
void unload_custom_particle_generators() {
    for(
        auto g = game.custom_particle_generators.begin();
        g != game.custom_particle_generators.end();
        ++g
    ) {
        game.bitmaps.detach(g->second.base_particle.bitmap);
    }
    game.custom_particle_generators.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads hazards loaded in memory.
 */
void unload_hazards() {
    game.hazards.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded liquids from memory.
 */
void unload_liquids() {
    for(auto &l : game.liquids) {
        l.second->anim_db.destroy();
        delete l.second;
    }
    game.liquids.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads miscellaneous graphics, sounds, and other resources.
 */
void unload_misc_resources() {
    al_destroy_bitmap(game.sys_assets.bmp_checkbox_check);
    al_destroy_bitmap(game.sys_assets.bmp_cursor);
    al_destroy_bitmap(game.sys_assets.bmp_cursor_invalid);
    al_destroy_bitmap(game.sys_assets.bmp_enemy_spirit);
    al_destroy_bitmap(game.sys_assets.bmp_icon);
    al_destroy_bitmap(game.sys_assets.bmp_idle_glow);
    al_destroy_bitmap(game.sys_assets.bmp_mouse_cursor);
    al_destroy_bitmap(game.sys_assets.bmp_mouse_wd_icon);
    al_destroy_bitmap(game.sys_assets.bmp_mouse_wu_icon);
    al_destroy_bitmap(game.sys_assets.bmp_notification);
    al_destroy_bitmap(game.sys_assets.bmp_pikmin_silhouette);
    al_destroy_bitmap(game.sys_assets.bmp_pikmin_spirit);
    al_destroy_bitmap(game.sys_assets.bmp_rock);
    al_destroy_bitmap(game.sys_assets.bmp_shadow);
    al_destroy_bitmap(game.sys_assets.bmp_smack);
    al_destroy_bitmap(game.sys_assets.bmp_smoke);
    al_destroy_bitmap(game.sys_assets.bmp_sparkle);
    al_destroy_bitmap(game.sys_assets.bmp_spotlight);
    al_destroy_bitmap(game.sys_assets.bmp_swarm_arrow);
    al_destroy_bitmap(game.sys_assets.bmp_wave_ring);
    for(unsigned char i = 0; i < 3; ++i) {
        game.bitmaps.detach(game.sys_assets.bmp_mouse_button_icon[i]);
    }
    
    game.sys_assets.sfx_attack.destroy();
    game.sys_assets.sfx_pikmin_attack.destroy();
    game.sys_assets.sfx_pikmin_carrying.destroy();
    game.sys_assets.sfx_pikmin_carrying_grab.destroy();
    game.sys_assets.sfx_pikmin_caught.destroy();
    game.sys_assets.sfx_pikmin_dying.destroy();
    game.sys_assets.sfx_pikmin_held.destroy();
    game.sys_assets.sfx_pikmin_idle.destroy();
    game.sys_assets.sfx_pikmin_thrown.destroy();
    game.sys_assets.sfx_pikmin_plucked.destroy();
    game.sys_assets.sfx_pikmin_called.destroy();
    game.sys_assets.sfx_throw.destroy();
    game.sys_assets.sfx_switch_pikmin.destroy();
    game.sys_assets.sfx_camera.destroy();
}


/* ----------------------------------------------------------------------------
 * Unloads spike damage types loaded in memory.
 */
void unload_spike_damage_types() {
    game.spike_damage_types.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded spray types from memory.
 */
void unload_spray_types() {
    for(size_t s = 0; s < game.spray_types.size(); ++s) {
        game.bitmaps.detach(game.spray_types[s].bmp_spray);
    }
    game.spray_types.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded status effect types from memory.
 */
void unload_status_types(const bool unload_resources) {

    for(auto &s : game.status_types) {
        if(unload_resources) {
            s.second->anim_db.destroy();
        }
        delete s.second;
    }
    game.status_types.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads loaded weather conditions.
 */
void unload_weather() {
    game.weather_conditions.clear();
}
