/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general area editor-related functions.
 */

#ifndef AREA_EDITOR_IMGUI_INCLUDED
#define AREA_EDITOR_IMGUI_INCLUDED

#include <deque>

#include "../editor.h"
#include "../../imgui/imgui.h"

using namespace std;

class area_editor_imgui : public editor {
private:
    struct layout_drawing_node {
        //Raw coordinates of the mouse click.
        point raw_spot;
        //Final spot of the node, after snapping to an existing vertex/edge.
        point snapped_spot;
        //Is this node on top of an existing vertex? This points to it if so.
        vertex* on_vertex;
        //on_vertex's vertex number.
        size_t on_vertex_nr;
        //Is this node on top of an existing edge? This points to it if so.
        edge* on_edge;
        //on_edge's edge number.
        size_t on_edge_nr;
        //Is this node just on top of a sector? This points to it if so.
        sector* on_sector;
        //on_sector's sector number.
        size_t on_sector_nr;
        //Is on_vertex a new vertex, created during the sector creation?
        bool is_new_vertex;
        layout_drawing_node(area_editor_imgui* ae_ptr, const point &mouse_click);
        layout_drawing_node();
    };
    
    //This struct represents the selected sectors, mobs, etc.
    struct selection_representation {
        //How many sectors, mobs, etc. are currently selected.
        size_t nr_selections;
        //Has the user confirmed that all of the selected sectors, mobs, etc.
        //should use the same properties?
        bool homogenous;
        //Sector, mob, etc. that represents the homogenous selection.
        void* representative;
    };
    
    struct texture_suggestion {
        ALLEGRO_BITMAP* bmp;
        string name;
        texture_suggestion(const string &n);
        void destroy();
    };
    
    
    enum EDITOR_STATES {
        EDITOR_STATE_INFO,
        EDITOR_STATE_MAIN,
        EDITOR_STATE_LAYOUT,
        EDITOR_STATE_ASB,
        EDITOR_STATE_TEXTURE,
        EDITOR_STATE_ASA,
        EDITOR_STATE_MOBS,
        EDITOR_STATE_PATHS,
        EDITOR_STATE_DETAILS,
        EDITOR_STATE_REVIEW,
        EDITOR_STATE_TOOLS,
        EDITOR_STATE_STT,
        EDITOR_STATE_OPTIONS,
    };
    
    enum EDITOR_SUB_STATES {
        EDITOR_SUB_STATE_NONE,
        EDITOR_SUB_STATE_DRAWING,
        EDITOR_SUB_STATE_CIRCLE_SECTOR,
        EDITOR_SUB_STATE_NEW_MOB,
        EDITOR_SUB_STATE_DUPLICATE_MOB,
        EDITOR_SUB_STATE_ADD_MOB_LINK,
        EDITOR_SUB_STATE_DEL_MOB_LINK,
        EDITOR_SUB_STATE_PATH_DRAWING,
        EDITOR_SUB_STATE_NEW_SHADOW,
        EDITOR_SUB_STATE_TEXTURE_VIEW,
    };
    
    enum EDITOR_PROBLEM_TYPES {
        EPT_NONE_YET,
        EPT_NONE,
        EPT_INTERSECTING_EDGES,   //Two edges intersect.
        EPT_LONE_EDGE,            //An edge is all by itself.
        EPT_OVERLAPPING_VERTEXES, //Two vertexes in the same spot.
        EPT_BAD_SECTOR,           //A sector is corrupted.
        EPT_MISSING_LEADER,       //No leader mob found.
        EPT_UNKNOWN_TEXTURE,      //A texture is not found in the game files.
        EPT_TYPELESS_MOB,         //Mob with no type.
        EPT_MOB_OOB,              //Mob out of bounds.
        EPT_MOB_IN_WALL,          //Mob stuck in a wall.
        EPT_SECTORLESS_BRIDGE,    //Bridge mob missing a bridge sector.
        EPT_LONE_PATH_STOP,       //A path stop is all by itself.
        EPT_PATH_STOP_OOB,        //A path stop is out of bounds.
        EPT_PATH_STOPS_TOGETHER,  //Two path stops are in the same place.
        EPT_PATHS_UNCONNECTED,    //The path graph is unconnected.
        EPT_INVALID_SHADOW,       //Invalid tree shadow image.
    };
    
    enum DRAWING_LINE_ERRORS {
        DRAWING_LINE_NO_ERROR,
        DRAWING_LINE_WAYWARD_SECTOR,
        DRAWING_LINE_CROSSES_EDGES,
        DRAWING_LINE_CROSSES_DRAWING,
    };
    
    enum SELECTION_FILTERS {
        SELECTION_FILTER_SECTORS,
        SELECTION_FILTER_EDGES,
        SELECTION_FILTER_VERTEXES,
        N_SELECTION_FILTERS,
    };
    
    enum SNAP_MODES {
        SNAP_GRID,
        SNAP_VERTEXES,
        SNAP_EDGES,
        SNAP_NOTHING,
        N_SNAP_MODES,
    };
    
    enum VIEW_MODES {
        VIEW_MODE_TEXTURES,
        VIEW_MODE_WIREFRAME,
        VIEW_MODE_HEIGHTMAP,
        VIEW_MODE_BRIGHTNESS,
        N_VIEW_MODES,
    };
    
    static const float         CROSS_SECTION_POINT_RADIUS;
    static const float         CURSOR_SNAP_DISTANCE;
    static const float         CURSOR_SNAP_UPDATE_INTERVAL;
    static const float         DEBUG_TEXT_SCALE;
    static const unsigned char DEF_REFERENCE_ALPHA;
    static const unsigned char MAX_CIRCLE_SECTOR_POINTS;
    static const float         MAX_GRID_INTERVAL;
    static const size_t        MAX_TEXTURE_SUGGESTIONS;
    static const unsigned char MIN_CIRCLE_SECTOR_POINTS;
    static const float         MIN_GRID_INTERVAL;
    static const float         MOB_LINK_THICKNESS;
    static const float         NEW_SECTOR_ERROR_TINT_DURATION;
    static const float         PATH_LINK_THICKNESS;
    static const float         PATH_PREVIEW_CHECKPOINT_RADIUS;
    static const float         PATH_PREVIEW_TIMER_DUR;
    static const float         PATH_STOP_RADIUS;
    static const float         POINT_LETTER_TEXT_SCALE;
    static const unsigned char SELECTION_COLOR[3];
    static const float         SELECTION_EFFECT_SPEED;
    static const float         UNDO_SAVE_LOCK_DURATION;
    static const float         VERTEX_MERGE_RADIUS;
    static const float         ZOOM_MAX_LEVEL_EDITOR;
    static const float         ZOOM_MIN_LEVEL_EDITOR;
    
    //Time left until a backup is generated.
    timer backup_timer;
    //Where the cross-section tool points are.
    point cross_section_checkpoints[2];
    //Cross-section window's start coordinates.
    point cross_section_window_start;
    //Cross-section window's end coordinates.
    point cross_section_window_end;
    //Cross-section Z legend window's start coordinates.
    point cross_section_z_window_start;
    //Cross-section Z legend window's end coordinates.
    point cross_section_z_window_end;
    //Name of the area currently loaded.
    string cur_area_name;
    //When showing a hazard in the list, this is the index of the current one.
    size_t cur_hazard_nr;
    //Last known cursor snap position for heavy snap modes.
    point cursor_snap_cache;
    //Time left to update the cursor snap position for heavy snap modes.
    timer cursor_snap_timer;
    //Debug tool -- show the edge numbers?
    bool debug_edge_nrs;
    //Debug tool -- show the sector numbers?
    bool debug_sector_nrs;
    //Debug tool -- show the triangulation?
    bool debug_triangulation;
    //Debug tool -- show the vertex numbers?
    bool debug_vertex_nrs;
    //Nodes of the drawing.
    vector<layout_drawing_node> drawing_nodes;
    //Reason why the current drawing line is invalid. Use DRAWING_LINE_*.
    unsigned char drawing_line_error;
    //Category of the last mob placed.
    mob_category* last_mob_category;
    //Mob type of the last mob placed.
    mob_type* last_mob_type;
    //List of lone edges found.
    unordered_set<edge*> lone_edges;
    //Closest mob to the mouse when moving.
    mob_gen* move_closest_mob;
    //Closest mob was here when the move started (world coords).
    point move_closest_mob_start_pos;
    //Closest path stop to the mouse when moving.
    path_stop* move_closest_stop;
    //Closest path stop was here when the move started (world coords).
    point move_closest_stop_start_pos;
    //Closest vertex to the mouse when moving.
    vertex* move_closest_vertex;
    //Closest vertex was here when the move started (world coords).
    point move_closest_vertex_start_pos;
    //The mouse cursor was here when the move started (world coords).
    point move_mouse_start_pos;
    //Currently moving the selected vertexes, objects, etc.?
    bool moving;
    //Path preview checkpoint that is currently being moved, or -1 for none.
    signed char moving_path_preview_checkpoint;
    //Cross-section point that is currently being moved, or -1 for none.
    signed char moving_cross_section_point;
    //New circle sector's second point.
    point new_circle_sector_anchor;
    //New circle sector's center.
    point new_circle_sector_center;
    //Points where the new circle sector's vertexes will end up.
    vector<point> new_circle_sector_points;
    //What step of the circular sector building process are we in?
    unsigned char new_circle_sector_step;
    //For each edge of the new circle sector, is it valid?
    vector<bool> new_circle_sector_valid_edges;
    //Time left to keep the error-redness of the new sector's line(s) for.
    timer new_sector_error_tint_timer;
    //Non-simple sectors found, and their reason for being broken.
    map<sector*, TRIANGULATION_ERRORS> non_simples;
    //When drawing a path, create normal links. False for one-way links.
    bool path_drawing_normals;
    //First stop of the next link when drawing a path.
    path_stop* path_drawing_stop_1;
    //Path stops that make up the current path preview.
    vector<path_stop*> path_preview;
    //Location of the two path preview checkpoints.
    point path_preview_checkpoints[2];
    //Only calculate the preview path when this time is up.
    timer path_preview_timer;
    //Area data before vertex movement.
    area_data* pre_move_area_data;
    //Position of the selected mobs before movement.
    map<mob_gen*, point> pre_move_mob_coords;
    //Position of the selected tree shadow before movement.
    point pre_move_shadow_coords;
    //Position of the selected path stops before movement.
    map<path_stop*, point> pre_move_stop_coords;
    //Position of the selected vertexes before movement.
    map<vertex*, point> pre_move_vertex_coords;
    //Information about the problematic intersecting edges, if any.
    edge_intersection problem_edge_intersection;
    //Pointer to the problematic mob, if any.
    mob_gen* problem_mob_ptr;
    //Pointer to the problematic path stop, if any.
    path_stop* problem_path_stop_ptr;
    //Type of the current problem found in the review panel.
    unsigned char problem_type;
    //Pointer to the problematic sector, if any.
    sector* problem_sector_ptr;
    //Pointer to the problematic tree shadow, if any.
    tree_shadow* problem_shadow_ptr;
    //String with extra information about the current problem, if any.
    string problem_string;
    //Pointer to the problematic vertex, if any.
    vertex* problem_vertex_ptr;
    //Opacity of the reference image.
    unsigned char reference_alpha;
    //Bitmap of the reference image.
    ALLEGRO_BITMAP* reference_bitmap;
    //File name of the reference image.
    string reference_file_name;
    //Current transformations on the reference image.
    transformation_controller reference_transformation;
    //Currently selected edges.
    set<edge*> selected_edges;
    //Currently selected mobs.
    set<mob_gen*> selected_mobs;
    //Currently selected path links.
    set<pair<path_stop*, path_stop*> > selected_path_links;
    //Currently selected path stops.
    set<path_stop*> selected_path_stops;
    //Currently selected sectors.
    set<sector*> selected_sectors;
    //Currently selected tree shadow.
    tree_shadow* selected_shadow;
    //Transformation controller of the selected tree shadow.
    transformation_controller selected_shadow_transformation;
    //Currently selected vertexes.
    set<vertex*> selected_vertexes;
    //Is the user currently performing a rectangle box?
    bool selecting;
    //The selection's alpha depends on this value.
    float selection_effect;
    //Point where the selection is currently at.
    point selection_end;
    //Current selection filter.
    unsigned char selection_filter;
    //Has the user agreed to homogenize the selection?
    bool selection_homogenized;
    //Point where the selection started.
    point selection_start;
    //Show the path stop closest to the cursor?
    bool show_closest_stop;
    //Use the cross-section view tool?
    bool show_cross_section;
    //When using the cross-section view tool, render the grid?
    bool show_cross_section_grid;
    //Show the path preview and the checkpoints?
    bool show_path_preview;
    //Render the reference image?
    bool show_reference;
    //Render the tree shadows?
    bool show_shadows;
    //Current cursor snapping mode.
    unsigned char snap_mode;
    //Starting coordinates of a sector texture transformer drag.
    point stt_drag_start;
    //Original angle of the sector in the sector texture transformer.
    float stt_orig_angle;
    //Original offset of the sector in the sector texture transformer.
    point stt_orig_offset;
    //Original scale of the sector in the sector texture transformer.
    point stt_orig_scale;
    //Current mode for the sector texture transformer.
    unsigned char stt_mode;
    //Sector currently being edited in the sector texture transformer.
    sector* stt_sector;
    //List of texture suggestions.
    vector<texture_suggestion> texture_suggestions;
    //Undo history, with the state of the area at each point.
    deque<pair<area_data*, string> > undo_history;
    //Name of the undo operation responsible for the lock.
    string undo_save_lock_operation;
    //During this timer, don't save state for operations matching the last one.
    timer undo_save_lock_timer;
    
    bool are_nodes_traversable(
        const layout_drawing_node &n1,
        const layout_drawing_node &n2
    );
    void calculate_preview_path();
    void cancel_circle_sector();
    void cancel_layout_drawing();
    void cancel_layout_moving();
    void change_snap_mode(const size_t new_mode);
    void check_drawing_line(const point &pos);
    void clear_circle_sector();
    void clear_current_area();
    void clear_layout_drawing();
    void clear_layout_moving();
    void clear_problems();
    void clear_selection();
    void clear_texture_suggestions();
    void clear_undo_history();
    void create_area();
    void delete_selected_mobs();
    void delete_selected_path_elements();
    static void draw_canvas_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void draw_canvas();
    void draw_cross_section_sector(
        const float start_ratio, const float end_ratio, const float proportion,
        const float lowest_z, sector* sector_ptr
    );
    void draw_debug_text(
        const ALLEGRO_COLOR color, const point &where, const string &text,
        const unsigned char dots = 0
    );
    void draw_line_dist(const point &focus, const point &other);
    unsigned char find_problems();
    void finish_circle_sector();
    void finish_layout_drawing();
    void finish_layout_moving();
    void forget_prepared_state(area_data* prepared_change);
    unordered_set<sector*> get_affected_sectors(set<vertex*> &vertexes);
    void get_clicked_layout_element(
        vertex** clicked_vertex, edge** clicked_edge, sector** clicked_sector
    );
    edge* get_closest_edge_to_angle(
        vertex* v_ptr, const float angle, const bool clockwise,
        float* closest_edge_angle
    );
    bool get_common_sector(
        vector<vertex*> &vertexes, vector<edge*> &edges, sector** result
    );
    edge* get_correct_post_split_edge(
        vertex* v_ptr, edge* e1_ptr, edge* e2_ptr
    );
    bool get_drawing_outer_sector(sector** result);
    edge* get_edge_under_point(const point &p, edge* after = NULL);
    vector<edge_intersection> get_intersecting_edges();
    float get_mob_gen_radius(mob_gen* m);
    bool get_mob_link_under_point(
        const point &p,
        pair<mob_gen*, mob_gen*>* data1, pair<mob_gen*, mob_gen*>* data2
    );
    mob_gen* get_mob_under_point(const point &p);
    bool get_path_link_under_point(
        const point &p,
        pair<path_stop*, path_stop*>* data1, pair<path_stop*, path_stop*>* data2
    );
    path_stop* get_path_stop_under_point(const point &p);
    sector* get_sector_under_point(const point &p);
    vertex* get_vertex_under_point(const point &p);
    void goto_problem();
    void handle_line_error();
    void homogenize_selected_mobs();
    void homogenize_selected_sectors();
    void load_area(const bool from_backup);
    void load_backup();
    void load_reference();
    void merge_vertex(
        vertex* v1, vertex* v2, unordered_set<sector*>* affected_sectors
    );
    area_data* prepare_state();
    void process_gui();
    void register_change(
        const string &operation_name, area_data* pre_prepared_change = NULL
    );
    bool remove_isolated_sectors();
    void resize_everything(const float mult);
    bool save_area(const bool to_backup);
    void save_backup();
    void save_reference();
    void select_edge(edge* e);
    void select_sector(sector* s);
    void select_tree_shadow(tree_shadow* v);
    void select_vertex(vertex* v);
    void set_new_circle_sector_points();
    point snap_point(const point &p);
    vertex* split_edge(edge* e_ptr, const point &where);
    path_stop* split_path_link(
        const pair<path_stop*, path_stop*> &l1,
        const pair<path_stop*, path_stop*> &l2,
        const point &where
    );
    void start_mob_move();
    void start_path_stop_move();
    void start_shadow_move();
    void start_vertex_move();
    void undo();
    void undo_layout_drawing_node();
    bool update_backup_status();
    void update_reference(const string &new_file_name);
    void update_sector_texture(sector* s_ptr, const string &file_name);
    void update_texture_suggestions(const string &n);
    void update_undo_history();

    //Input handler functions.
    void handle_key_char_anywhere(const ALLEGRO_EVENT &ev);
    void handle_key_char_canvas(const ALLEGRO_EVENT &ev);
    void handle_key_down_anywhere(const ALLEGRO_EVENT &ev);
    void handle_key_down_canvas(const ALLEGRO_EVENT &ev);
    void handle_lmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_lmb_down(const ALLEGRO_EVENT &ev);
    void handle_lmb_drag(const ALLEGRO_EVENT &ev);
    void handle_lmb_up(const ALLEGRO_EVENT &ev);
    void handle_mmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_mmb_down(const ALLEGRO_EVENT &ev);
    void handle_mmb_drag(const ALLEGRO_EVENT &ev);
    void handle_mouse_update(const ALLEGRO_EVENT &ev);
    void handle_mouse_wheel(const ALLEGRO_EVENT &ev);
    void handle_rmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_rmb_down(const ALLEGRO_EVENT &ev);
    void handle_rmb_drag(const ALLEGRO_EVENT &ev);
    void pan_cam(const ALLEGRO_EVENT &ev);
    void reset_cam_xy(const ALLEGRO_EVENT &ev);
    void reset_cam_zoom(const ALLEGRO_EVENT &ev);
    
public:
    string auto_load_area;

    void do_logic();
    void do_drawing();
    void load();
    void unload();
    void handle_event(ALLEGRO_EVENT *ev);

    void hide_all_frames();
    void change_to_right_frame();
    void create_new_from_picker(
        const size_t picker_id, const string &name
    );
    void pick(
        const size_t picker_id, const string &name, const string &category
    );

    area_editor_imgui();
    ~area_editor_imgui();
};



#endif //ifndef AREA_EDITOR_IMGUI_INCLUDED
