/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General GUI manager and GUI item classes.
 * These are used during gameplay and menus, and are not related to Dear ImGui,
 * which is the GUI library used for the editors.
 */

#include <algorithm>

#include "gui.h"

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace GUI {

//Interval between auto-repeat activations, at the slowest speed.
const float AUTO_REPEAT_MAX_INTERVAL = 0.3f;

//Interval between auto-repeat activations, at the fastest speed.
const float AUTO_REPEAT_MIN_INTERVAL = 0.011f;

//How long it takes for the auto-repeat activations to reach max speed.
const float AUTO_REPEAT_RAMP_TIME = 0.9f;

//Padding before/after the circle in a bullet point item.
const float BULLET_PADDING = 6.0f;

//Radius of the circle that represents the bullet in a bullet point item.
const float BULLET_RADIUS = 4.0f;

//Speed at which the focus cursor's alpha changes.
const float FOCUS_CURSOR_ALPHA_SPEED = 4.0f;

//The focus cursor's bobbing makes it offset its size by this much at most.
const float FOCUS_CURSOR_BOB_OFFSET = 2.0f;

//The focus cursor's bobbing speed is multiplied by this much.
const float FOCUS_CURSOR_BOB_TIME_MULT = TAU;

//When fading away, the cursor grows these many pixels at most.
const float FOCUS_CURSOR_FADE_GROW_OFFSET = 30.0f;

//The focus cursor's is these many pixels larger than the item.
const float FOCUS_CURSOR_SIZE_ADDER = 10.0f;

//Dampen the focus cursor's position or size changes by this much.
const float FOCUS_CURSOR_SMOOTHNESS_MULT = 15.0f;

//When an item does a juicy grow, this is the full effect duration.
const float JUICY_GROW_DURATION = 0.3f;

//When an item does a juicy elastic grow, this is the full effect duration.
const float JUICY_GROW_ELASTIC_DURATION = 0.4f;

//Grow scale multiplier for a juicy icon grow animation.
const float JUICY_GROW_ICON_MULT = 5.0f;

//Grow scale multiplier for a juicy text high grow animation.
const float JUICY_GROW_TEXT_HIGH_MULT = 0.15f;

//Grow scale multiplier for a juicy text low grow animation.
const float JUICY_GROW_TEXT_LOW_MULT = 0.02f;

//Grow scale multiplier for a juicy text medium grow animation.
const float JUICY_GROW_TEXT_MEDIUM_MULT = 0.05f;

//Standard size of the content inside of a GUI item, in ratio.
const Point STANDARD_CONTENT_SIZE = Point(0.95f, 0.80f);

}


/**
 * @brief Constructs a new bullet point GUI item object.
 *
 * @param text Text to display on the bullet point.
 * @param font Font for the button's text.
 * @param color Color of the button's text.
 */
BulletGuiItem::BulletGuiItem(
    const string& text, ALLEGRO_FONT* font, const ALLEGRO_COLOR& color
) :
    GuiItem(true),
    text(text),
    font(font),
    color(color) {
    
    onDraw =
    [this] (const DrawInfo & draw) {
        this->defDrawCode(draw);
    };
}


/**
 * @brief Default bullet GUI item draw code.
 *
 * @param draw Information on how to draw.
 */
void BulletGuiItem::defDrawCode(
    const DrawInfo& draw
) {
    float itemXStart = draw.center.x - draw.size.x * 0.5;
    float textXOffset =
        GUI::BULLET_RADIUS * 2 +
        GUI::BULLET_PADDING * 2;
    Point textSpace(
        std::max(1.0f, draw.size.x - textXOffset),
        draw.size.y
    );
    
    drawBitmap(
        game.sysContent.bmpHardBubble,
        Point(
            itemXStart + GUI::BULLET_RADIUS + GUI::BULLET_PADDING,
            draw.center.y
        ),
        Point(GUI::BULLET_RADIUS * 2),
        0.0f, this->color
    );
    float juicyGrowAmount = getJuiceValue();
    drawText(
        this->text, this->font,
        Point(itemXStart + textXOffset, draw.center.y),
        textSpace * GUI::STANDARD_CONTENT_SIZE,
        this->color, ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW,
        Point(1.0 + juicyGrowAmount)
    );
}


/**
 * @brief Constructs a new button GUI item object.
 *
 * @param text Text to display on the button.
 * @param font Font for the button's text.
 * @param color Color of the button's text.
 */
ButtonGuiItem::ButtonGuiItem(
    const string& text, ALLEGRO_FONT* font, const ALLEGRO_COLOR& color
) :
    GuiItem(true),
    text(text),
    font(font),
    color(color) {
    
    onDraw =
    [this] (const DrawInfo & draw) {
        this->defDrawCode(draw);
    };
}


/**
 * @brief Default button GUI item draw code.
 *
 * @param draw Information on how to draw.
 */
void ButtonGuiItem::defDrawCode(
    const DrawInfo& draw
) {
    drawButton(
        draw.center, draw.size,
        this->text, this->font, this->color, focused,
        getJuiceValue()
    );
}


/**
 * @brief Constructs a new check GUI item object.
 *
 * @param value Current value.
 * @param text Text to display on the checkbox.
 * @param font Font for the checkbox's text.
 * @param color Color of the checkbox's text.
 */
CheckGuiItem::CheckGuiItem(
    bool value, const string& text, ALLEGRO_FONT* font,
    const ALLEGRO_COLOR& color
) :
    GuiItem(true),
    value(value),
    text(text),
    font(font),
    color(color) {
    
    onDraw =
    [this] (const DrawInfo & draw) {
        this->defDrawCode(draw);
    };
    
    onActivate =
    [this] (const Point&) {
        this->defActivateCode();
    };
}


/**
 * @brief Constructs a new check GUI item object.
 *
 * @param valuePtr Pointer to the boolean that stores the current value.
 * @param text Text to display on the checkbox.
 * @param font Font for the checkbox's text.
 * @param color Color of the checkbox's text.
 */
CheckGuiItem::CheckGuiItem(
    bool* valuePtr, const string& text, ALLEGRO_FONT* font,
    const ALLEGRO_COLOR& color
) :
    CheckGuiItem(*valuePtr, text, font, color) {
    
    this->valuePtr = valuePtr;
}


/**
 * @brief Default check GUI item activation code.
 */
void CheckGuiItem::defActivateCode() {
    value = !value;
    if(valuePtr) (*valuePtr) = !(*valuePtr);
    startJuiceAnimation(JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM);
}


/**
 * @brief Default check GUI item draw code.
 *
 * @param draw Information on how to draw.
 */
void CheckGuiItem::defDrawCode(const DrawInfo& draw) {
    float juicyGrowAmount = getJuiceValue();
    drawText(
        this->text, this->font,
        Point(draw.center.x - draw.size.x * 0.45, draw.center.y),
        Point(draw.size.x * 0.95, draw.size.y) * GUI::STANDARD_CONTENT_SIZE,
        this->color, ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW,
        Point(1.0f + juicyGrowAmount)
    );
    
    drawBitmap(
        this->value ?
        game.sysContent.bmpCheckboxCheck :
        game.sysContent.bmpCheckboxNoCheck,
        this->text.empty() ?
        draw.center :
        Point((draw.center.x + draw.size.x * 0.5) - 40, draw.center.y),
        Point(32, -1)
    );
    
    ALLEGRO_COLOR boxTint =
        focused ? al_map_rgb(87, 200, 208) : COLOR_WHITE;
        
    drawTexturedBox(
        draw.center, draw.size, game.sysContent.bmpBubbleBox, boxTint
    );
}


/**
 * @brief Constructs a new GUI item object.
 *
 * @param focusable Can the item be focused by the player?
 */
GuiItem::GuiItem(bool focusable) :
    focusable(focusable) {
    
}


/**
 * @brief Activates the item.
 *
 * @param cursorPos Mouse cursor coordinates, if applicable.
 * @return Whether it could activate it.
 */
bool GuiItem::activate(const Point& cursorPos) {
    if(!onActivate) return false;
    onActivate(cursorPos);
    
    ALLEGRO_SAMPLE* sample =
        this == manager->backItem ?
        game.sysContent.sndMenuBack :
        playFailSound ?
        game.sysContent.sndMenuFail :
        game.sysContent.sndMenuActivate;
    game.audio.createUiSoundSource(sample, { .volume = 0.75f });
    playFailSound = false;
    
    return true;
}


/**
 * @brief Adds a child item.
 *
 * @param item Item to add as a child item.
 * @return Whether it succeeded.
 */
bool GuiItem::addChild(GuiItem* item) {
    if(!item) return false;
    children.push_back(item);
    item->parent = this;
    return true;
}


/**
 * @brief Removes and deletes all children items.
 *
 * @return Whether all of the deletions succeeded.
 */
bool GuiItem::deleteAllChildren() {
    bool success = true;
    while(!children.empty()) {
        GuiItem* iPtr = children[0];
        removeChild(iPtr);
        success &= manager->removeItem(iPtr);
        delete iPtr;
    }
    return success;
}


/**
 * @brief Returns the bottommost Y coordinate (in height ratio),
 * or the rightmost X coordinate (in width ratio),
 * of the item's children items.
 *
 * @param horizontal If true, check the horizontal reach (width). If false,
 * check the vertical reach (height).
 * @return The X or Y coordinate.
 */
float GuiItem::getChildrenSpan(bool horizontal) const {
    float span = 0.0f;
    for(size_t c = 0; c < children.size(); c++) {
        GuiItem* cPtr = children[c];
        float cCoord =
            horizontal ?
            cPtr->ratioCenter.x + (cPtr->ratioSize.x / 2.0f) :
            cPtr->ratioCenter.y + (cPtr->ratioSize.y / 2.0f);
        span = std::max(span, cCoord);
    }
    return span;
}


/**
 * @brief Returns the value related to the current juice animation.
 *
 * @return The juice value, or 0 if there's no animation.
 */
float GuiItem::getJuiceValue() const {
    switch(juiceType) {
    case JUICE_TYPE_GROW_TEXT_LOW: {
        float animRatio =
            1.0f - (juiceTimer / GUI::JUICY_GROW_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN, animRatio) *
            GUI::JUICY_GROW_TEXT_LOW_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_MEDIUM: {
        float animRatio =
            1.0f - (juiceTimer / GUI::JUICY_GROW_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN, animRatio) *
            GUI::JUICY_GROW_TEXT_MEDIUM_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_HIGH: {
        float animRatio =
            1.0f - (juiceTimer / GUI::JUICY_GROW_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN, animRatio) *
            GUI::JUICY_GROW_TEXT_HIGH_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_ELASTIC_LOW: {
        float animRatio =
            1.0f - (juiceTimer / GUI::JUICY_GROW_ELASTIC_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN_ELASTIC, animRatio) *
            GUI::JUICY_GROW_TEXT_LOW_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM: {
        float animRatio =
            1.0f - (juiceTimer / GUI::JUICY_GROW_ELASTIC_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN_ELASTIC, animRatio) *
            GUI::JUICY_GROW_TEXT_MEDIUM_MULT;
    }
    case JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH: {
        float animRatio =
            1.0f - (juiceTimer / GUI::JUICY_GROW_ELASTIC_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN_ELASTIC, animRatio) *
            GUI::JUICY_GROW_TEXT_HIGH_MULT;
    }
    case JUICE_TYPE_GROW_ICON: {
        float animRatio =
            1.0f - (juiceTimer / GUI::JUICY_GROW_DURATION);
        return
            ease(EASE_METHOD_UP_AND_DOWN, animRatio) *
            GUI::JUICY_GROW_ICON_MULT;
    }
    default: {
        return 0.0f;
    }
    }
}


/**
 * @brief Returns the reference center coordinates,
 * i.e. used when not animating.
 *
 * @return The center.
 */
Point GuiItem::getReferenceCenter() const {
    if(parent) {
        Point parentS =
            parent->getReferenceSize() - (parent->padding * 2.0f);
        Point parentC =
            parent->getReferenceCenter();
        Point result = ratioCenter * parentS;
        result.x += parentC.x - parentS.x / 2.0f;
        result.x -= parentS.x * parent->offset.x;
        result.y += parentC.y - parentS.y / 2.0f;
        result.y -= parentS.y * parent->offset.y;
        return result;
    } else {
        return Point(ratioCenter.x * game.winW, ratioCenter.y * game.winH);
    }
}


/**
 * @brief Returns the reference width and height, i.e. used when not animating.
 *
 * @return The size.
 */
Point GuiItem::getReferenceSize() const {
    Point mult;
    if(parent) {
        mult = parent->getReferenceSize() - (parent->padding * 2.0f);
    } else {
        mult.x = game.winW;
        mult.y = game.winH;
    }
    return ratioSize * mult;
}


/**
 * @brief Returns whether the mouse cursor is on top of it.
 *
 * @param cursorPos Position of the mouse cursor, in window coordinates.
 * @return Whether the cursor is on top.
 */
bool GuiItem::isMouseOn(const Point& cursorPos) const {
    if(parent && !parent->isMouseOn(cursorPos)) {
        return false;
    }
    
    Point c = getReferenceCenter();
    Point s = getReferenceSize();
    return
        (
            cursorPos.x >= c.x - s.x * 0.5 &&
            cursorPos.x <= c.x + s.x * 0.5 &&
            cursorPos.y >= c.y - s.y * 0.5 &&
            cursorPos.y <= c.y + s.y * 0.5
        );
}


/**
 * @brief Returns whether or not it is responsive, and also checks the parents.
 *
 * @return Whether it is responsive.
 */
bool GuiItem::isResponsive() const {
    if(parent) return parent->isResponsive();
    return responsive;
}


/**
 * @brief Returns whether or not it is visible, and also checks the parents.
 *
 * @return Whether it is visible.
 */
bool GuiItem::isVisible() const {
    if(!visible) return false;
    if(parent) return parent->isVisible();
    return true;
}


/**
 * @brief Removes an item from the list of children, without deleting it.
 *
 * @param item Child item to remove.
 * @return Whether it succeeded.
 */
bool GuiItem::removeChild(GuiItem* item) {
    bool success = false;
    for(size_t c = 0; c < children.size(); c++) {
        if(children[c] == item) {
            children.erase(children.begin() + c);
            item->parent = nullptr;
            success = true;
        }
    }
    return success;
}


/**
 * @brief Starts some juice animation.
 *
 * @param type Type of juice animation.
 * @return Whether it succeeded.
 */
bool GuiItem::startJuiceAnimation(JUICE_TYPE type) {
    juiceType = type;
    switch(type) {
    case JUICE_TYPE_GROW_TEXT_LOW:
    case JUICE_TYPE_GROW_TEXT_MEDIUM:
    case JUICE_TYPE_GROW_TEXT_HIGH:
    case JUICE_TYPE_GROW_ICON: {
        juiceTimer = GUI::JUICY_GROW_DURATION;
        return true;
        break;
    }
    case JUICE_TYPE_GROW_TEXT_ELASTIC_LOW:
    case JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM:
    case JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH: {
        juiceTimer = GUI::JUICY_GROW_ELASTIC_DURATION;
        return true;
        break;
    }
    default: {
        return false;
        break;
    }
    }
}


/**
 * @brief Constructs a new GUI manager object.
 */
GuiManager::GuiManager() :
    autoRepeater(&autoRepeaterSettings) {
    
    animTimer =
        Timer(
    0.0f, [this] () {
        switch(animType) {
        case GUI_MANAGER_ANIM_IN_TO_OUT:
        case GUI_MANAGER_ANIM_CENTER_TO_UP:
        case GUI_MANAGER_ANIM_CENTER_TO_DOWN:
        case GUI_MANAGER_ANIM_CENTER_TO_LEFT:
        case GUI_MANAGER_ANIM_CENTER_TO_RIGHT: {
            visible = false;
            break;
        }
        default: {
            visible = true;
            break;
        }
        }
    }
        );
}


/**
 * @brief Add an item to the list.
 *
 * @param item Pointer to the new item.
 * @param id If this item has an associated ID, specify it here.
 * Empty string if none.
 * @return Whether it succeeded.
 */
bool GuiManager::addItem(GuiItem* item, const string& id) {
    if(!item) return false;
    
    auto c = registeredCenters.find(id);
    if(c != registeredCenters.end()) {
        item->ratioCenter = c->second;
    }
    auto s = registeredSizes.find(id);
    if(s != registeredSizes.end()) {
        item->ratioSize = s->second;
    }
    
    items.push_back(item);
    item->manager = this;
    
    return true;
}


/**
 * @brief Destroys and deletes all items and information.
 *
 * @return Whether it succeeded.
 */
bool GuiManager::destroy() {
    setFocusedItem(nullptr);
    backItem = nullptr;
    for(size_t i = 0; i < items.size(); i++) {
        delete items[i];
    }
    items.clear();
    registeredCenters.clear();
    registeredSizes.clear();
    return true;
}


/**
 * @brief Draws all items.
 *
 * @return Whether it succeeded.
 */
bool GuiManager::draw() {
    if(!visible) return false;
    
    int ocrX = 0;
    int ocrY = 0;
    int ocrW = 0;
    int ocrH = 0;
    
    for(size_t i = 0; i < items.size(); i++) {
    
        GuiItem* iPtr = items[i];
        
        if(!iPtr->onDraw) continue;
        
        DrawInfo draw;
        draw.center = iPtr->getReferenceCenter();
        draw.size = iPtr->getReferenceSize();
        
        if(!getItemDrawInfo(iPtr, &draw)) continue;
        
        if(iPtr->parent) {
            DrawInfo parentDraw;
            if(!getItemDrawInfo(iPtr->parent, &parentDraw)) {
                continue;
            }
            al_get_clipping_rectangle(&ocrX, &ocrY, &ocrW, &ocrH);
            al_set_clipping_rectangle(
                (parentDraw.center.x - parentDraw.size.x / 2.0f) + 1,
                (parentDraw.center.y - parentDraw.size.y / 2.0f) + 1,
                parentDraw.size.x - 2,
                parentDraw.size.y - 2
            );
        }
        
        iPtr->onDraw(draw);
        
        if(iPtr->parent) {
            al_set_clipping_rectangle(ocrX, ocrY, ocrW, ocrH);
        }
    }
    
    if(focusCursorAlpha > 0.0f) {
        float sizeAddition =
            GUI::FOCUS_CURSOR_SIZE_ADDER +
            sin(game.timePassed * GUI::FOCUS_CURSOR_BOB_TIME_MULT) *
            GUI::FOCUS_CURSOR_BOB_OFFSET +
            GUI::FOCUS_CURSOR_FADE_GROW_OFFSET * (1.0f - focusCursorAlpha);
        drawTexturedBox(
            focusCursorPos, focusCursorSize + sizeAddition,
            game.sysContent.bmpFocusBox,
            mapAlpha(255 * ease(EASE_METHOD_OUT, focusCursorAlpha))
        );
    }
    
    return true;
}


/**
 * @brief Returns the currently focused item's tooltip, if any.
 *
 * @return The tooltip.
 */
string GuiManager::getCurrentTooltip() const {
    if(!focusedItem) return string();
    if(!focusedItem->onGetTooltip) return string();
    return focusedItem->onGetTooltip();
}


/**
 * @brief Returns a given item's drawing information.
 *
 * @param item What item to check.
 * @param draw Information on how to draw.
 * @return True if the item exists and is meant to be drawn, false otherwise.
 */
bool GuiManager::getItemDrawInfo(
    GuiItem* item, DrawInfo* draw
) const {
    if(!item->isVisible()) return false;
    if(item->ratioSize.x == 0.0f) return false;
    
    Point finalCenter = item->getReferenceCenter();
    Point finalSize = item->getReferenceSize();
    
    if(animTimer.timeLeft > 0.0f) {
        switch(animType) {
        case GUI_MANAGER_ANIM_OUT_TO_IN: {
            Point startCenter;
            float angle =
                getAngle(
                    Point(game.winW, game.winH) / 2.0f,
                    finalCenter
                );
            startCenter.x = finalCenter.x + cos(angle) * game.winW;
            startCenter.y = finalCenter.y + sin(angle) * game.winH;
            
            finalCenter.x =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, startCenter.x, finalCenter.x
                );
            finalCenter.y =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, startCenter.y, finalCenter.y
                );
            break;
            
        } case GUI_MANAGER_ANIM_IN_TO_OUT: {
            Point endCenter;
            float angle =
                getAngle(
                    Point(game.winW, game.winH) / 2.0f,
                    finalCenter
                );
            endCenter.x = finalCenter.x + cos(angle) * game.winW;
            endCenter.y = finalCenter.y + sin(angle) * game.winH;
            
            finalCenter.x =
                interpolateNumber(
                    ease(EASE_METHOD_IN, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.x, endCenter.x
                );
            finalCenter.y =
                interpolateNumber(
                    ease(EASE_METHOD_IN, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.y, endCenter.y
                );
            break;
            
        } case GUI_MANAGER_ANIM_UP_TO_CENTER: {
            finalCenter.y =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.y - game.winH, finalCenter.y
                );
            break;
            
        } case GUI_MANAGER_ANIM_CENTER_TO_UP: {
            finalCenter.y =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.y, finalCenter.y - game.winH
                );
            break;
            
        } case GUI_MANAGER_ANIM_DOWN_TO_CENTER: {
            finalCenter.y =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.y + game.winH, finalCenter.y
                );
            break;
            
        } case GUI_MANAGER_ANIM_CENTER_TO_DOWN: {
            finalCenter.y =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.y, finalCenter.y + game.winH
                );
            break;
            
        } case GUI_MANAGER_ANIM_LEFT_TO_CENTER: {
            finalCenter.x =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.x - game.winW, finalCenter.x
                );
            break;
            
        } case GUI_MANAGER_ANIM_CENTER_TO_LEFT: {
            finalCenter.x =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.x, finalCenter.x - game.winW
                );
            break;
            
        } case GUI_MANAGER_ANIM_RIGHT_TO_CENTER: {
            finalCenter.x =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.x + game.winW, finalCenter.x
                );
            break;
            
        } case GUI_MANAGER_ANIM_CENTER_TO_RIGHT: {
            finalCenter.x =
                interpolateNumber(
                    ease(EASE_METHOD_OUT, 1.0f - animTimer.getRatioLeft()),
                    0.0f, 1.0f, finalCenter.x, finalCenter.x + game.winW
                );
            break;
            
        } default: {
            break;
            
        }
        }
    }
    
    draw->center = finalCenter;
    draw->size = finalSize;
    return true;
}


/**
 * @brief Returns which item is currently focused.
 *
 * @return The item, or nullptr for none.
 */
GuiItem* GuiManager::getFocusedItem() const {
    return focusedItem;
}


/**
 * @brief Handle an Allegro event.
 * Controls are handled in handlePlayerAction.
 *
 * @param ev Event.
 * @return Whether it got handled.
 */
bool GuiManager::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    if(!responsive) return false;
    if(animTimer.getRatioLeft() > 0.0f && ignoreInputOnAnimation) return false;
    
    bool handled = false;
    bool mouseMoved = false;
    
    //Mousing over an item and clicking.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
    ) {
        GuiItem* focusResult = nullptr;
        for(size_t i = 0; i < items.size(); i++) {
            GuiItem* iPtr = items[i];
            if(
                iPtr->isMouseOn(Point(ev.mouse.x, ev.mouse.y)) &&
                iPtr->isResponsive() &&
                iPtr->focusable
            ) {
                focusResult = iPtr;
                if(iPtr->onMouseOver) {
                    iPtr->onMouseOver(Point(ev.mouse.x, ev.mouse.y));
                }
                break;
            }
        }
        setFocusedItem(focusResult);
        mouseMoved = true;
        handled = true;
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
        if(
            focusedItem &&
            focusedItem->isResponsive() &&
            focusedItem->onActivate
        ) {
            focusedItem->activate(Point(ev.mouse.x, ev.mouse.y));
            autoRepeater.start();
        }
        mouseMoved = true;
        handled = true;
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 1) {
        autoRepeater.stop();
        mouseMoved = true;
        handled = true;
    }
    
    for(size_t i = 0; i < items.size(); i++) {
        if(items[i]->isResponsive() && items[i]->onAllegroEvent) {
            items[i]->onAllegroEvent(ev);
        }
    }
    
    if(mouseMoved) lastInputWasMouse = true;
    
    return handled;
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 * @return Whether the action was consumed.
 */
bool GuiManager::handlePlayerAction(const Inpution::Action& action) {
    if(!responsive) {
        return false;
    }
    if(
        animTimer.getRatioLeft() > 0.0f &&
        ignoreInputOnAnimation
    ) {
        return false;
    }
    
    bool isDown = (action.value >= 0.5);
    bool buttonRecognized = true;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_MENU_RIGHT:
    case PLAYER_ACTION_TYPE_MENU_UP:
    case PLAYER_ACTION_TYPE_MENU_LEFT:
    case PLAYER_ACTION_TYPE_MENU_DOWN: {

        //Focusing a different item with the arrow keys.
        size_t pressed = PLAYER_ACTION_TYPE_NONE;
        
        switch(action.actionTypeId) {
        case PLAYER_ACTION_TYPE_MENU_RIGHT: {
            if(!rightPressed && isDown) {
                pressed = PLAYER_ACTION_TYPE_MENU_RIGHT;
            }
            rightPressed = isDown;
            break;
        } case PLAYER_ACTION_TYPE_MENU_UP: {
            if(!upPressed && isDown) {
                pressed = PLAYER_ACTION_TYPE_MENU_UP;
            }
            upPressed = isDown;
            break;
        } case PLAYER_ACTION_TYPE_MENU_LEFT: {
            if(!leftPressed && isDown) {
                pressed = PLAYER_ACTION_TYPE_MENU_LEFT;
            }
            leftPressed = isDown;
            break;
        } case PLAYER_ACTION_TYPE_MENU_DOWN: {
            if(!downPressed && isDown) {
                pressed = PLAYER_ACTION_TYPE_MENU_DOWN;
            }
            downPressed = isDown;
            break;
        } default: {
            break;
        }
        }
        
        if(pressed == PLAYER_ACTION_TYPE_NONE) break;
        
        if(!focusedItem) {
            for(size_t i = 0; i < items.size(); i++) {
                if(items[i]->isResponsive() && items[i]->focusable) {
                    setFocusedItem(items[i]);
                    break;
                }
            }
            if(focusedItem) {
                break;
            }
        }
        if(!focusedItem) {
            //No item can be focused.
            break;
        }
        
        vector<Point> focusables;
        vector<GuiItem*> focusablePtrs;
        size_t focusableIdx = INVALID;
        float direction = 0.0f;
        
        switch(pressed) {
        case PLAYER_ACTION_TYPE_MENU_DOWN: {
            direction = TAU * 0.25f;
            break;
        }
        case PLAYER_ACTION_TYPE_MENU_LEFT: {
            direction = TAU * 0.50f;
            break;
        }
        case PLAYER_ACTION_TYPE_MENU_UP: {
            direction = TAU * 0.75f;
            break;
        }
        }
        
        if(
            focusedItem &&
            focusedItem->isResponsive() &&
            focusedItem->onMenuDirButton
        ) {
            if(focusedItem->onMenuDirButton(pressed)) {
                //If it returned true, that means the following logic about
                //changing the current item needs to be skipped.
                break;
            }
        }
        
        float minY = 0;
        float maxY = game.winH;
        
        for(size_t i = 0; i < items.size(); i++) {
            GuiItem* iPtr = items[i];
            if(iPtr->isResponsive() && iPtr->focusable) {
                Point iCenter = iPtr->getReferenceCenter();
                if(iPtr == focusedItem) {
                    focusableIdx = focusables.size();
                }
                
                minY = std::min(minY, iCenter.y);
                maxY = std::max(maxY, iCenter.y);
                
                focusablePtrs.push_back(iPtr);
                focusables.push_back(iPtr->getReferenceCenter());
            }
        }
        
        size_t newFocusableIdx =
            focusNextItemDirectionally(
                focusables,
                focusableIdx,
                direction,
                Point(game.winW, maxY - minY)
            );
            
        if(newFocusableIdx != focusableIdx) {
            setFocusedItem(focusablePtrs[newFocusableIdx]);
            if(
                focusedItem->parent &&
                focusedItem->parent->onChildDirFocused
            ) {
                focusedItem->parent->onChildDirFocused(
                    focusedItem
                );
            }
        }
        
        break;
        
    } case PLAYER_ACTION_TYPE_MENU_OK: {
        if(
            isDown &&
            focusedItem &&
            focusedItem->onActivate &&
            focusedItem->isResponsive()
        ) {
            focusedItem->activate(Point(LARGE_FLOAT));
            autoRepeater.start();
        } else if(!isDown) {
            autoRepeater.stop();
        }
        break;
        
    } case PLAYER_ACTION_TYPE_MENU_BACK: {
        if(isDown && backItem && backItem->isResponsive()) {
            backItem->activate(Point(LARGE_FLOAT));
        }
        break;
        
    } default: {
        buttonRecognized = false;
        break;
        
    }
    }
    
    if(buttonRecognized) {
        lastInputWasMouse = false;
    }
    return buttonRecognized;
}


/**
 * @brief Hides all items until an animation shows them again.
 *
 * @return Whether it succeeded.
 */
bool GuiManager::hideItems() {
    if(!visible) return false;
    visible = false;
    return true;
}


/**
 * @brief Reads item default centers and sizes from a data node.
 *
 * @param node Data node to read from.
 * @return Whether it succeeded.
 */
bool GuiManager::readCoords(DataNode* node) {
    size_t nItems = node->getNrOfChildren();
    for(size_t i = 0; i < nItems; i++) {
        DataNode* itemNode = node->getChild(i);
        vector<string> words = split(itemNode->value);
        if(words.size() < 4) {
            continue;
        }
        registerCoords(
            itemNode->name,
            s2f(words[0]), s2f(words[1]), s2f(words[2]), s2f(words[3])
        );
    }
    return true;
}


/**
 * @brief Registers an item's default center and size.
 *
 * @param id String ID of the item.
 * @param cx Center X, in window percentage.
 * @param cy Center Y, in window percentage.
 * @param w Width, in window percentage.
 * @param h Height, in window percentage.
 * @return Whether it succeeded.
 */
bool GuiManager::registerCoords(
    const string& id, float cx, float cy, float w, float h
) {
    registeredCenters[id] = Point(cx / 100.0f, cy / 100.0f);
    registeredSizes[id] = Point(w / 100.0f, h / 100.0f);
    return true;
}


/**
 * @brief Removes an item from the list.
 *
 * @param item Item to remove.
 * @return Whether it succeeded.
 */
bool GuiManager::removeItem(GuiItem* item) {
    if(!item) return false;
    bool success = false;
    
    if(focusedItem == item) {
        setFocusedItem(nullptr);
    }
    if(backItem == item) {
        backItem = nullptr;
    }
    
    for(size_t i = 0; i < items.size(); i++) {
        if(items[i] == item) {
            items.erase(items.begin() + i);
            item->manager = nullptr;
            success = true;
        }
    }
    
    return success;
}


/**
 * @brief Sets the given item as the one that is focused, or none.
 *
 * @param item Item to focus, or nullptr for none.
 * @param silent If true, no sound effect will play.
 * Useful if you want the item to be focused not because of user input,
 * but because it's the default focused item when the GUI loads.
 * @return Whether it succeeded.
 */
bool GuiManager::setFocusedItem(GuiItem* item, bool silent) {
    if(focusedItem == item) return false;
    
    autoRepeater.stop();
    
    if(focusedItem) {
        focusedItem->focused = false;
    }
    focusedItem = item;
    if(focusedItem) {
        focusedItem->focused = true;
    }
    
    if(onFocusChanged) onFocusChanged();
    if(focusedItem) {
        if(focusedItem->onFocused) {
            focusedItem->onFocused();
        }
    }
    
    if(focusedItem && !silent) {
        game.audio.createUiSoundSource(
            game.sysContent.sndMenuFocus,
        { .stackMinPos = 0.01f, .volume = 0.5f, .speedDeviation = 0.1f }
        );
    }
    
    return true;
}


/**
 * @brief Shows all items, if they were hidden.
 *
 * @return Whether it succeeded.
 */
bool GuiManager::showItems() {
    if(visible) return false;
    visible = true;
    return true;
}


/**
 * @brief Starts an animation that affects all items.
 *
 * @param type Type of animation to start.
 * @param duration Total duration of the animation.
 * @return Whether it succeeded.
 */
bool GuiManager::startAnimation(
    const GUI_MANAGER_ANIM type, float duration
) {
    animType = type;
    animTimer.start(duration);
    visible = true;
    return true;
}


/**
 * @brief Ticks the time of all items by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 * @return Whether it succeeded.
 */
bool GuiManager::tick(float deltaT) {
    //Tick the animation.
    animTimer.tick(deltaT);
    
    //Tick all items.
    for(size_t i = 0; i < items.size(); i++) {
        GuiItem* iPtr = items[i];
        if(iPtr->onTick) {
            iPtr->onTick(deltaT);
        }
        if(iPtr->juiceTimer > 0) {
            iPtr->juiceTimer =
                std::max(0.0f, iPtr->juiceTimer - deltaT);
        } else {
            iPtr->juiceType = GuiItem::JUICE_TYPE_NONE;
        }
    }
    
    //Auto-repeat activations of the focused item, if applicable.
    size_t autoRepeatTriggers = autoRepeater.tick(deltaT);
    if(
        focusedItem &&
        focusedItem->canAutoRepeat && focusedItem->onActivate
    ) {
        for(size_t r = 0; r < autoRepeatTriggers; r++) {
            focusedItem->activate(Point(LARGE_FLOAT));
        }
    }
    
    //Tick the focus cursor.
    DrawInfo focusedItemDraw;
    bool mustDrawFocusedItem = false;
    if(focusedItem) {
        mustDrawFocusedItem = getItemDrawInfo(focusedItem, &focusedItemDraw);
    }
    if(
        focusedItem && mustDrawFocusedItem &&
        focusedItem->focusable
    ) {
        if(focusCursorAlpha == 0.0f) {
            focusCursorPos = focusedItemDraw.center;
            focusCursorSize = focusedItemDraw.size;
        } else {
            Point posDelta = focusedItemDraw.center - focusCursorPos;
            Point sizeDelta = focusedItemDraw.size - focusCursorSize;
            focusCursorPos +=
                posDelta * (GUI::FOCUS_CURSOR_SMOOTHNESS_MULT * deltaT);
            focusCursorSize +=
                sizeDelta * (GUI::FOCUS_CURSOR_SMOOTHNESS_MULT * deltaT);
        }
        focusCursorAlpha =
            inchTowards(
                focusCursorAlpha, 1.0f, GUI::FOCUS_CURSOR_ALPHA_SPEED * deltaT
            );
    } else {
        focusCursorAlpha =
            inchTowards(
                focusCursorAlpha, 0.0f, GUI::FOCUS_CURSOR_ALPHA_SPEED * deltaT
            );
    }
    
    return true;
}


/**
 * @brief Returns whether the last input was a mouse input.
 *
 * @return Whether it was a mouse input.
 */
bool GuiManager::wasLastInputMouse() const {
    return lastInputWasMouse;
}


/**
 * @brief Constructs a new list GUI item object.
 */
ListGuiItem::ListGuiItem() :
    GuiItem() {
    
    padding = 8.0f;
    onDraw =
    [this] (const DrawInfo & draw) {
        this->defDrawCode(draw);
    };
    onTick =
    [this] (float deltaT) {
        this->defTickCode(deltaT);
    };
    onAllegroEvent =
    [this] (const ALLEGRO_EVENT & ev) {
        this->defEventCode(ev);
    };
    onChildDirFocused =
    [this] (const GuiItem * child) {
        this->defChildDirFocusedCode(child);
    };
}


/**
 * @brief Default list GUI item child directionally focused code.
 *
 * @param child The child item.
 */
void ListGuiItem::defChildDirFocusedCode(const GuiItem* child) {
    //Try to center the child.
    float childrenSpan = getChildrenSpan(horizontal);
    float* offsetPtr = !horizontal ? &offset.y : &offset.x;
    float coord = !horizontal ? child->ratioCenter.y : child->ratioCenter.x;
    
    if(childrenSpan <= 1.0f && *offsetPtr == 0.0f) return;
    targetOffset = std::clamp(coord - 0.5f, 0.0f, childrenSpan - 1.0f);
}


/**
 * @brief Default list GUI item draw code.
 *
 * @param draw Information on how to draw.
 */
void ListGuiItem::defDrawCode(const DrawInfo& draw) {
    drawTexturedBox(
        draw.center, draw.size, game.sysContent.bmpFrameBox,
        COLOR_TRANSPARENT_WHITE
    );
    if(offset.y > 0.0f && !horizontal) {
        //Shade effect at the top.
        ALLEGRO_VERTEX vertexes[8];
        for(size_t v = 0; v < 8; v++) {
            vertexes[v].z = 0.0f;
        }
        float y1 = draw.center.y - draw.size.y / 2.0f;
        float y2 = y1 + 20.0f;
        ALLEGRO_COLOR cOpaque = al_map_rgba(255, 255, 255, 64);
        ALLEGRO_COLOR cEmpty = al_map_rgba(255, 255, 255, 0);
        vertexes[0].x = draw.center.x - draw.size.x * 0.49;
        vertexes[0].y = y1;
        vertexes[0].color = cEmpty;
        vertexes[1].x = draw.center.x - draw.size.x * 0.49;
        vertexes[1].y = y2;
        vertexes[1].color = cEmpty;
        vertexes[2].x = draw.center.x - draw.size.x * 0.47;
        vertexes[2].y = y1;
        vertexes[2].color = cOpaque;
        vertexes[3].x = draw.center.x - draw.size.x * 0.47;
        vertexes[3].y = y2;
        vertexes[3].color = cEmpty;
        vertexes[4].x = draw.center.x + draw.size.x * 0.47;
        vertexes[4].y = y1;
        vertexes[4].color = cOpaque;
        vertexes[5].x = draw.center.x + draw.size.x * 0.47;
        vertexes[5].y = y2;
        vertexes[5].color = cEmpty;
        vertexes[6].x = draw.center.x + draw.size.x * 0.49;
        vertexes[6].y = y1;
        vertexes[6].color = cEmpty;
        vertexes[7].x = draw.center.x + draw.size.x * 0.49;
        vertexes[7].y = y2;
        vertexes[7].color = cEmpty;
        al_draw_prim(
            vertexes, nullptr, nullptr, 0, 8, ALLEGRO_PRIM_TRIANGLE_STRIP
        );
    }
    float childrenVSpan = getChildrenSpan();
    if(
        childrenVSpan > 1.0f && offset.y < childrenVSpan - 1.0f && !horizontal
    ) {
        //Shade effect at the bottom.
        ALLEGRO_VERTEX vertexes[8];
        for(size_t v = 0; v < 8; v++) {
            vertexes[v].z = 0.0f;
        }
        float y1 = draw.center.y + draw.size.y / 2.0f;
        float y2 = y1 - 20.0f;
        ALLEGRO_COLOR cOpaque = al_map_rgba(255, 255, 255, 64);
        ALLEGRO_COLOR cEmpty = al_map_rgba(255, 255, 255, 0);
        vertexes[0].x = draw.center.x - draw.size.x * 0.49;
        vertexes[0].y = y1;
        vertexes[0].color = cEmpty;
        vertexes[1].x = draw.center.x - draw.size.x * 0.49;
        vertexes[1].y = y2;
        vertexes[1].color = cEmpty;
        vertexes[2].x = draw.center.x - draw.size.x * 0.47;
        vertexes[2].y = y1;
        vertexes[2].color = cOpaque;
        vertexes[3].x = draw.center.x - draw.size.x * 0.47;
        vertexes[3].y = y2;
        vertexes[3].color = cEmpty;
        vertexes[4].x = draw.center.x + draw.size.x * 0.47;
        vertexes[4].y = y1;
        vertexes[4].color = cOpaque;
        vertexes[5].x = draw.center.x + draw.size.x * 0.47;
        vertexes[5].y = y2;
        vertexes[5].color = cEmpty;
        vertexes[6].x = draw.center.x + draw.size.x * 0.49;
        vertexes[6].y = y1;
        vertexes[6].color = cEmpty;
        vertexes[7].x = draw.center.x + draw.size.x * 0.49;
        vertexes[7].y = y2;
        vertexes[7].color = cEmpty;
        al_draw_prim(
            vertexes, nullptr, nullptr, 0, 8, ALLEGRO_PRIM_TRIANGLE_STRIP
        );
    }
    
    if(offset.x > 0.0f && horizontal) {
        //Shade effect at the left.
        ALLEGRO_VERTEX vertexes[8];
        for(size_t v = 0; v < 8; v++) {
            vertexes[v].z = 0.0f;
        }
        float x1 = draw.center.x - draw.size.x / 2.0f;
        float x2 = x1 + 20.0f;
        ALLEGRO_COLOR cOpaque = al_map_rgba(255, 255, 255, 64);
        ALLEGRO_COLOR cEmpty = al_map_rgba(255, 255, 255, 0);
        vertexes[0].x = x1;
        vertexes[0].y = draw.center.y - draw.size.y * 0.49;
        vertexes[0].color = cEmpty;
        vertexes[1].x = x2;
        vertexes[1].y = draw.center.y - draw.size.y * 0.49;
        vertexes[1].color = cEmpty;
        vertexes[2].x = x1;
        vertexes[2].y = draw.center.y - draw.size.y * 0.47;
        vertexes[2].color = cOpaque;
        vertexes[3].x = x2;
        vertexes[3].y = draw.center.y - draw.size.y * 0.47;
        vertexes[3].color = cEmpty;
        vertexes[4].x = x1;
        vertexes[4].y = draw.center.y + draw.size.y * 0.47;
        vertexes[4].color = cOpaque;
        vertexes[5].x = x2;
        vertexes[5].y = draw.center.y + draw.size.y * 0.47;
        vertexes[5].color = cEmpty;
        vertexes[6].x = x1;
        vertexes[6].y = draw.center.y + draw.size.y * 0.49;
        vertexes[6].color = cEmpty;
        vertexes[7].x = x2;
        vertexes[7].y = draw.center.y + draw.size.y * 0.49;
        vertexes[7].color = cEmpty;
        al_draw_prim(
            vertexes, nullptr, nullptr, 0, 8, ALLEGRO_PRIM_TRIANGLE_STRIP
        );
    }
    float childrenHSpan = getChildrenSpan(true);
    if(childrenHSpan > 1.0f && offset.x < childrenHSpan - 1.0f && horizontal) {
        //Shade effect at the right.
        ALLEGRO_VERTEX vertexes[8];
        for(size_t v = 0; v < 8; v++) {
            vertexes[v].z = 0.0f;
        }
        float x1 = draw.center.x + draw.size.x / 2.0f;
        float x2 = x1 - 20.0f;
        ALLEGRO_COLOR cOpaque = al_map_rgba(255, 255, 255, 64);
        ALLEGRO_COLOR cEmpty = al_map_rgba(255, 255, 255, 0);
        vertexes[0].x = x1;
        vertexes[0].y = draw.center.y - draw.size.y * 0.49;
        vertexes[0].color = cEmpty;
        vertexes[1].x = x2;
        vertexes[1].y = draw.center.y - draw.size.y * 0.49;
        vertexes[1].color = cEmpty;
        vertexes[2].x = x1;
        vertexes[2].y = draw.center.y - draw.size.y * 0.47;
        vertexes[2].color = cOpaque;
        vertexes[3].x = x2;
        vertexes[3].y = draw.center.y - draw.size.y * 0.47;
        vertexes[3].color = cEmpty;
        vertexes[4].x = x1;
        vertexes[4].y = draw.center.y + draw.size.y * 0.47;
        vertexes[4].color = cOpaque;
        vertexes[5].x = x2;
        vertexes[5].y = draw.center.y + draw.size.y * 0.47;
        vertexes[5].color = cEmpty;
        vertexes[6].x = x1;
        vertexes[6].y = draw.center.y + draw.size.y * 0.49;
        vertexes[6].color = cEmpty;
        vertexes[7].x = x2;
        vertexes[7].y = draw.center.y + draw.size.y * 0.49;
        vertexes[7].color = cEmpty;
        al_draw_prim(
            vertexes, nullptr, nullptr, 0, 8, ALLEGRO_PRIM_TRIANGLE_STRIP
        );
    }
}


/**
 * @brief Default list GUI item event code.
 *
 * @param ev The Allegro event.
 */
void ListGuiItem::defEventCode(const ALLEGRO_EVENT& ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES &&
        isMouseOn(Point(ev.mouse.x, ev.mouse.y)) &&
        ev.mouse.dz != 0.0f
    ) {
        float childrenSpan = getChildrenSpan(horizontal);
        if(childrenSpan <= 1.0f && offset.y == 0.0f) {
            return;
        }
        targetOffset =
            std::clamp(
                targetOffset + (-ev.mouse.dz) * 0.2f,
                0.0f,
                childrenSpan - 1.0f
            );
    }
}


/**
 * @brief Default list GUI item tick code.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void ListGuiItem::defTickCode(float deltaT) {
    float childrenSpan = getChildrenSpan(horizontal);
    float* offsetPtr = !horizontal ? &offset.y : &offset.x;
    if(childrenSpan < 1.0f) {
        targetOffset = 0.0f;
        *offsetPtr = 0.0f;
    } else {
        targetOffset = std::clamp(targetOffset, 0.0f, childrenSpan - 1.0f);
        *offsetPtr += (targetOffset - *offsetPtr) * (10.0f * deltaT);
        *offsetPtr = std::clamp(*offsetPtr, 0.0f, childrenSpan - 1.0f);
        if(*offsetPtr <= 0.01f) *offsetPtr = 0.0f;
        if(childrenSpan > 1.0f) {
            if(childrenSpan - *offsetPtr - 1.0f <= 0.01f) {
                *offsetPtr = childrenSpan - 1.0f;
            }
        }
    }
}


/**
 * @brief Constructs a new picker GUI item object.
 *
 * @param baseText Text to display before the current option's name.
 * @param option Text that matches the current option.
 * @param nrOptions Total amount of options.
 * @param curOptionIdx Index of the currently chosen option.
 */
PickerGuiItem::PickerGuiItem(
    const string& baseText, const string& option,
    size_t nrOptions, size_t curOptionIdx
) :
    GuiItem(true),
    baseText(baseText),
    option(option),
    nrOptions(nrOptions),
    curOptionIdx(curOptionIdx) {
    
    onDraw =
    [this] (const DrawInfo & draw) {
        this->defDrawCode(draw);
    };
    
    onActivate =
    [this] (const Point & cursorPos) {
        this->defActivateCode(cursorPos);
    };
    
    onMenuDirButton =
    [this] (size_t playerActionId) -> bool{
        return this->defMenuDirCode(playerActionId);
    };
    
    onMouseOver =
    [this] (const Point & cursorPos) {
        this->defMouseOverCode(cursorPos);
    };
}


/**
 * @brief Default picker GUI item activate code.
 *
 * @param cursorPos Mouse cursor position.
 */
void PickerGuiItem::defActivateCode(const Point& cursorPos) {
    if(cursorPos.x >= getReferenceCenter().x) {
        onNext();
    } else {
        onPrevious();
    }
}


/**
 * @brief Default picker GUI item draw code.
 *
 * @param draw Information on how to draw.
 */
void PickerGuiItem::defDrawCode(const DrawInfo& draw) {
    if(this->nrOptions != 0 && focused) {
        Point optionBoxesStart(
            draw.center.x - draw.size.x / 2.0f + 20.0f,
            draw.center.y + draw.size.y / 2.0f - 12.0f
        );
        float optionBoxesInterval =
            (draw.size.x - 40.0f) / (this->nrOptions - 0.5f);
        for(size_t o = 0; o < this->nrOptions; o++) {
            float x1 = optionBoxesStart.x + o * optionBoxesInterval;
            float y1 = optionBoxesStart.y;
            al_draw_filled_rectangle(
                x1, y1,
                x1 + optionBoxesInterval * 0.5f, y1 + 4.0f,
                this->curOptionIdx == o ?
                al_map_rgba(255, 255, 255, 160) :
                al_map_rgba(255, 255, 255, 64)
            );
        }
    }
    
    unsigned char realArrowHighlight = 255;
    if(
        focused &&
        manager &&
        manager->wasLastInputMouse()
    ) {
        realArrowHighlight = arrowHighlight;
    }
    ALLEGRO_COLOR arrowHighlightColor = al_map_rgb(87, 200, 208);
    ALLEGRO_COLOR arrowRegularColor = COLOR_WHITE;
    Point arrowHighlightScale = Point(1.4f);
    Point arrowRegularScale = Point(1.0f);
    
    Point arrowBox(
        draw.size.x * 0.10 * GUI::STANDARD_CONTENT_SIZE.x,
        draw.size.y * GUI::STANDARD_CONTENT_SIZE.y
    );
    drawText(
        "<",
        game.sysContent.fntStandard,
        Point(draw.center.x - draw.size.x * 0.45, draw.center.y),
        arrowBox,
        realArrowHighlight == 0 ?
        arrowHighlightColor :
        arrowRegularColor,
        ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW,
        realArrowHighlight == 0 ?
        arrowHighlightScale :
        arrowRegularScale
    );
    drawText(
        ">",
        game.sysContent.fntStandard,
        Point(draw.center.x + draw.size.x * 0.45, draw.center.y),
        arrowBox,
        realArrowHighlight == 1 ?
        arrowHighlightColor :
        arrowRegularColor,
        ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW,
        realArrowHighlight == 1 ?
        arrowHighlightScale :
        arrowRegularScale
    );
    
    float juicyGrowAmount = this->getJuiceValue();
    
    Point textBox(
        draw.size.x * 0.80, draw.size.y * GUI::STANDARD_CONTENT_SIZE.y
    );
    drawText(
        this->baseText + this->option,
        game.sysContent.fntStandard,
        Point(draw.center.x - draw.size.x * 0.40, draw.center.y),
        textBox,
        COLOR_WHITE,
        ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW,
        Point(1.0f + juicyGrowAmount)
    );
    
    ALLEGRO_COLOR boxTint =
        focused ? al_map_rgb(87, 200, 208) : COLOR_WHITE;
        
    drawTexturedBox(
        draw.center, draw.size, game.sysContent.bmpBubbleBox, boxTint
    );
}


/**
 * @brief Default picker GUI item menu dir code.
 *
 * @param actionId ID of the player action.
 */
bool PickerGuiItem::defMenuDirCode(size_t actionId) {
    if(actionId == PLAYER_ACTION_TYPE_MENU_RIGHT) {
        onNext();
        return true;
    } else if(actionId == PLAYER_ACTION_TYPE_MENU_LEFT) {
        onPrevious();
        return true;
    }
    return false;
}


/**
 * @brief Default picker GUI item mouse over code.
 *
 * @param cursorPos Mouse cursor position.
 */
void PickerGuiItem::defMouseOverCode(const Point& cursorPos) {
    arrowHighlight =
        cursorPos.x >= getReferenceCenter().x ? 1 : 0;
}


/**
 * @brief Constructs a new scroll GUI item object.
 */
ScrollGuiItem::ScrollGuiItem() :
    GuiItem() {
    
    onDraw =
    [this] (const DrawInfo & draw) {
        this->defDrawCode(draw);
    };
    onAllegroEvent =
    [this] (const ALLEGRO_EVENT & ev) {
        this->defEventCode(ev);
    };
}


/**
 * @brief Default scroll GUI item draw code.
 *
 * @param draw Information on how to draw.
 */
void ScrollGuiItem::defDrawCode(const DrawInfo& draw) {
    if(!horizontal) {
        float barY = 0.0f; //Top, in height ratio.
        float barH = 0.0f; //In height ratio.
        float listBottom = listItem->getChildrenSpan();
        unsigned char alpha = 48;
        if(listBottom > 1.0f) {
            float offset = std::min(listItem->offset.y, listBottom - 1.0f);
            barY = offset / listBottom;
            barH = 1.0f / listBottom;
            alpha = 128;
        }
        
        drawTexturedBox(
            draw.center, draw.size, game.sysContent.bmpFrameBox,
            al_map_rgba(255, 255, 255, alpha)
        );
        
        if(barH != 0.0f) {
            drawTexturedBox(
                Point(
                    draw.center.x,
                    (draw.center.y - draw.size.y * 0.5f) +
                    (draw.size.y * barY) +
                    (draw.size.y * barH * 0.5f)
                ),
                Point(draw.size.x, (draw.size.y * barH)),
                game.sysContent.bmpBubbleBox
            );
        }
    } else {
        float barX = 0.0f; //Left, in width ratio.
        float barW = 0.0f; //In width ratio.
        float listDepth = listItem->getChildrenSpan(true);
        unsigned char alpha = 48;
        if(listDepth > 1.0f) {
            float offset = std::min(listItem->offset.x, listDepth - 1.0f);
            barX = offset / listDepth;
            barW = 1.0f / listDepth;
            alpha = 128;
        }
        
        drawTexturedBox(
            draw.center, draw.size, game.sysContent.bmpFrameBox,
            al_map_rgba(255, 255, 255, alpha)
        );
        
        if(barW != 0.0f) {
            drawTexturedBox(
                Point(
                    (draw.center.x - draw.size.x * 0.5f) +
                    (draw.size.x * barX) +
                    (draw.size.x * barW * 0.5f),
                    draw.center.y
                ),
                Point((draw.size.x * barW), draw.size.y),
                game.sysContent.bmpBubbleBox
            );
        }
    }
}


/**
 * @brief Default scroll GUI item event code.
 *
 * @param ev The Allegro event.
 */
void ScrollGuiItem::defEventCode(const ALLEGRO_EVENT& ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        ev.mouse.button == 1 &&
        isMouseOn(Point(ev.mouse.x, ev.mouse.y))
    ) {
        isMouseDragging = true;
        setOffsetFromMouse(ev.mouse.x, ev.mouse.y);
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP &&
        ev.mouse.button == 1
    ) {
        isMouseDragging = false;
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES &&
        isMouseDragging
    ) {
        setOffsetFromMouse(ev.mouse.x, ev.mouse.y);
    }
}


/**
 * @brief Sets the list box's offset based on where the mouse cursor is
 * pointing in the scroll item.
 *
 * @param x Mouse's X coordinate, in window coordinates.
 * @param y Mouse's Y coordinate, in window coordinates.
 */
void ScrollGuiItem::setOffsetFromMouse(float x, float y) {
    if(!horizontal) {
        float listBottom = listItem->getChildrenSpan();
        isMouseDragging = true;
        
        if(listBottom <= 1.0f) {
            return;
        }
        
        Point c = getReferenceCenter();
        Point s = getReferenceSize();
        float barH = (1.0f / listBottom) * s.y;
        float y1 = (c.y - s.y / 2.0f) + barH / 2.0f;
        float y2 = (c.y + s.y / 2.0f) - barH / 2.0f;
        float click = (y - y1) / (y2 - y1);
        click = std::clamp(click, 0.0f, 1.0f);
        
        listItem->targetOffset = click * (listBottom - 1.0f);
        
    } else {
        float listDepth = listItem->getChildrenSpan(true);
        isMouseDragging = true;
        
        if(listDepth <= 1.0f) {
            return;
        }
        
        Point c = getReferenceCenter();
        Point s = getReferenceSize();
        float barW = (1.0f / listDepth) * s.x;
        float x1 = (c.x - s.x / 2.0f) + barW / 2.0f;
        float x2 = (c.x + s.x / 2.0f) - barW / 2.0f;
        float click = (x - x1) / (x2 - x1);
        click = std::clamp(click, 0.0f, 1.0f);
        
        listItem->targetOffset = click * (listDepth - 1.0f);
        
    }
}


/**
 * @brief Constructs a new text GUI item object.
 *
 * @param text Text to display.
 * @param font Font to use for the text.
 * @param color Color to use for the text.
 * @param flags Allegro text flags to use.
 */
TextGuiItem::TextGuiItem(
    const string& text, ALLEGRO_FONT* font, const ALLEGRO_COLOR& color,
    int flags
) :
    GuiItem(),
    text(text),
    font(font),
    color(color),
    flags(flags) {
    
    onDraw =
    [this] (const DrawInfo & draw) {
        this->defDrawCode(draw);
    };
}


/**
 * @brief Default text GUI item draw code.
 *
 * @param draw Information on how to draw.
 */
void TextGuiItem::defDrawCode(const DrawInfo& draw) {
    int textX = draw.center.x;
    switch(this->flags) {
    case ALLEGRO_ALIGN_LEFT: {
        textX = draw.center.x - draw.size.x * 0.5;
        break;
    } case ALLEGRO_ALIGN_RIGHT: {
        textX = draw.center.x + draw.size.x * 0.5;
        break;
    }
    }
    
    float juicyGrowAmount = getJuiceValue();
    int textY = draw.center.y;
    
    if(lineWrap) {
    
        textY = draw.center.y - draw.size.y / 2.0f;
        int lineHeight = al_get_font_line_height(this->font);
        vector<StringToken> tokens =
            tokenizeString(this->text);
        setStringTokenWidths(
            tokens, this->font, game.sysContent.fntSlim, lineHeight, false
        );
        vector<vector<StringToken> > tokensPerLine =
            splitLongStringWithTokens(tokens, draw.size.x);
            
        for(size_t l = 0; l < tokensPerLine.size(); l++) {
            drawStringTokens(
                tokensPerLine[l], this->font, game.sysContent.fntSlim,
                false,
                Point(
                    textX,
                    textY + l * lineHeight
                ),
                this->flags,
                Point(draw.size.x, lineHeight),
                Point(1.0f + juicyGrowAmount)
            );
        }
        
    } else {
    
        drawText(
            this->text, this->font, Point(textX, textY), draw.size,
            this->color, this->flags, V_ALIGN_MODE_CENTER,
            TEXT_SETTING_FLAG_CANT_GROW,
            Point(1.0 + juicyGrowAmount)
        );
        
    }
}


/**
 * @brief Constructs a new tooltip GUI item object.
 *
 * @param gui Pointer to the GUI it belongs to.
 */
TooltipGuiItem::TooltipGuiItem(GuiManager* gui) :
    GuiItem(),
    gui(gui) {
    
    onDraw =
    [this] (const DrawInfo & draw) {
        this->defDrawCode(draw);
    };
}


/**
 * @brief Default tooltip GUI item draw code.
 *
 * @param draw Information on how to draw.
 */
void TooltipGuiItem::defDrawCode(const DrawInfo& draw) {
    string curText = this->gui->getCurrentTooltip();
    if(curText != this->prevText) {
        this->startJuiceAnimation(JUICE_TYPE_GROW_TEXT_LOW);
        this->prevText = curText;
    }
    float juicyGrowAmount = getJuiceValue();
    drawText(
        curText, game.sysContent.fntStandard,
        draw.center, draw.size,
        COLOR_WHITE, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
        TEXT_SETTING_FLAG_CANT_GROW,
        Point(0.7f + juicyGrowAmount)
    );
}
