/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Title screen state class and title screen state-related functions.
 */

#include <algorithm>

#include "title_screen.h"

#include "../core/drawing.h"
#include "../core/game.h"
#include "../core/load.h"
#include "../core/misc_functions.h"
#include "../util/allegro_utils.h"
#include "../util/os_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace TITLE_SCREEN {

//Name of the GUI information file.
const string GUI_FILE_NAME = "main_menu";

//How long the menu items take to move when switching pages.
const float HUD_MOVE_TIME = 0.5f;

//Name of the make page GUI information file.
const string MAKE_GUI_FILE_NAME = "main_menu_make";

//Name of the play page GUI information file.
const string PLAY_GUI_FILE_NAME = "main_menu_play";

//Name of the tutorial question page GUI information file.
const string TUTORIAL_GUI_FILE_NAME = "main_menu_tutorial";

}


/**
 * @brief Draws the title screen.
 */
void TitleScreen::doDrawing() {
    al_clear_to_color(COLOR_BLACK);
    
    if(game.debug.showDearImGuiDemo) return;
    
    drawBitmap(
        bmpMenuBg, Point(game.winW * 0.5, game.winH * 0.5),
        Point(game.winW, game.winH)
    );
    
    //Draw the logo Pikmin.
    Point pikSize = logoPikminSize;
    pikSize.x *= game.winW / 100.0f;
    pikSize.y *= game.winH / 100.0f;
    
    for(size_t p = 0; p < logoPikmin.size(); p++) {
        LogoPikmin* pik = &logoPikmin[p];
        drawBitmapInBox(
            game.sysContent.bmpShadow,
            pik->pos + pikSize * 0.30f, pikSize * 1.2f,
            true, 0.0f, COLOR_TRANSPARENT_WHITE
        );
    }
    for(size_t p = 0; p < logoPikmin.size(); p++) {
        LogoPikmin* pik = &logoPikmin[p];
        drawBitmapInBox(
            pik->top, pik->pos, pikSize, true, pik->angle
        );
    }
    
    drawText(
        "Pikifen and contents are fan works. Pikmin is (c) Nintendo.",
        game.sysContent.fntSlim,
        Point(8.0f),
        Point(game.winW * 0.45f, game.winH * 0.02f), mapAlpha(192),
        ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
    );
    string versionText;
    if(!game.config.general.name.empty()) {
        versionText = game.config.general.name;
        if(!game.config.general.version.empty()) {
            versionText += " " + game.config.general.version;
        }
        versionText += ", powered by ";
    }
    versionText +=
        "Pikifen " + getEngineVersionString();
    drawText(
        versionText, game.sysContent.fntSlim,
        Point(game.winW - 8, 8),
        Point(game.winW * 0.45f, game.winH * 0.02f), mapAlpha(192),
        ALLEGRO_ALIGN_RIGHT, V_ALIGN_MODE_TOP
    );
    
    mainGui.draw();
    playGui.draw();
    makeGui.draw();
    tutorialGui.draw();
    
    drawMouseCursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Ticks a frame's worth of logic.
 */
void TitleScreen::doLogic() {
    if(game.debug.showDearImGuiDemo) return;
    
    //Animate the logo Pikmin.
    int largestWindowDim = std::max(game.winW, game.winH);
    for(size_t p = 0; p < logoPikmin.size(); p++) {
        LogoPikmin* pik = &logoPikmin[p];
        
        if(!pik->reachedDestination) {
            float a = getAngle(pik->pos, pik->destination);
            float speed =
                std::min(
                    (float) (pik->speed * largestWindowDim * game.deltaT),
                    Distance(pik->pos, pik->destination).toFloat() *
                    logoPikminSpeedSmoothness
                );
            pik->pos.x += cos(a) * speed;
            pik->pos.y += sin(a) * speed;
            if(
                fabs(pik->pos.x - pik->destination.x) < 1.0 &&
                fabs(pik->pos.y - pik->destination.y) < 1.0
            ) {
                pik->destination = pik->pos;
                pik->reachedDestination = true;
            }
            
        } else {
            pik->swayVar += pik->swaySpeed * game.deltaT;
            pik->pos.x =
                pik->destination.x +
                sin(pik->swayVar) * logoPikminSwayAmount;
        }
    }
    
    if(!game.fadeMgr.isFading()) {
        for(size_t a = 0; a < game.playerActions.size(); a++) {
            mainGui.handlePlayerAction(game.playerActions[a]);
            playGui.handlePlayerAction(game.playerActions[a]);
            makeGui.handlePlayerAction(game.playerActions[a]);
            tutorialGui.handlePlayerAction(game.playerActions[a]);
        }
    }
    
    mainGui.tick(game.deltaT);
    playGui.tick(game.deltaT);
    makeGui.tick(game.deltaT);
    tutorialGui.tick(game.deltaT);
    
    //Fade manager needs to come last, because if
    //the fade finishes and the state changes, and
    //after that we still attempt to do stuff in
    //this function, we're going to have a bad time.
    game.fadeMgr.tick(game.deltaT);
    
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string TitleScreen::getName() const {
    return "title screen";
}


/**
 * @brief Handles Allegro events.
 *
 * @param ev Event to handle.
 */
void TitleScreen::handleAllegroEvent(ALLEGRO_EVENT& ev) {
    if(game.fadeMgr.isFading()) return;
    
    mainGui.handleAllegroEvent(ev);
    playGui.handleAllegroEvent(ev);
    makeGui.handleAllegroEvent(ev);
    tutorialGui.handleAllegroEvent(ev);
}


/**
 * @brief Loads the GUI elements for the main menu's main page.
 */
void TitleScreen::initGuiMainPage() {
    DataNode* guiFile = &game.content.guiDefs.list[TITLE_SCREEN::GUI_FILE_NAME];
    
    //Button icon positions.
    DataNode* iconsNode = guiFile->getChildByName("icons_to_the_left");
    
#define iconLeft(name, def) s2b(iconsNode->getChildByName(name)-> \
                                getValueOrDefault(def))
    
    bool playIconLeft = iconLeft("play", "true");
    bool makeIconLeft = iconLeft("make", "false");
    bool helpIconLeft = iconLeft("help", "true");
    bool optionsIconLeft = iconLeft("options", "true");
    bool statsIconLeft = iconLeft("statistics", "true");
    bool quitIconLeft = iconLeft("quit", "false");
    
#undef iconLeft
    
    //Menu items.
    mainGui.registerCoords("play",       42, 58, 44, 12);
    mainGui.registerCoords("make",       58, 72, 44, 12);
    mainGui.registerCoords("help",       24, 83, 24,  6);
    mainGui.registerCoords("options",    50, 83, 24,  6);
    mainGui.registerCoords("stats",      76, 83, 24,  6);
    mainGui.registerCoords("discord",    74, 91,  4,  5);
    mainGui.registerCoords("github",     80, 91,  4,  5);
    mainGui.registerCoords("exit",       91, 91, 14,  6);
    mainGui.registerCoords("exit_input", 97, 93,  4,  4);
    mainGui.registerCoords("tooltip",    50, 96, 96,  4);
    mainGui.readCoords(guiFile->getChildByName("positions"));
    
    //Play button.
    ButtonGuiItem* playButton =
        new ButtonGuiItem("Play", game.sysContent.fntAreaName);
    playButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_PLAY, draw.center, draw.size, playIconLeft
        );
        drawButton(
            draw.center, draw.size,
            playButton->text, playButton->font,
            playButton->color, playButton->selected,
            playButton->getJuiceValue()
        );
    };
    playButton->onActivate =
    [this] (const Point&) {
        mainGui.responsive = false;
        mainGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        if(game.statistics.areaEntries == 0) {
            tutorialGui.responsive = true;
            tutorialGui.startAnimation(
                GUI_MANAGER_ANIM_LEFT_TO_CENTER,
                TITLE_SCREEN::HUD_MOVE_TIME
            );
        } else {
            playGui.responsive = true;
            playGui.startAnimation(
                GUI_MANAGER_ANIM_LEFT_TO_CENTER,
                TITLE_SCREEN::HUD_MOVE_TIME
            );
        }
    };
    playButton->onGetTooltip =
    [] () { return "Choose an area to play in."; };
    mainGui.addItem(playButton, "play");
    
    //Make button.
    ButtonGuiItem* makeButton =
        new ButtonGuiItem("Make", game.sysContent.fntAreaName);
    makeButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_MAKE, draw.center, draw.size, makeIconLeft
        );
        drawButton(
            draw.center, draw.size,
            makeButton->text, makeButton->font,
            makeButton->color, makeButton->selected,
            makeButton->getJuiceValue()
        );
    };
    makeButton->onActivate =
    [this] (const Point&) {
        mainGui.responsive = false;
        mainGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        makeGui.responsive = true;
        makeGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
    };
    makeButton->onGetTooltip =
    [] () { return "Make your own content, like areas or animations."; };
    mainGui.addItem(makeButton, "make");
    
    //Help button.
    ButtonGuiItem* helpButton =
        new ButtonGuiItem("Help", game.sysContent.fntAreaName);
    helpButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_HELP, draw.center, draw.size, helpIconLeft
        );
        drawButton(
            draw.center, draw.size,
            helpButton->text, helpButton->font,
            helpButton->color, helpButton->selected,
            helpButton->getJuiceValue()
        );
    };
    helpButton->onActivate =
    [this] (const Point&) {
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.annexScreen);
        });
    };
    helpButton->onGetTooltip =
    [] () {
        return
            "Quick help and tips about how to play. "
            "You can also find this in the pause menu.";
    };
    mainGui.addItem(helpButton, "help");
    
    //Options button.
    ButtonGuiItem* optionsButton =
        new ButtonGuiItem("Options", game.sysContent.fntAreaName);
    optionsButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_OPTIONS, draw.center, draw.size, optionsIconLeft
        );
        drawButton(
            draw.center, draw.size,
            optionsButton->text, optionsButton->font,
            optionsButton->color, optionsButton->selected,
            optionsButton->getJuiceValue()
        );
    };
    optionsButton->onActivate =
    [] (const Point&) {
        game.fadeMgr.startFade(false, [] () {
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_OPTIONS;
            game.changeState(game.states.annexScreen);
        });
    };
    optionsButton->onGetTooltip =
    [] () {
        return
            "Customize your playing experience. "
            "You can also find this in the pause menu.";
    };
    mainGui.addItem(optionsButton, "options");
    
    //Statistics button.
    ButtonGuiItem* statsButton =
        new ButtonGuiItem("Statistics", game.sysContent.fntAreaName);
    statsButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_STATISTICS, draw.center, draw.size, statsIconLeft
        );
        drawButton(
            draw.center, draw.size,
            statsButton->text, statsButton->font,
            statsButton->color, statsButton->selected,
            statsButton->getJuiceValue()
        );
    };
    statsButton->onActivate =
    [] (const Point&) {
        game.fadeMgr.startFade(false, [] () {
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_STATS;
            game.changeState(game.states.annexScreen);
        });
    };
    statsButton->onGetTooltip =
    [] () {
        return
            "Check out some fun lifetime statistics. "
            "You can also find this in the pause menu.";
    };
    mainGui.addItem(statsButton, "stats");
    
    //Discord server button.
    ButtonGuiItem* discordButton =
        new ButtonGuiItem("", game.sysContent.fntAreaName);
    discordButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawBitmapInBox(
            game.sysContent.bmpDiscordIcon, draw.center, draw.size * 0.8f, true
        );
        drawButton(
            draw.center, draw.size,
            discordButton->text, discordButton->font,
            discordButton->color, discordButton->selected,
            discordButton->getJuiceValue()
        );
    };
    discordButton->onActivate =
    [] (const Point&) {
        openWebBrowser(DISCORD_SERVER_URL);
    };
    discordButton->onGetTooltip =
    [] () {
        return
            "Open the project's Discord server! Discussions! Feedback! "
            "Questions! New content!";
    };
    mainGui.addItem(discordButton, "discord");
    
    //GitHub page button.
    ButtonGuiItem* githubButton =
        new ButtonGuiItem("", game.sysContent.fntAreaName);
    githubButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawBitmapInBox(
            game.sysContent.bmpGithubIcon, draw.center, draw.size * 0.8f, true
        );
        drawButton(
            draw.center, draw.size,
            githubButton->text, githubButton->font,
            githubButton->color, githubButton->selected,
            githubButton->getJuiceValue()
        );
    };
    githubButton->onActivate =
    [] (const Point&) {
        openWebBrowser(GITHUB_PAGE_URL);
    };
    githubButton->onGetTooltip =
    [] () { return "Open the project's GitHub (development) page!"; };
    mainGui.addItem(githubButton, "github");
    
    //Exit button.
    mainGui.backItem =
        new ButtonGuiItem("Exit", game.sysContent.fntAreaName);
    mainGui.backItem->onDraw =
    [this, quitIconLeft] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_QUIT, draw.center, draw.size, quitIconLeft
        );
        drawButton(
            draw.center, draw.size,
            ((ButtonGuiItem*) mainGui.backItem)->text,
            ((ButtonGuiItem*) mainGui.backItem)->font,
            ((ButtonGuiItem*) mainGui.backItem)->color,
            mainGui.backItem->selected,
            mainGui.backItem->getJuiceValue()
        );
    };
    mainGui.backItem->onActivate =
    [] (const Point&) {
        saveStatistics();
        game.isGameRunning = false;
    };
    mainGui.backItem->onGetTooltip =
    [] () {
        return
            game.config.general.name.empty() ?
            "Quit Pikifen." :
            "Quit " + game.config.general.name + ".";
    };
    mainGui.addItem(mainGui.backItem, "exit");
    
    //Exit input icon.
    guiAddBackInputIcon(&mainGui, "exit_input");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&mainGui);
    mainGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    mainGui.setSelectedItem(playButton, true);
    mainGui.responsive = false;
    mainGui.hideItems();
}


/**
 * @brief Loads the GUI elements for the main menu's make page.
 */
void TitleScreen::initGuiMakePage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[TITLE_SCREEN::MAKE_GUI_FILE_NAME];
        
    //Button icon positions.
    DataNode* iconsNode = guiFile->getChildByName("icons_to_the_left");
    
#define iconLeft(name, def) s2b(iconsNode->getChildByName(name)-> \
                                getValueOrDefault(def))
    
    bool animEditorIconLeft = iconLeft("animation_editor", "true");
    bool areaEditorIconLeft = iconLeft("area_editor", "false");
    bool particleEditorIconLeft = iconLeft("particle_editor", "true");
    bool guiEditorIconLeft = iconLeft("gui_editor", "false");
    
#undef iconLeft
    
    //Menu items.
    makeGui.registerCoords("animation_editor", 27.5, 63, 43, 12);
    makeGui.registerCoords("area_editor",      72.5, 63, 43, 12);
    makeGui.registerCoords("gui_editor",         69, 78, 34,  8);
    makeGui.registerCoords("particle_editor",    31, 78, 34,  8);
    makeGui.registerCoords("back",                9, 91, 14,  6);
    makeGui.registerCoords("back_input",          3, 93,  4,  4);
    makeGui.registerCoords("more",               91, 91, 14,  6);
    makeGui.registerCoords("tooltip",            50, 96, 96,  4);
    makeGui.readCoords(guiFile->getChildByName("positions"));
    
    //Animation editor button.
    ButtonGuiItem* animEdButton =
        new ButtonGuiItem("Animations", game.sysContent.fntAreaName);
    animEdButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_ANIM_EDITOR, draw.center, draw.size, animEditorIconLeft
        );
        drawButton(
            draw.center, draw.size,
            animEdButton->text, animEdButton->font,
            animEdButton->color, animEdButton->selected,
            animEdButton->getJuiceValue()
        );
    };
    animEdButton->onActivate =
    [] (const Point&) {
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.animationEd);
        });
    };
    animEdButton->onGetTooltip =
    [] () { return "Make an animation for any object in the game."; };
    makeGui.addItem(animEdButton, "animation_editor");
    
    //Area editor button.
    ButtonGuiItem* areaEdButton =
        new ButtonGuiItem("Areas", game.sysContent.fntAreaName);
    areaEdButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_AREA_EDITOR, draw.center, draw.size, areaEditorIconLeft
        );
        drawButton(
            draw.center, draw.size,
            areaEdButton->text, areaEdButton->font,
            areaEdButton->color, areaEdButton->selected,
            areaEdButton->getJuiceValue()
        );
    };
    areaEdButton->onActivate =
    [] (const Point&) {
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.areaEd);
        });
    };
    areaEdButton->onGetTooltip =
    [] () { return "Make an area to play on."; };
    makeGui.addItem(areaEdButton, "area_editor");
    
    //Particle editor button.
    ButtonGuiItem* partEdButton =
        new ButtonGuiItem("Particles", game.sysContent.fntAreaName);
    partEdButton->onDraw =
    [ = ](const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_PARTICLE_EDITOR, draw.center, draw.size,
            particleEditorIconLeft
        );
        drawButton(
            draw.center, draw.size,
            partEdButton->text, partEdButton->font,
            partEdButton->color, partEdButton->selected,
            partEdButton->getJuiceValue()
        );
    };
    partEdButton->onActivate =
    [](const Point&) {
        game.fadeMgr.startFade(false, []() {
            game.changeState(game.states.particleEd);
        });
    };
    partEdButton->onGetTooltip =
    []() { return "Make generators that create particles."; };
    makeGui.addItem(partEdButton, "particle_editor");
    
    //GUI editor button.
    ButtonGuiItem* guiEdButton =
        new ButtonGuiItem("GUI", game.sysContent.fntAreaName);
    guiEdButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_GUI_EDITOR, draw.center, draw.size, guiEditorIconLeft
        );
        drawButton(
            draw.center, draw.size,
            guiEdButton->text, guiEdButton->font,
            guiEdButton->color, guiEdButton->selected,
            guiEdButton->getJuiceValue()
        );
    };
    guiEdButton->onActivate =
    [] (const Point&) {
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.guiEd);
        });
    };
    guiEdButton->onGetTooltip =
    [] () { return "Change the way menus and the gameplay HUD look."; };
    makeGui.addItem(guiEdButton, "gui_editor");
    
    //Back button.
    makeGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntAreaName);
    makeGui.backItem->onActivate =
    [this] (const Point&) {
        makeGui.responsive = false;
        makeGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        mainGui.responsive = true;
        mainGui.startAnimation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
    };
    makeGui.backItem->onGetTooltip =
    [] () {
        return "Return to the main page.";
    };
    makeGui.addItem(makeGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&makeGui);
    
    //More bullet point.
    BulletGuiItem* moreBullet =
        new BulletGuiItem("More...", game.sysContent.fntStandard);
    moreBullet->onActivate =
    [] (const Point&) {
        openManual("making.html");
    };
    moreBullet->onGetTooltip =
    [] () {
        return
            "Click to open the manual (in the game's folder) for "
            "more info on content making.";
    };
    makeGui.addItem(moreBullet, "more");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&makeGui);
    makeGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    makeGui.setSelectedItem(animEdButton, true);
    makeGui.responsive = false;
    makeGui.hideItems();
}


/**
 * @brief Loads the GUI elements for the main menu's play page.
 */
void TitleScreen::initGuiPlayPage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[TITLE_SCREEN::PLAY_GUI_FILE_NAME];
        
    //Button icon positions.
    DataNode* iconsNode = guiFile->getChildByName("icons_to_the_left");
    
#define iconLeft(name, def) s2b(iconsNode->getChildByName(name)-> \
                                getValueOrDefault(def))
    
    bool simpleAreasIconLeft = iconLeft("simple_areas", "true");
    bool missionsIconLeft = iconLeft("missions", "true");
    
#undef iconLeft
    
    //Menu items.
    playGui.registerCoords("simple",     42, 60, 60, 12.5);
    playGui.registerCoords("mission",    44, 78, 60, 12.5);
    playGui.registerCoords("back",        9, 91, 14,    6);
    playGui.registerCoords("back_input",  3, 93,  4,    4);
    playGui.registerCoords("tooltip",    50, 96, 96,    4);
    playGui.readCoords(guiFile->getChildByName("positions"));
    
    //Play a simple area button.
    ButtonGuiItem* simpleButton =
        new ButtonGuiItem("Simple areas", game.sysContent.fntAreaName);
    simpleButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_SIMPLE_AREAS, draw.center, draw.size, simpleAreasIconLeft
        );
        drawButton(
            draw.center, draw.size,
            simpleButton->text, simpleButton->font,
            simpleButton->color, simpleButton->selected,
            simpleButton->getJuiceValue()
        );
    };
    simpleButton->onActivate =
    [] (const Point&) {
        game.fadeMgr.startFade(false, [] () {
            game.states.annexScreen->areaMenuAreaType =
                AREA_TYPE_SIMPLE;
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.changeState(game.states.annexScreen);
        });
    };
    simpleButton->onGetTooltip =
    [] () { return "Pick a simple area with no goal, and start playing!"; };
    playGui.addItem(simpleButton, "simple");
    
    //Play a mission area button.
    ButtonGuiItem* missionButton =
        new ButtonGuiItem("Missions", game.sysContent.fntAreaName);
    missionButton->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_MISSIONS, draw.center, draw.size, missionsIconLeft
        );
        drawButton(
            draw.center, draw.size,
            missionButton->text, missionButton->font,
            missionButton->color, missionButton->selected,
            missionButton->getJuiceValue()
        );
    };
    missionButton->onActivate =
    [] (const Point&) {
        game.fadeMgr.startFade(false, [] () {
            game.states.annexScreen->areaMenuAreaType =
                AREA_TYPE_MISSION;
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.changeState(game.states.annexScreen);
        });
    };
    missionButton->onGetTooltip =
    [] () {
        return
            "Pick a mission area with goals and limitations, "
            "and start playing!";
    };
    playGui.addItem(missionButton, "mission");
    
    //Back button.
    playGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntAreaName);
    playGui.backItem->onActivate =
    [this] (const Point&) {
        playGui.responsive = false;
        playGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        mainGui.responsive = true;
        mainGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
    };
    playGui.backItem->onGetTooltip =
    [] () {
        return "Return to the main page.";
    };
    playGui.addItem(playGui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&playGui);
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&playGui);
    playGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    playGui.setSelectedItem(simpleButton, true);
    playGui.responsive = false;
    playGui.hideItems();
}


/**
 * @brief Loads the GUI elements for the main menu's tutorial question page.
 */
void TitleScreen::initGuiTutorialPage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[TITLE_SCREEN::TUTORIAL_GUI_FILE_NAME];
        
    //Menu items.
    tutorialGui.registerCoords("question", 50,     60, 60,  12.5);
    tutorialGui.registerCoords("no",       26, 80.875, 40, 10.25);
    tutorialGui.registerCoords("no_input",  7,     85,  4,     4);
    tutorialGui.registerCoords("yes",      74,     81, 40,    10);
    tutorialGui.registerCoords("tooltip",  50,     96, 96,     4);
    tutorialGui.readCoords(guiFile->getChildByName("positions"));
    
    //Question text.
    TextGuiItem* questionText =
        new TextGuiItem(
        "If you're new to Pikifen, it is recommended to play the "
        "\"Tutorial Meadow\" mission first.\n\n"
        "Do you want to play there now?",
        game.sysContent.fntStandard
    );
    questionText->lineWrap = true;
    tutorialGui.addItem(questionText, "question");
    
    //No button.
    tutorialGui.backItem =
        new ButtonGuiItem("No", game.sysContent.fntStandard);
    tutorialGui.backItem->onActivate =
    [this] (const Point&) {
        tutorialGui.responsive = false;
        tutorialGui.startAnimation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
        playGui.responsive = true;
        playGui.startAnimation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            TITLE_SCREEN::HUD_MOVE_TIME
        );
    };
    tutorialGui.backItem->onGetTooltip =
    [] () {
        return
            "Go to the standard area selection menu.";
    };
    tutorialGui.addItem(tutorialGui.backItem, "no");
    
    //No input icon.
    guiAddBackInputIcon(&tutorialGui, "no_input");
    
    //Yes button.
    ButtonGuiItem* yesButton =
        new ButtonGuiItem("Yes", game.sysContent.fntStandard);
    yesButton->onActivate =
    [] (const Point&) {
        game.states.gameplay->pathOfAreaToLoad =
            game.content.areas.manifestToPath(
                ContentManifest(
                    FOLDER_NAMES::TUTORIAL_AREA, "", FOLDER_NAMES::BASE_PACK
                ), AREA_TYPE_MISSION
            );
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.gameplay);
        });
    };
    yesButton->onGetTooltip =
    [] () {
        return
            "Play Tutorial Meadow now.";
    };
    tutorialGui.addItem(yesButton, "yes");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&tutorialGui);
    tutorialGui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    tutorialGui.setSelectedItem(yesButton, true);
    tutorialGui.responsive = false;
    tutorialGui.hideItems();
}


/**
 * @brief Loads the title screen into memory.
 */
void TitleScreen::load() {
    drawLoadingScreen("", "", 1.0);
    al_flip_display();
    
    //Game content.
    game.content.reloadPacks();
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Misc. initializations.
    initGuiMainPage();
    initGuiPlayPage();
    initGuiMakePage();
    initGuiTutorialPage();
    
    switch(pageToLoad) {
    case MAIN_MENU_PAGE_MAIN: {
        mainGui.responsive = true;
        mainGui.showItems();
        break;
    } case MAIN_MENU_PAGE_PLAY: {
        playGui.responsive = true;
        playGui.showItems();
        break;
    } case MAIN_MENU_PAGE_MAKE: {
        makeGui.responsive = true;
        makeGui.showItems();
        break;
    }
    }
    pageToLoad = MAIN_MENU_PAGE_MAIN;
    
    DataNode* settingsFile =
        &game.content.guiDefs.list[TITLE_SCREEN::GUI_FILE_NAME];
        
    //Resources.
    bmpMenuBg =
        game.content.bitmaps.list.get(game.sysContentNames.bmpTitleScreenBg);
        
    //Logo pikmin.
    DataNode* logoNode = settingsFile->getChildByName("logo");
    ReaderSetter lRS(logoNode);
    
    DataNode* pikTypesNode =
        logoNode->getChildByName("pikmin_types");
    for(size_t t = 0; t < pikTypesNode->getNrOfChildren(); t++) {
        DataNode* typeNode = pikTypesNode->getChild(t);
        if(typeNode->name.empty()) continue;
        logoTypeBitmaps[typeNode->name[0]] =
            game.content.bitmaps.list.get(typeNode->value, typeNode);
    }
    
    DataNode* mapNode =
        logoNode->getChildByName("map");
    size_t mapTotalRows = mapNode->getNrOfChildren();
    size_t mapTotalCols = 0;
    for(size_t r = 0; r < mapTotalRows; r++) {
        mapTotalCols =
            std::max(mapTotalCols, mapNode->getChild(r)->name.size());
    }
    
    lRS.set("min_window_limit", logoMinWindowLimit);
    lRS.set("max_window_limit", logoMaxWindowLimit);
    lRS.set("pikmin_max_speed", logoPikminMaxSpeed);
    lRS.set("pikmin_min_speed", logoPikminMinSpeed);
    lRS.set("pikmin_speed_smoothness", logoPikminSpeedSmoothness);
    lRS.set("pikmin_sway_amount", logoPikminSwayAmount);
    lRS.set("pikmin_sway_max_speed", logoPikminSwayMaxSpeed);
    lRS.set("pikmin_sway_min_speed", logoPikminSwayMinSpeed);
    lRS.set("pikmin_size", logoPikminSize);
    
    bool mapOk = true;
    
    for(size_t r = 0; r < mapTotalRows; r++) {
        string row = mapNode->getChild(r)->name;
        
        for(size_t c = 0; c < row.size(); c++) {
            if(row[c] == '.') continue;
            if(!isInMap(logoTypeBitmaps, row[c])) {
                mapOk = false;
                game.errors.report(
                    "Title screen Pikmin logo map has an unknown character \"" +
                    string(1, row[c]) + "\" on row " + i2s(r + 1) +
                    ", column " + i2s(c + 1) + "!"
                );
                break;
            }
            
            LogoPikmin pik;
            
            Point minPos = logoMinWindowLimit;
            minPos.x *= game.winW / 100.0f;
            minPos.y *= game.winH / 100.0f;
            Point maxPos = logoMaxWindowLimit;
            maxPos.x *= game.winW / 100.0f;
            maxPos.y *= game.winH / 100.0f;
            
            pik.top = logoTypeBitmaps[row[c]];
            pik.destination =
                Point(
                    minPos.x +
                    (maxPos.x - minPos.x) *
                    (c / (float) mapTotalCols),
                    minPos.y +
                    (maxPos.y - minPos.y) *
                    (r / (float) mapTotalRows)
                );
                
            pik.pos =
                Point(game.winW / 2.0f, game.winH / 2.0f) +
                getRandomPointInRectangularRing(
                    Point(game.winW * 1.2, game.winH * 1.2),
                    Point(game.winW * 1.4, game.winH * 1.4),
                    game.rng.i(0, 1), game.rng.f(0.0f, 1.0f),
                    game.rng.f(0.0f, 1.0f), game.rng.f(0.0f, 1.0f),
                    game.rng.i(0, 1)
                );
                
            pik.angle = game.rng.f(0, TAU);
            pik.speed = game.rng.f(logoPikminMinSpeed, logoPikminMaxSpeed);
            pik.swaySpeed =
                game.rng.f(logoPikminSwayMinSpeed, logoPikminSwayMaxSpeed);
            pik.swayVar = 0;
            pik.reachedDestination = false;
            logoPikmin.push_back(pik);
        }
        
        if(!mapOk) break;
    }
    
    //Finishing touches.
    game.audio.setCurrentSong(game.sysContentNames.sngMenus, false);
    if(game.timePassed == 0.0f) {
        game.fadeMgr.setNextFadeDuration(GAME::FADE_SLOW_DURATION);
    }
    game.fadeMgr.startFade(true, nullptr);
    if(game.debug.showDearImGuiDemo) game.mouseCursor.show();
}


/**
 * @brief Unloads the title screen from memory.
 */
void TitleScreen::unload() {
    //Resources.
    game.content.bitmaps.list.free(bmpMenuBg);
    bmpMenuBg = nullptr;
    for(const auto& t : logoTypeBitmaps) {
        game.content.bitmaps.list.free(t.second);
    }
    logoTypeBitmaps.clear();
    
    //Menu items.
    mainGui.destroy();
    playGui.destroy();
    makeGui.destroy();
    tutorialGui.destroy();
    
    //Misc.
    logoPikmin.clear();
    
    //Game content.
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
    }
    );
}
