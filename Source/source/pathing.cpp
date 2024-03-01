/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Paths, path-finding, and related functions.
 */

#include "pathing.h"

#include "functions.h"
#include "game.h"
#include "mobs/mob_utils.h"


using std::map;
using std::string;
using std::unordered_set;
using std::vector;


namespace PATHS {

//Minimum radius of a path stop.
const float MIN_STOP_RADIUS = 16.0f;

}


/**
 * @brief Constructs a new path follow settings object.
 */
path_follow_settings::path_follow_settings() :
    target_mob(nullptr),
    final_target_distance(MOB::DEF_CHASE_TARGET_DISTANCE),
    flags(0) {
    
}


/**
 * @brief Constructs a new path link object.
 *
 * @param start_ptr The path stop at the start of this link.
 * @param end_ptr The path stop at the end of this link.
 * @param end_nr Index number of the path stop at the end of this link.
 */
path_link::path_link(path_stop* start_ptr, path_stop* end_ptr, size_t end_nr) :
    start_ptr(start_ptr),
    end_ptr(end_ptr),
    end_nr(end_nr),
    type(PATH_LINK_TYPE_NORMAL),
    distance(0),
    blocked_by_obstacle(false) {
    
}


/**
 * @brief Calculates and stores the distance between the two stops.
 * Because the link doesn't know about the starting stop,
 * you need to provide it as a parameter when calling the function.
 *
 * @param start_ptr The path stop at the start of this link.
 */
void path_link::calculate_dist(const path_stop* start_ptr) {
    distance = dist(start_ptr->pos, end_ptr->pos).to_float();
}


/**
 * @brief Clones a path link's properties onto another,
 * not counting the path stops.
 *
 * @param destination Path link to clone the data into.
 */
void path_link::clone(path_link* destination) const {
    destination->type = type;
}


/**
 * @brief Checks if a path link is a plain one-way link,
 * or if it's actually one part of a normal, two-way link.
 *
 * @return Whether it's one-way.
 */
bool path_link::is_one_way() const {
    return end_ptr->get_link(start_ptr) == NULL;
}


/**
 * @brief Clears all info.
 */
void path_manager::clear() {
    obstructions.clear();
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        for(size_t l = 0; l < s_ptr->links.size(); ++l) {
            game.cur_area_data.path_stops[s]->links[l]->blocked_by_obstacle =
                false;
        }
    }
}


/**
 * @brief Handles the area having been loaded. It checks all path stops
 * and saves any sector hazards found.
 */
void path_manager::handle_area_load() {
    //Go through all path stops and check if they're on hazardous sectors.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        if(!s_ptr->sector_ptr) continue;
        if(s_ptr->sector_ptr->hazards.empty()) continue;
        hazardous_stops.insert(s_ptr);
    }
}


/**
 * @brief Handles an obstacle having been placed. This way, any link with that
 * obstruction can get updated.
 *
 * @param m Pointer to the obstacle mob that got added.
 */
void path_manager::handle_obstacle_add(mob* m) {
    //Add the obstacle to our list, if needed.
    bool paths_changed = false;
    
    //Go through all path links and check if they have obstacles.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        
        for(size_t l = 0; l < s_ptr->links.size(); ++l) {
            path_link* l_ptr = game.cur_area_data.path_stops[s]->links[l];
            
            if(
                circle_intersects_line_seg(
                    m->pos, m->radius,
                    s_ptr->pos, l_ptr->end_ptr->pos
                )
            ) {
                obstructions[l_ptr].insert(m);
                l_ptr->blocked_by_obstacle = true;
                paths_changed = true;
            }
        }
    }
    
    if(paths_changed) {
        //Re-calculate the paths of mobs taking paths.
        for(size_t m2 = 0; m2 < game.states.gameplay->mobs.all.size(); ++m2) {
            mob* m2_ptr = game.states.gameplay->mobs.all[m2];
            if(!m2_ptr->path_info) continue;
            
            m2_ptr->fsm.run_event(MOB_EV_PATHS_CHANGED);
        }
    }
}


/**
 * @brief Handles an obstacle having been cleared. This way, any link with that
 * obstruction can get updated.
 *
 * @param m Pointer to the obstacle mob that got cleared.
 */
void path_manager::handle_obstacle_remove(mob* m) {
    //Remove the obstacle from our list, if it's there.
    bool paths_changed = false;
    
    for(auto o = obstructions.begin(); o != obstructions.end();) {
        bool to_delete = false;
        if(o->second.erase(m) > 0) {
            if(o->second.empty()) {
                o->first->blocked_by_obstacle = false;
                to_delete = true;
                paths_changed = true;
            }
        }
        if(to_delete) {
            o = obstructions.erase(o);
        } else {
            ++o;
        }
    }
    
    if(paths_changed) {
        //Re-calculate the paths of mobs taking paths.
        for(size_t m2 = 0; m2 < game.states.gameplay->mobs.all.size(); ++m2) {
            mob* m2_ptr = game.states.gameplay->mobs.all[m2];
            if(!m2_ptr->path_info) continue;
            
            m2_ptr->fsm.run_event(MOB_EV_PATHS_CHANGED);
        }
    }
}


/**
 * @brief Handles a sector having changed its hazards.
 * This way, any stop on that sector can be updated.
 *
 * @param sector_ptr Pointer to the sector whose hazards got updated.
 */
void path_manager::handle_sector_hazard_change(sector* sector_ptr) {
    //Remove relevant stops from our list.
    bool paths_changed = false;
    
    for(auto s = hazardous_stops.begin(); s != hazardous_stops.end();) {
        bool to_delete = false;
        if((*s)->sector_ptr == sector_ptr) {
            paths_changed = true;
            if(sector_ptr->hazards.empty()) {
                //We only want to delete it if it became hazardless.
                to_delete = true;
            }
        }
        
        if(to_delete) {
            s = hazardous_stops.erase(s);
        } else {
            ++s;
        }
    }
    
    if(paths_changed) {
        //Re-calculate the paths of mobs taking paths.
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
            mob* m_ptr = game.states.gameplay->mobs.all[m];
            if(!m_ptr->path_info) continue;
            
            m_ptr->fsm.run_event(MOB_EV_PATHS_CHANGED);
        }
    }
}


/**
 * @brief Constructs a new path stop object.
 *
 * @param pos Its coordinates.
 * @param links List of path links, linking it to other stops.
 */
path_stop::path_stop(const point &pos, const vector<path_link*> &links) :
    pos(pos),
    radius(PATHS::MIN_STOP_RADIUS),
    flags(0),
    links(links),
    sector_ptr(nullptr) {
    
}


/**
 * @brief Destroys the path stop object.
 */
path_stop::~path_stop() {
    while(!links.empty()) {
        delete *(links.begin());
        links.erase(links.begin());
    }
}


/**
 * @brief Adds a link between this stop and another, whether it's
 * one-way or not. Also adds the link to the other stop, if applicable.
 * If these two stops already had some link, it gets removed.
 *
 * @param other_stop Pointer to the other stop.
 * @param normal Normal link? False means one-way link.
 */
void path_stop::add_link(path_stop* other_stop, const bool normal) {
    PATH_LINK_TYPES link_type = PATH_LINK_TYPE_NORMAL;
    
    path_link* old_link_data = get_link(other_stop);
    if(!old_link_data) {
        old_link_data = other_stop->get_link(this);
    }
    if(old_link_data) {
        link_type = old_link_data->type;
    }
    
    remove_link(old_link_data);
    other_stop->remove_link(this);
    
    path_link* new_link = new path_link(this, other_stop, INVALID);
    new_link->type = link_type;
    links.push_back(new_link);
    
    if(normal) {
        new_link = new path_link(other_stop, this, INVALID);
        new_link->type = link_type;
        other_stop->links.push_back(new_link);
    }
}


/**
 * @brief Calculates the distance between it and all neighbors.
 */
void path_stop::calculate_dists() {
    for(size_t l = 0; l < links.size(); ++l) {
        links[l]->calculate_dist(this);
    }
}


/**
 * @brief Calculates the distance between it and all neighbors, and then
 * goes through the neighbors and updates their distance back to this stop,
 * if that neighbor links back.
 */
void path_stop::calculate_dists_plus_neighbors() {
    for(size_t l = 0; l < links.size(); ++l) {
        path_link* l_ptr = links[l];
        l_ptr->calculate_dist(this);
    }
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        path_link* l_ptr = s_ptr->get_link(this);
        if(l_ptr) {
            l_ptr->calculate_dist(s_ptr);
        }
    }
}


/**
 * @brief Clones a path stop's properties onto another, not counting the links.
 *
 * @param destination Path stop to clone the data into.
 */
void path_stop::clone(path_stop* destination) const {
    destination->radius = radius;
    destination->flags = flags;
    destination->label = label;
}


/**
 * @brief Returns the pointer of the link between this stop and another.
 * The links in memory are one-way, meaning that if the only link
 * is from the other stop to this one, it will not count.
 *
 * @param other_stop Path stop to check against.
 * @return The link, or NULL if it does not link to that stop.
 */
path_link* path_stop::get_link(const path_stop* other_stop) const {
    for(size_t l = 0; l < links.size(); ++l) {
        if(links[l]->end_ptr == other_stop) return links[l];
    }
    return NULL;
}


/**
 * @brief Removes the specified link.
 * Does nothing if there is no such link.
 *
 * @param link_ptr Pointer to the link to remove.
 */
void path_stop::remove_link(const path_link* link_ptr) {
    for(size_t l = 0; l < links.size(); ++l) {
        if(links[l] == link_ptr) {
            delete links[l];
            links.erase(links.begin() + l);
            return;
        }
    }
}


/**
 * @brief Removes the link between this stop and the specified one.
 * Does nothing if there is no such link.
 *
 * @param other_stop Path stop to remove the link from.
 */
void path_stop::remove_link(const path_stop* other_stop) {
    for(size_t l = 0; l < links.size(); ++l) {
        if(links[l]->end_ptr == other_stop) {
            delete links[l];
            links.erase(links.begin() + l);
            return;
        }
    }
}


/**
 * @brief Checks if a path stop can be taken given some contraints.
 *
 * @param stop_ptr Stop to check.
 * @param settings Settings about how the path should be followed.
 * @param reason If not NULL, the reason is returned here.
 * @return Whether it can be taken.
 */
bool can_take_path_stop(
    path_stop* stop_ptr, const path_follow_settings &settings,
    PATH_BLOCK_REASONS* reason
) {
    sector* sector_ptr = stop_ptr->sector_ptr;
    if(!sector_ptr) {
        //We're probably in the area editor, where things change too often
        //for us to cache the sector pointer and access said cache.
        //Let's calculate now real quick.
        sector_ptr = get_sector(stop_ptr->pos, NULL, false);
        if(!sector_ptr) {
            //It's really the void. Nothing that can be done here then.
            if(reason) *reason = PATH_BLOCK_REASON_STOP_IN_VOID;
            return false;
        }
    }
    
    return can_take_path_stop(stop_ptr, settings, sector_ptr, reason);
}


/**
 * @brief Checks if a path stop can be taken given some contraints.
 *
 * @param stop_ptr Stop to check.
 * @param settings Settings about how the path should be followed.
 * @param sector_ptr Pointer to the sector this stop is on.
 * @param reason If not NULL, the reason is returned here.
 * @return Whether it can take it.
 */
bool can_take_path_stop(
    const path_stop* stop_ptr, const path_follow_settings &settings,
    sector* sector_ptr, PATH_BLOCK_REASONS* reason
) {
    //Check if the end stop has limitations based on the stop flags.
    if(
        has_flag(stop_ptr->flags, PATH_STOP_SCRIPT_ONLY) &&
        !has_flag(settings.flags, PATH_FOLLOW_FLAG_SCRIPT_USE)
    ) {
        if(reason) *reason = PATH_BLOCK_REASON_NOT_IN_SCRIPT;
        return false;
    }
    if(
        has_flag(stop_ptr->flags, PATH_STOP_LIGHT_LOAD_ONLY) &&
        !has_flag(settings.flags, PATH_FOLLOW_FLAG_LIGHT_LOAD)
    ) {
        if(reason) *reason = PATH_BLOCK_REASON_NOT_LIGHT_LOAD;
        return false;
    }
    if(
        has_flag(stop_ptr->flags, PATH_STOP_AIRBORNE_ONLY) &&
        !has_flag(settings.flags, PATH_FOLLOW_FLAG_AIRBORNE)
    ) {
        if(reason) *reason = PATH_BLOCK_REASON_NOT_AIRBORNE;
        return false;
    }
    
    //Check if the travel is limited to stops with a certain label.
    if(!settings.label.empty() && stop_ptr->label != settings.label) {
        if(reason) *reason = PATH_BLOCK_REASON_NOT_RIGHT_LABEL;
        return false;
    }
    
    //Check if the end stop is hazardous, by checking its sector.
    bool touching_hazard =
        sector_ptr && (
            !sector_ptr->hazard_floor ||
            !has_flag(settings.flags, PATH_FOLLOW_FLAG_AIRBORNE)
        );
        
    if(
        !has_flag(settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES) &&
        touching_hazard &&
        sector_ptr &&
        !sector_ptr->hazards.empty()
    ) {
        for(size_t sh = 0; sh < sector_ptr->hazards.size(); ++sh) {
            bool invulnerable = false;
            for(size_t ih = 0; ih < settings.invulnerabilities.size(); ++ih) {
                if(
                    settings.invulnerabilities[ih] ==
                    sector_ptr->hazards[sh]
                ) {
                    invulnerable = true;
                    break;
                }
            }
            if(!invulnerable) {
                if(reason) *reason = PATH_BLOCK_REASON_HAZARDOUS_STOP;
                return false;
            }
        }
    }
    
    //All good!
    return true;
}


/**
 * @brief Checks if a link can be traversed given some contraints.
 *
 * @param link_ptr Link to check.
 * @param settings Settings about how the path should be followed.
 * @param reason If not NULL, the reason is returned here.
 * @return Whether it can traverse it.
 */
bool can_traverse_path_link(
    path_link* link_ptr, const path_follow_settings &settings,
    PATH_BLOCK_REASONS* reason
) {
    if(reason) *reason = PATH_BLOCK_REASON_NONE;
    
    //Check if there's an obstacle in the way.
    if(
        !has_flag(settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES) &&
        link_ptr->blocked_by_obstacle
    ) {
        if(reason) *reason = PATH_BLOCK_REASON_OBSTACLE;
        return false;
    }
    
    //Get the start and end sectors.
    sector* start_sector = link_ptr->start_ptr->sector_ptr;
    if(!start_sector) {
        //We're probably in the area editor, where things change too often
        //for us to cache the sector pointer and access said cache.
        //Let's calculate now real quick.
        start_sector = get_sector(link_ptr->start_ptr->pos, NULL, false);
        if(!start_sector) {
            //It's really the void. Nothing that can be done here then.
            if(reason) *reason = PATH_BLOCK_REASON_STOP_IN_VOID;
            return false;
        }
    }
    sector* end_sector = link_ptr->end_ptr->sector_ptr;
    if(!end_sector) {
        //Same as above.
        end_sector = get_sector(link_ptr->end_ptr->pos, NULL, false);
        if(!end_sector) {
            if(reason) *reason = PATH_BLOCK_REASON_STOP_IN_VOID;
            return false;
        }
    }
    
    //Check if the link has limitations based on link type.
    switch(link_ptr->type) {
    case PATH_LINK_TYPE_NORMAL: {
        break;
    } case PATH_LINK_TYPE_LEDGE: {
        if(
            !has_flag(settings.flags, PATH_FOLLOW_FLAG_AIRBORNE) &&
            (end_sector->z - start_sector->z) > GEOMETRY::STEP_HEIGHT
        ) {
            if(reason) *reason = PATH_BLOCK_REASON_UP_LEDGE;
            return false;
        }
        break;
    }
    }
    
    //Check if there's any problem with the stop.
    if(!can_take_path_stop(link_ptr->end_ptr, settings, end_sector, reason)) {
        return false;
    }
    
    //All good!
    return true;
}


/**
 * @brief Traverses a graph using the depth first search algorithm.
 *
 * @param nodes Vector of nodes.
 * @param visited Set with the visited nodes.
 * @param start Starting node.
 */
void depth_first_search(
    vector<path_stop*> &nodes, unordered_set<path_stop*> &visited,
    path_stop* start
) {
    visited.insert(start);
    unordered_set<path_stop*> links;
    
    for(size_t l = 0; l < start->links.size(); ++l) {
        links.insert(start->links[l]->end_ptr);
    }
    
    for(size_t n = 0; n < nodes.size(); ++n) {
        path_stop* n_ptr = nodes[n];
        if(n_ptr == start) continue;
        if(visited.find(n_ptr) != visited.end()) continue;
        if(n_ptr->get_link(start)) {
            links.insert(n_ptr);
        }
    }
    
    for(auto &l : links) {
        if(visited.find(l) != visited.end()) continue;
        depth_first_search(nodes, visited, l);
    }
}


/**
 * @brief Uses Dijkstra's algorithm to get the shortest path between two nodes.
 *
 * @param final_path The stops to visit, in order, are returned here.
 * @param start_node Start node.
 * @param end_node End node.
 * @param settings Settings about how the path should be followed.
 * @param total_dist If not NULL, place the total path distance here.
 * @return The operation's result.
 */
PATH_RESULTS dijkstra(
    vector<path_stop*> &final_path,
    path_stop* start_node, path_stop* end_node,
    const path_follow_settings &settings,
    float* total_dist
) {
    //https://en.wikipedia.org/wiki/Dijkstra's_algorithm
    
    //All nodes that have never been visited.
    unordered_set<path_stop*> unvisited;
    //Distance from starting node + previous stop on the best solution.
    map<path_stop*, std::pair<float, path_stop*> > data;
    //Whether the end node is in the same graph as the start node or not.
    bool in_graph = true;
    
    //Initialize the algorithm.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        unvisited.insert(s_ptr);
        data[s_ptr] = std::make_pair(FLT_MAX, (path_stop*) NULL);
    }
    
    //The distance between the start node and the start node is 0.
    data[start_node].first = 0;
    
    //Start iterating.
    while(!unvisited.empty()) {
    
        //Figure out what node to work on in this iteration.
        path_stop* shortest_node = NULL;
        float shortest_node_dist = 0;
        std::unordered_set<path_stop*>::iterator shortest_node_it =
            unvisited.end();
            
        std::pair<float, path_stop*> shortest_node_data;
        
        for(auto u = unvisited.begin(); u != unvisited.end(); u++) {
            std::pair<float, path_stop*> d = data[*u];
            if(!shortest_node || d.first < shortest_node_dist) {
                shortest_node = *u;
                shortest_node_dist = d.first;
                shortest_node_data = d;
                shortest_node_it = u;
            }
        }
        
        //If the node we're processing is the end node, then
        //that's it, best path found!
        if(shortest_node == end_node) {
        
            //Construct the path.
            float td = data[end_node].first;
            final_path.clear();
            final_path.push_back(end_node);
            path_stop* next = data[end_node].second;
            while(next) {
                final_path.insert(final_path.begin(), next);
                next = data[next].second;
            }
            
            if(final_path.size() < 2) {
                //If we can't work our way back to the start node, that means
                //the end node is not in the same graph as the start node.
                in_graph = false;
                break;
            } else {
                //Success!
                if(total_dist) *total_dist = td;
                return PATH_RESULT_NORMAL_PATH;
            }
            
        }
        
        //This node's been visited.
        unvisited.erase(shortest_node_it);
        
        //Check the neighbors.
        for(size_t l = 0; l < shortest_node->links.size(); ++l) {
            path_link* l_ptr = shortest_node->links[l];
            
            //If this neighbor's been visited, forget it.
            if(unvisited.find(l_ptr->end_ptr) == unvisited.end()) continue;
            
            //Can this link be traversed?
            if(!can_traverse_path_link(l_ptr, settings)) {
                continue;
            }
            
            float dist_so_far = shortest_node_data.first + l_ptr->distance;
            auto d = &data[l_ptr->end_ptr];
            
            if(dist_so_far < d->first) {
                //Found a shorter path to this node.
                d->first = dist_so_far;
                d->second = shortest_node;
            }
        }
    }
    
    //If we got to this point, there means that there is no available path!
    
    if(!has_flag(settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES)) {
        //Let's try again, this time ignoring obstacles.
        path_follow_settings new_settings = settings;
        enable_flag(new_settings.flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES);
        PATH_RESULTS new_result =
            dijkstra(
                final_path,
                start_node, end_node,
                new_settings,
                total_dist
            );
        if(new_result == PATH_RESULT_NORMAL_PATH) {
            //If we only managed to succeed with this ignore-obstacle attempt,
            //then that means a path exists, but there are obstacles.
            return PATH_RESULT_PATH_WITH_OBSTACLES;
        } else {
            return new_result;
        }
    }
    
    //Nothing that can be done. No path.
    final_path.clear();
    if(total_dist) *total_dist = 0;
    if(!in_graph) {
        return PATH_RESULT_END_STOP_UNREACHABLE;
    } else {
        return PATH_RESULT_ERROR;
    }
}


/**
 * @brief Gets the shortest available path between two points, following
 * the area's path graph.
 *
 * @param start Start coordinates.
 * @param end End coordinates.
 * @param settings Settings about how the path should be followed.
 * @param full_path The stops to visit, in order, are returned here, if any.
 * @param total_dist If not NULL, place the total path distance here.
 * @param start_stop If not NULL, the closest stop to the start is
 * returned here.
 * @param end_stop If not NULL, the closest stop to the end is returned here.
 * @return The operation's result.
 */
PATH_RESULTS get_path(
    const point &start, const point &end,
    const path_follow_settings &settings,
    vector<path_stop*> &full_path, float* total_dist,
    path_stop** start_stop, path_stop** end_stop
) {

    full_path.clear();
    
    if(game.cur_area_data.path_stops.empty()) {
        if(total_dist) *total_dist = 0.0f;
        return PATH_RESULT_DIRECT_NO_STOPS;
    }
    
    point start_to_use =
        has_flag(settings.flags, PATH_FOLLOW_FLAG_FAKED_START) ?
        settings.faked_start :
        start;
        
    point end_to_use =
        has_flag(settings.flags, PATH_FOLLOW_FLAG_FAKED_END) ?
        settings.faked_end :
        end;
        
    //Start by finding the closest stops to the start and finish.
    path_stop* closest_to_start = NULL;
    path_stop* closest_to_end = NULL;
    float closest_to_start_dist = 0.0f;
    float closest_to_end_dist = 0.0f;
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        
        float dist_to_start =
            dist(start_to_use, s_ptr->pos).to_float() - s_ptr->radius;
        float dist_to_end =
            dist(end_to_use, s_ptr->pos).to_float() - s_ptr->radius;
        dist_to_start = std::max(0.0f, dist_to_start);
        dist_to_end = std::max(0.0f, dist_to_end);
        
        bool is_new_start =
            !closest_to_start || dist_to_start < closest_to_start_dist;
        bool is_new_end =
            !closest_to_end || dist_to_end < closest_to_end_dist;
            
        if(is_new_start || is_new_end) {
            //We actually want this stop. Check now if it can be used.
            //We're not checking this earlier due to performance.
            if(!can_take_path_stop(s_ptr, settings)) {
                //Can't be taken. Skip.
                continue;
            }
        } else {
            //Not the closest so far. Skip.
            continue;
        }
        
        if(is_new_start) {
            closest_to_start_dist = dist_to_start;
            closest_to_start = s_ptr;
        }
        if(is_new_end) {
            closest_to_end_dist = dist_to_end;
            closest_to_end = s_ptr;
        }
    }
    
    if(start_stop) *start_stop = closest_to_start;
    if(end_stop) *end_stop = closest_to_end;
    
    //Let's just check something real quick:
    //if the destination is closer than any stop,
    //just go there right away!
    dist start_to_end_dist(start_to_use, end_to_use);
    if(start_to_end_dist <= closest_to_start_dist) {
        if(total_dist) {
            *total_dist = start_to_end_dist.to_float();
        }
        return PATH_RESULT_DIRECT;
    }
    
    //If the start and destination share the same closest spot,
    //that means this is the only stop in the path.
    if(closest_to_start == closest_to_end) {
        full_path.push_back(closest_to_start);
        if(total_dist) {
            *total_dist = closest_to_start_dist;
            *total_dist += closest_to_end_dist;
        }
        return PATH_RESULT_PATH_WITH_SINGLE_STOP;
    }
    
    //Potential optimization: instead of calculating with this graph, consult
    //a different one where nodes that only have two links are removed.
    //e.g. A -> B -> C becomes A -> C.
    //This means traversing fewer nodes when figuring out the shortest path.
    
    //Calculate the path.
    PATH_RESULTS result =
        dijkstra(
            full_path,
            closest_to_start, closest_to_end,
            settings, total_dist
        );
        
    if(total_dist && !full_path.empty()) {
        *total_dist +=
            dist(start_to_use, full_path[0]->pos).to_float();
        *total_dist +=
            dist(full_path[full_path.size() - 1]->pos, end_to_use).to_float();
    }
    
    return result;
}


/**
 * @brief Returns a string representation of a path block reason.
 *
 * @param reason Reason to convert.
 * @return The string.
 */
string path_block_reason_to_string(PATH_BLOCK_REASONS reason) {
    switch(reason) {
    case PATH_BLOCK_REASON_NONE: {
        return "None";
        break;
    } case PATH_BLOCK_REASON_NO_PATH: {
        return "Invalid path";
        break;
    } case PATH_BLOCK_REASON_OBSTACLE: {
        return "Obstacle mob in the way";
        break;
    } case PATH_BLOCK_REASON_NOT_IN_SCRIPT: {
        return "Mob path should be from script";
        break;
    } case PATH_BLOCK_REASON_NOT_LIGHT_LOAD: {
        return "Mob should be light load";
        break;
    } case PATH_BLOCK_REASON_NOT_AIRBORNE: {
        return "Mob should be airborne";
        break;
    } case PATH_BLOCK_REASON_UP_LEDGE: {
        return "Mob cannot go up ledge";
        break;
    } case PATH_BLOCK_REASON_NOT_RIGHT_LABEL: {
        return "Mob's following links with a different label";
        break;
    } case PATH_BLOCK_REASON_STOP_IN_VOID: {
        return "Next path stop is in the void";
        break;
    } case PATH_BLOCK_REASON_HAZARDOUS_STOP: {
        return "Next stop is in hazardous sector";
        break;
    }
    }
    return "";
}



/**
 * @brief Returns a string representation of a path result.
 *
 * @param result Result to convert.
 * @return The string.
 */
string path_result_to_string(PATH_RESULTS result) {
    switch(result) {
    case PATH_RESULT_NORMAL_PATH: {
        return "Normal open path";
        break;
    } case PATH_RESULT_PATH_WITH_OBSTACLES: {
        return "Path exists, but with obstacles";
        break;
    } case PATH_RESULT_PATH_WITH_SINGLE_STOP: {
        return "Only a single stop is visited";
        break;
    } case PATH_RESULT_DIRECT: {
        return "Go directly";
        break;
    } case PATH_RESULT_DIRECT_NO_STOPS: {
        return "No stops, so go directly";
        break;
    } case PATH_RESULT_END_STOP_UNREACHABLE: {
        return "Final stop cannot be reached from first stop";
        break;
    } case PATH_RESULT_NO_DESTINATION: {
        return "Destination was never set";
        break;
    } case PATH_RESULT_ERROR: {
        return "Could not calculate a path";
        break;
    } case PATH_RESULT_NOT_CALCULATED: {
        return "Not calculated yet";
        break;
    }
    }
    return "";
}
