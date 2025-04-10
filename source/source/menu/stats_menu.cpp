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

#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../core/load.h"
#include "../util/string_utils.h"


namespace STATS_MENU {

//Name of the statistics menu GUI information file.
const string GUI_FILE_NAME = "statistics_menu";

}


/**
 * @brief Adds a new header to the stats list GUI item.
 * @param label Name of the header.
 */
void StatsMenu::addHeader(const string &label) {
    float list_bottom_y = stats_list->getChildBottom();
    const float HEADER_HEIGHT = 0.09f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float stat_center_y =
        list_bottom_y + (HEADER_HEIGHT / 2.0f) +
        (list_bottom_y == 0.0f ? STATS_OFFSET : STAT_PADDING);
        
    TextGuiItem* label_text =
        new TextGuiItem(label, game.sys_content.fnt_area_name);
    label_text->ratio_center =
        Point(0.50f, stat_center_y);
    label_text->ratio_size =
        Point(0.96f, HEADER_HEIGHT);
    stats_list->addChild(label_text);
    gui.addItem(label_text);
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
    const string &label, const string &value, const string &description
) {
    float list_bottom_y = stats_list->getChildBottom();
    const float STAT_HEIGHT = 0.08f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float stat_center_y =
        list_bottom_y + (STAT_HEIGHT / 2.0f) +
        (list_bottom_y == 0.0f ? STATS_OFFSET : STAT_PADDING);
        
    BulletGuiItem* label_bullet =
        new BulletGuiItem(
        label, game.sys_content.fnt_standard
    );
    label_bullet->ratio_center =
        Point(0.50f, stat_center_y);
    label_bullet->ratio_size =
        Point(0.96f, STAT_HEIGHT);
    label_bullet->on_get_tooltip = [description] () { return description; };
    stats_list->addChild(label_bullet);
    gui.addItem(label_bullet);
    
    TextGuiItem* value_text =
        new TextGuiItem(
        value, game.sys_content.fnt_counter, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    value_text->ratio_center =
        Point(0.75f, stat_center_y);
    value_text->ratio_size =
        Point(0.44f, STAT_HEIGHT);
    stats_list->addChild(value_text);
    gui.addItem(value_text);
    
    return value_text;
}


/**
 * @brief Initializes the main GUI.
 */
void StatsMenu::initGuiMain() {
    //Menu items.
    gui.registerCoords("back",        12,  5, 20,  6);
    gui.registerCoords("back_input",   3,  7,  4,  4);
    gui.registerCoords("header",      50,  5, 50,  6);
    gui.registerCoords("list",        50, 51, 76, 82);
    gui.registerCoords("list_scroll", 91, 51,  2, 82);
    gui.registerCoords("tooltip",     50, 96, 96,  4);
    gui.readCoords(
        game.content.gui_defs.list[STATS_MENU::GUI_FILE_NAME].getChildByName("positions")
    );
    
    //Back button.
    gui.back_item =
        new ButtonGuiItem("Back", game.sys_content.fnt_standard);
    gui.back_item->on_activate =
    [this] (const Point &) {
        saveStatistics();
        leave();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Return to the previous menu."; };
    gui.addItem(gui.back_item, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "STATISTICS",
        game.sys_content.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    gui.addItem(header_text, "header");
    
    //Statistics list.
    stats_list = new ListGuiItem();
    gui.addItem(stats_list, "list");
    
    //Statistics list scrollbar.
    ScrollGuiItem* list_scroll = new ScrollGuiItem();
    list_scroll->list_item = stats_list;
    gui.addItem(list_scroll, "list_scroll");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltip_text, "tooltip");
    
    populateStatsList();
    
    //Finishing touches.
    gui.setSelectedItem(gui.back_item, true);
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
        (game.config.general.name.empty() ? "Pikifen" : game.config.general.name) +
        " use"
    );
    addStat(
        "Startups", i2s(game.statistics.startups),
        "Total number of times " +
        (game.config.general.name.empty() ? "Pikifen" : game.config.general.name) +
        " was started."
    );
    runtime_value_text =
        addStat(
            "Runtime", "",
            "Total amount of time " +
            (game.config.general.name.empty() ? "Pikifen" : game.config.general.name) +
            " was running for, in seconds."
        );
    updateRuntimeValueText();
    addStat(
        "Gameplay time",
        timeToStr3(game.statistics.gameplay_time, ":", ":", ""),
        "Total amount of gameplay time, in seconds. Menus, editors, "
        "pause menu, etc. don't count."
    );
    addStat(
        "Area entries", i2s(game.statistics.area_entries),
        "Total number of times that areas were entered. Includes retries "
        "and area editor tests."
    );
    
    addHeader("Pikmin life");
    addStat(
        "Pikmin births", i2s(game.statistics.pikmin_births),
        "Total number of times Pikmin were born from an Onion."
    );
    addStat(
        "Pikmin deaths", i2s(game.statistics.pikmin_deaths),
        "Total number of times Pikmin died in any way."
    );
    addStat(
        "Pikmin eaten", i2s(game.statistics.pikmin_eaten),
        "Total number of times Pikmin were swallowed by an enemy."
    );
    addStat(
        "Pikmin hazard deaths", i2s(game.statistics.pikmin_hazard_deaths),
        "Total number of times Pikmin died from a hazard."
    );
    addStat(
        "Pikmin bloom count", i2s(game.statistics.pikmin_blooms),
        "Total number of times Pikmin matured (leaf to bud, leaf to flower, "
        "or bud to flower)."
    );
    addStat(
        "Pikmin saved", i2s(game.statistics.pikmin_saved),
        "Total number of times the whistle saved Pikmin from a hazard that was "
        "killing them."
    );
    addStat(
        "Enemy deaths", i2s(game.statistics.enemy_deaths),
        "Total number of enemies that died."
    );
    
    addHeader("Leader control");
    addStat(
        "Pikmin thrown", i2s(game.statistics.pikmin_thrown),
        "Total number of times Pikmin were thrown. Leaders thrown don't count."
    );
    addStat(
        "Whistle uses", i2s(game.statistics.whistle_uses),
        "Total number of times the whistle was used."
    );
    addStat(
        "Distance walked (m)",
        f2s((game.statistics.distance_walked * CM_PER_PIXEL) / 100.0),
        "Total distance walked by an active leader, in meters."
    );
    addStat(
        "Leader damage suffered", i2s(game.statistics.leader_damage_suffered),
        "Total amount of damage suffered by leaders."
    );
    addStat(
        "Punch damage caused", i2s(game.statistics.punch_damage_caused),
        "Total amount of damage caused by a leader punching."
    );
    addStat(
        "Leader KOs", i2s(game.statistics.leader_kos),
        "Total amount of times a leader got KO'd."
    );
    addStat(
        "Sprays used", i2s(game.statistics.sprays_used),
        "Total amount of times a spray was used."
    );
    
    DataNode mission_records_file;
    mission_records_file.loadFile(
        FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true
    );
    
    size_t mission_clears = 0;
    size_t mission_platinums = 0;
    long mission_scores = 0;
    
    for(size_t a = 0; a < game.content.areas.list[AREA_TYPE_MISSION].size(); a++) {
        Area* area_ptr = game.content.areas.list[AREA_TYPE_MISSION][a];
        MissionRecord record;
        loadAreaMissionRecord(&mission_records_file, area_ptr, record);
        if(record.clear) {
            mission_clears++;
        }
        if(record.isPlatinum(area_ptr->mission)) {
            mission_platinums++;
        }
        if(area_ptr->mission.grading_mode == MISSION_GRADING_MODE_POINTS) {
            mission_scores += record.score;
        }
    }
    
    addHeader("Missions");
    addStat(
        "Cleared",
        i2s(mission_clears) + "/" + i2s(game.content.areas.list[AREA_TYPE_MISSION].size()),
        "Total amount of missions where the current record is a goal clear."
    );
    addStat(
        "Platinum medals",
        i2s(mission_platinums) + "/" + i2s(game.content.areas.list[AREA_TYPE_MISSION].size()),
        "Total amount of missions where the current record is a platinum medal."
    );
    addStat(
        "Combined score", i2s(mission_scores),
        "Total combined score points of the current records of all missions."
    );
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void StatsMenu::tick(float delta_t) {
    updateRuntimeValueText();
    Menu::tick(delta_t);
}


/**
 * @brief Updates the GUI text item for the runtime stat value.
 */
void StatsMenu::updateRuntimeValueText() {
    runtime_value_text->text =
        timeToStr3(game.statistics.runtime, ":", ":", "");
}
