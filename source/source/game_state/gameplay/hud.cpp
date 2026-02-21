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

//Delay before the control guide is allowed to appear.
const float CONTROL_GUIDE_DELAY = 1.0f;

//The control guide's opacity changes these many units per second.
const float CONTROL_GUIDE_OPACITY_SPEED = 2.0f;

//Smoothen the mission goal indicator's movement by this factor.
const float GOAL_INDICATOR_SMOOTHNESS_FACTOR = 5.5f;

//Name of the GUI definition file.
const string GUI_FILE_NAME = "gameplay";

//Maximum amount in any direction that the leader health wheel is allowed
//to offset when shaking.
const float HEALTH_SHAKE_MAX_OFFSET = 20.0f;

//How long the leader swap juice animation lasts for.
const float LEADER_SWAP_JUICE_DURATION = 0.7f;

//How long the medal "Got it!" text juice animation lasts for.
const float MEDAL_GOT_IT_JUICE_DURATION = 1.3f;

//Standard scale of the "Got it!" mission medal text.
const float MEDAL_GOT_IT_SCALE = 0.5f;

//Standard mission score medal icon scale, for the obtained medal.
const float MEDAL_ICON_SCALE_CUR = 1.5f;

//Multiply time by this much to get the right scale animation amount.
const float MEDAL_ICON_SCALE_MULT = 0.3f;

//Standard mission score medal icon scale, for the next medal.
const float MEDAL_ICON_SCALE_NEXT = 1.0f;

//Multiply time by this much to get the right scale animation speed.
const float MEDAL_ICON_SCALE_TIME_MULT = 4.0f;

//Name of the GUI definition file for the mission amount (one amount) items.
const string MISSION_AMT_ONE_GUI_FILE_NAME = "gameplay_mission_amount_one";

//Name of the GUI definition file for the mission amount (two amounts) items.
const string MISSION_AMT_TWO_GUI_FILE_NAME = "gameplay_mission_amount_two";

//Name of the GUI definition file for the mission clock items.
const string MISSION_CLOCK_GUI_FILE_NAME = "gameplay_mission_clock";

//Name of the GUI definition file for the mission score items.
const string MISSION_SCORE_GUI_FILE_NAME = "gameplay_mission_score";

//Name of the GUI definition file for the mission custom text items.
const string MISSION_TEXT_GUI_FILE_NAME = "gameplay_mission_text";

//Smoothen the mission score indicator's movement by this factor.
const float SCORE_INDICATOR_SMOOTHNESS_FACTOR = 5.5f;

//Ratio of the score gamut to show around the mission score ruler flapper.
const float SCORE_RULER_RATIO_RANGE = 0.20f;

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
    standbyIconMgr(&gui) {
    
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
    gui.registerCoords("mission_goal_main",            50,   37, 12, 10); //TODO
    gui.registerCoords("mission_goal_score",           50,   37, 12, 10); //TODO
    gui.registerCoords("mission_goal_clock",           50,   37, 12, 10); //TODO
    gui.registerCoords("mission_goal_misc",            50,   37, 12, 10); //TODO
    gui.registerCoords("control_guide",                50,   37, 12, 10); //TODO
    gui.registerCoords("inventory_shortcut_usage",     50,   37, 12, 10);
    gui.readDataFile(hudFileNode);
    
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
                tintColor(
                    changeAlpha(icon.color, 128),
                    draw.tint
                )
            );
            drawBitmapInBox(
                icon.bmp,
                finalDraw.center, finalDraw.size, true, 0.0f, draw.tint
            );
            drawBitmapInBox(
                bmpBubble,
                finalDraw.center, finalDraw.size, true, 0.0f, draw.tint
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
                draw.tint.a,
                std::min(finalDraw.size.x, finalDraw.size.y) * 0.47f,
                true
            );
            drawBitmapInBox(
                bmpHardBubble,
                finalDraw.center,
                finalDraw.size,
                true, 0.0f,
                tintColor(
                    interpolateColor(
                        health.redness, 0.0f, 1.0f,
                        COLOR_WHITE, al_map_rgb(255, 0, 0)
                    ),
                    draw.tint
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
                alphaKeyframes.addNew(0.2f, 255);
                alphaKeyframes.addNew(0.3f, 255);
                alphaKeyframes.addNew(0.8f, 0);
                alphaKeyframes.addNew(1.0f, 0);
                
                float cautionRingSize =
                    std::min(finalDraw.size.x, finalDraw.size.y) *
                    cautionRingScale;
                    
                drawBitmap(
                    game.sysContent.bmpLowHealthRing,
                    finalDraw.center,
                    Point(cautionRingSize),
                    0.0f,
                    tintColor(
                        mapAlpha(alphaKeyframes.get(animRatio)), draw.tint
                    )
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
        if(!game.options.misc.showGuiInputIcons) return;
        if(game.states.gameplay->availableLeaders.size() < 2) return;
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_NEXT_LEADER, draw.center, draw.size,
            true, game.sysContent.fntSlim, draw.tint
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
            Point(sunRadius * 0.9), 0.0f, draw.tint
        );
        drawBitmap(
            bmpHardBubble,
            Point(firstDotX + dotsSpan * 0.5, dotsY),
            Point(sunRadius * 0.9), 0.0f, draw.tint
        );
        drawBitmap(
            bmpHardBubble,
            Point(firstDotX + dotsSpan * 1.0, dotsY),
            Point(sunRadius * 0.9), 0.0f, draw.tint
        );
        
        for(unsigned char h = 0; h < nHours + 1; h++) {
            drawBitmap(
                bmpHardBubble,
                Point(firstDotX + h * dotInterval, dotsY),
                Point(sunRadius * 0.6), 0.0f, draw.tint
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
            sunSize, 0.0f, draw.tint
        );
        //Spinning sun.
        drawBitmap(
            bmpSun,
            Point(firstDotX + dayPassedRatio * dotsSpan, dotsY),
            sunSize,
            sunMeterSunAngle, draw.tint
        );
        //Bubble in front the sun.
        drawBitmap(
            bmpHardBubble,
            Point(firstDotX + dayPassedRatio * dotsSpan, dotsY),
            sunSize,
            0.0f,
            tintColor(al_map_rgb(255, 192, 128), draw.tint)
        );
    };
    sunMeter->onTick =
    [this, sunMeter] (float deltaT) {
        float dayLength =
            game.config.misc.dayMinutesEnd - game.config.misc.dayMinutesStart;
        float preTickDayMinutes =
            game.states.gameplay->dayMinutes -
            game.curArea->dayTimeSpeed * deltaT / 60.0f;
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
        drawBitmapInBox(
            bmpDayBubble, draw.center, draw.size, true, 0.0f, draw.tint
        );
    };
    gui.addItem(dayBubble, "day_bubble");
    
    
    //Day number text.
    GuiItem* dayNr = new GuiItem();
    dayNr->onDraw =
    [this] (const DrawInfo & draw) {
        drawText(
            i2s(game.states.gameplay->day),
            game.sysContent.fntCounter, draw.center,
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f), draw.tint
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
        if(!game.options.misc.showGuiInputIcons) return;
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
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_NEXT_TYPE, draw.center, draw.size,
            true, game.sysContent.fntSlim,
            tintColor(mapAlpha(this->standbyItemsOpacity * 255), draw.tint)
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
        if(!game.options.misc.showGuiInputIcons) return;
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
        drawPlayerActionInputSourceIcon(
            PLAYER_ACTION_TYPE_PREV_TYPE, draw.center, draw.size,
            true, game.sysContent.fntSlim,
            tintColor(mapAlpha(this->standbyItemsOpacity * 255), draw.tint)
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
                0.0f, tintColor(color, draw.tint)
            );
            drawBitmapInBox(
                bmpBubble, draw.center,
                draw.size + standbyMaturityIcon->getJuiceValue(),
                true, 0.0f, tintColor(color, draw.tint)
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
            tintColor(mapAlpha(this->standbyItemsOpacity * 255), draw.tint)
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
            tintColor(mapAlpha(this->standbyItemsOpacity * 255), draw.tint),
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
            draw.center, draw.size, 0.0f, draw.tint
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
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f), draw.tint,
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
            draw.center, draw.size, 0.0f, draw.tint
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
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f), draw.tint,
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
            draw.center, draw.size, 0.0f, draw.tint
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
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f), draw.tint,
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
            tintColor(mapAlpha(this->standbyItemsOpacity * 255), draw.tint)
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
                "/", game.sysContent.fntCounter, draw.center, draw.size,
                draw.tint
            );
        };
        gui.addItem(counterSlash, "counters_slash_" + i2s(s + 1));
    }
    
    
    if(game.curArea->type == AREA_TYPE_MISSION) {
        //Mission "goal" item.
        GuiItem* missionGoalMainItem = new GuiItem();
        gui.addItem(missionGoalMainItem, "mission_goal_main");
        setupMissionHudItem(MISSION_HUD_ITEM_ID_GOAL, missionGoalMainItem);
        
        //Mission "score" item.
        GuiItem* missionGoalSecItem = new GuiItem();
        gui.addItem(missionGoalSecItem, "mission_goal_score");
        setupMissionHudItem(MISSION_HUD_ITEM_ID_SCORE, missionGoalSecItem);
        
        //Mission "clock" item.
        GuiItem* missionFailMainItem = new GuiItem();
        gui.addItem(missionFailMainItem, "mission_fail_clock");
        setupMissionHudItem(MISSION_HUD_ITEM_ID_CLOCK, missionFailMainItem);
        
        //Mission "misc." item.
        GuiItem* missionFailSecItem = new GuiItem();
        gui.addItem(missionFailSecItem, "mission_fail_misc");
        setupMissionHudItem(MISSION_HUD_ITEM_ID_MISC, missionFailSecItem);
    }
    
    
    //Control guide.
    const string controlGuideText =
        "\\k move_up \\k \\k move_left \\k \\k move_down \\k "
        "\\k move_right \\k Move\n"
        "\\k throw \\k Throw Pikmin\n"
        "\\k whistle \\k Whistle Pikmin\n"
        "\n"
        "\\k prev_type \\k \\k next_type \\k Swap Pikmin\n"
        "\\k next_leader \\k Swap leader\n"
        "\\k swarm_cursor \\k Swarm Pikmin\n"
        "\\k dismiss \\k Dismiss\n"
        "\n"
        "\\k inventory \\k Open inventory\n"
        "\\k radar \\k Open radar\n"
        "\n"
        "Pause (\\k pause \\k) and hit \"Help\" for more!";
    TextGuiItem* controlGuide =
        new TextGuiItem(controlGuideText, game.sysContent.fntSlim);
    controlGuide->flags = ALLEGRO_ALIGN_LEFT;
    controlGuide->lineWrap = true;
    controlGuide->controlCondensed = true;
    controlGuide->onDraw =
    [this, controlGuide] (const DrawInfo & draw) {
        if(!game.options.misc.showControlGuide) return;
        DrawInfo drawWithAlpha = draw;
        drawWithAlpha.tint.a *= controlGuideOpacity;
        drawFilledRoundedRectangle(
            draw.center, draw.size, 8.0f,
            tintColor(game.config.guiColors.pauseBg, drawWithAlpha.tint)
        );
        DrawInfo drawSmaller = drawWithAlpha;
        drawSmaller.size *= 0.95f;
        controlGuide->defDrawCode(drawSmaller);
    };
    gui.addItem(controlGuide, "control_guide");
    
    
    //Inventory shortcut usage display.
    GuiItem* inventoryShortcutUsage = new GuiItem();
    inventoryShortcutUsage->onDraw =
    [this] (const DrawInfo & draw) {
        if(player->inventoryShortcutDisplayIdx != INVALID) {
            const string& itemIName =
                game.options.controls.inventoryShortcuts[player->playerNr]
                [player->inventoryShortcutDisplayIdx];
            KeyframeInterpolator<float> alphaKI;
            alphaKI.addNew(1.0f, 0.0f);
            alphaKI.addNew(0.9f, 1.0f);
            alphaKI.addNew(0.5f, 1.0f);
            alphaKI.addNew(0.0f, 0.0f);
            KeyframeInterpolator<float> yOffsetKI;
            yOffsetKI.addNew(1.0f, 15.0f);
            yOffsetKI.addNew(0.9f, 0.0f, EASE_METHOD_IN);
            float timeRatio =
                player->inventoryShortcutDisplayTimer /
                DRAWING::INVENTORY_SHORTCUT_DISPLAY_DURATION;
            float alphaMult = alphaKI.get(timeRatio);
            InventoryItem* iPtr =
                game.inventoryItems.getByIName(itemIName);
            Point offset(0.0f, yOffsetKI.get(timeRatio));
            
            drawBitmapInBox(
                iPtr->icon, draw.center + offset, draw.size, true, 0.0f,
                multAlpha(draw.tint, alphaMult)
            );
            if(iPtr->onGetAmount) {
                drawText(
                    "x" + i2s(iPtr->onGetAmount(player)),
                    game.sysContent.fntCounter,
                    draw.center + offset + draw.size / 2.0f,
                    Point(0.80f, 0.50f) * draw.size,
                    multAlpha(draw.tint, alphaMult),
                    ALLEGRO_ALIGN_RIGHT, V_ALIGN_MODE_BOTTOM
                );
            }
        }
    };
    inventoryShortcutUsage->forceSquare = true;
    gui.addItem(inventoryShortcutUsage, "inventory_shortcut_usage");
    
    
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
    
    ALLEGRO_COLOR color =
        tintColor(mapAlpha(standbyItemsOpacity * 255), draw.tint);
        
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
 * @brief Sets up a mission HUD item.
 *
 * @param which Which of the items to set up.
 * @param item The item being set up.
 */
void Hud::setupMissionHudItem(MISSION_HUD_ITEM_ID which, GuiItem* item) {
    item->clipChildren = false;
    MissionHudItem* itemInfo = &game.curArea->mission.hudItems[which];
    if(!itemInfo->enabled) return;
    
    switch(itemInfo->contentType) {
    case MISSION_HUD_ITEM_CONTENT_TEXT: {
        //Text.
        
        DataNode* guiFile =
            &game.content.guiDefs.list[HUD::MISSION_TEXT_GUI_FILE_NAME];
        gui.registerCoords("mission_text_text", 50, 50, 92, 56);
        gui.readDataFile(guiFile, item);
        
        //The text.
        GuiItem* text = new GuiItem();
        text->onDraw =
        [itemInfo, this] (const DrawInfo & draw) {
            drawText(
                itemInfo->text, game.sysContent.fntStandard,
                draw.center, draw.size,
                tintColor(mapAlpha(224), draw.tint)
            );
        };
        item->addChild(text);
        gui.addItem(text, "mission_text_text");
        
        break;
        
    } case MISSION_HUD_ITEM_CONTENT_CLOCK_DOWN:
    case MISSION_HUD_ITEM_CONTENT_CLOCK_UP: {
        //Clock.
        
        DataNode* guiFile =
            &game.content.guiDefs.list[HUD::MISSION_CLOCK_GUI_FILE_NAME];
        gui.registerCoords("mission_clock_analog",  22, 50, 36, 92);
        gui.registerCoords("mission_clock_digital", 70, 50, 52, 92);
        gui.readDataFile(guiFile, item);
        
        //Analog clock.
        GuiItem* analog = new GuiItem();
        analog->onDraw =
        [itemInfo, this] (const DrawInfo & draw) {
            drawBitmap(
                game.sysContent.bmpClock,
                draw.center, draw.size, 0.0f, draw.tint
            );
            float clockHandAngle = (-TAU / 4.0f); //Start pointing upwards.
            if(itemInfo->contentType == MISSION_HUD_ITEM_CONTENT_CLOCK_DOWN) {
                if(
                    game.curArea->mission.timeLimit > 0.0f &&
                    game.states.gameplay->gameplayTimePassed <=
                    game.curArea->mission.timeLimit
                ) {
                    float timeSpentRatio =
                        game.states.gameplay->gameplayTimePassed /
                        game.curArea->mission.timeLimit;
                    clockHandAngle += timeSpentRatio * TAU;
                }
            } else {
                float minuteSpentRatio =
                    fmod(
                        game.states.gameplay->gameplayTimePassed, 60.0f
                    ) / 60.0f;
                clockHandAngle += minuteSpentRatio * TAU;
            }
            drawBitmap(
                game.sysContent.bmpClockHand,
                draw.center, draw.size, clockHandAngle, draw.tint
            );
        };
        analog->forceSquare = true;
        item->addChild(analog);
        gui.addItem(analog, "mission_clock_analog");
        
        //Digital clock.
        GuiItem* digital = new GuiItem();
        digital->onDraw =
        [itemInfo, this] (const DrawInfo & draw) {
            size_t seconds = 0;
            if(itemInfo->contentType == MISSION_HUD_ITEM_CONTENT_CLOCK_DOWN) {
                if(
                    game.curArea->mission.timeLimit > 0.0f &&
                    game.states.gameplay->gameplayTimePassed <=
                    game.curArea->mission.timeLimit
                ) {
                    seconds =
                        game.curArea->mission.timeLimit -
                        game.states.gameplay->gameplayTimePassed;
                }
            } else {
                seconds = game.states.gameplay->gameplayTimePassed;
            }
            drawText(
                timeToStr2(seconds), game.sysContent.fntCounter,
                draw.center, draw.size, draw.tint
            );
        };
        item->addChild(digital);
        gui.addItem(digital, "mission_clock_digital");
        
        break;
        
    } case MISSION_HUD_ITEM_CONTENT_SCORE: {
        //Score.
        
        DataNode* guiFile =
            &game.content.guiDefs.list[HUD::MISSION_SCORE_GUI_FILE_NAME];
        gui.registerCoords("mission_score_label",        15, 74, 22, 44);
        gui.registerCoords("mission_score_points",       50, 74, 40, 44);
        gui.registerCoords("mission_score_points_label", 85, 80, 22, 32);
        gui.registerCoords("mission_score_ruler",        50, 26, 88, 40);
        gui.readDataFile(guiFile, item);
        
        //"Score" label.
        GuiItem* scoreLabel = new GuiItem();
        scoreLabel->onDraw =
        [this] (const DrawInfo & draw) {
            drawText(
                "Score:", game.sysContent.fntStandard,
                Point(draw.center.x + draw.size.x / 2.0f, draw.center.y),
                draw.size,
                tintColor(mapAlpha(128), draw.tint), ALLEGRO_ALIGN_RIGHT
            );
        };
        item->addChild(scoreLabel);
        gui.addItem(scoreLabel, "mission_score_label");
        
        //Score points.
        GuiItem* points = new GuiItem();
        points->onDraw =
            [this, points]
        (const DrawInfo & draw) {
            float juicyGrowAmount = points->getJuiceValue();
            drawText(
                i2s(game.states.gameplay->missionScore),
                game.sysContent.fntCounter, draw.center, draw.size, draw.tint,
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0 + juicyGrowAmount)
            );
        };
        item->addChild(points);
        gui.addItem(points, "mission_score_points");
        game.states.gameplay->missionScoreCurText = points;
        
        //"Points" label.
        GuiItem* pointsLabel = new GuiItem();
        pointsLabel->onDraw =
        [this] (const DrawInfo & draw) {
            drawText(
                "pts", game.sysContent.fntStandard,
                Point(draw.center.x + draw.size.x / 2.0f, draw.center.y),
                draw.size,
                tintColor(mapAlpha(128), draw.tint),
                ALLEGRO_ALIGN_RIGHT,
                V_ALIGN_MODE_CENTER, 0, 0.66f
            );
        };
        item->addChild(pointsLabel);
        gui.addItem(pointsLabel, "mission_score_points_label");
        
        //Ruler.
        GuiItem* ruler = new GuiItem();
        ruler->onDraw =
        [this] (const DrawInfo & draw) {
            //Setup.
            const float lowestNormalValue =
                std::min(0, game.curArea->mission.bronzeReq);
            const float highestNormalValue =
                std::max(
                    game.curArea->mission.startingPoints,
                    game.curArea->mission.platinumReq
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
                (float) game.curArea->mission.bronzeReq,
                (float) game.curArea->mission.silverReq,
                (float) game.curArea->mission.goldReq,
                (float) game.curArea->mission.platinumReq,
                std::max(
                    endValue,
                    (float) game.curArea->mission.platinumReq
                )
            };
            ALLEGRO_COLOR segColorsTop[] = {
                al_map_rgba(152, 160, 152, 96),  //Negatives.
                al_map_rgba(204, 229, 172, 160), //No medal.
                al_map_rgb(229, 175, 126),       //Bronze.
                al_map_rgb(190, 224, 229),       //Silver.
                al_map_rgb(229, 212, 110),       //Gold.
                al_map_rgb(110, 229, 193)        //Platinum.
            };
            ALLEGRO_COLOR segColorsBottom[] = {
                al_map_rgba(152, 160, 152, 96),  //Negatives.
                al_map_rgba(190, 214, 160, 160), //No medal.
                al_map_rgb(214, 111, 13),        //Bronze.
                al_map_rgb(156, 207, 214),       //Silver.
                al_map_rgb(214, 184, 4),         //Gold.
                al_map_rgb(3, 214, 144)          //Platinum.
            };
            for(unsigned char c = 0; c < 6; c++) {
                segColorsTop[c] = tintColor(segColorsTop[c], draw.tint);
                segColorsTop[c] = tintColor(segColorsBottom[c], draw.tint);
            }
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
                    tintColor(al_map_rgb(100, 110, 180), draw.tint)
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
                    Point(-1, draw.size.y * iconScale), 0.0f, draw.tint
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
                    Point(-1, draw.size.y * curMedalScale), 0.0f, draw.tint
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
                        draw.size.y * HUD::MEDAL_GOT_IT_SCALE*
                        ease(juiceTime, EASE_METHOD_OUT_ELASTIC)
                    ),
                    TAU * 0.05f, draw.tint
                );
            }
            
            //Draw the flapper.
            al_draw_filled_triangle(
                draw.center.x, draw.center.y + draw.size.y / 2.0f,
                draw.center.x, draw.center.y,
                draw.center.x + (draw.size.y * 0.4),
                draw.center.y + draw.size.y / 2.0f,
                tintColor(al_map_rgb(64, 186, 64), draw.tint)
            );
            al_draw_filled_triangle(
                draw.center.x, draw.center.y + draw.size.y / 2.0f,
                draw.center.x, draw.center.y,
                draw.center.x - (draw.size.y * 0.4),
                draw.center.y + draw.size.y / 2.0f,
                tintColor(al_map_rgb(75, 218, 75), draw.tint)
            );
        };
        item->addChild(ruler);
        gui.addItem(ruler, "mission_score_ruler");
        
        break;
        
    } case MISSION_HUD_ITEM_CONTENT_CUR_TOT:
    case MISSION_HUD_ITEM_CONTENT_REM_TOT:
    case MISSION_HUD_ITEM_CONTENT_CUR_AMT:
    case MISSION_HUD_ITEM_CONTENT_REM_AMT:
    case MISSION_HUD_ITEM_CONTENT_TOT_AMT: {
        //Amount.
        
        bool oneAmount =
            itemInfo->contentType == MISSION_HUD_ITEM_CONTENT_CUR_AMT ||
            itemInfo->contentType == MISSION_HUD_ITEM_CONTENT_REM_AMT ||
            itemInfo->contentType == MISSION_HUD_ITEM_CONTENT_TOT_AMT;
            
        if(oneAmount) {
            DataNode* guiFile =
                &game.content.guiDefs.list[HUD::MISSION_AMT_ONE_GUI_FILE_NAME];
            gui.registerCoords("mission_amount_1_label", 50, 18, 92, 28);
            gui.registerCoords("mission_amount_1_first", 50, 65, 92, 58);
            gui.readDataFile(guiFile, item);
            
        } else {
            DataNode* guiFile =
                &game.content.guiDefs.list[HUD::MISSION_AMT_TWO_GUI_FILE_NAME];
            gui.registerCoords("mission_amount_2_label",  50, 18, 92, 28);
            gui.registerCoords("mission_amount_2_first",  22, 65, 36, 58);
            gui.registerCoords("mission_amount_2_second", 78, 65, 36, 58);
            gui.readDataFile(guiFile, item);
            
        }
        
        const auto getAmounts =
        [itemInfo] (int* amt1, int* amt2) {
            int currentAmount = 0;
            int remainingAmount = 0;
            int totalAmount = 0;
            
            switch(itemInfo->amountType) {
            case MISSION_HUD_ITEM_AMT_MOB_CHECKLIST: {
                for(size_t c = 0; c < itemInfo->idxsList.size(); c++) {
                    MissionMobChecklistStatus* cPtr =
                        &game.states.gameplay->missionMobChecklists[
                            itemInfo->idxsList[c] - 1
                        ];
                    currentAmount +=
                        cPtr->startingAmount - cPtr->remaining.size();
                    remainingAmount += cPtr->remaining.size();
                    totalAmount += cPtr->requiredAmount;
                }
                break;
                
            } case MISSION_HUD_ITEM_AMT_LEADERS_IN_REGION: {
                unordered_set<Leader*> leadersInRegions;
                for(size_t r = 0; r < itemInfo->idxsList.size(); r++) {
                    AreaRegionStatus* rPtr =
                        &game.states.gameplay->areaRegions[
                            itemInfo->idxsList[r] - 1
                        ];
                    leadersInRegions.insert(
                        rPtr->leadersInside.begin(), rPtr->leadersInside.end()
                    );
                }
                currentAmount = leadersInRegions.size();
                remainingAmount = itemInfo->totalAmount - currentAmount;
                totalAmount = itemInfo->totalAmount;
                break;
                
            } case MISSION_HUD_ITEM_AMT_PIKMIN: {
                currentAmount = game.states.gameplay->getAmountOfTotalPikmin();
                remainingAmount = itemInfo->totalAmount - currentAmount;
                totalAmount = itemInfo->totalAmount;
                break;
                
            } case MISSION_HUD_ITEM_AMT_LEADERS: {
                for(
                    size_t l = 0;
                    l < game.states.gameplay->mobs.leaders.size(); l++
                ) {
                    Leader* lPtr = game.states.gameplay->mobs.leaders[l];
                    if(lPtr->health > 0) {
                        currentAmount++;
                    }
                }
                remainingAmount = itemInfo->totalAmount - currentAmount;
                totalAmount = itemInfo->totalAmount;
                break;
                
            } case MISSION_HUD_ITEM_AMT_PIKMIN_DEATHS: {
                currentAmount = game.states.gameplay->pikminDeaths;
                remainingAmount = itemInfo->totalAmount - currentAmount;
                totalAmount = itemInfo->totalAmount;
                break;
                
            } case MISSION_HUD_ITEM_AMT_LEADER_KOS: {
                currentAmount = game.states.gameplay->leadersKod;
                remainingAmount = itemInfo->totalAmount - currentAmount;
                totalAmount = itemInfo->totalAmount;
                break;
                
            }
            }
            
            switch(itemInfo->contentType) {
            case MISSION_HUD_ITEM_CONTENT_CUR_TOT: {
                *amt1 = currentAmount;
                *amt2 = totalAmount;
                break;
            } case MISSION_HUD_ITEM_CONTENT_REM_TOT: {
                *amt1 = remainingAmount;
                *amt2 = totalAmount;
                break;
            } case MISSION_HUD_ITEM_CONTENT_CUR_AMT: {
                *amt1 = currentAmount;
                break;
            } case MISSION_HUD_ITEM_CONTENT_REM_AMT: {
                *amt1 = remainingAmount;
                break;
            } case MISSION_HUD_ITEM_CONTENT_TOT_AMT: {
                *amt1 = totalAmount;
                break;
            } default: {
                break;
            }
            }
        };
        
        //Label.
        GuiItem* label = new GuiItem();
        label->onDraw =
        [itemInfo, this] (const DrawInfo & draw) {
            drawText(
                itemInfo->text, game.sysContent.fntStandard,
                draw.center, draw.size,
                tintColor(mapAlpha(128), draw.tint)
            );
        };
        item->addChild(label);
        gui.addItem(
            label,
            oneAmount ? "mission_amount_1_label" : "mission_amount_2_label"
        );
        
        //First amount.
        GuiItem* amt1Text = new GuiItem();
        amt1Text->onDraw =
        [itemInfo, getAmounts, amt1Text] (const DrawInfo & draw) {
            static int oldAmt1 = INT_MAX;
            static int amt1 = 0;
            int dummy = 0;
            getAmounts(&amt1, &dummy);
            
            if(oldAmt1 != amt1) {
                amt1Text->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
                oldAmt1 = amt1;
            }
            float juicyGrowAmount = amt1Text->getJuiceValue();
            drawText(
                i2s(amt1), game.sysContent.fntCounter, draw.center, draw.size,
                draw.tint, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0 + juicyGrowAmount)
            );
        };
        item->addChild(amt1Text);
        gui.addItem(
            amt1Text,
            oneAmount ? "mission_amount_1_first" : "mission_amount_2_first"
        );
        
        if(!oneAmount) {
        
            //Second amount.
            GuiItem* amt2Text = new GuiItem();
            amt2Text->onDraw =
            [itemInfo, getAmounts, amt2Text] (const DrawInfo & draw) {
                static int oldAmt2 = INT_MAX;
                static int amt2 = 0;
                int dummy = 0;
                getAmounts(&dummy, &amt2);
                
                if(oldAmt2 != amt2) {
                    amt2Text->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
                    );
                    oldAmt2 = amt2;
                }
                float juicyGrowAmount = amt2Text->getJuiceValue();
                drawText(
                    i2s(amt2), game.sysContent.fntCounter,
                    draw.center, draw.size,
                    draw.tint, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                    Point(1.0 + juicyGrowAmount)
                );
            };
            item->addChild(amt2Text);
            gui.addItem(
                amt2Text, "mission_amount_2_second"
            );
            
        }
        
        break;
        
    }
    }
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
    
    //Update the control guide.
    bool playerIsIdling = false;
    if(
        player->leaderPtr &&
        player->leaderPtr->fsm.curState->id == LEADER_STATE_ACTIVE &&
        player->leaderPtr->anim.curAnim->name != "walking"
    ) {
        playerIsIdling = true;
    }
    
    if(playerIsIdling) {
        controlGuideActivityTimer += deltaT;
    } else {
        controlGuideActivityTimer = 0.0f;
    }
    if(controlGuideActivityTimer >= HUD::CONTROL_GUIDE_DELAY) {
        controlGuideOpacity += HUD::CONTROL_GUIDE_OPACITY_SPEED * deltaT;
    } else {
        controlGuideOpacity -= HUD::CONTROL_GUIDE_OPACITY_SPEED * deltaT;
    }
    controlGuideOpacity = std::clamp(controlGuideOpacity, 0.0f, 1.0f);
    
    //Tick the GUI items proper.
    gui.tick(game.deltaT);
}
