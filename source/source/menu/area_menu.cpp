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
#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../core/load.h"
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
void AreaMenu::addBullet(ListGuiItem* list, const string &text) {
    size_t bullet_idx = list->children.size();
    const float BULLET_HEIGHT = 0.18f;
    const float BULLET_PADDING = 0.01f;
    const float BULLETS_OFFSET = 0.01f;
    const float bullet_center_y =
        (BULLETS_OFFSET + BULLET_HEIGHT / 2.0f) +
        ((BULLET_HEIGHT + BULLET_PADDING) * bullet_idx);
        
    BulletGuiItem* bullet =
        new BulletGuiItem(
        text, game.sysContent.fntStandard, COLOR_WHITE
    );
    bullet->ratioCenter = Point(0.50f, bullet_center_y);
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
    difficultyText->startJuiceAnimation(
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
 * @param area_idx Index of the newly-selected area.
 */
void AreaMenu::changeInfo(size_t area_idx) {
    if(area_idx == curAreaIdx) return;
    curAreaIdx = area_idx;
    
    //Start by clearing them all, for sanitization's sake.
    infoNameText->text.clear();
    subtitleText->text.clear();
    descriptionText->text.clear();
    difficultyText->text.clear();
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
    Area* area_ptr = game.content.areas.list[areaType][area_idx];
    infoNameText->text = area_ptr->name;
    subtitleText->text =
        getSubtitleOrMissionGoal(
            area_ptr->subtitle,
            areaType,
            area_ptr->mission.goal
        );
    descriptionText->text = area_ptr->description;
    if(area_ptr->difficulty == 0) {
        difficultyText->text.clear();
    } else {
        difficultyText->text =
            "Difficulty: " +
            i2s(area_ptr->difficulty) + "/5 - ";
        switch(area_ptr->difficulty) {
        case 1: {
            difficultyText->text += "Very easy";
            break;
        } case 2: {
            difficultyText->text += "Easy";
            break;
        } case 3: {
            difficultyText->text += "Medium";
            break;
        } case 4: {
            difficultyText->text += "Hard";
            break;
        } case 5: {
            difficultyText->text += "Very hard";
            break;
        }
        }
    }
    tagsText->text =
        (area_ptr->tags.empty() ? "" : "Tags: " + area_ptr->tags);
    makerText->text =
        (area_ptr->maker.empty() ? "" : "Maker: " + area_ptr->maker);
    versionText->text =
        (area_ptr->version.empty() ? "" : "Version: " + area_ptr->version);
    curThumb = area_ptr->thumbnail.get();
    if(areaType == AREA_TYPE_MISSION) {
        int score = areaRecords[area_idx].score;
        bool record_exists = !areaRecords[area_idx].date.empty();
        recordInfoText->text =
            !record_exists ?
            "(None)" :
            area_ptr->mission.gradingMode ==
            MISSION_GRADING_MODE_POINTS ?
            amountStr(score, "point") :
            "";
        curStamp =
            !record_exists ?
            nullptr :
            areaRecords[area_idx].clear ?
            game.sysContent.bmpMissionClear :
            game.sysContent.bmpMissionFail;
        if(!record_exists) {
            curMedal = nullptr;
        } else {
            switch(area_ptr->mission.gradingMode) {
            case MISSION_GRADING_MODE_POINTS: {
                if(score >= area_ptr->mission.platinumReq) {
                    curMedal = game.sysContent.bmpMedalPlatinum;
                } else if(score >= area_ptr->mission.goldReq) {
                    curMedal = game.sysContent.bmpMedalGold;
                } else if(score >= area_ptr->mission.silverReq) {
                    curMedal = game.sysContent.bmpMedalSilver;
                } else if(score >= area_ptr->mission.bronzeReq) {
                    curMedal = game.sysContent.bmpMedalBronze;
                } else {
                    curMedal = game.sysContent.bmpMedalNone;
                }
                break;
            } case MISSION_GRADING_MODE_GOAL: {
                if(areaRecords[area_idx].clear) {
                    curMedal = game.sysContent.bmpMedalPlatinum;
                }
                break;
            } case MISSION_GRADING_MODE_PARTICIPATION: {
                curMedal = game.sysContent.bmpMedalPlatinum;
                break;
            }
            }
        }
        recordDateText->text = areaRecords[area_idx].date;
    }
    
    //Now fill in the mission specs.
    if(areaType == AREA_TYPE_MISSION) {
        specsNameText->text = area_ptr->name;
        MissionData &mission = area_ptr->mission;
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
            vector<string> score_notes;
            for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
                MissionScoreCriterion* c_ptr =
                    game.missionScoreCriteria[c];
                int mult = c_ptr->getMultiplier(&mission);
                if(mult != 0) {
                    score_notes.push_back(
                        "    " + c_ptr->getName() + " x " + i2s(mult) + "."
                    );
                }
            }
            if(!score_notes.empty()) {
                addBullet(
                    gradingList,
                    "Your score is calculated like so:"
                );
                for(size_t s = 0; s < score_notes.size(); s++) {
                    addBullet(gradingList, score_notes[s]);
                }
            } else {
                addBullet(
                    gradingList,
                    "In this mission, your score will always be 0."
                );
            }
            vector<string> loss_notes;
            for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
                MissionScoreCriterion* c_ptr =
                    game.missionScoreCriteria[c];
                if(
                    hasFlag(
                        mission.pointLossData,
                        getIdxBitmask(c)
                    )
                ) {
                    loss_notes.push_back("    " + c_ptr->getName());
                }
            }
            if(!loss_notes.empty()) {
                addBullet(
                    gradingList,
                    "If you fail, you'll lose your score for:"
                );
                for(size_t l = 0; l < loss_notes.size(); l++) {
                    addBullet(gradingList, loss_notes[l]);
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
        game.content.guiDefs.list[AREA_MENU::INFO_GUI_FILE_NAME].getChildByName("positions")
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
        GuiItem* thumb_item = new GuiItem();
        thumb_item->onDraw =
        [this] (const DrawInfo & draw) {
            //Make it a square.
            Point final_size(
                std::min(draw.size.x, draw.size.y),
                std::min(draw.size.x, draw.size.y)
            );
            //Align it to the top-right corner.
            Point final_center(
                (draw.center.x + draw.size.x / 2.0f) - final_size.x / 2.0f,
                (draw.center.y - draw.size.y / 2.0f) + final_size.y / 2.0f
            );
            if(curThumb) {
                drawBitmap(curThumb, final_center, final_size - 4.0f);
            }
            drawTexturedBox(
                final_center, final_size, game.sysContent.bmpFrameBox,
                COLOR_TRANSPARENT_WHITE
            );
        };
        infoBox->addChild(thumb_item);
        gui.addItem(thumb_item, "thumbnail");
        
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
            TextGuiItem* record_label_text =
                new TextGuiItem("Record:", game.sysContent.fntStandard);
            infoBox->addChild(record_label_text);
            gui.addItem(record_label_text, "record_label");
            
            //Record info.
            recordInfoText =
                new TextGuiItem("", game.sysContent.fntStandard);
            infoBox->addChild(recordInfoText);
            gui.addItem(recordInfoText, "record_info");
            
            //Record stamp.
            GuiItem* record_stamp_item = new GuiItem();
            record_stamp_item->onDraw =
            [this] (const DrawInfo & draw) {
                if(curStamp) {
                    drawBitmapInBox(
                        curStamp, draw.center, draw.size, true
                    );
                }
            };
            infoBox->addChild(record_stamp_item);
            gui.addItem(record_stamp_item, "record_stamp");
            
            //Record medal.
            GuiItem* record_medal_item = new GuiItem();
            record_medal_item->onDraw =
            [this] (const DrawInfo & draw) {
                if(curMedal) {
                    drawBitmapInBox(
                        curMedal, draw.center, draw.size, true
                    );
                }
            };
            infoBox->addChild(record_medal_item);
            gui.addItem(record_medal_item, "record_medal");
            
            //Record date.
            recordDateText =
                new TextGuiItem(
                "", game.sysContent.fntSlim, al_map_rgb(128, 128, 128)
            );
            infoBox->addChild(recordDateText);
            gui.addItem(recordDateText, "record_date");
        }
        
        //Difficulty text.
        difficultyText =
            new TextGuiItem(
            "", game.sysContent.fntStandard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        infoBox->addChild(difficultyText);
        gui.addItem(difficultyText, "difficulty");
        
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
        game.content.guiDefs.list[AREA_MENU::GUI_FILE_NAME].getChildByName("positions")
    );
    
    //Back button.
    gui.backItem =
        new ButtonGuiItem(
        "Back", game.sysContent.fntStandard
    );
    gui.backItem->onActivate =
    [this] (const Point &) {
        leave();
    };
    gui.backItem->onGetTooltip =
    [] () { return "Return to the previous menu."; };
    gui.addItem(gui.backItem, "back");
    
    //Back input icon.
    guiAddBackInputIcon(&gui);
    
    //Header text.
    TextGuiItem* header_text =
        new TextGuiItem(
        "PICK AN AREA:",
        game.sysContent.fntAreaName, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_LEFT
    );
    gui.addItem(header_text, "header");
    
    if(!game.content.areas.list[areaType].empty()) {
    
        //Area list box.
        listBox = new ListGuiItem();
        gui.addItem(listBox, "list");
        
        //Area list scrollbar.
        ScrollGuiItem* list_scroll = new ScrollGuiItem();
        list_scroll->listItem = listBox;
        gui.addItem(list_scroll, "list_scroll");
        
        //Items for the various areas.
        for(size_t a = 0; a < game.content.areas.list[areaType].size(); a++) {
            Area* area_ptr = game.content.areas.list[areaType][a];
            const float BUTTON_HEIGHT = 0.09f;
            const float center_y = 0.045f + a * 0.10f;
            
            //Area button.
            ButtonGuiItem* area_button =
                new ButtonGuiItem(area_ptr->name, game.sysContent.fntStandard);
            area_button->ratioCenter =
                Point(
                    areaType == AREA_TYPE_MISSION ? 0.40f : 0.50f,
                    center_y
                );
            area_button->ratioSize =
                Point(
                    areaType == AREA_TYPE_MISSION ? 0.80f : 1.00f,
                    BUTTON_HEIGHT
                );
            area_button->onActivate =
            [this, area_ptr] (const Point &) {
                game.states.gameplay->pathOfAreaToLoad = area_ptr->manifest->path;
                game.fadeMgr.startFade(false, [] () {
                    game.changeState(game.states.gameplay);
                });
            };
            area_button->onSelected =
            [this, a] () { changeInfo(a); };
            area_button->onGetTooltip =
            [area_ptr] () { return "Play " + area_ptr->name + "."; };
            listBox->addChild(area_button);
            gui.addItem(area_button);
            areaButtons.push_back(area_button);
            if(!firstAreaButton) {
                firstAreaButton = area_button;
            }
            
            if(areaType == AREA_TYPE_MISSION) {
                //Stamp item.
                GuiItem* stamp_item = new GuiItem();
                stamp_item->ratioCenter =
                    Point(0.85f, center_y - (BUTTON_HEIGHT * 0.15f));
                stamp_item->ratioSize =
                    Point(0.12f, BUTTON_HEIGHT * 0.60f);
                stamp_item->onDraw =
                [this, a] (const DrawInfo & draw) {
                    if(areaRecords[a].clear) {
                        drawBitmapInBox(
                            game.sysContent.bmpMissionClear,
                            draw.center, draw.size, true
                        );
                    }
                };
                listBox->addChild(stamp_item);
                gui.addItem(stamp_item);
                
                //Medal item.
                GuiItem* medal_item = new GuiItem();
                medal_item->ratioCenter =
                    Point(0.95f, center_y + (BUTTON_HEIGHT * 0.15f));
                medal_item->ratioSize =
                    Point(0.12f, BUTTON_HEIGHT * 0.60f);
                medal_item->onDraw =
                [this, area_ptr, a] (const DrawInfo & draw) {
                    ALLEGRO_BITMAP* medal_bmp = nullptr;
                    switch(area_ptr->mission.gradingMode) {
                    case MISSION_GRADING_MODE_POINTS: {
                        int score = areaRecords[a].score;
                        if(score >= area_ptr->mission.platinumReq) {
                            medal_bmp = game.sysContent.bmpMedalPlatinum;
                        } else if(score >= area_ptr->mission.goldReq) {
                            medal_bmp = game.sysContent.bmpMedalGold;
                        } else if(score >= area_ptr->mission.silverReq) {
                            medal_bmp = game.sysContent.bmpMedalSilver;
                        } else if(score >= area_ptr->mission.bronzeReq) {
                            medal_bmp = game.sysContent.bmpMedalBronze;
                        }
                        break;
                    } case MISSION_GRADING_MODE_GOAL: {
                        if(areaRecords[a].clear) {
                            medal_bmp = game.sysContent.bmpMedalPlatinum;
                        }
                        break;
                    } case MISSION_GRADING_MODE_PARTICIPATION: {
                        medal_bmp = game.sysContent.bmpMedalPlatinum;
                    }
                    }
                    
                    if(medal_bmp) {
                        drawBitmapInBox(
                            medal_bmp, draw.center, draw.size, true
                        );
                    }
                };
                listBox->addChild(medal_item);
                gui.addItem(medal_item);
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
        ButtonGuiItem* random_button =
            new ButtonGuiItem("", game.sysContent.fntStandard);
        random_button->onDraw =
        [random_button] (const DrawInfo & draw) {
            drawButton(
                draw.center, draw.size, "", game.sysContent.fntStandard, COLOR_WHITE,
                random_button->selected
            );
            drawBitmapInBox(
                game.sysContent.bmpRandom,
                draw.center, draw.size - 8, true
            );
        };
        random_button->onActivate =
        [this] (const Point &) {
            size_t area_idx = game.rng.i(0, (int) (areaButtons.size() - 1));
            areaButtons[area_idx]->onActivate(Point());
        };
        random_button->onGetTooltip =
        [] () { return "Pick a random area."; };
        gui.addItem(random_button, "random");
        
        if(areaType == AREA_TYPE_MISSION) {
            //View toggle button.
            ButtonGuiItem* view_toggle_button =
                new ButtonGuiItem(
                "Show mission specs",
                game.sysContent.fntStandard
            );
            view_toggle_button->onActivate =
            [this, view_toggle_button] (const Point &) {
                GuiItem* box_to_show = nullptr;
                GuiItem* box_to_hide = nullptr;
                if(showMissionSpecs) {
                    box_to_show = infoBox;
                    box_to_hide = specsBox;
                    showMissionSpecs = false;
                    view_toggle_button->text = "Show mission specs";
                } else {
                    box_to_show = specsBox;
                    box_to_hide = infoBox;
                    showMissionSpecs = true;
                    view_toggle_button->text = "Show standard info";
                }
                box_to_show->visible = true;
                box_to_show->responsive = true;
                box_to_hide->visible = false;
                box_to_hide->responsive = false;
                animateInfoAndSpecs();
            };
            view_toggle_button->onGetTooltip =
            [] () {
                return "Toggles between basic area info and mission specs.";
            };
            gui.addItem(view_toggle_button, "view_toggle");
            
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
        TextGuiItem* no_areas_text =
            new TextGuiItem(
            "No areas found! Try making your own in the area editor!",
            game.sysContent.fntStandard
        );
        gui.addItem(no_areas_text, "no_areas_text");
        
    }
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltip_text, "tooltip");
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
        game.content.guiDefs.list[AREA_MENU::SPECS_GUI_FILE_NAME].getChildByName("positions")
    );
    
    if(!game.content.areas.list[areaType].empty()) {
    
        //Name text.
        specsNameText =
            new TextGuiItem("", game.sysContent.fntAreaName, COLOR_GOLD);
        specsBox->addChild(specsNameText);
        gui.addItem(specsNameText, "specs_name");
        
        //Goal header text.
        TextGuiItem* goal_header_text =
            new TextGuiItem("Goal", game.sysContent.fntAreaName);
        specsBox->addChild(goal_header_text);
        gui.addItem(goal_header_text, "goal_header");
        
        //Goal explanation text.
        goalText =
            new TextGuiItem("", game.sysContent.fntStandard);
        specsBox->addChild(goalText);
        gui.addItem(goalText, "goal");
        
        //Fail conditions header text.
        TextGuiItem* fail_header_text =
            new TextGuiItem("Fail conditions", game.sysContent.fntAreaName);
        specsBox->addChild(fail_header_text);
        gui.addItem(fail_header_text, "fail_header");
        
        //Fail condition explanation list.
        failList = new ListGuiItem();
        specsBox->addChild(failList);
        gui.addItem(failList, "fail_list");
        
        //Fail condition explanation scrollbar.
        ScrollGuiItem* fail_scroll = new ScrollGuiItem();
        fail_scroll->listItem = failList;
        specsBox->addChild(fail_scroll);
        gui.addItem(fail_scroll, "fail_scroll");
        
        //Grading header text.
        TextGuiItem* grading_header_text =
            new TextGuiItem("Grading", game.sysContent.fntAreaName);
        specsBox->addChild(grading_header_text);
        gui.addItem(grading_header_text, "grading_header");
        
        //Grading explanation list.
        gradingList = new ListGuiItem();
        specsBox->addChild(gradingList);
        gui.addItem(gradingList, "grading_list");
        
        //Grading explanation scrollbar.
        ScrollGuiItem* grading_scroll = new ScrollGuiItem();
        grading_scroll->listItem = gradingList;
        specsBox->addChild(grading_scroll);
        gui.addItem(grading_scroll, "grading_scroll");
    }
}


/**
 * @brief Loads the menu.
 */
void AreaMenu::load() {
    //Mission records.
    if(areaType == AREA_TYPE_MISSION) {
        DataNode mission_records;
        mission_records.loadFile(FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true);
        
        for(size_t a = 0; a < game.content.areas.list[AREA_TYPE_MISSION].size(); a++) {
            Area* area_ptr = game.content.areas.list[AREA_TYPE_MISSION][a];
            MissionRecord record;
            
            loadAreaMissionRecord(&mission_records, area_ptr, record);
            
            areaRecords.push_back(record);
        }
    }
    
    //Initialize the GUIs.
    initGuiMain();
    initGuiInfoPage();
    if(areaType == AREA_TYPE_MISSION && !game.content.areas.list[AREA_TYPE_MISSION].empty()) {
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
