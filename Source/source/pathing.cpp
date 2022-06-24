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

#include "game.h"
#include "mobs/mob_utils.h"

using std::map;
using std::string;
using std::unordered_set;
using std::vector;


/* ----------------------------------------------------------------------------
 * Creates an instance of a structure with settings about how to follow a path.
 */
path_follow_settings::path_follow_settings() :
    target_mob(nullptr),
    final_target_distance(MOB::DEF_CHASE_TARGET_DISTANCE),
    flags(0) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new path stop.
 * pos:
 *   Its coordinates.
 * links:
 *   List of path links, linking it to other stops.
 */
path_stop::path_stop(const point &pos, const vector<path_link*> &links) :
    pos(pos),
    links(links),
    sector_ptr(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a path stop.
 */
path_stop::~path_stop() {
    while(!links.empty()) {
        delete *(links.begin());
        links.erase(links.begin());
    }
}


/* ----------------------------------------------------------------------------
 * Adds a link between this stop and another, whether it's one-way or not.
 * Also adds the link to the other stop, if applicable.
 * If these two stops already had some link, it gets removed.
 * other_stop:
 *   Pointer to the other stop.
 * normal:
 *   Normal link? False means one-way link.
 */
void path_stop::add_link(path_stop* other_stop, const bool normal) {
    PATH_LINK_TYPES link_type = PATH_LINK_TYPE_NORMAL;
    string link_label;
    
    path_link* old_link_data = get_link(other_stop);
    if(!old_link_data) {
        old_link_data = other_stop->get_link(this);
    }
    if(old_link_data) {
        link_type = old_link_data->type;
        link_label = old_link_data->label;
    }
    
    remove_link(old_link_data);
    other_stop->remove_link(this);
    
    path_link* new_link = new path_link(this, other_stop, INVALID);
    new_link->type = link_type;
    new_link->label = link_label;
    links.push_back(new_link);
    
    if(normal) {
        new_link = new path_link(other_stop, this, INVALID);
        new_link->type = link_type;
        new_link->label = link_label;
        other_stop->links.push_back(new_link);
    }
}


/* ----------------------------------------------------------------------------
 * Calculates the distance between it and all neighbors.
 */
void path_stop::calculate_dists() {
    for(size_t l = 0; l < links.size(); ++l) {
        links[l]->calculate_dist(this);
    }
}


/* ----------------------------------------------------------------------------
 * Calculates the distance between it and all neighbors, and then goes through
 * the neighbors and updates their distance back to this stop, if that
 * neighbor links back.
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


/* ----------------------------------------------------------------------------
 * Returns the pointer of the link between this stop and another.
 * The links in memory are one-way, meaning that if the only link
 * is from the other stop to this one, it will not count.
 * Returns NULL if it does not link to that stop.
 * other_stop:
 *   Path stop to check against.
 */
path_link* path_stop::get_link(path_stop* other_stop) const {
    for(size_t l = 0; l < links.size(); ++l) {
        if(links[l]->end_ptr == other_stop) return links[l];
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Removes the specified link.
 * Does nothing if there is no such link.
 * link_ptr:
 *   Pointer to the link to remove.
 */
void path_stop::remove_link(path_link* link_ptr) {
    for(size_t l = 0; l < links.size(); ++l) {
        if(links[l] == link_ptr) {
            delete links[l];
            links.erase(links.begin() + l);
            return;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Removes the link between this stop and the specified one.
 * Does nothing if there is no such link.
 * other_stop:
 *   Path stop to remove the link from.
 */
void path_stop::remove_link(path_stop* other_stop) {
    for(size_t l = 0; l < links.size(); ++l) {
        if(links[l]->end_ptr == other_stop) {
            delete links[l];
            links.erase(links.begin() + l);
            return;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new stop link.
 * start_ptr:
 *   The path stop at the start of this link.
 * end_ptr:
 *   The path stop at the end of this link.
 * end_nr:
 *   Index number of the path stop at the end of this link.
 */
path_link::path_link(path_stop* start_ptr, path_stop* end_ptr, size_t end_nr) :
    start_ptr(start_ptr),
    end_ptr(end_ptr),
    end_nr(end_nr),
    type(PATH_LINK_TYPE_NORMAL),
    distance(0),
    blocked_by_obstacle(false) {
    
}


/* ----------------------------------------------------------------------------
 * Calculates and stores the distance between the two stops.
 * Because the link doesn't know about the starting stop,
 * you need to provide it as a parameter when calling the function.
 * start_ptr:
 *   The path stop at the start of this link.
 */
void path_link::calculate_dist(path_stop* start_ptr) {
    distance = dist(start_ptr->pos, end_ptr->pos).to_float();
}


/* ----------------------------------------------------------------------------
 * Clears all info.
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


/* ----------------------------------------------------------------------------
 * Handles the area having been loaded. It checks all path stops and saves
 * any sector hazards found.
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


/* ----------------------------------------------------------------------------
 * Handles an obstacle having been placed. This way, any link with that
 * obstruction can get updated.
 * m:
 *   Pointer to the obstacle mob that got added.
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
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
            mob* m_ptr = game.states.gameplay->mobs.all[m];
            if(!m_ptr->path_info) continue;
            
            m_ptr->fsm.run_event(MOB_EV_PATHS_CHANGED);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles an obstacle having been cleared. This way, any link with that
 * obstruction can get updated.
 * m:
 *   Pointer to the obstacle mob that got cleared.
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
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
            mob* m_ptr = game.states.gameplay->mobs.all[m];
            if(!m_ptr->path_info) continue;
            
            m_ptr->fsm.run_event(MOB_EV_PATHS_CHANGED);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles a sector having changed its hazards. This way, any stop on that
 * sector can be updated.
 * sector_ptr:
 *   Pointer to the sector whose hazards got updated.
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


/* ----------------------------------------------------------------------------
 * Checks if a link can be traversed given some contraints.
 * link_ptr:
 *   Link to check.
 * settings:
 *   Settings about how the path should be followed.
 */
bool can_traverse_path_link(
    path_link* link_ptr, const path_follow_settings &settings
) {
    //Check if there's an obstacle in the way.
    if(
        !(settings.flags & PATH_FOLLOW_FLAG_IGNORE_OBSTACLES) &&
        link_ptr->blocked_by_obstacle
    ) {
        return false;
    }
    
    //Check if the link has limitations based on link type.
    switch(link_ptr->type) {
    case PATH_LINK_TYPE_NORMAL: {
        break;
    } case PATH_LINK_TYPE_SCRIPT_ONLY: {
        if((settings.flags & PATH_FOLLOW_FLAG_SCRIPT_USE) == 0) {
            return false;
        }
        break;
    } case PATH_LINK_TYPE_LIGHT_LOAD_ONLY: {
        if((settings.flags & PATH_FOLLOW_FLAG_LIGHT_LOAD) == 0) {
            return false;
        }
        break;
    } case PATH_LINK_TYPE_AIRBORNE_ONLY: {
        if((settings.flags & PATH_FOLLOW_FLAG_AIRBORNE) == 0) {
            return false;
        }
        break;
    }
    }
    
    //Check if the travel is limited to links with a certain label.
    if(!settings.label.empty() && link_ptr->label != settings.label) {
        return false;
    }
    
    //Check if the link's end path stop is hazardous, by checking its sector.
    sector* end_sector = link_ptr->end_ptr->sector_ptr;
    if(!end_sector) {
        //We're probably in the area editor, where things change too often
        //for us to cache the sector pointer and access said cache.
        //Let's calculate now real quick.
        end_sector = get_sector(link_ptr->end_ptr->pos, NULL, false);
        if(!end_sector) {
            //It's really the void. Nothing that can be done here then.
            return false;
        }
    }
    
    bool touching_hazard =
        !end_sector->hazard_floor ||
        (settings.flags & PATH_FOLLOW_FLAG_AIRBORNE) == 0;
        
    if(
        !(settings.flags & PATH_FOLLOW_FLAG_IGNORE_OBSTACLES) &&
        link_ptr->end_ptr->sector_ptr &&
        touching_hazard &&
        !end_sector->hazards.empty()
    ) {
        for(size_t sh = 0; sh < end_sector->hazards.size(); ++sh) {
            bool invulnerable = false;
            for(size_t ih = 0; ih < settings.invulnerabilities.size(); ++ih) {
                if(
                    settings.invulnerabilities[ih] ==
                    end_sector->hazards[sh]
                ) {
                    invulnerable = true;
                    break;
                }
            }
            if(!invulnerable) {
                return false;
            }
        }
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Traverses a graph using the depth first search algorithm.
 * nodes:
 *   Vector of nodes.
 * visited:
 *   Set with the visited nodes.
 * start:
 *   Starting node.
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
    
    for(auto l : links) {
        if(visited.find(l) != visited.end()) continue;
        depth_first_search(nodes, visited, l);
    }
}


/* ----------------------------------------------------------------------------
 * Uses Dijkstra's algorithm to get the shortest path between two nodes.
 * start_node:
 *   Start node.
 * end_node:
 *   End node.
 * settings:
 *   Settings about how the path should be followed.
 * total_dist:
 *   If not NULL, place the total path distance here.
 */
vector<path_stop*> dijkstra(
    path_stop* start_node, path_stop* end_node,
    const path_follow_settings &settings,
    float* total_dist
) {
    //https://en.wikipedia.org/wiki/Dijkstra's_algorithm
    
    unordered_set<path_stop*> unvisited;
    //Distance from starting node + previous stop on the best solution.
    map<path_stop*, std::pair<float, path_stop*> > data;
    //If we found an error, set this to true.
    bool got_error = false;
    
    //Initialize the algorithm.
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        unvisited.insert(s_ptr);
        data[s_ptr] = std::make_pair(FLT_MAX, (path_stop*) NULL);
    }
    
    //The distance between the start node and the start node is 0.
    data[start_node].first = 0;
    
    while(!unvisited.empty() && !got_error) {
    
        //Figure out what node to work on.
        path_stop* shortest_node = NULL;
        float shortest_node_dist = 0;
        std::pair<float, path_stop*> shortest_node_data;
        
        for(auto u : unvisited) {
            std::pair<float, path_stop*> d = data[u];
            if(!shortest_node || d.first < shortest_node_dist) {
                shortest_node = u;
                shortest_node_dist = d.first;
                shortest_node_data = d;
            }
        }
        
        //If we reached the end node, that's it, best path found!
        if(shortest_node == end_node) {
        
            vector<path_stop*> final_path;
            path_stop* next = data[end_node].second;
            final_path.push_back(end_node);
            float td = data[end_node].first;
            //Construct the path.
            while(next) {
                final_path.insert(final_path.begin(), next);
                next = data[next].second;
            }
            
            if(final_path.size() < 2) {
                //This can't be right... Something went wrong.
                got_error = true;
                break;
            } else {
                if(total_dist) *total_dist = td;
                return final_path;
            }
            
        }
        
        //This node's been visited.
        unvisited.erase(unvisited.find(shortest_node));
        
        //Check the neighbors.
        for(size_t l = 0; l < shortest_node->links.size(); ++l) {
            path_link* l_ptr = shortest_node->links[l];
            
            //If this neighbor's been visited, forget it.
            if(unvisited.find(l_ptr->end_ptr) == unvisited.end()) continue;
            
            //Can this link be traversed?
            if(
                !can_traverse_path_link(l_ptr, settings)
            ) {
                continue;
            }
            
            float total_dist = shortest_node_data.first + l_ptr->distance;
            auto d = &data[l_ptr->end_ptr];
            
            if(total_dist < d->first) {
                //Found a shorter path to this node.
                d->first = total_dist;
                d->second = shortest_node;
            }
        }
    }
    
    //If we got to this point, there means that there is no available path!
    
    if(!(settings.flags & PATH_FOLLOW_FLAG_IGNORE_OBSTACLES)) {
        //Let's try again, this time ignoring obstacles.
        path_follow_settings new_settings = settings;
        new_settings.flags |= PATH_FOLLOW_FLAG_IGNORE_OBSTACLES;
        return
            dijkstra(
                start_node, end_node,
                new_settings,
                total_dist
            );
    } else {
        //Nothing that can be done. No path.
        if(total_dist) *total_dist = 0;
        return vector<path_stop*>();
    }
}


/* ----------------------------------------------------------------------------
 * Returns the shortest available path between two points, following
 * the area's path graph.
 * start:
 *   Start coordinates.
 * end:
 *   End coordinates.
 * settings:
 *   Settings about how the path should be followed.
 * go_straight:
 *   This is set according to whether it's better
 *   to go straight to the end point.
 * total_dist:
 *   If not NULL, place the total path distance here.
 * start_stop:
 *   If not NULL, the closest stop to the start is returned here.
 * end_stop:
 *   If not NULL, the closest stop to the end is returned here.
 */
vector<path_stop*> get_path(
    const point &start, const point &end,
    const path_follow_settings &settings,
    bool* go_straight, float* total_dist,
    path_stop** start_stop, path_stop** end_stop
) {

    vector<path_stop*> full_path;
    
    if(game.cur_area_data.path_stops.empty()) {
        if(go_straight) *go_straight = true;
        return full_path;
    } else {
        if(go_straight) *go_straight = false;
    }
    
    point start_to_use =
        (settings.flags & PATH_FOLLOW_FLAG_FAKED_START) ?
        settings.faked_start :
        start;
        
    point end_to_use =
        (settings.flags & PATH_FOLLOW_FLAG_FAKED_END) ?
        settings.faked_end :
        end;
        
    //Start by finding the closest stops to the start and finish.
    path_stop* closest_to_start = NULL;
    path_stop* closest_to_end = NULL;
    dist closest_to_start_dist;
    dist closest_to_end_dist;
    
    for(size_t s = 0; s < game.cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = game.cur_area_data.path_stops[s];
        
        dist dist_to_start(start_to_use, s_ptr->pos);
        dist dist_to_end(end_to_use, s_ptr->pos);
        
        if(!closest_to_start || dist_to_start < closest_to_start_dist) {
            closest_to_start_dist = dist_to_start;
            closest_to_start = s_ptr;
        }
        if(!closest_to_end || dist_to_end < closest_to_end_dist) {
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
        if(go_straight) *go_straight = true;
        if(total_dist) {
            *total_dist = start_to_end_dist.to_float();
        }
        return full_path;
    }
    
    //If the start and destination share the same closest spot,
    //that means this is the only stop in the path.
    if(closest_to_start == closest_to_end) {
        full_path.push_back(closest_to_start);
        if(total_dist) {
            *total_dist = closest_to_start_dist.to_float();
            *total_dist += closest_to_end_dist.to_float();
        }
        return full_path;
    }
    
    //Potential optimization: instead of calculating with this graph, consult
    //a different one where nodes that only have two links are removed.
    //e.g. A -> B -> C becomes A -> C.
    //This means traversing fewer nodes when figuring out the shortest path.
    
    //Calculate the path.
    full_path =
        dijkstra(
            closest_to_start, closest_to_end,
            settings, total_dist
        );
        
    if(total_dist && !full_path.empty()) {
        *total_dist +=
            dist(start_to_use, full_path[0]->pos).to_float();
        *total_dist +=
            dist(full_path[full_path.size() - 1]->pos, end_to_use).to_float();
    }
    
    return full_path;
}


/* ----------------------------------------------------------------------------
 * Returns what active obstacle stands in the way of these two stops, if any.
 * If multiple ones do, it returns the closest.
 * s1:
 *   First path stop to check.
 * s2:
 *   Second path stop to check.
 */
mob* get_path_link_obstacle(path_stop* s1, path_stop* s2) {
    mob* closest_obs = NULL;
    dist closest_obs_dist;
    
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
        mob* m_ptr = game.states.gameplay->mobs.all[m];
        if(!m_ptr->type->can_block_paths) continue;
        
        if(
            m_ptr->health != 0 &&
            circle_intersects_line_seg(
                m_ptr->pos,
                m_ptr->radius,
                s1->pos, s2->pos
            )
        ) {
            dist d(s1->pos, m_ptr->pos);
            if(!closest_obs || d < closest_obs_dist) {
                closest_obs = m_ptr;
                closest_obs_dist = d;
            }
        }
    }
    
    return closest_obs;
}
