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
    
    
    enum EDITOR_STATES {
        EDITOR_STATE_MAIN,
        EDITOR_STATE_LAYOUT,
        EDITOR_STATE_ASB,
        EDITOR_STATE_TEXTURE,
        EDITOR_STATE_ASA,
        EDITOR_STATE_OBJECTS,
        EDITOR_STATE_PATHS,
        EDITOR_STATE_DETAILS,
        EDITOR_STATE_REVIEW,
        EDITOR_STATE_DATA,
        EDITOR_STATE_TOOLS,
        EDITOR_STATE_OPTIONS,
    };
    
    enum EDITOR_SUB_STATES {
        EDITOR_SUB_STATE_NONE,
        EDITOR_SUB_STATE_TEXTURE_VIEW,
    };
    
    enum AREA_EDITOR_PICKER_TYPES {
        AREA_EDITOR_PICKER_AREA,
        AREA_EDITOR_PICKER_SECTOR_TYPE,
        AREA_EDITOR_PICKER_HAZARD,
        AREA_EDITOR_PICKER_MOB_CATEGORY,
        AREA_EDITOR_PICKER_MOB_TYPE,
    };
    
    static const float  DEBUG_TEXT_SCALE;
    static const float  DEF_GRID_INTERVAL;
    static const float  DOUBLE_CLICK_TIMEOUT;
    static const size_t MAX_TEXTURE_SUGGESTIONS;
    static const float  SELECTION_EFFECT_SPEED;
    static const float  ZOOM_MAX_LEVEL_EDITOR;
    static const float  ZOOM_MIN_LEVEL_EDITOR;
    
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
    lafi::frame* frm_asb;
    lafi::frame* frm_texture;
    lafi::frame* frm_asa;
    lafi::frame* frm_objects;
    lafi::frame* frm_object;
    lafi::frame* frm_paths;
    lafi::frame* frm_details;
    lafi::frame* frm_shadow;
    lafi::frame* frm_review;
    lafi::frame* frm_data;
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
    //Only preview the path when this time is up.
    timer path_preview_timer;
    //Currently selected edges.
    unordered_set<edge*> selected_edges;
    //Currently selected sectors.
    unordered_set<sector*> selected_sectors;
    //Currently selected vertexes.
    unordered_set<vertex*> selected_vertexes;
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
    //List of texture suggestions.
    vector<texture_suggestion> texture_suggestions;
    
    void clear_current_area();
    void create_new_from_picker(const string &name);
    void delete_current_hazard();
    void draw_debug_text(
        const ALLEGRO_COLOR color, const point &where, const string &text
    );
    edge* get_edge_under_mouse();
    sector* get_lone_selected_sector();
    sector* get_sector_under_mouse();
    vertex* get_vertex_under_mouse();
    void load_area(const bool from_backup);
    void open_picker(const unsigned char type);
    void populate_texture_suggestions();
    void pick(const string &name, const unsigned char type);
    void select_different_hazard(const bool next);
    void update_texture_suggestions(const string &n);
    void zoom(const float new_zoom);
    
    //Input handler functions.
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
    void data_to_gui();
    void details_to_gui();
    void gui_to_asa();
    void gui_to_asb();
    void gui_to_sector();
    void hide_all_frames();
    void object_to_gui();
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
