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

//Path to the mission briefing GUI definition file.
const string BRIEFING_GUI_FILE_NAME = "area_menu_briefing";

//Name of the main GUI definition file.
const string GUI_FILE_NAME = "area_menu";

//Path to the area info GUI definition file.
const string INFO_GUI_FILE_NAME = "area_menu_info";

//How long to animate the page swapping for.
const float PAGE_SWAP_DURATION = 0.5f;

}


/**
 * @brief Creates and adds a new bullet point to either the fail condition list
 * or the grading explanation list.
 *
 * @param list List to add to.
 * @param text Text.
 */
void AreaMenu::addNewBullet(ListGuiItem* list, const string& text) {
    size_t bulletIdx = list->children.size();
    const float BULLET_HEIGHT = 0.22f;
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
 * @brief Animates the GUI items inside of the info and briefing pages.
 */
void AreaMenu::animateInfoAndBriefing() {
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
        briefingNameText->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        objectiveText->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        for(size_t c = 0; c < noteList->children.size(); c++) {
            noteList->children[c]->startJuiceAnimation(
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
        objectiveText->text.clear();
        briefingNameText->text.clear();
        noteList->deleteAllChildren();
        gradingList->deleteAllChildren();
    }
    
    //Fill in the area's info.
    Area* areaPtr = game.content.areas.list[areaType][areaIdx];
    infoNameText->text = areaPtr->name;
    subtitleText->text =
        calculateAreaSubtitle(
            areaPtr->subtitle, areaType, areaPtr->mission.preset
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
            areaPtr->missionOld.gradingMode ==
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
            switch(areaPtr->missionOld.gradingMode) {
            case MISSION_GRADING_MODE_POINTS: {
                MISSION_MEDAL medal = areaPtr->missionOld.getScoreMedal(score);
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
    
    //Now fill in the mission briefing page.
    if(areaType == AREA_TYPE_MISSION) {
        briefingNameText->text = areaPtr->name;
        MissionData& mission = areaPtr->mission;
        objectiveText->text = mission.getBriefingObjectiveText();
        
        vector<string> noteBPStrs = mission.getNoteBulletPoints();
        for(size_t p = 0; p < noteBPStrs.size(); p++) {
            addNewBullet(noteList, noteBPStrs[p]);
        }
        
        vector<string> gradingBPStrs = mission.getGradingBulletPoints();
        for(size_t p = 0; p < gradingBPStrs.size(); p++) {
            addNewBullet(gradingList, gradingBPStrs[p]);
        }
    }
    
    animateInfoAndBriefing();
}


/**
 * @brief Initializes the area info page GUI items.
 */
void AreaMenu::initGuiInfoPage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[AREA_MENU::INFO_GUI_FILE_NAME];
    gui.registerCoords("name",         36,  6, 68,  8);
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
    gui.readDataFile(guiFile);
    
    if(!game.content.areas.list[areaType].empty()) {
    
        //Name text.
        infoNameText =
            new TextGuiItem(
            "", game.sysContent.fntAreaName, game.config.guiColors.gold
        );
        infoBox->addChild(infoNameText);
        gui.addItem(infoNameText, "name");
        
        //Subtitle text.
        subtitleText = new TextGuiItem("", game.sysContent.fntAreaName);
        infoBox->addChild(subtitleText);
        gui.addItem(subtitleText, "subtitle");
        
        //Thumbnail.
        GuiItem* thumbItem = new GuiItem();
        thumbItem->forceSquare = true;
        thumbItem->onDraw =
        [this] (const DrawInfo & draw) {
            //Align it to the top-right corner.
            Point finalCenter(
                (draw.center.x + draw.size.x / 2.0f) - draw.size.x / 2.0f,
                (draw.center.y - draw.size.y / 2.0f) + draw.size.y / 2.0f
            );
            if(curThumb) {
                drawBitmap(
                    curThumb, finalCenter, draw.size - 4.0f, 0.0f, draw.tint
                );
            }
            drawTexturedBox(
                finalCenter, draw.size, game.sysContent.bmpFrameBox,
                tintColor(COLOR_TRANSPARENT_WHITE, draw.tint)
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
                        curStamp, draw.center, draw.size, true, 0.0f, draw.tint
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
                        curMedal, draw.center, draw.size, true, 0.0f, draw.tint
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
                draw.tint, ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER,
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
                    iconSize, 0.0f, draw.tint
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
    DataNode* guiFile = &game.content.guiDefs.list[AREA_MENU::GUI_FILE_NAME];
    gui.registerCoords("back",          12,  5, 20,  6);
    gui.registerCoords("back_input",     3,  7,  4,  4);
    gui.registerCoords("list",          20, 51, 36, 82);
    gui.registerCoords("list_scroll",   40, 51,  2, 82);
    gui.registerCoords("view_toggle",   74,  5, 32,  6);
    gui.registerCoords("info_box",      70, 51, 56, 82);
    gui.registerCoords("briefing_box",  70, 51, 56, 82);
    gui.registerCoords("random",        95,  5,  6,  6);
    gui.registerCoords("tooltip",       50, 96, 96,  4);
    gui.registerCoords("no_areas_text", 50, 50, 96, 10);
    gui.readDataFile(guiFile);
    
    //Back button.
    gui.backItem =
        new ButtonGuiItem(
        "Back", game.sysContent.fntStandard, game.config.guiColors.back
    );
    gui.backItem->onActivate =
    [this] (const Point&) {
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    gui.addItem(gui.backItem, "back");
    
    //Back input icon.
    guiCreateBackInputIcon(&gui);
    
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
            areaButton->onFocused =
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
                            draw.center, draw.size, true, 0.0f, draw.tint
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
                    switch(areaPtr->missionOld.gradingMode) {
                    case MISSION_GRADING_MODE_POINTS: {
                        int score = areaRecords[a].score;
                        MISSION_MEDAL medal =
                            areaPtr->missionOld.getScoreMedal(score);
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
                            medalBmp, draw.center, draw.size, true,
                            0.0f, draw.tint
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
                tintColor(COLOR_TRANSPARENT_WHITE, draw.tint)
            );
        };
        gui.addItem(infoBox, "info_box");
        
        //Random button.
        ButtonGuiItem* randomButton =
            new ButtonGuiItem("", game.sysContent.fntStandard);
        randomButton->forceSquare = true;
        randomButton->onDraw =
        [randomButton] (const DrawInfo & draw) {
            drawButton(
                draw.center, draw.size, "",
                game.sysContent.fntStandard, COLOR_WHITE,
                randomButton->focused, 0.0f, draw.tint
            );
            drawBitmapInBox(
                game.sysContent.bmpRandom,
                draw.center, draw.size - 8, true, 0.0f, draw.tint
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
                "Show mission briefing",
                game.sysContent.fntStandard
            );
            viewToggleButton->onActivate =
            [this, viewToggleButton] (const Point&) {
                GuiItem* boxToShow = nullptr;
                GuiItem* boxToHide = nullptr;
                if(showMissionBriefing) {
                    boxToShow = infoBox;
                    boxToHide = briefingBox;
                    showMissionBriefing = false;
                    viewToggleButton->text = "Show mission briefing";
                } else {
                    boxToShow = briefingBox;
                    boxToHide = infoBox;
                    showMissionBriefing = true;
                    viewToggleButton->text = "Show standard info";
                }
                boxToShow->visible = true;
                boxToShow->responsive = true;
                boxToHide->visible = false;
                boxToHide->responsive = false;
                animateInfoAndBriefing();
            };
            viewToggleButton->onGetTooltip =
            [] () {
                return
                    "Toggles between showing basic area info"
                    "and mission briefing info.";
            };
            gui.addItem(viewToggleButton, "view_toggle");
            
            //Briefing box item.
            briefingBox = new GuiItem();
            briefingBox->onDraw =
            [] (const DrawInfo & draw) {
                drawTexturedBox(
                    draw.center, draw.size, game.sysContent.bmpFrameBox,
                    tintColor(COLOR_TRANSPARENT_WHITE, draw.tint)
                );
            };
            gui.addItem(briefingBox, "briefing_box");
            
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
 * @brief Initializes the mission briefing page GUI items.
 */
void AreaMenu::initGuiBriefingPage() {
    DataNode* guiFile =
        &game.content.guiDefs.list[AREA_MENU::BRIEFING_GUI_FILE_NAME];
    gui.registerCoords("briefing_area_name", 50,  5, 96,  6);
    gui.registerCoords("objective_header",    50, 13, 96,  6);
    gui.registerCoords("objective",           50, 21, 96,  6);
    gui.registerCoords("notes_header",    50, 29, 96,  6);
    gui.registerCoords("notes_list",      47, 48, 90, 28);
    gui.registerCoords("notes_scroll",    96, 48,  4, 28);
    gui.registerCoords("grading_header", 50, 67, 96,  6);
    gui.registerCoords("grading_list",   47, 85, 90, 26);
    gui.registerCoords("grading_scroll", 96, 85,  4, 26);
    gui.readDataFile(guiFile);
    
    if(!game.content.areas.list[areaType].empty()) {
    
        //Name text.
        briefingNameText =
            new TextGuiItem(
            "", game.sysContent.fntAreaName, game.config.guiColors.gold
        );
        briefingBox->addChild(briefingNameText);
        gui.addItem(briefingNameText, "briefing_area_name");
        
        //Objective header text.
        TextGuiItem* objectiveHeaderText =
            new TextGuiItem(
            "Objective", game.sysContent.fntAreaName,
            game.config.guiColors.smallHeader
        );
        briefingBox->addChild(objectiveHeaderText);
        gui.addItem(objectiveHeaderText, "objective_header");
        
        //Objective explanation text.
        objectiveText =
            new TextGuiItem("", game.sysContent.fntStandard);
        objectiveText->lineWrap = true;
        briefingBox->addChild(objectiveText);
        gui.addItem(objectiveText, "objective");
        
        //Notes header text.
        TextGuiItem* notesHeaderText =
            new TextGuiItem(
            "Notes", game.sysContent.fntAreaName,
            game.config.guiColors.smallHeader
        );
        briefingBox->addChild(notesHeaderText);
        gui.addItem(notesHeaderText, "notes_header");
        
        //Notes list.
        noteList = new ListGuiItem();
        briefingBox->addChild(noteList);
        gui.addItem(noteList, "notes_list");
        
        //Notes scrollbar.
        ScrollGuiItem* notesScroll = new ScrollGuiItem();
        notesScroll->listItem = noteList;
        briefingBox->addChild(notesScroll);
        gui.addItem(notesScroll, "notes_scroll");
        
        //Grading header text.
        TextGuiItem* gradingHeaderText =
            new TextGuiItem(
            "Grading", game.sysContent.fntAreaName,
            game.config.guiColors.smallHeader
        );
        briefingBox->addChild(gradingHeaderText);
        gui.addItem(gradingHeaderText, "grading_header");
        
        //Grading explanation list.
        gradingList = new ListGuiItem();
        briefingBox->addChild(gradingList);
        gui.addItem(gradingList, "grading_list");
        
        //Grading explanation scrollbar.
        ScrollGuiItem* gradingScroll = new ScrollGuiItem();
        gradingScroll->listItem = gradingList;
        briefingBox->addChild(gradingScroll);
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
            FILE_PATHS_FROM_ROOT::MISSION_RECORDS, nullptr, true, false, true
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
        initGuiBriefingPage();
        briefingBox->visible = false;
        briefingBox->responsive = false;
    }
    if(firstAreaButton) {
        gui.setFocusedItem(firstAreaButton, true);
    }
    
    //Finish the menu class setup.
    guis.push_back(&gui);
    Menu::load();
}
