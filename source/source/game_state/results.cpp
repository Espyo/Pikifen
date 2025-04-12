/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Exploration/mission results state class and results state-related functions.
 */

#include "results.h"

#include "../core/drawing.h"
#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace RESULTS {

//Name of the GUI information file.
const string GUI_FILE_NAME = "results_menu";

}


/**
 * @brief Adds a new mission score criterion-related stat to the stats list
 * GUI item, if applicable.
 *
 * @param criterion Mission score criterion to use.
 */
void Results::addScoreStat(const MISSION_SCORE_CRITERIA criterion) {
    if(
        game.curAreaData->type != AREA_TYPE_MISSION ||
        game.curAreaData->mission.gradingMode != MISSION_GRADING_MODE_POINTS
    ) {
        return;
    }
    
    MissionScoreCriterion* c_ptr = game.missionScoreCriteria[criterion];
    MissionData* mission = &game.curAreaData->mission;
    int mult = c_ptr->getMultiplier(mission);
    
    if(mult == 0) return;
    
    bool goal_was_cleared =
        game.states.gameplay->missionFailReason ==
        (MISSION_FAIL_COND) INVALID;
    bool lost =
        hasFlag(
            game.curAreaData->mission.pointLossData,
            getIdxBitmask(criterion)
        ) &&
        !goal_was_cleared;
        
    if(lost) {
        addStat(
            "    x 0 points (mission fail) = ",
            "0",
            COLOR_GOLD
        );
    } else {
        addStat(
            "    x " +
            amountStr(mult, "point") +
            " = ",
            i2s(c_ptr->getScore(game.states.gameplay, mission)),
            COLOR_GOLD
        );
    }
}


/**
 * @brief Adds a new stat to the stats list GUI item.
 *
 * @param label Label text of this stat.
 * @param value Value of this stat.
 * @param color Color.
 */
void Results::addStat(
    const string &label, const string &value,
    const ALLEGRO_COLOR &color
) {
    size_t stat_idx = statsList->children.size() / 2.0f;
    const float STAT_HEIGHT = 0.12f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float stat_center_y =
        (STATS_OFFSET + STAT_HEIGHT / 2.0f) +
        ((STAT_HEIGHT + STAT_PADDING) * stat_idx);
        
    BulletGuiItem* label_bullet =
        new BulletGuiItem(
        label, game.sysContent.fntStandard, color
    );
    label_bullet->ratioCenter =
        Point(0.50f, stat_center_y);
    label_bullet->ratioSize =
        Point(0.96f, STAT_HEIGHT);
    statsList->addChild(label_bullet);
    gui.addItem(label_bullet);
    
    TextGuiItem* value_text =
        new TextGuiItem(
        value, game.sysContent.fntCounter, color, ALLEGRO_ALIGN_RIGHT
    );
    value_text->ratioCenter =
        Point(0.75f, stat_center_y);
    value_text->ratioSize =
        Point(0.44f, STAT_HEIGHT);
    statsList->addChild(value_text);
    gui.addItem(value_text);
    textToAnimate.push_back(value_text);
}


/**
 * @brief Leaves the results menu and goes back to the gameplay state to
 * continue playing the area.
 */
void Results::continuePlaying() {
    game.fadeMgr.startFade(false, [] () {
        game.states.gameplay->afterHours = true;
        game.states.gameplay->missionFailReason =
            (MISSION_FAIL_COND) INVALID;
        game.changeState(game.states.gameplay, true, false);
        game.states.gameplay->enter();
    });
}


/**
 * @brief Draws the results state.
 */
void Results::doDrawing() {
    //Background.
    al_clear_to_color(al_map_rgb(143, 149, 62));
    
    float logo_width = al_get_bitmap_width(game.sysContent.bmpIcon);
    float logo_height = al_get_bitmap_height(game.sysContent.bmpIcon);
    logo_height = game.winW * 0.08f * (logo_width / logo_height);
    logo_width = game.winW * 0.08f;
    drawBackgroundLogos(
        guiTimeSpent, 6, 6, Point(logo_width, logo_height), mapAlpha(75),
        Point(-60.0f, 30.0f), -TAU / 6.0f
    );
    
    gui.draw();
    
    drawMouseCursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Ticks one frame's worth of logic.
 */
void Results::doLogic() {
    if(!game.fadeMgr.isFading()) {
        for(size_t a = 0; a < game.playerActions.size(); a++) {
            gui.handlePlayerAction(game.playerActions[a]);
        }
    }
    
    guiTimeSpent += game.deltaT;
    
    //Make the different texts grow every two or so seconds.
    const float TEXT_ANIM_ALL_DURATION = 1.5f;
    const float TEXT_ANIM_PAUSE_DURATION = 1.0f;
    const float anim_time =
        fmod(guiTimeSpent, TEXT_ANIM_ALL_DURATION + TEXT_ANIM_PAUSE_DURATION);
    const float time_per_item = TEXT_ANIM_ALL_DURATION / textToAnimate.size();
    const int old_time_cp = (anim_time - game.deltaT) / time_per_item;
    const int new_time_cp = anim_time / time_per_item;
    
    if(
        old_time_cp != new_time_cp &&
        old_time_cp >= 0 &&
        old_time_cp <= (int) textToAnimate.size() - 1
    ) {
        textToAnimate[old_time_cp]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
    
    gui.tick(game.deltaT);
    
    game.fadeMgr.tick(game.deltaT);
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string Results::getName() const {
    return "results";
}


/**
 * @brief Handles Allegro events.
 *
 * @param ev Event to handle.
 */
void Results::handleAllegroEvent(ALLEGRO_EVENT &ev) {
    if(game.fadeMgr.isFading()) return;
    
    gui.handleAllegroEvent(ev);
}


/**
 * @brief Leaves the results menu and goes to the area menu.
 */
void Results::leave() {
    game.fadeMgr.startFade(false, [] () {
        AREA_TYPE area_type = game.curAreaData->type;
        game.unloadLoadedState(game.states.gameplay);
        if(game.states.areaEd->quickPlayAreaPath.empty()) {
            game.states.annexScreen->areaMenuAreaType =
                area_type;
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.changeState(game.states.annexScreen);
        } else {
            game.changeState(game.states.areaEd);
        }
    });
}


/**
 * @brief Loads the results state into memory.
 */
void Results::load() {
    bool goal_was_cleared =
        game.states.gameplay->missionFailReason ==
        (MISSION_FAIL_COND) INVALID;
        
    //Calculate score things.
    finalMissionScore = game.curAreaData->mission.startingPoints;
    
    for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
        MissionScoreCriterion* c_ptr =
            game.missionScoreCriteria[c];
        int c_score =
            c_ptr->getScore(game.states.gameplay, &game.curAreaData->mission);
        bool lost =
            hasFlag(
                game.curAreaData->mission.pointLossData,
                getIdxBitmask(c)
            ) &&
            !goal_was_cleared;
            
        if(!lost) {
            finalMissionScore += c_score;
        }
    }
    
    //Record loading and saving logic.
    MissionRecord old_record;
    
    DataNode mission_records;
    mission_records.loadFile(FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true);
    string record_entry_name =
        getMissionRecordEntryName(game.curAreaData);
    DataNode* entry_node;
    if(mission_records.getNrOfChildrenByName(record_entry_name) > 0) {
        entry_node =
            mission_records.getChildByName(record_entry_name);
    } else {
        entry_node = mission_records.addNew(record_entry_name, "");
    }
    
    vector<string> old_record_parts = split(entry_node->value, ";", true);
    
    if(old_record_parts.size() == 3) {
        old_record.clear = old_record_parts[0] == "1";
        old_record.score = s2i(old_record_parts[1]);
        old_record.date = s2i(old_record_parts[2]);
    }
    
    bool is_new_record = false;
    if(!old_record.clear && goal_was_cleared) {
        is_new_record = true;
    } else if(old_record.clear == goal_was_cleared) {
        if(
            game.curAreaData->mission.gradingMode == MISSION_GRADING_MODE_POINTS &&
            old_record.score < finalMissionScore
        ) {
            is_new_record = true;
        }
    }
    
    bool saved_successfully = true;
    if(
        is_new_record &&
        game.states.areaEd->quickPlayAreaPath.empty() &&
        !game.makerTools.usedHelpingTools &&
        !game.states.gameplay->afterHours
    ) {
        string clear_str = goal_was_cleared ? "1" : "0";
        string score_str = i2s(finalMissionScore);
        string date_str = getCurrentTime(false);
        
        entry_node->value = clear_str + ";" + score_str + ";" + date_str;
        saved_successfully =
            mission_records.saveFile(
                FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true
            );
    }
    
    if(!saved_successfully) {
        showSystemMessageBox(
            nullptr, "Save failed!",
            "Could not save this result!",
            (
                "An error occured while saving the mission record to the "
                "file \"" + FILE_PATHS_FROM_ROOT::MISSION_RECORDS + "\". Make sure that "
                "the folder it is saving to exists and it is not read-only, "
                "and try beating the mission again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
    }
    
    textToAnimate.clear();
    
    //Menu items.
    gui.registerCoords("area_name",        50,  7, 45, 10);
    gui.registerCoords("area_subtitle",    50, 18, 40, 10);
    gui.registerCoords("goal_stamp",       15, 15, 22, 22);
    gui.registerCoords("end_reason",       15, 28, 26,  4);
    gui.registerCoords("medal",            85, 15, 22, 22);
    gui.registerCoords("medal_reason",     85, 28, 26,  4);
    gui.registerCoords("conclusion_label", 50, 32, 36,  4);
    gui.registerCoords("conclusion",       50, 36, 96,  4);
    gui.registerCoords("stats_label",      50, 42, 36,  4);
    gui.registerCoords("stats",            50, 63, 80, 38);
    gui.registerCoords("stats_scroll",     93, 63,  2, 38);
    gui.registerCoords("retry",            20, 88, 24,  8);
    gui.registerCoords("continue",         50, 88, 24,  8);
    gui.registerCoords("pick_area",        80, 88, 24,  8);
    gui.registerCoords("pick_area_input",  91, 91,  4,  4);
    gui.registerCoords("tooltip",          50, 96, 96,  4);
    gui.readCoords(
        game.content.guiDefs.list[RESULTS::GUI_FILE_NAME].getChildByName("positions")
    );
    
    //Area name text.
    TextGuiItem* area_name_text =
        new TextGuiItem(
        game.curAreaData->name, game.sysContent.fntAreaName, COLOR_GOLD
    );
    gui.addItem(area_name_text, "area_name");
    textToAnimate.push_back(area_name_text);
    
    //Area subtitle text.
    string subtitle =
        getSubtitleOrMissionGoal(
            game.curAreaData->subtitle,
            game.curAreaData->type,
            game.curAreaData->mission.goal
        );
    if(!subtitle.empty()) {
        TextGuiItem* area_subtitle_text =
            new TextGuiItem(subtitle, game.sysContent.fntAreaName);
        gui.addItem(area_subtitle_text, "area_subtitle");
        textToAnimate.push_back(area_subtitle_text);
    }
    
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
        //Goal stamp image item.
        GuiItem* goal_stamp_item = new GuiItem;
        goal_stamp_item->onDraw =
        [goal_was_cleared] (const DrawInfo & draw) {
            drawBitmapInBox(
                goal_was_cleared ?
                game.sysContent.bmpMissionClear :
                game.sysContent.bmpMissionFail,
                draw.center, draw.size,
                true
            );
        };
        gui.addItem(goal_stamp_item, "goal_stamp");
        
        //End reason text, if any.
        string end_reason;
        if(goal_was_cleared) {
            end_reason =
                game.missionGoals[game.curAreaData->mission.goal]->
                getEndReason(&game.curAreaData->mission);
        } else {
            end_reason =
                game.missionFailConds[
                    game.states.gameplay->missionFailReason
                ]->getEndReason(
                    &game.curAreaData->mission
                );
        }
        
        if(!end_reason.empty()) {
            TextGuiItem* end_reason_text =
                new TextGuiItem(
                end_reason, game.sysContent.fntStandard,
                goal_was_cleared ?
                al_map_rgba(112, 200, 100, 192) :
                al_map_rgba(242, 160, 160, 192)
            );
            gui.addItem(end_reason_text, "end_reason");
        }
        
        //Medal reason text, if any.
        MISSION_MEDAL medal = MISSION_MEDAL_NONE;
        string medal_reason;
        ALLEGRO_COLOR medal_reason_color;
        switch(game.curAreaData->mission.gradingMode) {
        case MISSION_GRADING_MODE_POINTS: {
            medal_reason = "Got " + i2s(finalMissionScore) + " points";
            if(
                finalMissionScore >=
                game.curAreaData->mission.platinumReq
            ) {
                medal = MISSION_MEDAL_PLATINUM;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(145, 226, 210, 192);
            } else if(
                finalMissionScore >=
                game.curAreaData->mission.goldReq
            ) {
                medal = MISSION_MEDAL_GOLD;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(233, 200, 80, 192);
            } else if(
                finalMissionScore >=
                game.curAreaData->mission.silverReq
            ) {
                medal = MISSION_MEDAL_SILVER;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(216, 216, 200, 192);
            } else if(
                finalMissionScore >=
                game.curAreaData->mission.bronzeReq
            ) {
                medal = MISSION_MEDAL_BRONZE;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(200, 132, 74, 192);
            } else {
                medal = MISSION_MEDAL_NONE;
                medal_reason += "...";
                medal_reason_color = al_map_rgba(200, 200, 200, 192);
            }
            break;
        } case MISSION_GRADING_MODE_GOAL: {
            if(goal_was_cleared) {
                medal = MISSION_MEDAL_PLATINUM;
                medal_reason = "Reached the goal!";
                medal_reason_color = al_map_rgba(145, 226, 210, 192);
            } else {
                medal = MISSION_MEDAL_NONE;
                medal_reason = "Did not reach the goal...";
                medal_reason_color = al_map_rgba(200, 200, 200, 192);
            }
            break;
        } case MISSION_GRADING_MODE_PARTICIPATION: {
            medal = MISSION_MEDAL_PLATINUM;
            medal_reason = "Played the mission!";
            medal_reason_color = al_map_rgba(145, 226, 210, 192);
            break;
        }
        }
        
        //Medal image item.
        GuiItem* medal_item = new GuiItem;
        medal_item->onDraw =
        [medal] (const DrawInfo & draw) {
            ALLEGRO_BITMAP* bmp = nullptr;
            switch(medal) {
            case MISSION_MEDAL_NONE: {
                bmp = game.sysContent.bmpMedalNone;
                break;
            } case MISSION_MEDAL_BRONZE: {
                bmp = game.sysContent.bmpMedalBronze;
                break;
            } case MISSION_MEDAL_SILVER: {
                bmp = game.sysContent.bmpMedalSilver;
                break;
            } case MISSION_MEDAL_GOLD: {
                bmp = game.sysContent.bmpMedalGold;
                break;
            } case MISSION_MEDAL_PLATINUM: {
                bmp = game.sysContent.bmpMedalPlatinum;
                break;
            }
            }
            drawBitmapInBox(bmp, draw.center, draw.size, true);
        };
        gui.addItem(medal_item, "medal");
        
        //Medal reason.
        TextGuiItem* medal_reason_text =
            new TextGuiItem(
            medal_reason, game.sysContent.fntStandard, medal_reason_color
        );
        gui.addItem(medal_reason_text, "medal_reason");
    }
    
    //Conclusion label text.
    string conclusion_label = "Conclusion:";
    TextGuiItem* conclusion_label_text =
        new TextGuiItem(
        conclusion_label, game.sysContent.fntStandard,
        al_map_rgba(255, 255, 255, 192)
    );
    gui.addItem(conclusion_label_text, "conclusion_label");
    
    //Conclusion text.
    string conclusion;
    switch(game.curAreaData->type) {
    case AREA_TYPE_SIMPLE: {
        if(!game.states.areaEd->quickPlayAreaPath.empty()) {
            conclusion =
                "Area editor playtest ended.";
        } else if(game.makerTools.usedHelpingTools) {
            conclusion =
                "Nothing to report, other than maker tools being used.";
        } else {
            conclusion =
                "Nothing to report.";
        }
        break;
    } case AREA_TYPE_MISSION: {
        if(game.states.gameplay->afterHours) {
            conclusion =
                "Played in after hours, so the "
                "result past that point won't be saved.";
        } else if(!game.states.areaEd->quickPlayAreaPath.empty()) {
            conclusion =
                "This was an area editor playtest, "
                "so the result won't be saved.";
        } else if(game.makerTools.usedHelpingTools) {
            conclusion =
                "Maker tools were used, "
                "so the result won't be saved.";
        } else if(
            game.curAreaData->mission.gradingMode == MISSION_GRADING_MODE_POINTS &&
            old_record.clear &&
            !goal_was_cleared &&
            old_record.score < finalMissionScore
        ) {
            conclusion =
                "High score, but the old record was a "
                "clear, so this result won't be saved.";
        } else if(!is_new_record) {
            conclusion =
                "This result is not a new record, so "
                "it won't be saved.";
        } else if(!saved_successfully) {
            conclusion =
                "FAILED TO SAVE THIS RESULT AS A NEW RECORD!";
        } else {
            conclusion =
                "Saved this result as a new record!";
        }
    } case N_AREA_TYPES: {
        break;
    }
    }
    TextGuiItem* conclusion_text =
        new TextGuiItem(conclusion, game.sysContent.fntStandard);
    gui.addItem(conclusion_text, "conclusion");
    
    //Stats label text.
    TextGuiItem* stats_label_text =
        new TextGuiItem(
        "Stats:", game.sysContent.fntStandard, al_map_rgba(255, 255, 255, 192)
    );
    gui.addItem(stats_label_text, "stats_label");
    
    //Stats box.
    statsList = new ListGuiItem();
    statsList->onDraw =
    [this] (const DrawInfo & draw) {
        drawFilledRoundedRectangle(
            draw.center, draw.size, 8.0f, al_map_rgba(0, 0, 0, 40)
        );
        drawTexturedBox(
            draw.center, draw.size, game.sysContent.bmpFrameBox,
            COLOR_TRANSPARENT_WHITE
        );
    };
    gui.addItem(statsList, "stats");
    
    //Stats list scrollbar.
    ScrollGuiItem* stats_scroll = new ScrollGuiItem();
    stats_scroll->listItem = statsList;
    gui.addItem(stats_scroll, "stats_scroll");
    
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.startingPoints != 0
    ) {
        //Starting score bullet.
        addStat(
            "Starting score: ",
            i2s(game.curAreaData->mission.startingPoints),
            COLOR_GOLD
        );
    }
    
    //Time taken bullet.
    unsigned int ds =
        fmod(game.states.gameplay->gameplayTimePassed * 10, 10);
    unsigned char seconds =
        fmod(game.states.gameplay->gameplayTimePassed, 60);
    size_t minutes =
        game.states.gameplay->gameplayTimePassed / 60.0f;
    addStat(
        "Time taken:",
        i2s(minutes) + ":" + padString(i2s(seconds), 2, '0') + "." + i2s(ds)
    );
    
    //Pikmin born bullet.
    addStat("Pikmin born:", i2s(game.states.gameplay->pikminBorn));
    
    //Pikmin born points bullet.
    addScoreStat(MISSION_SCORE_CRITERIA_PIKMIN_BORN);
    
    //Pikmin deaths bullet.
    addStat("Pikmin deaths:", i2s(game.states.gameplay->pikminDeaths));
    
    //Pikmin death points bullet.
    addScoreStat(MISSION_SCORE_CRITERIA_PIKMIN_DEATH);
    
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.pointsPerSecLeft != 0
    ) {
        //Seconds left bullet.
        addStat(
            "Seconds left:",
            i2s(
                game.curAreaData->mission.failTimeLimit -
                floor(game.states.gameplay->gameplayTimePassed)
            )
        );
        
        //Seconds left points bullet.
        addScoreStat(MISSION_SCORE_CRITERIA_SEC_LEFT);
    }
    
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.pointsPerSecPassed != 0
    ) {
        //Seconds passed bullet.
        addStat(
            "Seconds passed:",
            i2s(game.states.gameplay->gameplayTimePassed)
        );
        
        //Seconds passed points bullet.
        addScoreStat(MISSION_SCORE_CRITERIA_SEC_PASSED);
    }
    
    //Treasures bullet.
    addStat(
        "Treasures:",
        i2s(game.states.gameplay->treasuresCollected) + "/" +
        i2s(game.states.gameplay->treasuresTotal)
    );
    
    //Treasure points bullet.
    addStat(
        "Treasure points:",
        i2s(game.states.gameplay->treasurePointsCollected) + "/" +
        i2s(game.states.gameplay->treasurePointsTotal)
    );
    
    //Treasure points points bullet.
    addScoreStat(MISSION_SCORE_CRITERIA_TREASURE_POINTS);
    
    //Enemy deaths bullet.
    addStat(
        "Enemy deaths:",
        i2s(game.states.gameplay->enemyDeaths) + "/" +
        i2s(game.states.gameplay->enemyTotal)
    );
    
    //Enemy points bullet.
    addStat(
        "Enemy kill points:",
        i2s(game.states.gameplay->enemyPointsCollected) + "/" +
        i2s(game.states.gameplay->enemyPointsTotal)
    );
    
    //Enemy points points bullet.
    addScoreStat(MISSION_SCORE_CRITERIA_ENEMY_POINTS);
    
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.gradingMode == MISSION_GRADING_MODE_POINTS
    ) {
        //Final score bullet.
        addStat(
            "Final score:",
            i2s(finalMissionScore),
            COLOR_GOLD
        );
        
        //Old record bullet:
        addStat(
            "Previous record:",
            old_record.date.empty() ? "-" : i2s(old_record.score),
            COLOR_WHITE
        );
        
        //Maker's record bullet.
        if(!game.curAreaData->mission.makerRecordDate.empty()) {
            addStat(
                "Maker's record:",
                i2s(game.curAreaData->mission.makerRecord),
                COLOR_WHITE
            );
        }
    }
    
    
    //Retry button.
    ButtonGuiItem* retry_button =
        new ButtonGuiItem("Retry", game.sysContent.fntStandard);
    retry_button->onActivate =
    [this] (const Point &) {
        retryArea();
    };
    retry_button->onGetTooltip =
    [] () { return "Retry the area from the start."; };
    gui.addItem(retry_button, "retry");
    
    //Keep playing button.
    if(
        game.states.gameplay->missionFailReason ==
        MISSION_FAIL_COND_TIME_LIMIT
    ) {
        ButtonGuiItem* continue_button =
            new ButtonGuiItem("Keep playing", game.sysContent.fntStandard);
        continue_button->onActivate =
        [this] (const Point &) {
            continuePlaying();
        };
        continue_button->onGetTooltip =
        [] () {
            return
                "Continue playing anyway, from where you left off. "
                "Your result after this point won't count.";
        };
        gui.addItem(continue_button, "continue");
    }
    
    //Pick an area button.
    gui.backItem =
        new ButtonGuiItem(
        game.states.areaEd->quickPlayAreaPath.empty() ?
        "Pick an area" :
        "Back to editor",
        game.sysContent.fntStandard
    );
    gui.backItem->onActivate =
    [this] (const Point &) {
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () {
        return
            game.states.areaEd->quickPlayAreaPath.empty() ?
            "Return to the area selection menu." :
            "Return to the area editor.";
    };
    gui.addItem(gui.backItem, "pick_area");
    
    //Pick an area input icon.
    guiAddBackInputIcon(&gui, "pick_area_input");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    game.audio.setCurrentSong(game.sysContentNames.sngEditors);
    game.fadeMgr.startFade(true, nullptr);
    gui.setSelectedItem(gui.backItem, true);
    guiTimeSpent = 0.0f;
}


/**
 * @brief Leaves the results menu and goes back to the gameplay state to retry
 * the area.
 */
void Results::retryArea() {
    game.fadeMgr.startFade(false, [] () {
        game.unloadLoadedState(game.states.gameplay);
        game.changeState(game.states.gameplay);
    });
}


/**
 * @brief Unloads the results state from memory.
 */
void Results::unload() {
    //Menu items.
    gui.destroy();
}
