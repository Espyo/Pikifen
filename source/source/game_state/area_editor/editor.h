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

#include "../editor.h"

#include "../../content/area/area.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../util/general_utils.h"


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
extern const float UNDO_SAVE_LOCK_DURATION;
extern const float VERTEX_MERGE_RADIUS;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the area editor.
 */
class AreaEditor : public Editor {

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
    Point quick_play_cam_pos;
    
    //Editor camera zoom before quick-play.
    float quick_play_cam_z = 1.0f;
    
    //This hack fixes a glitch by skipping drawing for one frame.
    bool hack_skip_drawing = false;
    
    
    //--- Function declarations ---
    
    AreaEditor();
    void doLogic() override;
    void doDrawing() override;
    void load() override;
    void unload() override;
    string getName() const override;
    void drawCanvas();
    string getOpenedContentPath() const;
    
private:

    /**
     * @brief Represents a suggested texture.
     */
    struct TextureSuggestion {
    
        //--- Members ---
        
        //Bitmap of the texture.
        ALLEGRO_BITMAP* bmp = nullptr;
        
        //Internal name of the texture.
        string name;
        
        
        //--- Function declarations ---
        
        explicit TextureSuggestion(const string &n);
        void destroy();
        
    };
    
    /**
     * @brief Represents a point in the current layout drawing.
     */
    struct LayoutDrawingNode {
    
        //--- Members ---
        
        //Raw coordinates of the mouse click.
        Point raw_spot;
        
        //Final spot of the node, after snapping to an existing vertex/edge.
        Point snapped_spot;
        
        //Is this node on top of an existing vertex? This points to it if so.
        Vertex* on_vertex = nullptr;
        
        //on_vertex's vertex index.
        size_t on_vertex_idx = INVALID;
        
        //Is this node on top of an existing edge? This points to it if so.
        Edge* on_edge = nullptr;
        
        //on_edge's edge index.
        size_t on_edge_idx = INVALID;
        
        //Is this node just on top of a sector? This points to it if so.
        Sector* on_sector = nullptr;
        
        //on_sector's sector index.
        size_t on_sector_idx = INVALID;
        
        //Is on_vertex a new vertex, created during the sector creation?
        bool is_new_vertex = false;
        
        
        //--- Function declarations ---
        
        LayoutDrawingNode(
            const AreaEditor* ae_ptr, const Point &mouse_click
        );
        LayoutDrawingNode();
        
    };
    
    /**
     * @brief Info pertaining to a sector split operation.
     */
    struct SectorSplit {
    
        //--- Members ---
        
        //Area data from before the split.
        Area* pre_split_area_data = nullptr;
        
        //Sector being worked on in a sector split operation.
        Sector* working_sector = nullptr;
        
        //Edges of the sector split sector, before the split operation.
        vector<Edge*> working_sector_old_edges;
        
        //Edges traversed in each step.
        vector<Edge*> traversed_edges[2];
        
        //Vertexes traversed in each step.
        vector<Vertex*> traversed_vertexes[2];
        
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
    Timer backup_timer;
    
    //When the player copies an edge's properties, they go here.
    Edge* copy_buffer_edge = nullptr;
    
    //When the player copies a mob's properties, they go here.
    MobGen* copy_buffer_mob = nullptr;
    
    //When the player copies a path link's properties, they go here.
    PathLink* copy_buffer_path_link = nullptr;
    
    //When the player copies a sector's properties, they go here.
    Sector* copy_buffer_sector = nullptr;
    
    //Where the cross-section tool points are.
    Point cross_section_checkpoints[2];
    
    //Cross-section window's start coordinates.
    Point cross_section_window_start;
    
    //Cross-section window's end coordinates.
    Point cross_section_window_end;
    
    //Cross-section Z legend window's start coordinates.
    Point cross_section_z_window_start;
    
    //Cross-section Z legend window's end coordinates.
    Point cross_section_z_window_end;
    
    //When showing a hazard in the list, this is the index of the current one.
    size_t cur_hazard_idx = INVALID;
    
    //The current transformation widget.
    TransformationWidget cur_transformation_widget;
    
    //Last known cursor snap position for heavy snap modes.
    Point cursor_snap_cache;
    
    //Time left to update the cursor snap position for heavy snap modes.
    Timer cursor_snap_timer = Timer(AREA_EDITOR::CURSOR_SNAP_UPDATE_INTERVAL);
    
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
    vector<LayoutDrawingNode> drawing_nodes;
    
    //Result of the current drawing line.
    DRAWING_LINE_RESULT drawing_line_result = DRAWING_LINE_RESULT_OK;
    
    //Currently highlighted edge, if any.
    Edge* highlighted_edge = nullptr;
    
    //Currently highlighted mob, if any.
    MobGen* highlighted_mob = nullptr;
    
    //Currently highlighted path link, if any.
    PathLink* highlighted_path_link = nullptr;
    
    //Currently highlighted path stop, if any.
    PathStop* highlighted_path_stop = nullptr;
    
    //Currently highlighted sector, if any.
    Sector* highlighted_sector = nullptr;
    
    //Currently highlighted vertex, if any.
    Vertex* highlighted_vertex = nullptr;
    
    //Category name of the last mob placed.
    string last_mob_custom_cat_name;
    
    //Mob type of the last mob placed.
    MobType* last_mob_type = nullptr;
    
    //Are we editing sectors or edges?
    LAYOUT_MODE layout_mode = LAYOUT_MODE_SECTORS;
    
    //Picker info for the picker in the "load" dialog.
    Picker load_dialog_picker;
    
    //Closest mob to the mouse when moving.
    MobGen* move_closest_mob = nullptr;
    
    //Closest path stop to the mouse when moving.
    PathStop* move_closest_stop = nullptr;
    
    //Closest vertex to the mouse when moving.
    Vertex* move_closest_vertex = nullptr;
    
    //The moved thing was here when the move started (world coords).
    Point move_start_pos;
    
    //The mouse cursor was here when the move started (world coords).
    Point move_mouse_start_pos;
    
    //Currently moving the selected vertexes, objects, etc.?
    bool moving = false;
    
    //Path preview checkpoint that is currently being moved, or -1 for none.
    signed char moving_path_preview_checkpoint = -1;
    
    //Cross-section point that is currently being moved, or -1 for none.
    signed char moving_cross_section_point = -1;
    
    //New circle sector's second point.
    Point new_circle_sector_anchor;
    
    //New circle sector's center.
    Point new_circle_sector_center;
    
    //Points where the new circle sector's vertexes will end up.
    vector<Point> new_circle_sector_points;
    
    //What step of the circular sector building process are we in?
    unsigned char new_circle_sector_step = 0;
    
    //For each edge of the new circle sector, is it valid?
    vector<bool> new_circle_sector_valid_edges;
    
    //Time left to keep the error-redness of the new sector's line(s) for.
    Timer new_sector_error_tint_timer = Timer(AREA_EDITOR::NEW_SECTOR_ERROR_TINT_DURATION);
    
    //Mouse drag start coordinates, when using on-canvas texture effect editing.
    Point octee_drag_start;
    
    //Texture's original angle, when using on-canvas texture effect editing.
    float octee_orig_angle = 0.0f;
    
    //Texture's original offset, when using on-canvas texture effect editing.
    Point octee_orig_offset;
    
    //Texture's original scale, when using on-canvas texture effect editing.
    Point octee_orig_scale;
    
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
    PathStop* path_drawing_stop_1 = nullptr;
    
    //Path stops that make up the current path preview.
    vector<PathStop*> path_preview;
    
    //Location of the two path preview checkpoints.
    Point path_preview_checkpoints[2];
    
    //The closest stop to the path preview start and end.
    PathStop* path_preview_closest[2] = { nullptr, nullptr };
    
    //Total distance of the previewed path.
    float path_preview_dist = 0.0f;
    
    //Settings for the path preview.
    PathFollowSettings path_preview_settings;
    
    //Result of the path preview's calculation.
    PATH_RESULT path_preview_result = PATH_RESULT_NOT_CALCULATED;
    
    //Only calculate the preview path when this time is up.
    Timer path_preview_timer;
    
    //Area data before vertex movement.
    Area* pre_move_area_data = nullptr;
    
    //Position of the selected mobs before movement.
    map<MobGen*, Point> pre_move_mob_coords;
    
    //Position of the selected path stops before movement.
    map<PathStop*, Point> pre_move_stop_coords;
    
    //Position of the selected vertexes before movement.
    map<Vertex*, Point> pre_move_vertex_coords;
    
    //Is preview mode on?
    bool preview_mode = false;
    
    //Name of the area song we're previewing, if any.
    string preview_song;
    
    //Description of the current problem found.
    string problem_description;
    
    //Information about the problematic intersecting edges, if any.
    EdgeIntersection problem_edge_intersection = EdgeIntersection(nullptr, nullptr);
    
    //Pointer to the problematic mob, if any.
    MobGen* problem_mob_ptr = nullptr;
    
    //Pointer to the problematic path stop, if any.
    PathStop* problem_path_stop_ptr = nullptr;
    
    //Type of the current problem found in the review panel.
    EPT problem_type = EPT_NONE_YET;
    
    //Pointer to the problematic sector, if any.
    Sector* problem_sector_ptr = nullptr;
    
    //Pointer to the problematic tree shadow, if any.
    TreeShadow* problem_shadow_ptr = nullptr;
    
    //Title of the current problem found.
    string problem_title;
    
    //Pointer to the problematic vertex, if any.
    Vertex* problem_vertex_ptr = nullptr;
    
    //Sector heights when the quick height set mode was entered.
    map<Sector*, float> quick_height_set_start_heights;
    
    //Mouse coordinates (screen) when the quick height set mode was entered.
    Point quick_height_set_start_pos;
    
    //Time left in the quick preview mode, including fade out.
    Timer quick_preview_timer = Timer(AREA_EDITOR::QUICK_PREVIEW_DURATION);
    
    //Redo history, with the state of the area at each point. Front = latest.
    deque<std::pair<Area*, string> > redo_history;
    
    //Opacity of the reference image.
    unsigned char reference_alpha = 255;
    
    //Reference image center.
    Point reference_center;
    
    //Reference image dimensions.
    Point reference_size;
    
    //Bitmap of the reference image.
    ALLEGRO_BITMAP* reference_bitmap = nullptr;
    
    //File path of the reference image.
    string reference_file_path;
    
    //Keep the aspect ratio when resizing the reference?
    bool reference_keep_aspect_ratio = true;
    
    //Info about the current sector split operation.
    SectorSplit sector_split_info;
    
    //Currently selected edges.
    set<Edge*> selected_edges;
    
    //Currently selected mobs.
    set<MobGen*> selected_mobs;
    
    //Currently selected path links.
    set<PathLink*> selected_path_links;
    
    //Currently selected path stops.
    set<PathStop*> selected_path_stops;
    
    //Currently selected sectors.
    set<Sector*> selected_sectors;
    
    //Currently selected tree shadow.
    TreeShadow* selected_shadow = nullptr;
    
    //Keep the aspect ratio of the currently selected shadow?
    bool selected_shadow_keep_aspect_ratio = false;
    
    //Currently selected vertexes.
    set<Vertex*> selected_vertexes;
    
    //Is the user currently performing a rectangle box?
    bool selecting = false;
    
    //Angle of the selection.
    float selection_angle = 0.0f;
    
    //Center of the selection.
    Point selection_center;
    
    //The selection's alpha depends on this value.
    float selection_effect = 0.0f;
    
    //Point where the selection is currently at.
    Point selection_end;
    
    //Current selection filter.
    SELECTION_FILTER selection_filter = SELECTION_FILTER_SECTORS;
    
    //Has the user agreed to homogenize the selection?
    bool selection_homogenized = false;
    
    //Angle of the selection, before it got transformed.
    float selection_orig_angle = 0.0f;
    
    //Center of the selection, before it got transformed.
    Point selection_orig_center;
    
    //Size of the selection, before it got transformed.
    Point selection_orig_size;
    
    //Size of the selection, padding included.
    Point selection_size;
    
    //Point where the selection started.
    Point selection_start;
    
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
    vector<TextureSuggestion> texture_suggestions;
    
    //Position of the load widget.
    Point load_widget_pos;
    
    //Position of the reload widget.
    Point reload_widget_pos;
    
    //Position of the quit widget.
    Point quit_widget_pos;
    
    //Was the area's thumbnail changed in any way since the last save?
    bool thumbnail_needs_saving = false;
    
    //Was the area's thumbnail changed in any way since the last backup save?
    bool thumbnail_backup_needs_saving = false;
    
    //Undo history, with the state of the area at each point. Front = latest.
    deque<std::pair<Area*, string> > undo_history;
    
    //Name of the undo operation responsible for the lock.
    string undo_save_lock_operation;
    
    //During this timer, don't save state for operations matching the last one.
    Timer undo_save_lock_timer;
    
    struct {
    
        //Selected pack.
        string pack;
        
        //Internal name of the new area.
        string internal_name = "my_area";
        
        //Selected area type.
        int type = AREA_TYPE_SIMPLE;
        
        //Path to the new area.
        string area_path;
        
        //Last time we checked if the new area path existed, it was this.
        string last_checked_area_path;
        
        //Does a folder already exist under the new area's path?
        bool area_path_exists = false;
        
        //Whether we need to focus on the text input widget.
        bool needs_text_focus = true;
        
    } new_dialog;
    
    
    //--- Function declarations ---
    
    bool areNodesTraversable(
        const LayoutDrawingNode &n1,
        const LayoutDrawingNode &n2
    ) const;
    float calculateDaySpeed(
        float day_start_min, float day_end_min,
        float mission_min
    );
    void cancelCircleSector();
    void cancelLayoutDrawing();
    void cancelLayoutMoving();
    float calculatePreviewPath();
    void changeState(const EDITOR_STATE new_state);
    void checkDrawingLine(const Point &pos);
    void clearCircleSector();
    void clearCurrentArea();
    void clearLayoutDrawing();
    void clearLayoutMoving();
    void clearProblems();
    void clearSelection();
    void clearTextureSuggestions();
    void clearUndoHistory();
    void closeLoadDialog();
    void closeOptionsDialog();
    void copyEdgeProperties();
    void copyMobProperties();
    void copyPathLinkProperties();
    void copySectorProperties();
    void createArea(const string &requested_area_path);
    void createDrawingVertexes();
    void createMobUnderCursor();
    Sector* createSectorForLayoutDrawing(const Sector* copy_from);
    void deleteCurrentArea();
    void deleteEdge(Edge* e_ptr);
    bool deleteEdges(const set<Edge*> &which);
    void deleteMobs(const set<MobGen*> &which);
    void deletePathLinks(const set<PathLink*> &which);
    void deletePathStops(const set<PathStop*> &which);
    void doSectorSplit();
    void emitTriangulationErrorStatusBarMessage(
        const TRIANGULATION_ERROR error
    );
    string findGoodFirstTexture();
    void findProblems();
    void findProblemsBridgePath();
    void findProblemsIntersectingEdge();
    void findProblemsLoneEdge();
    void findProblemsLonePathStop();
    void findProblemsMissingLeader();
    void findProblemsMissingTexture();
    void findProblemsMobInsideWalls();
    void findProblemsMobLinksToSelf();
    void findProblemsMobStoredInLoop();
    void findProblemsNoGoalMob();
    void findProblemsNoScoreCriteria();
    void findProblemsNonSimpleSector();
    void findProblemsOobMob();
    void findProblemsOobPathStop();
    void findProblemsOverlappingVertex();
    void findProblemsPathStopOnLink();
    void findProblemsPathStopsIntersecting();
    void findProblemsPikminOverLimit();
    void findProblemsTypelessMob();
    void findProblemsUnknownTexture();
    void findProblemsUnknownTreeShadow();
    void finishCircleSector();
    void finishLayoutMoving();
    void finishNewSectorDrawing();
    void forgetPreparedState(Area* prepared_change);
    void getAffectedSectors(
        Sector* s_ptr, unordered_set<Sector*> &list
    ) const;
    void getAffectedSectors(
        const set<Sector*> &sectors, unordered_set<Sector*> &list
    ) const;
    void getAffectedSectors(
        const set<Vertex*> &vertexes, unordered_set<Sector*> &list
    ) const;
    void getHoveredLayoutElement(
        Vertex** clicked_vertex, Edge** clicked_edge, Sector** clicked_sector
    ) const;
    Edge* getClosestEdgeToAngle(
        Vertex* v_ptr, float angle, bool clockwise,
        float* out_closest_edge_angle
    ) const;
    bool getCommonSector(
        vector<Vertex*> &vertexes, vector<Edge*> &edges, Sector** result
    ) const;
    Edge* getCorrectPostSplitEdge(
        const Vertex* v_ptr, Edge* e1_ptr, Edge* e2_ptr
    ) const;
    bool getDrawingOuterSector(Sector** result) const;
    Edge* getEdgeUnderPoint(
        const Point &p, const Edge* after = nullptr
    ) const;
    string getFolderTooltip(
        const string &path, const string &user_data_path
    ) const;
    vector<EdgeIntersection> getIntersectingEdges() const;
    size_t getMissionRequiredMobCount() const;
    float getMobGenRadius(MobGen* m) const;
    bool getMobLinkUnderPoint(
        const Point &p,
        std::pair<MobGen*, MobGen*>* data1,
        std::pair<MobGen*, MobGen*>* data2
    ) const;
    MobGen* getMobUnderPoint(
        const Point &p, size_t* out_idx = nullptr
    ) const;
    bool getPathLinkUnderPoint(
        const Point &p, PathLink** link1, PathLink** link2
    ) const;
    PathStop* getPathStopUnderPoint(const Point &p) const;
    float getQuickHeightSetOffset() const;
    SECTOR_SPLIT_RESULT getSectorSplitEvaluation();
    Sector* getSectorUnderPoint(const Point &p) const;
    Vertex* getVertexUnderPoint(const Point &p) const;
    void goToUndoHistoryPoint(size_t p);
    void goToProblem();
    void handleLineError();
    void homogenizeSelectedEdges();
    void homogenizeSelectedMobs();
    void homogenizeSelectedPathLinks();
    void homogenizeSelectedPathStops();
    void homogenizeSelectedSectors();
    void loadAreaFolder(
        const string &requested_area_path,
        bool from_backup, bool should_update_history
    );
    void loadBackup();
    void loadReference();
    bool mergeSectors(Sector* s1, Sector* s2);
    void mergeVertex(
        const Vertex* v1, Vertex* v2, unordered_set<Sector*>* affected_sectors
    );
    void pasteEdgeProperties();
    void pasteMobProperties();
    void pastePathLinkProperties();
    void pasteSectorProperties();
    void pasteSectorTexture();
    Area* prepareState();
    void recreateDrawingNodes();
    void redo();
    void registerChange(
        const string &operation_name, Area* pre_prepared_change = nullptr
    );
    void reloadAreas();
    void removeThumbnail();
    void resizeEverything(float mults[2]);
    void rollbackToPreparedState(Area* prepared_state);
    void rotateMobGensToPoint(const Point &pos);
    bool saveArea(bool to_backup);
    void saveBackup();
    void saveReference();
    void selectEdge(Edge* e_ptr);
    void selectPathStopsWithLabel(const string &label);
    void selectSector(Sector* s_ptr);
    void selectTreeShadow(TreeShadow* s_ptr);
    void selectVertex(Vertex* v_ptr);
    void setNewCircleSectorPoints();
    void setSelectionStatusText();
    void setStateFromUndoOrRedoHistory(Area* state);
    void setupForNewAreaPost();
    void setupForNewAreaPre();
    void setupSectorSplit();
    Point snapPoint(const Point &p, bool ignore_selected = false);
    Vertex* splitEdge(Edge* e_ptr, const Point &where);
    PathStop* splitPathLink(
        PathLink* l1, PathLink* l2,
        const Point &where
    );
    void startMobMove();
    void startPathStopMove();
    void startVertexMove();
    void traverseSectorForSplit(
        const Sector* s_ptr, Vertex* begin, const Vertex* checkpoint,
        vector<Edge*>* edges, vector<Vertex*>* vertexes,
        bool* working_sector_left
    );
    void undo();
    void undoLayoutDrawingNode();
    void updateAffectedSectors(
        const unordered_set<Sector*> &affected_sectors
    );
    void updateAllEdgeOffsetCaches();
    void updateInnerSectorsOuterSector(
        const vector<Edge*> &edges_to_check,
        const Sector* old_outer, Sector* new_outer
    );
    void updateReference();
    void updateLayoutDrawingStatusText();
    void updateSectorTexture(Sector* s_ptr, const string &internal_name);
    void updateTextureSuggestions(const string &n);
    void updateUndoHistory();
    void updateVertexSelection();
    void drawArrow(
        const Point &start, const Point &end,
        float start_offset, float end_offset,
        float thickness, const ALLEGRO_COLOR &color
    );
    static void drawCanvasDearImGuiCallback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void drawCrossSectionSector(
        float start_ratio, float end_ratio, float proportion,
        float lowest_z, const Sector* sector_ptr
    );
    void drawDebugText(
        const ALLEGRO_COLOR color, const Point &where, const string &text,
        unsigned char dots = 0
    );
    void drawLineDist(
        const Point &focus, const Point &other, const string &prefix = ""
    );
    void openLoadDialog();
    void openNewDialog();
    void openOptionsDialog();
    void pickAreaFolder(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void pickTexture(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void circleSectorCmd(float input_value);
    void copyPropertiesCmd(float input_value);
    void deleteAreaCmd(float input_value);
    void deleteCmd(float input_value);
    void deleteEdgeCmd(float input_value);
    void deleteMobCmd(float input_value);
    void deletePathCmd(float input_value);
    void deleteTreeShadowCmd(float input_value);
    void duplicateMobsCmd(float input_value);
    void gridIntervalDecreaseCmd(float input_value);
    void gridIntervalIncreaseCmd(float input_value);
    void layoutDrawingCmd(float input_value);
    void loadCmd(float input_value);
    void newMobCmd(float input_value);
    void newPathCmd(float input_value);
    void newTreeShadowCmd(float input_value);
    void pastePropertiesCmd(float input_value);
    void pasteTextureCmd(float input_value);
    void quickPlayCmd(float input_value);
    void quitCmd(float input_value);
    void redoCmd(float input_value);
    void referenceToggleCmd(float input_value);
    void reloadCmd(float input_value);
    void saveCmd(float input_value);
    void selectAllCmd(float input_value);
    void selectionFilterCmd(float input_value);
    void snapModeCmd(float input_value);
    void undoCmd(float input_value);
    void zoomAndPosResetCmd(float input_value);
    void zoomEverythingCmd(float input_value);
    void zoomInCmd(float input_value);
    void zoomOutCmd(float input_value);
    void processGui();
    void processGuiControlPanel();
    void processGuiDeleteAreaDialog();
    void processGuiGradingCriterionWidgets(
        int* value_ptr, MISSION_SCORE_CRITERIA criterion_idx,
        const string &widget_label, const string &tooltip
    );
    void processGuiGradingMedalWidgets(
        int* requirement_ptr, const string &widget_label,
        int widget_min_value, int widget_max_value,
        const string &tooltip
    );
    void processGuiGradingModeWidgets(
        int value, const string &widget_label, const string &tooltip
    );
    void processGuiLoadDialog();
    void processGuiNewDialog();
    void processGuiMenuBar();
    void processGuiMobScriptVars(MobGen* gen);
    void processGuiPanelDetails();
    void processGuiPanelEdge();
    void processGuiPanelGameplay();
    void processGuiPanelInfo();
    void processGuiPanelLayout();
    void processGuiPanelMain();
    void processGuiPanelMission();
    void processGuiPanelMissionFail(bool* day_duration_needs_update);
    void processGuiPanelMissionGoalBe();
    void processGuiPanelMissionGoalCt();
    void processGuiPanelMissionGoalGte();
    void processGuiPanelMissionGrading();
    void processGuiPanelMob();
    void processGuiPanelMobs();
    void processGuiPanelPathLink();
    void processGuiPanelPathStop();
    void processGuiPanelPaths();
    void processGuiPanelReview();
    void processGuiPanelSector();
    void processGuiPanelTools();
    void processGuiOptionsDialog();
    void processGuiStatusBar();
    void processGuiToolbar();
    void handleKeyCharAnywhere(const ALLEGRO_EVENT &ev) override;
    void handleKeyCharCanvas(const ALLEGRO_EVENT &ev) override;
    void handleKeyDownAnywhere(const ALLEGRO_EVENT &ev) override;
    void handleKeyDownCanvas(const ALLEGRO_EVENT &ev) override;
    void handleKeyUpAnywhere(const ALLEGRO_EVENT &ev) override;
    void handleLmbDoubleClick(const ALLEGRO_EVENT &ev) override;
    void handleLmbDown(const ALLEGRO_EVENT &ev) override;
    void handleLmbDrag(const ALLEGRO_EVENT &ev) override;
    void handleLmbUp(const ALLEGRO_EVENT &ev) override;
    void handleMmbDoubleClick(const ALLEGRO_EVENT &ev) override;
    void handleMmbDown(const ALLEGRO_EVENT &ev) override;
    void handleMmbDrag(const ALLEGRO_EVENT &ev) override;
    void handleMouseUpdate(const ALLEGRO_EVENT &ev) override;
    void handleMouseWheel(const ALLEGRO_EVENT &ev) override;
    void handleRmbDoubleClick(const ALLEGRO_EVENT &ev) override;
    void handleRmbDown(const ALLEGRO_EVENT &ev) override;
    void handleRmbDrag(const ALLEGRO_EVENT &ev) override;
    void panCam(const ALLEGRO_EVENT &ev);
    void resetCamXY();
    void resetCamZoom();
    
};
