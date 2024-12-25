/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general editor-related functions.
 */

#pragma once

#include <string>
#include <vector>

#include "game_state.h"
#include "../libs/imgui/imgui.h"
#include "../utils/general_utils.h"
#include "../misc_structs.h"


using std::map;
using std::pair;
using std::string;
using std::vector;


namespace EDITOR {
extern const size_t DEF_MAX_HISTORY_SIZE;
extern const float DOUBLE_CLICK_TIMEOUT;
extern const int ICON_BMP_PADDING;
extern const int ICON_BMP_SIZE;
extern const float KEYBOARD_CAM_ZOOM;
extern const float OP_ERROR_CURSOR_SHAKE_SPEED;
extern const float OP_ERROR_CURSOR_SHAKE_WIDTH;
extern const float OP_ERROR_CURSOR_SIZE;
extern const float OP_ERROR_CURSOR_THICKNESS;
extern const float OP_ERROR_FLASH_DURATION;
extern const float PICKER_IMG_BUTTON_MAX_SIZE;
extern const float PICKER_IMG_BUTTON_MIN_SIZE;
extern const float TW_DEF_SIZE;
extern const float TW_HANDLE_RADIUS;
extern const float TW_OUTLINE_THICKNESS;
extern const float TW_ROTATION_HANDLE_THICKNESS;
}


/**
 * @brief Info about an editor. This contains data and functions common
 * to all editors in Pikifen.
 */
class editor : public game_state {

public:

    //--- Members ---
    
    //History for the last content entries that were opened.
    //Saves pairs of path-name.
    vector<pair<string, string> > history;
    
    
    //--- Function declarations ---
    
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

    //--- Misc. declarations ---
    
    //Editor icons.
    enum EDITOR_ICON {
    
        //Save.
        EDITOR_ICON_SAVE,
        
        //Save.
        EDITOR_ICON_SAVE_UNSAVED,
        
        //Load.
        EDITOR_ICON_LOAD,
        
        //Quit.
        EDITOR_ICON_QUIT,
        
        //Toggle hitboxes.
        EDITOR_ICON_HITBOXES,
        
        //Toggle reference image.
        EDITOR_ICON_REFERENCE,
        
        //Info.
        EDITOR_ICON_INFO,
        
        //Gameplay.
        EDITOR_ICON_GAMEPLAY,
        
        //Resize.
        EDITOR_ICON_RESIZE,
        
        //Play/pause.
        EDITOR_ICON_PLAY_PAUSE,
        
        //Stop.
        EDITOR_ICON_STOP,
        
        //Next.
        EDITOR_ICON_NEXT,
        
        //Previous.
        EDITOR_ICON_PREVIOUS,
        
        //Add.
        EDITOR_ICON_ADD,
        
        //Remove.
        EDITOR_ICON_REMOVE,
        
        //Move to the right.
        EDITOR_ICON_MOVE_RIGHT,
        
        //Move to the left.
        EDITOR_ICON_MOVE_LEFT,
        
        //Select none.
        EDITOR_ICON_SELECT_NONE,
        
        //Duplicate.
        EDITOR_ICON_DUPLICATE,
        
        //Add a stop.
        EDITOR_ICON_ADD_STOP,
        
        //Add a link.
        EDITOR_ICON_ADD_LINK,
        
        //Add a one-way link.
        EDITOR_ICON_ADD_1W_LINK,
        
        //Remove a stop.
        EDITOR_ICON_REMOVE_STOP,
        
        //Remove a link.
        EDITOR_ICON_REMOVE_LINK,
        
        //Add a circular sector.
        EDITOR_ICON_ADD_CIRCLE_SECTOR,
        
        //Vertexes.
        EDITOR_ICON_VERTEXES,
        
        //Edges.
        EDITOR_ICON_EDGES,
        
        //Sectors.
        EDITOR_ICON_SECTORS,
        
        //Mobs.
        EDITOR_ICON_MOBS,
        
        //Paths.
        EDITOR_ICON_PATHS,
        
        //Details.
        EDITOR_ICON_DETAILS,
        
        //Review.
        EDITOR_ICON_REVIEW,
        
        //Tools.
        EDITOR_ICON_TOOLS,
        
        //Options.
        EDITOR_ICON_OPTIONS,
        
        //Undo (and redo, when mirrored).
        EDITOR_ICON_UNDO,
        
        //Grid.
        EDITOR_ICON_GRID,
        
        //Mob radius.
        EDITOR_ICON_MOB_RADIUS,
        
        //Leader silhouette.
        EDITOR_ICON_LEADER_SILHOUETTE,
        
        //Animations.
        EDITOR_ICON_ANIMATIONS,
        
        //Sprites.
        EDITOR_ICON_SPRITES,
        
        //Body parts.
        EDITOR_ICON_BODY_PARTS,
        
        //Play.
        EDITOR_ICON_PLAY,
        
        //Snap to grid.
        EDITOR_ICON_SNAP_GRID,
        
        //Snap to vertexes.
        EDITOR_ICON_SNAP_VERTEXES,
        
        //Snap to edges.
        EDITOR_ICON_SNAP_EDGES,
        
        //Snap to nothing.
        EDITOR_ICON_SNAP_NOTHING,
        
        //Search.
        EDITOR_ICON_SEARCH,
        
        //Total amount of editor icons.
        N_EDITOR_ICONS
        
    };
    
    //Types of explanations for widgets that need them.
    enum WIDGET_EXPLANATION {
    
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
        
        //--- Function declarations ---
        
        void draw(
            const point* const center, const point* const size,
            const float* const angle, float zoom = 1.0f
        ) const;
        bool handle_mouse_down(
            const point &mouse_coords, const point* const center,
            const point* const size, const float* const angle,
            float zoom = 1.0f
        );
        bool handle_mouse_move(
            const point &mouse_coords, point* center, point* size, float* angle,
            float zoom = 1.0f,
            bool keep_aspect_ratio = false,
            bool keep_area = false,
            float min_size = -FLT_MAX,
            bool lock_center = true
        );
        bool handle_mouse_up();
        bool is_moving_center_handle();
        bool is_moving_handle();
        point get_old_center() const;
        
        private:
        
        //--- Members ---
        
        //What handle is being moved. -1 for none. 9 for the rotation handle.
        signed char moving_handle = -1;
        
        //Old center, before the user started dragging handles.
        point old_center;
        
        //Old size, before the user started dragging handles.
        point old_size;
        
        //Old angle, before the user started dragging handles.
        float old_angle = 0.0f;
        
        //Before rotation began, the mouse made this angle with the center.
        float old_mouse_angle = 0.0f;
        
        
        //--- Function declarations ---
        
        void get_locations(
            const point* const center, const point* const size,
            const float* const angle, point* points, float* radius,
            ALLEGRO_TRANSFORM* out_transform
        ) const;
        
    };
    
    /**
     * @brief Info about a dialog box.
     */
    class dialog_info {
    
    public:
    
        //--- Members ---
        
        //Callback for when it's time to process the dialog's contents.
        std::function<void()> process_callback = nullptr;
        
        //Callback for when an Allegro event happens.
        std::function<void(ALLEGRO_EVENT*)> event_callback = nullptr;
        
        //Callback for when the user closes the dialog, if any.
        std::function<void()> close_callback = nullptr;
        
        //Title to display on the dialog.
        string title;
        
        //Is it open?
        bool is_open = true;
        
        //Custom dialog position (center point). -1,-1 for default.
        point custom_pos = point(-1.0f, -1.0f);
        
        //Custom dialog size. -1,-1 for default.
        point custom_size = point(-1.0f, -1.0f);
        
        
        //--- Function declarations ---
        
        void process();
        
    };
    
    /**
     * @brief An item of a picker dialog.
     */
    struct picker_item {
    
        //--- Members ---
        
        //Its name.
        string name;
        
        //What top-level category it belongs to, or empty string for none.
        string top_category;
        
        //What second-level category it belongs to, or empty string for none.
        string sec_category;
        
        //Information to pass to the code when the item is picked, if any.
        void* info;
        
        //Tooltip, if any.
        string tooltip;
        
        //Bitmap, if any.
        ALLEGRO_BITMAP* bitmap = nullptr;
        
        
        //--- Function declarations ---
        
        explicit picker_item(
            const string &name,
            const string &top_category = "", const string &second_category = "",
            void* info = nullptr, const string &tooltip = "",
            ALLEGRO_BITMAP* bitmap = nullptr
        );
        
    };
    
    /**
     * @brief Info about a picker dialog.
     */
    class picker_info {
    
    public:
    
        //--- Members ---
        
        //List of picker dialog items to choose from.
        vector<picker_item> items;
        
        //Callback for when the user picks an item from the picker dialog.
        std::function<void(
            const string &, const string &, const string &, void*, bool
        )> pick_callback = nullptr;
        
        //Text to display above the picker dialog list.
        string list_header;
        
        //Can the user make a new item in the picker dialog?
        bool can_make_new = false;
        
        //When making a new item, the user must pick between these
        //top-level category choices, if applicable.
        vector<string> new_item_top_cat_choices;
        
        //Only show picker dialog items matching this filter.
        string filter;
        
        //If there's an associated dialog meant to auto-close, specify it here.
        dialog_info* dialog_ptr = nullptr;
        
        //--- Function declarations ---
        
        explicit picker_info(editor* editor_ptr);
        void process();
        
    private:
    
        //--- Members ---
        
        //Pointer to the editor that's using it.
        editor* editor_ptr = nullptr;
        
        //Top-level category the user picked for the new item, if applicable.
        string new_item_top_cat;
        
        //Second-level category the user picked for the new item, if applicable.
        string new_item_sec_cat;
        
        //Do we need to focus on the filter text box?
        bool needs_filter_box_focus = true;
        
    };
    
    /**
     * @brief Manages the user's changes and everything surrounding it.
     */
    struct changes_manager {
    
        public:
        
        //--- Function declarations ---
        
        explicit changes_manager(editor* ed);
        bool ask_if_unsaved(
            const point &pos,
            const string &action_long, const string &action_short,
            const std::function<void()> &action_callback,
            const std::function<bool()> &save_callback
        );
        bool exists_on_disk() const;
        size_t get_unsaved_changes() const;
        float get_unsaved_time_delta() const;
        const string &get_unsaved_warning_action_long() const;
        const string &get_unsaved_warning_action_short() const;
        const std::function<void()> &
        get_unsaved_warning_action_callback() const;
        const std::function<bool()> &
        get_unsaved_warning_save_callback() const;
        bool has_unsaved_changes();
        void mark_as_changed();
        void mark_as_non_existent();
        void mark_as_saved();
        void reset();
        
        
        private:
        
        //--- Members ---
        
        //Editor it belongs to.
        editor* ed = nullptr;
        
        //Whether the content exists on the disk.
        bool on_disk = true;
        
        //Cummulative number of unsaved changes since the last save.
        size_t unsaved_changes = 0;
        
        //When did it last go from saved to unsaved? 0 = no unsaved changes.
        float unsaved_time = 0.0f;
        
        //Long name of the action for the open unsaved changes warning dialog.
        string unsaved_warning_action_long;
        
        //Short name of the action for the open unsaved changes warning dialog.
        string unsaved_warning_action_short;
        
        //Action code callback for the open unsaved changes warning dialog.
        std::function<void()> unsaved_warning_action_callback = nullptr;
        
        //Save code callback for the open unsaved changes warning dialog.
        std::function<bool()> unsaved_warning_save_callback = nullptr;
        
    };
    
    
    /**
     * @brief Function executed by some command in the editor.
     *
     * The first parameter is the main value (0 to 1).
     */
    typedef std::function<void(float)> command_func_t;
    
    
    /**
     * @brief Represents one of the editor's possible commands.
     * These are usually triggered by shortcuts.
     */
    struct command {
    
        public:
        
        //--- Function declarations ---
        
        command(command_func_t f, const string &n);
        void run(float input_value);
        
        private:
        
        //--- Members ---
        
        //Function to run.
        command_func_t func = nullptr;
        
        //Name.
        string name;
        
    };
    
    
    //--- Members ---
    
    //Callback for when the item is really meant to be picked, in the base
    //content warning dialog.
    std::function<void()> base_content_warning_do_pick_callback = nullptr;
    
    //Currently-chosen bitmap name in the bitmap dialog.
    string bitmap_dialog_cur_bmp_name;
    
    //Currently-chosen bitmap pointer in the bitmap dialog.
    ALLEGRO_BITMAP* bitmap_dialog_cur_bmp_ptr = nullptr;
    
    //Name of the newly-chosen bitmap in the bitmap dialog.
    string bitmap_dialog_new_bmp_name;
    
    //Callback for when the user chooses a bitmap in the bitmap dialog.
    std::function<void(const string &)> bitmap_dialog_ok_callback = nullptr;
    
    //Picker for the bitmap dialog.
    picker_info bitmap_dialog_picker = picker_info(this);
    
    //Recommended folder in the bitmap dialog, if any. "." for graphics root.
    string bitmap_dialog_recommended_folder;
    
    //Bitmap with all of the editor icons.
    ALLEGRO_BITMAP* bmp_editor_icons = nullptr;
    
    //Top-left corner of the canvas.
    point canvas_tl;
    
    //Bottom right corner of the canvas.
    point canvas_br;
    
    //X coordinate of the canvas GUI separator. -1 = undefined.
    int canvas_separator_x = -1;
    
    //Manager of (unsaved) changes.
    changes_manager changes_mgr;
    
    //List of registered commands.
    vector<command> commands;
    
    //Maps a custom mob category name to an index of the types' vector.
    map<string, size_t> custom_cat_name_idxs;
    
    //What mob types belong in what custom mob category names.
    vector<vector<mob_type*>> custom_cat_types;
    
    //Currently open dialogs, if any.
    vector<dialog_info*> dialogs;
    
    //If the next click is within this time, it's a double-click.
    float double_click_time = 0.0f;
    
    //List of every individual editor icon.
    vector<ALLEGRO_BITMAP*> editor_icons;
    
    //If Escape was pressed the previous frame.
    bool escape_was_pressed = false;
    
    //Is the Alt key currently pressed down?
    bool is_alt_pressed = false;
    
    //Is the Ctrl key currently pressed down?
    bool is_ctrl_pressed = false;
    
    //Is the Shift key currently pressed down?
    bool is_shift_pressed = false;
    
    //Is the left mouse button currently pressed down?
    bool is_m1_pressed = false;
    
    //Is the right mouse button currently pressed down?
    bool is_m2_pressed = false;
    
    //Is the middle mouse button currently pressed down?
    bool is_m3_pressed = false;
    
    //Is the mouse currently hovering the gui? False if it's the canvas.
    bool is_mouse_in_gui = false;
    
    //Number of the mouse button pressed.
    size_t last_mouse_click = INVALID;
    
    //Screen location of the cursor on the last mouse button press.
    point last_mouse_click_pos;
    
    //Editor sub-state during the last mouse click.
    size_t last_mouse_click_sub_state = INVALID;
    
    //Was the last user input a keyboard press?
    bool last_input_was_keyboard = false;
    
    //Manifest for the current content.
    content_manifest manifest;
    
    //Message text in the message dialog.
    string message_dialog_message;
    
    //Is this a real mouse drag, or just a shaky click?
    bool mouse_drag_confirmed = false;
    
    //Starting coordinates of a raw mouse drag.
    point mouse_drag_start;
    
    //Time left in the operation error red flash effect.
    timer op_error_flash_timer = timer(EDITOR::OP_ERROR_FLASH_DURATION);
    
    //Position of the operation error red flash effect.
    point op_error_pos;
    
    //Current state.
    size_t state = 0;
    
    //Status bar text.
    string status_text;
    
    //Current sub-state.
    size_t sub_state = 0;
    
    //Maximum zoom level allowed.
    float zoom_max_level = 0.0f;
    
    //Minimum zoom level allowed.
    float zoom_min_level = 0.0f;
    
    
    //--- Function declarations ---
    
    void center_camera(
        const point &min_coords, const point &max_coords,
        bool instantaneous = false
    );
    void close_top_dialog();
    void do_logic_post();
    void do_logic_pre();
    void draw_grid(
        float interval,
        const ALLEGRO_COLOR &major_color, const ALLEGRO_COLOR &minor_color
    );
    void draw_op_error_cursor();
    point get_last_widget_pos();
    bool key_check(
        int pressed_key, int match_key,
        bool needs_ctrl = false, bool needs_shift = false
    );
    bool input_popup(
        const char* label, const char* prompt, string* text
    );
    bool is_internal_name_good(const string &name) const;
    bool list_popup(
        const char* label, const vector<string> &items, string* picked_item
    );
    void leave();
    void load_custom_mob_cat_types(bool is_area_editor);
    void open_base_content_warning_dialog(
        const std::function<void()> &do_pick_callback
    );
    void open_bitmap_dialog(
        std::function<void(const string &)> ok_callback,
        const string &recommended_folder
    );
    void open_dialog(
        const string &title,
        const std::function<void()> &process_callback
    );
    void open_message_dialog(
        const string &title, const string &message,
        const std::function<void()> &ok_callback
    );
    void open_new_pack_dialog();
    void open_picker_dialog(
        const string &title,
        const vector<picker_item> &items,
        const std::function<void(
            const string &, const string &, const string &, void*, bool
        )> &pick_callback,
        const string &list_header = "",
        bool can_make_new = false,
        const string &filter = ""
    );
    bool popup(const char* label, ImGuiWindowFlags flags = 0);
    void process_dialogs();
    void process_gui_base_content_warning_dialog();
    void process_gui_bitmap_dialog();
    void process_gui_editor_style();
    void process_gui_history(
        const std::function<string(const string &)> &name_display_callback,
        const std::function<void(const string &)> &pick_callback,
        const std::function<string(const string &)> &tooltip_callback
    );
    void process_gui_message_dialog();
    bool process_gui_mob_type_widgets(
        string* custom_cat_name, mob_type** type, const string &pack_filter = ""
    );
    bool process_gui_new_dialog_pack_widgets(string* pack);
    void process_gui_new_pack_dialog();
    bool process_gui_size_widgets(
        const char* label, point &size, float v_speed,
        bool keep_aspect_ratio, bool keep_area,
        float min_size
    );
    void process_gui_status_bar_text();
    void process_gui_unsaved_changes_dialog();
    void panel_title(const char* title);

    void keyframe_visualizer(keyframe_interpolator<ALLEGRO_COLOR> interpolator, size_t selected_index);
    void keyframe_visualizer(keyframe_interpolator<float> interpolator, size_t selected_index);
    void keyframe_visualizer(keyframe_interpolator<point> interpolator, size_t selected_index);
    template <class inter_t>
    void keyframe_navigation(
        const string& label, 
        keyframe_interpolator<inter_t>* interpolator, 
        size_t& selected_index
    );
    void keyframe_editor(
        const string& label, 
        keyframe_interpolator<float>* interpolator, 
        size_t& selected_index
    );
    void keyframe_editor(
        const string& label,
        keyframe_interpolator<ALLEGRO_COLOR>* interpolator,
        size_t& selected_index
    );
    void keyframe_editor(
        const string& label,
        keyframe_interpolator<point>* interpolator,
        size_t& selected_index
    );

    bool saveable_tree_node(const string &category, const string &label);
    void set_status(const string &text = "", bool error = false);
    void set_tooltip(
        const string &explanation, const string &shortcut = "",
        const WIDGET_EXPLANATION widget_explanation = WIDGET_EXPLANATION_NONE
    );
    point snap_point_to_axis(const point &p, const point &anchor);
    point snap_point_to_grid(const point &p, float grid_interval);
    void update_history(
        const content_manifest &manifest, const string &name
    );
    void zoom_with_cursor(float new_zoom);
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
