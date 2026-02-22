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

//The final score label sways with this much of a time offset.
const float FINAL_SCORE_LABEL_SWAY_TIME_OFFSET = -0.4f;

//The final score sways by these many pixels in either direction.
const float FINAL_SCORE_SWAY = 6.0f;

//Time scale for the final score's sway.
const float FINAL_SCORE_SWAY_TIME_SCALE = 2.0f;

//Name of the GUI definition file.
const string GUI_FILE_NAME = "results_menu";

//Multiply the medal size by this.
const float MEDAL_SCALE = 0.9f;

//Multiply the medal shine size by this.
const float MEDAL_SHINE_SCALE = 1.30f;

//Time scale for the medal shine's rotation.
const float MEDAL_SHINE_ROT_TIME_SCALE = 0.7f;

}


/**
 * @brief Creates and adds a new mission score criterion-related stat to the
 * stats list GUI item, if applicable.
 *
 * @param criterionIdx Index of the mission score criterion to use.
 */
void Results::addNewScoreStat(size_t criterionIdx) {
    if(
        game.curArea->type != AREA_TYPE_MISSION ||
        game.curArea->mission.gradingMode != MISSION_GRADING_MODE_POINTS
    ) {
        return;
    }
    
    MissionData* mission = &game.curArea->mission;
    MissionScoreCriterion* cPtr = &mission->scoreCriteria[criterionIdx];
    
    if(cPtr->points == 0) return;
    
    //TODO
    /*
    bool goalWasCleared =
        game.states.gameplay->missionFailReason ==
        (MISSION_FAIL_COND) INVALID;
    bool lost =
        hasFlag(
            game.curArea->missionOld.pointLossData,
            getIdxBitmask(criterion)
        ) &&
        !goalWasCleared;
    
    if(lost) {
        addNewStat(
            "    x 0 points (mission fail) = ",
            "0",
            game.config.guiColors.gold
        );
    } else {
        addNewStat(
            "    x " +
            amountStr(mult, "point") +
            " = ",
            i2s(cPtr->getScore(game.states.gameplay, mission)),
            game.config.guiColors.gold
        );
    }
    */
}


/**
 * @brief Creates and adds a new stat to the stats list GUI item.
 *
 * @param label Label text of this stat.
 * @param value Value of this stat.
 * @param color Color.
 */
void Results::addNewStat(
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
        game.states.gameplay->missionEndEventIdx = INVALID;
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
        AREA_TYPE areaType = game.curArea->type;
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
    //Some setup.
    textToAnimate.clear();
    
    finalMissionScore = game.states.gameplay->calculateMissionScore(false);
    
    MISSION_MEDAL medal = MISSION_MEDAL_NONE;
    switch(game.curArea->mission.gradingMode) {
    case MISSION_GRADING_MODE_POINTS: {
        if(game.states.gameplay->missionWasCleared) {
            medal = game.curArea->mission.getScoreMedal(finalMissionScore);
        }
        break;
    } case MISSION_GRADING_MODE_GOAL: {
        medal =
            game.states.gameplay->missionWasCleared ?
            MISSION_MEDAL_PLATINUM :
            MISSION_MEDAL_NONE;
        break;
    } case MISSION_GRADING_MODE_PARTICIPATION: {
        medal = MISSION_MEDAL_PLATINUM;
        break;
    }
    }
    
    MissionEvent* endEv = nullptr;
    if(game.states.gameplay->missionEndEventIdx != INVALID) {
        endEv =
            &game.curArea->mission.events[
                game.states.gameplay->missionEndEventIdx
            ];
    }
    
    //Record loading and saving logic.
    bool isNewRecord = false;
    bool savedSuccessfully = true;
    MissionRecord oldRecord;
    
    //TODO
    /*
    DataNode missionRecords;
    missionRecords.loadFile(
        FILE_PATHS_FROM_ROOT::MISSION_RECORDS, nullptr, true, false, true
    );
    string recordEntryName =
        getMissionRecordEntryName(game.curArea);
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
    
    if(!oldRecord.clear && goalWasCleared) {
        isNewRecord = true;
    } else if(oldRecord.clear == goalWasCleared) {
        if(
            game.curArea->missionOld.gradingMode ==
            MISSION_GRADING_MODE_POINTS &&
            oldRecord.score < finalMissionScore
        ) {
            isNewRecord = true;
        }
    }
    
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
    */
    
    //Menu items.
    DataNode* guiFile = &game.content.guiDefs.list[RESULTS::GUI_FILE_NAME];
    gui.registerCoords("area_name",         50,  7, 45, 10);
    gui.registerCoords("area_subtitle",     50, 18, 40, 10);
    gui.registerCoords("medal",             85, 15, 22, 22);
    gui.registerCoords("final_score",       85, 28, 26,  4);
    gui.registerCoords("final_score_label", 85, 28, 26,  4);
    gui.registerCoords("end_reason",        85, 28, 26,  4);
    gui.registerCoords("conclusion",        50, 36, 96,  4);
    gui.registerCoords("stats",             50, 63, 80, 38);
    gui.registerCoords("stats_scroll",      93, 63,  2, 38);
    gui.registerCoords("retry",             20, 88, 24,  8);
    gui.registerCoords("continue",          50, 88, 24,  8);
    gui.registerCoords("pick_area",         80, 88, 24,  8);
    gui.registerCoords("pick_area_input",   91, 91,  4,  4);
    gui.registerCoords("tooltip",           50, 96, 96,  4);
    gui.readDataFile(guiFile);
    
    //Area name text.
    TextGuiItem* areaNameText =
        new TextGuiItem(
        game.curArea->name, game.sysContent.fntAreaName,
        game.config.guiColors.gold
    );
    gui.addItem(areaNameText, "area_name");
    textToAnimate.push_back(areaNameText);
    
    //Area subtitle text.
    string subtitle =
        calculateAreaSubtitle(
            game.curArea->subtitle,
            game.curArea->type,
            game.curArea->mission.preset
        );
    if(!subtitle.empty()) {
        TextGuiItem* areaSubtitleText =
            new TextGuiItem(subtitle, game.sysContent.fntAreaName);
        gui.addItem(areaSubtitleText, "area_subtitle");
        textToAnimate.push_back(areaSubtitleText);
    }
    
    //Final score number text.
    if(
        game.curArea->type == AREA_TYPE_MISSION &&
        game.curArea->mission.gradingMode == MISSION_GRADING_MODE_POINTS
    ) {
        TextGuiItem* finalScoreText =
            new TextGuiItem(
            i2s(finalMissionScore), game.sysContent.fntAreaName,
            game.config.guiColors.gold
        );
        finalScoreText->onDraw =
        [finalScoreText] (const DrawInfo & draw) {
            DrawInfo swayDraw = draw;
            swayDraw.center.x +=
                RESULTS::FINAL_SCORE_SWAY *
                sin(game.timePassed * RESULTS::FINAL_SCORE_SWAY_TIME_SCALE);
            finalScoreText->defDrawCode(swayDraw);
        };
        gui.addItem(finalScoreText, "final_score");
    }
    
    //Final score label text.
    if(
        game.curArea->type == AREA_TYPE_MISSION &&
        game.curArea->mission.gradingMode == MISSION_GRADING_MODE_POINTS
    ) {
        TextGuiItem* finalScoreLabelText =
            new TextGuiItem("points!", game.sysContent.fntAreaName);
        finalScoreLabelText->onDraw =
        
        [finalScoreLabelText] (const DrawInfo & draw) {
            DrawInfo swayDraw = draw;
            swayDraw.center.x +=
                RESULTS::FINAL_SCORE_SWAY *
                sin(
                    (
                        game.timePassed +
                        RESULTS::FINAL_SCORE_LABEL_SWAY_TIME_OFFSET
                    ) *
                    RESULTS::FINAL_SCORE_SWAY_TIME_SCALE
                );
            finalScoreLabelText->defDrawCode(swayDraw);
        };
        gui.addItem(finalScoreLabelText, "final_score_label");
    }
    
    //Medal image.
    if(game.curArea->type == AREA_TYPE_MISSION) {
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
            if(medal != MISSION_MEDAL_NONE) {
                drawBitmapInBox(
                    game.sysContent.bmpIdleGlow,
                    draw.center, draw.size * RESULTS::MEDAL_SHINE_SCALE, true,
                    game.timePassed * RESULTS::MEDAL_SHINE_ROT_TIME_SCALE,
                    draw.tint
                );
            }
            drawBitmapInBox(
                bmp, draw.center, draw.size * RESULTS::MEDAL_SCALE,
                true, 0.0f, draw.tint
            );
        };
        gui.addItem(medalItem, "medal");
    }
    
    //End reason text, if any.
    if(game.curArea->type == AREA_TYPE_MISSION) {
        string endReason;
        if(endEv) {
            endReason =
                game.missionEvTypes[endEv->type]->getHudInfo(
                    endEv, &game.curArea->mission, game.states.gameplay
                ).reason;
        }
        
        if(!endReason.empty()) {
            TextGuiItem* endReasonText =
                new TextGuiItem(endReason, game.sysContent.fntStandard);
            gui.addItem(endReasonText, "end_reason");
            textToAnimate.push_back(endReasonText);
        }
    }
    
    //Conclusion text.
    string conclusion;
    switch(game.curArea->type) {
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
    textToAnimate.push_back(conclusionText);
    
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
    if(endEv && endEv->type == MISSION_EV_TIME_LIMIT) {
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
    guiCreateBackInputIcon(&gui, "pick_area_input");
    
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
        game.curArea->type == AREA_TYPE_MISSION &&
        game.curArea->missionOld.startingPoints != 0
    ) {
        //Starting score bullet.
        addNewStat(
            "Starting score: ",
            i2s(game.curArea->missionOld.startingPoints),
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
    addNewStat(
        "Time taken:",
        i2s(minutes) + ":" + padString(i2s(seconds), 2, '0') + "." + i2s(ds)
    );
    
    //Pikmin born bullet.
    addNewStat("Pikmin born:", i2s(game.states.gameplay->pikminBorn));
    
    //Pikmin born points bullet.
    addNewScoreStat(MISSION_SCORE_CRITERIA_PIKMIN_BORN);
    
    //Pikmin deaths bullet.
    addNewStat("Pikmin deaths:", i2s(game.states.gameplay->pikminDeaths));
    
    //Pikmin death points bullet.
    addNewScoreStat(MISSION_SCORE_CRITERIA_PIKMIN_DEATH);
    
    if(
        game.curArea->type == AREA_TYPE_MISSION &&
        game.curArea->missionOld.pointsPerSecLeft != 0
    ) {
        //Seconds left bullet.
        addNewStat(
            "Seconds left:",
            i2s(
                game.curArea->missionOld.failTimeLimit -
                floor(game.states.gameplay->gameplayTimePassed)
            )
        );
        
        //Seconds left points bullet.
        addNewScoreStat(MISSION_SCORE_CRITERIA_SEC_LEFT);
    }
    
    if(
        game.curArea->type == AREA_TYPE_MISSION &&
        game.curArea->missionOld.pointsPerSecPassed != 0
    ) {
        //Seconds passed bullet.
        addNewStat(
            "Seconds passed:",
            i2s(game.states.gameplay->gameplayTimePassed)
        );
        
        //Seconds passed points bullet.
        addNewScoreStat(MISSION_SCORE_CRITERIA_SEC_PASSED);
    }
    
    //Treasures bullet.
    addNewStat(
        "Treasures:",
        i2s(game.states.gameplay->treasuresCollected) + "/" +
        i2s(game.states.gameplay->treasuresTotal)
    );
    
    //Treasure points bullet.
    addNewStat(
        "Treasure points:",
        i2s(game.states.gameplay->treasurePointsObtained) + "/" +
        i2s(game.states.gameplay->treasurePointsTotal)
    );
    
    //Treasure points points bullet.
    addNewScoreStat(MISSION_SCORE_CRITERIA_TREASURE_POINTS);
    
    //Enemy defeats bullet.
    addNewStat(
        "Enemy defeats:",
        i2s(game.states.gameplay->enemyDefeats) + "/" +
        i2s(game.states.gameplay->enemyTotal)
    );
    
    //Enemy points bullet.
    addNewStat(
        "Enemy defeat points:",
        i2s(game.states.gameplay->enemyPointsObtained) + "/" +
        i2s(game.states.gameplay->enemyPointsTotal)
    );
    
    //Enemy points points bullet.
    addNewScoreStat(MISSION_SCORE_CRITERIA_ENEMY_POINTS);
    
    if(
        game.curArea->type == AREA_TYPE_MISSION &&
        game.curArea->missionOld.gradingMode == MISSION_GRADING_MODE_POINTS
    ) {
        //Final score bullet.
        addNewStat(
            "Final score:",
            i2s(finalMissionScore),
            game.config.guiColors.gold
        );
        
        //Old record bullet:
        addNewStat(
            "Previous record:",
            oldRecord.date.empty() ? "-" : i2s(oldRecord.score),
            COLOR_WHITE
        );
        
        //Maker's record bullet.
        if(!game.curArea->missionOld.makerRecordDate.empty()) {
            addNewStat(
                "Maker's record:",
                i2s(game.curArea->missionOld.makerRecord),
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
