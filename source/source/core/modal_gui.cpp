/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * GUI manager with a modal dialog.
 */


#include <allegro5/allegro_primitives.h>

#include "game.h"
#include "misc_functions.h"
#include "modal_gui.h"


using DrawInfo = GuiItem::DrawInfo;


namespace MODAL {

//Background opacity [0 - 1].
const float BG_OPACITY = 0.8f;

//Space between each button.
const float BUTTON_MARGIN = 0.05f;

//Blink interval for the text input caret.
const float CARET_BLINK_INTERVAL = 0.8f;

//How long the fade transition takes.
const float FADE_DURATION = 0.3f;

//Name of the GUI definition file.
const string GUI_FILE_NAME = "modal";

//Maximum number of characters for the text input.
const size_t TEXT_INPUT_MAX_SIZE = 50;

};


/**
 * @brief Constructs a new modal GUI manager object.
 */
ModalGuiManager::ModalGuiManager() {
    reset();
    hideItems();
    responsive = false;
}


/**
 * @brief Closes the modal.
 */
void ModalGuiManager::close() {
    startAnimation(GUI_MANAGER_ANIM_FADE_OUT, MODAL::FADE_DURATION);
    responsive = false;
}


/**
 * @brief Draws the manager.
 */
void ModalGuiManager::draw() {
    if(!visible) return;
    
    float opacityMult;
    if(animType == GUI_MANAGER_ANIM_FADE_OUT) {
        opacityMult = animTimer.getRatioLeft();
    } else {
        opacityMult = 1.0f - animTimer.getRatioLeft();
    }
    
    al_draw_filled_rectangle(
        0, 0, game.winW, game.winH,
        multAlpha(game.config.guiColors.pauseBg, opacityMult)
    );
    drawBitmap(
        game.sysContent.bmpVignette,
        Point(game.winW, game.winH) / 2.0f, Point(game.winW, game.winH), 0.0f,
        multAlpha(game.config.guiColors.pauseVignette, opacityMult)
    );
    
    GuiManager::draw();
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The event.
 * @return Whether it got handled.
 */
bool ModalGuiManager::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    bool handled = false;
    
    if(useTextInput && ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        if(ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
            if(!textInput.empty()) {
                textInput.pop_back();
            }
            handled = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_ENTER ||
            ev.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER
        ) {
            buttonItems[textInputEnterButtonIdx]->activate();
            handled = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE ||
            ev.keyboard.keycode == ALLEGRO_KEY_TAB
        ) {
            //Do nothing.
            
        } else if(ev.keyboard.unichar != 0) {
            if(textInput.size() < MODAL::TEXT_INPUT_MAX_SIZE) {
                textInput.push_back(ev.keyboard.unichar);
            }
            handled = true;
            
        }
    }
    
    handled |= GuiManager::handleAllegroEvent(ev);
    
    return handled;
}


/**
 * @brief Returns whether the modal is currently active.
 *
 * @return Whether it is active.
 */
bool ModalGuiManager::isActive() const {
    return visible;
}


/**
 * @brief Opens the modal.
 */
void ModalGuiManager::open() {
    showItems();
    startAnimation(GUI_MANAGER_ANIM_FADE_IN, MODAL::FADE_DURATION);
    responsive = true;
}


/**
 * @brief Resets the properties that control what the model contains.
 */
void ModalGuiManager::reset() {
    title.clear();
    prompt.clear();
    back = "Back";
    backTooltip = "Cancel.";
    extraButtons.clear();
    defaultFocusButtonIdx = 0;
    textInputEnterButtonIdx = 0;
    useTextInput = false;
    textInput.clear();
    setFocusedItem(nullptr);
    focusCursor.alpha = 0.0f;
}


/**
 * @brief Updates the GUI items based on the title, prompt,
 * and button properties of this modal.
 */
void ModalGuiManager::updateItems() {
    //Delete the old ones.
    destroy();
    buttonItems.clear();
    
    //Default coordinates.
    DataNode* guiFile =
        &game.content.guiDefs.list[MODAL::GUI_FILE_NAME];
    registerCoords("title",       50,  9, 92, 10);
    registerCoords("prompt",      50, 50, 92, 32);
    registerCoords("text_input",  50, 50, 92, 32);
    registerCoords("button_area", 50, 83, 92, 10);
    registerCoords("back_input",   5, 87,  4,  4);
    registerCoords("tooltip",     50, 96, 96,  4);
    readDataFile(guiFile);
    
    //Title text.
    TextGuiItem* titleItem = new TextGuiItem(
        title, game.sysContent.fntAreaName
    );
    addItem(titleItem, "title");
    
    //Prompt text.
    TextGuiItem* promptItem = new TextGuiItem(
        prompt, game.sysContent.fntStandard
    );
    promptItem->lineWrap = true;
    addItem(promptItem, "prompt");
    
    //Text input.
    if(useTextInput) {
        TextGuiItem* textInputItem = new TextGuiItem(
            "", game.sysContent.fntStandard, game.config.guiColors.gold
        );
        textInputItem->onDraw =
        [textInputItem, this] (const DrawInfo & draw) {
            textInputItem->text = textInput;
            textInputItem->defDrawCode(draw);
            float t = fmod(game.timePassed, MODAL::CARET_BLINK_INTERVAL);
            if(t < MODAL::CARET_BLINK_INTERVAL / 2.0f) {
                int textWidth =
                    al_get_text_width(
                        game.sysContent.fntStandard, textInput.c_str()
                    );
                int textHeight =
                    al_get_font_line_height(game.sysContent.fntStandard);
                textWidth = std::min((float) textWidth, draw.size.x);
                al_draw_line(
                    draw.center.x + textWidth / 2.0f,
                    draw.center.y - textHeight / 2.0f,
                    draw.center.x + textWidth / 2.0f,
                    draw.center.y + textHeight / 2.0f,
                    game.config.guiColors.gold, 2.0f
                );
            }
        };
        addItem(textInputItem, "text_input");
    }
    
    //Back button.
    backItem =
        new ButtonGuiItem(
        back, game.sysContent.fntStandard, game.config.guiColors.back
    );
    backItem->onGetTooltip =
    [this] () {
        return backTooltip;
    };
    backItem->onActivate =
    [this] (const Point&) {
        if(onBack) onBack();
        close();
    };
    addItem(backItem, "button_area");
    buttonItems.push_back(backItem);
    
    //Back input icon.
    guiCreateBackInputIcon(this);
    
    forIdx(b, extraButtons) {
        //Extra button.
        ButtonGuiItem* button =
            new ButtonGuiItem(
            extraButtons[b].text, game.sysContent.fntStandard,
            extraButtons[b].color
        );
        button->onActivate = [this, b] (const Point & cursorPos) {
            close();
            if(extraButtons[b].onActivate) {
                extraButtons[b].onActivate(cursorPos);
            }
        };
        button->onGetTooltip =
        [this, b] () {
            return extraButtons[b].tooltip;
        };
        addItem(button, "button_area");
        buttonItems.push_back(button);
    }
    
    //Tooltip text.
    TooltipGuiItem* tooltipItem = new TooltipGuiItem(this);
    addItem(tooltipItem, "tooltip");
    
    //Position the buttons.
    Point buttonAreaCenter = backItem->ratioCenter;
    Point buttonAreaSize = backItem->ratioSize;
    const float totalEmptySpace =
        MODAL::BUTTON_MARGIN * (buttonItems.size() - 1);
    const float buttonWidth =
        (buttonAreaSize.x - totalEmptySpace) / buttonItems.size();
    float curX = buttonAreaCenter.x - buttonAreaSize.x / 2.0f;
    
    forIdx(b, buttonItems) {
        buttonItems[b]->ratioCenter.x = curX + buttonWidth / 2.0f;
        buttonItems[b]->ratioSize.x = buttonWidth;
        curX += buttonWidth + MODAL::BUTTON_MARGIN;
    }
    
    //Finishing touches.
    setFocusedItem(buttonItems[defaultFocusButtonIdx], true);
}
