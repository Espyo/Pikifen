/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general area editor-related functions.
 */

#ifndef AREA_EDITOR_INCLUDED
#define AREA_EDITOR_INCLUDED

#include <deque>

#include "../editor.h"

#include "../../imgui/imgui_impl_allegro5.h"


using std::deque;
using std::size_t;
using std::string;


/* ----------------------------------------------------------------------------
 * Information about the area editor.
 */
class area_editor : public editor {
public:
    //Load this area when the area editor loads.
    string auto_load_area;
    //Area being edited when using the quick-play button.
    string quick_play_area;
    //Position the camera was it in the editor before quick-play.
    point quick_play_cam_pos;
    //Editor camera zoom before quick-play.
    float quick_play_cam_z;
    
    //Standard functions.
    void do_logic();
    void do_drawing();
    void draw_canvas();
    void load();
    void unload();
    string get_name() const;
    
    area_editor();
    
    //Ways for the cursor to snap.
    enum SNAP_MODES {
        //Snap to grid.
        SNAP_GRID,
        //Snap to vertexes.
        SNAP_VERTEXES,
        //Snap to edges.
        SNAP_EDGES,
        //Snap to nothing.
        SNAP_NOTHING,
        
        //Total amount of snap modes.
        N_SNAP_MODES,
    };
    
    //Ways for the area to be viewed in-editor.
    enum VIEW_MODES {
        //Textures.
        VIEW_MODE_TEXTURES,
        //Wireframe.
        VIEW_MODE_WIREFRAME,
        //Heightmap.
        VIEW_MODE_HEIGHTMAP,
        //Brightness map.
        VIEW_MODE_BRIGHTNESS,
        
        //Total amount of view modes.
        N_VIEW_MODES,
    };
    
private:

    struct texture_suggestion {
        //Bitmap of the texture.
        ALLEGRO_BITMAP* bmp;
        //File name of the texture.
        string name;
        
        texture_suggestion(const string &n);
        void destroy();
    };
    
    //Represents the selected sectors, mobs, etc.
    struct selection_representation {
        //How many sectors, mobs, etc. are currently selected.
        size_t nr_selections;
        //Has the user confirmed that all of the selected sectors, mobs, etc.
        //should use the same properties?
        bool homogenous;
        //Sector, mob, etc. that represents the homogenous selection.
        void* representative;
    };
    
    //Represents a point in the current layout drawing.
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
        layout_drawing_node(area_editor* ae_ptr, const point &mouse_click);
        layout_drawing_node();
    };
    
    
    //Possible errors after a line drawing operation.
    enum DRAWING_LINE_ERRORS {
        //No error.
        DRAWING_LINE_NO_ERROR,
        //Made a loop while trying to split a sector.
        DRAWING_LINE_LOOPS_IN_SPLIT,
        //Hit an existing edge or vertex when drawing a new sector.
        DRAWING_LINE_HIT_EDGE_OR_VERTEX,
        //Trying to draw along an existing edge.
        DRAWING_LINE_ALONG_EDGE,
        //Crosses exsting edges.
        DRAWING_LINE_CROSSES_EDGES,
        //Crosses previous parts of the drawing.
        DRAWING_LINE_CROSSES_DRAWING,
    };
    
    //Types of problems in the area.
    enum EDITOR_PROBLEM_TYPES {
        //None found so far.
        EPT_NONE_YET,
        //No problems.
        EPT_NONE,
        //Two edges intersect.
        EPT_INTERSECTING_EDGES,
        //An edge is all by itself.
        EPT_LONE_EDGE,
        //Two vertexes in the same spot.
        EPT_OVERLAPPING_VERTEXES,
        //A sector is corrupted.
        EPT_BAD_SECTOR,
        //No leader mob found.
        EPT_MISSING_LEADER,
        //A texture is not found in the game files.
        EPT_UNKNOWN_TEXTURE,
        //Mob with no type.
        EPT_TYPELESS_MOB,
        //Mob out of bounds.
        EPT_MOB_OOB,
        //Mob stuck in a wall.
        EPT_MOB_IN_WALL,
        //Bridge mob missing a bridge sector.
        EPT_SECTORLESS_BRIDGE,
        //A path stop is all by itself.
        EPT_LONE_PATH_STOP,
        //A path stop is out of bounds.
        EPT_PATH_STOP_OOB,
        //Two path stops are in the same place.
        EPT_PATH_STOPS_TOGETHER,
        //A path stop is on top of an unrelated link.
        EPT_PATH_STOP_ON_LINK,
        //Bridge blocks the path from pile to it.
        EPT_PILE_BRIDGE_PATH,
        //Unknown tree shadow image.
        EPT_UNKNOWN_SHADOW,
    };
    
    //Editor states.
    enum EDITOR_STATES {
        //Main menu.
        EDITOR_STATE_MAIN,
        //Area info editing.
        EDITOR_STATE_INFO,
        //Layout editing.
        EDITOR_STATE_LAYOUT,
        //Mob editing.
        EDITOR_STATE_MOBS,
        //Path editing.
        EDITOR_STATE_PATHS,
        //Detail editing.
        EDITOR_STATE_DETAILS,
        //Review.
        EDITOR_STATE_REVIEW,
        //Tools.
        EDITOR_STATE_TOOLS,
    };
    
    //Editor sub-states.
    enum EDITOR_SUB_STATES {
        //None.
        EDITOR_SUB_STATE_NONE,
        //Drawing a sector.
        EDITOR_SUB_STATE_DRAWING,
        //Drawing a circular sector.
        EDITOR_SUB_STATE_CIRCLE_SECTOR,
        //On-canvas texture effect editing.
        EDITOR_SUB_STATE_OCTEE,
        //Adding a new mob.
        EDITOR_SUB_STATE_NEW_MOB,
        //Duplicating a mob.
        EDITOR_SUB_STATE_DUPLICATE_MOB,
        //Adding a mob link.
        EDITOR_SUB_STATE_ADD_MOB_LINK,
        //Deleting a mob link.
        EDITOR_SUB_STATE_DEL_MOB_LINK,
        //Drawing paths.
        EDITOR_SUB_STATE_PATH_DRAWING,
        //Adding a new tree shadow.
        EDITOR_SUB_STATE_NEW_SHADOW,
        //In texture-view mode.
        EDITOR_SUB_STATE_TEXTURE_VIEW,
    };
    
    //On-canvas texture effect editing modes.
    enum OCTEE_MODES {
        //Editing texture offset.
        OCTEE_MODE_OFFSET,
        //Editing texture scale.
        OCTEE_MODE_SCALE,
        //Editing texture angle.
        OCTEE_MODE_ANGLE,
    };
    
    //Filters for selecting.
    enum SELECTION_FILTERS {
        //Select sectors, edges, and vertexes.
        SELECTION_FILTER_SECTORS,
        //Select edges and vertexes.
        SELECTION_FILTER_EDGES,
        //Select vertexes only.
        SELECTION_FILTER_VERTEXES,
        
        //Total amount of selection filters.
        N_SELECTION_FILTERS,
    };
    
    //Layout editing panel modes.
    enum LAYOUT_MODES {
        //Sector info.
        LAYOUT_MODE_SECTORS,
        //Edge info.
        LAYOUT_MODE_EDGES,
    };
    
    static const float         COMFY_DIST;
    static const float         CROSS_SECTION_POINT_RADIUS;
    static const float         CURSOR_SNAP_DISTANCE;
    static const float         CURSOR_SNAP_UPDATE_INTERVAL;
    static const float         DEBUG_TEXT_SCALE;
    static const unsigned char DEF_REFERENCE_ALPHA;
    static const float         KEYBOARD_PAN_AMOUNT;
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
    static const float         REFERENCE_MIN_SIZE;
    static const float         QUICK_PREVIEW_DURATION;
    static const unsigned char SELECTION_COLOR[3];
    static const float         SELECTION_EFFECT_SPEED;
    static const float         SELECTION_TW_PADDING;
    static const float         UNDO_SAVE_LOCK_DURATION;
    static const float         VERTEX_MERGE_RADIUS;
    static const float         ZOOM_MAX_LEVEL_EDITOR;
    static const float         ZOOM_MIN_LEVEL_EDITOR;
    
    
    //Time left until a backup is generated.
    timer backup_timer;
    //Does the area exist on disk, or RAM only?
    bool area_exists_on_disk;
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
    //Name of the folder of the currently loaded area.
    string area_folder_name;
    //When showing a hazard in the list, this is the index of the current one.
    size_t cur_hazard_nr;
    //The current transformation widget.
    transformation_widget cur_transformation_widget;
    //Last known cursor snap position for heavy snap modes.
    point cursor_snap_cache;
    //Time left to update the cursor snap position for heavy snap modes.
    timer cursor_snap_timer;
    //Debug tool -- show the edge numbers?
    bool debug_edge_nrs;
    //Debug tool -- show the sector numbers?
    bool debug_sector_nrs;
    //Debug tool -- show the path numbers?
    bool debug_path_nrs;
    //Debug tool -- show the triangulation?
    bool debug_triangulation;
    //Debug tool -- show the vertex numbers?
    bool debug_vertex_nrs;
    //Nodes of the drawing.
    vector<layout_drawing_node> drawing_nodes;
    //Reason why the current drawing line is invalid.
    DRAWING_LINE_ERRORS drawing_line_error;
    //Category of the last mob placed.
    mob_category* last_mob_category;
    //Mob type of the last mob placed.
    mob_type* last_mob_type;
    //Are we editing sectors or edges?
    LAYOUT_MODES layout_mode;
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
    //Mouse drag start coordinates, when using on-canvas texture effect editing.
    point octee_drag_start;
    //Texture's original angle, when using on-canvas texture effect editing.
    float octee_orig_angle;
    //Texture's original offset, when using on-canvas texture effect editing.
    point octee_orig_offset;
    //Texture's original scale, when using on-canvas texture effect editing.
    point octee_orig_scale;
    //Current on-canvas texture effect edit mode.
    OCTEE_MODES octee_mode;
    //When drawing a path, create normal links. False for one-way links.
    bool path_drawing_normals;
    //First stop of the next link when drawing a path.
    path_stop* path_drawing_stop_1;
    //Path stops that make up the current path preview.
    vector<path_stop*> path_preview;
    //Location of the two path preview checkpoints.
    point path_preview_checkpoints[2];
    //The closest stop to the path preview start and end.
    path_stop* path_preview_closest[2];
    //Total distance of the previewed path.
    float path_preview_dist;
    //Is the path preview going straight?
    bool path_preview_straight;
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
    //Description of the current problem found.
    string problem_description;
    //Information about the problematic intersecting edges, if any.
    edge_intersection problem_edge_intersection;
    //Pointer to the problematic mob, if any.
    mob_gen* problem_mob_ptr;
    //Pointer to the problematic path stop, if any.
    path_stop* problem_path_stop_ptr;
    //Type of the current problem found in the review panel.
    EDITOR_PROBLEM_TYPES problem_type;
    //Pointer to the problematic sector, if any.
    sector* problem_sector_ptr;
    //Pointer to the problematic tree shadow, if any.
    tree_shadow* problem_shadow_ptr;
    //Title of the current problem found.
    string problem_title;
    //Pointer to the problematic vertex, if any.
    vertex* problem_vertex_ptr;
    //Time left in the quick preview mode, including fade out.
    timer quick_preview_timer;
    //Opacity of the reference image.
    unsigned char reference_alpha;
    //Reference image center.
    point reference_center;
    //Reference image dimensions.
    point reference_size;
    //Bitmap of the reference image.
    ALLEGRO_BITMAP* reference_bitmap;
    //File name of the reference image.
    string reference_file_name;
    //Keep the aspect ratio when resizing the reference?
    bool reference_keep_aspect_ratio;
    //Currently selected edges.
    set<edge*> selected_edges;
    //Currently selected mobs.
    set<mob_gen*> selected_mobs;
    //Currently selected path links.
    set<path_link*> selected_path_links;
    //Currently selected path stops.
    set<path_stop*> selected_path_stops;
    //Currently selected sectors.
    set<sector*> selected_sectors;
    //Currently selected tree shadow.
    tree_shadow* selected_shadow;
    //Keep the aspect ratio of the currently selected shadow?
    bool selected_shadow_keep_aspect_ratio;
    //Currently selected vertexes.
    set<vertex*> selected_vertexes;
    //Is the user currently performing a rectangle box?
    bool selecting;
    //Angle of the selection.
    float selection_angle;
    //Center of the selection.
    point selection_center;
    //The selection's alpha depends on this value.
    float selection_effect;
    //Point where the selection is currently at.
    point selection_end;
    //Current selection filter.
    SELECTION_FILTERS selection_filter;
    //Has the user agreed to homogenize the selection?
    bool selection_homogenized;
    //Angle of the selection, before it got transformed.
    float selection_orig_angle;
    //Center of the selection, before it got transformed.
    point selection_orig_center;
    //Size of the selection, before it got transformed.
    point selection_orig_size;
    //Size of the selection, padding included.
    point selection_size;
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
    //List of texture suggestions.
    vector<texture_suggestion> texture_suggestions;
    //Undo history, with the state of the area at each point.
    std::deque<std::pair<area_data*, string> > undo_history;
    //Name of the undo operation responsible for the lock.
    string undo_save_lock_operation;
    //During this timer, don't save state for operations matching the last one.
    timer undo_save_lock_timer;
    //Position of the load widget.
    point load_widget_pos;
    //Position of the reload widget.
    point reload_widget_pos;
    //Position of the quit widget.
    point quit_widget_pos;
    
    
    //General functions.
    bool are_nodes_traversable(
        const layout_drawing_node &n1,
        const layout_drawing_node &n2
    ) const;
    void cancel_circle_sector();
    void cancel_layout_drawing();
    void cancel_layout_moving();
    float calculate_preview_path();
    void change_state(const EDITOR_STATES new_state);
    void check_drawing_line(const point &pos);
    void clear_circle_sector();
    void clear_current_area();
    void clear_layout_drawing();
    void clear_layout_moving();
    void clear_problems();
    void clear_selection();
    void clear_texture_suggestions();
    void clear_undo_history();
    void close_area_picker();
    void close_options_dialog();
    void create_area();
    void create_drawing_vertexes();
    void create_mob_under_cursor();
    void create_new_from_picker(const size_t picker_id, const string &name);
    sector* create_sector_for_layout_drawing(sector* copy_from);
    void delete_current_area();
    void delete_edge(edge* e_ptr);
    bool delete_edges(const set<edge*> &which);
    void delete_mobs(const set<mob_gen*> &which);
    void delete_path_links(const set<path_link*> &which);
    void delete_path_stops(const set<path_stop*> &which);
    static void draw_canvas_imgui_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void draw_cross_section_sector(
        const float start_ratio, const float end_ratio, const float proportion,
        const float lowest_z, sector* sector_ptr
    );
    void draw_debug_text(
        const ALLEGRO_COLOR color, const point &where, const string &text,
        const unsigned char dots = 0
    );
    void draw_line_dist(const point &focus, const point &other);
    void emit_triangulation_error_status_bar_message(
        const TRIANGULATION_ERRORS error
    );
    void find_problems();
    void finish_circle_sector();
    void finish_layout_moving();
    void finish_new_sector_drawing();
    void forget_prepared_state(area_data* prepared_change);
    void get_affected_sectors(
        sector* s_ptr, unordered_set<sector*> &list
    ) const;
    void get_affected_sectors(
        set<sector*> &sectors, unordered_set<sector*> &list
    ) const;
    void get_affected_sectors(
        set<vertex*> &vertexes, unordered_set<sector*> &list
    ) const;
    void get_clicked_layout_element(
        vertex** clicked_vertex, edge** clicked_edge, sector** clicked_sector
    ) const;
    edge* get_closest_edge_to_angle(
        vertex* v_ptr, const float angle, const bool clockwise,
        float* closest_edge_angle
    ) const;
    bool get_common_sector(
        vector<vertex*> &vertexes, vector<edge*> &edges, sector** result
    ) const;
    edge* get_correct_post_split_edge(
        vertex* v_ptr, edge* e1_ptr, edge* e2_ptr
    ) const;
    bool get_drawing_outer_sector(sector** result) const;
    edge* get_edge_under_point(const point &p, edge* after = NULL) const;
    vector<edge_intersection> get_intersecting_edges() const;
    float get_mob_gen_radius(mob_gen* m) const;
    bool get_mob_link_under_point(
        const point &p,
        std::pair<mob_gen*, mob_gen*>* data1,
        std::pair<mob_gen*, mob_gen*>* data2
    ) const;
    mob_gen* get_mob_under_point(const point &p) const;
    bool get_path_link_under_point(
        const point &p, path_link** link1, path_link** link2
    ) const;
    path_stop* get_path_stop_under_point(const point &p) const;
    sector* get_sector_under_point(const point &p) const;
    vertex* get_vertex_under_point(const point &p) const;
    void goto_problem();
    void handle_line_error();
    void homogenize_selected_edges();
    void homogenize_selected_mobs();
    void homogenize_selected_path_links();
    void homogenize_selected_sectors();
    void load_area(const bool from_backup);
    void load_backup();
    void load_reference();
    bool merge_sectors(sector* s1, sector* s2);
    void merge_vertex(
        vertex* v1, vertex* v2, unordered_set<sector*>* affected_sectors
    );
    void open_area_picker();
    void open_options_dialog();
    void populate_texture_suggestions();
    void pick_area(
        const string &name, const string &category, const bool is_new
    );
    void pick_texture(
        const string &name, const string &category, const bool is_new
    );
    area_data* prepare_state();
    void press_circle_sector_button();
    void press_delete_area_button();
    void press_duplicate_mobs_button();
    void press_load_button();
    void press_grid_interval_decrease_button();
    void press_grid_interval_increase_button();
    void press_new_mob_button();
    void press_new_path_button();
    void press_new_sector_button();
    void press_new_tree_shadow_button();
    void press_reference_button();
    void press_reload_button();
    void press_remove_edge_button();
    void press_remove_mob_button();
    void press_remove_path_button();
    void press_remove_tree_shadow_button();
    void press_quick_play_button();
    void press_quit_button();
    void press_save_button();
    void press_selection_filter_button();
    void press_snap_mode_button();
    void press_undo_button();
    void process_gui();
    void process_gui_control_panel();
    void process_gui_delete_area_dialog();
    void process_gui_menu_bar();
    void process_gui_mob_script_vars(mob_gen* gen);
    void process_gui_panel_details();
    void process_gui_panel_edge();
    void process_gui_panel_info();
    void process_gui_panel_layout();
    void process_gui_panel_main();
    void process_gui_panel_mob();
    void process_gui_panel_mobs();
    void process_gui_panel_options();
    void process_gui_panel_path_link();
    void process_gui_panel_paths();
    void process_gui_panel_review();
    void process_gui_panel_sector();
    void process_gui_panel_tools();
    void process_gui_options_dialog();
    void process_gui_status_bar();
    void process_gui_toolbar();
    void register_change(
        const string &operation_name, area_data* pre_prepared_change = NULL
    );
    bool remove_isolated_sector(sector* s_ptr);
    void resize_everything(const float mults[2]);
    void rollback_to_prepared_state(area_data* prepared_state);
    void rotate_mob_gens_to_point(const point &pos);
    bool save_area(const bool to_backup);
    void save_backup();
    void save_reference();
    void select_different_hazard(const bool next);
    void select_edge(edge* e_ptr);
    void select_path_links_with_label(const string &label);
    void select_sector(sector* s_ptr);
    void select_tree_shadow(tree_shadow* s_ptr);
    void select_vertex(vertex* v_ptr);
    void set_selection_status_text();
    void set_new_circle_sector_points();
    point snap_point(const point &p, const bool ignore_selected = false);
    void split_sector_with_drawing();
    vertex* split_edge(edge* e_ptr, const point &where);
    path_stop* split_path_link(
        path_link* l1, path_link* l2,
        const point &where
    );
    void start_mob_move();
    void start_path_stop_move();
    void start_shadow_move();
    void start_vertex_move();
    void toggle_duplicate_mob_mode();
    void traverse_sector_for_split(
        sector* s_ptr, vertex* begin, vertex* checkpoint,
        vector<edge*>* edges, vector<vertex*>* vertexes,
        bool* working_sector_left
    );
    void undo();
    void undo_layout_drawing_node();
    void update_affected_sectors(
        const unordered_set<sector*> &affected_sectors
    );
    void update_inner_sectors_outer_sector(
        const vector<edge*> &edges_to_check,
        sector* old_outer, sector* new_outer
    );
    void update_reference();
    void update_sector_texture(sector* s_ptr, const string &file_name);
    void update_texture_suggestions(const string &n);
    void update_undo_history();
    void update_vertex_selection();
    
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
    void reset_cam_xy();
    void reset_cam_zoom();
    
};


#endif //ifndef AREA_EDITOR_INCLUDED
