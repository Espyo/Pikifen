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
#include "../core/game.h"
#include "../core/misc_functions.h"
#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace RESULTS {

//Name of the GUI definition file.
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
    
    MissionScoreCriterion* cPtr = game.missionScoreCriteria[criterion];
    MissionData* mission = &game.curAreaData->mission;
    int mult = cPtr->getMultiplier(mission);
    
    if(mult == 0) return;
    
    bool goalWasCleared =
        game.states.gameplay->missionFailReason ==
        (MISSION_FAIL_COND) INVALID;
    bool lost =
        hasFlag(
            game.curAreaData->mission.pointLossData,
            getIdxBitmask(criterion)
        ) &&
        !goalWasCleared;
        
    if(lost) {
        addStat(
            "    x 0 points (mission fail) = ",
            "0",
            game.config.guiColors.gold
        );
    } else {
        addStat(
            "    x " +
            amountStr(mult, "point") +
            " = ",
            i2s(cPtr->getScore(game.states.gameplay, mission)),
            game.config.guiColors.gold
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
    const string& label, const string& value,
    const ALLEGRO_COLOR& color
) {
    size_t statIdx = statsList->children.size() / 2.0f;
    const float STAT_HEIGHT = 0.12f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float statCenterY =
        (STATS_OFFSET + STAT_HEIGHT / 2.0f) +
        ((STAT_HEIGHT + STAT_PADDING) * statIdx);
        
    BulletGuiItem* labelBullet =
        new BulletGuiItem(
        label, game.sysContent.fntStandard, color
    );
    labelBullet->ratioCenter =
        Point(0.50f, statCenterY);
    labelBullet->ratioSize =
        Point(0.96f, STAT_HEIGHT);
    statsList->addChild(labelBullet);
    gui.addItem(labelBullet);
    
    TextGuiItem* valueText =
        new TextGuiItem(
        value, game.sysContent.fntCounter, color, ALLEGRO_ALIGN_RIGHT
    );
    valueText->ratioCenter =
        Point(0.75f, statCenterY);
    valueText->ratioSize =
        Point(0.44f, STAT_HEIGHT);
    statsList->addChild(valueText);
    gui.addItem(valueText);
    textToAnimate.push_back(valueText);
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
        game.audio.setCurrentSong("");
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
    
    float logoWidth = al_get_bitmap_width(game.sysContent.bmpIcon);
    float logoHeight = al_get_bitmap_height(game.sysContent.bmpIcon);
    logoHeight = game.winW * 0.08f * (logoWidth / logoHeight);
    logoWidth = game.winW * 0.08f;
    drawBackgroundLogos(
        guiTimeSpent, 6, 6, Point(logoWidth, logoHeight), mapAlpha(75),
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
        for(size_t a = 0; a < game.controls.actionQueue.size(); a++) {
            gui.handlePlayerAction(game.controls.actionQueue[a]);
        }
    }
    
    guiTimeSpent += game.deltaT;
    
    //Make the different texts grow every two or so seconds.
    const float TEXT_ANIM_ALL_DURATION = 1.5f;
    const float TEXT_ANIM_PAUSE_DURATION = 1.0f;
    const float animTime =
        fmod(guiTimeSpent, TEXT_ANIM_ALL_DURATION + TEXT_ANIM_PAUSE_DURATION);
    const float timePerItem = TEXT_ANIM_ALL_DURATION / textToAnimate.size();
    const int oldTimeCp = (animTime - game.deltaT) / timePerItem;
    const int newTimeCp = animTime / timePerItem;
    
    if(
        oldTimeCp != newTimeCp &&
        oldTimeCp >= 0 &&
        oldTimeCp <= (int) textToAnimate.size() - 1
    ) {
        textToAnimate[oldTimeCp]->startJuiceAnimation(
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
void Results::handleAllegroEvent(ALLEGRO_EVENT& ev) {
    if(game.fadeMgr.isFading()) return;
    
    gui.handleAllegroEvent(ev);
}


/**
 * @brief Leaves the results menu and goes to the area menu.
 */
void Results::leave() {
    game.fadeMgr.startFade(false, [] () {
        AREA_TYPE areaType = game.curAreaData->type;
        game.unloadLoadedState(game.states.gameplay);
        if(game.quickPlay.areaPath.empty()) {
            game.states.annexScreen->areaMenuAreaType =
                areaType;
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.changeState(game.states.annexScreen);
        } else {
            game.changeState(game.quickPlay.editor);
        }
    });
}


/**
 * @brief Loads the results state into memory.
 */
void Results::load() {
    bool goalWasCleared =
        game.states.gameplay->missionFailReason ==
        (MISSION_FAIL_COND) INVALID;
        
    //Calculate score things.
    finalMissionScore = game.curAreaData->mission.startingPoints;
    
    for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
        MissionScoreCriterion* cPtr =
            game.missionScoreCriteria[c];
        int cScore =
            cPtr->getScore(game.states.gameplay, &game.curAreaData->mission);
        bool lost =
            hasFlag(
                game.curAreaData->mission.pointLossData,
                getIdxBitmask(c)
            ) &&
            !goalWasCleared;
            
        if(!lost) {
            finalMissionScore += cScore;
        }
    }
    
    //Record loading and saving logic.
    MissionRecord oldRecord;
    
    DataNode missionRecords;
    missionRecords.loadFile(
        FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true
    );
    string recordEntryName =
        getMissionRecordEntryName(game.curAreaData);
    DataNode* entryNode;
    if(missionRecords.getNrOfChildrenByName(recordEntryName) > 0) {
        entryNode =
            missionRecords.getChildByName(recordEntryName);
    } else {
        entryNode = missionRecords.addNew(recordEntryName, "");
    }
    
    vector<string> oldRecordParts = split(entryNode->value, ";", true);
    
    if(oldRecordParts.size() == 3) {
        oldRecord.clear = oldRecordParts[0] == "1";
        oldRecord.score = s2i(oldRecordParts[1]);
        oldRecord.date = s2i(oldRecordParts[2]);
    }
    
    bool isNewRecord = false;
    if(!oldRecord.clear && goalWasCleared) {
        isNewRecord = true;
    } else if(oldRecord.clear == goalWasCleared) {
        if(
            game.curAreaData->mission.gradingMode ==
            MISSION_GRADING_MODE_POINTS &&
            oldRecord.score < finalMissionScore
        ) {
            isNewRecord = true;
        }
    }
    
    bool savedSuccessfully = true;
    if(
        isNewRecord &&
        game.quickPlay.areaPath.empty() &&
        !game.makerTools.usedHelpingTools &&
        !game.states.gameplay->afterHours
    ) {
        string clearStr = goalWasCleared ? "1" : "0";
        string scoreStr = i2s(finalMissionScore);
        string dateStr = getCurrentTime(false);
        
        entryNode->value = clearStr + ";" + scoreStr + ";" + dateStr;
        savedSuccessfully =
            missionRecords.saveFile(
                FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true
            );
    }
    
    if(!savedSuccessfully) {
        showSystemMessageBox(
            nullptr, "Save failed!",
            "Could not save this result!",
            (
                "An error occurred while saving the mission record to the "
                "file \"" + FILE_PATHS_FROM_ROOT::MISSION_RECORDS +
                "\". Make sure that "
                "the folder it is saving to exists and it is not read-only, "
                "and try beating the mission again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
    }
    
    textToAnimate.clear();
    
    //Menu items.
    DataNode* guiFile = &game.content.guiDefs.list[RESULTS::GUI_FILE_NAME];
    gui.registerCoords("area_name",        50,  7, 45, 10);
    gui.registerCoords("area_subtitle",    50, 18, 40, 10);
    gui.registerCoords("goal_stamp",       15, 15, 22, 22);
    gui.registerCoords("end_reason",       15, 28, 26,  4);
    gui.registerCoords("medal",            85, 15, 22, 22);
    gui.registerCoords("medal_reason",     85, 28, 26,  4);
    gui.registerCoords("conclusion",       50, 36, 96,  4);
    gui.registerCoords("stats",            50, 63, 80, 38);
    gui.registerCoords("stats_scroll",     93, 63,  2, 38);
    gui.registerCoords("retry",            20, 88, 24,  8);
    gui.registerCoords("continue",         50, 88, 24,  8);
    gui.registerCoords("pick_area",        80, 88, 24,  8);
    gui.registerCoords("pick_area_input",  91, 91,  4,  4);
    gui.registerCoords("tooltip",          50, 96, 96,  4);
    gui.readDataFile(guiFile);
    
    //Area name text.
    TextGuiItem* areaNameText =
        new TextGuiItem(
        game.curAreaData->name, game.sysContent.fntAreaName,
        game.config.guiColors.gold
    );
    gui.addItem(areaNameText, "area_name");
    textToAnimate.push_back(areaNameText);
    
    //Area subtitle text.
    string subtitle =
        getSubtitleOrMissionGoal(
            game.curAreaData->subtitle,
            game.curAreaData->type,
            game.curAreaData->mission.goal
        );
    if(!subtitle.empty()) {
        TextGuiItem* areaSubtitleText =
            new TextGuiItem(subtitle, game.sysContent.fntAreaName);
        gui.addItem(areaSubtitleText, "area_subtitle");
        textToAnimate.push_back(areaSubtitleText);
    }
    
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
        //Goal stamp image item.
        GuiItem* goalStampItem = new GuiItem;
        goalStampItem->onDraw =
        [goalWasCleared] (const DrawInfo & draw) {
            drawBitmapInBox(
                goalWasCleared ?
                game.sysContent.bmpMissionClear :
                game.sysContent.bmpMissionFail,
                draw.center, draw.size,
                true, 0.0f, draw.tint
            );
        };
        gui.addItem(goalStampItem, "goal_stamp");
        
        //End reason text, if any.
        string endReason;
        if(goalWasCleared) {
            endReason =
                game.missionGoals[game.curAreaData->mission.goal]->
                getEndReason(&game.curAreaData->mission);
        } else {
            endReason =
                game.missionFailConds[
                    game.states.gameplay->missionFailReason
                ]->getEndReason(
                    &game.curAreaData->mission
                );
        }
        
        if(!endReason.empty()) {
            TextGuiItem* endReasonText =
                new TextGuiItem(
                endReason, game.sysContent.fntStandard,
                goalWasCleared ?
                al_map_rgba(112, 200, 100, 192) :
                al_map_rgba(242, 160, 160, 192)
            );
            gui.addItem(endReasonText, "end_reason");
        }
        
        //Medal reason text, if any.
        MISSION_MEDAL medal = MISSION_MEDAL_NONE;
        string medalReason;
        ALLEGRO_COLOR medalReasonColor;
        switch(game.curAreaData->mission.gradingMode) {
        case MISSION_GRADING_MODE_POINTS: {
            medalReason = "Got " + i2s(finalMissionScore) + " points";
            medal = game.curAreaData->mission.getScoreMedal(finalMissionScore);
            switch(medal) {
            case MISSION_MEDAL_NONE: {
                medalReason += "...";
                medalReasonColor = al_map_rgba(200, 200, 200, 192);
                break;
            } case MISSION_MEDAL_BRONZE: {
                medalReason += "!";
                medalReasonColor = al_map_rgba(200, 132, 74, 192);
                break;
            } case MISSION_MEDAL_SILVER: {
                medalReason += "!";
                medalReasonColor = al_map_rgba(216, 216, 200, 192);
                break;
            } case MISSION_MEDAL_GOLD: {
                medalReason += "!";
                medalReasonColor = al_map_rgba(233, 200, 80, 192);
                break;
            } case MISSION_MEDAL_PLATINUM: {
                medalReason += "!";
                medalReasonColor = al_map_rgba(145, 226, 210, 192);
                break;
            }
            }
            break;
        } case MISSION_GRADING_MODE_GOAL: {
            if(goalWasCleared) {
                medal = MISSION_MEDAL_PLATINUM;
                medalReason = "Reached the goal!";
                medalReasonColor = al_map_rgba(145, 226, 210, 192);
            } else {
                medal = MISSION_MEDAL_NONE;
                medalReason = "Did not reach the goal...";
                medalReasonColor = al_map_rgba(200, 200, 200, 192);
            }
            break;
        } case MISSION_GRADING_MODE_PARTICIPATION: {
            medal = MISSION_MEDAL_PLATINUM;
            medalReason = "Played the mission!";
            medalReasonColor = al_map_rgba(145, 226, 210, 192);
            break;
        }
        }
        
        //Medal image item.
        GuiItem* medalItem = new GuiItem;
        medalItem->onDraw =
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
            drawBitmapInBox(bmp, draw.center, draw.size, true, 0.0f, draw.tint);
        };
        gui.addItem(medalItem, "medal");
        
        //Medal reason.
        TextGuiItem* medalReasonText =
            new TextGuiItem(
            medalReason, game.sysContent.fntStandard, medalReasonColor
        );
        gui.addItem(medalReasonText, "medal_reason");
    }
    
    //Conclusion text.
    string conclusion;
    switch(game.curAreaData->type) {
    case AREA_TYPE_SIMPLE: {
        if(!game.quickPlay.areaPath.empty()) {
            conclusion =
                "Editor playtest ended.";
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
        } else if(!game.quickPlay.areaPath.empty()) {
            conclusion =
                "This was an editor playtest, "
                "so the result won't be saved.";
        } else if(game.makerTools.usedHelpingTools) {
            conclusion =
                "Maker tools were used, "
                "so the result won't be saved.";
        } else if(
            game.curAreaData->mission.gradingMode ==
            MISSION_GRADING_MODE_POINTS &&
            oldRecord.clear &&
            !goalWasCleared &&
            oldRecord.score < finalMissionScore
        ) {
            conclusion =
                "High score, but the old record was a "
                "clear, so this result won't be saved.";
        } else if(!isNewRecord) {
            conclusion =
                "This result is not a new record, so "
                "it won't be saved.";
        } else if(!savedSuccessfully) {
            conclusion =
                "COULD NOT SAVE THIS RESULT AS A NEW RECORD!";
        } else {
            conclusion =
                "Saved this result as a new record!";
        }
    } case N_AREA_TYPES: {
        break;
    }
    }
    TextGuiItem* conclusionText =
        new TextGuiItem(conclusion, game.sysContent.fntStandard);
    gui.addItem(conclusionText, "conclusion");
    
    //Stats box.
    statsList = new ListGuiItem();
    populateStatsList(oldRecord);
    gui.addItem(statsList, "stats");
    
    //Stats list scrollbar.
    ScrollGuiItem* statsScroll = new ScrollGuiItem();
    statsScroll->listItem = statsList;
    gui.addItem(statsScroll, "stats_scroll");
    
    //Retry button.
    ButtonGuiItem* retryButton =
        new ButtonGuiItem("Retry", game.sysContent.fntStandard);
    retryButton->onActivate =
    [this] (const Point&) {
        retryArea();
    };
    retryButton->onGetTooltip =
    [] () { return "Retry the area from the start."; };
    gui.addItem(retryButton, "retry");
    
    //Keep playing button.
    if(
        game.states.gameplay->missionFailReason ==
        MISSION_FAIL_COND_TIME_LIMIT
    ) {
        ButtonGuiItem* continueButton =
            new ButtonGuiItem("Keep playing", game.sysContent.fntStandard);
        continueButton->onActivate =
        [this] (const Point&) {
            continuePlaying();
        };
        continueButton->onGetTooltip =
        [] () {
            return
                "Continue playing anyway, from where you left off. "
                "Your result after this point won't count.";
        };
        gui.addItem(continueButton, "continue");
    }
    
    //Pick an area button.
    gui.backItem =
        new ButtonGuiItem(
        game.quickPlay.areaPath.empty() ?
        "Pick an area" :
        "Back to editor",
        game.sysContent.fntStandard
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () {
        return
            game.quickPlay.areaPath.empty() ?
            "Return to the area selection menu." :
            "Return to the editor.";
    };
    gui.addItem(gui.backItem, "pick_area");
    
    //Pick an area input icon.
    guiAddBackInputIcon(&gui, "pick_area_input");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    game.audio.setCurrentSong(game.sysContentNames.sngResults);
    game.fadeMgr.startFade(true, nullptr);
    gui.setFocusedItem(gui.backItem, true);
    guiTimeSpent = 0.0f;
}


/**
 * @brief Populates the list of statistics.
 *
 * @param oldRecord Old record.
 */
void Results::populateStatsList(const MissionRecord& oldRecord) {
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.startingPoints != 0
    ) {
        //Starting score bullet.
        addStat(
            "Starting score: ",
            i2s(game.curAreaData->mission.startingPoints),
            game.config.guiColors.gold
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
    
    //Enemy defeats bullet.
    addStat(
        "Enemy defeats:",
        i2s(game.states.gameplay->enemyDefeats) + "/" +
        i2s(game.states.gameplay->enemyTotal)
    );
    
    //Enemy points bullet.
    addStat(
        "Enemy defeat points:",
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
            game.config.guiColors.gold
        );
        
        //Old record bullet:
        addStat(
            "Previous record:",
            oldRecord.date.empty() ? "-" : i2s(oldRecord.score),
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
