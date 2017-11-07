/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general area editor-related functions.
 */

#ifndef AREA_EDITOR_INCLUDED
#define AREA_EDITOR_INCLUDED

#include "editor.h"
#include "../LAFI/frame.h"
#include "../LAFI/label.h"

using namespace std;

class area_editor : public editor {
private:
    struct texture_suggestion {
        ALLEGRO_BITMAP* bmp;
        string name;
        texture_suggestion(const string &n);
        void destroy();
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
        EDITOR_STATE_OPTIONS,
    };
    
    enum EDITOR_SUB_STATES {
        EDITOR_SUB_STATE_NONE,
        EDITOR_SUB_STATE_DRAWING,
        EDITOR_SUB_STATE_TEXTURE_VIEW,
    };
    
    enum AREA_EDITOR_PICKER_TYPES {
        AREA_EDITOR_PICKER_AREA,
        AREA_EDITOR_PICKER_SECTOR_TYPE,
        AREA_EDITOR_PICKER_HAZARD,
        AREA_EDITOR_PICKER_MOB_CATEGORY,
        AREA_EDITOR_PICKER_MOB_TYPE,
    };
    
    enum DRAWING_LINE_ERRORS {
        DRAWING_LINE_NO_ERROR,
        DRAWING_LINE_WAYWARD_SECTOR,
        DRAWING_LINE_CROSSES_EDGES,
        DRAWING_LINE_CROSSES_DRAWING,
        DRAWING_LINE_LEAVES_GAP,
    };
    
    static const float         DEBUG_TEXT_SCALE;
    static const float         DEF_GRID_INTERVAL;
    static const float         DOUBLE_CLICK_TIMEOUT;
    static const float         DRAWING_LINE_ERROR_TINT_DURATION;
    static const float         KEYBOARD_CAM_ZOOM;
    static const size_t        MAX_TEXTURE_SUGGESTIONS;
    static const float         MOUSE_DRAG_CONFIRM_RANGE;
    static const float         PATH_LINK_THICKNESS;
    static const float         PATH_STOP_RADIUS;
    static const unsigned char SELECTION_COLOR[3];
    static const float         SELECTION_EFFECT_SPEED;
    static const float         STATUS_OVERRIDE_DURATION;
    static const float         VERTEX_MERGE_RADIUS;
    static const float         ZOOM_MAX_LEVEL_EDITOR;
    static const float         ZOOM_MIN_LEVEL_EDITOR;
    
    static const string EDITOR_ICONS_FOLDER_NAME;
    static const string ICON_DELETE;
    static const string ICON_DELETE_LINK;
    static const string ICON_DELETE_STOP;
    static const string ICON_DUPLICATE;
    static const string ICON_EXIT;
    static const string ICON_NEW;
    static const string ICON_NEW_1WAY_LINK;
    static const string ICON_NEW_CIRCLE_SECTOR;
    static const string ICON_NEW_LINK;
    static const string ICON_NEW_STOP;
    static const string ICON_NEXT;
    static const string ICON_OPTIONS;
    static const string ICON_PREVIOUS;
    static const string ICON_REFERENCE;
    static const string ICON_SAVE;
    
    
    //GUI widgets.
    lafi::frame* frm_main;
    lafi::frame* frm_area;
    lafi::frame* frm_layout;
    lafi::frame* frm_sector;
    lafi::frame* frm_sector_multi;
    lafi::frame* frm_asb;
    lafi::frame* frm_texture;
    lafi::frame* frm_asa;
    lafi::frame* frm_mobs;
    lafi::frame* frm_mob;
    lafi::frame* frm_mob_multi;
    lafi::frame* frm_paths;
    lafi::frame* frm_details;
    lafi::frame* frm_shadow;
    lafi::frame* frm_review;
    lafi::frame* frm_info;
    lafi::frame* frm_tools;
    lafi::frame* frm_options;
    lafi::frame* frm_bottom;
    lafi::label* lbl_status_bar;
    
    //Current state.
    size_t state;
    //Current sub-state.
    size_t sub_state;
    
    //Time left until a backup is generated.
    timer backup_timer;
    //Name of the area currently loaded.
    string cur_area_name;
    //When showing a hazard in the list, this is the index of the current one.
    size_t cur_hazard_nr;
    //Debug tool -- show the edge numbers?
    bool debug_edge_nrs;
    //Debug tool -- show the sector numbers?
    bool debug_sector_nrs;
    //Debug tool -- show the triangulation?
    bool debug_triangulation;
    //Debug tool -- show the vertex numbers?
    bool debug_vertex_nrs;
    //If the next click is within this time, it's a double-click.
    float double_click_time;
    //Nodes of the drawing.
    vector<layout_drawing_node> drawing_nodes;
    //List of sectors that the drawing can be connected to.
    set<sector*> drawing_connected_sectors;
    //Reason why the current drawing line is invalid. Use DRAWING_LINE_*.
    unsigned char drawing_line_error;
    //Time left to keep the error-redness of the new line for.
    timer drawing_line_error_tint_timer;
    //Current grid interval.
    float grid_interval;
    //Is the GUI currently what's in focus, i.e. the last thing clicked?
    bool is_gui_focused;
    //Is Ctrl pressed down?
    bool is_ctrl_pressed;
    //Is Shift pressed down?
    bool is_shift_pressed;
    //Number of the mouse button pressed.
    size_t last_mouse_click;
    //Is this a mouse drag, or just a shaky click?
    bool mouse_drag_confirmed;
    //Starting coordinates of a raw mouse drag.
    point mouse_drag_start;
    //Closest vertex to the mouse when moving.
    vertex* move_closest_vertex;
    //Closest vertex was here when the move started (world coords).
    point move_closest_vertex_start_pos;
    //The mouse cursor was here when the move started (world coords).
    point move_mouse_start_pos;
    //Currently moving the selected vertexes, objects, etc.?
    bool moving;
    //Only preview the path when this time is up.
    timer path_preview_timer;
    //Area data before vertex movement.
    area_data pre_move_area_data;
    //Direction (is clockwise?) of each sector before movement.
    map<sector*, bool> pre_move_sector_directions;
    //Position of the selected vertexes before movement.
    map<vertex*, point> pre_move_vertex_coords;
    //Currently selected edges.
    set<edge*> selected_edges;
    //Currently selected mobs.
    set<mob_gen*> selected_mobs;
    //Currently selected path stops.
    set<path_stop*> selected_path_stops;
    //Currently selected sectors.
    set<sector*> selected_sectors;
    //Currently selected vertexes.
    set<vertex*> selected_vertexes;
    //Has the user agreed to homogenize the selection?
    bool selection_homogenized;
    //Is the user currently performing a rectangle box?
    bool selecting;
    //The selection's alpha depends on this value.
    float selection_effect;
    //Point where the selection is currently at.
    point selection_end;
    //Point where the selection started.
    point selection_start;
    //Render the reference image?
    bool show_reference;
    //State the editor was in before entering the options.
    size_t state_before_options;
    //Status bar override text.
    string status_override_text;
    //Time left to show the status bar override text for.
    timer status_override_timer;
    //List of texture suggestions.
    vector<texture_suggestion> texture_suggestions;
    
    bool are_nodes_traversable(
        const layout_drawing_node &n1,
        const layout_drawing_node &n2
    );
    void cancel_layout_drawing();
    void cancel_layout_moving();
    void center_camera(
        const point &min_coords, const point &max_coords
    );
    void check_drawing_line(const point &pos);
    void clear_current_area();
    void clear_selection();
    void create_new_from_picker(const string &name);
    void delete_current_hazard();
    void draw_debug_text(
        const ALLEGRO_COLOR color, const point &where, const string &text
    );
    void emit_status_bar_message(const string &text);
    void finish_layout_drawing();
    void finish_layout_moving();
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
    vector<unsigned char> get_drawing_node_events(
        const layout_drawing_node &n1, const layout_drawing_node &n2
    );
    bool get_drawing_outer_sector(sector** result);
    edge* get_edge_under_point(const point &p, edge* after = NULL);
    vector<pair<edge*, edge*> > get_intersecting_edges();
    float get_mob_gen_radius(mob_gen* m);
    mob_gen* get_mob_under_point(const point &p);
    path_stop* get_path_stop_under_point(const point &p);
    sector* get_sector_under_point(const point &p);
    vertex* get_vertex_under_point(const point &p);
    void handle_line_error();
    void homogenize_selected_mobs();
    void homogenize_selected_sectors();
    void load_area(const bool from_backup);
    void merge_vertex(
        vertex* v1, vertex* v2, unordered_set<sector*>* affected_sectors
    );
    void open_picker(const unsigned char type);
    void populate_texture_suggestions();
    void pick(const string &name, const unsigned char type);
    void select_different_hazard(const bool next);
    void select_edge(edge* e);
    void select_sector(sector* s);
    void select_vertex(vertex* v);
    point snap_to_grid(const point &p);
    vertex* split_edge(edge* e_ptr, const point &where);
    void start_vertex_move();
    void update_sector_texture(sector* s_ptr, const string file_name);
    void update_texture_suggestions(const string &n);
    void zoom(const float new_zoom, const bool anchor_cursor = true);
    
    //Input handler functions.
    void handle_key_char(const ALLEGRO_EVENT &ev);
    void handle_key_down(const ALLEGRO_EVENT &ev);
    void handle_key_up(const ALLEGRO_EVENT &ev);
    void handle_lmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_lmb_down(const ALLEGRO_EVENT &ev);
    void handle_lmb_drag(const ALLEGRO_EVENT &ev);
    void handle_lmb_up(const ALLEGRO_EVENT &ev);
    void handle_mmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_mmb_down(const ALLEGRO_EVENT &ev);
    void handle_mmb_drag(const ALLEGRO_EVENT &ev);
    void handle_mmb_up(const ALLEGRO_EVENT &ev);
    void handle_mouse_update(const ALLEGRO_EVENT &ev);
    void handle_mouse_wheel(const ALLEGRO_EVENT &ev);
    void handle_rmb_double_click(const ALLEGRO_EVENT &ev);
    void handle_rmb_down(const ALLEGRO_EVENT &ev);
    void handle_rmb_drag(const ALLEGRO_EVENT &ev);
    void handle_rmb_up(const ALLEGRO_EVENT &ev);
    
    //GUI functions.
    void asa_to_gui();
    void asb_to_gui();
    void change_to_right_frame();
    void info_to_gui();
    void details_to_gui();
    void gui_to_asa();
    void gui_to_asb();
    void gui_to_mob();
    void gui_to_sector();
    void hide_all_frames();
    void mob_to_gui();
    void path_to_gui();
    void review_to_gui();
    void sector_to_gui();
    void tools_to_gui();
    void update_main_frame();
    
public:
    virtual void do_logic();
    virtual void do_drawing();
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void load();
    virtual void unload();
    virtual void update_transformations();
    
    //TODO do I need these?
    vector<edge_intersection> intersecting_edges;
    unordered_set<sector*>    non_simples;
    unordered_set<edge*>      lone_edges;
    
    
    area_editor();
};



#endif //ifndef AREA_EDITOR_INCLUDED
