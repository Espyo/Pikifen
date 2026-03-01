/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for a GUI manager with a modal dialog.
 */

#pragma once


#include <functional>
#include <string>
#include <vector>


#include "../content/other/gui.h"


using std::function;
using std::string;
using std::vector;


namespace MODAL {
extern const float BG_OPACITY;
extern const float BUTTON_MARGIN;
extern const float CARET_BLINK_INTERVAL;
extern const float FADE_DURATION;
extern const string GUI_FILE_NAME;
extern const size_t TEXT_INPUT_MAX_SIZE;
};


/**
 * @brief Represents a GUI that is simply placed on top of another one,
 * serving as a modal dialog. It contains some simple text, dynamic buttons,
 * and an optional text input field.
 */
class ModalGuiManager : public GuiManager {

public:

    //--- Public misc. definitions ---
    
    /**
     * @brief Represents an extra button.
     */
    struct Button {
    
        //--- Public members ---
        
        //Text to show.
        string text;
        
        //Tooltip.
        string tooltip;
        
        //Text color.
        ALLEGRO_COLOR color = COLOR_WHITE;
        
        //Code to run on activation.
        function<void(const Point&)> onActivate = nullptr;
        
    };
    
    //--- Public members ---
    
    //Title text.
    string title;
    
    //Prompt text.
    string prompt;
    
    //Current text in the text input.
    string textInput;
    
    //Back button's text.
    string back;
    
    //Back button's tooltip.
    string backTooltip;
    
    //Code to run when the back button is pressed, if any.
    std::function<void()> onBack;
    
    //List of extra buttons.
    vector<Button> extraButtons;
    
    //Default focused button. Indexed from all of the buttons.
    //0 for the back button.
    size_t defaultFocusButtonIdx = 0;
    
    //Button to press when Enter is pressed when there's a text input.
    //Indexed from all of the buttons. 0 for the back button.
    size_t textInputEnterButtonIdx = 0;
    
    //Should this modal have a text input field?
    bool useTextInput = false;
    
    
    //--- Public function declarations ---
    
    ModalGuiManager();
    bool isActive() const;
    void draw();
    void open();
    void close();
    void reset();
    void updateItems();
    bool handleAllegroEvent(const ALLEGRO_EVENT& ev);
    
    
private:

    //--- Private members ---
    
    //Items for all the buttons.
    vector<GuiItem*> buttonItems;
    
};
