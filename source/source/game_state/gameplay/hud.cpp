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
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"
#include "gameplay.h"


using DrawInfo = GuiItem::DrawInfo;


namespace HUD {

//Dampen the mission goal indicator's movement by this much.
const float GOAL_INDICATOR_SMOOTHNESS_MULT = 5.5f;

//Name of the GUI information file.
const string GUI_FILE_NAME = "gameplay";

//How long the leader swap juice animation lasts for.
const float LEADER_SWAP_JUICE_DURATION = 0.7f;

//Standard mission score medal icon scale.
const float MEDAL_ICON_SCALE = 1.5f;

//Multiply time by this much to get the right scale animation amount.
const float MEDAL_ICON_SCALE_MULT = 0.3f;

//Multiply time by this much to get the right scale animation speed.
const float MEDAL_ICON_SCALE_TIME_MULT = 4.0f;

//Dampen the mission score indicator's movement by this much.
const float SCORE_INDICATOR_SMOOTHNESS_MULT = 5.5f;

//How many points to show before and after the mission score ruler flapper.
const int SCORE_RULER_RANGE = 125;

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
    
    DataNode* hud_file_node = &game.content.guiDefs.list[HUD::GUI_FILE_NAME];
    
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
    gui.readCoords(hud_file_node->getChildByName("positions"));
    
    //Leader health and icons.
    for(size_t l = 0; l < 3; l++) {
    
        //Icon.
        GuiItem* leader_icon = new GuiItem();
        leader_icon->onDraw =
        [this, l] (const DrawInfo & draw) {
            LeaderIconBubble icon;
            DrawInfo final_draw;
            game.states.gameplay->hud->leaderIconMgr.getDrawingInfo(
                l, &icon, &final_draw
            );
            
            if(!icon.bmp) return;
            
            al_draw_filled_circle(
                final_draw.center.x, final_draw.center.y,
                std::min(final_draw.size.x, final_draw.size.y) / 2.0f,
                changeAlpha(
                    icon.color,
                    128
                )
            );
            drawBitmapInBox(
                icon.bmp,
                final_draw.center, final_draw.size, true
            );
            drawBitmapInBox(
                bmpBubble,
                final_draw.center, final_draw.size, true
            );
        };
        gui.addItem(leader_icon, "leader_" + i2s(l + 1) + "_icon");
        leaderIconMgr.registerBubble(l, leader_icon);
        
        
        //Health wheel.
        GuiItem* leader_health = new GuiItem();
        leader_health->onDraw =
        [this, l] (const DrawInfo & draw) {
            LeaderHealthBubble health;
            DrawInfo final_draw;
            game.states.gameplay->hud->leaderHealthMgr.getDrawingInfo(
                l, &health, &final_draw
            );
            
            if(health.ratio <= 0.0f) return;
            
            if(health.cautionTimer > 0.0f) {
                float caution_ring_scale =
                    interpolateNumber(
                        health.cautionTimer,
                        0.0f, LEADER::HEALTH_CAUTION_RING_DURATION,
                        1.0f, 2.0f
                    );
                unsigned char caution_ring_alpha =
                    health.cautionTimer <
                    LEADER::HEALTH_CAUTION_RING_DURATION / 2.0f ?
                    interpolateNumber(
                        health.cautionTimer,
                        0.0f, LEADER::HEALTH_CAUTION_RING_DURATION / 2.0f,
                        0.0f, 192
                    ) :
                    interpolateNumber(
                        health.cautionTimer,
                        LEADER::HEALTH_CAUTION_RING_DURATION / 2.0f,
                        LEADER::HEALTH_CAUTION_RING_DURATION,
                        192, 0
                    );
                float caution_ring_size =
                    std::min(final_draw.size.x, final_draw.size.y) * caution_ring_scale;
                    
                drawBitmap(
                    game.sysContent.bmpBrightRing,
                    final_draw.center,
                    Point(caution_ring_size),
                    0.0f,
                    al_map_rgba(255, 0, 0, caution_ring_alpha)
                );
            }
            
            drawHealth(
                final_draw.center,
                health.ratio,
                1.0f,
                std::min(final_draw.size.x, final_draw.size.y) * 0.47f,
                true
            );
            drawBitmapInBox(
                bmpHardBubble,
                final_draw.center,
                final_draw.size,
                true
            );
        };
        gui.addItem(leader_health, "leader_" + i2s(l + 1) + "_health");
        leaderHealthMgr.registerBubble(l, leader_health);
        
    }
    
    
    //Next leader input.
    GuiItem* leader_next_input = new GuiItem();
    leader_next_input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(game.states.gameplay->availableLeaders.size() < 2) return;
        const PlayerInputSource &s =
            game.controls.findBind(PLAYER_ACTION_TYPE_NEXT_LEADER).
            inputSource;
        if(s.type == INPUT_SOURCE_TYPE_NONE) return;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size
        );
    };
    gui.addItem(leader_next_input, "leader_next_input");
    
    
    //Sun Meter.
    GuiItem* sun_meter = new GuiItem();
    sun_meter->onDraw =
    [this, sun_meter] (const DrawInfo & draw) {
        unsigned char n_hours =
            (
                game.config.misc.dayMinutesEnd -
                game.config.misc.dayMinutesStart
            ) / 60.0f;
        float day_length =
            game.config.misc.dayMinutesEnd - game.config.misc.dayMinutesStart;
        float day_passed_ratio =
            (
                game.states.gameplay->dayMinutes -
                game.config.misc.dayMinutesStart
            ) /
            (float) (day_length);
        float sun_radius = draw.size.y / 2.0;
        float first_dot_x = (draw.center.x - draw.size.x / 2.0) + sun_radius;
        float last_dot_x = (draw.center.x + draw.size.x / 2.0) - sun_radius;
        float dots_y = draw.center.y;
        //Width, from the center of the first dot to the center of the last.
        float dots_span = last_dot_x - first_dot_x;
        float dot_interval = dots_span / (float) n_hours;
        float sun_meter_sun_angle =
            game.states.gameplay->areaTimePassed *
            HUD::SUN_METER_SUN_SPIN_SPEED;
            
        //Larger bubbles at the start, middle and end of the meter.
        al_hold_bitmap_drawing(true);
        drawBitmap(
            bmpHardBubble,
            Point(first_dot_x + dots_span * 0.0, dots_y),
            Point(sun_radius * 0.9)
        );
        drawBitmap(
            bmpHardBubble,
            Point(first_dot_x + dots_span * 0.5, dots_y),
            Point(sun_radius * 0.9)
        );
        drawBitmap(
            bmpHardBubble,
            Point(first_dot_x + dots_span * 1.0, dots_y),
            Point(sun_radius * 0.9)
        );
        
        for(unsigned char h = 0; h < n_hours + 1; h++) {
            drawBitmap(
                bmpHardBubble,
                Point(first_dot_x + h * dot_interval, dots_y),
                Point(sun_radius * 0.6)
            );
        }
        al_hold_bitmap_drawing(false);
        
        Point sun_size =
            Point(sun_radius * 1.5) +
            sun_meter->getJuiceValue();
        //Static sun.
        drawBitmap(
            bmpSun,
            Point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            sun_size
        );
        //Spinning sun.
        drawBitmap(
            bmpSun,
            Point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            sun_size,
            sun_meter_sun_angle
        );
        //Bubble in front the sun.
        drawBitmap(
            bmpHardBubble,
            Point(first_dot_x + day_passed_ratio * dots_span, dots_y),
            sun_size,
            0.0f,
            al_map_rgb(255, 192, 128)
        );
    };
    sun_meter->onTick =
    [this, sun_meter] (float delta_t) {
        float day_length =
            game.config.misc.dayMinutesEnd - game.config.misc.dayMinutesStart;
        float pre_tick_day_minutes =
            game.states.gameplay->dayMinutes -
            game.curAreaData->dayTimeSpeed * delta_t / 60.0f;
        float post_tick_day_minutes =
            game.states.gameplay->dayMinutes;
        const float checkpoints[3] = {0.25f, 0.50f, 0.75f};
        for(unsigned char c = 0; c < 3; c++) {
            float checkpoint =
                game.config.misc.dayMinutesStart + day_length * checkpoints[c];
            if(
                pre_tick_day_minutes < checkpoint &&
                post_tick_day_minutes >= checkpoint
            ) {
                sun_meter->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_ICON
                );
                break;
            }
        }
    };
    gui.addItem(sun_meter, "time");
    
    
    //Day number bubble.
    GuiItem* day_bubble = new GuiItem();
    day_bubble->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmapInBox(bmpDayBubble, draw.center, draw.size, true);
    };
    gui.addItem(day_bubble, "day_bubble");
    
    
    //Day number text.
    GuiItem* day_nr = new GuiItem();
    day_nr->onDraw =
    [this] (const DrawInfo & draw) {
        drawText(
            i2s(game.states.gameplay->day),
            game.sysContent.fntCounter, draw.center,
            Point(draw.size.x * 0.70f, draw.size.y * 0.50f)
        );
    };
    gui.addItem(day_nr, "day_number");
    
    
    //Standby group member icon.
    GuiItem* standby_icon = new GuiItem();
    standby_icon->onDraw =
    [this] (const DrawInfo & draw) {
        game.states.gameplay->hud->drawStandbyIcon(BUBBLE_RELATION_CURRENT);
    };
    gui.addItem(standby_icon, "standby_icon");
    standbyIconMgr.registerBubble(BUBBLE_RELATION_CURRENT, standby_icon);
    
    
    //Next standby subgroup icon.
    GuiItem* standby_next_icon = new GuiItem();
    standby_next_icon->onDraw =
    [this] (const DrawInfo & draw) {
        game.states.gameplay->hud->drawStandbyIcon(BUBBLE_RELATION_NEXT);
    };
    gui.addItem(standby_next_icon, "standby_next_icon");
    standbyIconMgr.registerBubble(BUBBLE_RELATION_NEXT, standby_next_icon);
    
    
    //Next standby subgroup input.
    GuiItem* standby_next_input = new GuiItem();
    standby_next_input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!game.states.gameplay->curLeaderPtr) return;
        SubgroupType* next_type;
        game.states.gameplay->curLeaderPtr->group->getNextStandbyType(
            false, &next_type
        );
        if(
            next_type ==
            game.states.gameplay->curLeaderPtr->group->curStandbyType
        ) {
            return;
        }
        const PlayerInputSource &s =
            game.controls.findBind(PLAYER_ACTION_TYPE_NEXT_TYPE).
            inputSource;
        if(s.type == INPUT_SOURCE_TYPE_NONE) return;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            game.states.gameplay->hud->standbyItemsOpacity * 255
        );
    };
    gui.addItem(standby_next_input, "standby_next_input");
    
    
    //Previous standby subgroup icon.
    GuiItem* standby_prev_icon = new GuiItem();
    standby_prev_icon->onDraw =
    [this] (const DrawInfo & draw) {
        game.states.gameplay->hud->drawStandbyIcon(BUBBLE_RELATION_PREVIOUS);
    };
    gui.addItem(standby_prev_icon, "standby_prev_icon");
    standbyIconMgr.registerBubble(BUBBLE_RELATION_PREVIOUS, standby_prev_icon);
    
    
    //Previous standby subgroup input.
    GuiItem* standby_prev_input = new GuiItem();
    standby_prev_input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!game.states.gameplay->curLeaderPtr) return;
        SubgroupType* prev_type;
        game.states.gameplay->curLeaderPtr->group->getNextStandbyType(
            true, &prev_type
        );
        SubgroupType* next_type;
        game.states.gameplay->curLeaderPtr->group->getNextStandbyType(
            false, &next_type
        );
        if(
            prev_type ==
            game.states.gameplay->curLeaderPtr->group->curStandbyType ||
            prev_type == next_type
        ) {
            return;
        }
        const PlayerInputSource s =
            game.controls.findBind(PLAYER_ACTION_TYPE_PREV_TYPE).
            inputSource;
        if(s.type == INPUT_SOURCE_TYPE_NONE) return;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            game.states.gameplay->hud->standbyItemsOpacity * 255
        );
    };
    gui.addItem(standby_prev_input, "standby_prev_input");
    
    
    //Standby group member maturity.
    GuiItem* standby_maturity_icon = new GuiItem();
    standby_maturity_icon->onDraw =
    [this, standby_maturity_icon] (const DrawInfo & draw) {
        //Standby group member preparations.
        Leader* l_ptr = game.states.gameplay->curLeaderPtr;
        if(!l_ptr || !l_ptr->group) return;
        
        ALLEGRO_BITMAP* standby_mat_bmp = nullptr;
        Mob* closest =
            game.states.gameplay->closestGroupMember[BUBBLE_RELATION_CURRENT];
            
        if(l_ptr->group->curStandbyType && closest) {
            SUBGROUP_TYPE_CATEGORY c =
                l_ptr->group->curStandbyType->getCategory();
                
            switch(c) {
            case SUBGROUP_TYPE_CATEGORY_PIKMIN: {
                Pikmin* p_ptr =
                    dynamic_cast<Pikmin*>(closest);
                standby_mat_bmp =
                    p_ptr->pikType->bmpMaturityIcon[p_ptr->maturity];
                break;
                
            } default: {
                break;
                
            }
            }
        }
        
        ALLEGRO_COLOR color =
            mapAlpha(game.states.gameplay->hud->standbyItemsOpacity * 255);
            
        if(standby_mat_bmp) {
            drawBitmapInBox(
                standby_mat_bmp, draw.center,
                (draw.size * 0.8) + standby_maturity_icon->getJuiceValue(),
                true,
                0.0f, color
            );
            drawBitmapInBox(
                bmpBubble, draw.center,
                draw.size + standby_maturity_icon->getJuiceValue(),
                true, 0.0f, color
            );
        }
        
        if(
            l_ptr->group->curStandbyType != prevStandbyType ||
            standby_mat_bmp != prevMaturityIcon
        ) {
            standby_maturity_icon->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_ICON
            );
            prevStandbyType = l_ptr->group->curStandbyType;
            prevMaturityIcon = standby_mat_bmp;
        }
    };
    gui.addItem(standby_maturity_icon, "standby_maturity_icon");
    
    
    //Standby subgroup member amount bubble.
    GuiItem* standby_bubble = new GuiItem();
    standby_bubble->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            bmpCounterBubbleStandby,
            draw.center, draw.size,
            0.0f,
            mapAlpha(game.states.gameplay->hud->standbyItemsOpacity * 255)
        );
    };
    gui.addItem(standby_bubble, "standby_bubble");
    
    
    //Standby subgroup member amount.
    standbyAmount = new GuiItem();
    standbyAmount->onDraw =
    [this] (const DrawInfo & draw) {
        size_t n_standby_pikmin = 0;
        Leader* l_ptr = game.states.gameplay->curLeaderPtr;
        
        if(l_ptr && l_ptr->group->curStandbyType) {
            for(size_t m = 0; m < l_ptr->group->members.size(); m++) {
                Mob* m_ptr = l_ptr->group->members[m];
                if(m_ptr->subgroupTypePtr == l_ptr->group->curStandbyType) {
                    n_standby_pikmin++;
                }
            }
        }
        
        if(n_standby_pikmin != standbyCountNr) {
            standbyAmount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            standbyCountNr = n_standby_pikmin;
        }
        
        drawText(
            i2s(n_standby_pikmin), game.sysContent.fntCounter,
            draw.center, draw.size,
            mapAlpha(game.states.gameplay->hud->standbyItemsOpacity * 255),
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + standbyAmount->getJuiceValue())
        );
    };
    gui.addItem(standbyAmount, "standby_amount");
    
    
    //Group Pikmin amount bubble.
    GuiItem* group_bubble = new GuiItem();
    group_bubble->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.states.gameplay->curLeaderPtr) return;
        drawBitmap(
            bmpCounterBubbleGroup,
            draw.center, draw.size
        );
    };
    gui.addItem(group_bubble, "group_bubble");
    
    
    //Group Pikmin amount.
    groupAmount = new GuiItem();
    groupAmount->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.states.gameplay->curLeaderPtr) return;
        size_t cur_amount = game.states.gameplay->getAmountOfGroupPikmin();
        
        if(cur_amount != groupCountNr) {
            groupAmount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            groupCountNr = cur_amount;
        }
        
        drawText(
            i2s(cur_amount), game.sysContent.fntCounter,
            draw.center, Point(draw.size.x * 0.70f, draw.size.y * 0.50f), COLOR_WHITE,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + groupAmount->getJuiceValue())
        );
    };
    gui.addItem(groupAmount, "group_amount");
    
    
    //Field Pikmin amount bubble.
    GuiItem* field_bubble = new GuiItem();
    field_bubble->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            bmpCounterBubbleField,
            draw.center, draw.size
        );
    };
    gui.addItem(field_bubble, "field_bubble");
    
    
    //Field Pikmin amount.
    fieldAmount = new GuiItem();
    fieldAmount->onDraw =
    [this] (const DrawInfo & draw) {
        size_t cur_amount = game.states.gameplay->getAmountOfFieldPikmin();
        
        if(cur_amount != fieldCountNr) {
            fieldAmount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            fieldCountNr = cur_amount;
        }
        
        drawText(
            i2s(cur_amount), game.sysContent.fntCounter,
            draw.center, Point(draw.size.x * 0.70f, draw.size.y * 0.50f), COLOR_WHITE,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + fieldAmount->getJuiceValue())
        );
    };
    gui.addItem(fieldAmount, "field_amount");
    
    
    //Total Pikmin amount bubble.
    GuiItem* total_bubble = new GuiItem();
    total_bubble->onDraw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            bmpCounterBubbleTotal,
            draw.center, draw.size
        );
    };
    gui.addItem(total_bubble, "total_bubble");
    
    
    //Total Pikmin amount.
    totalAmount = new GuiItem();
    totalAmount->onDraw =
    [this] (const DrawInfo & draw) {
        size_t cur_amount = game.states.gameplay->getAmountOfTotalPikmin();
        
        if(cur_amount != totalCountNr) {
            totalAmount->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            totalCountNr = cur_amount;
        }
        
        drawText(
            i2s(totalCountNr), game.sysContent.fntCounter,
            draw.center, Point(draw.size.x * 0.70f, draw.size.y * 0.50f), COLOR_WHITE,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + totalAmount->getJuiceValue())
        );
    };
    gui.addItem(totalAmount, "total_amount");
    
    
    //Pikmin counter "x".
    GuiItem* counters_x = new GuiItem();
    counters_x->onDraw =
    [this] (const DrawInfo & draw) {
        drawText(
            "x", game.sysContent.fntCounter, draw.center, draw.size,
            mapAlpha(game.states.gameplay->hud->standbyItemsOpacity * 255)
        );
    };
    gui.addItem(counters_x, "counters_x");
    
    
    //Pikmin counter slashes.
    for(size_t s = 0; s < 3; s++) {
        GuiItem* counter_slash = new GuiItem();
        counter_slash->onDraw =
        [this] (const DrawInfo & draw) {
            if(!game.states.gameplay->curLeaderPtr) return;
            drawText(
                "/", game.sysContent.fntCounter, draw.center, draw.size
            );
        };
        gui.addItem(counter_slash, "counters_slash_" + i2s(s + 1));
    }
    
    
    //Spray 1 icon.
    GuiItem* spray_1_icon = new GuiItem();
    spray_1_icon->onDraw =
    [this] (const DrawInfo & draw) {
        drawSprayIcon(BUBBLE_RELATION_CURRENT);
    };
    gui.addItem(spray_1_icon, "spray_1_icon");
    sprayIconMgr.registerBubble(BUBBLE_RELATION_CURRENT, spray_1_icon);
    
    
    //Spray 1 amount.
    spray1Amount = new GuiItem();
    spray1Amount->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.states.gameplay->curLeaderPtr) return;
        
        size_t top_spray_idx = INVALID;
        if(game.content.sprayTypes.list.size() > 2) {
            top_spray_idx = game.states.gameplay->selectedSpray;
        } else if(!game.content.sprayTypes.list.empty() && game.content.sprayTypes.list.size() <= 2) {
            top_spray_idx = 0;
        }
        if(top_spray_idx == INVALID) return;
        
        drawText(
            "x" +
            i2s(game.states.gameplay->sprayStats[top_spray_idx].nrSprays),
            game.sysContent.fntCounter,
            Point(draw.center.x - draw.size.x / 2.0, draw.center.y), draw.size,
            mapAlpha(game.states.gameplay->hud->sprayItemsOpacity * 255),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + spray1Amount->getJuiceValue())
        );
    };
    gui.addItem(spray1Amount, "spray_1_amount");
    
    
    //Spray 1 input.
    GuiItem* spray_1_input = new GuiItem();
    spray_1_input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!game.states.gameplay->curLeaderPtr) return;
        
        size_t top_spray_idx = INVALID;
        if(game.content.sprayTypes.list.size() > 2) {
            top_spray_idx = game.states.gameplay->selectedSpray;
        } else if(!game.content.sprayTypes.list.empty() && game.content.sprayTypes.list.size() <= 2) {
            top_spray_idx = 0;
        }
        if(top_spray_idx == INVALID) return;
        if(game.states.gameplay->sprayStats[top_spray_idx].nrSprays == 0) {
            return;
        }
        
        PlayerInputSource s;
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
        if(s.type == INPUT_SOURCE_TYPE_NONE) return;
        
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            game.states.gameplay->hud->sprayItemsOpacity * 255
        );
    };
    gui.addItem(spray_1_input, "spray_1_input");
    
    
    //Spray 2 icon.
    GuiItem* spray_2_icon = new GuiItem();
    spray_2_icon->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.states.gameplay->curLeaderPtr) return;
        
        size_t bottom_spray_idx = INVALID;
        if(game.content.sprayTypes.list.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        
        drawBitmapInBox(
            game.config.misc.sprayOrder[bottom_spray_idx]->bmpSpray,
            draw.center, draw.size, true,
            0.0f,
            mapAlpha(game.states.gameplay->hud->sprayItemsOpacity * 255)
        );
    };
    gui.addItem(spray_2_icon, "spray_2_icon");
    
    
    //Spray 2 amount.
    spray2Amount = new GuiItem();
    spray2Amount->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.states.gameplay->curLeaderPtr) return;
        
        size_t bottom_spray_idx = INVALID;
        if(game.content.sprayTypes.list.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        
        drawText(
            "x" +
            i2s(game.states.gameplay->sprayStats[bottom_spray_idx].nrSprays),
            game.sysContent.fntCounter,
            Point(draw.center.x - draw.size.x / 2.0, draw.center.y), draw.size,
            mapAlpha(game.states.gameplay->hud->sprayItemsOpacity * 255),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + spray2Amount->getJuiceValue())
        );
    };
    gui.addItem(spray2Amount, "spray_2_amount");
    
    
    //Spray 2 input.
    GuiItem* spray_2_input = new GuiItem();
    spray_2_input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!game.states.gameplay->curLeaderPtr) return;
        
        size_t bottom_spray_idx = INVALID;
        if(game.content.sprayTypes.list.size() == 2) {
            bottom_spray_idx = 1;
        }
        if(bottom_spray_idx == INVALID) return;
        if(game.states.gameplay->sprayStats[bottom_spray_idx].nrSprays == 0) {
            return;
        }
        
        PlayerInputSource s;
        if(game.content.sprayTypes.list.size() == 2) {
            s =
                game.controls.findBind(PLAYER_ACTION_TYPE_USE_SPRAY_2).
                inputSource;
        }
        if(s.type == INPUT_SOURCE_TYPE_NONE) return;
        
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            game.states.gameplay->hud->sprayItemsOpacity * 255
        );
    };
    gui.addItem(spray_2_input, "spray_2_input");
    
    
    //Previous spray icon.
    GuiItem* prev_spray_icon = new GuiItem();
    prev_spray_icon->onDraw =
    [this] (const DrawInfo & draw) {
        drawSprayIcon(BUBBLE_RELATION_PREVIOUS);
    };
    gui.addItem(prev_spray_icon, "spray_prev_icon");
    sprayIconMgr.registerBubble(BUBBLE_RELATION_PREVIOUS, prev_spray_icon);
    
    
    //Previous spray input.
    GuiItem* prev_spray_input = new GuiItem();
    prev_spray_input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!game.states.gameplay->curLeaderPtr) return;
        
        size_t prev_spray_idx = INVALID;
        if(game.content.sprayTypes.list.size() >= 3) {
            prev_spray_idx =
                sumAndWrap(
                    (int) game.states.gameplay->selectedSpray,
                    -1,
                    (int) game.content.sprayTypes.list.size()
                );
        }
        if(prev_spray_idx == INVALID) return;
        
        PlayerInputSource s;
        if(game.content.sprayTypes.list.size() >= 3) {
            s =
                game.controls.findBind(PLAYER_ACTION_TYPE_PREV_SPRAY).
                inputSource;
        }
        if(s.type == INPUT_SOURCE_TYPE_NONE) return;
        
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            game.states.gameplay->hud->sprayItemsOpacity * 255
        );
    };
    gui.addItem(prev_spray_input, "spray_prev_input");
    
    
    //Next spray icon.
    GuiItem* next_spray_icon = new GuiItem();
    next_spray_icon->onDraw =
    [this] (const DrawInfo & draw) {
        drawSprayIcon(BUBBLE_RELATION_NEXT);
    };
    gui.addItem(next_spray_icon, "spray_next_icon");
    sprayIconMgr.registerBubble(BUBBLE_RELATION_NEXT, next_spray_icon);
    
    
    //Next spray input.
    GuiItem* next_spray_input = new GuiItem();
    next_spray_input->onDraw =
    [this] (const DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        if(!game.states.gameplay->curLeaderPtr) return;
        
        size_t next_spray_idx = INVALID;
        if(game.content.sprayTypes.list.size() >= 3) {
            next_spray_idx =
                sumAndWrap(
                    (int) game.states.gameplay->selectedSpray,
                    1,
                    (int) game.content.sprayTypes.list.size()
                );
        }
        if(next_spray_idx == INVALID) return;
        
        PlayerInputSource s;
        if(game.content.sprayTypes.list.size() >= 3) {
            s =
                game.controls.findBind(PLAYER_ACTION_TYPE_NEXT_SPRAY).
                inputSource;
        }
        if(s.type == INPUT_SOURCE_TYPE_NONE) return;
        
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size,
            game.states.gameplay->hud->sprayItemsOpacity * 255
        );
    };
    gui.addItem(next_spray_input, "spray_next_input");
    
    
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
    
        //Mission goal bubble.
        GuiItem* mission_goal_bubble = new GuiItem();
        mission_goal_bubble->onDraw =
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
        gui.addItem(mission_goal_bubble, "mission_goal_bubble");
        
        
        string goal_cur_label_text =
            game.missionGoals[game.curAreaData->mission.goal]->
            getHudLabel();
            
        if(!goal_cur_label_text.empty()) {
            //Mission goal current label.
            GuiItem* mission_goal_cur_label = new GuiItem();
            mission_goal_cur_label->onDraw =
                [this, goal_cur_label_text]
            (const DrawInfo & draw) {
                drawText(
                    goal_cur_label_text, game.sysContent.fntStandard,
                    draw.center, draw.size, al_map_rgba(255, 255, 255, 128)
                );
            };
            gui.addItem(mission_goal_cur_label, "mission_goal_cur_label");
            
            
            //Mission goal current.
            GuiItem* mission_goal_cur = new GuiItem();
            mission_goal_cur->onDraw =
                [this, mission_goal_cur]
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
                float juicy_grow_amount =
                    mission_goal_cur->getJuiceValue();
                drawText(
                    text, game.sysContent.fntCounter, draw.center, draw.size,
                    COLOR_WHITE, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                    Point(1.0 + juicy_grow_amount)
                );
            };
            gui.addItem(mission_goal_cur, "mission_goal_cur");
            game.states.gameplay->missionGoalCurText = mission_goal_cur;
            
            
            //Mission goal requirement label.
            GuiItem* mission_goal_req_label = new GuiItem();
            mission_goal_req_label->onDraw =
            [this] (const DrawInfo & draw) {
                drawText(
                    "Goal", game.sysContent.fntStandard, draw.center, draw.size,
                    al_map_rgba(255, 255, 255, 128)
                );
            };
            gui.addItem(mission_goal_req_label, "mission_goal_req_label");
            
            
            //Mission goal requirement.
            GuiItem* mission_goal_req = new GuiItem();
            mission_goal_req->onDraw =
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
            gui.addItem(mission_goal_req, "mission_goal_req");
            
            
            //Mission goal slash.
            GuiItem* mission_goal_slash = new GuiItem();
            mission_goal_slash->onDraw =
            [this] (const DrawInfo & draw) {
                drawText(
                    "/", game.sysContent.fntCounter, draw.center, draw.size
                );
            };
            gui.addItem(mission_goal_slash, "mission_goal_slash");
            
        } else {
        
            //Mission goal name text.
            GuiItem* mission_goal_name = new GuiItem();
            mission_goal_name->onDraw =
            [this] (const DrawInfo & draw) {
                drawText(
                    game.missionGoals[game.curAreaData->mission.goal]->
                    getName(), game.sysContent.fntStandard,
                    draw.center, draw.size, al_map_rgba(255, 255, 255, 128)
                );
            };
            gui.addItem(mission_goal_name, "mission_goal_name");
            
        }
        
    }
    
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.gradingMode == MISSION_GRADING_MODE_POINTS &&
        game.curAreaData->mission.pointHudData != 0
    ) {
    
        //Mission score bubble.
        GuiItem* mission_score_bubble = new GuiItem();
        mission_score_bubble->onDraw =
        [this] (const DrawInfo & draw) {
            drawFilledRoundedRectangle(
                draw.center, draw.size, 20.0f, al_map_rgba(86, 149, 50, 160)
            );
            drawTexturedBox(
                draw.center, draw.size, game.sysContent.bmpBubbleBox,
                al_map_rgba(255, 255, 255, 200)
            );
        };
        gui.addItem(mission_score_bubble, "mission_score_bubble");
        
        
        //Mission score "score" label.
        GuiItem* mission_score_score_label = new GuiItem();
        mission_score_score_label->onDraw =
        [this] (const DrawInfo & draw) {
            drawText(
                "Score:", game.sysContent.fntStandard,
                Point(draw.center.x + draw.size.x / 2.0f, draw.center.y), draw.size,
                al_map_rgba(255, 255, 255, 128), ALLEGRO_ALIGN_RIGHT
            );
        };
        gui.addItem(mission_score_score_label, "mission_score_score_label");
        
        
        //Mission score points.
        GuiItem* mission_score_points = new GuiItem();
        mission_score_points->onDraw =
            [this, mission_score_points]
        (const DrawInfo & draw) {
            float juicy_grow_amount = mission_score_points->getJuiceValue();
            drawText(
                i2s(game.states.gameplay->missionScore),
                game.sysContent.fntCounter, draw.center, draw.size, COLOR_WHITE,
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0 + juicy_grow_amount)
            );
        };
        gui.addItem(mission_score_points, "mission_score_points");
        game.states.gameplay->missionScoreCurText = mission_score_points;
        
        
        //Mission score "points" label.
        GuiItem* mission_score_points_label = new GuiItem();
        mission_score_points_label->onDraw =
        [this] (const DrawInfo & draw) {
            drawText(
                "pts", game.sysContent.fntStandard,
                Point(draw.center.x + draw.size.x / 2.0f, draw.center.y), draw.size,
                al_map_rgba(255, 255, 255, 128), ALLEGRO_ALIGN_RIGHT
            );
        };
        gui.addItem(mission_score_points_label, "mission_score_points_label");
        
        
        //Mission score ruler.
        GuiItem* mission_score_ruler = new GuiItem();
        mission_score_ruler->onDraw =
        [this] (const DrawInfo & draw) {
            //Setup.
            const float ruler_start_value =
                game.states.gameplay->scoreIndicator -
                HUD::SCORE_RULER_RANGE / 2.0f;
            const float ruler_end_value =
                game.states.gameplay->scoreIndicator +
                HUD::SCORE_RULER_RANGE / 2.0f;
            const float ruler_scale =
                draw.size.x / (float) HUD::SCORE_RULER_RANGE;
            const float ruler_start_x = draw.center.x - draw.size.x / 2.0f;
            const float ruler_end_x = draw.center.x + draw.size.x / 2.0f;
            
            const float seg_limits[] = {
                std::min(ruler_start_value, 0.0f),
                0,
                (float) game.curAreaData->mission.bronzeReq,
                (float) game.curAreaData->mission.silverReq,
                (float) game.curAreaData->mission.goldReq,
                (float) game.curAreaData->mission.platinumReq,
                std::max(
                    ruler_end_value,
                    (float) game.curAreaData->mission.platinumReq
                )
            };
            const ALLEGRO_COLOR seg_colors_top[] = {
                al_map_rgba(152, 160, 152, 128), //Negatives.
                al_map_rgb(158, 166, 158),       //No medal.
                al_map_rgb(203, 117, 37),        //Bronze.
                al_map_rgb(223, 227, 209),       //Silver.
                al_map_rgb(235, 209, 59),        //Gold.
                al_map_rgb(158, 222, 211)        //Platinum.
            };
            const ALLEGRO_COLOR seg_colors_bottom[] = {
                al_map_rgba(152, 160, 152, 128), //Negatives.
                al_map_rgb(119, 128, 118),       //No medal.
                al_map_rgb(131, 52, 18),         //Bronze.
                al_map_rgb(160, 154, 127),       //Silver.
                al_map_rgb(173, 127, 24),        //Gold.
                al_map_rgb(79, 172, 153)         //Platinum.
            };
            ALLEGRO_BITMAP* seg_icons[] = {
                nullptr,
                nullptr,
                game.sysContent.bmpMedalBronze,
                game.sysContent.bmpMedalSilver,
                game.sysContent.bmpMedalGold,
                game.sysContent.bmpMedalPlatinum
            };
            
            //Draw each segment (no medal, bronze, etc.).
            for(int s = 0; s < 6; s++) {
                float seg_start_value = seg_limits[s];
                float seg_end_value = seg_limits[s + 1];
                if(seg_end_value < ruler_start_value) continue;
                if(seg_start_value > ruler_end_value) continue;
                float seg_start_x =
                    draw.center.x -
                    (game.states.gameplay->scoreIndicator - seg_start_value) *
                    ruler_scale;
                float seg_end_x =
                    draw.center.x +
                    (seg_end_value - game.states.gameplay->scoreIndicator) *
                    ruler_scale;
                seg_start_x = std::max(seg_start_x, ruler_start_x);
                seg_end_x = std::min(ruler_end_x, seg_end_x);
                
                ALLEGRO_VERTEX vertexes[4];
                for(unsigned char v = 0; v < 4; v++) {
                    vertexes[v].z = 0.0f;
                }
                vertexes[0].x = seg_start_x;
                vertexes[0].y = draw.center.y - draw.size.y / 2.0f;
                vertexes[0].color = seg_colors_top[s];
                vertexes[1].x = seg_start_x;
                vertexes[1].y = draw.center.y + draw.size.y / 2.0f;
                vertexes[1].color = seg_colors_bottom[s];
                vertexes[2].x = seg_end_x;
                vertexes[2].y = draw.center.y + draw.size.y / 2.0f;
                vertexes[2].color = seg_colors_bottom[s];
                vertexes[3].x = seg_end_x;
                vertexes[3].y = draw.center.y - draw.size.y / 2.0f;
                vertexes[3].color = seg_colors_top[s];
                al_draw_prim(
                    vertexes, nullptr, nullptr, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            //Draw the markings.
            for(
                float m = floor(ruler_start_value / 25.0f) * 25.0f;
                m <= ruler_end_value;
                m += 25.0f
            ) {
                if(m < 0.0f || m < ruler_start_value) continue;
                float marking_x =
                    draw.center.x -
                    (game.states.gameplay->scoreIndicator - m) *
                    ruler_scale;
                float marking_length =
                    fmod(m, 100) == 0 ?
                    draw.size.y * 0.7f :
                    fmod(m, 50) == 0 ?
                    draw.size.y * 0.4f :
                    draw.size.y * 0.1f;
                al_draw_filled_triangle(
                    marking_x,
                    draw.center.y - draw.size.y / 2.0f + marking_length,
                    marking_x + 2.0f,
                    draw.center.y - draw.size.y / 2.0f,
                    marking_x - 2.0f,
                    draw.center.y - draw.size.y / 2.0f,
                    al_map_rgb(100, 110, 180)
                );
            }
            
            //Draw the medal icons.
            int cur_seg = 0;
            int last_passed_seg = 0;
            float cur_medal_scale =
                HUD::MEDAL_ICON_SCALE +
                sin(
                    game.states.gameplay->areaTimePassed *
                    HUD::MEDAL_ICON_SCALE_TIME_MULT
                ) *
                HUD::MEDAL_ICON_SCALE_MULT;
            for(int s = 0; s < 6; s++) {
                float seg_start_value = seg_limits[s];
                if(seg_start_value <= game.states.gameplay->scoreIndicator) {
                    cur_seg = s;
                }
                if(seg_start_value <= ruler_start_value) {
                    last_passed_seg = s;
                }
            }
            for(int s = 0; s < 6; s++) {
                if(!seg_icons[s]) continue;
                float seg_start_value = seg_limits[s];
                if(seg_start_value < ruler_start_value) continue;
                float seg_start_x =
                    draw.center.x -
                    (game.states.gameplay->scoreIndicator - seg_start_value) *
                    ruler_scale;
                float icon_x = seg_start_x;
                unsigned char icon_alpha = 255;
                float icon_scale = HUD::MEDAL_ICON_SCALE;
                if(cur_seg == s) {
                    icon_scale = cur_medal_scale;
                }
                if(seg_start_value > ruler_end_value) {
                    icon_x = ruler_end_x;
                    icon_alpha = 128;
                }
                drawBitmap(
                    seg_icons[s],
                    Point(icon_x, draw.center.y),
                    Point(-1, draw.size.y * icon_scale),
                    0,
                    al_map_rgba(255, 255, 255, icon_alpha)
                );
                if(seg_start_value > ruler_end_value) {
                    //If we found the first icon that goes past the ruler's end,
                    //then we shouldn't draw the other ones that come after.
                    break;
                }
            }
            if(seg_icons[last_passed_seg] && last_passed_seg == cur_seg) {
                drawBitmap(
                    seg_icons[last_passed_seg],
                    Point(ruler_start_x, draw.center.y),
                    Point(-1, draw.size.y * cur_medal_scale)
                );
            }
            
            //Draw the flapper.
            al_draw_filled_triangle(
                draw.center.x, draw.center.y + draw.size.y / 2.0f,
                draw.center.x, draw.center.y,
                draw.center.x + (draw.size.y * 0.4), draw.center.y + draw.size.y / 2.0f,
                al_map_rgb(105, 161, 105)
            );
            al_draw_filled_triangle(
                draw.center.x, draw.center.y + draw.size.y / 2.0f,
                draw.center.x, draw.center.y,
                draw.center.x - (draw.size.y * 0.4), draw.center.y + draw.size.y / 2.0f,
                al_map_rgb(124, 191, 124)
            );
        };
        gui.addItem(mission_score_ruler, "mission_score_ruler");
        
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
    
    
    DataNode* bitmaps_node = hud_file_node->getChildByName("files");
    
#define loader(var, name) \
    var = \
          game.content.bitmaps.list.get( \
                                         bitmaps_node->getChildByName(name)->value, \
                                         bitmaps_node->getChildByName(name) \
                                       );
    
    loader(bmpBubble,                 "bubble");
    loader(bmpCounterBubbleField,   "counter_bubble_field");
    loader(bmpCounterBubbleGroup,   "counter_bubble_group");
    loader(bmpCounterBubbleStandby, "counter_bubble_standby");
    loader(bmpCounterBubbleTotal,   "counter_bubble_total");
    loader(bmpDayBubble,             "day_bubble");
    loader(bmpDistantPikminMarker,  "distant_pikmin_marker");
    loader(bmpHardBubble,            "hard_bubble");
    loader(bmpNoPikminBubble,       "no_pikmin_bubble");
    loader(bmpSun,                    "sun");
    
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
    GuiItem* mission_fail_bubble = new GuiItem();
    mission_fail_bubble->onDraw =
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
        mission_fail_bubble,
        primary ?
        "mission_fail_1_bubble" :
        "mission_fail_2_bubble"
    );
    
    
    if(game.missionFailConds[cond]->hasHudContent()) {
    
        //Mission fail condition current label.
        GuiItem* mission_fail_cur_label = new GuiItem();
        mission_fail_cur_label->onDraw =
        [this, cond] (const DrawInfo & draw) {
            drawText(
                game.missionFailConds[cond]->
                getHudLabel(game.states.gameplay),
                game.sysContent.fntStandard, draw.center, draw.size,
                al_map_rgba(255, 255, 255, 128)
            );
        };
        gui.addItem(
            mission_fail_cur_label,
            primary ?
            "mission_fail_1_cur_label" :
            "mission_fail_2_cur_label"
        );
        
        
        //Mission fail condition current.
        GuiItem* mission_fail_cur = new GuiItem();
        mission_fail_cur->onDraw =
            [this, cond, mission_fail_cur]
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
            float juicy_grow_amount = mission_fail_cur->getJuiceValue();
            drawText(
                text, game.sysContent.fntCounter, draw.center, draw.size,
                COLOR_WHITE, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0 + juicy_grow_amount)
            );
        };
        gui.addItem(
            mission_fail_cur,
            primary ?
            "mission_fail_1_cur" :
            "mission_fail_2_cur"
        );
        if(primary) {
            game.states.gameplay->missionFail1CurText = mission_fail_cur;
        } else {
            game.states.gameplay->missionFail2CurText = mission_fail_cur;
        }
        
        
        //Mission fail condition requirement label.
        GuiItem* mission_fail_req_label = new GuiItem();
        mission_fail_req_label->onDraw =
            [this]
        (const DrawInfo & draw) {
            drawText(
                "Fail", game.sysContent.fntStandard, draw.center, draw.size,
                al_map_rgba(255, 255, 255, 128)
            );
        };
        gui.addItem(
            mission_fail_req_label,
            primary ?
            "mission_fail_1_req_label" :
            "mission_fail_2_req_label"
        );
        
        
        //Mission fail condition requirement.
        GuiItem* mission_fail_req = new GuiItem();
        mission_fail_req->onDraw =
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
            mission_fail_req,
            primary ?
            "mission_fail_1_req" :
            "mission_fail_2_req"
        );
        
        
        //Mission primary fail condition slash.
        GuiItem* mission_fail_slash = new GuiItem();
        mission_fail_slash->onDraw =
        [this] (const DrawInfo & draw) {
            drawText(
                "/", game.sysContent.fntCounter, draw.center, draw.size
            );
        };
        gui.addItem(
            mission_fail_slash,
            primary ?
            "mission_fail_1_slash" :
            "mission_fail_2_slash"
        );
        
    } else {
    
        //Mission fail condition name text.
        GuiItem* mission_fail_name = new GuiItem();
        mission_fail_name->onDraw =
        [this, cond] (const DrawInfo & draw) {
            drawText(
                "Fail: " +
                game.missionFailConds[cond]->getName(),
                game.sysContent.fntStandard, draw.center, draw.size,
                al_map_rgba(255, 255, 255, 128)
            );
        };
        gui.addItem(
            mission_fail_name,
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
    if(!game.states.gameplay->curLeaderPtr) return;
    
    DrawInfo draw;
    ALLEGRO_BITMAP* icon;
    game.states.gameplay->hud->sprayIconMgr.getDrawingInfo(
        which, &icon, &draw
    );
    
    if(!icon) return;
    drawBitmapInBox(
        icon, draw.center, draw.size, true, 0.0f,
        mapAlpha(game.states.gameplay->hud->sprayItemsOpacity * 255)
    );
}


/**
 * @brief Code to draw a standby icon with.
 *
 * @param which Which standby icon to draw -- the previous type's,
 * the current type's, or the next type's.
 */
void Hud::drawStandbyIcon(BUBBLE_RELATION which) {
    DrawInfo draw;
    ALLEGRO_BITMAP* icon;
    game.states.gameplay->hud->standbyIconMgr.getDrawingInfo(
        which, &icon, &draw
    );
    
    if(!icon) return;
    
    ALLEGRO_COLOR color =
        mapAlpha(game.states.gameplay->hud->standbyItemsOpacity * 255);
        
    drawBitmapInBox(icon, draw.center, draw.size * 0.8, true, 0.0f, color);
    
    if(
        game.states.gameplay->closestGroupMemberDistant &&
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
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Hud::tick(float delta_t) {
    //Update leader bubbles.
    for(size_t l = 0; l < 3; l++) {
        Leader* l_ptr = nullptr;
        if(l < game.states.gameplay->availableLeaders.size()) {
            size_t l_idx =
                (size_t) sumAndWrap(
                    (int) game.states.gameplay->curLeaderIdx,
                    (int) l,
                    (int) game.states.gameplay->availableLeaders.size()
                );
            l_ptr = game.states.gameplay->availableLeaders[l_idx];
        }
        
        LeaderIconBubble icon;
        icon.bmp = nullptr;
        icon.color = COLOR_EMPTY;
        if(l_ptr) {
            icon.bmp = l_ptr->leaType->bmpIcon;
            icon.color = l_ptr->leaType->mainColor;
        }
        
        leaderIconMgr.update(l, l_ptr, icon);
        
        LeaderHealthBubble health;
        health.ratio = 0.0f;
        health.cautionTimer = 0.0f;
        if(l_ptr) {
            health.ratio = l_ptr->healthWheelVisibleRatio;
            health.cautionTimer = l_ptr->healthWheelCautionTimer;
        }
        leaderHealthMgr.update(l, l_ptr, health);
    }
    leaderIconMgr.tick(delta_t);
    leaderHealthMgr.tick(delta_t);
    
    //Update standby bubbles.
    for(unsigned char s = 0; s < 3; s++) {
    
        ALLEGRO_BITMAP* icon = nullptr;
        Leader* cur_leader_ptr = game.states.gameplay->curLeaderPtr;
        Mob* member = game.states.gameplay->closestGroupMember[s];
        SubgroupType* type = nullptr;
        
        if(cur_leader_ptr) {
            switch(s) {
            case BUBBLE_RELATION_PREVIOUS: {
                SubgroupType* prev_type;
                cur_leader_ptr->group->getNextStandbyType(true, &prev_type);
                SubgroupType* next_type;
                cur_leader_ptr->group->getNextStandbyType(false, &next_type);
                if(
                    prev_type != cur_leader_ptr->group->curStandbyType &&
                    prev_type != next_type
                ) {
                    type = prev_type;
                }
                break;
            }
            case BUBBLE_RELATION_CURRENT: {
                type = cur_leader_ptr->group->curStandbyType;
                break;
            }
            case BUBBLE_RELATION_NEXT: {
                SubgroupType* next_type;
                cur_leader_ptr->group->getNextStandbyType(false, &next_type);
                if(next_type != cur_leader_ptr->group->curStandbyType) {
                    type = next_type;
                }
                break;
            }
            }
        }
        
        if(cur_leader_ptr && type && member) {
            SUBGROUP_TYPE_CATEGORY cat = type->getCategory();
            
            switch(cat) {
            case SUBGROUP_TYPE_CATEGORY_LEADER: {
                Leader* l_ptr = dynamic_cast<Leader*>(member);
                icon = l_ptr->leaType->bmpIcon;
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
    standbyIconMgr.tick(delta_t);
    
    //Update spray bubbles.
    size_t top_spray_idx = INVALID;
    if(game.content.sprayTypes.list.size() > 2) {
        top_spray_idx = game.states.gameplay->selectedSpray;
    } else if(!game.content.sprayTypes.list.empty() && game.content.sprayTypes.list.size() <= 2) {
        top_spray_idx = 0;
    }
    sprayIconMgr.update(
        BUBBLE_RELATION_CURRENT,
        top_spray_idx == INVALID ? nullptr :
        &game.states.gameplay->sprayStats[top_spray_idx],
        top_spray_idx == INVALID ? nullptr :
        game.config.misc.sprayOrder[top_spray_idx]->bmpSpray
    );
    
    size_t prev_spray_idx = INVALID;
    if(game.content.sprayTypes.list.size() >= 3) {
        prev_spray_idx =
            sumAndWrap(
                (int) game.states.gameplay->selectedSpray,
                -1,
                (int) game.content.sprayTypes.list.size()
            );
    }
    sprayIconMgr.update(
        BUBBLE_RELATION_PREVIOUS,
        prev_spray_idx == INVALID ? nullptr :
        &game.states.gameplay->sprayStats[prev_spray_idx],
        prev_spray_idx == INVALID ? nullptr :
        game.config.misc.sprayOrder[prev_spray_idx]->bmpSpray
    );
    
    size_t next_spray_idx = INVALID;
    if(game.content.sprayTypes.list.size() >= 3) {
        next_spray_idx =
            sumAndWrap(
                (int) game.states.gameplay->selectedSpray,
                1,
                (int) game.content.sprayTypes.list.size()
            );
    }
    sprayIconMgr.update(
        BUBBLE_RELATION_NEXT,
        next_spray_idx == INVALID ? nullptr :
        &game.states.gameplay->sprayStats[next_spray_idx],
        next_spray_idx == INVALID ? nullptr :
        game.config.misc.sprayOrder[next_spray_idx]->bmpSpray
    );
    
    sprayIconMgr.tick(delta_t);
    
    //Update the standby items opacity.
    if(
        !game.states.gameplay->curLeaderPtr ||
        game.states.gameplay->curLeaderPtr->group->members.empty()
    ) {
        if(standbyItemsFadeTimer > 0.0f) {
            standbyItemsFadeTimer -= delta_t;
        } else {
            standbyItemsOpacity -=
                HUD::UNNECESSARY_ITEMS_FADE_OUT_SPEED * delta_t;
        }
    } else {
        standbyItemsFadeTimer =
            HUD::UNNECESSARY_ITEMS_FADE_OUT_DELAY;
        standbyItemsOpacity +=
            HUD::UNNECESSARY_ITEMS_FADE_IN_SPEED * delta_t;
    }
    standbyItemsOpacity = std::clamp(standbyItemsOpacity, 0.0f, 1.0f);
    
    //Update the spray items opacity.
    size_t total_sprays = 0;
    for(size_t s = 0; s < game.states.gameplay->sprayStats.size(); s++) {
        total_sprays +=
            game.states.gameplay->sprayStats[s].nrSprays;
    }
    if(total_sprays == 0) {
        if(sprayItemsFadeTimer > 0.0f) {
            sprayItemsFadeTimer -= delta_t;
        } else {
            sprayItemsOpacity -=
                HUD::UNNECESSARY_ITEMS_FADE_OUT_SPEED * delta_t;
        }
    } else {
        sprayItemsFadeTimer =
            HUD::UNNECESSARY_ITEMS_FADE_OUT_DELAY;
        sprayItemsOpacity +=
            HUD::UNNECESSARY_ITEMS_FADE_IN_SPEED * delta_t;
    }
    sprayItemsOpacity = std::clamp(sprayItemsOpacity, 0.0f, 1.0f);
    
    //Tick the GUI items proper.
    gui.tick(game.deltaT);
}
