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

#include "../core/misc_structs.h"
#include "../lib/imgui/imgui.h"
#include "../util/general_utils.h"
#include "game_state.h"


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
extern const float MOUSE_COORDS_TEXT_WIDTH;
extern const float OP_ERROR_CURSOR_SHAKE_SPEED;
extern const float OP_ERROR_CURSOR_SHAKE_WIDTH;
extern const float OP_ERROR_CURSOR_SIZE;
extern const float OP_ERROR_CURSOR_THICKNESS;
extern const float OP_ERROR_FLASH_DURATION;
extern const float PICKER_IMG_BUTTON_SIZE;
extern const float STATUS_BAR_HEIGHT;
extern const float TW_DEF_SIZE;
extern const float TW_HANDLE_RADIUS;
extern const float TW_OUTLINE_THICKNESS;
extern const float TW_ROTATION_HANDLE_THICKNESS;
}


/**
 * @brief Info about an editor. This contains data and functions common
 * to all editors in Pikifen.
 */
class Editor : public GameState {

public:

    //--- Function declarations ---
    
    Editor();
    virtual ~Editor() = default;
    void doDrawing() override = 0;
    void doLogic() override = 0;
    void handleAllegroEvent(ALLEGRO_EVENT &ev) override;
    void load() override;
    void unload() override;
    virtual void updateStyle();
    void updateTransformations() override;
    string getName() const override = 0;
    virtual size_t getHistorySize() const;
    
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
    struct TransformationWidget {
    
        public:
        
        //--- Function declarations ---
        
        void draw(
            const Point* const center, const Point* const size,
            const float* const angle, float zoom = 1.0f
        ) const;
        bool handleMouseDown(
            const Point &mouse_coords, const Point* const center,
            const Point* const size, const float* const angle,
            float zoom = 1.0f
        );
        bool handleMouseMove(
            const Point &mouse_coords, Point* center, Point* size, float* angle,
            float zoom = 1.0f,
            bool keep_aspect_ratio = false,
            bool keep_area = false,
            float min_size = -FLT_MAX,
            bool lock_center = true
        );
        bool handleMouseUp();
        bool isMovingCenterHandle();
        bool isMovingHandle();
        Point getOldCenter() const;
        
        private:
        
        //--- Members ---
        
        //What handle is being moved. -1 for none. 9 for the rotation handle.
        signed char moving_handle = -1;
        
        //Old center, before the user started dragging handles.
        Point old_center;
        
        //Old size, before the user started dragging handles.
        Point old_size;
        
        //Old angle, before the user started dragging handles.
        float old_angle = 0.0f;
        
        //Before rotation began, the mouse made this angle with the center.
        float old_mouse_angle = 0.0f;
        
        
        //--- Function declarations ---
        
        void getLocations(
            const Point* const center, const Point* const size,
            const float* const angle, Point* points, float* radius,
            ALLEGRO_TRANSFORM* out_transform
        ) const;
        
    };
    
    /**
     * @brief Info about a dialog box.
     */
    class Dialog {
    
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
        Point custom_pos = Point(-1.0f);
        
        //Custom dialog size. -1,-1 for default.
        Point custom_size = Point(-1.0f);
        
        
        //--- Function declarations ---
        
        void process();
        
    };
    
    /**
     * @brief An item of a picker dialog.
     */
    struct PickerItem {
    
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
        
        explicit PickerItem(
            const string &name,
            const string &top_category = "", const string &second_category = "",
            void* info = nullptr, const string &tooltip = "",
            ALLEGRO_BITMAP* bitmap = nullptr
        );
        
    };
    
    /**
     * @brief Info about a picker dialog.
     */
    class Picker {
    
    public:
    
        //--- Members ---
        
        //List of picker dialog items to choose from.
        vector<PickerItem> items;
        
        //Callback for when the user picks an item from the picker dialog.
        std::function<void(
            const string &, const string &, const string &, void*, bool
        )> pick_callback = nullptr;
        
        //Text to display above the picker dialog list.
        string list_header;
        
        //Can the user make a new item in the picker dialog?
        bool can_make_new = false;
        
        //Use the monospace font for items?
        bool use_monospace = false;
        
        //When making a new item, the user must pick between these
        //top-level category choices, if applicable.
        vector<string> new_item_top_cat_choices;
        
        //Only show picker dialog items matching this filter.
        string filter;
        
        //If there's an associated dialog meant to auto-close, specify it here.
        Dialog* dialog_ptr = nullptr;
        
        //Do we need to focus on the filter text box?
        bool needs_filter_box_focus = true;
        
        //--- Function declarations ---
        
        explicit Picker(Editor* editor_ptr);
        void process();
        
    private:
    
        //--- Members ---
        
        //Pointer to the editor that's using it.
        Editor* editor_ptr = nullptr;
        
        //Top-level category the user picked for the new item, if applicable.
        string new_item_top_cat;
        
        //Second-level category the user picked for the new item, if applicable.
        string new_item_sec_cat;
        
    };
    
    /**
     * @brief Manages the user's changes and everything surrounding it.
     */
    struct ChangesManager {
    
        public:
        
        //--- Function declarations ---
        
        explicit ChangesManager(Editor* ed);
        bool askIfUnsaved(
            const Point &pos,
            const string &action_long, const string &action_short,
            const std::function<void()> &action_callback,
            const std::function<bool()> &save_callback
        );
        bool existsOnDisk() const;
        size_t getUnsavedChanges() const;
        float getUnsavedTimeDelta() const;
        const string &getUnsavedWarningActionLong() const;
        const string &getUnsavedWarningActionShort() const;
        const std::function<void()> &
        getUnsavedWarningActionCallback() const;
        const std::function<bool()> &
        getUnsavedWarningSaveCallback() const;
        bool hasUnsavedChanges();
        void markAsChanged();
        void markAsNonExistent();
        void markAsSaved();
        void reset();
        
        
        private:
        
        //--- Members ---
        
        //Editor it belongs to.
        Editor* ed = nullptr;
        
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
    struct Command {
    
        public:
        
        //--- Function declarations ---
        
        Command(command_func_t f, const string &n);
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
    Picker bitmap_dialog_picker = Picker(this);
    
    //Recommended folder in the bitmap dialog, if any. "." for graphics root.
    string bitmap_dialog_recommended_folder;
    
    //Bitmap with all of the editor icons.
    ALLEGRO_BITMAP* bmp_editor_icons = nullptr;
    
    //Top-left corner of the canvas.
    Point canvas_tl;
    
    //Bottom right corner of the canvas.
    Point canvas_br;
    
    //X coordinate of the canvas GUI separator. -1 = undefined.
    int canvas_separator_x = -1;
    
    //Manager of (unsaved) changes.
    ChangesManager changes_mgr;
    
    //List of registered commands.
    vector<Command> commands;
    
    //Maps a custom mob category name to an index of the types' vector.
    map<string, size_t> custom_cat_name_idxs;
    
    //What mob types belong in what custom mob category names.
    vector<vector<MobType*>> custom_cat_types;
    
    //Currently open dialogs, if any.
    vector<Dialog*> dialogs;
    
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
    Point last_mouse_click_pos;
    
    //Editor sub-state during the last mouse click.
    size_t last_mouse_click_sub_state = INVALID;
    
    //Was the last user input a keyboard press?
    bool last_input_was_keyboard = false;
    
    //Manifest for the current content.
    ContentManifest manifest;
    
    //Message text in the Dear ImGui help dialog.
    string help_dialog_message;
    
    //Page to open in the help dialog.
    string help_dialog_page;
    
    //Message text in the Dear ImGui message dialog.
    string message_dialog_message;
    
    //Is this a real mouse drag, or just a shaky click?
    bool mouse_drag_confirmed = false;
    
    //Starting coordinates of a raw mouse drag.
    Point mouse_drag_start;
    
    //Do we need to focus on the input popup's text widget?
    bool needs_input_popup_text_focus = true;
    
    //Do we need to focus on the new pack's name text widget?
    bool needs_new_pack_text_focus = true;
    
    //Index of the selected pack in the "new" content dialog.
    int new_content_dialog_pack_idx = 0;
    
    //Time left in the operation error red flash effect.
    Timer op_error_flash_timer = Timer(EDITOR::OP_ERROR_FLASH_DURATION);
    
    //Position of the operation error red flash effect.
    Point op_error_pos;
    
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
    
    void centerCamera(
        const Point &min_coords, const Point &max_coords,
        bool instantaneous = false
    );
    void closeTopDialog();
    void doLogicPost();
    void doLogicPre();
    void drawGrid(
        float interval,
        const ALLEGRO_COLOR &major_color, const ALLEGRO_COLOR &minor_color
    );
    void drawOpErrorCursor();
    Point getLastWidgetPost();
    bool guiNeedsKeyboard();
    bool keyCheck(
        int pressed_key, int match_key,
        bool needs_ctrl = false, bool needs_shift = false
    );
    bool isInternalNameGood(const string &name) const;
    bool listPopup(
        const char* label, const vector<string> &items, string* picked_item,
        bool use_monospace = false
    );
    bool listPopup(
        const char* label, const vector<string> &items, int* picked_item_idx,
        bool use_monospace = false
    );
    void leave();
    void loadCustomMobCatTypes(bool is_area_editor);
    void openBaseContentWarningDialog(
        const std::function<void()> &do_pick_callback
    );
    void openBitmapDialog(
        std::function<void(const string &)> ok_callback,
        const string &recommended_folder
    );
    void openDialog(
        const string &title,
        const std::function<void()> &process_callback
    );
    void openHelpDialog(
        const string &message, const string &page
    );
    void openInputPopup(const char* label);
    void openMessageDialog(
        const string &title, const string &message,
        const std::function<void()> &ok_callback = nullptr
    );
    void openNewPackDialog();
    void openPickerDialog(
        const string &title,
        const vector<PickerItem> &items,
        const std::function<void(
            const string &, const string &, const string &, void*, bool
        )> &pick_callback,
        const string &list_header = "",
        bool can_make_new = false,
        bool use_monospace = false,
        const string &filter = ""
    );
    bool popup(const char* label, ImGuiWindowFlags flags = 0);
    void processDialogs();
    void processGuiBaseContentWarningDialog();
    void processGuiBitmapDialog();
    void processGuiEditorStyle();
    bool processGuiHazardManagementWidgets(
        vector<string> &hazard_inames, int &selected_hazard_idx
    );
    void processGuiHelpDialog();
    void processGuiHistory(
        const vector<pair<string, string> > &history,
        const std::function<string(const string &)> &name_display_callback,
        const std::function<void(const string &)> &pick_callback,
        const std::function<string(const string &)> &tooltip_callback
    );
    bool processGuiInputPopup(
        const char* label, const char* prompt, string* text,
        bool use_monospace = false
    );
    void processGuiMessageDialog();
    bool processGuiMobTypeWidgets(
        string* custom_cat_name, MobType** type, const string &pack_filter = ""
    );
    bool processGuiNewDialogPackWidgets(string* pack);
    void processGuiNewPackDialog();
    bool processGuiSizeWidgets(
        const char* label, Point &size, float v_speed,
        bool keep_aspect_ratio, bool keep_area,
        float min_size
    );
    void processGuiStatusBarText();
    void processGuiUnsavedChangesDialog();
    void panelTitle(const char* title);
    
    void keyframeVisualizer(
        KeyframeInterpolator<ALLEGRO_COLOR> &interpolator,
        size_t sel_keyframe_idx
    );
    void keyframeVisualizer(
        KeyframeInterpolator<float> &interpolator,
        size_t sel_keyframe_idx
    );
    void keyframeVisualizer(
        KeyframeInterpolator<Point> &interpolator,
        size_t sel_keyframe_idx
    );
    template <class InterT>
    bool keyframeOrganizer(
        const string &button_id,
        KeyframeInterpolator<InterT> &interpolator,
        size_t &sel_keyframe_idx
    );
    bool keyframeEditor(
        const string &label,
        KeyframeInterpolator<float> &interpolator,
        size_t &sel_keyframe_idx
    );
    bool keyframeEditor(
        const string &label,
        KeyframeInterpolator<ALLEGRO_COLOR> &interpolator,
        size_t &sel_keyframe_idx
    );
    bool keyframeEditor(
        const string &label,
        KeyframeInterpolator<Point> &interpolator,
        size_t &sel_keyframe_idx
    );
    
    bool saveableTreeNode(const string &category, const string &label);
    void setStatus(const string &text = "", bool error = false);
    void setTooltip(
        const string &explanation, const string &shortcut = "",
        const WIDGET_EXPLANATION widget_explanation = WIDGET_EXPLANATION_NONE
    );
    Point snapPointToAxis(const Point &p, const Point &anchor);
    Point snapPointToGrid(const Point &p, float grid_interval);
    void updateHistory(
        vector<pair<string, string> > &history,
        const ContentManifest &manifest, const string &name
    );
    void zoomWithCursor(float new_zoom);
    virtual void handleKeyCharAnywhere(const ALLEGRO_EVENT &ev);
    virtual void handleKeyCharCanvas(const ALLEGRO_EVENT &ev);
    virtual void handleKeyDownAnywhere(const ALLEGRO_EVENT &ev);
    virtual void handleKeyDownCanvas(const ALLEGRO_EVENT &ev);
    virtual void handleKeyUpAnywhere(const ALLEGRO_EVENT &ev);
    virtual void handleKeyUpCanvas(const ALLEGRO_EVENT &ev);
    virtual void handleLmbDoubleClick(const ALLEGRO_EVENT &ev);
    virtual void handleLmbDown(const ALLEGRO_EVENT &ev);
    virtual void handleLmbDrag(const ALLEGRO_EVENT &ev);
    virtual void handleLmbUp(const ALLEGRO_EVENT &ev);
    virtual void handleMmbDoubleClick(const ALLEGRO_EVENT &ev);
    virtual void handleMmbDown(const ALLEGRO_EVENT &ev);
    virtual void handleMmbDrag(const ALLEGRO_EVENT &ev);
    virtual void handleMmbUp(const ALLEGRO_EVENT &ev);
    virtual void handleMouseUpdate(const ALLEGRO_EVENT &ev);
    virtual void handleMouseWheel(const ALLEGRO_EVENT &ev);
    virtual void handleRmbDoubleClick(const ALLEGRO_EVENT &ev);
    virtual void handleRmbDown(const ALLEGRO_EVENT &ev);
    virtual void handleRmbDrag(const ALLEGRO_EVENT &ev);
    virtual void handleRmbUp(const ALLEGRO_EVENT &ev);
};
