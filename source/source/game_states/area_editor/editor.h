/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general area editor-related functions.
 */

#pragma once

#include <deque>

#include "../../area/area.h"
#include "../editor.h"

#include "../../libs/imgui/imgui_impl_allegro5.h"
#include "../../utils/general_utils.h"


using std::deque;
using std::size_t;
using std::string;


namespace AREA_EDITOR {
extern const ALLEGRO_COLOR BLOCKING_COLOR;
extern const float COMFY_DIST;
extern const float CROSS_SECTION_POINT_RADIUS;
extern const float CURSOR_SNAP_DISTANCE;
extern const float CURSOR_SNAP_UPDATE_INTERVAL;
extern const float DEBUG_TEXT_SCALE;
extern const unsigned char DEF_REFERENCE_ALPHA;
extern const float KEYBOARD_PAN_AMOUNT;
extern const unsigned char MAX_CIRCLE_SECTOR_POINTS;
extern const float MAX_GRID_INTERVAL;
extern const size_t MAX_TEXTURE_SUGGESTIONS;
extern const ALLEGRO_COLOR MEASUREMENT_COLOR;
extern const unsigned char MIN_CIRCLE_SECTOR_POINTS;
extern const float MIN_GRID_INTERVAL;
extern const float MOB_LINK_THICKNESS;
extern const float MOUSE_COORDS_TEXT_WIDTH;
extern const float NEW_SECTOR_ERROR_TINT_DURATION;
extern const ALLEGRO_COLOR NON_BLOCKING_COLOR;
extern const float PATH_LINK_THICKNESS;
extern const float PATH_PREVIEW_CHECKPOINT_RADIUS;
extern const float PATH_PREVIEW_TIMER_DUR;
extern const float POINT_LETTER_TEXT_SCALE;
extern const float REFERENCE_MIN_SIZE;
extern const float QUICK_PREVIEW_DURATION;
extern const unsigned char SELECTION_COLOR[3];
extern const float SELECTION_EFFECT_SPEED;
extern const float SELECTION_TW_PADDING;
extern const string SONG_NAME;
extern const float UNDO_SAVE_LOCK_DURATION;
extern const float VERTEX_MERGE_RADIUS;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the area editor.
 */
class area_editor : public editor {

public:

    //--- Misc. declarations ---
    
    //Ways for the cursor to snap.
    enum SNAP_MODE {
    
        //Snap to grid.
        SNAP_MODE_GRID,
        
        //Snap to vertexes.
        SNAP_MODE_VERTEXES,
        
        //Snap to edges.
        SNAP_MODE_EDGES,
        
        //Snap to nothing.
        SNAP_MODE_NOTHING,
        
        //Total amount of snap modes.
        N_SNAP_MODES,
        
    };
    
    //Ways for the area to be viewed in-editor.
    enum VIEW_MODE {
    
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
    
    
    //--- Members ---
    
    //Automatically load this folder upon boot-up of the editor, if any.
    string auto_load_folder;
    
    //Area being edited when using the quick-play button.
    string quick_play_area_path;
    
    //Position the camera was it in the editor before quick-play.
    point quick_play_cam_pos;
    
    //Editor camera zoom before quick-play.
    float quick_play_cam_z = 1.0f;
    
    //This hack fixes a glitch by skipping drawing for one frame.
    bool hack_skip_drawing = false;
    
    
    //--- Function declarations ---
    
    area_editor();
    void do_logic() override;
    void do_drawing() override;
    void load() override;
    void unload() override;
    string get_name() const override;
    void draw_canvas();
    string get_opened_content_path() const;
    string get_history_option_prefix() const override;
    
private:

    /**
     * @brief Represents a suggested texture.
     */
    struct texture_suggestion {
    
        //--- Members ---
        
        //Bitmap of the texture.
        ALLEGRO_BITMAP* bmp = nullptr;
        
        //Internal name of the texture.
        string name;
        
        
        //--- Function declarations ---
        
        explicit texture_suggestion(const string &n);
        void destroy();
        
    };
    
    /**
     * @brief Represents a point in the current layout drawing.
     */
    struct layout_drawing_node {
    
        //--- Members ---
        
        //Raw coordinates of the mouse click.
        point raw_spot;
        
        //Final spot of the node, after snapping to an existing vertex/edge.
        point snapped_spot;
        
        //Is this node on top of an existing vertex? This points to it if so.
        vertex* on_vertex = nullptr;
        
        //on_vertex's vertex index.
        size_t on_vertex_idx = INVALID;
        
        //Is this node on top of an existing edge? This points to it if so.
        edge* on_edge = nullptr;
        
        //on_edge's edge index.
        size_t on_edge_idx = INVALID;
        
        //Is this node just on top of a sector? This points to it if so.
        sector* on_sector = nullptr;
        
        //on_sector's sector index.
        size_t on_sector_idx = INVALID;
        
        //Is on_vertex a new vertex, created during the sector creation?
        bool is_new_vertex = false;
        
        
        //--- Function declarations ---
        
        layout_drawing_node(
            const area_editor* ae_ptr, const point &mouse_click
        );
        layout_drawing_node();
        
    };
    
    /**
     * @brief Info pertaining to a sector split operation.
     */
    struct sector_split_t {
    
        //--- Members ---
        
        //Area data from before the split.
        area_data* pre_split_area_data = nullptr;
        
        //Sector being worked on in a sector split operation.
        sector* working_sector = nullptr;
        
        //Edges of the sector split sector, before the split operation.
        vector<edge*> working_sector_old_edges;
        
        //Edges traversed in each step.
        vector<edge*> traversed_edges[2];
        
        //Vertexes traversed in each step.
        vector<vertex*> traversed_vertexes[2];
        
        //During stage 1, was the working sector to the left?
        bool is_working_at_stage_1_left = false;
        
        //Number of drawing nodes before a useless split part 2. Or INVALID.
        size_t useless_split_part_2_checkpoint = INVALID;
        
    };
    
    //Possible results after a line drawing operation.
    enum DRAWING_LINE_RESULT {
    
        //No error.
        DRAWING_LINE_RESULT_OK,
        
        //Hit an existing edge or vertex when drawing a new sector.
        DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX,
        
        //Trying to draw along an existing edge.
        DRAWING_LINE_RESULT_ALONG_EDGE,
        
        //Crosses exsting edges.
        DRAWING_LINE_RESULT_CROSSES_EDGES,
        
        //Crosses previous parts of the drawing.
        DRAWING_LINE_RESULT_CROSSES_DRAWING,
        
        //Goes towards a sector different from the working sector.
        DRAWING_LINE_RESULT_WAYWARD_SECTOR,
        
    };
    
    //Possible errors for a sector split operation.
    enum SECTOR_SPLIT_RESULT {
    
        //Ok to split.
        SECTOR_SPLIT_RESULT_OK,
        
        //The split is invalid.
        SECTOR_SPLIT_RESULT_INVALID,
        
        //That wouldn't split in any useful way. e.g. Slice through a donut.
        SECTOR_SPLIT_RESULT_USELESS,
        
    };
    
    //Types of problems in the area (Editor Problem Type).
    enum EPT {
    
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
        
        //A texture is not found in the game content.
        EPT_UNKNOWN_TEXTURE,
        
        //Mob with no type.
        EPT_TYPELESS_MOB,
        
        //Mob out of bounds.
        EPT_MOB_OOB,
        
        //Mob stuck in a wall.
        EPT_MOB_IN_WALL,
        
        //Mob links to itself.
        EPT_MOB_LINKS_TO_SELF,
        
        //Mobs stored in a loop.
        EPT_MOB_STORED_IN_LOOP,
        
        //Pikmin amount goes over the limit.
        EPT_PIKMIN_OVER_LIMIT,
        
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
        
        //No active score criteria for this mission.
        EPT_NO_SCORE_CRITERIA,
        
        //No mission goal mobs.
        EPT_NO_GOAL_MOBS,
        
    };
    
    //Editor states.
    enum EDITOR_STATE {
    
        //Main menu.
        EDITOR_STATE_MAIN,
        
        //Area info editing.
        EDITOR_STATE_INFO,
        
        //Area gameplay settings editing.
        EDITOR_STATE_GAMEPLAY,
        
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
    enum EDITOR_SUB_STATE {
    
        //None.
        EDITOR_SUB_STATE_NONE,
        
        //Picking a mission exit region.
        EDITOR_SUB_STATE_MISSION_EXIT,
        
        //Drawing the layout.
        EDITOR_SUB_STATE_DRAWING,
        
        //Drawing a circular sector.
        EDITOR_SUB_STATE_CIRCLE_SECTOR,
        
        //On-canvas texture effect editing.
        EDITOR_SUB_STATE_OCTEE,
        
        //Quick sector height set.
        EDITOR_SUB_STATE_QUICK_HEIGHT_SET,
        
        //Adding a new mob.
        EDITOR_SUB_STATE_NEW_MOB,
        
        //Duplicating a mob.
        EDITOR_SUB_STATE_DUPLICATE_MOB,
        
        //Storing a mob inside another.
        EDITOR_SUB_STATE_STORE_MOB_INSIDE,
        
        //Adding a mob link.
        EDITOR_SUB_STATE_ADD_MOB_LINK,
        
        //Deleting a mob link.
        EDITOR_SUB_STATE_DEL_MOB_LINK,
        
        //Picking required mobs for the mission.
        EDITOR_SUB_STATE_MISSION_MOBS,
        
        //Drawing paths.
        EDITOR_SUB_STATE_PATH_DRAWING,
        
        //Adding a new tree shadow.
        EDITOR_SUB_STATE_NEW_SHADOW,
        
    };
    
    //On-canvas texture effect editing modes.
    enum OCTEE_MODE {
    
        //Editing texture offset.
        OCTEE_MODE_OFFSET,
        
        //Editing texture scale.
        OCTEE_MODE_SCALE,
        
        //Editing texture angle.
        OCTEE_MODE_ANGLE,
        
    };
    
    //Filters for selecting.
    enum SELECTION_FILTER {
    
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
    enum LAYOUT_MODE {
    
        //Sector info.
        LAYOUT_MODE_SECTORS,
        
        //Edge info.
        LAYOUT_MODE_EDGES,
        
    };
    
    
    //--- Members ---
    
    //Time left until a backup is generated.
    timer backup_timer;
    
    //When the player copies an edge's properties, they go here.
    edge* copy_buffer_edge = nullptr;
    
    //When the player copies a mob's properties, they go here.
    mob_gen* copy_buffer_mob = nullptr;
    
    //When the player copies a path link's properties, they go here.
    path_link* copy_buffer_path_link = nullptr;
    
    //When the player copies a sector's properties, they go here.
    sector* copy_buffer_sector = nullptr;
    
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
    
    //When showing a hazard in the list, this is the index of the current one.
    size_t cur_hazard_idx = INVALID;
    
    //The current transformation widget.
    transformation_widget cur_transformation_widget;
    
    //Last known cursor snap position for heavy snap modes.
    point cursor_snap_cache;
    
    //Time left to update the cursor snap position for heavy snap modes.
    timer cursor_snap_timer = timer(AREA_EDITOR::CURSOR_SNAP_UPDATE_INTERVAL);
    
    //Debug tool -- show the edge indexes?
    bool debug_edge_idxs = false;
    
    //Debug tool -- show the sector indexes?
    bool debug_sector_idxs = false;
    
    //Debug tool -- show the path indexes?
    bool debug_path_idxs = false;
    
    //Debug tool -- show the triangulation?
    bool debug_triangulation = false;
    
    //Debug tool -- show the vertex indexes?
    bool debug_vertex_idxs = false;
    
    //Nodes of the drawing.
    vector<layout_drawing_node> drawing_nodes;
    
    //Result of the current drawing line.
    DRAWING_LINE_RESULT drawing_line_result = DRAWING_LINE_RESULT_OK;
    
    //Currently highlighted edge, if any.
    edge* highlighted_edge = nullptr;
    
    //Currently highlighted mob, if any.
    mob_gen* highlighted_mob = nullptr;
    
    //Currently highlighted path link, if any.
    path_link* highlighted_path_link = nullptr;
    
    //Currently highlighted path stop, if any.
    path_stop* highlighted_path_stop = nullptr;
    
    //Currently highlighted sector, if any.
    sector* highlighted_sector = nullptr;
    
    //Currently highlighted vertex, if any.
    vertex* highlighted_vertex = nullptr;
    
    //Category name of the last mob placed.
    string last_mob_custom_cat_name;
    
    //Mob type of the last mob placed.
    mob_type* last_mob_type = nullptr;
    
    //Are we editing sectors or edges?
    LAYOUT_MODE layout_mode = LAYOUT_MODE_SECTORS;
    
    //Picker info for the picker in the "load" dialog.
    picker_info load_dialog_picker;
    
    //Closest mob to the mouse when moving.
    mob_gen* move_closest_mob = nullptr;
    
    //Closest path stop to the mouse when moving.
    path_stop* move_closest_stop = nullptr;
    
    //Closest vertex to the mouse when moving.
    vertex* move_closest_vertex = nullptr;
    
    //The moved thing was here when the move started (world coords).
    point move_start_pos;
    
    //The mouse cursor was here when the move started (world coords).
    point move_mouse_start_pos;
    
    //Currently moving the selected vertexes, objects, etc.?
    bool moving = false;
    
    //Path preview checkpoint that is currently being moved, or -1 for none.
    signed char moving_path_preview_checkpoint = -1;
    
    //Cross-section point that is currently being moved, or -1 for none.
    signed char moving_cross_section_point = -1;
    
    //New circle sector's second point.
    point new_circle_sector_anchor;
    
    //New circle sector's center.
    point new_circle_sector_center;
    
    //Points where the new circle sector's vertexes will end up.
    vector<point> new_circle_sector_points;
    
    //What step of the circular sector building process are we in?
    unsigned char new_circle_sector_step = 0;
    
    //For each edge of the new circle sector, is it valid?
    vector<bool> new_circle_sector_valid_edges;
    
    //Time left to keep the error-redness of the new sector's line(s) for.
    timer new_sector_error_tint_timer = timer(AREA_EDITOR::NEW_SECTOR_ERROR_TINT_DURATION);
    
    //Mouse drag start coordinates, when using on-canvas texture effect editing.
    point octee_drag_start;
    
    //Texture's original angle, when using on-canvas texture effect editing.
    float octee_orig_angle = 0.0f;
    
    //Texture's original offset, when using on-canvas texture effect editing.
    point octee_orig_offset;
    
    //Texture's original scale, when using on-canvas texture effect editing.
    point octee_orig_scale;
    
    //Current on-canvas texture effect edit mode.
    OCTEE_MODE octee_mode = OCTEE_MODE_OFFSET;
    
    //When drawing a path, use these stop flags.
    bitmask_8_t path_drawing_flags = 0;
    
    //When drawing a path, use this label.
    string path_drawing_label;
    
    //When drawing a path, create normal links. False for one-way links.
    bool path_drawing_normals = true;
    
    //When drawing a path, use this type.
    PATH_LINK_TYPE path_drawing_type = PATH_LINK_TYPE_NORMAL;
    
    //First stop of the next link when drawing a path.
    path_stop* path_drawing_stop_1 = nullptr;
    
    //Path stops that make up the current path preview.
    vector<path_stop*> path_preview;
    
    //Location of the two path preview checkpoints.
    point path_preview_checkpoints[2];
    
    //The closest stop to the path preview start and end.
    path_stop* path_preview_closest[2] = { nullptr, nullptr };
    
    //Total distance of the previewed path.
    float path_preview_dist = 0.0f;
    
    //Settings for the path preview.
    path_follow_settings path_preview_settings;
    
    //Result of the path preview's calculation.
    PATH_RESULT path_preview_result = PATH_RESULT_NOT_CALCULATED;
    
    //Only calculate the preview path when this time is up.
    timer path_preview_timer;
    
    //Area data before vertex movement.
    area_data* pre_move_area_data = nullptr;
    
    //Position of the selected mobs before movement.
    map<mob_gen*, point> pre_move_mob_coords;
    
    //Position of the selected path stops before movement.
    map<path_stop*, point> pre_move_stop_coords;
    
    //Position of the selected vertexes before movement.
    map<vertex*, point> pre_move_vertex_coords;
    
    //Is preview mode on?
    bool preview_mode = false;
    
    //Name of the area song we're previewing, if any.
    string preview_song;
    
    //Description of the current problem found.
    string problem_description;
    
    //Information about the problematic intersecting edges, if any.
    edge_intersection problem_edge_intersection = edge_intersection(nullptr, nullptr);
    
    //Pointer to the problematic mob, if any.
    mob_gen* problem_mob_ptr = nullptr;
    
    //Pointer to the problematic path stop, if any.
    path_stop* problem_path_stop_ptr = nullptr;
    
    //Type of the current problem found in the review panel.
    EPT problem_type = EPT_NONE_YET;
    
    //Pointer to the problematic sector, if any.
    sector* problem_sector_ptr = nullptr;
    
    //Pointer to the problematic tree shadow, if any.
    tree_shadow* problem_shadow_ptr = nullptr;
    
    //Title of the current problem found.
    string problem_title;
    
    //Pointer to the problematic vertex, if any.
    vertex* problem_vertex_ptr = nullptr;
    
    //Sector heights when the quick height set mode was entered.
    map<sector*, float> quick_height_set_start_heights;
    
    //Mouse coordinates (screen) when the quick height set mode was entered.
    point quick_height_set_start_pos;
    
    //Time left in the quick preview mode, including fade out.
    timer quick_preview_timer = timer(AREA_EDITOR::QUICK_PREVIEW_DURATION);
    
    //Redo history, with the state of the area at each point. Front = latest.
    deque<std::pair<area_data*, string> > redo_history;
    
    //Opacity of the reference image.
    unsigned char reference_alpha = 255;
    
    //Reference image center.
    point reference_center;
    
    //Reference image dimensions.
    point reference_size;
    
    //Bitmap of the reference image.
    ALLEGRO_BITMAP* reference_bitmap = nullptr;
    
    //File name of the reference image.
    string reference_file_name;
    
    //Keep the aspect ratio when resizing the reference?
    bool reference_keep_aspect_ratio = true;
    
    //Info about the current sector split operation.
    sector_split_t sector_split_info;
    
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
    tree_shadow* selected_shadow = nullptr;
    
    //Keep the aspect ratio of the currently selected shadow?
    bool selected_shadow_keep_aspect_ratio = false;
    
    //Currently selected vertexes.
    set<vertex*> selected_vertexes;
    
    //Is the user currently performing a rectangle box?
    bool selecting = false;
    
    //Angle of the selection.
    float selection_angle = 0.0f;
    
    //Center of the selection.
    point selection_center;
    
    //The selection's alpha depends on this value.
    float selection_effect = 0.0f;
    
    //Point where the selection is currently at.
    point selection_end;
    
    //Current selection filter.
    SELECTION_FILTER selection_filter = SELECTION_FILTER_SECTORS;
    
    //Has the user agreed to homogenize the selection?
    bool selection_homogenized = false;
    
    //Angle of the selection, before it got transformed.
    float selection_orig_angle = 0.0f;
    
    //Center of the selection, before it got transformed.
    point selection_orig_center;
    
    //Size of the selection, before it got transformed.
    point selection_orig_size;
    
    //Size of the selection, padding included.
    point selection_size;
    
    //Point where the selection started.
    point selection_start;
    
    //Show blocking vs. non-blocking sectors?
    bool show_blocking_sectors = false;
    
    //Show the path stop closest to the cursor?
    bool show_closest_stop = false;
    
    //Use the cross-section view tool?
    bool show_cross_section = false;
    
    //When using the cross-section view tool, render the grid?
    bool show_cross_section_grid = true;
    
    //Show the path preview and the checkpoints?
    bool show_path_preview = false;
    
    //Render the reference image?
    bool show_reference = true;
    
    //Render the tree shadows?
    bool show_shadows = false;
    
    //List of texture suggestions.
    vector<texture_suggestion> texture_suggestions;
    
    //Position of the load widget.
    point load_widget_pos;
    
    //Position of the reload widget.
    point reload_widget_pos;
    
    //Position of the quit widget.
    point quit_widget_pos;
    
    //Was the area's thumbnail changed in any way since the last save?
    bool thumbnail_needs_saving = false;
    
    //Was the area's thumbnail changed in any way since the last backup save?
    bool thumbnail_backup_needs_saving = false;
    
    //Undo history, with the state of the area at each point. Front = latest.
    deque<std::pair<area_data*, string> > undo_history;
    
    //Name of the undo operation responsible for the lock.
    string undo_save_lock_operation;
    
    //During this timer, don't save state for operations matching the last one.
    timer undo_save_lock_timer;
    
    struct {
    
        //Selected pack.
        string pack;
        
        //Internal name of the new area.
        string internal_name = "my_area";
        
        //Selected area type.
        int type = AREA_TYPE_SIMPLE;
        
        //Problem found, if any.
        string problem;
        
        //Path to the new area.
        string area_path;
        
        //Whether the dialog needs updating.
        bool must_update = true;
        
        //Whether we need to focus on the text input widget.
        bool needs_text_focus = true;
        
    } new_dialog;
    
    
    //--- Function declarations ---
    
    bool are_nodes_traversable(
        const layout_drawing_node &n1,
        const layout_drawing_node &n2
    ) const;
    float calculate_day_speed(
        float day_start_min, float day_end_min,
        float mission_min
    );
    void cancel_circle_sector();
    void cancel_layout_drawing();
    void cancel_layout_moving();
    float calculate_preview_path();
    void change_state(const EDITOR_STATE new_state);
    void check_drawing_line(const point &pos);
    void clear_circle_sector();
    void clear_current_area();
    void clear_layout_drawing();
    void clear_layout_moving();
    void clear_problems();
    void clear_selection();
    void clear_texture_suggestions();
    void clear_undo_history();
    void close_load_dialog();
    void close_options_dialog();
    void copy_edge_properties();
    void copy_mob_properties();
    void copy_path_link_properties();
    void copy_sector_properties();
    void create_area(const string &requested_area_path);
    void create_drawing_vertexes();
    void create_mob_under_cursor();
    sector* create_sector_for_layout_drawing(const sector* copy_from);
    void delete_current_area();
    void delete_edge(edge* e_ptr);
    bool delete_edges(const set<edge*> &which);
    void delete_mobs(const set<mob_gen*> &which);
    void delete_path_links(const set<path_link*> &which);
    void delete_path_stops(const set<path_stop*> &which);
    void do_sector_split();
    void emit_triangulation_error_status_bar_message(
        const TRIANGULATION_ERROR error
    );
    string find_good_first_texture();
    void find_problems();
    void finish_circle_sector();
    void finish_layout_moving();
    void finish_new_sector_drawing();
    void forget_prepared_state(area_data* prepared_change);
    void get_affected_sectors(
        sector* s_ptr, unordered_set<sector*> &list
    ) const;
    void get_affected_sectors(
        const set<sector*> &sectors, unordered_set<sector*> &list
    ) const;
    void get_affected_sectors(
        const set<vertex*> &vertexes, unordered_set<sector*> &list
    ) const;
    void get_hovered_layout_element(
        vertex** clicked_vertex, edge** clicked_edge, sector** clicked_sector
    ) const;
    edge* get_closest_edge_to_angle(
        vertex* v_ptr, float angle, bool clockwise,
        float* out_closest_edge_angle
    ) const;
    bool get_common_sector(
        vector<vertex*> &vertexes, vector<edge*> &edges, sector** result
    ) const;
    edge* get_correct_post_split_edge(
        const vertex* v_ptr, edge* e1_ptr, edge* e2_ptr
    ) const;
    bool get_drawing_outer_sector(sector** result) const;
    edge* get_edge_under_point(
        const point &p, const edge* after = nullptr
    ) const;
    string get_folder_tooltip(
        const string &path, const string &user_data_path
    ) const;
    vector<edge_intersection> get_intersecting_edges() const;
    size_t get_mission_required_mob_count() const;
    float get_mob_gen_radius(mob_gen* m) const;
    bool get_mob_link_under_point(
        const point &p,
        std::pair<mob_gen*, mob_gen*>* data1,
        std::pair<mob_gen*, mob_gen*>* data2
    ) const;
    mob_gen* get_mob_under_point(
        const point &p, size_t* out_idx = nullptr
    ) const;
    bool get_path_link_under_point(
        const point &p, path_link** link1, path_link** link2
    ) const;
    path_stop* get_path_stop_under_point(const point &p) const;
    float get_quick_height_set_offset() const;
    SECTOR_SPLIT_RESULT get_sector_split_evaluation();
    sector* get_sector_under_point(const point &p) const;
    vertex* get_vertex_under_point(const point &p) const;
    void go_to_undo_history_point(size_t p);
    void goto_problem();
    void handle_line_error();
    void homogenize_selected_edges();
    void homogenize_selected_mobs();
    void homogenize_selected_path_links();
    void homogenize_selected_path_stops();
    void homogenize_selected_sectors();
    void load_area_folder(
        const string &requested_area_path,
        bool from_backup, bool should_update_history
    );
    void load_backup();
    void load_reference();
    bool merge_sectors(sector* s1, sector* s2);
    void merge_vertex(
        const vertex* v1, vertex* v2, unordered_set<sector*>* affected_sectors
    );
    void paste_edge_properties();
    void paste_mob_properties();
    void paste_path_link_properties();
    void paste_sector_properties();
    void paste_sector_texture();
    area_data* prepare_state();
    void recreate_drawing_nodes();
    void redo();
    void register_change(
        const string &operation_name, area_data* pre_prepared_change = nullptr
    );
    void reload_areas();
    void remove_thumbnail();
    void resize_everything(float mults[2]);
    void rollback_to_prepared_state(area_data* prepared_state);
    void rotate_mob_gens_to_point(const point &pos);
    bool save_area(bool to_backup);
    void save_backup();
    void save_reference();
    void select_edge(edge* e_ptr);
    void select_path_stops_with_label(const string &label);
    void select_sector(sector* s_ptr);
    void select_tree_shadow(tree_shadow* s_ptr);
    void select_vertex(vertex* v_ptr);
    void set_new_circle_sector_points();
    void set_selection_status_text();
    void set_state_from_undo_or_redo_history(area_data* state);
    void setup_for_new_area_post();
    void setup_for_new_area_pre();
    void setup_sector_split();
    point snap_point(const point &p, bool ignore_selected = false);
    vertex* split_edge(edge* e_ptr, const point &where);
    path_stop* split_path_link(
        path_link* l1, path_link* l2,
        const point &where
    );
    void start_mob_move();
    void start_path_stop_move();
    void start_vertex_move();
    void traverse_sector_for_split(
        const sector* s_ptr, vertex* begin, const vertex* checkpoint,
        vector<edge*>* edges, vector<vertex*>* vertexes,
        bool* working_sector_left
    );
    void undo();
    void undo_layout_drawing_node();
    void update_affected_sectors(
        const unordered_set<sector*> &affected_sectors
    );
    void update_all_edge_offset_caches();
    void update_inner_sectors_outer_sector(
        const vector<edge*> &edges_to_check,
        const sector* old_outer, sector* new_outer
    );
    void update_reference();
    void update_layout_drawing_status_text();
    void update_sector_texture(sector* s_ptr, const string &internal_name);
    void update_texture_suggestions(const string &n);
    void update_undo_history();
    void update_vertex_selection();
    void draw_arrow(
        const point &start, const point &end,
        float start_offset, float end_offset,
        float thickness, const ALLEGRO_COLOR &color
    );
    static void draw_canvas_imgui_callback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void draw_cross_section_sector(
        float start_ratio, float end_ratio, float proportion,
        float lowest_z, const sector* sector_ptr
    );
    void draw_debug_text(
        const ALLEGRO_COLOR color, const point &where, const string &text,
        unsigned char dots = 0
    );
    void draw_line_dist(
        const point &focus, const point &other, const string &prefix = ""
    );
    void open_load_dialog();
    void open_new_dialog();
    void open_options_dialog();
    void pick_area_folder(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void pick_texture(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void circle_sector_cmd(float input_value);
    void copy_properties_cmd(float input_value);
    void delete_area_cmd(float input_value);
    void delete_cmd(float input_value);
    void delete_edge_cmd(float input_value);
    void delete_mob_cmd(float input_value);
    void delete_path_cmd(float input_value);
    void delete_tree_shadow_cmd(float input_value);
    void duplicate_mobs_cmd(float input_value);
    void grid_interval_decrease_cmd(float input_value);
    void grid_interval_increase_cmd(float input_value);
    void layout_drawing_cmd(float input_value);
    void load_cmd(float input_value);
    void new_mob_cmd(float input_value);
    void new_path_cmd(float input_value);
    void new_tree_shadow_cmd(float input_value);
    void paste_properties_cmd(float input_value);
    void paste_texture_cmd(float input_value);
    void quick_play_cmd(float input_value);
    void quit_cmd(float input_value);
    void redo_cmd(float input_value);
    void reference_toggle_cmd(float input_value);
    void reload_cmd(float input_value);
    void save_cmd(float input_value);
    void select_all_cmd(float input_value);
    void selection_filter_cmd(float input_value);
    void snap_mode_cmd(float input_value);
    void undo_cmd(float input_value);
    void zoom_and_pos_reset_cmd(float input_value);
    void zoom_everything_cmd(float input_value);
    void zoom_in_cmd(float input_value);
    void zoom_out_cmd(float input_value);
    void process_gui();
    void process_gui_control_panel();
    void process_gui_delete_area_dialog();
    void process_gui_load_dialog();
    void process_gui_new_dialog();
    void process_gui_menu_bar();
    void process_gui_mob_script_vars(mob_gen* gen);
    void process_gui_panel_details();
    void process_gui_panel_edge();
    void process_gui_panel_gameplay();
    void process_gui_panel_info();
    void process_gui_panel_layout();
    void process_gui_panel_main();
    void process_gui_panel_mission();
    void process_gui_panel_mob();
    void process_gui_panel_mobs();
    void process_gui_panel_path_link();
    void process_gui_panel_path_stop();
    void process_gui_panel_paths();
    void process_gui_panel_review();
    void process_gui_panel_sector();
    void process_gui_panel_tools();
    void process_gui_options_dialog();
    void process_gui_status_bar();
    void process_gui_toolbar();
    void handle_key_char_anywhere(const ALLEGRO_EVENT &ev) override;
    void handle_key_char_canvas(const ALLEGRO_EVENT &ev) override;
    void handle_key_down_anywhere(const ALLEGRO_EVENT &ev) override;
    void handle_key_down_canvas(const ALLEGRO_EVENT &ev) override;
    void handle_key_up_anywhere(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_double_click(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_drag(const ALLEGRO_EVENT &ev) override;
    void handle_lmb_up(const ALLEGRO_EVENT &ev) override;
    void handle_mmb_double_click(const ALLEGRO_EVENT &ev) override;
    void handle_mmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_mmb_drag(const ALLEGRO_EVENT &ev) override;
    void handle_mouse_update(const ALLEGRO_EVENT &ev) override;
    void handle_mouse_wheel(const ALLEGRO_EVENT &ev) override;
    void handle_rmb_double_click(const ALLEGRO_EVENT &ev) override;
    void handle_rmb_down(const ALLEGRO_EVENT &ev) override;
    void handle_rmb_drag(const ALLEGRO_EVENT &ev) override;
    void pan_cam(const ALLEGRO_EVENT &ev);
    void reset_cam_xy();
    void reset_cam_zoom();
    
};
