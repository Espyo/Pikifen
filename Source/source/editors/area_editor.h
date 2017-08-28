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
        EDITOR_STATE_TOOLS,
        EDITOR_STATE_OPTIONS,
    };
    
    enum EDITOR_SUB_STATES {
        EDITOR_SUB_STATE_NONE,
    };
    
    enum AREA_EDITOR_PICKER_TYPES {
        AREA_EDITOR_PICKER_AREA,
        AREA_EDITOR_PICKER_SECTOR_TYPE,
        AREA_EDITOR_PICKER_MOB_CATEGORY,
        AREA_EDITOR_PICKER_MOB_TYPE,
    };
    
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
    //If the next click is within this time, it's a double-click.
    float double_click_time;
    //Only preview the path when this time is up.
    timer path_preview_timer;
    //Render the reference image?
    bool show_reference;
    //State the editor was in before entering the options.
    size_t state_before_options;
    
    void clear_current_area();
    
    void hide_all_frames();
    void change_to_right_frame();
    void create_new_from_picker(const string &name);
    void handle_mouse_update(const ALLEGRO_EVENT &ev);
    void open_picker(const unsigned char type);
    void pick(const string &name, const unsigned char type);
    
    
public:
    virtual void do_logic();
    virtual void do_drawing();
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void load();
    virtual void unload();
    virtual void update_transformations();
    
    area_editor();
};



#endif //ifndef AREA_EDITOR_INCLUDED
