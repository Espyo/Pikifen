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
#include "../util/os_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace RESULTS {

//The player's score circle in the score chart grows and shrinks by this amount.
const float CHART_CIRCLE_SIZE_OFFSET = 3.0f;

//Time scale for the player's score circle in the score chart.
const float CHART_CIRCLE_TIME_SCALE = 4.0f;

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

//Time scale for the medal shine's rotation.
const float MEDAL_SHINE_ROT_TIME_SCALE = 0.7f;

//Multiply the medal shine size by this.
const float MEDAL_SHINE_SCALE = 1.30f;

//Name of the GUI definition file for the score chart information.
const string SCORE_CHART_GUI_FILE_NAME = "results_menu_score_chart";

//Name of the GUI definition file for the scoring criteria.
const string SCORING_GUI_FILE_NAME = "results_menu_scoring";

//Name of the GUI definition file for the stats.
const string STATS_GUI_FILE_NAME = "results_menu_stats";

}


/**
 * @brief Creates and adds a new bullet point to one of the list GUI items.
 *
 * @param list What list item to add to.
 * @param label Label text of this bullet point.
 * @param value Value of this bullet point.
 * @param color Color.
 */
void Results::addNewBulletPoint(
    GuiItem* list, const string& label, const string& value,
    const ALLEGRO_COLOR& color
) {
    size_t idx = list->children.size() / 2.0f;
    const float BP_HEIGHT = 0.12f;
    const float BP_PADDING = 0.02f;
    const float BP_OFFSET = 0.01f;
    const float centerY =
        (BP_OFFSET + BP_HEIGHT / 2.0f) +
        ((BP_HEIGHT + BP_PADDING) * idx);
        
    BulletGuiItem* labelBullet =
        new BulletGuiItem(
        label, game.sysContent.fntStandard, color
    );
    labelBullet->ratioCenter =
        Point(0.50f, centerY);
    labelBullet->ratioSize =
        Point(0.96f, BP_HEIGHT);
    list->addChild(labelBullet);
    gui.addItem(labelBullet);
    pageTextToAnimate.push_back(labelBullet);
    
    TextGuiItem* valueText =
        new TextGuiItem(
        value, game.sysContent.fntCounter, color, ALLEGRO_ALIGN_RIGHT
    );
    valueText->ratioCenter =
        Point(0.75f, centerY);
    valueText->ratioSize =
        Point(0.44f, BP_HEIGHT);
    list->addChild(valueText);
    gui.addItem(valueText);
    pageTextToAnimate.push_back(valueText);
    textToAnimate.push_back(valueText);
}


/**
 * @brief Creates a button meant for changing to a page either to the left or
 * to the right of the current one, and adds it to the GUI.
 *
 * @param targetPage Which page this button leads to.
 * @param left True if this page is to the left of the current,
 * false if to the right.
 * @param curBox The box GUI item of the current page.
 * @return The button.
 */
ButtonGuiItem* Results::addNewPageItem(
    RESULTS_MENU_PAGE targetPage, bool left, GuiItem* curBox
) {
    string pageName;
    string tooltipName;
    switch(targetPage) {
    case RESULTS_MENU_PAGE_STATS: {
        pageName = "Stats";
        tooltipName = "stats";
        break;
    } case RESULTS_MENU_PAGE_SCORING: {
        pageName = "Scoring";
        tooltipName = "scoring";
        break;
    } case RESULTS_MENU_PAGE_SCORE_CHART: {
        pageName = "Chart";
        tooltipName = "score chart";
        break;
    }
    }
    
    ButtonGuiItem* newButton =
        new ButtonGuiItem(
        left ?
        "< " + pageName :
        pageName + " >",
        game.sysContent.fntStandard,
        game.config.guiColors.pageChange
    );
    newButton->onActivate =
    [this, targetPage] (const Point&) {
        switchPage(targetPage);
    };
    newButton->onGetTooltip =
    [tooltipName] () {
        return "Go to the " + tooltipName + " page.";
    };
    
    return newButton;
}


/**
 * @brief Creates the buttons and input GUI items that allow switching pages,
 * and adds them to the GUI.
 *
 * @param curPage The current page.
 * @param curBox The box GUI item of the current page.
 * @param itemNamePrefix Prefix for the names of the GUI items.
 */
void Results::addNewPageItems(
    RESULTS_MENU_PAGE curPage, GuiItem* curBox, const string& itemNamePrefix
) {
    int curPageIdx = (int) curPage;
    RESULTS_MENU_PAGE leftPage =
        (RESULTS_MENU_PAGE) sumAndWrap(curPageIdx, -1, 3);
    RESULTS_MENU_PAGE rightPage =
        (RESULTS_MENU_PAGE) sumAndWrap(curPageIdx, 1, 3);
        
    //Left page button.
    ButtonGuiItem* leftPageButton =
        addNewPageItem(leftPage, true, curBox);
    curBox->addChild(leftPageButton);
    gui.addItem(leftPageButton, itemNamePrefix + "_left_page");
    pageTextToAnimate.push_back(leftPageButton);
    leftPageButtons.push_back(leftPageButton);
    
    //Left page input icon.
    GuiItem* leftPageInput = new GuiItem();
    leftPageInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showGuiInputIcons) return;
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_MENU_PAGE_LEFT, draw.center, draw.size,
            true, game.sysContent.fntSlim, draw.tint
        );
    };
    curBox->addChild(leftPageInput);
    gui.addItem(leftPageInput, itemNamePrefix + "_left_page_input");
    
    //Right page button.
    ButtonGuiItem* rightPageButton =
        addNewPageItem(rightPage, false, curBox);
    curBox->addChild(rightPageButton);
    gui.addItem(rightPageButton, itemNamePrefix + "_right_page");
    pageTextToAnimate.push_back(rightPageButton);
    rightPageButtons.push_back(rightPageButton);
    
    //Right page input icon.
    GuiItem* rightPageInput = new GuiItem();
    rightPageInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showGuiInputIcons) return;
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT, draw.center, draw.size,
            true, game.sysContent.fntSlim, draw.tint
        );
    };
    curBox->addChild(rightPageInput);
    gui.addItem(rightPageInput, itemNamePrefix + "_right_page_input");
}


/**
 * @brief Creates and adds a new score marker bullet point to the list GUI item.
 *
 * @param label Label text of this bullet point.
 * @param value Value of this bullet point.
 * @param totalBulletPoints How many bullet points will exist in total.
 * @param color Color.
 */
void Results::addNewScoreMarkerBulletPoint(
    const string& label, const string& value,
    const size_t totalBulletPoints, const ALLEGRO_COLOR& color
) {
    size_t idx = scoreChartList->children.size() / 2.0f;
    const float offset = 0.01f;
    const float workingArea = 1.0f - offset * 2.0f;
    const float bPPadding = 0.03f;
    const float totalPaddingSpace = bPPadding * (totalBulletPoints - 1);
    const float totalItemSpace = workingArea - totalPaddingSpace;
    const float bPHeight = totalItemSpace / totalBulletPoints;
    
    const float bPCenterY =
        (offset + bPHeight / 2.0f) +
        ((bPHeight + bPPadding) * idx);
        
    BulletGuiItem* labelBullet =
        new BulletGuiItem(
        label, game.sysContent.fntStandard, color
    );
    labelBullet->ratioCenter =
        Point(0.50f, bPCenterY);
    labelBullet->ratioSize =
        Point(0.96f, bPHeight);
    scoreChartList->addChild(labelBullet);
    gui.addItem(labelBullet);
    pageTextToAnimate.push_back(labelBullet);
    
    TextGuiItem* valueText =
        new TextGuiItem(
        value, game.sysContent.fntCounter, color, ALLEGRO_ALIGN_RIGHT
    );
    valueText->ratioCenter =
        Point(0.75f, bPCenterY);
    valueText->ratioSize =
        Point(0.44f, bPHeight);
    scoreChartList->addChild(valueText);
    gui.addItem(valueText);
    textToAnimate.push_back(valueText);
    pageTextToAnimate.push_back(valueText);
    
    scoreMarkerGuiItems.push_back(labelBullet);
}


/**
 * @brief Leaves the results menu and goes back to the gameplay state to
 * continue playing the area.
 */
void Results::continuePlaying() {
    game.fadeMgr.startFade(false, [] () {
        game.states.gameplay->afterHours = true;
        game.states.gameplay->missionEndCondIdx = INVALID;
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
    if(scoreChartPageBox->visible) drawScoreChartConnections();
    
    drawMouseCursor(game.config.guiColors.standardMouseCursor);
}


/**
 * @brief Ticks one frame's worth of logic.
 */
void Results::doLogic() {
    if(!game.fadeMgr.isFading()) {
        forIdx(a, game.controls.actionQueue) {
            gui.handlePlayerAction(game.controls.actionQueue[a]);
            switch(game.controls.actionQueue[a].actionTypeId) {
            case PLAYER_ACTION_TYPE_MENU_PAGE_LEFT: {
                if(game.controls.actionQueue[a].value < 0.5f) continue;
                forIdx(b, leftPageButtons) {
                    if(leftPageButtons[b]->isResponsive()) {
                        leftPageButtons[b]->activate();
                        break;
                    }
                }
                break;
            } case PLAYER_ACTION_TYPE_MENU_PAGE_RIGHT: {
                if(game.controls.actionQueue[a].value < 0.5f) continue;
                forIdx(b, rightPageButtons) {
                    if(rightPageButtons[b]->isResponsive()) {
                        rightPageButtons[b]->activate();
                        break;
                    }
                }
                break;
            }
            }
        }
    }
    
    guiTimeSpent += game.deltaT;
    
    //Make the different texts grow every two or so seconds.
    const float TEXT_ANIM_FALL_DURATION = 1.5f;
    const float TEXT_ANIM_PAUSE_DURATION = 1.0f;
    const float TEXT_ANIM_TOTAL_DURATION =
        TEXT_ANIM_FALL_DURATION + TEXT_ANIM_PAUSE_DURATION;
    const float newAnimTime = fmod(guiTimeSpent, TEXT_ANIM_TOTAL_DURATION);
    const float newTimeRatio = newAnimTime / TEXT_ANIM_FALL_DURATION;
    const float oldAnimTime = newAnimTime - game.deltaT;
    const float oldTimeRatio = oldAnimTime / TEXT_ANIM_FALL_DURATION;
    
    forIdx(t, textToAnimate) {
        DrawInfo itemDrawInfo;
        bool itemVisible = gui.getItemDrawInfo(textToAnimate[t], &itemDrawInfo);
        if(!itemVisible) continue;
        const float itemRatioY = itemDrawInfo.center.y / game.winH;
        if(passedBy(oldTimeRatio, newTimeRatio, itemRatioY)) {
            textToAnimate[t]->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
            );
        }
    }
    
    gui.tick(game.deltaT);
    
    game.fadeMgr.tick(game.deltaT);
}


/**
 * @brief Draws the score chart's connection lines.
 */
void Results::drawScoreChartConnections() {
    const ALLEGRO_COLOR LINE_COLOR = COLOR_BLACK;
    const ALLEGRO_COLOR SCORE_CIRCLE_COLOR = al_map_rgb(96, 96, 192);
    const ALLEGRO_COLOR MINOR_CIRCLE_COLOR = al_map_rgb(0, 0, 96);
    DrawInfo chartDraw;
    gui.getItemDrawInfo(scoreChartChart, &chartDraw);
    
    //The connector lines.
    forIdx(i, scoreMarkerGuiItems) {
        GuiItem* iPtr =
            scoreMarkerGuiItems[scoreMarkerGuiItems.size() - (1 + i)];
        DrawInfo itemDraw;
        gui.getItemDrawInfo(iPtr, &itemDraw);
        
        const float x1 = itemDraw.center.x + itemDraw.size.x / 2.0f;
        const float x2 = x1 + 24.0f;
        const float x3 = chartDraw.center.x - 24.0f;
        const float x4 = chartDraw.center.x;
        const float y1 = itemDraw.center.y;
        const float y2 = getScoreChartY(scoreMarkers[i].second) - 1.0f;
        const ALLEGRO_COLOR color =
            multAlpha(LINE_COLOR, iPtr->focused ? 0.75f : 0.20f);
        const float thickness = iPtr->focused ? 2.0f : 1.0f;
        
        al_draw_line(x1, y1, x2, y1, color, thickness);
        al_draw_line(x2, y1, x3, y2, color, thickness);
        al_draw_line(x3, y2, x4, y2, color, thickness);
    }
    
    //Circle for the player's score.
    al_draw_filled_circle(
        chartDraw.center.x, getScoreChartY(finalMissionScore),
        16.0f +
        sin(game.timePassed * RESULTS::CHART_CIRCLE_TIME_SCALE) *
        RESULTS::CHART_CIRCLE_SIZE_OFFSET, SCORE_CIRCLE_COLOR
    );
    
    //Circle for the old record.
    if(!oldRecord.date.empty()) {
        al_draw_filled_circle(
            chartDraw.center.x,
            getScoreChartY(oldRecord.score),
            12.0f, MINOR_CIRCLE_COLOR
        );
    }
    
    //Circle for the maker's record.
    if(!game.curArea->mission.makerRecordDate.empty()) {
        al_draw_filled_circle(
            chartDraw.center.x,
            getScoreChartY(game.curArea->mission.makerRecord),
            12.0f, MINOR_CIRCLE_COLOR
        );
    }
}


/**
 * @brief Draws the graphic representing the score chart.
 *
 * @param draw Draw info.
 */
void Results::drawScoreChartGraphic(const DrawInfo& draw) {
    const ALLEGRO_COLOR AXIS_COLOR = al_map_rgb(0, 0, 96);
    const auto drawRegion =
    [&draw] (float startY, float endY, ALLEGRO_COLOR color, bool endFades) {
        ALLEGRO_VERTEX av[4];
        for(size_t v = 0; v < 4; v++) {
            av[v].u = 0.0f;
            av[v].v = 0.0f;
            av[v].z = 0.0f;
        }
        
        av[0].x = draw.center.x;
        av[0].y = startY;
        av[0].color = color;
        av[1].x = draw.center.x;
        av[1].y = endY;
        av[1].color = endFades ? changeAlpha(color, 0) : color;
        av[2].x = draw.center.x - draw.size.x / 2.0f;
        av[2].y = endY;
        av[2].color = changeAlpha(color, 0);
        av[3].x = draw.center.x - draw.size.x / 2.0f;
        av[3].y = startY;
        av[3].color = changeAlpha(color, 0);
        
        al_draw_prim(
            av, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
        av[2].x = draw.center.x + draw.size.x / 2.0f;
        av[3].x = draw.center.x + draw.size.x / 2.0f;
        
        al_draw_prim(
            av, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
        );
        
    };
    
    //Bronze region.
    drawRegion(
        getScoreChartY(game.curArea->mission.bronzeReq),
        getScoreChartY(game.curArea->mission.silverReq),
        al_map_rgb(229, 175, 126), false
    );
    
    //Silver region.
    drawRegion(
        getScoreChartY(game.curArea->mission.silverReq),
        getScoreChartY(game.curArea->mission.goldReq),
        al_map_rgb(190, 224, 229), false
    );
    
    //Gold region.
    drawRegion(
        getScoreChartY(game.curArea->mission.goldReq),
        getScoreChartY(game.curArea->mission.platinumReq),
        al_map_rgb(229, 212, 110), false
    );
    
    //Platinum region.
    drawRegion(
        getScoreChartY(game.curArea->mission.platinumReq),
        getScoreChartY(scoreChartTop),
        al_map_rgb(110, 229, 193), true
    );
    
    //Line down the middle.
    al_draw_line(
        draw.center.x, draw.center.y - draw.size.y / 2.0f + 8.0f,
        draw.center.x, draw.center.y + draw.size.y / 2.0f,
        AXIS_COLOR, 4.0f
    );
    
    //Line for 0.
    float zeroY = getScoreChartY(0) - 2.0f;
    al_draw_line(
        draw.center.x - 16.0f, zeroY,
        draw.center.x + 16.0f, zeroY,
        AXIS_COLOR, 2.0f
    );
    
    //Triangle at the top.
    drawFilledEquilateralTriangle(
        Point(draw.center.x, draw.center.y - draw.size.y / 2.0f + 8),
        8.0f, -TAU / 4.0f, AXIS_COLOR
    );
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
 * @brief Returns the Y coordinate of the game window at which a given
 * score is represented in the score chart.
 *
 * @param score The score.
 * @return The Y coordinate.
 */
float Results::getScoreChartY(int score) const {
    DrawInfo chartDraw;
    gui.getItemDrawInfo(scoreChartChart, &chartDraw);
    const int chartRange = scoreChartTop - scoreChartBottom;
    const float chartBottomY = chartDraw.center.y + chartDraw.size.y / 2.0f;
    return
        chartBottomY - chartDraw.size.y *
        ((score - scoreChartBottom) / (float) chartRange);
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
 * @brief Initializes the main GUI items.
 */
void Results::initGuiMain() {
    DataNode* guiFile =
        &game.content.guiDefs.list[RESULTS::GUI_FILE_NAME];
    gui.registerCoords("area_name",         50,  7, 45, 10);
    gui.registerCoords("area_subtitle",     50, 18, 40, 10);
    gui.registerCoords("medal",             85, 15, 22, 22);
    gui.registerCoords("final_score",       14, 10, 24,  8);
    gui.registerCoords("final_score_label", 14, 18, 24,  8);
    gui.registerCoords("end_reason",        50, 32, 96,  4);
    gui.registerCoords("conclusion",        50, 36, 96,  4);
    gui.registerCoords("stats_page",        50, 61, 96, 42);
    gui.registerCoords("scoring_page",      50, 61, 96, 42);
    gui.registerCoords("score_chart_page",  50, 61, 96, 42);
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
    if(!game.curArea->subtitle.empty()) {
        TextGuiItem* areaSubtitleText =
            new TextGuiItem(
            game.curArea->subtitle, game.sysContent.fntAreaName
        );
        gui.addItem(areaSubtitleText, "area_subtitle");
        textToAnimate.push_back(areaSubtitleText);
    }
    
    //Final score number text.
    if(game.curArea->type == AREA_TYPE_MISSION) {
        ALLEGRO_COLOR finalScoreTextColor =
            game.curArea->mission.medalAwardMode ==
            MISSION_MEDAL_AWARD_MODE_POINTS ?
            game.config.guiColors.smallHeader :
            COLOR_WHITE;
        string finalScoreTextStr =
            game.curArea->mission.medalAwardMode ==
            MISSION_MEDAL_AWARD_MODE_POINTS ?
            i2s(finalMissionScore) :
            medal == MISSION_MEDAL_NONE ?
            "No" :
            "Got a";
            
        TextGuiItem* finalScoreText =
            new TextGuiItem(
            finalScoreTextStr, game.sysContent.fntAreaName,
            finalScoreTextColor
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
    if(game.curArea->type == AREA_TYPE_MISSION) {
        string finalScoreLabelStr =
            game.curArea->mission.medalAwardMode ==
            MISSION_MEDAL_AWARD_MODE_POINTS ?
            amountStr(finalMissionScore, "point", "", true) + "!" :
            "medal!";
        TextGuiItem* finalScoreLabelText =
            new TextGuiItem(
            finalScoreLabelStr, game.sysContent.fntAreaName
        );
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
        [this] (const DrawInfo & draw) {
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
        if(endCond) {
            endReason = endCond->reason;
        } else if(game.states.gameplay->missionEndFromPauseMenu) {
            endReason = "Ended from the pause menu!";
        }
        
        if(!endReason.empty()) {
            TextGuiItem* endReasonText =
                new TextGuiItem(
                endReason, game.sysContent.fntStandard,
                game.states.gameplay->missionWasCleared ?
                game.config.guiColors.good :
                game.config.guiColors.bad
            );
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
        } else if(!game.options.advanced.expoMode) {
            conclusion =
                "This is an expo showcase, "
                "so the result won't be saved.";
        } else if(!game.quickPlay.areaPath.empty()) {
            conclusion =
                "This was an editor playtest, "
                "so the result won't be saved.";
        } else if(game.makerTools.usedHelpingTools) {
            conclusion =
                "Maker tools were used, "
                "so the result won't be saved.";
        } else if(!game.states.gameplay->missionWasCleared) {
            conclusion =
                "Failed the mission, so the result "
                "won't be saved.";
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
    
    //Stats page box.
    statsPageBox = new GuiItem();
    gui.addItem(statsPageBox, "stats_page");
    
    //Scoring page box.
    scoringPageBox = new GuiItem();
    gui.addItem(scoringPageBox, "scoring_page");
    
    //Score chart page box.
    scoreChartPageBox = new GuiItem();
    gui.addItem(scoreChartPageBox, "score_chart_page");
    
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
        endCond &&
        endCond->type == MISSION_END_COND_METRIC_OR_LESS &&
        endCond->metricType == MISSION_METRIC_SECS_LEFT
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
    guiCreateBackInputIcon(&gui, "pick_area_input");
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltipText, "tooltip");
    
    //Finishing touches.
    gui.setFocusedItem(gui.backItem, true);
}


/**
 * @brief Initializes the score chart GUI items.
 */
void Results::initGuiScoreChart() {
    DataNode* guiFile =
        &game.content.guiDefs.list[RESULTS::SCORE_CHART_GUI_FILE_NAME];
    gui.registerCoords("score_chart_left_page",        13,  7, 18, 14);
    gui.registerCoords("score_chart_left_page_input",   4, 12,  8,  8);
    gui.registerCoords("score_chart_right_page",       87,  7, 18, 14);
    gui.registerCoords("score_chart_right_page_input", 96, 12,  8,  8);
    gui.registerCoords("score_chart_chart",            79, 59, 34, 82);
    gui.registerCoords("score_chart_list",             31, 59, 54, 82);
    gui.readDataFile(guiFile, scoreChartPageBox);
    
    //Score chart item list.
    scoreChartList = new ListGuiItem();
    scoreChartPageBox->addChild(scoreChartList);
    gui.addItem(scoreChartList, "score_chart_list");
    
    //Score chart graphic.
    scoreChartChart = new ListGuiItem();
    scoreChartPageBox->addChild(scoreChartChart);
    scoreChartChart->onDraw =
    [this] (const DrawInfo & draw) {
        drawScoreChartGraphic(draw);
        scoreChartChart->defDrawCode(draw);
    };
    gui.addItem(scoreChartChart, "score_chart_chart");
    
    populateScoreChart();
    
    addNewPageItems(
        RESULTS_MENU_PAGE_SCORE_CHART, scoreChartPageBox, "score_chart"
    );
}


/**
 * @brief Initializes the scoring criteria GUI items.
 */
void Results::initGuiScoring() {
    DataNode* guiFile =
        &game.content.guiDefs.list[RESULTS::SCORING_GUI_FILE_NAME];
    gui.registerCoords("scoring_left_page",        13,  7, 18, 14);
    gui.registerCoords("scoring_left_page_input",   4, 12,  8,  8);
    gui.registerCoords("scoring_right_page",       87,  7, 18, 14);
    gui.registerCoords("scoring_right_page_input", 96, 12,  8,  8);
    gui.registerCoords("scoring_list",             50, 59, 92, 82);
    gui.registerCoords("scoring_scroll",           99, 59,  2, 82);
    gui.readDataFile(guiFile, scoringPageBox);
    
    //Scoring criteria list.
    scoringList = new ListGuiItem();
    scoringPageBox->addChild(scoringList);
    gui.addItem(scoringList, "scoring_list");
    populateScoringList();
    
    //Scoring criteria list scrollbar.
    ScrollGuiItem* scoringScroll = new ScrollGuiItem();
    scoringScroll->listItem = scoringList;
    scoringPageBox->addChild(scoringScroll);
    gui.addItem(scoringScroll, "scoring_scroll");
    
    addNewPageItems(RESULTS_MENU_PAGE_SCORING, scoringPageBox, "scoring");
}


/**
 * @brief Initializes the stats GUI items.
 */
void Results::initGuiStats() {
    DataNode* guiFile =
        &game.content.guiDefs.list[RESULTS::STATS_GUI_FILE_NAME];
    gui.registerCoords("stats_left_page",        13,  7, 18, 14);
    gui.registerCoords("stats_left_page_input",   4, 12,  8,  8);
    gui.registerCoords("stats_right_page",       87,  7, 18, 14);
    gui.registerCoords("stats_right_page_input", 96, 12,  8,  8);
    gui.registerCoords("stats_list",             50, 59, 92, 82);
    gui.registerCoords("stats_scroll",           99, 59,  2, 82);
    gui.readDataFile(guiFile, statsPageBox);
    
    //Stats list.
    statsList = new ListGuiItem();
    statsPageBox->addChild(statsList);
    gui.addItem(statsList, "stats_list");
    populateStatsList();
    
    //Stats list scrollbar.
    ScrollGuiItem* statsScroll = new ScrollGuiItem();
    statsScroll->listItem = statsList;
    statsPageBox->addChild(statsScroll);
    gui.addItem(statsScroll, "stats_scroll");
    
    if(
        game.curArea->type == AREA_TYPE_MISSION &&
        game.curArea->mission.medalAwardMode == MISSION_MEDAL_AWARD_MODE_POINTS
    ) {
        addNewPageItems(RESULTS_MENU_PAGE_STATS, statsPageBox, "stats");
    }
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
    game.audio.setCurrentSong(game.sysContentNames.sngResults);
    game.fadeMgr.startFade(true, nullptr);
    guiTimeSpent = 0.0f;
    pageTextToAnimate.clear();
    leftPageButtons.clear();
    rightPageButtons.clear();
    textToAnimate.clear();
    oldRecord.clear();
    isNewRecord = false;
    
    finalMissionScore = game.states.gameplay->calculateMissionScore(false);
    
    medal = MISSION_MEDAL_NONE;
    switch(game.curArea->mission.medalAwardMode) {
    case MISSION_MEDAL_AWARD_MODE_POINTS: {
        if(game.states.gameplay->missionWasCleared) {
            medal = game.curArea->mission.getScoreMedal(finalMissionScore);
        }
        break;
    } case MISSION_MEDAL_AWARD_MODE_CLEAR: {
        medal =
            game.states.gameplay->missionWasCleared ?
            MISSION_MEDAL_PLATINUM :
            MISSION_MEDAL_NONE;
        break;
    } case MISSION_MEDAL_AWARD_MODE_PARTICIPATION: {
        medal = MISSION_MEDAL_PLATINUM;
        break;
    }
    }
    
    endCond = nullptr;
    if(game.states.gameplay->missionEndCondIdx != INVALID) {
        endCond =
            &game.curArea->mission.endConds[
                game.states.gameplay->missionEndCondIdx
            ];
    }
    
    //Record loading and saving logic.
    DataNode missionRecords;
    missionRecords.loadFile(
        FILE_PATHS_FROM_ROOT::MISSION_RECORDS, nullptr, true, false, true
    );
    string recordEntryName = getMissionRecordEntryName(game.curArea);
    DataNode* entryNode;
    if(missionRecords.getNrOfChildrenByName(recordEntryName) > 0) {
        entryNode = missionRecords.getChildByName(recordEntryName);
        oldRecord.loadFromDataNode(entryNode);
    } else {
        entryNode = missionRecords.addNew(recordEntryName, "");
    }
    
    if(
        oldRecord.date.empty() && game.states.gameplay->missionWasCleared
    ) {
        isNewRecord = true;
    } else if(
        !oldRecord.date.empty() && game.states.gameplay->missionWasCleared &&
        game.curArea->mission.medalAwardMode == MISSION_MEDAL_AWARD_MODE_POINTS
    ) {
        if(oldRecord.score < finalMissionScore) {
            isNewRecord = true;
        }
    }
    
    if(
        isNewRecord &&
        game.quickPlay.areaPath.empty() &&
        !game.makerTools.usedHelpingTools &&
        !game.options.advanced.expoMode &&
        !game.states.gameplay->afterHours
    ) {
        MissionRecord newRecord;
        newRecord.score = finalMissionScore;
        newRecord.date = getCurrentTime(false);
        
        newRecord.saveToDataNode(entryNode);
        savedSuccessfully = saveMissionRecords(&missionRecords);
        
        if(!savedSuccessfully) {
            showSystemMessageBox(
                nullptr, "Save failed!",
                "Could not save this result!",
                (
                    "An error occurred while saving the mission record to the "
                    "file \"" + FILE_PATHS_FROM_ROOT::MISSION_RECORDS +
                    "\". Make sure that "
                    "the folder it is saving to exists and it is not "
                    "read-only, and try beating the mission again."
                ).c_str(),
                nullptr,
                ALLEGRO_MESSAGEBOX_WARN
            );
        }
    }
    
    //Menu items.
    initGuiMain();
    initGuiStats();
    if(
        game.curArea->type == AREA_TYPE_MISSION &&
        game.curArea->mission.medalAwardMode == MISSION_MEDAL_AWARD_MODE_POINTS
    ) {
        initGuiScoring();
        initGuiScoreChart();
    }
    switchPage(RESULTS_MENU_PAGE_STATS);
}


/**
 * @brief Populates the list of score chart and the items.
 */
void Results::populateScoreChart() {
    //Create all score markers.
    scoreMarkers.clear();
    scoreMarkerGuiItems.clear();
    scoreMarkers.push_back(
        std::make_pair(SCORE_MARKER_BRONZE, game.curArea->mission.bronzeReq)
    );
    scoreMarkers.push_back(
        std::make_pair(SCORE_MARKER_SILVER, game.curArea->mission.silverReq)
    );
    scoreMarkers.push_back(
        std::make_pair(SCORE_MARKER_GOLD, game.curArea->mission.goldReq)
    );
    scoreMarkers.push_back(
        std::make_pair(SCORE_MARKER_PLATINUM, game.curArea->mission.platinumReq)
    );
    scoreMarkers.push_back(
        std::make_pair(SCORE_MARKER_SCORE, finalMissionScore)
    );
    if(!oldRecord.date.empty()) {
        scoreMarkers.push_back(
            std::make_pair(SCORE_MARKER_OLD_RECORD, oldRecord.score)
        );
    }
    if(!game.curArea->mission.makerRecordDate.empty()) {
        scoreMarkers.push_back(
            std::make_pair(
                SCORE_MARKER_MAKER_RECORD, game.curArea->mission.makerRecord
            )
        );
    }
    
    //Sort the markers.
    std::stable_sort(
        scoreMarkers.begin(), scoreMarkers.end(),
    [] (const auto & i1, const auto & i2) {
        return i1.second < i2.second;
    }
    );
    
    //Set the score chart's basic limits.
    scoreChartBottom = std::min(0, scoreMarkers[0].second);
    scoreChartTop = scoreMarkers.back().second;
    
    //Add a bit of padding so the arrow and 0 marker can breathe
    //and so that the platinum region can be more than a few pixels.
    const float range = scoreChartTop - scoreChartBottom;
    DrawInfo chartDraw;
    gui.getItemDrawInfo(scoreChartChart, &chartDraw);
    const float topPaddingHeightRatio = 32.0f / chartDraw.size.y;
    const float bottomPaddingHeightRatio = 4.0f / chartDraw.size.y;
    scoreChartTop += std::max(range * topPaddingHeightRatio, 1.0f);
    scoreChartBottom -= std::max(range * bottomPaddingHeightRatio, 1.0f);
    
    //Add score marker items to the list.
    for(int m = scoreMarkers.size() - 1; m >= 0; m--) {
        string name;
        bool highlight = false;
        switch(scoreMarkers[m].first) {
        case SCORE_MARKER_BRONZE: {
            name = "Bronze";
            break;
        } case SCORE_MARKER_SILVER: {
            name = "Silver";
            break;
        } case SCORE_MARKER_GOLD: {
            name = "Gold";
            break;
        } case SCORE_MARKER_PLATINUM: {
            name = "Platinum";
            break;
        } case SCORE_MARKER_SCORE: {
            name = "Your score";
            highlight = true;
            break;
        } case SCORE_MARKER_OLD_RECORD: {
            name = "Old record";
            break;
        } case SCORE_MARKER_MAKER_RECORD: {
            name = "Maker's record";
            break;
        }
        }
        
        addNewScoreMarkerBulletPoint(
            name, i2s(scoreMarkers[m].second), scoreMarkers.size(),
            highlight ? game.config.guiColors.gold : COLOR_WHITE
        );
    }
}


/**
 * @brief Populates the list of scoring criteria.
 */
void Results::populateScoringList() {
    addNewBulletPoint(
        scoringList, "Starting score", i2s(game.curArea->mission.startingPoints)
    );
    
    forIdx(c, game.curArea->mission.scoreCriteria) {
        MissionScoreCriterion* cPtr = &game.curArea->mission.scoreCriteria[c];
        MissionMetricType* typePtr =
            game.missionMetricTypes[cPtr->metricType];
            
        if(cPtr->points == 0) continue;
        
        bool red = false;
        int value = 0;
        if(
            (
                cPtr->metricType == MISSION_METRIC_SECS_LEFT ||
                cPtr->metricType == MISSION_METRIC_SECS_PASSED
            ) && game.states.gameplay->missionConsiderZeroTime
        ) {
            red = true;
        } else {
            value = typePtr->getAmount(cPtr->idxParam) * cPtr->points;
        }
        string name = typePtr->getInfo().friendlyName;
        if(name.empty()) name = typePtr->getInfo().name;
        addNewBulletPoint(
            scoringList,
            name + " x" + i2s(cPtr->points),
            i2s(value),
            red ? game.config.guiColors.bad : COLOR_WHITE
        );
    }
    
    addNewBulletPoint(
        scoringList, "Total", i2s(finalMissionScore)
    );
}


/**
 * @brief Populates the list of statistics.
 */
void Results::populateStatsList() {
    //Time taken bullet.
    unsigned int ds =
        fmod(game.states.gameplay->gameplayTimePassed * 10, 10);
    unsigned char seconds =
        fmod(game.states.gameplay->gameplayTimePassed, 60);
    size_t minutes =
        game.states.gameplay->gameplayTimePassed / 60.0f;
    addNewBulletPoint(
        statsList,
        "Time taken:",
        i2s(minutes) + ":" + padString(i2s(seconds), 2, '0') + "." + i2s(ds)
    );
    
    //Living Pikmin count bullet.
    addNewBulletPoint(
        statsList,
        "Living Pikmin:",
        i2s(game.states.gameplay->getAmountOfTotalPikmin(nullptr, true))
    );
    
    //Pikmin born bullet.
    addNewBulletPoint(
        statsList,
        "Pikmin born:", i2s(game.states.gameplay->pikminBorn)
    );
    
    //Pikmin deaths bullet.
    addNewBulletPoint(
        statsList,
        "Pikmin deaths:", i2s(game.states.gameplay->pikminDeaths)
    );
    
    //Treasures bullet.
    addNewBulletPoint(
        statsList,
        "Treasures collected:",
        i2s(game.states.gameplay->treasuresCollected) + "/" +
        i2s(game.states.gameplay->treasuresTotal)
    );
    
    //Enemy defeats bullet.
    addNewBulletPoint(
        statsList,
        "Enemies defeated:",
        i2s(game.states.gameplay->enemyDefeats) + "/" +
        i2s(game.states.gameplay->enemiesTotal)
    );
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
 * @brief Switches pages.
 *
 * @param newPage The new page to switch to.
 */
void Results::switchPage(RESULTS_MENU_PAGE newPage) {
    statsPageBox->responsive = false;
    statsPageBox->visible = false;
    scoringPageBox->responsive = false;
    scoringPageBox->visible = false;
    scoreChartPageBox->responsive = false;
    scoreChartPageBox->visible = false;
    
    switch(newPage) {
    case RESULTS_MENU_PAGE_STATS: {
        statsPageBox->responsive = true;
        statsPageBox->visible = true;
        break;
    } case RESULTS_MENU_PAGE_SCORING: {
        scoringPageBox->responsive = true;
        scoringPageBox->visible = true;
        break;
    } case RESULTS_MENU_PAGE_SCORE_CHART: {
        scoreChartPageBox->responsive = true;
        scoreChartPageBox->visible = true;
        break;
    }
    }
    
    //Animate them all indiscriminately. The ones that don't belong to the
    //new page won't show up anyway.
    forIdx(i, pageTextToAnimate) {
        pageTextToAnimate[i]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
}


/**
 * @brief Unloads the results state from memory.
 */
void Results::unload() {
    //Menu items.
    gui.destroy();
}
