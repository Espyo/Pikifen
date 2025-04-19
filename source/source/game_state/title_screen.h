/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the title screen class and related functions.
 */

#pragma once

#include <string>

#include "../content/other/gui.h"
#include "game_state.h"

using std::string;


namespace TITLE_SCREEN {
extern const string GUI_FILE_NAME;
extern const float HUD_MOVE_TIME;
extern const string MAKE_GUI_FILE_NAME;
extern const string PLAY_GUI_FILE_NAME;
extern const string TUTORIAL_GUI_FILE_NAME;
}


//Pages of the main menu.
enum MAIN_MENU_PAGE {

    //Main page.
    MAIN_MENU_PAGE_MAIN,
    
    //Play page.
    MAIN_MENU_PAGE_PLAY,
    
    //Make page.
    MAIN_MENU_PAGE_MAKE,
    
};



/**
 * @brief Info about the title screen.
 */
class TitleScreen : public GameState {

public:

    //--- Members ---
    
    //What page to load when it is created.
    MAIN_MENU_PAGE pageToLoad = MAIN_MENU_PAGE_MAIN;
    
    
    //--- Function declarations ---
    
    void load() override;
    void unload() override;
    void handleAllegroEvent(ALLEGRO_EVENT &ev) override;
    void doLogic() override;
    void doDrawing() override;
    string getName() const override;
    
private:

    //--- Misc. declarations ---
    
    /**
     * @brief Represents a Pikmin in the logo.
     */
    struct LogoPikmin {
    
        //--- Members ---
        
        //Position.
        Point pos;
        
        //Current angle.
        float angle = 0.0f;
        
        //Forward movement speed.
        float speed = 0.0f;
        
        //Its destination.
        Point destination;
        
        //Speed at which it sways.
        float swaySpeed = 0.0f;
        
        //Variable that controls its swaying.
        float swayVar = 0.0f;
        
        //Image that represents this Pikmin's top.
        ALLEGRO_BITMAP* top = nullptr;
        
        //Has it reached its destination?
        bool reachedDestination = false;
        
    };
    
    
    //--- Members ---
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmpMenuBg = nullptr;
    
    //List of Pikmin that make up the logo.
    vector<LogoPikmin> logoPikmin;
    
    //GUI for the main page.
    GuiManager mainGui;
    
    //GUI for the play page.
    GuiManager playGui;
    
    //GUI for the make page.
    GuiManager makeGui;
    
    //GUI for the tutorial question page.
    GuiManager tutorialGui;
    
    //Top-left coordinates of the logo, in window percentage.
    Point logoMinWindowLimit = Point(10.0f);
    
    //Bottom-right coordinates of the logo, in window percentage.
    Point logoMaxWindowLimit = Point(90.0f, 50.0f);
    
    //Maximum speed a logo Pikmin can move at, in window width or height ratio
    //per second (the largest of width or height).
    float logoPikminMaxSpeed = 800.0f;
    
    //Minimum speed a logo Pikmin can move at, in window width or height ratio
    //per second (the largest of width or height).
    float logoPikminMinSpeed = 600.0f;
    
    //How much to smooth a logo Pikmin's speed by.
    float logoPikminSpeedSmoothness = 0.08f;
    
    //How much to sway a logo Pikmin by.
    float logoPikminSwayAmount = 3.0f;
    
    //Maximum speed at which a logo Pikmin can sway.
    float logoPikminSwayMaxSpeed = 5.5f;
    
    //Minimum speed at which a logo Pikmin can sway.
    float logoPikminSwayMinSpeed = 2.5f;
    
    //Width and height of a logo Pikmin.
    Point logoPikminSize = Point(3.5f);
    
    //Map of what characters represent what Pikmin top bitmaps.
    map<unsigned char, ALLEGRO_BITMAP*> logoTypeBitmaps;
    
    
    //--- Function declarations ---
    
    void initGuiMainPage();
    void initGuiMakePage();
    void initGuiPlayPage();
    void initGuiTutorialPage();
    
};
