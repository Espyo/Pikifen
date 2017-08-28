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
    };
    
    enum EDITOR_SUB_STATES {
        EDITOR_SUB_STATE_NONE,
    };
    
    
    //GUI frames.
    lafi::frame* frm_main;
    lafi::frame* frm_area;
    
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
    
    void clear_current_area();
    
    void hide_all_frames();
    void change_to_right_frame();
    void create_new_from_picker(const string &name);
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
