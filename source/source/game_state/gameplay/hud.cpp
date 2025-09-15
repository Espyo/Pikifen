/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * In-game HUD classes and functions.
 */

#include <algorithm>

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"
#include "gameplay.h"

#include "hud.h"


using DrawInfo = GuiItem::DrawInfo;


namespace HUD {

//Dampen the mission goal indicator's movement by this much.
const float GOAL_INDICATOR_SMOOTHNESS_MULT = 5.5f;

//Name of the GUI definition file.
const string GUI_FILE_NAME = "gameplay";

//Maximum amount in any direction that the leader health wheel is allowed
//to offset when shaking.
const float HEALTH_SHAKE_MAX_OFFSET = 20.0f;

//How long the leader swap juice animation lasts for.
const float LEADER_SWAP_JUICE_DURATION = 0.7f;

//How long the medal "Got it!" text juice animation lasts for.
const float MEDAL_GOT_IT_JUICE_DURATION = 1.3f;

//Standard mission score medal icon scale, for the obtained medal.
const float MEDAL_ICON_SCALE_CUR = 2.0f;

//Multiply time by this much to get the right scale animation amount.
const float MEDAL_ICON_SCALE_MULT = 0.3f;

//Standard mission score medal icon scale, for the next medal.
const float MEDAL_ICON_SCALE_NEXT = 1.5f;

//Multiply time by this much to get the right scale animation speed.
const float MEDAL_ICON_SCALE_TIME_MULT = 4.0f;

//Dampen the mission score indicator's movement by this much.
const float SCORE_INDICATOR_SMOOTHNESS_MULT = 5.5f;

//Ratio of the score gamut to show around the mission score ruler flapper.
const float SCORE_RULER_RATIO_RANGE = 0.20f;

//How long the spray swap juice animation lasts for.
const float SPRAY_SWAP_JUICE_DURATION = 0.7f;

//How long the standby swap juice animation lasts for.
const float STANDBY_SWAP_JUICE_DURATION = 0.5f;

//The Sun Meter's sun spins these many radians per second.
const float SUN_METER_SUN_SPIN_SPEED = 0.5f;

//Speed at which previously-unnecessary items fade in, in alpha per second.
const float UNNECESSARY_ITEMS_FADE_IN_SPEED = 2.5f;

//Delay before unnecessary items start fading out.
const float UNNECESSARY_ITEMS_FADE_OUT_DELAY = 2.5f;

//Speed at which unnecessary items fade out, in alpha per second.
const float UNNECESSARY_ITEMS_FADE_OUT_SPEED = 0.5f;

}


/**
 * @brief Constructs a new HUD struct object.
 */
Hud::Hud() :
    leaderIconMgr(&gui),
    leaderHealthMgr(&gui),
    standbyIconMgr(&gui),
    sprayIconMgr(&gui) {
    
    DataNode* hudFileNode = &game.content.guiDefs.list[HUD::GUI_FILE_NAME];
    
    gui.registerCoords("time",                          0,    0,  0,  0);
    gui.registerCoords("day_bubble",                    0,    0,  0,  0);
    gui.registerCoords("day_number",                    0,    0,  0,  0);
    gui.registerCoords("leader_1_icon",                 7,   90,  8, 10);
    gui.registerCoords("leader_2_icon",                 6,   80,  5,  9);
    gui.registerCoords("leader_3_icon",                 6, 71.5,  5,  7);
    gui.registerCoords("leader_1_health",              16,   90,  8, 10);
    gui.registerCoords("leader_2_health",              12,   80,  5,  9);
    gui.registerCoords("leader_3_health",              12, 71.5,  5,  7);
    gui.registerCoords("leader_next_input",            4,   83,  3,  3);
    gui.registerCoords("standby_icon",                 50,   91,  8, 10);
    gui.registerCoords("standby_amount",               50,   97,  8,  4);
    gui.registerCoords("standby_bubble",                0,    0,  0,  0);
    gui.registerCoords("standby_maturity_icon",        54,   88,  4,  8);
    gui.registerCoords("standby_next_icon",            58,   93,  6,  8);
    gui.registerCoords("standby_next_input",           60,   96,  3,  3);
    gui.registerCoords("standby_prev_icon",            42,   93,  6,  8);
    gui.registerCoords("standby_prev_input",           40,   96,  3,  3);
    gui.registerCoords("group_amount",                 73,   91, 15, 14);
    gui.registerCoords("group_bubble",                 73,   91, 15, 14);
    gui.registerCoords("field_amount",                 91,   91, 15, 14);
    gui.registerCoords("field_bubble",                 91,   91, 15, 14);
    gui.registerCoords("total_amount",                  0,    0,  0,  0);
    gui.registerCoords("total_bubble",                  0,    0,  0,  0);
    gui.registerCoords("counters_x",                    0,    0,  0,  0);
    gui.registerCoords("counters_slash_1",             82,   91,  4,  8);
    gui.registerCoords("counters_slash_2",              0,    0,  0,  0);
    gui.registerCoords("counters_slash_3",              0,    0,  0,  0);
    gui.registerCoords("spray_1_icon",                  6,   36,  4,  7);
    gui.registerCoords("spray_1_amount",               13,   37, 10,  5);
    gui.registerCoords("spray_1_input",                 4,   39,  3,  3);
    gui.registerCoords("spray_2_icon",                  6,   52,  4,  7);
    gui.registerCoords("spray_2_amount",               13,   53, 10,  5);
    gui.registerCoords("spray_2_input",                 4,   55,  3,  3);
    gui.registerCoords("spray_prev_icon",               6,   48,  3,  5);
    gui.registerCoords("spray_prev_input",              4,   51,  4,  4);
    gui.registerCoords("spray_next_icon",              12,   48,  3,  5);
    gui.registerCoords("spray_next_input",             14,   51,  4,  4);
    gui.registerCoords("mission_goal_bubble",          18,    8, 32, 12);
    gui.registerCoords("mission_goal_cur_label",      9.5, 11.5, 13,  3);
    gui.registerCoords("mission_goal_cur",            9.5,  6.5, 13,  7);
    gui.registerCoords("mission_goal_req_label",     26.5, 11.5, 13,  3);
    gui.registerCoords("mission_goal_req",           26.5,  6.5, 13,  7);
    gui.registerCoords("mission_goal_slash",           18,  6.5,  4,  7);
    gui.registerCoords("mission_goal_name",            18,    8, 30, 10);
    gui.registerCoords("mission_score_bubble",         18,   20, 32, 10);
    gui.registerCoords("mission_score_score_label",   7.5,   22,  9,  4);
    gui.registerCoords("mission_score_points",         18, 21.5, 10,  5);
    gui.registerCoords("mission_score_points_label", 28.5,   22,  9,  4);
    gui.registerCoords("mission_score_ruler",          18,   17, 30,  2);
    gui.registerCoords("mission_fail_1_bubble",        82,    8, 32, 12);
    gui.registerCoords("mission_fail_1_cur_label",   73.5, 11.5, 13,  3);
    gui.registerCoords("mission_fail_1_cur",         73.5,  6.5, 13,  7);
    gui.registerCoords("mission_fail_1_req_label",   90.5, 11.5, 13,  3);
    gui.registerCoords("mission_fail_1_req",         90.5,  6.5, 13,  7);
    gui.registerCoords("mission_fail_1_slash",         82,  6.5,  4,  7);
    gui.registerCoords("mission_fail_1_name",          82,    8, 30, 10);
    gui.registerCoords("mission_fail_2_bubble",        82,   20, 32, 10);
    gui.registerCoords("mission_fail_2_cur_label",   73.5, 22.5, 13,  3);
    gui.registerCoords("mission_fail_2_cur",         73.5, 18.5, 13,  5);
    gui.registerCoords("mission_fail_2_req_label",   90.5, 22.5, 13,  3);
    gui.registerCoords("mission_fail_2_req",         90.5, 18.5, 13,  5);
    gui.registerCoords("mission_fail_2_slash",         82, 18.5,  4,  5);
    gui.registerCoords("mission_fail_2_name",          82,   20, 30,  8);
    gui.readCoords(hudFileNode->getChildByName("positions"));
    
    //Leader health and icons.
    for(size_t l = 0; l < 3; l++) {
    
        //Icon.
        GuiItem* leaderIcon = new GuiItem();
        leaderIcon->onDraw =
        [this, l] (const DrawInfo & draw) {
            LeaderIconBubble icon;
            DrawInfo finalDraw;
            this->leaderIconMgr.getDrawingInfo(
                l, &icon, &finalDraw
            );
            
            if(!icon.bmp) return;
            
            al_draw_filled_circle(
                finalDraw.center.x, finalDraw.center.y,
                std::min(finalDraw.size.x, finalDraw.size.y) / 2.0f,
                changeAlpha(
                    icon.color,
                    128
                )
            );
            drawBitmapInBox(
                icon.bmp,
                finalDraw.center, finalDraw.size, true
            );
            drawBitmapInBox(
                bmpBubble,
                finalDraw.center, finalDraw.size, true
            );
        };
        gui.addItem(leaderIcon, "leader_" + i2s(l + 1) + "_icon");
        leaderIconMgr.registerBubble(l, leaderIcon);
        
        
        //Health wheel.
        GuiItem* leaderHealth = new GuiItem();
        leaderHealth->onDraw =
        [this, l] (const DrawInfo & draw) {
            LeaderHealthBubble health;
            DrawInfo finalDraw;
            this->leaderHealthMgr.getDrawingInfo(
                l, &health, &finalDraw
            );
            finalDraw.center += health.offset * HUD::HEALTH_SHAKE_MAX_OFFSET;
            
            if(health.ratio <= 0.0f) return;
            
            drawHealth(
                finalDraw.center,
                health.ratio,
                1.0f,
                std::min(finalDraw.size.x, finalDraw.size.y) * 0.47f,
                true
            );
            drawBitmapInBox(
                bmpHardBubble,
                finalDraw.center,
                finalDraw.size,
                true, 0.0f,
                interpolateColor(
                    health.redness, 0.0f, 1.0f,
                    COLOR_WHITE, al_map_rgb(255, 0, 0)
                )
            );
            
            if(health.cautionTimer > 0.0f) {
                float animRatio =
                    health.cautionTimer / LEADER::HEALTH_CAUTION_RING_DURATION;
                float cautionRingScale =
                    interpolateNumber(
                        health.cautionTimer,
                        0.0f, LEADER::HEALTH_CAUTION_RING_DURATION,
                        1.2f, 1.8f
                    );
                    
                KeyframeInterpolator<unsigned char> alphaKeyframes(0);
                alphaKeyframes.add(0.2f, 255);
                alphaKeyframes.add(0.3f, 255);
                alphaKeyframes.add(0.8f, 0);
                alphaKeyframes.add(1.0f, 0);
                
                float cautionRingSize =
                    std::min(finalDraw.size.x, finalDraw.size.y) *
                    cautionRingScale;
                    
                drawBitmap(
                    game.sysContent.bmpLowHealthRing,
                    finalDraw.center,
                    Point(cautionRingSize),
                    0.0f,
                    al_map_rgba(255, 255, 255, alphaKeyframes.get(animRatio))
                );
            }
        };
        gui.addItem(leaderHealth, "leader_" + i2s(l + 1) + "_health");
        leaderHealthMgr.registerBubble(l, leaderHealth);
        
    }
    
    
    //Next leader input.
    GuiItem* leaderNextInput = new GuiItem();
    leaderNextInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(game.states.gameplay->availableLeaders.size() < 2) return;
        const Inpution::InputSource& s =
            game.controls.findBind(PLAYER_ACTION_TYPE_NEXT_LEADER).
            inputSource;
        if(s.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size
        );
    };
    gui.addItem(leaderNextInput, "leader_next_input");
    
    
    //Sun Meter.
    GuiItem* sunMeter = new GuiItem();
    sunMeter->onDraw =
    [this, sunMeter] (const DrawInfo & draw) {
        unsigned char nHours =
            (
                game.config.misc.dayMinutesEnd -
                game.config.misc.dayMinutesStart
            ) / 60.0f;
        float dayLength =
            game.config.misc.dayMinutesEnd - game.config.misc.dayMinutesStart;
        float dayPassedRatio =
            (
                game.states.gameplay->dayMinutes -
                game.config.misc.dayMinutesStart
            ) /
            (float) (dayLength);
        float sunRadius = draw.size.y / 2.0;
        float firstDotX = (draw.center.x - draw.size.x / 2.0) + sunRadius;
        float lastDotX = (draw.center.x + draw.size.x / 2.0) - sunRadius;
        float dotsY = draw.center.y;
        //Width, from the center of the first dot to the center of the last.
        float dotsSpan = lastDotX - firstDotX;
        float dotInterval = dotsSpan / (float) nHours;
        float sunMeterSunAngle =
            game.states.gameplay->areaTimePassed *
            HUD::SUN_METER_SUN_SPIN_SPEED;
            
        //Larger bubbles at the start, middle and end of the meter.
        al_hold_bitmap_drawing(true);
        drawBitmap(
            bmpHardBubble,
            Point(firstDotX + dotsSpan * 0.0, dotsY),
            Point(sunRadius * 0.9)
        );
        drawBitmap(
            bmpHardBubble,
            Point(firstDotX + dotsSpan * 0.5, dotsY),
            Point(sunRadius * 0.9)
        );
        drawBitmap(
            bmpHardBubble,
            Point(firstDotX + dotsSpan * 1.0, dotsY),
            Point(sunRadius * 0.9)
        );
        
        for(unsigned char h = 0; h < nHours + 1; h++) {
            drawBitmap(
                bmpHardBubble,
                Point(firstDotX + h * dotInterval, dotsY),
                Point(sunRadius * 0.6)
            );
        }
        al_hold_bitmap_drawing(false);
        
        Point sunSize =
            Point(sunRadius * 1.5) +
            sunMeter->getJuiceValue();
        //Static sun.
        drawBitmap(
            bmpSun,
            Point(firstDotX + dayPassedRatio * dotsSpan, dotsY),
            sunSize
        );
        //Spinning sun.
        drawBitmap(
            bmpSun,
            Point(firstDotX + dayPassedRatio * dotsSpan, dotsY),
            sunSize,
            sunMeterSunAngle
        );
        //Bubble in front the sun.
        drawBitmap(
            bmpHardBubble,
            Point(firstDotX + dayPassedRatio * dotsSpan, dotsY),
            sunSize,
            0.0f,
            al_map_rgb(255, 192, 128)
        );
    };
    sunMeter->onTick =
    [this, sunMeter] (float deltaT) {
        float dayLength =
            game.config.misc.dayMinutesEnd - game.config.misc.dayMinutesStart;
        float preTickDayMinutes =
            game.states.gameplay->dayMinutes -
            game.curAreaData->dayTimeSpeed * deltaT / 60.0f;
        float postTickDayMinutes =
            game.states.gameplay->dayMinutes;
        const float checkpoints[3] = {0.25f, 0.50f, 0.75f};
        for(unsigned char c = 0; c < 3; c++) {
            float checkpoint =
                game.config.misc.dayMinutesStart + dayLength * checkpoints[c];
            if(
                preTickDayMinutes < checkpoint &&
                postTickDayMinutes >= checkpoint
            ) {
                sunMeter->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_ICON
                );
                break;
            }
        }
    };
    gui.addItem(sunMeter, "time");
    
    
    //Day number bubble.
    GuiItem* dayBubble = new GuiItem();
    dayBubble->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmapInBox(bmpDayBubble, draw.center, draw.size, true);
    };
    gui.addItem(dayBubble, "day_bubble");
    
    
    //Day number text.
    GuiItem* dayNr = new GuiItem();
    dayNr->onDraw =
    [this] (const DrawInfo & draw) {
        drawText(
            i2s(game.states.gameplay->day),
            game.sysContent.fntCounter, draw.center,
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f)
        );
    };
    gui.addItem(dayNr, "day_number");
    
    
    //Standby group member icon.
    GuiItem* standbyIcon = new GuiItem();
    standbyIcon->onDraw =
    [this] (const DrawInfo & draw) {
        this->drawStandbyIcon(BUBBLE_RELATION_CURRENT);
    };
    gui.addItem(standbyIcon, "standby_icon");
    standbyIconMgr.registerBubble(BUBBLE_RELATION_CURRENT, standbyIcon);
    
    
    //Next standby subgroup icon.
    GuiItem* standbyNextIcon = new GuiItem();
    standbyNextIcon->onDraw =
    [this] (const DrawInfo & draw) {
        this->drawStandbyIcon(BUBBLE_RELATION_NEXT);
    };
    gui.addItem(standbyNextIcon, "standby_next_icon");
    standbyIconMgr.registerBubble(BUBBLE_RELATION_NEXT, standbyNextIcon);
    
    
    //Next standby subgroup input.
    GuiItem* standbyNextInput = new GuiItem();
    standbyNextInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!player->leaderPtr) return;
        SubgroupType* nextType;
        player->leaderPtr->group->getNextStandbyType(
            false, &nextType
        );
        if(
            nextType ==
            player->leaderPtr->group->curStandbyType
        ) {
            return;
        }
        const Inpution::InputSource& s =
            game.controls.findBind(PLAYER_ACTION_TYPE_NEXT_TYPE).
            inputSource;
        if(s.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            this->standbyItemsOpacity * 255
        );
    };
    gui.addItem(standbyNextInput, "standby_next_input");
    
    
    //Previous standby subgroup icon.
    GuiItem* standbyPrevIcon = new GuiItem();
    standbyPrevIcon->onDraw =
    [this] (const DrawInfo & draw) {
        this->drawStandbyIcon(BUBBLE_RELATION_PREVIOUS);
    };
    gui.addItem(standbyPrevIcon, "standby_prev_icon");
    standbyIconMgr.registerBubble(BUBBLE_RELATION_PREVIOUS, standbyPrevIcon);
    
    
    //Previous standby subgroup input.
    GuiItem* standbyPrevInput = new GuiItem();
    standbyPrevInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!player->leaderPtr) return;
        SubgroupType* prevType;
        player->leaderPtr->group->getNextStandbyType(
            true, &prevType
        );
        SubgroupType* nextType;
        player->leaderPtr->group->getNextStandbyType(
            false, &nextType
        );
        if(
            prevType ==
            player->leaderPtr->group->curStandbyType ||
            prevType == nextType
        ) {
            return;
        }
        const Inpution::InputSource s =
            game.controls.findBind(PLAYER_ACTION_TYPE_PREV_TYPE).
            inputSource;
        if(s.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            this->standbyItemsOpacity * 255
        );
    };
    gui.addItem(standbyPrevInput, "standby_prev_input");
    
    
    //Standby group member maturity.
    GuiItem* standbyMaturityIcon = new GuiItem();
    standbyMaturityIcon->onDraw =
    [this, standbyMaturityIcon] (const DrawInfo & draw) {
        //Standby group member preparations.
        Leader* lPtr = player->leaderPtr;
        if(!lPtr || !lPtr->group) return;
        
        ALLEGRO_BITMAP* standbyMatBmp = nullptr;
        Mob* closest =
            player->closestGroupMember[BUBBLE_RELATION_CURRENT];
            
        if(lPtr->group->curStandbyType && closest) {
            SUBGROUP_TYPE_CATEGORY c =
                lPtr->group->curStandbyType->getCategory();
                
            switch(c) {
            case SUBGROUP_TYPE_CATEGORY_PIKMIN: {
                Pikmin* pPtr =
                    dynamic_cast<Pikmin*>(closest);
                standbyMatBmp =
                    pPtr->pikType->bmpMaturityIcon[pPtr->maturity];
                break;
                
            } default: {
                break;
                
            }
            }
        }
        
        ALLEGRO_COLOR color =
            mapAlpha(this->standbyItemsOpacity * 255);
            
        if(standbyMatBmp) {
            drawBitmapInBox(
                standbyMatBmp, draw.center,
                (draw.size * 0.8) + standbyMaturityIcon->getJuiceValue(),
                true,
                0.0f, color
            );
            drawBitmapInBox(
                bmpBubble, draw.center,
                draw.size + standbyMaturityIcon->getJuiceValue(),
                true, 0.0f, color
            );
        }
        
        if(
            lPtr->group->curStandbyType != prevStandbyType ||
            standbyMatBmp != prevMaturityIcon
        ) {
            standbyMaturityIcon->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_ICON
            );
            prevStandbyType = lPtr->group->curStandbyType;
            prevMaturityIcon = standbyMatBmp;
        }
    };
    gui.addItem(standbyMaturityIcon, "standby_maturity_icon");
    
    
    //Standby subgroup member amount bubble.
    GuiItem* standbyBubble = new GuiItem();
    standbyBubble->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            bmpCounterBubbleStandby,
            draw.center, draw.size,
            0.0f,
            mapAlpha(this->standbyItemsOpacity * 255)
        );
    };
    gui.addItem(standbyBubble, "standby_bubble");
    
    
    //Standby subgroup member amount.
    standbyAmount = new GuiItem();
    standbyAmount->onDraw =
    [this] (const DrawInfo & draw) {
        size_t nStandbyPikmin = 0;
        Leader* lPtr = player->leaderPtr;
        
        if(lPtr && lPtr->group->curStandbyType) {
            for(size_t m = 0; m < lPtr->group->members.size(); m++) {
                Mob* mPtr = lPtr->group->members[m];
                if(mPtr->subgroupTypePtr == lPtr->group->curStandbyType) {
                    nStandbyPikmin++;
                }
            }
        }
        
        if(nStandbyPikmin != standbyCountNr) {
            standbyAmount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            standbyCountNr = nStandbyPikmin;
        }
        
        drawText(
            i2s(nStandbyPikmin), game.sysContent.fntCounter,
            draw.center, draw.size,
            mapAlpha(this->standbyItemsOpacity * 255),
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + standbyAmount->getJuiceValue())
        );
    };
    gui.addItem(standbyAmount, "standby_amount");
    
    
    //Group Pikmin amount bubble.
    GuiItem* groupBubble = new GuiItem();
    groupBubble->onDraw =
    [this] (const DrawInfo & draw) {
        if(!player->leaderPtr) return;
        drawBitmap(
            bmpCounterBubbleGroup,
            draw.center, draw.size
        );
    };
    gui.addItem(groupBubble, "group_bubble");
    
    
    //Group Pikmin amount.
    groupAmount = new GuiItem();
    groupAmount->onDraw =
    [this] (const DrawInfo & draw) {
        if(!player->leaderPtr) return;
        size_t curAmount = game.states.gameplay->getAmountOfGroupPikmin(player);
        
        if(curAmount != groupCountNr) {
            groupAmount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            groupCountNr = curAmount;
        }
        
        drawText(
            i2s(curAmount), game.sysContent.fntCounter,
            draw.center,
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f), COLOR_WHITE,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + groupAmount->getJuiceValue())
        );
    };
    gui.addItem(groupAmount, "group_amount");
    
    
    //Field Pikmin amount bubble.
    GuiItem* fieldBubble = new GuiItem();
    fieldBubble->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            bmpCounterBubbleField,
            draw.center, draw.size
        );
    };
    gui.addItem(fieldBubble, "field_bubble");
    
    
    //Field Pikmin amount.
    fieldAmount = new GuiItem();
    fieldAmount->onDraw =
    [this] (const DrawInfo & draw) {
        size_t curAmount = game.states.gameplay->getAmountOfFieldPikmin();
        
        if(curAmount != fieldCountNr) {
            fieldAmount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            fieldCountNr = curAmount;
        }
        
        drawText(
            i2s(curAmount), game.sysContent.fntCounter,
            draw.center,
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f), COLOR_WHITE,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + fieldAmount->getJuiceValue())
        );
    };
    gui.addItem(fieldAmount, "field_amount");
    
    
    //Total Pikmin amount bubble.
    GuiItem* totalBubble = new GuiItem();
    totalBubble->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            bmpCounterBubbleTotal,
            draw.center, draw.size
        );
    };
    gui.addItem(totalBubble, "total_bubble");
    
    
    //Total Pikmin amount.
    totalAmount = new GuiItem();
    totalAmount->onDraw =
    [this] (const DrawInfo & draw) {
        size_t curAmount = game.states.gameplay->getAmountOfTotalPikmin();
        
        if(curAmount != totalCountNr) {
            totalAmount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            totalCountNr = curAmount;
        }
        
        drawText(
            i2s(totalCountNr), game.sysContent.fntCounter,
            draw.center,
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f), COLOR_WHITE,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + totalAmount->getJuiceValue())
        );
    };
    gui.addItem(totalAmount, "total_amount");
    
    
    //Pikmin counter "x".
    GuiItem* countersX = new GuiItem();
    countersX->onDraw =
    [this] (const DrawInfo & draw) {
        drawText(
            "x", game.sysContent.fntCounter, draw.center, draw.size,
            mapAlpha(this->standbyItemsOpacity * 255)
        );
    };
    gui.addItem(countersX, "counters_x");
    
    
    //Pikmin counter slashes.
    for(size_t s = 0; s < 3; s++) {
        GuiItem* counterSlash = new GuiItem();
        counterSlash->onDraw =
        [this] (const DrawInfo & draw) {
            if(!player->leaderPtr) return;
            drawText(
                "/", game.sysContent.fntCounter, draw.center, draw.size
            );
        };
        gui.addItem(counterSlash, "counters_slash_" + i2s(s + 1));
    }
    
    
    //Spray 1 icon.
    GuiItem* spray1Icon = new GuiItem();
    spray1Icon->onDraw =
    [this] (const DrawInfo & draw) {
        drawSprayIcon(BUBBLE_RELATION_CURRENT);
    };
    gui.addItem(spray1Icon, "spray_1_icon");
    sprayIconMgr.registerBubble(BUBBLE_RELATION_CURRENT, spray1Icon);
    
    
    //Spray 1 amount.
    spray1Amount = new GuiItem();
    spray1Amount->onDraw =
    [this] (const DrawInfo & draw) {
        if(!player->leaderPtr) return;
        
        size_t topSprayIdx = INVALID;
        if(game.content.sprayTypes.list.size() > 2) {
            topSprayIdx = player->selectedSpray;
        } else if(
            !game.content.sprayTypes.list.empty() &&
            game.content.sprayTypes.list.size() <= 2
        ) {
            topSprayIdx = 0;
        }
        if(topSprayIdx == INVALID) return;
        
        size_t curAmount = player->team->sprayStats[topSprayIdx].nrSprays;
        
        if(curAmount != spray1CountNr) {
            spray1Amount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            spray1CountNr = curAmount;
        }
        
        drawText(
            "x" +
            i2s(curAmount), game.sysContent.fntCounter,
            Point(draw.center.x - draw.size.x / 2.0, draw.center.y), draw.size,
            mapAlpha(this->sprayItemsOpacity * 255),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + spray1Amount->getJuiceValue())
        );
    };
    gui.addItem(spray1Amount, "spray_1_amount");
    
    
    //Spray 1 input.
    GuiItem* spray1Input = new GuiItem();
    spray1Input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!player->leaderPtr) return;
        
        size_t topSprayIdx = INVALID;
        if(game.content.sprayTypes.list.size() > 2) {
            topSprayIdx = player->selectedSpray;
        } else if(
            !game.content.sprayTypes.list.empty() &&
            game.content.sprayTypes.list.size() <= 2
        ) {
            topSprayIdx = 0;
        }
        if(topSprayIdx == INVALID) return;
        if(player->team->sprayStats[topSprayIdx].nrSprays == 0) {
            return;
        }
        
        Inpution::InputSource s;
        if(
            game.content.sprayTypes.list.size() > 2
        ) {
            s =
                game.controls.findBind(PLAYER_ACTION_TYPE_USE_SPRAY).
                inputSource;
        } else if(
            !game.content.sprayTypes.list.empty() &&
            game.content.sprayTypes.list.size() <= 2
        ) {
            s =
                game.controls.findBind(PLAYER_ACTION_TYPE_USE_SPRAY_1).
                inputSource;
        }
        if(s.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
        
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            this->sprayItemsOpacity * 255
        );
    };
    gui.addItem(spray1Input, "spray_1_input");
    
    
    //Spray 2 icon.
    GuiItem* spray2Icon = new GuiItem();
    spray2Icon->onDraw =
    [this] (const DrawInfo & draw) {
        if(!player->leaderPtr) return;
        
        size_t bottomSprayIdx = INVALID;
        if(game.content.sprayTypes.list.size() == 2) {
            bottomSprayIdx = 1;
        }
        if(bottomSprayIdx == INVALID) return;
        
        drawBitmapInBox(
            game.config.misc.sprayOrder[bottomSprayIdx]->bmpSpray,
            draw.center, draw.size, true,
            0.0f,
            mapAlpha(this->sprayItemsOpacity * 255)
        );
    };
    gui.addItem(spray2Icon, "spray_2_icon");
    
    
    //Spray 2 amount.
    spray2Amount = new GuiItem();
    spray2Amount->onDraw =
    [this] (const DrawInfo & draw) {
        if(!player->leaderPtr) return;
        
        size_t bottomSprayIdx = INVALID;
        if(game.content.sprayTypes.list.size() == 2) {
            bottomSprayIdx = 1;
        }
        if(bottomSprayIdx == INVALID) return;
        
        size_t curAmount = player->team->sprayStats[bottomSprayIdx].nrSprays;
        
        if(curAmount != spray2CountNr) {
            spray2Amount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            spray2CountNr = curAmount;
        }
        
        drawText(
            "x" +
            i2s(curAmount), game.sysContent.fntCounter,
            Point(draw.center.x - draw.size.x / 2.0, draw.center.y), draw.size,
            mapAlpha(this->sprayItemsOpacity * 255),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + spray2Amount->getJuiceValue())
        );
    };
    gui.addItem(spray2Amount, "spray_2_amount");
    
    
    //Spray 2 input.
    GuiItem* spray2Input = new GuiItem();
    spray2Input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!player->leaderPtr) return;
        
        size_t bottomSprayIdx = INVALID;
        if(game.content.sprayTypes.list.size() == 2) {
            bottomSprayIdx = 1;
        }
        if(bottomSprayIdx == INVALID) return;
        if(player->team->sprayStats[bottomSprayIdx].nrSprays == 0) {
            return;
        }
        
        Inpution::InputSource s;
        if(game.content.sprayTypes.list.size() == 2) {
            s =
                game.controls.findBind(PLAYER_ACTION_TYPE_USE_SPRAY_2).
                inputSource;
        }
        if(s.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
        
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            this->sprayItemsOpacity * 255
        );
    };
    gui.addItem(spray2Input, "spray_2_input");
    
    
    //Previous spray icon.
    GuiItem* prevSprayIcon = new GuiItem();
    prevSprayIcon->onDraw =
    [this] (const DrawInfo & draw) {
        drawSprayIcon(BUBBLE_RELATION_PREVIOUS);
    };
    gui.addItem(prevSprayIcon, "spray_prev_icon");
    sprayIconMgr.registerBubble(BUBBLE_RELATION_PREVIOUS, prevSprayIcon);
    
    
    //Previous spray input.
    GuiItem* prevSprayInput = new GuiItem();
    prevSprayInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!player->leaderPtr) return;
        
        size_t prevSprayIdx = INVALID;
        if(game.content.sprayTypes.list.size() >= 3) {
            prevSprayIdx =
                sumAndWrap(
                    (int) player->selectedSpray,
                    -1,
                    (int) game.content.sprayTypes.list.size()
                );
        }
        if(prevSprayIdx == INVALID) return;
        
        Inpution::InputSource s;
        if(game.content.sprayTypes.list.size() >= 3) {
            s =
                game.controls.findBind(PLAYER_ACTION_TYPE_PREV_SPRAY).
                inputSource;
        }
        if(s.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
        
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            this->sprayItemsOpacity * 255
        );
    };
    gui.addItem(prevSprayInput, "spray_prev_input");
    
    
    //Next spray icon.
    GuiItem* nextSprayIcon = new GuiItem();
    nextSprayIcon->onDraw =
    [this] (const DrawInfo & draw) {
        drawSprayIcon(BUBBLE_RELATION_NEXT);
    };
    gui.addItem(nextSprayIcon, "spray_next_icon");
    sprayIconMgr.registerBubble(BUBBLE_RELATION_NEXT, nextSprayIcon);
    
    
    //Next spray input.
    GuiItem* nextSprayInput = new GuiItem();
    nextSprayInput->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!player->leaderPtr) return;
        
        size_t nextSprayIdx = INVALID;
        if(game.content.sprayTypes.list.size() >= 3) {
            nextSprayIdx =
                sumAndWrap(
                    (int) player->selectedSpray,
                    1,
                    (int) game.content.sprayTypes.list.size()
                );
        }
        if(nextSprayIdx == INVALID) return;
        
        Inpution::InputSource s;
        if(game.content.sprayTypes.list.size() >= 3) {
            s =
                game.controls.findBind(PLAYER_ACTION_TYPE_NEXT_SPRAY).
                inputSource;
        }
        if(s.type == Inpution::INPUT_SOURCE_TYPE_NONE) return;
        
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            this->sprayItemsOpacity * 255
        );
    };
    gui.addItem(nextSprayInput, "spray_next_input");
    
    
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
    
        //Mission goal bubble.
        GuiItem* missionGoalBubble = new GuiItem();
        missionGoalBubble->onDraw =
        [this] (const DrawInfo & draw) {
            int cx = 0;
            int cy = 0;
            int cw = 0;
            int ch = 0;
            al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
            setCombinedClippingRectangles(
                cx, cy, cw, ch,
                draw.center.x - draw.size.x / 2.0f,
                draw.center.y - draw.size.y / 2.0f,
                draw.size.x * game.states.gameplay->goalIndicatorRatio + 1,
                draw.size.y
            );
            drawFilledRoundedRectangle(
                draw.center, draw.size, 20.0f, al_map_rgba(86, 149, 50, 160)
            );
            setCombinedClippingRectangles(
                cx, cy, cw, ch,
                draw.center.x - draw.size.x / 2.0f +
                draw.size.x * game.states.gameplay->goalIndicatorRatio,
                draw.center.y - draw.size.y / 2.0f,
                draw.size.x * (1 - game.states.gameplay->goalIndicatorRatio),
                draw.size.y
            );
            drawFilledRoundedRectangle(
                draw.center, draw.size, 20.0f, al_map_rgba(34, 102, 102, 80)
            );
            al_set_clipping_rectangle(cx, cy, cw, ch);
            drawTexturedBox(
                draw.center, draw.size, game.sysContent.bmpBubbleBox,
                al_map_rgba(255, 255, 255, 200)
            );
        };
        gui.addItem(missionGoalBubble, "mission_goal_bubble");
        
        
        string goalCurLabelText =
            game.missionGoals[game.curAreaData->mission.goal]->
            getHudLabel();
            
        if(!goalCurLabelText.empty()) {
            //Mission goal current label.
            GuiItem* missionGoalCurLabel = new GuiItem();
            missionGoalCurLabel->onDraw =
                [this, goalCurLabelText]
            (const DrawInfo & draw) {
                drawText(
                    goalCurLabelText, game.sysContent.fntStandard,
                    draw.center, draw.size, al_map_rgba(255, 255, 255, 128)
                );
            };
            gui.addItem(missionGoalCurLabel, "mission_goal_cur_label");
            
            
            //Mission goal current.
            GuiItem* missionGoalCur = new GuiItem();
            missionGoalCur->onDraw =
                [this, missionGoalCur]
            (const DrawInfo & draw) {
                int value =
                    game.missionGoals[game.curAreaData->mission.goal]->
                    getCurAmount(game.states.gameplay);
                string text;
                if(
                    game.curAreaData->mission.goal ==
                    MISSION_GOAL_TIMED_SURVIVAL
                ) {
                    text = timeToStr2(value, ":", "");
                } else {
                    text = i2s(value);
                }
                float juicyGrowAmount =
                    missionGoalCur->getJuiceValue();
                drawText(
                    text, game.sysContent.fntCounter, draw.center, draw.size,
                    COLOR_WHITE, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                    Point(1.0 + juicyGrowAmount)
                );
            };
            gui.addItem(missionGoalCur, "mission_goal_cur");
            game.states.gameplay->missionGoalCurText = missionGoalCur;
            
            
            //Mission goal requirement label.
            GuiItem* missionGoalReqLabel = new GuiItem();
            missionGoalReqLabel->onDraw =
            [this] (const DrawInfo & draw) {
                drawText(
                    "Goal", game.sysContent.fntStandard, draw.center, draw.size,
                    al_map_rgba(255, 255, 255, 128)
                );
            };
            gui.addItem(missionGoalReqLabel, "mission_goal_req_label");
            
            
            //Mission goal requirement.
            GuiItem* missionGoalReq = new GuiItem();
            missionGoalReq->onDraw =
            [this] (const DrawInfo & draw) {
                int value =
                    game.missionGoals[game.curAreaData->mission.goal]->
                    getReqAmount(game.states.gameplay);
                string text;
                if(
                    game.curAreaData->mission.goal ==
                    MISSION_GOAL_TIMED_SURVIVAL
                ) {
                    text = timeToStr2(value, ":", "");
                } else {
                    text = i2s(value);
                }
                drawText(
                    text, game.sysContent.fntCounter, draw.center, draw.size
                );
            };
            gui.addItem(missionGoalReq, "mission_goal_req");
            
            
            //Mission goal slash.
            GuiItem* missionGoalSlash = new GuiItem();
            missionGoalSlash->onDraw =
            [this] (const DrawInfo & draw) {
                drawText(
                    "/", game.sysContent.fntCounter, draw.center, draw.size
                );
            };
            gui.addItem(missionGoalSlash, "mission_goal_slash");
            
        } else {
        
            //Mission goal name text.
            GuiItem* missionGoalName = new GuiItem();
            missionGoalName->onDraw =
            [this] (const DrawInfo & draw) {
                drawText(
                    game.missionGoals[game.curAreaData->mission.goal]->
                    getName(), game.sysContent.fntStandard,
                    draw.center, draw.size, al_map_rgba(255, 255, 255, 128)
                );
            };
            gui.addItem(missionGoalName, "mission_goal_name");
            
        }
        
    }
    
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.gradingMode == MISSION_GRADING_MODE_POINTS &&
        game.curAreaData->mission.pointHudData != 0
    ) {
    
        //Mission score bubble.
        GuiItem* missionScoreBubble = new GuiItem();
        missionScoreBubble->onDraw =
        [this] (const DrawInfo & draw) {
            drawFilledRoundedRectangle(
                draw.center, draw.size, 20.0f, al_map_rgba(86, 149, 50, 160)
            );
            drawTexturedBox(
                draw.center, draw.size, game.sysContent.bmpBubbleBox,
                al_map_rgba(255, 255, 255, 200)
            );
        };
        gui.addItem(missionScoreBubble, "mission_score_bubble");
        
        
        //Mission score "score" label.
        GuiItem* missionScoreScoreLabel = new GuiItem();
        missionScoreScoreLabel->onDraw =
        [this] (const DrawInfo & draw) {
            drawText(
                "Score:", game.sysContent.fntStandard,
                Point(draw.center.x + draw.size.x / 2.0f, draw.center.y),
                draw.size,
                al_map_rgba(255, 255, 255, 128), ALLEGRO_ALIGN_RIGHT
            );
        };
        gui.addItem(missionScoreScoreLabel, "mission_score_score_label");
        
        
        //Mission score points.
        GuiItem* missionScorePoints = new GuiItem();
        missionScorePoints->onDraw =
            [this, missionScorePoints]
        (const DrawInfo & draw) {
            float juicyGrowAmount = missionScorePoints->getJuiceValue();
            drawText(
                i2s(game.states.gameplay->missionScore),
                game.sysContent.fntCounter, draw.center, draw.size, COLOR_WHITE,
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0 + juicyGrowAmount)
            );
        };
        gui.addItem(missionScorePoints, "mission_score_points");
        game.states.gameplay->missionScoreCurText = missionScorePoints;
        
        
        //Mission score "points" label.
        GuiItem* missionScorePointsLabel = new GuiItem();
        missionScorePointsLabel->onDraw =
        [this] (const DrawInfo & draw) {
            drawText(
                "pts", game.sysContent.fntStandard,
                Point(draw.center.x + draw.size.x / 2.0f, draw.center.y),
                draw.size,
                al_map_rgba(255, 255, 255, 128), ALLEGRO_ALIGN_RIGHT,
                V_ALIGN_MODE_CENTER, 0, 0.66f
            );
        };
        gui.addItem(missionScorePointsLabel, "mission_score_points_label");
        
        
        //Mission score ruler.
        GuiItem* missionScoreRuler = new GuiItem();
        missionScoreRuler->onDraw =
        [this] (const DrawInfo & draw) {
            //Setup.
            const float lowestNormalValue =
                std::min(0, game.curAreaData->mission.bronzeReq);
            const float highestNormalValue =
                std::max(
                    game.curAreaData->mission.startingPoints,
                    game.curAreaData->mission.platinumReq
                );
            const float valueRange =
                (highestNormalValue - lowestNormalValue) *
                HUD::SCORE_RULER_RATIO_RANGE;
            const float startValue =
                game.states.gameplay->scoreFlapper -
                valueRange / 2.0f;
            const float endValue =
                game.states.gameplay->scoreFlapper +
                valueRange / 2.0f;
            const float valueScale = draw.size.x / valueRange;
            const float startX = draw.center.x - draw.size.x / 2.0f;
            const float endX = draw.center.x + draw.size.x / 2.0f;
            
            auto valueToWindowX = [&draw, &valueScale] (float value) {
                return
                    draw.center.x -
                    (game.states.gameplay->scoreFlapper - value) *
                    valueScale;
            };
            
            const float segLimits[] = {
                std::min(startValue, 0.0f),
                0,
                (float) game.curAreaData->mission.bronzeReq,
                (float) game.curAreaData->mission.silverReq,
                (float) game.curAreaData->mission.goldReq,
                (float) game.curAreaData->mission.platinumReq,
                std::max(
                    endValue,
                    (float) game.curAreaData->mission.platinumReq
                )
            };
            const ALLEGRO_COLOR segColorsTop[] = {
                al_map_rgba(152, 160, 152, 96),  //Negatives.
                al_map_rgba(204, 229, 172, 160), //No medal.
                al_map_rgb(229, 175, 126),       //Bronze.
                al_map_rgb(190, 224, 229),       //Silver.
                al_map_rgb(229, 212, 110),       //Gold.
                al_map_rgb(110, 229, 193)        //Platinum.
            };
            const ALLEGRO_COLOR segColorsBottom[] = {
                al_map_rgba(152, 160, 152, 96),  //Negatives.
                al_map_rgba(190, 214, 160, 160), //No medal.
                al_map_rgb(214, 111, 13),        //Bronze.
                al_map_rgb(156, 207, 214),       //Silver.
                al_map_rgb(214, 184, 4),         //Gold.
                al_map_rgb(3, 214, 144)          //Platinum.
            };
            ALLEGRO_BITMAP* segIcons[] = {
                nullptr,
                nullptr,
                game.sysContent.bmpMedalBronze,
                game.sysContent.bmpMedalSilver,
                game.sysContent.bmpMedalGold,
                game.sysContent.bmpMedalPlatinum
            };
            
            //Draw each segment (negatives, no medal, bronze, etc.).
            for(int s = 0; s < 6; s++) {
                float segStartValue =
                    s == 0 ? -FLT_MAX : segLimits[s];
                float segEndValue =
                    s == 5 ? FLT_MAX : segLimits[s + 1];
                float segStartX =
                    s == 0 ? -FLT_MAX : valueToWindowX(segStartValue);
                float segEndX =
                    s == 5 ? FLT_MAX : valueToWindowX(segEndValue);
                if(endX < segStartX) continue;
                if(startX > segEndX) continue;
                float segVisStartX = std::max(segStartX, startX);
                float segVisEndX = std::min(segEndX, endX);
                const ALLEGRO_COLOR& colorTop1 =
                    segColorsTop[s];
                const ALLEGRO_COLOR& colorTop2 =
                    s == 5 ? segColorsTop[5] : segColorsTop[s + 1];
                const ALLEGRO_COLOR& colorBottom1 =
                    segColorsBottom[s];
                const ALLEGRO_COLOR& colorBottom2 =
                    s == 5 ? segColorsBottom[5] : segColorsBottom[s + 1];
                ALLEGRO_COLOR segVisStartColorTop =
                    interpolateColor(
                        segVisStartX, segStartX, segEndX,
                        colorTop1, colorTop2
                    );
                ALLEGRO_COLOR segVisStartColorBottom =
                    interpolateColor(
                        segVisStartX, segStartX, segEndX,
                        colorBottom1, colorBottom2
                    );
                ALLEGRO_COLOR segVisEndColorTop =
                    interpolateColor(
                        segVisEndX, segStartX, segEndX,
                        colorTop1, colorTop2
                    );
                ALLEGRO_COLOR segVisEndColorBottom =
                    interpolateColor(
                        segVisEndX, segStartX, segEndX,
                        colorBottom1, colorBottom2
                    );
                    
                ALLEGRO_VERTEX vertexes[4];
                for(unsigned char v = 0; v < 4; v++) {
                    vertexes[v].z = 0.0f;
                }
                vertexes[0].x = segVisStartX;
                vertexes[0].y = draw.center.y - draw.size.y / 2.0f;
                vertexes[0].color = segVisStartColorTop;
                vertexes[1].x = segVisStartX;
                vertexes[1].y = draw.center.y + draw.size.y / 2.0f;
                vertexes[1].color = segVisStartColorBottom;
                vertexes[2].x = segVisEndX;
                vertexes[2].y = draw.center.y + draw.size.y / 2.0f;
                vertexes[2].color = segVisEndColorBottom;
                vertexes[3].x = segVisEndX;
                vertexes[3].y = draw.center.y - draw.size.y / 2.0f;
                vertexes[3].color = segVisEndColorTop;
                al_draw_prim(
                    vertexes, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            //Draw the markings.
            for(
                float m = floor(startValue / 25.0f) * 25.0f;
                m <= endValue;
                m += 25.0f
            ) {
                if(m < 0.0f || m < startValue) continue;
                float markingX = valueToWindowX(m);
                float markingLength =
                    fmod(m, 100) == 0 ?
                    draw.size.y * 0.7f :
                    fmod(m, 50) == 0 ?
                    draw.size.y * 0.4f :
                    draw.size.y * 0.1f;
                al_draw_filled_triangle(
                    markingX,
                    draw.center.y - draw.size.y / 2.0f + markingLength,
                    markingX + 2.0f,
                    draw.center.y - draw.size.y / 2.0f,
                    markingX - 2.0f,
                    draw.center.y - draw.size.y / 2.0f,
                    al_map_rgb(100, 110, 180)
                );
            }
            
            //Draw the medal icons.
            int curSeg = 0;
            int lastPassedSeg = 0;
            float curMedalScale =
                HUD::MEDAL_ICON_SCALE_CUR +
                sin(
                    game.states.gameplay->areaTimePassed *
                    HUD::MEDAL_ICON_SCALE_TIME_MULT
                ) *
                HUD::MEDAL_ICON_SCALE_MULT;
            for(int s = 0; s < 6; s++) {
                float segStartValue = segLimits[s];
                if(segStartValue <= game.states.gameplay->missionScore) {
                    curSeg = s;
                }
                if(segStartValue <= startValue) {
                    lastPassedSeg = s;
                }
            }
            float gotItX = LARGE_FLOAT;
            for(int s = 0; s < 6; s++) {
                if(!segIcons[s]) continue;
                float segStartValue = segLimits[s];
                if(segStartValue < startValue) continue;
                if(segStartValue > endValue) continue;
                float segVisStartX = valueToWindowX(segStartValue);
                float iconX = segVisStartX;
                float iconScale = HUD::MEDAL_ICON_SCALE_NEXT;
                if(curSeg == s) {
                    iconScale = curMedalScale;
                }
                drawBitmap(
                    segIcons[s],
                    Point(iconX, draw.center.y),
                    Point(-1, draw.size.y * iconScale)
                );
                if(curSeg == s) {
                    gotItX = iconX;
                }
                if(segStartValue > endValue) {
                    //If we found the first icon that goes past the ruler's end,
                    //then we shouldn't draw the other ones that come after.
                    break;
                }
            }
            if(segIcons[lastPassedSeg] && lastPassedSeg == curSeg) {
                drawBitmap(
                    segIcons[lastPassedSeg],
                    Point(startX, draw.center.y),
                    Point(-1, draw.size.y * curMedalScale)
                );
                gotItX = startX;
            }
            
            if(gotItX != LARGE_FLOAT) {
                float juiceTime =
                    game.states.gameplay->medalGotItJuiceTimer /
                    HUD::MEDAL_GOT_IT_JUICE_DURATION;
                juiceTime = std::min(juiceTime, 1.0f);
                drawBitmap(
                    game.sysContent.bmpMedalGotIt,
                    Point(gotItX, draw.center.y + draw.size.y / 2.0f),
                    Point(
                        -1,
                        draw.size.y * ease(EASE_METHOD_OUT_ELASTIC, juiceTime)
                    ),
                    TAU * 0.05f
                );
            }
            
            //Draw the flapper.
            al_draw_filled_triangle(
                draw.center.x, draw.center.y + draw.size.y / 2.0f,
                draw.center.x, draw.center.y,
                draw.center.x + (draw.size.y * 0.4),
                draw.center.y + draw.size.y / 2.0f,
                al_map_rgb(64, 186, 64)
            );
            al_draw_filled_triangle(
                draw.center.x, draw.center.y + draw.size.y / 2.0f,
                draw.center.x, draw.center.y,
                draw.center.x - (draw.size.y * 0.4),
                draw.center.y + draw.size.y / 2.0f,
                al_map_rgb(75, 218, 75)
            );
        };
        gui.addItem(missionScoreRuler, "mission_score_ruler");
        
    }
    
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.failHudPrimaryCond != INVALID
    ) {
        createMissionFailCondItems(true);
    }
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.failHudSecondaryCond != INVALID
    ) {
        createMissionFailCondItems(false);
    }
    
    
    DataNode* bitmapsNode = hudFileNode->getChildByName("bitmaps");
    
#define loader(var, name) \
    var = \
        game.content.bitmaps.list.get( \
            bitmapsNode->getChildByName(name)->value, \
            bitmapsNode->getChildByName(name) \
        );
    
    loader(bmpBubble,               "bubble");
    loader(bmpCounterBubbleField,   "counter_bubble_field");
    loader(bmpCounterBubbleGroup,   "counter_bubble_group");
    loader(bmpCounterBubbleStandby, "counter_bubble_standby");
    loader(bmpCounterBubbleTotal,   "counter_bubble_total");
    loader(bmpDayBubble,            "dayBubble");
    loader(bmpDistantPikminMarker,  "distant_pikmin_marker");
    loader(bmpHardBubble,           "hard_bubble");
    loader(bmpNoPikminBubble,       "no_pikmin_bubble");
    loader(bmpSun,                  "sun");
    
#undef loader
    
    leaderIconMgr.moveMethod = HUD_BUBBLE_MOVE_METHOD_CIRCLE;
    leaderIconMgr.transitionDuration = HUD::LEADER_SWAP_JUICE_DURATION;
    leaderHealthMgr.moveMethod = HUD_BUBBLE_MOVE_METHOD_CIRCLE;
    leaderHealthMgr.transitionDuration = HUD::LEADER_SWAP_JUICE_DURATION;
    standbyIconMgr.moveMethod = HUD_BUBBLE_MOVE_METHOD_STRAIGHT;
    standbyIconMgr.transitionDuration = HUD::STANDBY_SWAP_JUICE_DURATION;
    sprayIconMgr.moveMethod = HUD_BUBBLE_MOVE_METHOD_STRAIGHT;
    sprayIconMgr.transitionDuration = HUD::SPRAY_SWAP_JUICE_DURATION;
    
}


/**
 * @brief Destroys the HUD struct object.
 */
Hud::~Hud() {
    game.content.bitmaps.list.free(bmpBubble);
    game.content.bitmaps.list.free(bmpCounterBubbleField);
    game.content.bitmaps.list.free(bmpCounterBubbleGroup);
    game.content.bitmaps.list.free(bmpCounterBubbleStandby);
    game.content.bitmaps.list.free(bmpCounterBubbleTotal);
    game.content.bitmaps.list.free(bmpDayBubble);
    game.content.bitmaps.list.free(bmpDistantPikminMarker);
    game.content.bitmaps.list.free(bmpHardBubble);
    game.content.bitmaps.list.free(bmpNoPikminBubble);
    game.content.bitmaps.list.free(bmpSun);
}


/**
 * @brief Creates either the primary or the secondary mission fail condition
 * HUD items.
 *
 * @param primary True if it's the primary HUD item,
 * false if it's the secondary.
 */
void Hud::createMissionFailCondItems(bool primary) {
    MISSION_FAIL_COND cond =
        primary ?
        (MISSION_FAIL_COND)
        game.curAreaData->mission.failHudPrimaryCond :
        (MISSION_FAIL_COND)
        game.curAreaData->mission.failHudSecondaryCond;
        
    //Mission fail condition bubble.
    GuiItem* missionFailBubble = new GuiItem();
    missionFailBubble->onDraw =
    [this, primary] (const DrawInfo & draw) {
        int cx = 0;
        int cy = 0;
        int cw = 0;
        int ch = 0;
        float ratio =
            primary ?
            game.states.gameplay->fail1IndicatorRatio :
            game.states.gameplay->fail2IndicatorRatio;
        al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
        setCombinedClippingRectangles(
            cx, cy, cw, ch,
            draw.center.x - draw.size.x / 2.0f,
            draw.center.y - draw.size.y / 2.0f,
            draw.size.x * ratio + 1,
            draw.size.y
        );
        drawFilledRoundedRectangle(
            draw.center, draw.size, 20.0f, al_map_rgba(149, 80, 50, 160)
        );
        setCombinedClippingRectangles(
            cx, cy, cw, ch,
            draw.center.x - draw.size.x / 2.0f +
            draw.size.x * ratio,
            draw.center.y - draw.size.y / 2.0f,
            draw.size.x * (1 - ratio),
            draw.size.y
        );
        drawFilledRoundedRectangle(
            draw.center, draw.size, 20.0f, al_map_rgba(149, 130, 50, 160)
        );
        al_set_clipping_rectangle(cx, cy, cw, ch);
        drawTexturedBox(
            draw.center, draw.size, game.sysContent.bmpBubbleBox,
            al_map_rgba(255, 255, 255, 200)
        );
    };
    gui.addItem(
        missionFailBubble,
        primary ?
        "mission_fail_1_bubble" :
        "mission_fail_2_bubble"
    );
    
    
    if(game.missionFailConds[cond]->hasHudContent()) {
    
        //Mission fail condition current label.
        GuiItem* missionFailCurLabel = new GuiItem();
        missionFailCurLabel->onDraw =
        [this, cond] (const DrawInfo & draw) {
            drawText(
                game.missionFailConds[cond]->
                getHudLabel(game.states.gameplay),
                game.sysContent.fntStandard, draw.center, draw.size,
                al_map_rgba(255, 255, 255, 128)
            );
        };
        gui.addItem(
            missionFailCurLabel,
            primary ?
            "mission_fail_1_cur_label" :
            "mission_fail_2_cur_label"
        );
        
        
        //Mission fail condition current.
        GuiItem* missionFailCur = new GuiItem();
        missionFailCur->onDraw =
            [this, cond, missionFailCur]
        (const DrawInfo & draw) {
            int value =
                game.missionFailConds[cond]->
                getCurAmount(game.states.gameplay);
            string text;
            if(cond == MISSION_FAIL_COND_TIME_LIMIT) {
                text = timeToStr2(value, ":", "");
            } else {
                text = i2s(value);
            }
            float juicyGrowAmount = missionFailCur->getJuiceValue();
            drawText(
                text, game.sysContent.fntCounter, draw.center, draw.size,
                COLOR_WHITE, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0 + juicyGrowAmount)
            );
        };
        gui.addItem(
            missionFailCur,
            primary ?
            "mission_fail_1_cur" :
            "mission_fail_2_cur"
        );
        if(primary) {
            game.states.gameplay->missionFail1CurText = missionFailCur;
        } else {
            game.states.gameplay->missionFail2CurText = missionFailCur;
        }
        
        
        //Mission fail condition requirement label.
        GuiItem* missionFailReqLabel = new GuiItem();
        missionFailReqLabel->onDraw =
            [this]
        (const DrawInfo & draw) {
            drawText(
                "Fail", game.sysContent.fntStandard, draw.center, draw.size,
                al_map_rgba(255, 255, 255, 128)
            );
        };
        gui.addItem(
            missionFailReqLabel,
            primary ?
            "mission_fail_1_req_label" :
            "mission_fail_2_req_label"
        );
        
        
        //Mission fail condition requirement.
        GuiItem* missionFailReq = new GuiItem();
        missionFailReq->onDraw =
        [this, cond] (const DrawInfo & draw) {
            int value =
                game.missionFailConds[cond]->
                getReqAmount(game.states.gameplay);
            string text;
            if(cond == MISSION_FAIL_COND_TIME_LIMIT) {
                text = timeToStr2(value, ":", "");
            } else {
                text = i2s(value);
            }
            drawText(
                text, game.sysContent.fntCounter, draw.center, draw.size
            );
        };
        gui.addItem(
            missionFailReq,
            primary ?
            "mission_fail_1_req" :
            "mission_fail_2_req"
        );
        
        
        //Mission primary fail condition slash.
        GuiItem* missionFailSlash = new GuiItem();
        missionFailSlash->onDraw =
        [this] (const DrawInfo & draw) {
            drawText(
                "/", game.sysContent.fntCounter, draw.center, draw.size
            );
        };
        gui.addItem(
            missionFailSlash,
            primary ?
            "mission_fail_1_slash" :
            "mission_fail_2_slash"
        );
        
    } else {
    
        //Mission fail condition name text.
        GuiItem* missionFailName = new GuiItem();
        missionFailName->onDraw =
        [this, cond] (const DrawInfo & draw) {
            drawText(
                "Fail: " +
                game.missionFailConds[cond]->getName(),
                game.sysContent.fntStandard, draw.center, draw.size,
                al_map_rgba(255, 255, 255, 128)
            );
        };
        gui.addItem(
            missionFailName,
            primary ?
            "mission_fail_1_name" :
            "mission_fail_2_name"
        );
        
    }
}


/**
 * @brief Code to draw a spray icon with. This does not apply to the
 * second spray.
 *
 * @param which Which spray icon to draw -- the previous type's,
 * the current type's, or the next type's.
 */
void Hud::drawSprayIcon(BUBBLE_RELATION which) {
    if(!player->leaderPtr) return;
    
    DrawInfo draw;
    ALLEGRO_BITMAP* icon;
    sprayIconMgr.getDrawingInfo(
        which, &icon, &draw
    );
    
    if(!icon) return;
    drawBitmapInBox(
        icon, draw.center, draw.size, true, 0.0f,
        mapAlpha(sprayItemsOpacity * 255)
    );
}


/**
 * @brief Code to draw a standby icon with.
 *
 * @param which Which standby icon to draw -- the previous type's,
 * the current type's, or the next type's.
 */
void Hud::drawStandbyIcon(BUBBLE_RELATION which) {
    if(!player) return;
    
    DrawInfo draw;
    ALLEGRO_BITMAP* icon;
    standbyIconMgr.getDrawingInfo(
        which, &icon, &draw
    );
    
    if(!icon) return;
    
    ALLEGRO_COLOR color = mapAlpha(standbyItemsOpacity * 255);
    
    drawBitmapInBox(icon, draw.center, draw.size * 0.8, true, 0.0f, color);
    
    if(
        player->closestGroupMemberDistant &&
        which == BUBBLE_RELATION_CURRENT
    ) {
        drawBitmapInBox(
            bmpDistantPikminMarker,
            draw.center,
            draw.size * 0.8,
            true,
            0.0f, color
        );
    }
    
    drawBitmapInBox(bmpBubble, draw.center, draw.size, true, 0.0f, color);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Hud::tick(float deltaT) {
    if(!player) return;
    
    //Update leader bubbles.
    for(size_t l = 0; l < 3; l++) {
        Leader* lPtr = nullptr;
        if(l < game.states.gameplay->availableLeaders.size()) {
            size_t lIdx =
                (size_t) sumAndWrap(
                    (int) player->leaderIdx,
                    (int) l,
                    (int) game.states.gameplay->availableLeaders.size()
                );
            lPtr = game.states.gameplay->availableLeaders[lIdx];
        }
        
        LeaderIconBubble icon;
        icon.bmp = nullptr;
        icon.color = COLOR_EMPTY;
        if(lPtr) {
            icon.bmp = lPtr->leaType->bmpIcon;
            icon.color = lPtr->leaType->mainColor;
        }
        
        leaderIconMgr.update(l, lPtr, icon);
        
        LeaderHealthBubble health;
        health.ratio = 0.0f;
        health.cautionTimer = 0.0f;
        if(lPtr) {
            health.ratio = lPtr->healthWheelVisibleRatio;
            health.cautionTimer = lPtr->healthWheelCautionTimer;
            lPtr->healthWheelShaker.getOffsets(
                &health.offset.x, &health.offset.y
            );
            health.redness = lPtr->healthWheelShaker.getTrauma();
        }
        leaderHealthMgr.update(l, lPtr, health);
    }
    leaderIconMgr.tick(deltaT);
    leaderHealthMgr.tick(deltaT);
    
    //Update standby bubbles.
    for(unsigned char s = 0; s < 3; s++) {
    
        ALLEGRO_BITMAP* icon = nullptr;
        Leader* curLeaderPtr = player->leaderPtr;
        Mob* member = player->closestGroupMember[s];
        SubgroupType* type = nullptr;
        
        if(curLeaderPtr) {
            switch(s) {
            case BUBBLE_RELATION_PREVIOUS: {
                SubgroupType* prevType;
                curLeaderPtr->group->getNextStandbyType(true, &prevType);
                SubgroupType* nextType;
                curLeaderPtr->group->getNextStandbyType(false, &nextType);
                if(
                    prevType != curLeaderPtr->group->curStandbyType &&
                    prevType != nextType
                ) {
                    type = prevType;
                }
                break;
            }
            case BUBBLE_RELATION_CURRENT: {
                type = curLeaderPtr->group->curStandbyType;
                break;
            }
            case BUBBLE_RELATION_NEXT: {
                SubgroupType* nextType;
                curLeaderPtr->group->getNextStandbyType(false, &nextType);
                if(nextType != curLeaderPtr->group->curStandbyType) {
                    type = nextType;
                }
                break;
            }
            }
        }
        
        if(curLeaderPtr && type && member) {
            SUBGROUP_TYPE_CATEGORY cat = type->getCategory();
            
            switch(cat) {
            case SUBGROUP_TYPE_CATEGORY_LEADER: {
                Leader* lPtr = dynamic_cast<Leader*>(member);
                icon = lPtr->leaType->bmpIcon;
                break;
            } default: {
                icon = type->getIcon();
                break;
            }
            }
        }
        
        if(!icon && s == BUBBLE_RELATION_CURRENT) {
            icon = bmpNoPikminBubble;
        }
        
        standbyIconMgr.update(s, type, icon);
    }
    standbyIconMgr.tick(deltaT);
    
    //Update spray bubbles.
    size_t topSprayIdx = INVALID;
    if(game.content.sprayTypes.list.size() > 2) {
        topSprayIdx = player->selectedSpray;
    } else if(
        !game.content.sprayTypes.list.empty() &&
        game.content.sprayTypes.list.size() <= 2
    ) {
        topSprayIdx = 0;
    }
    sprayIconMgr.update(
        BUBBLE_RELATION_CURRENT,
        topSprayIdx == INVALID ? nullptr :
        &player->team->sprayStats[topSprayIdx],
        topSprayIdx == INVALID ? nullptr :
        game.config.misc.sprayOrder[topSprayIdx]->bmpSpray
    );
    
    size_t prevSprayIdx = INVALID;
    if(game.content.sprayTypes.list.size() >= 3) {
        prevSprayIdx =
            sumAndWrap(
                (int) player->selectedSpray,
                -1,
                (int) game.content.sprayTypes.list.size()
            );
    }
    sprayIconMgr.update(
        BUBBLE_RELATION_PREVIOUS,
        prevSprayIdx == INVALID ? nullptr :
        &player->team->sprayStats[prevSprayIdx],
        prevSprayIdx == INVALID ? nullptr :
        game.config.misc.sprayOrder[prevSprayIdx]->bmpSpray
    );
    
    size_t nextSprayIdx = INVALID;
    if(game.content.sprayTypes.list.size() >= 3) {
        nextSprayIdx =
            sumAndWrap(
                (int) player->selectedSpray,
                1,
                (int) game.content.sprayTypes.list.size()
            );
    }
    sprayIconMgr.update(
        BUBBLE_RELATION_NEXT,
        nextSprayIdx == INVALID ? nullptr :
        &player->team->sprayStats[nextSprayIdx],
        nextSprayIdx == INVALID ? nullptr :
        game.config.misc.sprayOrder[nextSprayIdx]->bmpSpray
    );
    
    sprayIconMgr.tick(deltaT);
    
    //Update the standby items opacity.
    if(
        !player->leaderPtr ||
        player->leaderPtr->group->members.empty()
    ) {
        if(standbyItemsFadeTimer > 0.0f) {
            standbyItemsFadeTimer -= deltaT;
        } else {
            standbyItemsOpacity -=
                HUD::UNNECESSARY_ITEMS_FADE_OUT_SPEED * deltaT;
        }
    } else {
        standbyItemsFadeTimer =
            HUD::UNNECESSARY_ITEMS_FADE_OUT_DELAY;
        standbyItemsOpacity +=
            HUD::UNNECESSARY_ITEMS_FADE_IN_SPEED * deltaT;
    }
    standbyItemsOpacity = std::clamp(standbyItemsOpacity, 0.0f, 1.0f);
    
    //Update the spray items opacity.
    size_t totalSprays = 0;
    for(size_t s = 0; s < player->team->sprayStats.size(); s++) {
        totalSprays +=
            player->team->sprayStats[s].nrSprays;
    }
    if(totalSprays == 0) {
        if(sprayItemsFadeTimer > 0.0f) {
            sprayItemsFadeTimer -= deltaT;
        } else {
            sprayItemsOpacity -=
                HUD::UNNECESSARY_ITEMS_FADE_OUT_SPEED * deltaT;
        }
    } else {
        sprayItemsFadeTimer =
            HUD::UNNECESSARY_ITEMS_FADE_OUT_DELAY;
        sprayItemsOpacity +=
            HUD::UNNECESSARY_ITEMS_FADE_IN_SPEED * deltaT;
    }
    sprayItemsOpacity = std::clamp(sprayItemsOpacity, 0.0f, 1.0f);
    
    //Tick the GUI items proper.
    gui.tick(game.deltaT);
}
