/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area selection menu struct and related functions.
 */

#include <algorithm>

#include "area_menu.h"

#include "../core/drawing.h"
#include "../core/game.h"
#include "../core/load.h"
#include "../core/misc_functions.h"
#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace AREA_MENU {

//Name of the main GUI information file.
const string GUI_FILE_NAME = "area_menu";

//Path to the area info GUI information file.
const string INFO_GUI_FILE_NAME = "area_menu_info";

//How long to animate the page swapping for.
const float PAGE_SWAP_DURATION = 0.5f;

//Path to the mission specs GUI information file.
const string SPECS_GUI_FILE_NAME = "area_menu_specs";

}


/**
 * @brief Adds a new bullet point to either the fail condition list, or the
 * grading explanation list.
 *
 * @param list List to add to.
 * @param text Text.
 */
void AreaMenu::addBullet(ListGuiItem* list, const string& text) {
    size_t bulletIdx = list->children.size();
    const float BULLET_HEIGHT = 0.18f;
    const float BULLET_PADDING = 0.01f;
    const float BULLETS_OFFSET = 0.01f;
    const float bulletCenterY =
        (BULLETS_OFFSET + BULLET_HEIGHT / 2.0f) +
        ((BULLET_HEIGHT + BULLET_PADDING) * bulletIdx);
        
    BulletGuiItem* bullet =
        new BulletGuiItem(
        text, game.sysContent.fntStandard, COLOR_WHITE
    );
    bullet->ratioCenter = Point(0.50f, bulletCenterY);
    bullet->ratioSize = Point(0.96f, BULLET_HEIGHT);
    list->addChild(bullet);
    gui.addItem(bullet);
}


/**
 * @brief Animates the GUI items inside of the info and specs pages.
 */
void AreaMenu::animateInfoAndSpecs() {
    infoNameText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    subtitleText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    descriptionText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
    );
    difficultyItem->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    tagsText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    makerText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    versionText->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
    );
    if(areaType == AREA_TYPE_MISSION) {
        recordInfoText->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        recordDateText->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        specsNameText->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        goalText->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        for(size_t c = 0; c < failList->children.size(); c++) {
            failList->children[c]->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
            );
        }
        for(size_t c = 0; c < gradingList->children.size(); c++) {
            gradingList->children[c]->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
            );
        }
    }
}


/**
 * @brief Changes the area information to a new area's information.
 *
 * @param areaIdx Index of the newly-selected area.
 */
void AreaMenu::changeInfo(size_t areaIdx) {
    if(areaIdx == curAreaIdx) return;
    curAreaIdx = areaIdx;
    
    //Start by clearing them all, for sanitization's sake.
    infoNameText->text.clear();
    subtitleText->text.clear();
    descriptionText->text.clear();
    curThumb = nullptr;
    tagsText->text.clear();
    makerText->text.clear();
    versionText->text.clear();
    curStamp = nullptr;
    curMedal = nullptr;
    if(areaType == AREA_TYPE_MISSION) {
        recordInfoText->text.clear();
        recordDateText->text.clear();
        goalText->text.clear();
        specsNameText->text.clear();
        failList->deleteAllChildren();
        gradingList->deleteAllChildren();
    }
    
    //Fill in the area's info.
    Area* areaPtr = game.content.areas.list[areaType][areaIdx];
    infoNameText->text = areaPtr->name;
    subtitleText->text =
        getSubtitleOrMissionGoal(
            areaPtr->subtitle,
            areaType,
            areaPtr->mission.goal
        );
    descriptionText->text = areaPtr->description;
    tagsText->text =
        (areaPtr->tags.empty() ? "" : "Tags: " + areaPtr->tags);
    makerText->text =
        (areaPtr->maker.empty() ? "" : "Maker: " + areaPtr->maker);
    versionText->text =
        (areaPtr->version.empty() ? "" : "Version: " + areaPtr->version);
    curThumb = areaPtr->thumbnail.get();
    if(areaType == AREA_TYPE_MISSION) {
        int score = areaRecords[areaIdx].score;
        bool recordExists = !areaRecords[areaIdx].date.empty();
        recordInfoText->text =
            !recordExists ?
            "(None)" :
            areaPtr->mission.gradingMode ==
            MISSION_GRADING_MODE_POINTS ?
            amountStr(score, "point") :
            "";
        curStamp =
            !recordExists ?
            nullptr :
            areaRecords[areaIdx].clear ?
            game.sysContent.bmpMissionClear :
            game.sysContent.bmpMissionFail;
        if(!recordExists) {
            curMedal = nullptr;
        } else {
            switch(areaPtr->mission.gradingMode) {
            case MISSION_GRADING_MODE_POINTS: {
                MISSION_MEDAL medal = areaPtr->mission.getScoreMedal(score);
                switch(medal) {
                case MISSION_MEDAL_NONE: {
                    curMedal = game.sysContent.bmpMedalNone;
                    break;
                } case MISSION_MEDAL_BRONZE: {
                    curMedal = game.sysContent.bmpMedalBronze;
                    break;
                } case MISSION_MEDAL_SILVER: {
                    curMedal = game.sysContent.bmpMedalSilver;
                    break;
                } case MISSION_MEDAL_GOLD: {
                    curMedal = game.sysContent.bmpMedalGold;
                    break;
                } case MISSION_MEDAL_PLATINUM: {
                    curMedal = game.sysContent.bmpMedalPlatinum;
                    break;
                }
                }
                break;
            } case MISSION_GRADING_MODE_GOAL: {
                if(areaRecords[areaIdx].clear) {
                    curMedal = game.sysContent.bmpMedalPlatinum;
                }
                break;
            } case MISSION_GRADING_MODE_PARTICIPATION: {
                curMedal = game.sysContent.bmpMedalPlatinum;
                break;
            }
            }
        }
        recordDateText->text = areaRecords[areaIdx].date;
    }
    
    //Now fill in the mission specs.
    if(areaType == AREA_TYPE_MISSION) {
        specsNameText->text = areaPtr->name;
        MissionData& mission = areaPtr->mission;
        goalText->text =
            game.missionGoals[mission.goal]->
            getPlayerDescription(&mission);
            
        for(size_t f = 0; f < game.missionFailConds.size(); f++) {
            if(hasFlag(mission.failConditions, getIdxBitmask(f))) {
                MissionFail* cond = game.missionFailConds[f];
                addBullet(
                    failList,
                    cond->getPlayerDescription(&mission)
                );
            }
        }
        
        if(mission.failConditions == 0) {
            addBullet(failList, "(None)");
        }
        
        switch(mission.gradingMode) {
        case MISSION_GRADING_MODE_POINTS: {
            addBullet(
                gradingList,
                "Your medal depends on your score:"
            );
            addBullet(
                gradingList,
                "    Platinum: " + i2s(mission.platinumReq) + "+ points."
            );
            addBullet(
                gradingList,
                "    Gold: " + i2s(mission.goldReq) + "+ points."
            );
            addBullet(
                gradingList,
                "    Silver: " + i2s(mission.silverReq) + "+ points."
            );
            addBullet(
                gradingList,
                "    Bronze: " + i2s(mission.bronzeReq) + "+ points."
            );
            vector<string> scoreNotes;
            for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
                MissionScoreCriterion* cPtr =
                    game.missionScoreCriteria[c];
                int mult = cPtr->getMultiplier(&mission);
                if(mult != 0) {
                    scoreNotes.push_back(
                        "    " + cPtr->getName() + " x " + i2s(mult) + "."
                    );
                }
            }
            if(!scoreNotes.empty()) {
                addBullet(
                    gradingList,
                    "Your score is calculated like so:"
                );
                for(size_t s = 0; s < scoreNotes.size(); s++) {
                    addBullet(gradingList, scoreNotes[s]);
                }
            } else {
                addBullet(
                    gradingList,
                    "In this mission, your score will always be 0."
                );
            }
            vector<string> lossNotes;
            for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
                MissionScoreCriterion* cPtr =
                    game.missionScoreCriteria[c];
                if(
                    hasFlag(
                        mission.pointLossData,
                        getIdxBitmask(c)
                    )
                ) {
                    lossNotes.push_back("    " + cPtr->getName());
                }
            }
            if(!lossNotes.empty()) {
                addBullet(
                    gradingList,
                    "If you fail, you'll lose your score for:"
                );
                for(size_t l = 0; l < lossNotes.size(); l++) {
                    addBullet(gradingList, lossNotes[l]);
                }
            }
            if(!mission.makerRecordDate.empty()) {
                addBullet(
                    gradingList,
                    "Maker's record: " + i2s(mission.makerRecord) +
                    " (" + mission.makerRecordDate + ")"
                );
            }
            break;
        }
        case MISSION_GRADING_MODE_GOAL: {
            addBullet(
                gradingList,
                "You get a platinum medal if you clear the goal."
            );
            addBullet(
                gradingList,
                "You get no medal if you fail."
            );
            break;
        }
        case MISSION_GRADING_MODE_PARTICIPATION: {
            addBullet(
                gradingList,
                "You get a platinum medal just by playing the mission."
            );
            break;
        }
        }
    }
    
    animateInfoAndSpecs();
}


/**
 * @brief Initializes the area info page GUI items.
 */
void AreaMenu::initGuiInfoPage() {
    gui.registerCoords("info_name",    36,  6, 68,  8);
    gui.registerCoords("subtitle",     36, 16, 68,  8);
    gui.registerCoords("thumbnail",    85, 14, 26, 24);
    gui.registerCoords("description",  50, 40, 96, 24);
    gui.registerCoords("record_label", 50, 56, 96,  4);
    gui.registerCoords("record_info",  50, 62, 36,  4);
    gui.registerCoords("record_stamp", 20, 65, 20, 14);
    gui.registerCoords("record_medal", 80, 65, 20, 14);
    gui.registerCoords("record_date",  50, 66, 28,  4);
    gui.registerCoords("difficulty",   50, 79, 96,  6);
    gui.registerCoords("tags",         50, 87, 96,  6);
    gui.registerCoords("maker",        28, 95, 52,  6);
    gui.registerCoords("version",      76, 95, 44,  6);
    gui.readCoords(
        game.content.guiDefs.list[AREA_MENU::INFO_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    if(!game.content.areas.list[areaType].empty()) {
    
        //Name text.
        infoNameText =
            new TextGuiItem("", game.sysContent.fntAreaName, COLOR_GOLD);
        infoBox->addChild(infoNameText);
        gui.addItem(infoNameText, "info_name");
        
        //Subtitle text.
        subtitleText = new TextGuiItem("", game.sysContent.fntAreaName);
        infoBox->addChild(subtitleText);
        gui.addItem(subtitleText, "subtitle");
        
        //Thumbnail.
        GuiItem* thumbItem = new GuiItem();
        thumbItem->onDraw =
        [this] (const DrawInfo & draw) {
            //Make it a square.
            Point finalSize(
                std::min(draw.size.x, draw.size.y),
                std::min(draw.size.x, draw.size.y)
            );
            //Align it to the top-right corner.
            Point finalCenter(
                (draw.center.x + draw.size.x / 2.0f) - finalSize.x / 2.0f,
                (draw.center.y - draw.size.y / 2.0f) + finalSize.y / 2.0f
            );
            if(curThumb) {
                drawBitmap(curThumb, finalCenter, finalSize - 4.0f);
            }
            drawTexturedBox(
                finalCenter, finalSize, game.sysContent.bmpFrameBox,
                COLOR_TRANSPARENT_WHITE
            );
        };
        infoBox->addChild(thumbItem);
        gui.addItem(thumbItem, "thumbnail");
        
        //Description text.
        descriptionText =
            new TextGuiItem(
            "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        descriptionText->lineWrap = true;
        infoBox->addChild(descriptionText);
        gui.addItem(descriptionText, "description");
        
        if(areaType == AREA_TYPE_MISSION) {
            //Record label.
            TextGuiItem* recordLabelText =
                new TextGuiItem("Record:", game.sysContent.fntStandard);
            infoBox->addChild(recordLabelText);
            gui.addItem(recordLabelText, "record_label");
            
            //Record info.
            recordInfoText =
                new TextGuiItem("", game.sysContent.fntStandard);
            infoBox->addChild(recordInfoText);
            gui.addItem(recordInfoText, "record_info");
            
            //Record stamp.
            GuiItem* recordStampItem = new GuiItem();
            recordStampItem->onDraw =
            [this] (const DrawInfo & draw) {
                if(curStamp) {
                    drawBitmapInBox(
                        curStamp, draw.center, draw.size, true
                    );
                }
            };
            infoBox->addChild(recordStampItem);
            gui.addItem(recordStampItem, "record_stamp");
            
            //Record medal.
            GuiItem* recordMedalItem = new GuiItem();
            recordMedalItem->onDraw =
            [this] (const DrawInfo & draw) {
                if(curMedal) {
                    drawBitmapInBox(
                        curMedal, draw.center, draw.size, true
                    );
                }
            };
            infoBox->addChild(recordMedalItem);
            gui.addItem(recordMedalItem, "record_medal");
            
            //Record date.
            recordDateText =
                new TextGuiItem(
                "", game.sysContent.fntSlim, al_map_rgb(128, 128, 128)
            );
            infoBox->addChild(recordDateText);
            gui.addItem(recordDateText, "record_date");
        }
        
        //Difficulty item.
        difficultyItem = new GuiItem();
        difficultyItem->onDraw = [this] (const DrawInfo & draw) {
            Area* areaPtr = game.content.areas.list[areaType][curAreaIdx];
            if(areaPtr->difficulty == 0) return;
            const string difficultyText = "Difficulty: ";
            
            drawText(
                difficultyText, game.sysContent.fntStandard,
                Point(draw.center.x - draw.size.x / 2.0f, draw.center.y),
                draw.size,
                COLOR_WHITE, ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER,
                TEXT_SETTING_FLAG_CANT_GROW,
                Point(1.0 + difficultyItem->getJuiceValue())
            );
            
            Point iconSize =
                resizeToBoxKeepingAspectRatio(
                    getBitmapDimensions(game.sysContent.bmpDifficulty),
                    Point(draw.size.x / 5.0f, draw.size.y)
                );
            const float iconsX2 = draw.center.x + draw.size.x / 2.0f;
            for(unsigned char i = 0; i < areaPtr->difficulty; i++) {
                drawBitmap(
                    game.sysContent.bmpDifficulty,
                    Point(
                        iconsX2 - iconSize.x * i - iconSize.x / 2.0f,
                        draw.center.y
                    ),
                    iconSize
                );
            }
        };
        infoBox->addChild(difficultyItem);
        gui.addItem(difficultyItem, "difficulty");
        
        //Tags text.
        tagsText =
            new TextGuiItem(
            "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        infoBox->addChild(tagsText);
        gui.addItem(tagsText, "tags");
        
        //Maker text.
        makerText =
            new TextGuiItem(
            "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        infoBox->addChild(makerText);
        gui.addItem(makerText, "maker");
        
        //Version text.
        versionText =
            new TextGuiItem(
            "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
        );
        infoBox->addChild(versionText);
        gui.addItem(versionText, "version");
        
    }
}


/**
 * @brief Initializes the main GUI items.
 */
void AreaMenu::initGuiMain() {
    gui.registerCoords("back",          12,  5, 20,  6);
    gui.registerCoords("back_input",     3,  7,  4,  4);
    gui.registerCoords("header",        40,  5, 32,  6);
    gui.registerCoords("list",          20, 51, 36, 82);
    gui.registerCoords("list_scroll",   40, 51,  2, 82);
    gui.registerCoords("view_toggle",   74,  5, 32,  6);
    gui.registerCoords("info_box",      70, 51, 56, 82);
    gui.registerCoords("specs_box",     70, 51, 56, 82);
    gui.registerCoords("random",        95,  5,  6,  6);
    gui.registerCoords("tooltip",       50, 96, 96,  4);
    gui.registerCoords("no_areas_text", 50, 50, 96, 10);
    
    gui.readCoords(
        game.content.guiDefs.list[AREA_MENU::GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    //Back button.
    gui.backItem =
        new ButtonGuiItem(
        "Back", game.sysContent.fntStandard
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
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
        "PICK AN AREA:",
        game.sysContent.fntAreaName, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_LEFT
    );
    gui.addItem(headerText, "header");
    
    if(!game.content.areas.list[areaType].empty()) {
    
        //Area list box.
        listBox = new ListGuiItem();
        gui.addItem(listBox, "list");
        
        //Area list scrollbar.
        ScrollGuiItem* listScroll = new ScrollGuiItem();
        listScroll->listItem = listBox;
        gui.addItem(listScroll, "list_scroll");
        
        //Items for the various areas.
        for(size_t a = 0; a < game.content.areas.list[areaType].size(); a++) {
            Area* areaPtr = game.content.areas.list[areaType][a];
            const float BUTTON_HEIGHT = 0.09f;
            const float centerY = 0.045f + a * 0.10f;
            
            //Area button.
            ButtonGuiItem* areaButton =
                new ButtonGuiItem(areaPtr->name, game.sysContent.fntStandard);
            areaButton->ratioCenter =
                Point(
                    areaType == AREA_TYPE_MISSION ? 0.40f : 0.50f,
                    centerY
                );
            areaButton->ratioSize =
                Point(
                    areaType == AREA_TYPE_MISSION ? 0.80f : 1.00f,
                    BUTTON_HEIGHT
                );
            areaButton->onActivate =
            [this, areaPtr] (const Point&) {
                game.states.gameplay->pathOfAreaToLoad =
                    areaPtr->manifest->path;
                game.fadeMgr.startFade(false, [] () {
                    game.changeState(game.states.gameplay);
                });
            };
            areaButton->onSelected =
            [this, a] () { changeInfo(a); };
            areaButton->onGetTooltip =
            [areaPtr] () { return "Play " + areaPtr->name + "."; };
            listBox->addChild(areaButton);
            gui.addItem(areaButton);
            areaButtons.push_back(areaButton);
            if(!firstAreaButton) {
                firstAreaButton = areaButton;
            }
            
            if(areaType == AREA_TYPE_MISSION) {
                //Stamp item.
                GuiItem* stampItem = new GuiItem();
                stampItem->ratioCenter =
                    Point(0.85f, centerY - (BUTTON_HEIGHT * 0.15f));
                stampItem->ratioSize =
                    Point(0.12f, BUTTON_HEIGHT * 0.60f);
                stampItem->onDraw =
                [this, a] (const DrawInfo & draw) {
                    if(areaRecords[a].clear) {
                        drawBitmapInBox(
                            game.sysContent.bmpMissionClear,
                            draw.center, draw.size, true
                        );
                    }
                };
                listBox->addChild(stampItem);
                gui.addItem(stampItem);
                
                //Medal item.
                GuiItem* medalItem = new GuiItem();
                medalItem->ratioCenter =
                    Point(0.95f, centerY + (BUTTON_HEIGHT * 0.15f));
                medalItem->ratioSize =
                    Point(0.12f, BUTTON_HEIGHT * 0.60f);
                medalItem->onDraw =
                [this, areaPtr, a] (const DrawInfo & draw) {
                    ALLEGRO_BITMAP* medalBmp = nullptr;
                    switch(areaPtr->mission.gradingMode) {
                    case MISSION_GRADING_MODE_POINTS: {
                        int score = areaRecords[a].score;
                        MISSION_MEDAL medal =
                            areaPtr->mission.getScoreMedal(score);
                        switch(medal) {
                        case MISSION_MEDAL_NONE: {
                            break;
                        } case MISSION_MEDAL_BRONZE: {
                            medalBmp = game.sysContent.bmpMedalBronze;
                            break;
                        } case MISSION_MEDAL_SILVER: {
                            medalBmp = game.sysContent.bmpMedalSilver;
                            break;
                        } case MISSION_MEDAL_GOLD: {
                            medalBmp = game.sysContent.bmpMedalGold;
                            break;
                        } case MISSION_MEDAL_PLATINUM: {
                            medalBmp = game.sysContent.bmpMedalPlatinum;
                            break;
                        }
                        }
                        break;
                    } case MISSION_GRADING_MODE_GOAL: {
                        if(areaRecords[a].clear) {
                            medalBmp = game.sysContent.bmpMedalPlatinum;
                        }
                        break;
                    } case MISSION_GRADING_MODE_PARTICIPATION: {
                        medalBmp = game.sysContent.bmpMedalPlatinum;
                    }
                    }
                    
                    if(medalBmp) {
                        drawBitmapInBox(
                            medalBmp, draw.center, draw.size, true
                        );
                    }
                };
                listBox->addChild(medalItem);
                gui.addItem(medalItem);
            }
        }
        
        //Info box item.
        infoBox = new GuiItem();
        infoBox->onDraw =
        [] (const DrawInfo & draw) {
            drawTexturedBox(
                draw.center, draw.size, game.sysContent.bmpFrameBox,
                COLOR_TRANSPARENT_WHITE
            );
        };
        gui.addItem(infoBox, "info_box");
        
        //Random button.
        ButtonGuiItem* randomButton =
            new ButtonGuiItem("", game.sysContent.fntStandard);
        randomButton->onDraw =
        [randomButton] (const DrawInfo & draw) {
            drawButton(
                draw.center, draw.size, "",
                game.sysContent.fntStandard, COLOR_WHITE,
                randomButton->selected
            );
            drawBitmapInBox(
                game.sysContent.bmpRandom,
                draw.center, draw.size - 8, true
            );
        };
        randomButton->onActivate =
        [this] (const Point&) {
            size_t areaIdx = game.rng.i(0, (int) (areaButtons.size() - 1));
            areaButtons[areaIdx]->onActivate(Point());
        };
        randomButton->onGetTooltip =
        [] () { return "Pick a random area."; };
        gui.addItem(randomButton, "random");
        
        if(areaType == AREA_TYPE_MISSION) {
            //View toggle button.
            ButtonGuiItem* viewToggleButton =
                new ButtonGuiItem(
                "Show mission specs",
                game.sysContent.fntStandard
            );
            viewToggleButton->onActivate =
            [this, viewToggleButton] (const Point&) {
                GuiItem* boxToShow = nullptr;
                GuiItem* boxToHide = nullptr;
                if(showMissionSpecs) {
                    boxToShow = infoBox;
                    boxToHide = specsBox;
                    showMissionSpecs = false;
                    viewToggleButton->text = "Show mission specs";
                } else {
                    boxToShow = specsBox;
                    boxToHide = infoBox;
                    showMissionSpecs = true;
                    viewToggleButton->text = "Show standard info";
                }
                boxToShow->visible = true;
                boxToShow->responsive = true;
                boxToHide->visible = false;
                boxToHide->responsive = false;
                animateInfoAndSpecs();
            };
            viewToggleButton->onGetTooltip =
            [] () {
                return "Toggles between basic area info and mission specs.";
            };
            gui.addItem(viewToggleButton, "view_toggle");
            
            //Specs box item.
            specsBox = new GuiItem();
            specsBox->onDraw =
            [] (const DrawInfo & draw) {
                drawTexturedBox(
                    draw.center, draw.size, game.sysContent.bmpFrameBox,
                    COLOR_TRANSPARENT_WHITE
                );
            };
            gui.addItem(specsBox, "specs_box");
            
        }
        
    } else {
    
        //No areas found text.
        TextGuiItem* noAreasText =
            new TextGuiItem(
            "No areas found! Try making your own in the area editor!",
            game.sysContent.fntStandard
        );
        gui.addItem(noAreasText, "no_areas_text");
        
    }
    
    //Tooltip text.
    TooltipGuiItem* tooltipText =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltipText, "tooltip");
}


/**
 * @brief Initializes the mission specs page GUI items.
 */
void AreaMenu::initGuiSpecsPage() {
    gui.registerCoords("specs_name",     50,  5, 96,  6);
    gui.registerCoords("goal_header",    50, 13, 96,  6);
    gui.registerCoords("goal",           50, 21, 96,  6);
    gui.registerCoords("fail_header",    50, 29, 96,  6);
    gui.registerCoords("fail_list",      47, 48, 90, 28);
    gui.registerCoords("fail_scroll",    96, 48,  4, 28);
    gui.registerCoords("grading_header", 50, 67, 96,  6);
    gui.registerCoords("grading_list",   47, 85, 90, 26);
    gui.registerCoords("grading_scroll", 96, 85,  4, 26);
    gui.readCoords(
        game.content.guiDefs.list[AREA_MENU::SPECS_GUI_FILE_NAME].
        getChildByName("positions")
    );
    
    if(!game.content.areas.list[areaType].empty()) {
    
        //Name text.
        specsNameText =
            new TextGuiItem("", game.sysContent.fntAreaName, COLOR_GOLD);
        specsBox->addChild(specsNameText);
        gui.addItem(specsNameText, "specs_name");
        
        //Goal header text.
        TextGuiItem* goalHeaderText =
            new TextGuiItem("Goal", game.sysContent.fntAreaName);
        specsBox->addChild(goalHeaderText);
        gui.addItem(goalHeaderText, "goal_header");
        
        //Goal explanation text.
        goalText =
            new TextGuiItem("", game.sysContent.fntStandard);
        specsBox->addChild(goalText);
        gui.addItem(goalText, "goal");
        
        //Fail conditions header text.
        TextGuiItem* failHeaderText =
            new TextGuiItem("Fail conditions", game.sysContent.fntAreaName);
        specsBox->addChild(failHeaderText);
        gui.addItem(failHeaderText, "fail_header");
        
        //Fail condition explanation list.
        failList = new ListGuiItem();
        specsBox->addChild(failList);
        gui.addItem(failList, "fail_list");
        
        //Fail condition explanation scrollbar.
        ScrollGuiItem* failScroll = new ScrollGuiItem();
        failScroll->listItem = failList;
        specsBox->addChild(failScroll);
        gui.addItem(failScroll, "fail_scroll");
        
        //Grading header text.
        TextGuiItem* gradingHeaderText =
            new TextGuiItem("Grading", game.sysContent.fntAreaName);
        specsBox->addChild(gradingHeaderText);
        gui.addItem(gradingHeaderText, "grading_header");
        
        //Grading explanation list.
        gradingList = new ListGuiItem();
        specsBox->addChild(gradingList);
        gui.addItem(gradingList, "grading_list");
        
        //Grading explanation scrollbar.
        ScrollGuiItem* gradingScroll = new ScrollGuiItem();
        gradingScroll->listItem = gradingList;
        specsBox->addChild(gradingScroll);
        gui.addItem(gradingScroll, "grading_scroll");
    }
}


/**
 * @brief Loads the menu.
 */
void AreaMenu::load() {
    //Mission records.
    if(areaType == AREA_TYPE_MISSION) {
        DataNode missionRecords;
        missionRecords.loadFile(
            FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true
        );
        
        for(
            size_t a = 0;
            a < game.content.areas.list[AREA_TYPE_MISSION].size(); a++
        ) {
            Area* areaPtr = game.content.areas.list[AREA_TYPE_MISSION][a];
            MissionRecord record;
            
            loadAreaMissionRecord(&missionRecords, areaPtr, record);
            
            areaRecords.push_back(record);
        }
    }
    
    //Initialize the GUIs.
    initGuiMain();
    initGuiInfoPage();
    if(
        areaType == AREA_TYPE_MISSION &&
        !game.content.areas.list[AREA_TYPE_MISSION].empty()
    ) {
        initGuiSpecsPage();
        specsBox->visible = false;
        specsBox->responsive = false;
    }
    if(firstAreaButton) {
        gui.setSelectedItem(firstAreaButton, true);
    }
    
    //Finish the menu class setup.
    guis.push_back(&gui);
    Menu::load();
}
