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
#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../core/load.h"
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
    Point pik_size = logoPikminSize;
    pik_size.x *= game.winW / 100.0f;
    pik_size.y *= game.winH / 100.0f;
    
    for(size_t p = 0; p < logoPikmin.size(); p++) {
        LogoPikmin* pik = &logoPikmin[p];
        
        drawBitmapInBox(
            pik->top, pik->pos, pik_size, true, pik->angle
        );
    }
    
    drawText(
        "Pikifen and contents are fan works. Pikmin is (c) Nintendo.",
        game.sysContent.fntSlim,
        Point(8.0f),
        Point(game.winW * 0.45f, game.winH * 0.02f), mapAlpha(192),
        ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
    );
    string version_text;
    if(!game.config.general.name.empty()) {
        version_text = game.config.general.name;
        if(!game.config.general.version.empty()) {
            version_text += " " + game.config.general.version;
        }
        version_text += ", powered by ";
    }
    version_text +=
        "Pikifen " + getEngineVersionString();
    drawText(
        version_text, game.sysContent.fntSlim,
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
    for(size_t p = 0; p < logoPikmin.size(); p++) {
        LogoPikmin* pik = &logoPikmin[p];
        
        if(!pik->reachedDestination) {
            float a = getAngle(pik->pos, pik->destination);
            float speed =
                std::min(
                    (float) (pik->speed * game.deltaT),
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
void TitleScreen::handleAllegroEvent(ALLEGRO_EVENT &ev) {
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
    DataNode* gui_file = &game.content.guiDefs.list[TITLE_SCREEN::GUI_FILE_NAME];
    
    //Button icon positions.
    DataNode* icons_node = gui_file->getChildByName("icons_to_the_left");
    
#define icon_left(name, def) s2b(icons_node->getChildByName(name)-> \
                                 getValueOrDefault(def))
    
    bool play_icon_left = icon_left("play", "true");
    bool make_icon_left = icon_left("make", "false");
    bool help_icon_left = icon_left("help", "true");
    bool options_icon_left = icon_left("options", "true");
    bool stats_icon_left = icon_left("statistics", "true");
    bool quit_icon_left = icon_left("quit", "false");
    
#undef icon_left
    
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
    mainGui.readCoords(gui_file->getChildByName("positions"));
    
    //Play button.
    ButtonGuiItem* play_button =
        new ButtonGuiItem("Play", game.sysContent.fntAreaName);
    play_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_PLAY, draw.center, draw.size, play_icon_left
        );
        drawButton(
            draw.center, draw.size,
            play_button->text, play_button->font,
            play_button->color, play_button->selected,
            play_button->getJuiceValue()
        );
    };
    play_button->onActivate =
    [this] (const Point &) {
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
    play_button->onGetTooltip =
    [] () { return "Choose an area to play in."; };
    mainGui.addItem(play_button, "play");
    
    //Make button.
    ButtonGuiItem* make_button =
        new ButtonGuiItem("Make", game.sysContent.fntAreaName);
    make_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_MAKE, draw.center, draw.size, make_icon_left
        );
        drawButton(
            draw.center, draw.size,
            make_button->text, make_button->font,
            make_button->color, make_button->selected,
            make_button->getJuiceValue()
        );
    };
    make_button->onActivate =
    [this] (const Point &) {
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
    make_button->onGetTooltip =
    [] () { return "Make your own content, like areas or animations."; };
    mainGui.addItem(make_button, "make");
    
    //Help button.
    ButtonGuiItem* help_button =
        new ButtonGuiItem("Help", game.sysContent.fntAreaName);
    help_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_HELP, draw.center, draw.size, help_icon_left
        );
        drawButton(
            draw.center, draw.size,
            help_button->text, help_button->font,
            help_button->color, help_button->selected,
            help_button->getJuiceValue()
        );
    };
    help_button->onActivate =
    [this] (const Point &) {
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.annexScreen);
        });
    };
    help_button->onGetTooltip =
    [] () {
        return
            "Quick help and tips about how to play. "
            "You can also find this in the pause menu.";
    };
    mainGui.addItem(help_button, "help");
    
    //Options button.
    ButtonGuiItem* options_button =
        new ButtonGuiItem("Options", game.sysContent.fntAreaName);
    options_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_OPTIONS, draw.center, draw.size, options_icon_left
        );
        drawButton(
            draw.center, draw.size,
            options_button->text, options_button->font,
            options_button->color, options_button->selected,
            options_button->getJuiceValue()
        );
    };
    options_button->onActivate =
    [] (const Point &) {
        game.fadeMgr.startFade(false, [] () {
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_OPTIONS;
            game.changeState(game.states.annexScreen);
        });
    };
    options_button->onGetTooltip =
    [] () {
        return
            "Customize your playing experience. "
            "You can also find this in the pause menu.";
    };
    mainGui.addItem(options_button, "options");
    
    //Statistics button.
    ButtonGuiItem* stats_button =
        new ButtonGuiItem("Statistics", game.sysContent.fntAreaName);
    stats_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_STATISTICS, draw.center, draw.size, stats_icon_left
        );
        drawButton(
            draw.center, draw.size,
            stats_button->text, stats_button->font,
            stats_button->color, stats_button->selected,
            stats_button->getJuiceValue()
        );
    };
    stats_button->onActivate =
    [] (const Point &) {
        game.fadeMgr.startFade(false, [] () {
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_STATS;
            game.changeState(game.states.annexScreen);
        });
    };
    stats_button->onGetTooltip =
    [] () {
        return
            "Check out some fun lifetime statistics. "
            "You can also find this in the pause menu.";
    };
    mainGui.addItem(stats_button, "stats");
    
    //Discord server button.
    ButtonGuiItem* discord_button =
        new ButtonGuiItem("", game.sysContent.fntAreaName);
    discord_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawBitmapInBox(
            game.sysContent.bmpDiscordIcon, draw.center, draw.size * 0.8f, true
        );
        drawButton(
            draw.center, draw.size,
            discord_button->text, discord_button->font,
            discord_button->color, discord_button->selected,
            discord_button->getJuiceValue()
        );
    };
    discord_button->onActivate =
    [] (const Point &) {
        openWebBrowser(DISCORD_SERVER_URL);
    };
    discord_button->onGetTooltip =
    [] () {
        return
            "Open the project's Discord server! Discussions! Feedback! "
            "Questions! New content!";
    };
    mainGui.addItem(discord_button, "discord");
    
    //GitHub page button.
    ButtonGuiItem* github_button =
        new ButtonGuiItem("", game.sysContent.fntAreaName);
    github_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawBitmapInBox(
            game.sysContent.bmpGithubIcon, draw.center, draw.size * 0.8f, true
        );
        drawButton(
            draw.center, draw.size,
            github_button->text, github_button->font,
            github_button->color, github_button->selected,
            github_button->getJuiceValue()
        );
    };
    github_button->onActivate =
    [] (const Point &) {
        openWebBrowser(GITHUB_PAGE_URL);
    };
    github_button->onGetTooltip =
    [] () { return "Open the project's GitHub (development) page!"; };
    mainGui.addItem(github_button, "github");
    
    //Exit button.
    mainGui.backItem =
        new ButtonGuiItem("Exit", game.sysContent.fntAreaName);
    mainGui.backItem->onDraw =
    [this, quit_icon_left] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_QUIT, draw.center, draw.size, quit_icon_left
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
    [] (const Point &) {
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
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&mainGui);
    mainGui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    mainGui.setSelectedItem(play_button, true);
    mainGui.responsive = false;
    mainGui.hideItems();
}


/**
 * @brief Loads the GUI elements for the main menu's make page.
 */
void TitleScreen::initGuiMakePage() {
    DataNode* gui_file = &game.content.guiDefs.list[TITLE_SCREEN::MAKE_GUI_FILE_NAME];
    
    //Button icon positions.
    DataNode* icons_node = gui_file->getChildByName("icons_to_the_left");
    
#define icon_left(name, def) s2b(icons_node->getChildByName(name)-> \
                                 getValueOrDefault(def))
    
    bool anim_editor_icon_left = icon_left("animation_editor", "true");
    bool area_editor_icon_left = icon_left("area_editor", "false");
    bool particle_editor_icon_left = icon_left("particle_editor", "true");
    bool gui_editor_icon_left = icon_left("gui_editor", "false");
    
#undef icon_left
    
    //Menu items.
    makeGui.registerCoords("animation_editor", 27.5, 63, 43, 12);
    makeGui.registerCoords("area_editor",      72.5, 63, 43, 12);
    makeGui.registerCoords("gui_editor",         69, 78, 34,  8);
    makeGui.registerCoords("particle_editor",    31, 78, 34,  8);
    makeGui.registerCoords("back",                9, 91, 14,  6);
    makeGui.registerCoords("back_input",          3, 93,  4,  4);
    makeGui.registerCoords("more",               91, 91, 14,  6);
    makeGui.registerCoords("tooltip",            50, 96, 96,  4);
    makeGui.readCoords(gui_file->getChildByName("positions"));
    
    //Animation editor button.
    ButtonGuiItem* anim_ed_button =
        new ButtonGuiItem("Animations", game.sysContent.fntAreaName);
    anim_ed_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_ANIM_EDITOR, draw.center, draw.size, anim_editor_icon_left
        );
        drawButton(
            draw.center, draw.size,
            anim_ed_button->text, anim_ed_button->font,
            anim_ed_button->color, anim_ed_button->selected,
            anim_ed_button->getJuiceValue()
        );
    };
    anim_ed_button->onActivate =
    [] (const Point &) {
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.animationEd);
        });
    };
    anim_ed_button->onGetTooltip =
    [] () { return "Make an animation for any object in the game."; };
    makeGui.addItem(anim_ed_button, "animation_editor");
    
    //Area editor button.
    ButtonGuiItem* area_ed_button =
        new ButtonGuiItem("Areas", game.sysContent.fntAreaName);
    area_ed_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_AREA_EDITOR, draw.center, draw.size, area_editor_icon_left
        );
        drawButton(
            draw.center, draw.size,
            area_ed_button->text, area_ed_button->font,
            area_ed_button->color, area_ed_button->selected,
            area_ed_button->getJuiceValue()
        );
    };
    area_ed_button->onActivate =
    [] (const Point &) {
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.areaEd);
        });
    };
    area_ed_button->onGetTooltip =
    [] () { return "Make an area to play on."; };
    makeGui.addItem(area_ed_button, "area_editor");
    
    //Particle editor button.
    ButtonGuiItem* part_ed_button =
        new ButtonGuiItem("Particles", game.sysContent.fntAreaName);
    part_ed_button->onDraw =
    [ = ](const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_PARTICLE_EDITOR, draw.center, draw.size, particle_editor_icon_left
        );
        drawButton(
            draw.center, draw.size,
            part_ed_button->text, part_ed_button->font,
            part_ed_button->color, part_ed_button->selected,
            part_ed_button->getJuiceValue()
        );
    };
    part_ed_button->onActivate =
    [](const Point &) {
        game.fadeMgr.startFade(false, []() {
            game.changeState(game.states.particleEd);
        });
    };
    part_ed_button->onGetTooltip =
    []() { return "Make generators that create particles."; };
    makeGui.addItem(part_ed_button, "particle_editor");
    
    //GUI editor button.
    ButtonGuiItem* gui_ed_button =
        new ButtonGuiItem("GUI", game.sysContent.fntAreaName);
    gui_ed_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_GUI_EDITOR, draw.center, draw.size, gui_editor_icon_left
        );
        drawButton(
            draw.center, draw.size,
            gui_ed_button->text, gui_ed_button->font,
            gui_ed_button->color, gui_ed_button->selected,
            gui_ed_button->getJuiceValue()
        );
    };
    gui_ed_button->onActivate =
    [] (const Point &) {
        game.fadeMgr.startFade(false, [] () {
            game.changeState(game.states.guiEd);
        });
    };
    gui_ed_button->onGetTooltip =
    [] () { return "Change the way menus and the gameplay HUD look."; };
    makeGui.addItem(gui_ed_button, "gui_editor");
    
    //Back button.
    makeGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntAreaName);
    makeGui.backItem->onActivate =
    [this] (const Point &) {
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
    BulletGuiItem* more_bullet =
        new BulletGuiItem("More...", game.sysContent.fntStandard);
    more_bullet->onActivate =
    [] (const Point &) {
        openManual("making.html");
    };
    more_bullet->onGetTooltip =
    [] () {
        return
            "Click to open the manual (in the game's folder) for "
            "more info on content making.";
    };
    makeGui.addItem(more_bullet, "more");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&makeGui);
    makeGui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    makeGui.setSelectedItem(anim_ed_button, true);
    makeGui.responsive = false;
    makeGui.hideItems();
}


/**
 * @brief Loads the GUI elements for the main menu's play page.
 */
void TitleScreen::initGuiPlayPage() {
    DataNode* gui_file = &game.content.guiDefs.list[TITLE_SCREEN::PLAY_GUI_FILE_NAME];
    
    //Button icon positions.
    DataNode* icons_node = gui_file->getChildByName("icons_to_the_left");
    
#define icon_left(name, def) s2b(icons_node->getChildByName(name)-> \
                                 getValueOrDefault(def))
    
    bool simple_areas_icon_left = icon_left("simple_areas", "true");
    bool missions_icon_left = icon_left("missions", "true");
    
#undef icon_left
    
    //Menu items.
    playGui.registerCoords("simple",     42, 60, 60, 12.5);
    playGui.registerCoords("mission",    44, 78, 60, 12.5);
    playGui.registerCoords("back",        9, 91, 14,    6);
    playGui.registerCoords("back_input",  3, 93,  4,    4);
    playGui.registerCoords("tooltip",    50, 96, 96,    4);
    playGui.readCoords(gui_file->getChildByName("positions"));
    
    //Play a simple area button.
    ButtonGuiItem* simple_button =
        new ButtonGuiItem("Simple areas", game.sysContent.fntAreaName);
    simple_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_SIMPLE_AREAS, draw.center, draw.size, simple_areas_icon_left
        );
        drawButton(
            draw.center, draw.size,
            simple_button->text, simple_button->font,
            simple_button->color, simple_button->selected,
            simple_button->getJuiceValue()
        );
    };
    simple_button->onActivate =
    [] (const Point &) {
        game.fadeMgr.startFade(false, [] () {
            game.states.annexScreen->areaMenuAreaType =
                AREA_TYPE_SIMPLE;
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.changeState(game.states.annexScreen);
        });
    };
    simple_button->onGetTooltip =
    [] () { return "Pick a simple area with no goal, and start playing!"; };
    playGui.addItem(simple_button, "simple");
    
    //Play a mission area button.
    ButtonGuiItem* mission_button =
        new ButtonGuiItem("Missions", game.sysContent.fntAreaName);
    mission_button->onDraw =
    [ = ] (const DrawInfo & draw) {
        drawMenuButtonIcon(
            MENU_ICON_MISSIONS, draw.center, draw.size, missions_icon_left
        );
        drawButton(
            draw.center, draw.size,
            mission_button->text, mission_button->font,
            mission_button->color, mission_button->selected,
            mission_button->getJuiceValue()
        );
    };
    mission_button->onActivate =
    [] (const Point &) {
        game.fadeMgr.startFade(false, [] () {
            game.states.annexScreen->areaMenuAreaType =
                AREA_TYPE_MISSION;
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.changeState(game.states.annexScreen);
        });
    };
    mission_button->onGetTooltip =
    [] () {
        return
            "Pick a mission area with goals and limitations, "
            "and start playing!";
    };
    playGui.addItem(mission_button, "mission");
    
    //Back button.
    playGui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntAreaName);
    playGui.backItem->onActivate =
    [this] (const Point &) {
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
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&playGui);
    playGui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    playGui.setSelectedItem(simple_button, true);
    playGui.responsive = false;
    playGui.hideItems();
}


/**
 * @brief Loads the GUI elements for the main menu's tutorial question page.
 */
void TitleScreen::initGuiTutorialPage() {
    DataNode* gui_file = &game.content.guiDefs.list[TITLE_SCREEN::TUTORIAL_GUI_FILE_NAME];
    
    //Menu items.
    tutorialGui.registerCoords("question", 50,     60, 60,  12.5);
    tutorialGui.registerCoords("no",       26, 80.875, 40, 10.25);
    tutorialGui.registerCoords("no_input",  7,     85,  4,     4);
    tutorialGui.registerCoords("yes",      74,     81, 40,    10);
    tutorialGui.registerCoords("tooltip",  50,     96, 96,     4);
    tutorialGui.readCoords(gui_file->getChildByName("positions"));
    
    //Question text.
    TextGuiItem* question_text =
        new TextGuiItem(
        "If you're new to Pikifen, it is recommended to play the "
        "\"Tutorial Meadow\" mission first.\n\n"
        "Do you want to play there now?",
        game.sysContent.fntStandard
    );
    question_text->lineWrap = true;
    tutorialGui.addItem(question_text, "question");
    
    //No button.
    tutorialGui.backItem =
        new ButtonGuiItem("No", game.sysContent.fntStandard);
    tutorialGui.backItem->onActivate =
    [this] (const Point &) {
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
    ButtonGuiItem* yes_button =
        new ButtonGuiItem("Yes", game.sysContent.fntStandard);
    yes_button->onActivate =
    [] (const Point &) {
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
    yes_button->onGetTooltip =
    [] () {
        return
            "Play Tutorial Meadow now.";
    };
    tutorialGui.addItem(yes_button, "yes");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&tutorialGui);
    tutorialGui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    tutorialGui.setSelectedItem(yes_button, true);
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
    
    DataNode* settings_file = &game.content.guiDefs.list[TITLE_SCREEN::GUI_FILE_NAME];
    
    //Resources.
    bmpMenuBg = game.content.bitmaps.list.get(game.sysContentNames.bmpTitleScreenBg);
    
    //Logo pikmin.
    DataNode* logo_node = settings_file->getChildByName("logo");
    ReaderSetter logo_rs(logo_node);
    
    DataNode* pik_types_node =
        logo_node->getChildByName("pikmin_types");
    for(size_t t = 0; t < pik_types_node->getNrOfChildren(); t++) {
        DataNode* type_node = pik_types_node->getChild(t);
        if(type_node->name.empty()) continue;
        logoTypeBitmaps[type_node->name[0]] =
            game.content.bitmaps.list.get(type_node->value, type_node);
    }
    
    DataNode* map_node =
        logo_node->getChildByName("map");
    size_t map_total_rows = map_node->getNrOfChildren();
    size_t map_total_cols = 0;
    for(size_t r = 0; r < map_total_rows; r++) {
        map_total_cols =
            std::max(map_total_cols, map_node->getChild(r)->name.size());
    }
    
    logo_rs.set("min_window_limit", logoMinWindowLimit);
    logo_rs.set("max_window_limit", logoMaxWindowLimit);
    logo_rs.set("pikmin_max_speed", logoPikminMaxSpeed);
    logo_rs.set("pikmin_min_speed", logoPikminMinSpeed);
    logo_rs.set("pikmin_speed_smoothness", logoPikminSpeedSmoothness);
    logo_rs.set("pikmin_sway_amount", logoPikminSwayAmount);
    logo_rs.set("pikmin_sway_max_speed", logoPikminSwayMaxSpeed);
    logo_rs.set("pikmin_sway_min_speed", logoPikminSwayMinSpeed);
    logo_rs.set("pikmin_size", logoPikminSize);
    
    bool map_ok = true;
    
    for(size_t r = 0; r < map_total_rows; r++) {
        string row = map_node->getChild(r)->name;
        
        for(size_t c = 0; c < row.size(); c++) {
            if(row[c] == '.') continue;
            if(logoTypeBitmaps.find(row[c]) == logoTypeBitmaps.end()) {
                map_ok = false;
                game.errors.report(
                    "Title screen Pikmin logo map has an unknown character \"" +
                    string(1, row[c]) + "\" on row " + i2s(r + 1) +
                    ", column " + i2s(c + 1) + "!"
                );
                break;
            }
            
            LogoPikmin pik;
            
            Point min_pos = logoMinWindowLimit;
            min_pos.x *= game.winW / 100.0f;
            min_pos.y *= game.winH / 100.0f;
            Point max_pos = logoMaxWindowLimit;
            max_pos.x *= game.winW / 100.0f;
            max_pos.y *= game.winH / 100.0f;
            
            pik.top = logoTypeBitmaps[row[c]];
            pik.destination =
                Point(
                    min_pos.x +
                    (max_pos.x - min_pos.x) *
                    (c / (float) map_total_cols),
                    min_pos.y +
                    (max_pos.y - min_pos.y) *
                    (r / (float) map_total_rows)
                );
                
            unsigned char h_side = game.rng.i(0, 1);
            unsigned char v_side = game.rng.i(0, 1);
            
            pik.pos =
                Point(
                    game.rng.f(0, game.winW * 0.5),
                    game.rng.f(0, game.winH * 0.5)
                );
                
            if(h_side == 0) {
                pik.pos.x -= game.winW * 1.2;
            } else {
                pik.pos.x += game.winW * 1.2;
            }
            if(v_side == 0) {
                pik.pos.y -= game.winH * 1.2;
            } else {
                pik.pos.y += game.winH * 1.2;
            }
            
            pik.angle = game.rng.f(0, TAU);
            pik.speed = game.rng.f(logoPikminMinSpeed, logoPikminMaxSpeed);
            pik.swaySpeed =
                game.rng.f(logoPikminSwayMinSpeed, logoPikminSwayMaxSpeed);
            pik.swayVar = 0;
            pik.reachedDestination = false;
            logoPikmin.push_back(pik);
        }
        
        if(!map_ok) break;
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
    for(const auto &t : logoTypeBitmaps) {
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
