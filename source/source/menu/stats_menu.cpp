/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Statistics menu struct and functions.
 */

#include "stats_menu.h"

#include "../core/game.h"
#include "../core/load.h"
#include "../core/misc_functions.h"
#include "../util/string_utils.h"


namespace STATS_MENU {

//Name of the statistics menu GUI definition file.
const string GUI_FILE_NAME = "statistics_menu";

}


/**
 * @brief Adds a new header to the stats list GUI item.
 * @param label Name of the header.
 */
void StatsMenu::addHeader(const string& label) {
    float listBottomY = statsList->getChildrenSpan();
    const float HEADER_HEIGHT = 0.09f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float statCenterY =
        listBottomY + (HEADER_HEIGHT / 2.0f) +
        (listBottomY == 0.0f ? STATS_OFFSET : STAT_PADDING);
        
    TextGuiItem* labelText =
        new TextGuiItem(label, game.sysContent.fntAreaName);
    labelText->ratioCenter =
        Point(0.50f, statCenterY);
    labelText->ratioSize =
        Point(0.96f, HEADER_HEIGHT);
    statsList->addChild(labelText);
    gui.addItem(labelText);
}


/**
 * @brief Adds a new stat to the stats list GUI item.
 *
 * @param label Name of the statistic.
 * @param value Its value.
 * @param description Tooltip description.
 * @return The text GUI item for the value.
 */
TextGuiItem* StatsMenu::addStat(
    const string& label, const string& value, const string& description
) {
    float listBottomY = statsList->getChildrenSpan();
    const float STAT_HEIGHT = 0.08f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float statCenterY =
        listBottomY + (STAT_HEIGHT / 2.0f) +
        (listBottomY == 0.0f ? STATS_OFFSET : STAT_PADDING);
        
    BulletGuiItem* labelBullet =
        new BulletGuiItem(
        label, game.sysContent.fntStandard
    );
    labelBullet->ratioCenter =
        Point(0.50f, statCenterY);
    labelBullet->ratioSize =
        Point(0.96f, STAT_HEIGHT);
    labelBullet->onGetTooltip = [description] () { return description; };
    statsList->addChild(labelBullet);
    gui.addItem(labelBullet);
    
    TextGuiItem* valueText =
        new TextGuiItem(
        value, game.sysContent.fntCounter, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    valueText->ratioCenter =
        Point(0.75f, statCenterY);
    valueText->ratioSize =
        Point(0.44f, STAT_HEIGHT);
    statsList->addChild(valueText);
    gui.addItem(valueText);
    
    return valueText;
}


/**
 * @brief Initializes the main GUI.
 */
void StatsMenu::initGuiMain() {
    //Menu items.
    DataNode* guiFile = &game.content.guiDefs.list[STATS_MENU::GUI_FILE_NAME];
    gui.registerCoords("back",        12,  5, 20,  6);
    gui.registerCoords("back_input",   3,  7,  4,  4);
    gui.registerCoords("header",      50,  5, 50,  6);
    gui.registerCoords("list",        50, 51, 76, 82);
    gui.registerCoords("list_scroll", 91, 51,  2, 82);
    gui.registerCoords("tooltip",     50, 96, 96,  4);
    gui.readDataFile(guiFile);
    
    //Back button.
    gui.backItem =
        new ButtonGuiItem("Back", game.sysContent.fntStandard);
    gui.backItem->onActivate =
    [this] (const Point&) {
        saveStatistics();
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    gui.addItem(gui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&gui);
    
    //Header text.
    TextGuiItem* headerText =
        new TextGuiItem(
        "STATISTICS",
        game.sysContent.fntAreaName,
        COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    gui.addItem(headerText, "header");
    
    //Statistics list.
    statsList = new ListGuiItem();
    populateStatsList();
    gui.addItem(statsList, "list");
    
    //Statistics list scrollbar.
    ScrollGuiItem* listScroll = new ScrollGuiItem();
    listScroll->listItem = statsList;
    gui.addItem(listScroll, "list_scroll");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    gui.setFocusedItem(gui.backItem, true);
}


/**
 * @brief Loads the menu.
 */
void StatsMenu::load() {
    //Initialize the GUIs.
    initGuiMain();
    
    //Finish the class menu setup.
    guis.push_back(&gui);
    Menu::load();
}


/**
 * @brief Populates the stats menu with bullet points.
 */
void StatsMenu::populateStatsList() {
    addHeader(
        (
            game.config.general.name.empty() ?
            "Pikifen" :
            game.config.general.name
        ) +
        " use"
    );
    addStat(
        "Startups", i2s(game.statistics.startups),
        "Total number of times " +
        (
            game.config.general.name.empty() ?
            "Pikifen" :
            game.config.general.name
        ) +
        " was started."
    );
    runtimeValueText =
        addStat(
            "Runtime", "",
            "Total amount of time " +
            (
                game.config.general.name.empty() ?
                "Pikifen" :
                game.config.general.name
            ) +
            " was running for, in seconds."
        );
    updateRuntimeValueText();
    addStat(
        "Gameplay time",
        timeToStr3(game.statistics.gameplayTime, ":", ":", ""),
        "Total amount of gameplay time, in seconds. Menus, editors, "
        "pause menu, etc. don't count."
    );
    addStat(
        "Area entries", i2s(game.statistics.areaEntries),
        "Total number of times that areas were entered. Includes retries "
        "and area editor tests."
    );
    
    addHeader("Pikmin life");
    addStat(
        "Pikmin births", i2s(game.statistics.pikminBirths),
        "Total number of times Pikmin were born from an Onion."
    );
    addStat(
        "Pikmin deaths", i2s(game.statistics.pikminDeaths),
        "Total number of times Pikmin died in any way."
    );
    addStat(
        "Pikmin eaten", i2s(game.statistics.pikminEaten),
        "Total number of times Pikmin were swallowed by an enemy."
    );
    addStat(
        "Pikmin hazard deaths", i2s(game.statistics.pikminHazardDeaths),
        "Total number of times Pikmin died from a hazard."
    );
    addStat(
        "Pikmin bloom count", i2s(game.statistics.pikminBlooms),
        "Total number of times Pikmin matured (leaf to bud, leaf to flower, "
        "or bud to flower)."
    );
    addStat(
        "Pikmin saved", i2s(game.statistics.pikminSaved),
        "Total number of times the whistle saved Pikmin from a hazard that was "
        "killing them."
    );
    addStat(
        "Enemy defeats", i2s(game.statistics.enemyDefeats),
        "Total number of enemies that were defeated."
    );
    
    addHeader("Leader control");
    addStat(
        "Pikmin thrown", i2s(game.statistics.pikminThrown),
        "Total number of times Pikmin were thrown. Leaders thrown don't count."
    );
    addStat(
        "Whistle uses", i2s(game.statistics.whistleUses),
        "Total number of times the whistle was used."
    );
    addStat(
        "Distance walked (m)",
        f2s((game.statistics.distanceWalked * CM_PER_PIXEL) / 100.0),
        "Total distance walked by an active leader, in meters."
    );
    addStat(
        "Leader damage suffered", i2s(game.statistics.leaderDamageSuffered),
        "Total amount of damage suffered by leaders."
    );
    addStat(
        "Punch damage caused", i2s(game.statistics.punchDamageCaused),
        "Total amount of damage caused by a leader punching."
    );
    addStat(
        "Leader KOs", i2s(game.statistics.leaderKos),
        "Total amount of times a leader got KO'd."
    );
    addStat(
        "Sprays used", i2s(game.statistics.spraysUsed),
        "Total amount of times a spray was used."
    );
    
    DataNode missionRecordsFile;
    missionRecordsFile.loadFile(
        FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true
    );
    
    size_t missionClears = 0;
    size_t missionPlatinums = 0;
    long missionScores = 0;
    
    for(
        size_t a = 0;
        a < game.content.areas.list[AREA_TYPE_MISSION].size(); a++
    ) {
        Area* areaPtr = game.content.areas.list[AREA_TYPE_MISSION][a];
        MissionRecord record;
        loadAreaMissionRecord(&missionRecordsFile, areaPtr, record);
        if(record.clear) {
            missionClears++;
        }
        if(record.isPlatinum(areaPtr->mission)) {
            missionPlatinums++;
        }
        if(areaPtr->mission.gradingMode == MISSION_GRADING_MODE_POINTS) {
            missionScores += record.score;
        }
    }
    
    addHeader("Missions");
    addStat(
        "Cleared",
        i2s(missionClears) + "/" +
        i2s(game.content.areas.list[AREA_TYPE_MISSION].size()),
        "Total amount of missions where the current record is a goal clear."
    );
    addStat(
        "Platinum medals",
        i2s(missionPlatinums) + "/" +
        i2s(game.content.areas.list[AREA_TYPE_MISSION].size()),
        "Total amount of missions where the current record is a platinum medal."
    );
    addStat(
        "Combined score", i2s(missionScores),
        "Total combined score points of the current records of all missions."
    );
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void StatsMenu::tick(float deltaT) {
    updateRuntimeValueText();
    Menu::tick(deltaT);
}


/**
 * @brief Updates the GUI text item for the runtime stat value.
 */
void StatsMenu::updateRuntimeValueText() {
    runtimeValueText->text =
        timeToStr3(game.statistics.runtime, ":", ":", "");
}
