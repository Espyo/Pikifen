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
extern const ALLEGRO_COLOR GRID_COLOR_MAJOR;
extern const ALLEGRO_COLOR GRID_COLOR_MINOR;
extern const ALLEGRO_COLOR GRID_COLOR_ORIGIN;
extern const int ICON_BMP_PADDING;
extern const int ICON_BMP_SIZE;
extern const float KEYBOARD_CAM_ZOOM;
extern const float MOUSE_COORDS_TEXT_WIDTH;
extern const float OP_ERROR_CURSOR_SHAKE_SPEED;
extern const float OP_ERROR_CURSOR_SHAKE_WIDTH;
extern const float OP_ERROR_CURSOR_SIZE;
extern const float OP_ERROR_CURSOR_THICKNESS;
extern const float OP_ERROR_FLASH_DURATION;
extern const float PICKER_IMG_BUTTON_MIN_SIZE;
extern const float PICKER_IMG_BUTTON_SIZE;
extern const float RUBBER_BAND_TEXTURE_TIME_MULT;
extern const ALLEGRO_COLOR SILHOUETTE_COLOR;
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

    //--- Public function declarations ---
    
    Editor();
    virtual ~Editor() = default;
    void doDrawing() override = 0;
    void doLogic() override = 0;
    void handleAllegroEvent(ALLEGRO_EVENT& ev) override;
    void load() override;
    void unload() override;
    virtual void updateStyle();
    string getName() const override = 0;
    virtual size_t getHistorySize() const;
    
protected:

    //--- Protected misc. declarations ---
    
    //Editor icons.
    enum EDITOR_ICON {
    
        //Save.
        EDITOR_ICON_SAVE,
        
        //Save, but with an unsaved warning mark.
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
        
        //Resize something.
        EDITOR_ICON_RESIZE,
        
        //Play/pause.
        EDITOR_ICON_PLAY_PAUSE,
        
        //Stop.
        EDITOR_ICON_STOP,
        
        //Next in a list.
        EDITOR_ICON_NEXT,
        
        //Previous in a list.
        EDITOR_ICON_PREVIOUS,
        
        //Add or create something.
        EDITOR_ICON_ADD,
        
        //Remove or delete something.
        EDITOR_ICON_REMOVE,
        
        //Move something to the right in a list.
        EDITOR_ICON_MOVE_RIGHT,
        
        //Move something to the left in a list.
        EDITOR_ICON_MOVE_LEFT,
        
        //Select none.
        EDITOR_ICON_SELECT_NONE,
        
        //Duplicate.
        EDITOR_ICON_DUPLICATE,
        
        //Create a path stop.
        EDITOR_ICON_NEW_PATH_STOP,
        
        //Create a path link.
        EDITOR_ICON_NEW_PATH_LINK,
        
        //Create a one-way link.
        EDITOR_ICON_NEW_1W_PATH_LINK,
        
        //Delete a path stop.
        EDITOR_ICON_DELETE_PATH_STOP,
        
        //Delete a path link.
        EDITOR_ICON_DELETE_PATH_LINK,
        
        //Create a circular sector.
        EDITOR_ICON_NEW_CIRCLE_SECTOR,
        
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
        
        //Mission.
        EDITOR_ICON_MISSION,
        
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
    
    /**
     * @brief Wrapper for a list with the indexes of the currently
     * selected items.
     */
    struct SelectionList {
    
        public:
        
        //--- Public function declarations ---
        
        bool add(size_t idx);
        bool addAll(size_t totalAmount);
        bool remove(size_t idx);
        bool clear();
        bool setItemIdxs(const set<size_t>& idxs);
        bool setSingle(size_t idx);
        
        bool contains(size_t idx) const;
        bool hasMultiple() const;
        bool hasOne() const;
        bool hasAny() const;
        size_t getFirstItemIdx() const;
        size_t getSingleItemIdx() const;
        const set<size_t>& getItemIdxs() const;
        size_t getCount() const;
        
        
        private:
        
        //--- Private members ---
        
        //The list proper.
        set<size_t> list;
        
    };
    
    /**
     * @brief Manages the selection of a specific type of item.
     * It only stores item indexes and knows how to manipulate the items'
     * 2D data.
     */
    struct SelectionManager {
    
        public:
        
        //--- Public members ---
        
        //Callback for when the info of an item needs to be retrieved.
        //The second argument is where the center is returned, the third
        //is where the size is returned, the fourth is where the angle is
        //returned.
        //The default center and size values are 0,0.
        std::function<void(size_t, Point*, Point*, float*)>
        onGetInfo = nullptr;
        
        //Callback for when the total amount of all editor items needs to
        //be retrieved.
        //The default total value is 0.
        std::function<size_t()> onGetTotal = nullptr;
        
        //Callback for when whether a given item is eligible to be selected
        //needs to be figured out.
        //The default eligibility is true.
        std::function<bool(size_t)> onIsEligible = nullptr;
        
        //Callback for when the info of an item needs to be set.
        //The second argument is the new center, the third
        //is the new size, the fourth is the new angle.
        std::function<void(size_t, const Point&, const Point&, float)>
        onSetInfo = nullptr;
        
        //Callback for when the selection changed.
        std::function<void()> onSelectionChanged = nullptr;
        
        //Callback for checking if an item is under the cursor, if it should
        //be different from the default checks.
        //The first argument is the item index.
        //The second argument is the cursor position.
        std::function<bool(size_t, const Point&)> onCheckUnderCursor = nullptr;
        
        //Whether items are rectangular in shape or circular.
        //Affects mouse clicking detection.
        bool itemsAreRectangular = true;
        
        //Whether the items can be resized.
        bool itemsCanResize = false;
        
        //Whether the items can rotate.
        bool itemsCanRotate = false;
        
        //Whether to disable the editing the item's translation data.
        bool disableChanges = false;
        
        
        //--- Public function declarations ---
        
        bool add(size_t idx);
        bool addAll(size_t totalAmount);
        bool remove(size_t idx);
        bool clear();
        bool setItemIdxs(const set<size_t>& idxs);
        bool setSingle(size_t idx);
        
        bool contains(size_t idx) const;
        bool hasMultiple() const;
        bool hasOne() const;
        bool hasAny() const;
        size_t getFirstItemIdx() const;
        size_t getSingleItemIdx() const;
        const set<size_t>& getItemIdxs() const;
        size_t getCount() const;
        
        bool enable();
        bool disable();
        
        bool startOperation();
        bool applyDragMove(const Point& cursorPos);
        bool applyTransformation(
            const Point& newCenter, const Point& newSize, float newAngle
        );
        bool setHomogenized(bool homogenized);
        bool isHomogenized() const;
        
        void calculateSelectionPortion(
            const Point& largerPreTransCenter, const Point& largerPreTransSize,
            const Point& largerNewCenter, const Point& largerNewSize,
            Point* outPortionedNewCenter, Point* outPortionedNewSize
        ) const;
        bool getBBox(
            Point* outCenter, Point* outSize,
            Point* outCentersOnlyCenter = nullptr,
            Point* outCentersOnlySize = nullptr
        ) const;
        vector<size_t> getItemsUnderCursor(const Point& cursorPos) const;
        void getItemInfo(
            size_t idx, Point* outCenter, Point* outSize, float* outAngle
        ) const;
        bool getItemIsEligible(size_t idx) const;
        size_t getNrTotalItems() const;
        
        
        private:
        
        //--- Private members ---
        
        //The list of selected items.
        SelectionList selectedItems;
        
        //Whether it is currently enabled.
        bool enabled = false;
        
        //Has the user agreed to homogenize the selection?
        bool homogenized = false;
        
        //Center of each selected item before the latest operation began.
        map<size_t, Point> preOpItemCenters;
        
        //Size of each selected item before the latest operation began.
        map<size_t, Point> preOpItemSizes;
        
        //Center of the entire selection before the latest operation began.
        //Cache for performance.
        Point preOpSelCenter;
        
        //Size of the entire selection before the latest operation began.
        //Cache for performance.
        Point preOpSelSize;
        
        //If the transformation should apply to centers only (i.e. items
        //can't resize), use this selection center.
        Point preOpCentersOnlySelCenter;
        
        //If the transformation should apply to centers only (i.e. items
        //can't resize), use this selection size.
        Point preOpCentersOnlySelSize;
        
    };
    
    /**
     * @brief User controller for controlling selections in a given set of
     * types of items.
     */
    struct SelectionController {
    
        public:
        
        //--- Public misc. declarations ---
        
        //Rules for how and when a given operation can be started.
        enum OP_RULE {
        
            //Always allow it.
            OP_RULE_ALWAYS,
            
            //Allow it if it's only one item.
            OP_RULE_ONE_ITEM,
            
            //Allow it if it's only multiple items.
            OP_RULE_MULTIPLE_ITEMS,
            
            //Cannot drag move.
            OP_RULE_NEVER,
            
        };
        
        
        //--- Public members ---
        
        //List of managers it controls.
        vector<SelectionManager*> managers;
        
        //Callback for when a point should probably be snapped.
        std::function<Point(const Point&)> onSnapPoint = nullptr;
        
        //How and when a transformation widget transformation can be started.
        OP_RULE twTransformRule = OP_RULE_ALWAYS;
        
        //How and when a drag move can be started.
        OP_RULE dragMoveRule = OP_RULE_NEVER;
        
        //When clicking on overlapping items, cycle selection between a single
        //one, or always select the one with the lowest index?
        bool overlapsCycle = false;
        
        //Whether clicking a selected item unselects the other selected items.
        bool clickingSelectedUnselectsOthers = true;
        
        
        //--- Public function declarations ---
        
        void draw(const Point& cursorPos, float zoom) const;
        
        bool chooseViaMouseDown(
            const Point& cursorPos, bool rubberBandMod, bool addToSelectionMod
        );
        
        bool startTransforming();
        bool isTransforming() const;
        bool applyTransformation(
            const Point& newCenter, const Point& newSize, float newAngle
        );
        bool stopTransforming();
        
        bool startRubberBand(const Point& cursorPos);
        bool isCreatingRubberBand() const;
        bool updateRubberBand(
            const Point& cursorPos, bool rubberBandMod, bool addToSelectionMod
        );
        bool stopRubberBand();
        
        bool startDragMove(const Point& cursorPos);
        bool isDragMoving() const;
        bool updateDragMove(const Point& cursorPos);
        bool stopDragMove();
        
        bool getTotalBBox(
            Point* outCenter, Point* outSize,
            Point* outCentersOnlyCenter = nullptr,
            Point* outCentersOnlySize = nullptr
        ) const;
        bool getSelectedItemAngle(float* outAngle, bool* outCanChange) const;
        size_t getSelectionTotalCount(bool* outCanChange = nullptr) const;
        bool isTransformationWidgetAvailable(bool* outCanChange) const;
        bool isOpRuleRespected(OP_RULE rule) const;
        
        bool handleMouseUp();
        
        bool enable();
        bool disable();
        bool isIdle() const;
        
        
    private:
    
        //--- Private misc. declarations ---
        
        //User state, in the selection manager's context.
        enum STATE {
        
            //Idling.
            STATE_IDLING,
            
            //Creating a rubber band selection box.
            STATE_RUBBER_BAND,
            
            //Moving the selection by dragging one of its items.
            STATE_DRAG_MOVING,
            
            //Transforming the selection via a transformation widget.
            STATE_TW_TRANSFORMING,
            
        };
        
        
        //--- Private members ---
        
        //Current user state.
        STATE state = STATE_IDLING;
        
        //Whether it is currently enabled.
        bool enabled = false;
        
        //Cursor position when the current operation started.
        Point preOpCursorPos;
        
        //Center of the entire selection before the latest operation began.
        //Cache for performance.
        Point preOpSelCenter;
        
        //Size of the entire selection before the latest operation began.
        //Cache for performance.
        Point preOpSelSize;
        
        //If the operation should apply to centers only (i.e. items
        //can't resize), use this selection center.
        Point preOpCentersOnlySelCenter;
        
        //If the operation should apply to centers only (i.e. items
        //can't resize), use this selection size.
        Point preOpCentersOnlySelSize;
        
        //Position of the pivot item before an operation.
        //i.e. the item closest to the cursor.
        Point preOpPivotItemPos;
        
    };
    
    /*
     * A widget that's drawn on the window, and with handles that the user
     * can drag in order to translate, scale, or rotate something.
     * The transformation properties are not tied to anything, and are
     * meant to be fed into the widget's functions so it can edit them.
     */
    struct TransformationWidget {
    
        public:
        
        //--- Public misc. declarations ---
        
        //Flags.
        enum TW_FLAG {
        
            //Use position, but don't allow the position handle.
            TW_FLAG_DISABLE_CENTER = 1 << 0,
            
            //Use size, but don't allow the resizing handles.
            TW_FLAG_DISABLE_SIZE = 1 << 1,
            
            //Use rotation, but don't allow the rotation handle.
            TW_FLAG_DISABLE_ANGLE = 1 << 2,
            
            //Whether to keep the aspect ratio when resizing.
            TW_FLAG_KEEP_RATIO = 1 << 3,
            
            //Whether to keep the total area when resizing.
            //Used for squash and stretch.
            TW_FLAG_KEEP_AREA = 1 << 4,
            
            //Whether to lock to the center when resizing.
            //If not set, the opposite edge or corner is locked instead.
            TW_FLAG_LOCK_CENTER = 1 << 5,
            
        };
        
        
        //--- Public function declarations ---
        
        void draw(
            const Point* const center, const Point* const size,
            const float* const angle, float zoom = 1.0f, Bitmask8 flags = 0
        ) const;
        bool handleMouseDown(
            const Point& mouseCoords, const Point* const center,
            const Point* const size, const float* const angle,
            float zoom = 1.0f, Bitmask8 flags = 0
        );
        bool handleMouseMove(
            const Point& mouseCoords, Point* center, Point* size, float* angle,
            float zoom = 1.0f, Bitmask8 flags = 0, float minSize = -FLT_MAX
        );
        bool handleMouseUp();
        bool isMovingCenterHandle();
        bool isMovingHandle();
        Point getOldCenter() const;
        
        
    private:
    
        //--- Private members ---
        
        //What handle is being moved. -1 for none. 9 for the rotation handle.
        signed char movingHandle = -1;
        
        //Old center, before the user started dragging handles.
        Point oldCenter;
        
        //Old size, before the user started dragging handles.
        Point oldSize;
        
        //Old angle, before the user started dragging handles.
        float oldAngle = 0.0f;
        
        //Before rotation began, the mouse made this angle with the center.
        float oldMouseAngle = 0.0f;
        
        
        //--- Private function declarations ---
        
        void getLocations(
            const Point* const center, const Point* const size,
            const float* const angle, Point* points, float* radius,
            ALLEGRO_TRANSFORM* outTransform
        ) const;
        
    };
    
    /**
     * @brief Info about a dialog box.
     */
    class Dialog {
    
    public:
    
        //--- Public members ---
        
        //Callback for when it's time to process the dialog's contents.
        std::function<void()> processCallback = nullptr;
        
        //Callback for when an Allegro event happens.
        std::function<void(ALLEGRO_EVENT*)> eventCallback = nullptr;
        
        //Callback for when the user closes the dialog, if any.
        std::function<void()> closeCallback = nullptr;
        
        //Title to display on the dialog.
        string title;
        
        //Is it open?
        bool isOpen = true;
        
        //Custom dialog position (center point). -1,-1 for default.
        Point customPos = Point(-1.0f);
        
        //Custom dialog size. -1,-1 for default.
        Point customSize = Point(-1.0f);
        
        
        //--- Public function declarations ---
        
        void process();
        
    };
    
    /**
     * @brief An item of a picker dialog.
     */
    struct PickerItem {
    
        //--- Public members ---
        
        //Its name.
        string name;
        
        //What top-level category it belongs to, or empty string for none.
        string topCategory;
        
        //What second-level category it belongs to, or empty string for none.
        string secCategory;
        
        //Information to pass to the code when the item is picked, if any.
        void* info;
        
        //Tooltip, if any.
        string tooltip;
        
        //Bitmap, if any.
        ALLEGRO_BITMAP* bitmap = nullptr;
        
        
        //--- Public function declarations ---
        
        explicit PickerItem(
            const string& name,
            const string& topCategory = "", const string& secondCategory = "",
            void* info = nullptr, const string& tooltip = "",
            ALLEGRO_BITMAP* bitmap = nullptr
        );
        
    };
    
    /**
     * @brief Info about a picker dialog.
     */
    class Picker {
    
    public:
    
        //--- Public members ---
        
        //List of picker dialog items to choose from.
        vector<PickerItem> items;
        
        //Callback for when the user picks an item from the picker dialog.
        std::function<void(
            const string&, const string&, const string&, void*, bool
        )> pickCallback = nullptr;
        
        //Text to display above the picker dialog list.
        string listHeader;
        
        //Can the user make a new item in the picker dialog?
        bool canMakeNew = false;
        
        //Use the monospace font for items?
        bool useMonospace = false;
        
        //When making a new item, the user must pick between these
        //top-level category choices, if applicable.
        vector<string> newItemTopCatChoices;
        
        //Only show picker dialog items matching this filter.
        string filter;
        
        //If there's an associated dialog meant to auto-close, specify it here.
        Dialog* dialogPtr = nullptr;
        
        //Do we need to focus on the filter text box?
        bool needsFilterBoxFocus = true;
        
        //--- Public function declarations ---
        
        explicit Picker(Editor* editorPtr);
        void process();
        
    private:
    
        //--- Private members ---
        
        //Pointer to the editor that's using it.
        Editor* editorPtr = nullptr;
        
        //Top-level category the user picked for the new item, if applicable.
        string newItemTopCat;
        
        //Second-level category the user picked for the new item, if applicable.
        string newItemSecCat;
        
    };
    
    /**
     * @brief Manages the user's changes and everything surrounding it.
     */
    struct ChangesManager {
    
        public:
        
        //--- Public function declarations ---
        
        explicit ChangesManager(Editor* ed);
        bool askIfUnsaved(
            const Point& pos,
            const string& actionLong, const string& actionShort,
            const std::function<void()>& actionCallback,
            const std::function<bool()>& saveCallback
        );
        bool existsOnDisk() const;
        size_t getUnsavedChanges() const;
        float getUnsavedTimeDelta() const;
        const string& getUnsavedWarningActionLong() const;
        const string& getUnsavedWarningActionShort() const;
        const std::function<void()>&
        getUnsavedWarningActionCallback() const;
        const std::function<bool()>&
        getUnsavedWarningSaveCallback() const;
        bool hasUnsavedChanges();
        void markAsChanged();
        void markAsNonExistent();
        void markAsSaved();
        void reset();
        
        
        private:
        
        //--- Private members ---
        
        //Editor it belongs to.
        Editor* ed = nullptr;
        
        //Whether the content exists in the disk.
        bool inDisk = true;
        
        //Cumulative number of unsaved changes since the last save.
        size_t unsavedChanges = 0;
        
        //When did it last go from saved to unsaved? 0 = no unsaved changes.
        float unsavedTime = 0.0f;
        
        //Long name of the action for the open unsaved changes warning dialog.
        string unsavedWarningActionLong;
        
        //Short name of the action for the open unsaved changes warning dialog.
        string unsavedWarningActionShort;
        
        //Action code callback for the open unsaved changes warning dialog.
        std::function<void()> unsavedWarningActionCallback = nullptr;
        
        //Save code callback for the open unsaved changes warning dialog.
        std::function<bool()> unsavedWarningSaveCallback = nullptr;
        
    };
    
    //Style of the different things to draw in the canvas.
    struct CanvasStyle {
    
        //Alpha for the selection effects [0 - 1].
        float selectionAlpha = 0.75f;
        
        //Grid alpha [0 - 1].
        float gridAlpha = 1.0f;
        
        //Highlights color.
        ALLEGRO_COLOR highlightColor = COLOR_WHITE;
        
    };
    
    
    /**
     * @brief Function executed by some command in the editor.
     *
     * The first parameter is the main value [0 - 1].
     */
    typedef std::function<void(float)> CommandFunc;
    
    
    /**
     * @brief Represents one of the editor's possible commands.
     * These are usually triggered by shortcuts.
     */
    struct Command {
    
        public:
        
        //--- Public function declarations ---
        
        Command(CommandFunc f, const string& n);
        void run(float inputValue);
        
        private:
        
        //--- Private members ---
        
        //Function to run.
        CommandFunc func = nullptr;
        
        //Name.
        string name;
        
    };
    
    
    //--- Private members ---
    
    //Callback for when the item is really meant to be picked, in the base
    //content warning dialog.
    std::function<void()> baseContentWarningDoPickCallback = nullptr;
    
    //Currently-chosen bitmap name in the bitmap dialog.
    string bitmapDialogCurBmpName;
    
    //Currently-chosen bitmap pointer in the bitmap dialog.
    ALLEGRO_BITMAP* bitmapDialogCurBmpPtr = nullptr;
    
    //Name of the newly-chosen bitmap in the bitmap dialog.
    string bitmapDialogNewBmpName;
    
    //Callback for when the user chooses a bitmap in the bitmap dialog.
    std::function<void(const string&)> bitmapDialogOkCallback = nullptr;
    
    //Picker for the bitmap dialog.
    Picker bitmapDialogPicker = Picker(this);
    
    //Recommended folder in the bitmap dialog, if any. "." for graphics root.
    string bitmapDialogRecommendedFolder;
    
    //Bitmap with all of the editor icons.
    ALLEGRO_BITMAP* bmpEditorIcons = nullptr;
    
    //X coordinate of the canvas GUI separator. -1 = undefined.
    int canvasSeparatorX = -1;
    
    //Manager of (unsaved) changes.
    ChangesManager changesMgr;
    
    //List of registered commands.
    vector<Command> commands;
    
    //Maps a custom mob category name to an index of the types' vector.
    map<string, size_t> customCatNameIdxs;
    
    //What mob types belong in what custom mob category names.
    vector<vector<MobType*>> customCatTypes;
    
    //Currently open dialogs, if any.
    vector<Dialog*> dialogs;
    
    //If the next click is within this time, it's a double-click.
    float doubleClickTime = 0.0f;
    
    //List of every individual editor icon.
    vector<ALLEGRO_BITMAP*> editorIcons;
    
    //If Escape was pressed the previous frame.
    bool escapeWasPressed = false;
    
    //Is the Alt key currently pressed down?
    bool isAltPressed = false;
    
    //Is the Ctrl key currently pressed down?
    bool isCtrlPressed = false;
    
    //Is the Shift key currently pressed down?
    bool isShiftPressed = false;
    
    //Is the left mouse button currently pressed down?
    bool isM1Pressed = false;
    
    //Is the right mouse button currently pressed down?
    bool isM2Pressed = false;
    
    //Is the middle mouse button currently pressed down?
    bool isM3Pressed = false;
    
    //Is the mouse currently hovering the gui? False if it's the canvas.
    bool isMouseInGui = false;
    
    //Whether the left mouse button's drag start was on the GUI or canvas.
    bool isM1DragStartInGui = false;
    
    //Whether the right mouse button's drag start was on the GUI or canvas.
    bool isM2DragStartInGui = false;
    
    //Whether the middle mouse button's drag start was on the GUI or canvas.
    bool isM3DragStartInGui = false;
    
    //Number of the mouse button pressed.
    size_t lastMouseClick = INVALID;
    
    //Window location of the mouse cursor on the last mouse button press.
    Point lastMouseClickPos;
    
    //Editor sub-state during the last mouse click.
    size_t lastMouseClickSubState = INVALID;
    
    //Was the last user input a keyboard press?
    bool lastInputWasKeyboard = false;
    
    //Manifest for the current content.
    ContentManifest manifest;
    
    //Message text in the Dear ImGui help dialog.
    string helpDialogMessage;
    
    //Page to open in the help dialog.
    string helpDialogPage;
    
    //Message text in the Dear ImGui message dialog.
    string messageDialogMessage;
    
    //Is this a real mouse drag, or just a shaky click?
    bool mouseDragConfirmed = false;
    
    //Starting coordinates of a raw mouse drag.
    Point mouseDragStart;
    
    //Do we need to focus on the input popup's text widget?
    bool needsInputPopupTextFocus = true;
    
    //Do we need to focus on the new pack's name text widget?
    bool needsNewPackTextFocus = true;
    
    //Index of the selected pack in the "new" content dialog.
    int newContentDialogPackIdx = 0;
    
    //Time left in the operation error red flash effect.
    Timer opErrorFlashTimer = Timer(EDITOR::OP_ERROR_FLASH_DURATION);
    
    //Position of the operation error red flash effect.
    Point opErrorPos;
    
    //Current state.
    size_t state = 0;
    
    //Status bar text.
    string statusText;
    
    //Current sub-state.
    size_t subState = 0;
    
    //Maximum zoom level allowed.
    float zoomMaxLevel = 0.0f;
    
    //Minimum zoom level allowed.
    float zoomMinLevel = 0.0f;
    
    //Prefix for the widget internal names in the current nav box.
    string curNavBoxItemPrefix;
    
    //Singular term for the items of the current nav box.
    string curNavBoxItemTerm;
    
    //Plural term for the items of the current nav box, if different from
    //the singular term plus an 's'.
    string curNavBoxItemTermPlural;
    
    //Pointer to the selected item's index in the current nav box.
    size_t* curNavBoxSelIdxPtr = nullptr;
    
    //Callback for when the list size needs to be retrieved
    //for the current nav box.
    std::function<size_t()> curNavBoxOnGetSize = nullptr;
    
    //Callback for when the selection size needs to be retrieved
    //for the current nav box.
    std::function<size_t()> curNavBoxOnGetSelSize = nullptr;
    
    
    //--- Private function declarations ---
    
    void centerCamera(
        const Point& minCoords, const Point& maxCoords,
        bool instantaneous = false
    );
    void closeTopDialog();
    void doLogicPost();
    void doLogicPre();
    void drawGrid(
        float interval,
        const ALLEGRO_COLOR& majorColor, const ALLEGRO_COLOR& minorColor
    );
    void drawOpErrorCursor();
    void drawSelectionAndTransformationThings(
        const SelectionController& selCtrl,
        const TransformationWidget& traWid
    );
    Point getLastWidgetPost();
    void getQuickPlayAreaList(
        string selectedAreaPath,
        vector<string>* outAreaNames, vector<string>* outAreaPaths,
        int* outSelectedAreaIdx
    ) const;
    bool guiFocusedText();
    bool keyCheck(
        int pressedKey, int matchKey,
        bool needsCtrl = false, bool needsShift = false
    );
    bool isInternalNameGood(const string& name) const;
    bool listPopup(
        const char* label, const vector<string>& items, string* pickedItem,
        bool useMonospace = false
    );
    bool listPopup(
        const char* label, const vector<string>& items, int* pickedItemIdx,
        bool useMonospace = false
    );
    void leave();
    void loadCustomMobCatTypes(bool isAreaEditor);
    void openBaseContentWarningDialog(
        const std::function<void()>& doPickCallback
    );
    void openBitmapDialog(
        std::function<void(const string&)> okCallback,
        const string& recommendedFolder = ""
    );
    void openDialog(
        const string& title,
        const std::function<void()>& processCallback
    );
    void openHelpDialog(
        const string& message, const string& page
    );
    void openInputPopup(const char* label);
    void openMessageDialog(
        const string& title, const string& message,
        const std::function<void()>& okCallback = nullptr
    );
    void openNewPackDialog();
    void openPickerDialog(
        const string& title,
        const vector<PickerItem>& items,
        const std::function<void(
            const string&, const string&, const string&, void*, bool
        )>& pickCallback,
        const string& listHeader = "",
        bool canMakeNew = false,
        bool useMonospace = false,
        const string& filter = ""
    );
    bool popup(const char* label, ImGuiWindowFlags flags = 0);
    void processDialogs();
    void processGuiCanvas();
    void processGuiDialogBaseContent();
    void processGuiDialogBitmap();
    void processGuiDialogHelp();
    void processGuiDialogMessage();
    void processGuiDialogNewPack();
    void processGuiDialogUnsavedChanges();
    void processGuiEditorStyle();
    void processGuiHistory(
        const vector<pair<string, string> >& history,
        const std::function<string(const string&)>& nameDisplayCallback,
        const std::function<void(const string&)>& pickCallback,
        const std::function<string(const string&)>& tooltipCallback
    );
    bool processGuiPopupInput(
        const char* label, const char* prompt, string* text,
        bool useMonospace = false
    );
    void processGuiStatusBarText();
    bool processGuiWidgetsHazardManagement(string& selectedHazardIname);
    bool processGuiWidgetsMobType(
        string* customCatName, MobType** type, const string& packFilter = ""
    );
    bool processGuiWidgetsNewDialogPack(string* pack);
    bool processGuiWidgetsSize(
        const char* label, Point& size, float vSpeed,
        bool keepAspectRatio, bool keepArea,
        float minSize
    );
    void processGuiNavSetup(
        size_t* curItemIdx, size_t listSize, bool allowInvalid
    );
    bool processGuiNavWidgetNew(
        size_t* curItemIdx, size_t listSize,
        const string& customButtonId = "", float buttonScale = 1.0f
    );
    bool processGuiNavWidgetDel(
        size_t* curItemIdx, size_t listSize,
        const string& customButtonId = "", float buttonScale = 1.0f
    );
    bool processGuiNavWidgetPrev(
        size_t* curItemIdx, size_t listSize,
        const string& customButtonId = "", float buttonScale = 1.0f
    );
    bool processGuiNavWidgetNext(
        size_t* curItemIdx, size_t listSize,
        const string& customButtonId = "", float buttonScale = 1.0f
    );
    bool processGuiNavWidgetMoveLeft(
        size_t* curItemIdx, size_t listSize,
        const string& customButtonId = "", float buttonScale = 1.0f
    );
    bool processGuiNavWidgetMoveRight(
        size_t* curItemIdx, size_t listSize,
        const string& customButtonId = "", float buttonScale = 1.0f
    );
    void processGuiNavBoxStart(
        const string& widgetsPrefix, const string& itemsTerm,
        const string& itemsTermPlural, size_t* selIdxPtr,
        const std::function<size_t()>& onGetSize,
        const std::function<size_t()>& onGetSelSize
    );
    bool processGuiNavBoxPrev();
    void processGuiNavBoxCur(
        const string& curItemName = "", bool curItemNameMono = false,
        bool showTermNormally = true
    );
    bool processGuiNavBoxNext();
    void processGuiNavBoxPlaceholder();
    void processGuiNavBoxSecondLine(size_t nrItems);
    void processGuiNavBoxEnd();
    void getGuiNavCurText(
        size_t curItemIdx, size_t listSize, size_t selectionSize,
        const string& itemTerm, const string& itemTermPlural,
        bool showTermNormally, const string& curItemName, bool curItemNameMono,
        string* outText1, bool* outText1Disabled,
        string* outText2, bool* outText2Mono
    );
    void panelTitle(const char* title);
    
    void angleVisualizer(float angle);
    void keyframeVisualizer(
        KeyframeInterpolator<ALLEGRO_COLOR>& interpolator,
        size_t selKeyframeIdx
    );
    void keyframeVisualizer(
        KeyframeInterpolator<float>& interpolator,
        size_t selKeyframeIdx
    );
    void keyframeVisualizer(
        KeyframeInterpolator<Point>& interpolator,
        size_t selKeyframeIdx
    );
    template <class InterT>
    bool keyframeOrganizer(
        const string& buttonId,
        KeyframeInterpolator<InterT>& interpolator,
        size_t& selKeyframeIdx
    );
    bool keyframeEditor(
        const string& label,
        KeyframeInterpolator<float>& interpolator,
        size_t& selKeyframeIdx
    );
    bool keyframeEditor(
        const string& label,
        KeyframeInterpolator<ALLEGRO_COLOR>& interpolator,
        size_t& selKeyframeIdx
    );
    bool keyframeEditor(
        const string& label,
        KeyframeInterpolator<Point>& interpolator,
        size_t& selKeyframeIdx
    );
    
    bool saveableTreeNode(const string& category, const string& label);
    void setStatus(const string& text = "", bool error = false);
    void setTooltip(
        const string& explanation, const string& shortcut = "",
        const WIDGET_EXPLANATION widgetExplanation = WIDGET_EXPLANATION_NONE
    );
    Point snapPointToAxis(const Point& p, const Point& anchor);
    Point snapPointToGrid(const Point& p, float gridInterval);
    void updateHistory(
        vector<pair<string, string> >& history,
        const ContentManifest& manifest, const string& name
    );
    void zoomWithCursor(float newZoom);
    virtual void handleKeyCharAnywhere(const ALLEGRO_EVENT& ev);
    virtual void handleKeyCharCanvas(const ALLEGRO_EVENT& ev);
    virtual void handleKeyDownAnywhere(const ALLEGRO_EVENT& ev);
    virtual void handleKeyDownCanvas(const ALLEGRO_EVENT& ev);
    virtual void handleKeyUpAnywhere(const ALLEGRO_EVENT& ev);
    virtual void handleKeyUpCanvas(const ALLEGRO_EVENT& ev);
    virtual void handleLmbDoubleClick(const ALLEGRO_EVENT& ev);
    virtual void handleLmbDown(const ALLEGRO_EVENT& ev);
    virtual void handleLmbDrag(const ALLEGRO_EVENT& ev);
    virtual void handleLmbUp(const ALLEGRO_EVENT& ev);
    virtual void handleMmbDoubleClick(const ALLEGRO_EVENT& ev);
    virtual void handleMmbDown(const ALLEGRO_EVENT& ev);
    virtual void handleMmbDrag(const ALLEGRO_EVENT& ev);
    virtual void handleMmbUp(const ALLEGRO_EVENT& ev);
    virtual void handleMouseUpdate(const ALLEGRO_EVENT& ev);
    virtual void handleMouseWheel(const ALLEGRO_EVENT& ev);
    virtual void handleRmbDoubleClick(const ALLEGRO_EVENT& ev);
    virtual void handleRmbDown(const ALLEGRO_EVENT& ev);
    virtual void handleRmbDrag(const ALLEGRO_EVENT& ev);
    virtual void handleRmbUp(const ALLEGRO_EVENT& ev);
    void handleSelectionAndTransformationLmbDown(
        SelectionController& selCtrl, TransformationWidget& traWid
    );
    bool handleSelectionAndTransformationLmbDrag(
        SelectionController& selCtrl, TransformationWidget& traWid,
        const Point& mouseCursor, std::function<void()> onPreTransform = nullptr
    );
    void handleSelectionAndTransformationLmbUp(
        SelectionController& selCtrl, TransformationWidget& traWid
    );
    string getAmountOrIdxDescription(
        size_t singleIdx, size_t amount,
        const string& singularTerm, const string& pluralTerm = ""
    ) const;
};
