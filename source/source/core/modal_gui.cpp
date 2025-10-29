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
#include "modal_gui.h"
#include "misc_functions.h"


using DrawInfo = GuiItem::DrawInfo;


namespace MODAL {

//Background opacity [0 - 1].
const float BG_OPACITY = 0.8f;

//Space between each button.
const float BUTTON_MARGIN = 0.05f;

//How long the fade transition takes.
const float FADE_DURATION = 0.3f;

//Name of the GUI definition file.
const string GUI_FILE_NAME = "modal";

};


/**
 * @brief Constructs a new modal GUI manager object.
 */
ModalGuiManager::ModalGuiManager() {
    reset();
    hideItems();
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
    registerCoords("button_area", 50, 83, 92, 10);
    registerCoords("back_input",   5, 87,  4,  4);
    registerCoords("tooltip",     50, 96, 96,  4);
    readDataFile(guiFile);
    
    //Title text.
    titleItem = new TextGuiItem(
        title, game.sysContent.fntAreaName
    );
    addItem(titleItem, "title");
    
    //Prompt text.
    promptItem = new TextGuiItem(
        prompt, game.sysContent.fntStandard
    );
    promptItem->lineWrap = true;
    addItem(promptItem, "prompt");
    
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
        close();
    };
    addItem(backItem, "button_area");
    buttonItems.push_back(backItem);
    
    //Back input icon.
    guiAddBackInputIcon(this);
    
    for(size_t b = 0; b < extraButtons.size(); b++) {
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
    tooltipItem = new TooltipGuiItem(this);
    addItem(tooltipItem, "tooltip");
    
    //Position the buttons.
    Point buttonAreaCenter = backItem->ratioCenter;
    Point buttonAreaSize = backItem->ratioSize;
    const float totalEmptySpace =
        MODAL::BUTTON_MARGIN * (buttonItems.size() - 1);
    const float buttonWidth =
        (buttonAreaSize.x - totalEmptySpace) / buttonItems.size();
    float curX = buttonAreaCenter.x - buttonAreaSize.x / 2.0f;
    
    for(size_t b = 0; b < buttonItems.size(); b++) {
        buttonItems[b]->ratioCenter.x = curX + buttonWidth / 2.0f;
        buttonItems[b]->ratioSize.x = buttonWidth;
        curX += buttonWidth + MODAL::BUTTON_MARGIN;
    }
    
    //Finishing touches.
    setFocusedItem(backItem);
}
