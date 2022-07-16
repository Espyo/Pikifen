/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general editor-related functions.
 */

#ifndef EDITOR_INCLUDED
#define EDITOR_INCLUDED

#include <string>
#include <vector>

#include "game_state.h"
#include "../imgui/imgui.h"


using std::map;
using std::string;
using std::vector;


namespace EDITOR {
extern const size_t DEF_MAX_HISTORY_SIZE;
extern const float DOUBLE_CLICK_TIMEOUT;
extern const int ICON_BMP_PADDING;
extern const int ICON_BMP_SIZE;
extern const float KEYBOARD_CAM_ZOOM;
extern const float PICKER_IMG_BUTTON_MAX_SIZE;
extern const float PICKER_IMG_BUTTON_MIN_SIZE;
extern const float TW_DEF_SIZE;
extern const float TW_HANDLE_RADIUS;
extern const float TW_OUTLINE_THICKNESS;
extern const float TW_ROTATION_HANDLE_THICKNESS;
extern const float UNSAVED_CHANGES_WARNING_DURATION;
extern const int UNSAVED_CHANGES_WARNING_HEIGHT;
extern const int UNSAVED_CHANGES_WARNING_SPIKE_SIZE;
extern const int UNSAVED_CHANGES_WARNING_WIDTH;
}


/* ----------------------------------------------------------------------------
 * Information about an editor. This contains data and functions common
 * to all editors in Pikifen.
 */
class editor : public game_state {
public:

    //History for the last content entries that were opened.
    vector<string> history;
    
    editor();
    virtual ~editor() = default;
    
    virtual void do_drawing() override = 0;
    virtual void do_logic() override = 0;
    virtual void handle_allegro_event(ALLEGRO_EVENT &ev) override;
    virtual void load() override;
    virtual void unload() override;
    virtual void update_style();
    virtual void update_transformations() override;
    virtual string get_name() const override = 0;
    
    virtual string get_history_option_prefix() const = 0;
    virtual size_t get_history_size() const;
    
protected:

    //Editor icons.
    enum EDITOR_ICONS {
        //Save.
        ICON_SAVE,
        //Save.
        ICON_SAVE_UNSAVED,
        //Load.
        ICON_LOAD,
        //Quit.
        ICON_QUIT,
        //Toggle hitboxes.
        ICON_HITBOXES,
        //Toggle reference image.
        ICON_REFERENCE,
        //Info.
        ICON_INFO,
        //Gameplay.
        ICON_GAMEPLAY,
        //Resize.
        ICON_RESIZE,
        //Play/pause.
        ICON_PLAY_PAUSE,
        //Next.
        ICON_NEXT,
        //Previous.
        ICON_PREVIOUS,
        //Add.
        ICON_ADD,
        //Remove.
        ICON_REMOVE,
        //Move to the right.
        ICON_MOVE_RIGHT,
        //Move to the left.
        ICON_MOVE_LEFT,
        //Select none.
        ICON_SELECT_NONE,
        //Duplicate.
        ICON_DUPLICATE,
        //Add a stop.
        ICON_ADD_STOP,
        //Add a link.
        ICON_ADD_LINK,
        //Add a one-way link.
        ICON_ADD_1W_LINK,
        //Remove a stop.
        ICON_REMOVE_STOP,
        //Remove a link.
        ICON_REMOVE_LINK,
        //Add a circular sector.
        ICON_ADD_CIRCLE_SECTOR,
        //Vertexes.
        ICON_VERTEXES,
        //Edges.
        ICON_EDGES,
        //Sectors.
        ICON_SECTORS,
        //Mobs.
        ICON_MOBS,
        //Paths.
        ICON_PATHS,
        //Details.
        ICON_DETAILS,
        //Review.
        ICON_REVIEW,
        //Tools.
        ICON_TOOLS,
        //Options.
        ICON_OPTIONS,
        //Undo.
        ICON_UNDO,
        //Grid.
        ICON_GRID,
        //Mob radius.
        ICON_MOB_RADIUS,
        //Pikmin silhouette.
        ICON_PIKMIN_SILHOUETTE,
        //Animations.
        ICON_ANIMATIONS,
        //Sprites.
        ICON_SPRITES,
        //Body parts.
        ICON_BODY_PARTS,
        //Play.
        ICON_PLAY,
        //Snap to grid.
        ICON_SNAP_GRID,
        //Snap to vertexes.
        ICON_SNAP_VERTEXES,
        //Snap to edges.
        ICON_SNAP_EDGES,
        //Snap to nothing.
        ICON_SNAP_NOTHING,
        //Search.
        ICON_SEARCH,
        
        //Total amount of editor icons.
        N_EDITOR_ICONS
    };
    
    //Types of explanations for widgets that need them.
    enum WIDGET_EXPLANATIONS {
        //None.
        WIDGET_EXPLANATION_NONE,
        //Drag widget.
        WIDGET_EXPLANATION_DRAG,
        //Slider widget.
        WIDGET_EXPLANATION_SLIDER,
    };
    
    
    /*
     * A widget that's drawn on-screen, and with handles that the user
     * can drag in order to translate, scale, or rotate something.
     * The transformation properties are not tied to anything, and are
     * meant to be fed into the widget's functions so it can edit them.
     */
    struct transformation_widget {
    public:
        void draw(
            const point* const center, const point* const size,
            const float* const angle, const float zoom = 1.0f
        ) const;
        bool handle_mouse_down(
            const point &mouse_coords, const point* const center,
            const point* const size, const float* const angle,
            const float zoom = 1.0f
        );
        bool handle_mouse_move(
            const point &mouse_coords, point* center, point* size, float* angle,
            const float zoom = 1.0f,
            const bool keep_aspect_ratio = false,
            const float min_size = -FLT_MAX
        );
        bool handle_mouse_up();
        bool is_moving_handle();
        
        transformation_widget();
    private:
        //What handle is being moved. -1 for none. 9 for the rotation handle.
        signed char moving_handle;
        //Old size, before the user started dragging handles.
        point old_size;
        //Old angle, before the user started dragging handles.
        float old_angle;
        //Before rotation began, the mouse made this angle with the center.
        float old_mouse_angle;
        
        void get_locations(
            const point* const center, const point* const size,
            const float* const angle, point* points, float* radius,
            ALLEGRO_TRANSFORM* transform
        ) const;
    };
    
    class dialog_info {
    public:
        //Callback for when it's time to process the dialog's contents.
        std::function<void()> process_callback;
        //Callback for when the user closes the dialog, if any.
        std::function<void()> close_callback;
        //Title to display on the dialog.
        string title;
        //Is it open?
        bool is_open;
        //Custom dialog size.
        point custom_size;
        
        dialog_info();
        void process();
    };
    
    //An item of a picker dialog.
    struct picker_item {
        //Its name.
        string name;
        //What category it belongs to, or empty string for none.
        string category;
        //Bitmap, if any.
        ALLEGRO_BITMAP* bitmap;
        
        explicit picker_item(
            const string &name,
            const string &category = "", ALLEGRO_BITMAP* bitmap = nullptr
        );
    };
    
    class picker_info {
    private:
        //Pointer to the editor that's using it.
        editor* editor_ptr;
        //Category the user picked for the new item, if applicable.
        string new_item_category;
        //Do we need to focus on the filter text box?
        bool needs_filter_box_focus;
    public:
        //List of picker dialog items to choose from.
        vector<picker_item> items;
        //Callback for when the user picks an item from the picker dialog.
        std::function<void(
            const string &, const string &, const bool
        )> pick_callback;
        //Text to display above the picker dialog list.
        string list_header;
        //Can the user make a new item in the picker dialog?
        bool can_make_new;
        //When making a new item, the user must pick between these, if any.
        vector<string> new_item_category_choices;
        //Only show picker dialog items matching this filter.
        string filter;
        //If there's an associated dialog meant to auto-close, specify it here.
        dialog_info* dialog_ptr;
        
        explicit picker_info(editor* editor_ptr);
        void process();
    };
    
    
    //Bitmap with all of the editor icons.
    ALLEGRO_BITMAP* bmp_editor_icons;
    //Top-left corner of the canvas.
    point canvas_tl;
    //Bottom right corner of the canvas.
    point canvas_br;
    //X coordinate of the canvas GUI separator. -1 = undefined.
    int canvas_separator_x;
    //Currently open dialogs, if any.
    vector<dialog_info*> dialogs;
    //If the next click is within this time, it's a double-click.
    float double_click_time;
    //List of every individual editor icon.
    vector<ALLEGRO_BITMAP*> editor_icons;
    //Is the Ctrl key currently pressed down?
    bool is_ctrl_pressed;
    //Is the GUI currently in focus? False if it's the canvas.
    bool is_gui_focused;
    //Is the left mouse button currently pressed down?
    bool is_m1_pressed;
    //Is the right mouse button currently pressed down?
    bool is_m2_pressed;
    //Is the middle mouse button currently pressed down?
    bool is_m3_pressed;
    //Is the mouse currently hovering the gui? False if it's the canvas.
    bool is_mouse_in_gui;
    //Is the Shift key currently pressed down?
    bool is_shift_pressed;
    //Number of the mouse button pressed.
    size_t last_mouse_click;
    //Location of the cursor on the last mouse button press.
    point last_mouse_click_pos;
    //Editor sub-state during the last mouse click.
    size_t last_mouse_click_sub_state;
    //Was the last user input a keyboard press?
    bool last_input_was_keyboard;
    //Has the user picked any content to load yet?
    bool loaded_content_yet;
    //Has the user made any unsaved changes yet?
    bool has_unsaved_changes;
    //Is this a real mouse drag, or just a shaky click?
    bool mouse_drag_confirmed;
    //Starting coordinates of a raw mouse drag.
    point mouse_drag_start;
    //Current state.
    size_t state;
    //Status bar text.
    string status_text;
    //Current sub-state.
    size_t sub_state;
    //When placing the unsaved changes warning, focus on these coordinates.
    point unsaved_changes_warning_pos;
    //Time left for the unsaved changes warning to be on-screen.
    timer unsaved_changes_warning_timer;
    //Has the user been warned about having unsaved changes yet?
    bool was_warned_about_unsaved_changes;
    //Maximum zoom level allowed.
    float zoom_max_level;
    //Minimum zoom level allowed.
    float zoom_min_level;
    
    
    //Standard functions.
    void center_camera(
        const point &min_coords, const point &max_coords
    );
    bool check_new_unsaved_changes(const point &pos = point());
    void close_top_dialog();
    void do_logic_post();
    void do_logic_pre();
    void draw_grid(
        const float interval,
        const ALLEGRO_COLOR &major_color, const ALLEGRO_COLOR &minor_color
    );
    void draw_unsaved_changes_warning();
    point get_last_widget_pos();
    bool key_check(
        const int pressed_key, const int match_key,
        const bool needs_ctrl = false, const bool needs_shift = false
    );
    bool input_popup(
        const char* label, const char* prompt, string* text
    );
    bool list_popup(
        const char* label, const vector<string> &items, string* picked_item
    );
    void leave();
    void mark_new_changes();
    void open_dialog(
        const string &title,
        const std::function<void()> &process_callback
    );
    void open_picker_dialog(
        const string &title,
        const vector<picker_item> &items,
        const std::function<void(
            const string &, const string &, const bool
        )> &pick_callback,
        const string &list_header = "",
        const bool can_make_new = false,
        const string &filter = ""
    );
    void process_dialogs();
    void process_gui_editor_style();
    void process_gui_history(
        const std::function<string(const string &)> &name_display_callback,
        const std::function<void(const string &)> &pick_callback
    );
    void process_gui_mob_type_widgets(
        mob_category** cat, mob_type** typ,
        const bool only_show_area_editor_types,
        const std::function<void()> &category_change_callback = nullptr,
        const std::function<void()> &type_change_callback = nullptr
    );
    bool process_gui_size_widgets(
        const char* label, point &size, const float v_speed,
        const bool keep_aspect_ratio, const float min_size,
        const std::function<void()> &pre_change_callback = nullptr
    );
    void panel_title(const char* title, const float width);
    bool saveable_tree_node(const string &category, const string &label);
    void set_tooltip(
        const string &explanation, const string &shortcut = "",
        const WIDGET_EXPLANATIONS widget_explanation = WIDGET_EXPLANATION_NONE
    );
    void update_history(const string &n);
    void zoom_with_cursor(const float new_zoom);
    
    
    //Input handler functions.
    virtual void handle_key_char_anywhere(const ALLEGRO_EVENT &ev);
    virtual void handle_key_char_canvas(const ALLEGRO_EVENT &ev);
    virtual void handle_key_down_anywhere(const ALLEGRO_EVENT &ev);
    virtual void handle_key_down_canvas(const ALLEGRO_EVENT &ev);
    virtual void handle_key_up_anywhere(const ALLEGRO_EVENT &ev);
    virtual void handle_key_up_canvas(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_double_click(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_down(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_drag(const ALLEGRO_EVENT &ev);
    virtual void handle_lmb_up(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_double_click(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_down(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_drag(const ALLEGRO_EVENT &ev);
    virtual void handle_mmb_up(const ALLEGRO_EVENT &ev);
    virtual void handle_mouse_update(const ALLEGRO_EVENT &ev);
    virtual void handle_mouse_wheel(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_double_click(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_down(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_drag(const ALLEGRO_EVENT &ev);
    virtual void handle_rmb_up(const ALLEGRO_EVENT &ev);
    
};


#endif //ifndef EDITOR_INCLUDED
