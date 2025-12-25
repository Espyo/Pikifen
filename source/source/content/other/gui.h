/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general GUI manager and GUI item classes.
 * These are used during gameplay and menus, and are not related to Dear ImGui,
 * which is the GUI library used for the editors.
 */

#pragma once

#include <functional>
#include <map>
#include <vector>

#include <allegro5/allegro_font.h>

#include "../../core/const.h"
#include "../../core/controls_mediator.h"
#include "../../core/misc_structs.h"
#include "../../lib/data_file/data_file.h"
#include "../../lib/easy_spat_nav/easy_spat_nav.h"
#include "../../util/drawing_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"


using std::map;
using std::vector;


namespace GUI {
extern const float AUTO_REPEAT_MAX_INTERVAL;
extern const float AUTO_REPEAT_MIN_INTERVAL;
extern const float AUTO_REPEAT_RAMP_TIME;
extern const float BULLET_PADDING;
extern const float BULLET_RADIUS;
extern const unsigned char DRAWING_LAYER_CUSTOM_AFTER;
extern const unsigned char DRAWING_LAYER_CUSTOM_BEFORE;
extern const unsigned char DRAWING_LAYER_NORMAL;
extern const float FOCUS_CURSOR_ALPHA_SPEED;
extern const float FOCUS_CURSOR_BOB_OFFSET;
extern const float FOCUS_CURSOR_BOB_TIME_MULT;
extern const float FOCUS_CURSOR_FADE_GROW_OFFSET;
extern const float FOCUS_CURSOR_SIZE_ADDER;
extern const float FOCUS_CURSOR_SMOOTHNESS_FACTOR;
extern const float JUICY_GROW_DURATION;
extern const float JUICY_GROW_ELASTIC_DURATION;
extern const float JUICY_GROW_ICON_MULT;
extern const float JUICY_GROW_TEXT_HIGH_MULT;
extern const float JUICY_GROW_TEXT_LOW_MULT;
extern const float JUICY_GROW_TEXT_MEDIUM_MULT;
extern const Point STANDARD_CONTENT_SIZE;
}


//Animations for the GUI manager to animate its items with.
enum GUI_MANAGER_ANIM {

    //None.
    GUI_MANAGER_ANIM_NONE,
    
    //Items are outward out of view, and slide inward into view.
    GUI_MANAGER_ANIM_OUT_TO_IN,
    
    //Items are in view, and slide outward out of view.
    GUI_MANAGER_ANIM_IN_TO_OUT,
    
    //Items are above out of view, and slide downward into view.
    GUI_MANAGER_ANIM_UP_TO_CENTER,
    
    //Items are in view, and slide up out of view.
    GUI_MANAGER_ANIM_CENTER_TO_UP,
    
    //Items are below out of view, and slide upward into view.
    GUI_MANAGER_ANIM_DOWN_TO_CENTER,
    
    //Items are in view, and slide down out of view.
    GUI_MANAGER_ANIM_CENTER_TO_DOWN,
    
    //Items are to the left out of view, and slide right into view.
    GUI_MANAGER_ANIM_LEFT_TO_CENTER,
    
    //Items are in view, and slide left out of view.
    GUI_MANAGER_ANIM_CENTER_TO_LEFT,
    
    //Items are to the right out of view, and slide left into view.
    GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
    
    //Items are in view, and slide right out of view.
    GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
    
    //Fade in from complete transparency.
    GUI_MANAGER_ANIM_FADE_IN,
    
    //Fade out to complete transparency.
    GUI_MANAGER_ANIM_FADE_OUT,
    
};


//Type of things to draw in a custom item.
enum CUSTOM_GUI_ITEM_TYPE {

    //A bitmap.
    CUSTOM_GUI_ITEM_TYPE_BITMAP,
    
    //A 9-slice texture.
    CUSTOM_GUI_ITEM_TYPE_9_SLICE,
    
    //Text.
    CUSTOM_GUI_ITEM_TYPE_TEXT,
    
    //A non-filled rectangle.
    CUSTOM_GUI_ITEM_TYPE_RECTANGLE,
    
    //A filled rectangle.
    CUSTOM_GUI_ITEM_TYPE_FILLED_RECTANGLE,
    
    //A non-filled square.
    CUSTOM_GUI_ITEM_TYPE_SQUARE,
    
    //A filled square.
    CUSTOM_GUI_ITEM_TYPE_FILLED_SQUARE,
    
    //A non-filled ellipse.
    CUSTOM_GUI_ITEM_TYPE_ELLIPSE,
    
    //A filled ellipse.
    CUSTOM_GUI_ITEM_TYPE_FILLED_ELLIPSE,
    
    //A non-filled circle.
    CUSTOM_GUI_ITEM_TYPE_CIRCLE,
    
    //A filled circle.
    CUSTOM_GUI_ITEM_TYPE_FILLED_CIRCLE,
    
};


class GuiManager;


/**
 * @brief Definition of a GUI item. This has no data about the item's behavior,
 * just data about what it is and where.
 */
struct GuiItemDef {

    //--- Members ---
    
    //Its internal name.
    string name;
    
    //Center coordinates.
    Point center;
    
    //Width and height.
    Point size;
    
    //Optional description. Shows up in the GUI editor.
    string description;
    
};



/**
 * @brief Definition of a hardcoded GUI item, an item that must exist in the GUI
 * since the engine expects it.
 */
struct HardcodedGuiItemDef : public GuiItemDef {

};


/**
 * @brief Definition of a custom GUI item, added by the user, that is not a part
 * of the items the engine expects for the GUI.
 */
struct CustomGuiItemDef : public GuiItemDef {

    //--- Members ---
    
    //Type.
    CUSTOM_GUI_ITEM_TYPE type = CUSTOM_GUI_ITEM_TYPE_BITMAP;
    
    //Tint or shape color.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    //Draw before the hardcoded items.
    bool drawBeforeHardcoded = false;
    
    //Internal name of the bitmap to use, if any.
    string bitmapName;
    
    //Image to use, if any.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //Text to write, if any.
    string text;
    
    //Type of the font for the text, if any.
    ENGINE_FONT fontType = ENGINE_FONT_STANDARD;
    
    //Font for the text, if any.
    ALLEGRO_FONT* font = nullptr;
    
    //Text alignment.
    unsigned char textAlignment = ALLEGRO_ALIGN_CENTER;
    
    //Shape thickness, if any.
    float thickness = 1.0f;
    
    //Rectangle corner rounding, if any.
    float rectangleRounding = 0.0f;
    
    
    //--- Function declarations ---
    
    void clearBitmap();
    
};


/**
 * @brief An item in the GUI. This can be a HUD element, a button,
 * some text, etc.
 */
class GuiItem {

public:

    //--- Misc. definitions ---
    
    //Juicy animation types for GUI items.
    enum JUICE_TYPE {
    
        //None.
        JUICE_TYPE_NONE,
        
        //Text grow effect, low impact.
        JUICE_TYPE_GROW_TEXT_LOW,
        
        //Text grow effect, medium impact.
        JUICE_TYPE_GROW_TEXT_MEDIUM,
        
        //Text grow effect, high impact.
        JUICE_TYPE_GROW_TEXT_HIGH,
        
        //Elastic text grow effect, low impact.
        JUICE_TYPE_GROW_TEXT_ELASTIC_LOW,
        
        //Elastic text grow effect, medium impact.
        JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM,
        
        //Elastic text grow effect, high impact.
        JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH,
        
        //Icon grow effect.
        JUICE_TYPE_GROW_ICON,
        
    };
    
    //Information about how the item should be drawn.
    struct DrawInfo {
    
        //Center pixel coordinates.
        Point center;
        
        //Pixel dimensions.
        Point size;
        
        //Tint color.
        ALLEGRO_COLOR tint = COLOR_WHITE;
        
    };
    
    
    //--- Members ---
    
    //What GUI manager it belongs to, if any.
    GuiManager* manager = nullptr;
    
    //Its raw on-window position, in window ratio (or parent ratio).
    Point ratioCenter;
    
    //Its raw width and height, in window ratio (or parent ratio).
    Point ratioSize;
    
    //Is it currently visible?
    bool visible = true;
    
    //Is it currently responsive?
    bool responsive = true;
    
    //Can it be focused?
    bool focusable = false;
    
    //Is it currently the focused item?
    bool focused = false;
    
    //If it is placed inside of another item, specify it here.
    GuiItem* parent = nullptr;
    
    //List of children items, that are placed inside this one.
    vector<GuiItem*> children;
    
    //Offset (width/height percentage) of the items inside of it, if any.
    Point offset;
    
    //Padding amount, if it has items inside of it.
    float padding = 0.0f;
    
    //Drawing layer. The lower the number, the sooner it'll be drawn.
    unsigned char drawingLayer = GUI::DRAWING_LAYER_NORMAL;
    
    //Can this item's activation be auto-repeated by holding the button down?
    bool canAutoRepeat = false;
    
    //Is this item focusable from the mouse?
    bool focusableFromMouse = true;
    
    //Is this item focusable from spatial navigation?
    bool focusableFromSN = true;
    
    //Force its final dimensions to be square, such that they fit inside
    //the reference dimensions.
    bool forceSquare = false;
    
    //Type of the current juice animation.
    JUICE_TYPE juiceType = JUICE_TYPE_NONE;
    
    //Timer that controls the current juice animation.
    float juiceTimer = 0.0f;
    
    //What to do when it's time to draw it.
    std::function<void(const DrawInfo& draw)> onDraw = nullptr;
    
    //What to do when it's time to tick one frame.
    std::function<void(float time)> onTick = nullptr;
    
    //What to do when it receives any Allegro event.
    std::function<void(const ALLEGRO_EVENT& ev)> onAllegroEvent = nullptr;
    
    //What to do when the item is activated.
    std::function<void(const Point& cursorPos)> onActivate = nullptr;
    
    //What to do when the mouse cursor is on top of it this frame.
    std::function<void(const Point& cursorPos)> onMouseOver = nullptr;
    
    //What to do when a spatial navigation action is performed
    //when the item is focused. The return value is whether the logic
    //to change focused items should be skipped.
    std::function<bool(PLAYER_ACTION_TYPE playerActionId)> onMenuSNAction =
        nullptr;
        
    //What to do when it gets focused.
    std::function<void()> onFocused = nullptr;
    
    //What to do when one of its children became the focused item via
    //spatial navigation.
    std::function<void(const GuiItem* child)> onChildFocusedViaSN = nullptr;
    
    //What to do when its tooltip needs to be retrieved.
    std::function<string()> onGetTooltip = nullptr;
    
    //Play a "failure" sound on activation instead, for the next activation.
    bool playFailSound = false;
    
    
    //--- Function declarations ---
    
    explicit GuiItem(bool focusable = false);
    virtual ~GuiItem() = default;
    bool activate(const Point& cursorPos = Point());
    bool addChild(GuiItem* item);
    bool deleteAllChildren();
    float getChildrenSpan(bool horizontal = false) const;
    float getJuiceValue() const;
    Point getReferenceCenter() const;
    Point getReferenceSize() const;
    bool isMouseOn(const Point& cursorPos) const;
    bool isResponsive() const;
    bool isVisible() const;
    bool removeChild(GuiItem* item);
    bool startJuiceAnimation(JUICE_TYPE type);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a bullet
 * point in a list.
 */
class BulletGuiItem : public GuiItem {

public:

    //--- Members ---
    
    //Text to display on the bullet point.
    string text;
    
    //Font to display the text with.
    ALLEGRO_FONT* font = nullptr;
    
    //Color to tint the text with.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    
    //--- Function declarations ---
    
    BulletGuiItem(
        const string& text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR& color = COLOR_WHITE
    );
    
    void defDrawCode(const DrawInfo& draw);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a button.
 */
class ButtonGuiItem : public GuiItem {

public:

    //--- Members ---
    
    //Text to display on the button.
    string text;
    
    //Font to display the text with.
    ALLEGRO_FONT* font = nullptr;
    
    //Color to tint the text with.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    
    //--- Function declarations ---
    
    ButtonGuiItem(
        const string& text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR& color = COLOR_WHITE
    );
    
    void defDrawCode(const DrawInfo& draw);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a checkbox.
 */
class CheckGuiItem : public GuiItem {

public:

    //--- Members ---
    
    //Current value.
    bool value = false;
    
    //If not nullptr, the value is automatically adjusted to reflect this
    //variable and vice-versa.
    bool* valuePtr = nullptr;
    
    //Text to display on the button.
    string text;
    
    //Font to display the text with.
    ALLEGRO_FONT* font = nullptr;
    
    //Color to tint the text with.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    
    //--- Function declarations ---
    
    explicit CheckGuiItem(
        bool value, const string& text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR& color = COLOR_WHITE
    );
    explicit CheckGuiItem(
        bool* valuePtr, const string& text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR& color = COLOR_WHITE
    );
    
    void defActivateCode();
    void defDrawCode(const DrawInfo& draw);
    
};


class ScrollGuiItem;

/**
 * @brief A GUI item with fields ready to make it behave like a list.
 */
class ListGuiItem : public GuiItem {

public:

    //--- Members ---
    
    //What the offset is supposed to be, after it finishes animating.
    float targetOffset = 0.0f;
    
    //Whether it's designed to be scrolled horizontally or vertically.
    bool horizontal = false;
    
    
    //--- Function declarations ---
    
    ListGuiItem();
    
    void defChildFocusedViaSNCode(const GuiItem* child);
    void defDrawCode(const DrawInfo& draw);
    void defEventCode(const ALLEGRO_EVENT& ev);
    void defTickCode(float deltaT);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a previous/next
 * option picker.
 */
class PickerGuiItem : public GuiItem {

public:

    //--- Members ---
    
    //The text to show before the currently chosen option.
    string baseText;
    
    //The currently chosen option.
    string option;
    
    //Total amount of options. Optional.
    size_t nrOptions = 0;
    
    //Index of the currently chosen option. Only used if nrOptions > 0.
    size_t curOptionIdx = INVALID;
    
    //What to do when the user picks the previous option.
    std::function<void()> onPrevious = nullptr;
    
    //What to do when the user picks the next option.
    std::function<void()> onNext = nullptr;
    
    
    //--- Function declarations ---
    
    PickerGuiItem(
        const string& baseText, const string& option,
        size_t nrOptions = 0, size_t curOptionIdx = INVALID
    );
    
    void defActivateCode(const Point& cursorPos);
    void defDrawCode(const DrawInfo& draw);
    bool defMenuSNCode(PLAYER_ACTION_TYPE playerActionId);
    void defMouseOverCode(const Point& cursorPos);
    
    
private:

    //--- Members ---
    
    //Highlight one of the arrows due to mouse-over. 255 = none.
    unsigned char arrowHighlight = 255;
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a scrollbar.
 */
class ScrollGuiItem : public GuiItem {

public:

    //--- Members ---
    
    //What item this scrollbar is in charge of controlling.
    ListGuiItem* listItem = nullptr;
    
    //Whether it's meant to be horizontal or vertical.
    bool horizontal = false;
    
    //Is the left mouse button being dragged, starting on this widget?
    bool isMouseDragging = false;
    
    
    //--- Function declarations ---
    
    ScrollGuiItem();
    
    void defDrawCode(const DrawInfo& draw);
    void defEventCode(const ALLEGRO_EVENT& ev);
    
    
private:

    //--- Function declarations ---
    
    void setOffsetFromMouse(float x, float y);
    
};


/**
 * @brief A GUI item with fields ready to make it behave like a simple
 * text display.
 */
class TextGuiItem : public GuiItem {

public:

    //--- Members ---
    
    //Text to display.
    string text;
    
    //Font to display the text with.
    ALLEGRO_FONT* font = nullptr;
    
    //Color to tint the text with.
    ALLEGRO_COLOR color = COLOR_WHITE;
    
    //Allegro flags.
    int flags = ALLEGRO_ALIGN_CENTER;
    
    //Wrap long lines. Also enables markup.
    bool lineWrap = false;
    
    
    //--- Function declarations ---
    
    TextGuiItem(
        const string& text, ALLEGRO_FONT* font,
        const ALLEGRO_COLOR& color = COLOR_WHITE,
        int flags = ALLEGRO_ALIGN_CENTER
    );
    
    void defDrawCode(const DrawInfo& draw);
    
};


/**
 * @brief A GUI item with fields ready to make it specialize in showing another
 * item's tooltip.
 */
class TooltipGuiItem : public GuiItem {

public:

    //--- Members ---
    
    //The GUI it belongs to.
    GuiManager* gui = nullptr;
    
    
    //--- Function declarations ---
    
    explicit TooltipGuiItem(GuiManager* gui);
    
    void defDrawCode(const DrawInfo& draw);
    
    
private:

    //--- Members ---
    
    //Text it was showing the previous frame.
    string prevText;
    
};


/**
 * @brief GUI manager.
 *
 * This manager is not used in the editors, since those work with
 * Dear ImGui. It is responsible for holding information about all GUI
 * items present on the game window, managing their coordinates, which one is
 * focused, ordering them to be rendered or to handle being activated, etc.
 * Due to the system's flexibility, this is used both to manage the game's
 * heads-up display (HUD) during gameplay, as well as the interactable elements
 * of menus.
 */
class GuiManager {

public:

    //--- Members ---
    
    //List of items.
    vector<GuiItem*> items;
    
    //Item to activate when the user chooses to go back, if any.
    GuiItem* backItem = nullptr;
    
    //Is it currently responding to input?
    bool responsive = true;
    
    //Should it ignore input while animating?
    bool ignoreInputOnAnimation = true;
    
    //What to do when the currently focused item changes.
    std::function<void()> onFocusChanged = nullptr;
    
    
    //--- Function declarations ---
    
    GuiManager();
    bool addItem(GuiItem* item, const string& id = "");
    bool draw();
    bool tick(float deltaT);
    string getCurrentTooltip() const;
    static bool getItemDefsFromDataFile(
        DataNode* file,
        vector<HardcodedGuiItemDef>* outHardcodedItemDefs,
        vector<CustomGuiItemDef>* outCustomItemDefs
    );
    bool getItemDrawInfo(GuiItem* item, GuiItem::DrawInfo* draw) const;
    GuiItem* getFocusedItem() const;
    bool handleAllegroEvent(const ALLEGRO_EVENT& ev);
    bool handlePlayerAction(const Inpution::Action& action);
    bool hideItems();
    bool readDataFile(DataNode* node);
    bool registerCoords(
        const string& id,
        float cx, float cy, float w, float h
    );
    bool removeItem(GuiItem* item);
    bool setFocusedItem(GuiItem* item, bool silent = false);
    bool shouldHandleEvents();
    bool showItems();
    bool startAnimation(
        const GUI_MANAGER_ANIM type, float duration
    );
    bool wasLastInputMouse() const;
    static bool writeItemDefsToDataFile(
        DataNode* file,
        const vector<HardcodedGuiItemDef>& hardcodedItemDefs,
        const vector<CustomGuiItemDef>& customItemDefs
    );
    bool destroy();
    
    
protected:

    //--- Members ---
    
    //Which item is currently focused.
    GuiItem* focusedItem = nullptr;
    
    //Registered default centers.
    map<string, Point> registeredCenters;
    
    //Registered default sizes.
    map<string, Point> registeredSizes;
    
    //Custom items.
    vector<CustomGuiItemDef> customItemDefs;
    
    //Is the OK button pressed?
    bool okPressed = false;
    
    //Is the back button pressed?
    bool backPressed = false;
    
    //Was the last input given a mouse movement?
    bool lastInputWasMouse = false;
    
    //Auto-repeater settings.
    AutoRepeaterSettings autoRepeaterSettings;
    
    //Auto-repeat data for the current item's activation.
    AutoRepeater autoRepeater;
    
    //Type of the current animation, if any.
    GUI_MANAGER_ANIM animType = GUI_MANAGER_ANIM_NONE;
    
    //Timer for the current animation.
    Timer animTimer;
    
    //Are the items currently visible?
    bool visible = true;
    
    //Easy Spatial Navigation interface.
    EasySpatNav::Interface esnInterface;
    
    //Focus cursor.
    struct FocusCursor {
    
        //Current center coordinates.
        Point curPos;
        
        //Current base width and height.
        Point curSize;
        
        //Intended center coordinates.
        Point intendedPos;
        
        //Intended base width and height.
        Point intendedSize;
        
        //Current base opacity [0 - 1].
        float alpha = 0.0f;
        
    } focusCursor;
    
    
    //--- Function declarations ---
    void createCustomItems();
    void handleSpatialNavigationAction(const Inpution::Action& action);
    
};
