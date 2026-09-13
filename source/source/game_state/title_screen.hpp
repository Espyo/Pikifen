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

#include "../content/other/gui.hpp"
#include "../menu/menu.hpp"
#include "game_state.hpp"


using std::string;


namespace MAIN_MENU {
extern const float FADE_IN_FAST_DURATION;
extern const float FADE_IN_DELAY;
extern const float FADE_IN_DURATION;
extern const string GUI_FILE_NAME;
extern const float HUD_MOVE_TIME;
extern const string MAKE_GUI_FILE_NAME;
extern const string PLAY_GUI_FILE_NAME;
extern const float ZOOM_DURATION;
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
 * @brief Info about the main menu present in the title screen.
 */
class MainMenu : public Menu {

public:

    //--- Public members ---
    
    //What page to load when it is created.
    MAIN_MENU_PAGE pageToLoad = MAIN_MENU_PAGE_MAIN;
    
    //GUI for the main page.
    GuiManager mainGui;
    
    //GUI for the play page.
    GuiManager playGui;
    
    //GUI for the make page.
    GuiManager makeGui;
    
    
    //--- Public function declarations ---
    
    void load() override;
    void hide();
    void startFadingIn();
    void speedUpFadeIn();
    
    
private:

    //--- Private function declarations ---
    
    void initGuiMainPage();
    void initGuiMakePage();
    void initGuiPlayPage();
    
};



/**
 * @brief Info about the title screen.
 */
class TitleScreen : public GameState {

public:

    //--- Public members ---
    
    //What page to load when it is created.
    MAIN_MENU_PAGE pageToLoad = MAIN_MENU_PAGE_MAIN;
    
    
    //--- Public function declarations ---
    
    void load() override;
    void unload() override;
    void handleAllegroEvent(ALLEGRO_EVENT& ev) override;
    void doLogic() override;
    void doDrawing() override;
    string getName() const override;
    
    
private:

    //--- Private misc. declarations ---
    
    /**
     * @brief Represents a Pikmin in the game's wordmark.
     */
    struct WordmarkPikmin {
    
        //--- Public members ---
        
        //Center coordinates.
        Point center;
        
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
    
    
    //--- Private members ---
    
    //Main menu.
    MainMenu mainMenu;
    
    //Bitmap of the menu background.
    ALLEGRO_BITMAP* bmpMenuBg = nullptr;
    
    //Buffer where the wordmark Pikmin shadows are drawn.
    ALLEGRO_BITMAP* bmpWordmarkShadows = nullptr;
    
    //List of Pikmin that make up the wordmark.
    vector<WordmarkPikmin> wordmarkPikmin;
    
    //Top-left coordinates of the wordmark, in window percentage.
    Point wordmarkMinWindowLimit = Point(10.0f);
    
    //Bottom-right coordinates of the wordmark, in window percentage.
    Point wordmarkMaxWindowLimit = Point(90.0f, 50.0f);
    
    //Maximum speed a wordmark Pikmin can move at, in window width or height ratio
    //per second (the largest of width or height).
    float wordmarkPikminMaxSpeed = 800.0f;
    
    //Minimum speed a wordmark Pikmin can move at, in window width or height ratio
    //per second (the largest of width or height).
    float wordmarkPikminMinSpeed = 600.0f;
    
    //How much to smooth a wordmark Pikmin's speed by.
    float wordmarkPikminSpeedSmoothness = 0.08f;
    
    //How much to sway a wordmark Pikmin by.
    float wordmarkPikminSwayAmount = 3.0f;
    
    //Maximum speed at which a wordmark Pikmin can sway.
    float wordmarkPikminSwayMaxSpeed = 5.5f;
    
    //Minimum speed at which a wordmark Pikmin can sway.
    float wordmarkPikminSwayMinSpeed = 2.5f;
    
    //Width and height of a wordmark Pikmin.
    Point wordmarkPikminSize = Point(3.5f);
    
    //Map of what characters represent what Pikmin top bitmaps.
    map<unsigned char, ALLEGRO_BITMAP*> wordmarkTypeBitmaps;
    
    //Time left until we start fading the GUI in.
    float guiFadeTimer = 0.0f;
    
    //Time left for the zoom-in effect.
    float zoomInTimer = 0.0f;
    
    
    //--- Private function declarations ---
    
    void drawDecorations(bool justWordmark) const;
    void drawFixedText() const;
    
};
