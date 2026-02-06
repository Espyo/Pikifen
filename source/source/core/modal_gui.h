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
extern const float FADE_DURATION;
extern const string GUI_FILE_NAME;
};


/**
 * @brief Represents a GUI that is simply placed on top of another one,
 * serving as a modal dialog. It contains some simple text and buttons.
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
    
    //Back button's text.
    string back;
    
    //Back button's tooltip.
    string backTooltip;
    
    //List of extra buttons.
    vector<Button> extraButtons;
    
    //Default focused button. Indexed from all of the buttons.
    //0 for the back button.
    size_t defaultFocusButtonIdx = 0;
    
    
    //--- Public function declarations ---
    
    ModalGuiManager();
    bool isActive() const;
    void draw();
    void open();
    void close();
    void reset();
    void updateItems();
    
    
private:

    //--- Private members ---
    
    //Item for the title text.
    TextGuiItem* titleItem = nullptr;
    
    //Item for the prompt text.
    TextGuiItem* promptItem = nullptr;
    
    //Item for the tooltip text.
    TooltipGuiItem* tooltipItem = nullptr;
    
    //Items for all the buttons.
    vector<GuiItem*> buttonItems;
    
};
