/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Main gameplay drawing functions.
 */

#include <algorithm>

#include "gameplay.h"

#include "../../content/mob/group_task.h"
#include "../../content/mob/pile.h"
#include "../../content/mob/scale.h"
#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


#pragma warning(disable: 4701)


/**
 * @brief Does the drawing for the main game loop.
 *
 * @param bmp_output If not nullptr, draw the area onto this.
 * @param bmp_transform Transformation to use when drawing to a bitmap.
 * @param bmp_settings Settings to use when drawing to a bitmap.
 */
void GameplayState::doGameDrawing(
    ALLEGRO_BITMAP* bmp_output, const ALLEGRO_TRANSFORM* bmp_transform,
    const MakerTools::AreaImageSettings &bmp_settings
) {

    /*  ***************************************
      *** |  |                           |  | ***
    ***** |__|          DRAWING          |__| *****
      ***  \/                             \/  ***
        ***************************************/
    
    ALLEGRO_TRANSFORM old_world_to_window_transform;
    int blend_old_op, blend_old_src, blend_old_dst,
        blend_old_aop, blend_old_asrc, blend_old_adst;
        
    if(bmp_output) {
        old_world_to_window_transform = game.view.worldToWindowTransform;
        game.view.worldToWindowTransform = *bmp_transform;
        al_set_target_bitmap(bmp_output);
        al_get_separate_blender(
            &blend_old_op, &blend_old_src, &blend_old_dst,
            &blend_old_aop, &blend_old_asrc, &blend_old_adst
        );
        al_set_separate_blender(
            ALLEGRO_ADD, ALLEGRO_ALPHA,
            ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD,
            ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA
        );
    }
    
    al_clear_to_color(game.curAreaData->bgColor);
    
    //Layer 1 -- Background.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Drawing -- Background");
    }
    drawBackground(bmp_output);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Layer 2 -- World components.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Drawing -- World");
    }
    al_use_transform(&game.view.worldToWindowTransform);
    drawWorldComponents(bmp_output);
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Layer 3 -- In-game text.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Drawing -- In-game text");
    }
    if(!bmp_output && game.makerTools.hud) {
        drawIngameText();
    }
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Layer 4 -- Precipitation.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Drawing -- precipitation");
    }
    if(!bmp_output) {
        drawPrecipitation();
    }
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Layer 5 -- Tree shadows.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Drawing -- Tree shadows");
    }
    if(!(bmp_output && !bmp_settings.shadows)) {
        drawTreeShadows();
    }
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Finish dumping to a bitmap image here.
    if(bmp_output) {
        al_set_separate_blender(
            blend_old_op, blend_old_src, blend_old_dst,
            blend_old_aop, blend_old_asrc, blend_old_adst
        );
        game.view.worldToWindowTransform = old_world_to_window_transform;
        al_set_target_backbuffer(game.display);
        return;
    }
    
    //Layer 6 -- Lighting filter.
    if(game.perfMon) {
        game.perfMon->startMeasurement("Drawing -- Lighting");
    }
    drawLightingFilter();
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Layer 7 -- Leader cursor.
    al_use_transform(&game.view.worldToWindowTransform);
    ALLEGRO_COLOR cursor_color = game.config.aestheticGen.noPikminColor;
    if(closestGroupMember[BUBBLE_RELATION_CURRENT]) {
        cursor_color =
            closestGroupMember[BUBBLE_RELATION_CURRENT]->type->mainColor;
    }
    if(curLeaderPtr && game.makerTools.hud) {
        cursor_color =
            changeColorLighting(cursor_color, cursorHeightDiffLight);
        drawLeaderCursor(cursor_color);
    }
    
    //Layer 8 -- HUD.
    al_use_transform(&game.identityTransform);
    
    if(game.perfMon) {
        game.perfMon->startMeasurement("Drawing -- HUD");
    }
    
    if(game.makerTools.hud) {
        hud->gui.draw();
        
        drawBigMsg();
        
        if(msgBox) {
            drawGameplayMessageBox();
        } else if(onionMenu) {
            drawOnionMenu();
        } else if(pauseMenu) {
            drawPauseMenu();
        } else {
            drawMouseCursor(cursor_color);
        }
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //Layer 9 -- System stuff.
    if(game.makerTools.hud) {
        drawSystemStuff();
        
        if(areaTitleFadeTimer.timeLeft > 0) {
            drawLoadingScreen(
                game.curAreaData->name,
                getSubtitleOrMissionGoal(
                    game.curAreaData->subtitle,
                    game.curAreaData->type,
                    game.curAreaData->mission.goal
                ),
                areaTitleFadeTimer.getRatioLeft()
            );
        }
        
    }
    
    drawDebugTools();
}


#pragma warning(default: 4701)


/**
 * @brief Draws the area background.
 *
 * @param bmp_output If not nullptr, draw the background onto this.
 */
void GameplayState::drawBackground(ALLEGRO_BITMAP* bmp_output) {
    if(!game.curAreaData->bgBmp) return;
    
    ALLEGRO_VERTEX bg_v[4];
    for(unsigned char v = 0; v < 4; v++) {
        bg_v[v].color = COLOR_WHITE;
        bg_v[v].z = 0;
    }
    
    //Not gonna lie, this uses some fancy-shmancy numbers.
    //I mostly got here via trial and error.
    //I apologize if you're trying to understand what it means.
    int bmp_w = bmp_output ? al_get_bitmap_width(bmp_output) : game.winW;
    int bmp_h = bmp_output ? al_get_bitmap_height(bmp_output) : game.winH;
    float zoom_to_use = bmp_output ? 0.5 : game.view.cam.zoom;
    Point final_zoom(
        bmp_w * 0.5 * game.curAreaData->bgDist / zoom_to_use,
        bmp_h * 0.5 * game.curAreaData->bgDist / zoom_to_use
    );
    
    bg_v[0].x =
        0;
    bg_v[0].y =
        0;
    bg_v[0].u =
        (game.view.cam.pos.x - final_zoom.x) / game.curAreaData->bgBmpZoom;
    bg_v[0].v =
        (game.view.cam.pos.y - final_zoom.y) / game.curAreaData->bgBmpZoom;
    bg_v[1].x =
        bmp_w;
    bg_v[1].y =
        0;
    bg_v[1].u =
        (game.view.cam.pos.x + final_zoom.x) / game.curAreaData->bgBmpZoom;
    bg_v[1].v =
        (game.view.cam.pos.y - final_zoom.y) / game.curAreaData->bgBmpZoom;
    bg_v[2].x =
        bmp_w;
    bg_v[2].y =
        bmp_h;
    bg_v[2].u =
        (game.view.cam.pos.x + final_zoom.x) / game.curAreaData->bgBmpZoom;
    bg_v[2].v =
        (game.view.cam.pos.y + final_zoom.y) / game.curAreaData->bgBmpZoom;
    bg_v[3].x =
        0;
    bg_v[3].y =
        bmp_h;
    bg_v[3].u =
        (game.view.cam.pos.x - final_zoom.x) / game.curAreaData->bgBmpZoom;
    bg_v[3].v =
        (game.view.cam.pos.y + final_zoom.y) / game.curAreaData->bgBmpZoom;
        
    al_draw_prim(
        bg_v, nullptr, game.curAreaData->bgBmp,
        0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
    );
}


/**
 * @brief Draws the current big message, if any.
 */
void GameplayState::drawBigMsg() {
    switch(curBigMsg) {
    case BIG_MESSAGE_NONE: {
        return;
        
    } case BIG_MESSAGE_READY: {
        const float TEXT_W = game.winW * 0.60f;
        const float TEXT_INITIAL_HEIGHT = 0.10;
        const float TEXT_VARIATION_DUR = 0.08f;
        const float TEXT_START_T = 0.15f;
        const float TEXT_MOVE_MID_T = 0.30f;
        const float TEXT_PAUSE_T = 0.60f;
        const float TEXT_SHRINK_T = 0.95f;
        const float t = bigMsgTime / GAMEPLAY::BIG_MSG_READY_DUR;
        
        KeyframeInterpolator<float> ki_y(game.winH * (-0.2f));
        ki_y.add(TEXT_START_T, game.winH * (-0.2f));
        ki_y.add(TEXT_MOVE_MID_T, game.winH * 0.40f, EASE_METHOD_IN);
        ki_y.add(TEXT_PAUSE_T, game.winH / 2.0f, EASE_METHOD_OUT_ELASTIC);
        ki_y.add(TEXT_SHRINK_T, game.winH / 2.0f);
        KeyframeInterpolator<float> ki_h(TEXT_INITIAL_HEIGHT);
        ki_h.add(TEXT_SHRINK_T, TEXT_INITIAL_HEIGHT * 1.4f);
        ki_h.add(1.0f, 0.0f, EASE_METHOD_IN);
        
        for(size_t c = 0; c < GAMEPLAY::BIG_MSG_READY_TEXT.size(); c++) {
            float char_ratio =
                c / ((float) GAMEPLAY::BIG_MSG_READY_TEXT.size() - 1);
            char_ratio = 1.0f - char_ratio;
            float x_offset = (TEXT_W / 2.0f) - (TEXT_W * char_ratio);
            float y = ki_y.get(t + char_ratio * TEXT_VARIATION_DUR);
            drawText(
                string(1, GAMEPLAY::BIG_MSG_READY_TEXT[c]),
                game.sysContent.fntAreaName,
                Point((game.winW / 2.0f) + x_offset, y),
                Point(LARGE_FLOAT, game.winH * ki_h.get(t)), COLOR_GOLD
            );
        }
        break;
        
    } case BIG_MESSAGE_GO: {

        const float TEXT_GROW_STOP_T = 0.10f;
        const float t = bigMsgTime / GAMEPLAY::BIG_MSG_GO_DUR;
        
        KeyframeInterpolator<float> ki_h(0.0f);
        ki_h.add(TEXT_GROW_STOP_T, 0.20f, EASE_METHOD_OUT_ELASTIC);
        ki_h.add(1.0f, 0.22f);
        KeyframeInterpolator<float> ki_a(1.0f);
        ki_a.add(TEXT_GROW_STOP_T, 1.0f);
        ki_a.add(1.0f, 0.0f);
        
        drawText(
            GAMEPLAY::BIG_MSG_GO_TEXT,
            game.sysContent.fntAreaName,
            Point(game.winW / 2.0f, game.winH / 2.0f),
            Point(LARGE_FLOAT, game.winH * ki_h.get(t)),
            changeAlpha(COLOR_GOLD, 255 * ki_a.get(t))
        );
        break;
        
    } case BIG_MESSAGE_MISSION_CLEAR:
    case BIG_MESSAGE_MISSION_FAILED: {
        const string &TEXT =
            curBigMsg == BIG_MESSAGE_MISSION_CLEAR ?
            GAMEPLAY::BIG_MSG_MISSION_CLEAR_TEXT :
            GAMEPLAY::BIG_MSG_MISSION_FAILED_TEXT;
        const float TEXT_W = game.winW * 0.80f;
        const float TEXT_INITIAL_HEIGHT = 0.05f;
        const float TEXT_VARIATION_DUR = 0.08f;
        const float TEXT_MOVE_MID_T = 0.30f;
        const float TEXT_PAUSE_T = 0.50f;
        const float TEXT_FADE_T = 0.90f;
        const float t =
            curBigMsg == BIG_MESSAGE_MISSION_CLEAR ?
            (bigMsgTime / GAMEPLAY::BIG_MSG_MISSION_CLEAR_DUR) :
            (bigMsgTime / GAMEPLAY::BIG_MSG_MISSION_FAILED_DUR);
            
        KeyframeInterpolator<float> ki_y(game.winH * (-0.2f));
        ki_y.add(TEXT_MOVE_MID_T, game.winH * 0.40f, EASE_METHOD_IN);
        ki_y.add(TEXT_PAUSE_T, game.winH / 2.0f, EASE_METHOD_OUT_ELASTIC);
        KeyframeInterpolator<float> ki_h(TEXT_INITIAL_HEIGHT);
        ki_h.add(1.0f, TEXT_INITIAL_HEIGHT * 1.4f, EASE_METHOD_IN);
        KeyframeInterpolator<float> ki_a(1.0f);
        ki_a.add(TEXT_FADE_T, 1.0f);
        ki_a.add(1.0f, 0.0f);
        
        float alpha = ki_a.get(t);
        
        for(size_t c = 0; c < TEXT.size(); c++) {
            float char_ratio = c / ((float) TEXT.size() - 1);
            char_ratio = 1.0f - char_ratio;
            float x_offset = (TEXT_W / 2.0f) - (TEXT_W * char_ratio);
            float y = ki_y.get(t + char_ratio * TEXT_VARIATION_DUR);
            
            drawText(
                string(1, TEXT[c]), game.sysContent.fntAreaName,
                Point((game.winW / 2.0f) + x_offset, y),
                Point(LARGE_FLOAT, game.winH * ki_h.get(t)),
                changeAlpha(COLOR_GOLD, 255 * alpha)
            );
        }
        break;
        
    }
    }
}


/**
 * @brief Draws any debug visualization tools useful for debugging.
 */
void GameplayState::drawDebugTools() {
    //Tests using Dear ImGui.
    /*
    ImGui::GetIO().MouseDrawCursor = true;
    //GUI logic goes here.
    */
    
    //Raw analog stick viewer.
    /*
    const float RAW_STICK_VIEWER_X = 8;
    const float RAW_STICK_VIEWER_Y = 8;
    const float RAW_STICK_VIEWER_SIZE = 100;
    
    point raw_stick_coords;
    raw_stick_coords.x = game.controls.mgr.rawSticks[0][0][0];
    raw_stick_coords.y = game.controls.mgr.rawSticks[0][0][1];
    float raw_stick_angle;
    float raw_stick_mag;
    coordinatesToAngle(
        raw_stick_coords, &raw_stick_angle, &raw_stick_mag
    );
    al_draw_filled_rectangle(
        RAW_STICK_VIEWER_X,
        RAW_STICK_VIEWER_Y,
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE,
        al_map_rgba(0, 0, 0, 200)
    );
    al_draw_circle(
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE / 2.0f,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE / 2.0f,
        RAW_STICK_VIEWER_SIZE / 2.0f,
        raw_stick_mag >= 0.99f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    al_draw_line(
        RAW_STICK_VIEWER_X,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE / 2.0f,
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE / 2.0f,
        fabs(raw_stick_coords.y) <= 0.01f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    al_draw_line(
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE / 2.0f,
        RAW_STICK_VIEWER_Y,
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE / 2.0f,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE,
        fabs(raw_stick_coords.x) <= 0.01f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    point raw_draw_coords =
        raw_stick_coords * RAW_STICK_VIEWER_SIZE / 2.0f;
    al_draw_filled_circle(
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE / 2.0f +
        raw_draw_coords.x,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE / 2.0f +
        raw_draw_coords.y,
        3.5f, al_map_rgb(255, 64, 64)
    );
    al_draw_filled_rectangle(
        RAW_STICK_VIEWER_X,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE + 1,
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE + 18,
        al_map_rgba(0, 0, 0, 200)
    );
    al_draw_text(
        game.sysContent.fnt_builtin,
        al_map_rgb(255, 64, 64),
        RAW_STICK_VIEWER_X, RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE + 1,
        ALLEGRO_ALIGN_LEFT,
        (
            boxString(
                (raw_stick_coords.x >= 0.0f ? " " : "") +
                f2s(raw_stick_coords.x), 6
            ) + " " + boxString(
                (raw_stick_coords.y >= 0.0f ? " " : "") +
                f2s(raw_stick_coords.y), 6
            )
        ).c_str()
    );
    al_draw_text(
        game.sysContent.fnt_builtin,
        al_map_rgb(255, 64, 64),
        RAW_STICK_VIEWER_X, RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE + 1 + 8,
        ALLEGRO_ALIGN_LEFT,
        (
            boxString(
                (raw_stick_angle >= 0.0f ? " " : "") +
                f2s(raw_stick_angle), 6
            ) + " " + boxString(
                (raw_stick_mag >= 0.0f ? " " : "") +
                f2s(raw_stick_mag), 6
            )
        ).c_str()
    );
    */
    
    //Clean analog stick viewer.
    /*
    const float CLEAN_STICK_VIEWER_X = 116;
    const float CLEAN_STICK_VIEWER_Y = 8;
    const float CLEAN_STICK_VIEWER_SIZE = 100;
    
    point clean_stick_coords;
    clean_stick_coords.x =
        game.controls.getPlayerActionTypeValue(PLAYER_ACTION_TYPE_RIGHT) -
        game.controls.getPlayerActionTypeValue(PLAYER_ACTION_TYPE_LEFT);
    clean_stick_coords.y =
        game.controls.getPlayerActionTypeValue(PLAYER_ACTION_TYPE_DOWN) -
        game.controls.getPlayerActionTypeValue(PLAYER_ACTION_TYPE_UP);
    float clean_stick_angle;
    float clean_stick_mag;
    coordinatesToAngle(
        clean_stick_coords, &clean_stick_angle, &clean_stick_mag
    );
    al_draw_filled_rectangle(
        CLEAN_STICK_VIEWER_X,
        CLEAN_STICK_VIEWER_Y,
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE,
        al_map_rgba(0, 0, 0, 200)
    );
    al_draw_circle(
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        CLEAN_STICK_VIEWER_SIZE / 2.0f,
        clean_stick_mag >= 0.99f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    al_draw_line(
        CLEAN_STICK_VIEWER_X,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        fabs(clean_stick_coords.y) <= 0.01f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    al_draw_line(
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        CLEAN_STICK_VIEWER_Y,
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE,
        fabs(clean_stick_coords.x) <= 0.01f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    point clean_draw_coords =
        clean_stick_coords * CLEAN_STICK_VIEWER_SIZE / 2.0f;
    al_draw_filled_circle(
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE / 2.0f +
        clean_draw_coords.x,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE / 2.0f +
        clean_draw_coords.y,
        3.5f, al_map_rgb(255, 64, 64)
    );
    al_draw_filled_rectangle(
        CLEAN_STICK_VIEWER_X,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE + 1,
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE + 18,
        al_map_rgba(0, 0, 0, 200)
    );
    al_draw_text(
        game.sysContent.fnt_builtin,
        al_map_rgb(255, 64, 64),
        CLEAN_STICK_VIEWER_X,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE + 1,
        ALLEGRO_ALIGN_LEFT,
        (
            boxString(
                (clean_stick_coords.x >= 0.0f ? " " : "") +
                f2s(clean_stick_coords.x), 6
            ) + " " + boxString(
                (clean_stick_coords.y >= 0.0f ? " " : "") +
                f2s(clean_stick_coords.y), 6
            )
        ).c_str()
    );
    al_draw_text(
        game.sysContent.fnt_builtin,
        al_map_rgb(255, 64, 64),
        CLEAN_STICK_VIEWER_X, CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE +
        1 + 8,
        ALLEGRO_ALIGN_LEFT,
        (
            boxString(
                (clean_stick_angle >= 0.0f ? " " : "") +
                f2s(clean_stick_angle), 6
            ) + " " + boxString(
                (clean_stick_mag >= 0.0f ? " " : "") +
                f2s(clean_stick_mag), 6
            )
        ).c_str()
    );
    */
    
    //Group stuff.
    /*
    al_use_transform(&game.view.worldToWindowTransform);
    for(size_t m = 0; m < curLeaderPtr->group->members.size(); m++) {
        point offset = curLeaderPtr->group->get_spot_offset(m);
        al_draw_filled_circle(
            curLeaderPtr->group->anchor.x + offset.x,
            curLeaderPtr->group->anchor.y + offset.y,
            3.0f,
            al_map_rgba(0, 0, 0, 192)
        );
    }
    al_draw_circle(
        curLeaderPtr->group->anchor.x,
        curLeaderPtr->group->anchor.y,
        3.0f,
        curLeaderPtr->group->mode == Group::MODE_SHUFFLE ?
        al_map_rgba(0, 255, 0, 192) :
        curLeaderPtr->group->mode == Group::MODE_FOLLOW_BACK ?
        al_map_rgba(255, 255, 0, 192) :
        al_map_rgba(255, 0, 0, 192),
        2.0f
    );
    
    point group_mid_point =
        curLeaderPtr->group->anchor +
        rotate_point(
            point(curLeaderPtr->group->radius, 0.0f),
            curLeaderPtr->group->anchor_angle
        );
    al_draw_filled_circle(
        group_mid_point.x,
        group_mid_point.y,
        3.0f,
        al_map_rgb(0, 0, 255)
    );
    */
}


/**
 * @brief Draws the in-game text.
 */
void GameplayState::drawIngameText() {
    //Mob things.
    size_t n_mobs = mobs.all.size();
    for(size_t m = 0; m < n_mobs; m++) {
        Mob* mob_ptr = mobs.all[m];
        
        //Fractions and health.
        if(mob_ptr->healthWheel) {
            mob_ptr->healthWheel->draw();
        }
        if(mob_ptr->fraction) {
            mob_ptr->fraction->draw();
        }
        
        //Maker tool -- draw hitboxes.
        if(game.makerTools.hitboxes) {
            Sprite* s;
            mob_ptr->getSpriteData(&s, nullptr, nullptr);
            if(s) {
                for(size_t h = 0; h < s->hitboxes.size(); h++) {
                    Hitbox* h_ptr = &s->hitboxes[h];
                    ALLEGRO_COLOR hc;
                    switch(h_ptr->type) {
                    case HITBOX_TYPE_NORMAL: {
                        hc = al_map_rgba(0, 128, 0, 192); //Green.
                        break;
                    } case HITBOX_TYPE_ATTACK: {
                        hc = al_map_rgba(128, 0, 0, 192); //Red.
                        break;
                    } case HITBOX_TYPE_DISABLED: {
                        hc = al_map_rgba(128, 128, 0, 192); //Yellow.
                        break;
                    } default:
                        hc = COLOR_BLACK;
                        break;
                    }
                    Point p =
                        mob_ptr->pos + rotatePoint(h_ptr->pos, mob_ptr->angle);
                    al_draw_filled_circle(p.x, p.y, h_ptr->radius, hc);
                }
            }
        }
        
        //Maker tool -- draw collision.
        if(game.makerTools.collision) {
            if(mob_ptr->type->pushesWithHitboxes) {
                Sprite* s;
                mob_ptr->getSpriteData(&s, nullptr, nullptr);
                if(s) {
                    for(size_t h = 0; h < s->hitboxes.size(); h++) {
                        Hitbox* h_ptr = &s->hitboxes[h];
                        Point p =
                            mob_ptr->pos +
                            rotatePoint(h_ptr->pos, mob_ptr->angle);
                        al_draw_circle(
                            p.x, p.y,
                            h_ptr->radius, COLOR_WHITE, 1
                        );
                    }
                }
            } else if(mob_ptr->rectangularDim.x != 0) {
                Point tl(
                    -mob_ptr->rectangularDim.x / 2.0f,
                    -mob_ptr->rectangularDim.y / 2.0f
                );
                Point br(
                    mob_ptr->rectangularDim.x / 2.0f,
                    mob_ptr->rectangularDim.y / 2.0f
                );
                vector<Point> rect_vertices {
                    rotatePoint(tl, mob_ptr->angle) +
                    mob_ptr->pos,
                    rotatePoint(Point(tl.x, br.y),  mob_ptr->angle) +
                    mob_ptr->pos,
                    rotatePoint(br, mob_ptr->angle) +
                    mob_ptr->pos,
                    rotatePoint(Point(br.x, tl.y), mob_ptr->angle) +
                    mob_ptr->pos
                };
                float vertices[] {
                    rect_vertices[0].x,
                    rect_vertices[0].y,
                    rect_vertices[1].x,
                    rect_vertices[1].y,
                    rect_vertices[2].x,
                    rect_vertices[2].y,
                    rect_vertices[3].x,
                    rect_vertices[3].y
                };
                
                al_draw_polygon(vertices, 4, 0, COLOR_WHITE, 1, 10);
            } else {
                al_draw_circle(
                    mob_ptr->pos.x, mob_ptr->pos.y,
                    mob_ptr->radius, COLOR_WHITE, 1
                );
            }
        }
    }
    
    //Maker tool -- draw path info.
    if(
        game.makerTools.infoLock &&
        game.makerTools.pathInfo &&
        game.makerTools.infoLock->pathInfo
    ) {
        Path* path = game.makerTools.infoLock->pathInfo;
        Point target_pos =
            hasFlag(path->settings.flags, PATH_FOLLOW_FLAG_FOLLOW_MOB) ?
            path->settings.targetMob->pos :
            path->settings.targetPoint;
            
        if(!path->path.empty()) {
        
            //Faint lines for the entire path.
            for(size_t s = 0; s < path->path.size() - 1; s++) {
                bool is_blocked = false;
                PathLink* l_ptr = path->path[s]->get_link(path->path[s + 1]);
                auto l_it = pathMgr.obstructions.find(l_ptr);
                if(l_it != pathMgr.obstructions.end()) {
                    is_blocked = !l_it->second.empty();
                }
                
                al_draw_line(
                    path->path[s]->pos.x,
                    path->path[s]->pos.y,
                    path->path[s + 1]->pos.x,
                    path->path[s + 1]->pos.y,
                    is_blocked ?
                    al_map_rgba(200, 0, 0, 150) :
                    al_map_rgba(0, 0, 200, 150),
                    2.0f
                );
            }
            
            //Colored circles for the first and last stops.
            al_draw_filled_circle(
                path->path[0]->pos.x,
                path->path[0]->pos.y,
                16.0f,
                al_map_rgba(192, 0, 0, 200)
            );
            al_draw_filled_circle(
                path->path.back()->pos.x,
                path->path.back()->pos.y,
                16.0f,
                al_map_rgba(0, 192, 0, 200)
            );
            
        }
        
        if(
            path->result == PATH_RESULT_DIRECT ||
            path->result == PATH_RESULT_DIRECT_NO_STOPS ||
            path->cur_path_stop_idx == path->path.size()
        ) {
            bool is_blocked = path->block_reason != PATH_BLOCK_REASON_NONE;
            //Line directly to the target.
            al_draw_line(
                game.makerTools.infoLock->pos.x,
                game.makerTools.infoLock->pos.y,
                target_pos.x,
                target_pos.y,
                is_blocked ?
                al_map_rgba(255, 0, 0, 200) :
                al_map_rgba(0, 0, 255, 200),
                4.0f
            );
        } else if(path->cur_path_stop_idx < path->path.size()) {
            bool is_blocked = path->block_reason != PATH_BLOCK_REASON_NONE;
            //Line to the next stop, and circle for the next stop in blue.
            al_draw_line(
                game.makerTools.infoLock->pos.x,
                game.makerTools.infoLock->pos.y,
                path->path[path->cur_path_stop_idx]->pos.x,
                path->path[path->cur_path_stop_idx]->pos.y,
                is_blocked ?
                al_map_rgba(255, 0, 0, 200) :
                al_map_rgba(0, 0, 255, 200),
                4.0f
            );
            al_draw_filled_circle(
                path->path[path->cur_path_stop_idx]->pos.x,
                path->path[path->cur_path_stop_idx]->pos.y,
                10.0f,
                is_blocked ?
                al_map_rgba(192, 0, 0, 200) :
                al_map_rgba(0, 0, 192, 200)
            );
        }
        
        //Square on the target spot, and target distance.
        al_draw_filled_rectangle(
            target_pos.x - 8.0f,
            target_pos.y - 8.0f,
            target_pos.x + 8.0f,
            target_pos.y + 8.0f,
            al_map_rgba(0, 192, 0, 200)
        );
        al_draw_circle(
            target_pos.x,
            target_pos.y,
            path->settings.finalTargetDistance,
            al_map_rgba(0, 255, 0, 200),
            1.0f
        );
        
        //Diamonds for faked starts and ends.
        if(hasFlag(path->settings.flags, PATH_FOLLOW_FLAG_FAKED_START)) {
            drawFilledDiamond(
                path->settings.fakedStart, 8, al_map_rgba(255, 0, 0, 200)
            );
        }
        if(hasFlag(path->settings.flags, PATH_FOLLOW_FLAG_FAKED_END)) {
            drawFilledDiamond(
                path->settings.fakedEnd, 8, al_map_rgba(0, 255, 0, 200)
            );
        }
    }
    
    notification.draw();
}


/**
 * @brief Draws the leader's cursor and associated effects.
 *
 * @param color Color to tint it by.
 */
void GameplayState::drawLeaderCursor(const ALLEGRO_COLOR &color) {
    if(!curLeaderPtr) return;
    
    size_t n_arrows = curLeaderPtr->swarmArrows.size();
    for(size_t a = 0; a < n_arrows; a++) {
        Point pos(
            cos(swarmAngle) * curLeaderPtr->swarmArrows[a],
            sin(swarmAngle) * curLeaderPtr->swarmArrows[a]
        );
        float alpha =
            64 + std::min(
                191,
                (int) (
                    191 *
                    (curLeaderPtr->swarmArrows[a] /
                     (game.config.rules.cursorMaxDist * 0.4))
                )
            );
        drawBitmap(
            game.sysContent.bmpSwarmArrow,
            curLeaderPtr->pos + pos,
            Point(
                16 * (1 + curLeaderPtr->swarmArrows[a] /
                      game.config.rules.cursorMaxDist),
                -1
            ),
            swarmAngle,
            mapAlpha(alpha)
        );
    }
    
    size_t n_rings = whistle.rings.size();
    float cursor_angle =
        getAngle(curLeaderPtr->pos, leaderCursorW);
    float cursor_distance =
        Distance(curLeaderPtr->pos, leaderCursorW).toFloat();
    for(size_t r = 0; r < n_rings; r++) {
        Point pos(
            curLeaderPtr->pos.x + cos(cursor_angle) * whistle.rings[r],
            curLeaderPtr->pos.y + sin(cursor_angle) * whistle.rings[r]
        );
        float ring_to_whistle_distance = cursor_distance - whistle.rings[r];
        float scale =
            interpolateNumber(
                ring_to_whistle_distance,
                0, cursor_distance,
                whistle.radius * 2, 0
            );
        float alpha =
            interpolateNumber(
                ring_to_whistle_distance,
                0, cursor_distance,
                0, 100
            );
        unsigned char n = whistle.ringColors[r];
        drawBitmap(
            game.sysContent.bmpBrightRing,
            pos,
            Point(scale),
            0.0f,
            al_map_rgba(
                WHISTLE::RING_COLORS[n][0],
                WHISTLE::RING_COLORS[n][1],
                WHISTLE::RING_COLORS[n][2],
                alpha
            )
        );
    }
    
    if(whistle.radius > 0 || whistle.fadeTimer.timeLeft > 0.0f) {
        al_draw_filled_circle(
            whistle.center.x, whistle.center.y,
            whistle.radius,
            al_map_rgba(48, 128, 120, 64)
        );
        
        unsigned char n_dots = 16 * WHISTLE::N_DOT_COLORS;
        for(unsigned char d = 0; d < WHISTLE::N_DOT_COLORS; d++) {
            for(unsigned char d2 = 0; d2 < 16; d2++) {
                unsigned char current_dot = d2 * WHISTLE::N_DOT_COLORS + d;
                float angle =
                    TAU / n_dots *
                    current_dot -
                    WHISTLE::DOT_SPIN_SPEED * areaTimePassed;
                    
                Point dot_pos(
                    whistle.center.x +
                    cos(angle) * whistle.dotRadius[d],
                    whistle.center.y +
                    sin(angle) * whistle.dotRadius[d]
                );
                
                ALLEGRO_COLOR dot_color =
                    al_map_rgb(
                        WHISTLE::DOT_COLORS[d][0],
                        WHISTLE::DOT_COLORS[d][1],
                        WHISTLE::DOT_COLORS[d][2]
                    );
                unsigned char dot_alpha = 255;
                if(whistle.fadeTimer.timeLeft > 0.0f) {
                    dot_alpha = 255 * whistle.fadeTimer.getRatioLeft();
                }
                
                drawBitmap(
                    game.sysContent.bmpBrightCircle,
                    dot_pos, Point(5.0f),
                    0.0f, changeAlpha(dot_color, dot_alpha)
                );
            }
        }
    }
    
    //Leader cursor.
    Point bmp_cursor_size = getBitmapDimensions(game.sysContent.bmpCursor);
    
    drawBitmap(
        game.sysContent.bmpCursor,
        leaderCursorW,
        bmp_cursor_size / 2.0f,
        cursor_angle,
        changeColorLighting(
            color,
            cursorHeightDiffLight
        )
    );
    
    //Throw preview.
    drawThrowPreview();
    
    //Standby type count.
    size_t n_standby_pikmin = 0;
    if(curLeaderPtr->group->curStandbyType) {
        for(
            size_t m = 0; m < curLeaderPtr->group->members.size(); m++
        ) {
            Mob* m_ptr = curLeaderPtr->group->members[m];
            if(
                m_ptr->subgroupTypePtr ==
                curLeaderPtr->group->curStandbyType
            ) {
                n_standby_pikmin++;
            }
        }
    }
    
    al_use_transform(&game.identityTransform);
    
    float count_offset =
        std::max(bmp_cursor_size.x, bmp_cursor_size.y) * 0.18f * game.view.cam.zoom;
        
    if(n_standby_pikmin > 0) {
        drawText(
            i2s(n_standby_pikmin), game.sysContent.fntCursorCounter,
            leaderCursorWin +
            Point(count_offset),
            Point(LARGE_FLOAT, game.winH * 0.02f), color,
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
        );
    }
    
    al_use_transform(&game.view.worldToWindowTransform);
}


/**
 * @brief Draws the full-window effects that will represent lighting.
 */
void GameplayState::drawLightingFilter() {
    al_use_transform(&game.identityTransform);
    
    //Draw the fog effect.
    ALLEGRO_COLOR fog_c =
        game.curAreaData->weatherCondition.getFogColor();
    if(fog_c.a > 0) {
        //Start by drawing the central fog fade out effect.
        
        Point fog_top_left =
            game.view.cam.pos -
            Point(
                game.curAreaData->weatherCondition.fogFar,
                game.curAreaData->weatherCondition.fogFar
            );
        Point fog_bottom_right =
            game.view.cam.pos +
            Point(
                game.curAreaData->weatherCondition.fogFar,
                game.curAreaData->weatherCondition.fogFar
            );
        al_transform_coordinates(
            &game.view.worldToWindowTransform,
            &fog_top_left.x, &fog_top_left.y
        );
        al_transform_coordinates(
            &game.view.worldToWindowTransform,
            &fog_bottom_right.x, &fog_bottom_right.y
        );
        
        if(bmpFog) {
            drawBitmap(
                bmpFog,
                (fog_top_left + fog_bottom_right) / 2,
                (fog_bottom_right - fog_top_left),
                0, fog_c
            );
        }
        
        //Now draw the fully opaque fog around the central fade.
        //Top-left and top-center.
        al_draw_filled_rectangle(
            0, 0,
            fog_bottom_right.x, fog_top_left.y,
            fog_c
        );
        //Top-right and center-right.
        al_draw_filled_rectangle(
            fog_bottom_right.x, 0,
            game.winW, fog_bottom_right.y,
            fog_c
        );
        //Bottom-right and bottom-center.
        al_draw_filled_rectangle(
            fog_top_left.x, fog_bottom_right.y,
            game.winW, game.winH,
            fog_c
        );
        //Bottom-left and center-left.
        al_draw_filled_rectangle(
            0, fog_top_left.y,
            fog_top_left.x, game.winH,
            fog_c
        );
        
    }
    
    //Draw the daylight.
    ALLEGRO_COLOR daylight_c =
        game.curAreaData->weatherCondition.getDaylightColor();
    if(daylight_c.a > 0) {
        al_draw_filled_rectangle(0, 0, game.winW, game.winH, daylight_c);
    }
    
    //Draw the blackout effect.
    unsigned char blackout_s =
        game.curAreaData->weatherCondition.getBlackoutStrength();
    if(blackout_s > 0) {
        //First, we'll create the lightmap.
        //This is inverted (white = darkness, black = light), because we'll
        //apply it to the window using a subtraction operation.
        al_set_target_bitmap(lightmapBmp);
        
        //For starters, the whole window is dark (white in the map).
        al_clear_to_color(mapGray(blackout_s));
        
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(
            &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
        );
        al_set_separate_blender(
            ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ONE, ALLEGRO_ONE,
            ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE
        );
        
        //Then, find out spotlights, and draw
        //their lights on the map (as black).
        al_hold_bitmap_drawing(true);
        for(size_t m = 0; m < mobs.all.size(); m++) {
            Mob* m_ptr = mobs.all[m];
            if(
                hasFlag(m_ptr->flags, MOB_FLAG_HIDDEN) ||
                m_ptr->type->blackoutRadius == 0.0f
            ) {
                continue;
            }
            
            Point pos = m_ptr->pos;
            al_transform_coordinates(
                &game.view.worldToWindowTransform,
                &pos.x, &pos.y
            );
            float radius = 4.0f * game.view.cam.zoom;
            
            if(m_ptr->type->blackoutRadius > 0.0f) {
                radius *= m_ptr->type->blackoutRadius;
            } else {
                radius *= m_ptr->radius;
            }
            
            al_draw_scaled_bitmap(
                game.sysContent.bmpSpotlight,
                0, 0, 64, 64,
                pos.x - radius, pos.y - radius,
                radius * 2.0, radius * 2.0,
                0
            );
        }
        al_hold_bitmap_drawing(false);
        
        //Now, simply darken the window using the map.
        al_set_target_backbuffer(game.display);
        
        al_draw_bitmap(lightmapBmp, 0, 0, 0);
        
        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
        
    }
    
}


/**
 * @brief Draws a gameplay message box.
 */
void GameplayState::drawGameplayMessageBox() {
    //Mouse cursor.
    drawMouseCursor(GAME::CURSOR_STANDARD_COLOR);
    
    al_use_transform(&game.identityTransform);
    
    //Transition things.
    float transition_ratio =
        msgBox->transitionIn ?
        msgBox->transitionTimer / GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME :
        (1 - msgBox->transitionTimer / GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME);
    int line_height = al_get_font_line_height(game.sysContent.fntStandard);
    float box_height = line_height * 4;
    float offset =
        box_height * ease(EASE_METHOD_IN, transition_ratio);
        
    //Draw a rectangle to darken gameplay.
    al_draw_filled_rectangle(
        0.0f, 0.0f,
        game.winW, game.winH,
        al_map_rgba(0, 0, 0, 64 * (1 - transition_ratio))
    );
    
    //Draw the message box proper.
    drawTexturedBox(
        Point(
            game.winW / 2,
            game.winH - (box_height / 2.0f) - 4 + offset
        ),
        Point(game.winW - 16, box_height),
        game.sysContent.bmpBubbleBox
    );
    
    //Draw the speaker's icon, if any.
    if(msgBox->speakerIcon) {
        drawBitmap(
            msgBox->speakerIcon,
            Point(
                40,
                game.winH - box_height - 16 + offset
            ),
            Point(48.0f)
        );
        drawBitmap(
            hud->bmpBubble,
            Point(
                40,
                game.winH - box_height - 16 + offset
            ),
            Point(64.0f)
        );
    }
    
    //Draw the button to advance, if it's time.
    drawPlayerInputSourceIcon(
        game.sysContent.fntSlim,
        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).inputSource,
        true,
        Point(
            game.winW - (GAMEPLAY_MSG_BOX::MARGIN + GAMEPLAY_MSG_BOX::PADDING + 8.0f),
            game.winH - (GAMEPLAY_MSG_BOX::MARGIN + GAMEPLAY_MSG_BOX::PADDING + 8.0f) +
            offset
        ),
        Point(32.0f),
        msgBox->advanceButtonAlpha * 255
    );
    
    //Draw the message's text.
    size_t token_idx = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t line_idx = msgBox->curSection * 3 + l;
        if(line_idx >= msgBox->tokensPerLine.size()) {
            break;
        }
        
        //Figure out what scaling is necessary, if any.
        unsigned int total_width = 0;
        float x_scale = 1.0f;
        for(size_t t = 0; t < msgBox->tokensPerLine[line_idx].size(); t++) {
            total_width += msgBox->tokensPerLine[line_idx][t].width;
        }
        const float max_text_width = (GAMEPLAY_MSG_BOX::MARGIN + GAMEPLAY_MSG_BOX::PADDING) * 2;
        if(total_width > game.winW - max_text_width) {
            x_scale = (game.winW - max_text_width) / total_width;
        }
        
        float caret =
            GAMEPLAY_MSG_BOX::MARGIN + GAMEPLAY_MSG_BOX::PADDING;
        float start_y =
            game.winH - line_height * 4 + GAMEPLAY_MSG_BOX::PADDING + offset;
            
        for(size_t t = 0; t < msgBox->tokensPerLine[line_idx].size(); t++) {
            token_idx++;
            if(token_idx >= msgBox->curToken) break;
            StringToken &cur_token = msgBox->tokensPerLine[line_idx][t];
            
            float x = caret;
            float y = start_y + line_height * l;
            unsigned char alpha = 255;
            float this_token_anim_time;
            
            //Change the token's position and alpha, if it needs animating.
            //First, check for the typing animation.
            if(token_idx >= msgBox->skippedAtToken) {
                this_token_anim_time = msgBox->totalSkipAnimTime;
            } else {
                this_token_anim_time =
                    msgBox->totalTokenAnimTime -
                    ((token_idx + 1) * game.config.aestheticGen.gameplayMsgChInterval);
            }
            if(
                this_token_anim_time > 0 &&
                this_token_anim_time < GAMEPLAY_MSG_BOX::TOKEN_ANIM_DURATION
            ) {
                float ratio =
                    this_token_anim_time / GAMEPLAY_MSG_BOX::TOKEN_ANIM_DURATION;
                x +=
                    GAMEPLAY_MSG_BOX::TOKEN_ANIM_X_AMOUNT *
                    ease(EASE_METHOD_UP_AND_DOWN_ELASTIC, ratio);
                y +=
                    GAMEPLAY_MSG_BOX::TOKEN_ANIM_Y_AMOUNT *
                    ease(EASE_METHOD_UP_AND_DOWN_ELASTIC, ratio);
                alpha = ratio * 255;
            }
            
            //Now, for the swiping animation.
            if(msgBox->swipeTimer > 0.0f) {
                float ratio =
                    1 - (msgBox->swipeTimer / GAMEPLAY_MSG_BOX::TOKEN_SWIPE_DURATION);
                x += GAMEPLAY_MSG_BOX::TOKEN_SWIPE_X_AMOUNT * ratio;
                y += GAMEPLAY_MSG_BOX::TOKEN_SWIPE_Y_AMOUNT * ratio;
                alpha = std::max(0, (signed int) (alpha - ratio * 255));
            }
            
            //Actually draw it now.
            float token_final_width = cur_token.width * x_scale;
            switch(cur_token.type) {
            case STRING_TOKEN_CHAR: {
                drawText(
                    cur_token.content, game.sysContent.fntStandard,
                    Point(x, y),
                    Point(token_final_width, LARGE_FLOAT),
                    mapAlpha(alpha),
                    ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP, 0,
                    Point(x_scale, 1.0f)
                );
                break;
            }
            case STRING_TOKEN_CONTROL_BIND: {
                drawPlayerInputSourceIcon(
                    game.sysContent.fntSlim,
                    game.controls.findBind(cur_token.content).inputSource,
                    true,
                    Point(
                        x + token_final_width / 2.0f,
                        y + line_height / 2.0f
                    ),
                    Point(token_final_width, line_height)
                );
                break;
            }
            default: {
                break;
            }
            }
            caret += token_final_width;
        }
    }
}


/**
 * @brief Draws the current Onion menu.
 */
void GameplayState::drawOnionMenu() {
    al_draw_filled_rectangle(
        0, 0, game.winW, game.winH,
        al_map_rgba(24, 64, 60, 220 * onionMenu->bgAlphaMult)
    );
    
    onionMenu->gui.draw();
    
    drawMouseCursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Draws the current pause menu.
 */
void GameplayState::drawPauseMenu() {
    al_draw_filled_rectangle(
        0, 0, game.winW, game.winH,
        al_map_rgba(24, 64, 60, 200 * pauseMenu->bgAlphaMult)
    );
    
    pauseMenu->draw();
    
    drawMouseCursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Draws the precipitation.
 */
void GameplayState::drawPrecipitation() {
    if(
        game.curAreaData->weatherCondition.precipitationType !=
        PRECIPITATION_TYPE_NONE
    ) {
        size_t n_precipitation_particles = precipitation.size();
        for(size_t p = 0; p < n_precipitation_particles; p++) {
            al_draw_filled_circle(
                precipitation[p].x, precipitation[p].y,
                3, COLOR_WHITE
            );
        }
    }
}


/**
 * @brief Draws system stuff.
 */
void GameplayState::drawSystemStuff() {
    if(!game.makerTools.infoPrintText.empty()) {
        float alpha_mult = 1;
        if(
            game.makerTools.infoPrintTimer.timeLeft <
            game.makerTools.infoPrintFadeDuration
        ) {
            alpha_mult =
                game.makerTools.infoPrintTimer.timeLeft /
                game.makerTools.infoPrintFadeDuration;
        }
        
        size_t n_lines =
            split(game.makerTools.infoPrintText, "\n", true).size();
        int fh = al_get_font_line_height(game.sysContent.fntBuiltin);
        //We add n_lines - 1 because there is a 1px gap between each line.
        int total_height = (int) n_lines * fh + (int) (n_lines - 1);
        
        al_draw_filled_rectangle(
            0, 0, game.winW, total_height + 16,
            al_map_rgba(0, 0, 0, 96 * alpha_mult)
        );
        drawTextLines(
            game.makerTools.infoPrintText,
            game.sysContent.fntBuiltin,
            Point(8.0f),
            Point(LARGE_FLOAT),
            al_map_rgba(255, 255, 255, 128 * alpha_mult),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP, TEXT_SETTING_FLAG_CANT_GROW
        );
    }
    
    if(game.showSystemInfo && !game.framerateHistory.empty()) {
        //Draw the framerate chart.
        al_draw_filled_rectangle(
            game.winW - GAME::FRAMERATE_HISTORY_SIZE, 0,
            game.winW, 100,
            al_map_rgba(0, 0, 0, 192)
        );
        double chart_min = 1.0f; //1 FPS.
        double chart_max =
            game.options.advanced.targetFps + game.options.advanced.targetFps * 0.05f;
        for(size_t f = 0; f < game.framerateHistory.size(); f++) {
            float fps =
                std::min(
                    (float) (1.0f / game.framerateHistory[f]),
                    (float) game.options.advanced.targetFps
                );
            float fps_y =
                interpolateNumber(
                    fps,
                    chart_min, chart_max,
                    0, 100
                );
            al_draw_line(
                game.winW - GAME::FRAMERATE_HISTORY_SIZE + f + 0.5, 0,
                game.winW - GAME::FRAMERATE_HISTORY_SIZE + f + 0.5, fps_y,
                al_map_rgba(24, 96, 192, 192), 1
            );
        }
        float target_fps_y =
            interpolateNumber(
                game.options.advanced.targetFps,
                chart_min, chart_max,
                0, 100
            );
        al_draw_line(
            game.winW - GAME::FRAMERATE_HISTORY_SIZE, target_fps_y,
            game.winW, target_fps_y,
            al_map_rgba(128, 224, 128, 48), 1
        );
    }
}


/**
 * @brief Draws a leader's throw preview.
 */
void GameplayState::drawThrowPreview() {
    if(!curLeaderPtr) return;
    
    ALLEGRO_VERTEX vertexes[16];
    
    if(!curLeaderPtr->throwee) {
        //Just draw a simple line and leave.
        unsigned char n_vertexes =
            getThrowPreviewVertexes(
                vertexes, 0.0f, 1.0f,
                curLeaderPtr->pos, throwDest,
                changeAlpha(
                    game.config.aestheticGen.noPikminColor,
                    GAMEPLAY::PREVIEW_OPACITY / 2.0f
                ),
                0.0f, 1.0f, false
            );
            
        for(unsigned char v = 0; v < n_vertexes; v += 4) {
            al_draw_prim(
                vertexes, nullptr, nullptr,
                v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
            );
        }
        
        return;
    }
    
    //Check which edges exist near the throw.
    set<Edge*> candidate_edges;
    
    game.curAreaData->bmap.getEdgesInRegion(
        Point(
            std::min(curLeaderPtr->pos.x, throwDest.x),
            std::min(curLeaderPtr->pos.y, throwDest.y)
        ),
        Point(
            std::max(curLeaderPtr->pos.x, throwDest.x),
            std::max(curLeaderPtr->pos.y, throwDest.y)
        ),
        candidate_edges
    );
    
    float wall_collision_r = 2.0f;
    bool wall_is_blocking_sector = false;
    Distance leader_to_dest_dist(
        curLeaderPtr->pos, throwDest
    );
    float throw_h_angle = 0.0f;
    float throw_v_angle = 0.0f;
    float throw_speed = 0.0f;
    float throw_h_speed = 0.0f;
    coordinatesToAngle(
        curLeaderPtr->throweeSpeed, &throw_h_angle, &throw_h_speed
    );
    coordinatesToAngle(
        Point(throw_h_speed, curLeaderPtr->throweeSpeedZ),
        &throw_v_angle, &throw_speed
    );
    float texture_offset =
        fmod(
            areaTimePassed * GAMEPLAY::PREVIEW_TEXTURE_TIME_MULT,
            al_get_bitmap_width(game.sysContent.bmpThrowPreview) *
            GAMEPLAY::PREVIEW_TEXTURE_SCALE
        );
        
    //For each edge, check if it crosses the throw line.
    for(Edge* e : candidate_edges) {
        if(!e->sectors[0] || !e->sectors[1]) {
            continue;
        }
        
        float r = 0.0f;
        if(
            !lineSegsIntersect(
                curLeaderPtr->pos, throwDest,
                v2p(e->vertexes[0]), v2p(e->vertexes[1]),
                &r, nullptr
            )
        ) {
            //No collision.
            continue;
        }
        
        //If this is a blocking sector then yeah, collision.
        if(
            (
                e->sectors[0]->type == SECTOR_TYPE_BLOCKING ||
                e->sectors[1]->type == SECTOR_TYPE_BLOCKING
            ) &&
            r < wall_collision_r
        ) {
            wall_collision_r = r;
            wall_is_blocking_sector = true;
            continue;
        }
        
        //Otherwise, let's check for walls.
        
        if(e->sectors[0]->z == e->sectors[1]->z) {
            //Edges where both sectors have the same height have no wall.
            continue;
        }
        
        //Calculate the throwee's vertical position at that point.
        float edge_z = std::max(e->sectors[0]->z, e->sectors[1]->z);
        float x_at_edge =
            leader_to_dest_dist.toFloat() * r;
        float y_at_edge =
            tan(throw_v_angle) * x_at_edge -
            (
                -MOB::GRAVITY_ADDER /
                (
                    2 * throw_speed * throw_speed *
                    cos(throw_v_angle) * cos(throw_v_angle)
                )
            ) * x_at_edge * x_at_edge;
        y_at_edge += curLeaderPtr->z;
        
        //If the throwee would hit the wall at these coordinates, collision.
        if(edge_z >= y_at_edge && r < wall_collision_r) {
            wall_collision_r = r;
            wall_is_blocking_sector = false;
        }
    }
    
    /*
     * Time to draw. There are three possible scenarios.
     * 1. Nothing interrupts the throw, so we can draw directly from
     *   the leader to the throw destination.
     * 2. The throwee could never reach because it's too high, so draw the
     *   line colliding against the edge.
     * 3. The throwee will collide against a wall, but can theoretically reach
     *   the target, since it's within the height limit. After the wall
     *   collision, its trajectory is unpredictable.
     */
    
    if(wall_collision_r > 1.0f) {
        //No collision. Free throw.
        
        unsigned char n_vertexes =
            getThrowPreviewVertexes(
                vertexes, 0.0f, 1.0f,
                curLeaderPtr->pos, throwDest,
                changeAlpha(
                    curLeaderPtr->throwee->type->mainColor,
                    GAMEPLAY::PREVIEW_OPACITY
                ),
                texture_offset, GAMEPLAY::PREVIEW_TEXTURE_SCALE, true
            );
            
        for(unsigned char v = 0; v < n_vertexes; v += 4) {
            al_draw_prim(
                vertexes, nullptr, game.sysContent.bmpThrowPreview,
                v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
            );
        }
        
    } else {
        //Wall collision.
        
        Point collision_point(
            curLeaderPtr->pos.x +
            (throwDest.x - curLeaderPtr->pos.x) *
            wall_collision_r,
            curLeaderPtr->pos.y +
            (throwDest.y - curLeaderPtr->pos.y) *
            wall_collision_r
        );
        
        if(!curLeaderPtr->throweeCanReach || wall_is_blocking_sector) {
            //It's impossible to reach.
            
            unsigned char n_vertexes =
                getThrowPreviewVertexes(
                    vertexes, 0.0f, wall_collision_r,
                    curLeaderPtr->pos, throwDest,
                    changeAlpha(
                        curLeaderPtr->throwee->type->mainColor,
                        GAMEPLAY::PREVIEW_OPACITY
                    ),
                    texture_offset, GAMEPLAY::PREVIEW_TEXTURE_SCALE, true
                );
                
            for(unsigned char v = 0; v < n_vertexes; v += 4) {
                al_draw_prim(
                    vertexes, nullptr, game.sysContent.bmpThrowPreview,
                    v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            drawBitmap(
                game.sysContent.bmpThrowInvalid,
                collision_point, Point(32.0f), throw_h_angle,
                changeAlpha(
                    curLeaderPtr->throwee->type->mainColor,
                    GAMEPLAY::PREVIEW_OPACITY
                )
            );
            
        } else {
            //Trajectory is unknown after collision. Can theoretically reach.
            
            unsigned char n_vertexes =
                getThrowPreviewVertexes(
                    vertexes, 0.0f, wall_collision_r,
                    curLeaderPtr->pos, throwDest,
                    changeAlpha(
                        curLeaderPtr->throwee->type->mainColor,
                        GAMEPLAY::COLLISION_OPACITY
                    ),
                    texture_offset, GAMEPLAY::PREVIEW_TEXTURE_SCALE, true
                );
                
            for(unsigned char v = 0; v < n_vertexes; v += 4) {
                al_draw_prim(
                    vertexes, nullptr, game.sysContent.bmpThrowPreview,
                    v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            n_vertexes =
                getThrowPreviewVertexes(
                    vertexes, wall_collision_r, 1.0f,
                    curLeaderPtr->pos, throwDest,
                    changeAlpha(
                        curLeaderPtr->throwee->type->mainColor,
                        GAMEPLAY::PREVIEW_OPACITY
                    ),
                    0.0f, 1.0f, true
                );
                
            for(unsigned char v = 0; v < n_vertexes; v += 4) {
                al_draw_prim(
                    vertexes, nullptr, game.sysContent.bmpThrowPreviewDashed,
                    v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            drawBitmap(
                game.sysContent.bmpThrowInvalid,
                collision_point, Point(16.0f), throw_h_angle,
                changeAlpha(
                    curLeaderPtr->throwee->type->mainColor,
                    GAMEPLAY::PREVIEW_OPACITY
                )
            );
            
        }
    }
    
}


/**
 * @brief Draws the current area and mobs to a bitmap and returns it.
 *
 * @param settings What settings to use.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* GameplayState::drawToBitmap(
    const MakerTools::AreaImageSettings &settings
) {
    //First, get the full dimensions of the map.
    Point min_coords(FLT_MAX, FLT_MAX);
    Point max_coords(-FLT_MAX, -FLT_MAX);
    
    for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
        Vertex* v_ptr = game.curAreaData->vertexes[v];
        updateMinMaxCoords(
            min_coords, max_coords, v2p(v_ptr)
        );
    }
    
    //Figure out the scale that will fit on the image.
    float area_w = max_coords.x - min_coords.x + settings.padding;
    float area_h = max_coords.y - min_coords.y + settings.padding;
    float final_bmp_w = settings.size;
    float final_bmp_h = settings.size;
    float scale;
    
    if(area_w > area_h) {
        scale = settings.size / area_w;
        final_bmp_h *= area_h / area_w;
    } else {
        scale = settings.size / area_h;
        final_bmp_w *= area_w / area_h;
    }
    
    //Create the bitmap.
    ALLEGRO_BITMAP* bmp = al_create_bitmap(final_bmp_w, final_bmp_h);
    
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_translate_transform(
        &t,
        -min_coords.x + settings.padding / 2.0f,
        -min_coords.y + settings.padding / 2.0f
    );
    al_scale_transform(&t, scale, scale);
    
    //Begin drawing!
    doGameDrawing(bmp, &t);
    
    return bmp;
}


/**
 * @brief Draws tree shadows.
 */
void GameplayState::drawTreeShadows() {
    for(size_t s = 0; s < game.curAreaData->treeShadows.size(); s++) {
        TreeShadow* s_ptr = game.curAreaData->treeShadows[s];
        
        unsigned char alpha =
            (
                (s_ptr->alpha / 255.0) *
                game.curAreaData->weatherCondition.getSunStrength()
            ) * 255;
            
        drawBitmap(
            s_ptr->bitmap,
            Point(
                s_ptr->center.x + GAMEPLAY::TREE_SHADOW_SWAY_AMOUNT*
                cos(GAMEPLAY::TREE_SHADOW_SWAY_SPEED * areaTimePassed) *
                s_ptr->sway.x,
                s_ptr->center.y + GAMEPLAY::TREE_SHADOW_SWAY_AMOUNT*
                sin(GAMEPLAY::TREE_SHADOW_SWAY_SPEED * areaTimePassed) *
                s_ptr->sway.y
            ),
            s_ptr->size,
            s_ptr->angle, mapAlpha(alpha)
        );
    }
}


/**
 * @brief Draws the components that make up the game world:
 * layout, objects, etc.
 *
 * @param bmp_output If not nullptr, draw the area onto this.
 */
void GameplayState::drawWorldComponents(ALLEGRO_BITMAP* bmp_output) {
    ALLEGRO_BITMAP* custom_wall_offset_effect_buffer = nullptr;
    ALLEGRO_BITMAP* custom_liquid_limit_effect_buffer = nullptr;
    if(!bmp_output) {
        updateOffsetEffectBuffer(
            game.view.box[0], game.view.box[1],
            game.liquidLimitEffectCaches,
            game.liquidLimitEffectBuffer,
            true
        );
        updateOffsetEffectBuffer(
            game.view.box[0], game.view.box[1],
            game.wallSmoothingEffectCaches,
            game.wallOffsetEffectBuffer,
            true
        );
        updateOffsetEffectBuffer(
            game.view.box[0], game.view.box[1],
            game.wallShadowEffectCaches,
            game.wallOffsetEffectBuffer,
            false
        );
        
    } else {
        custom_liquid_limit_effect_buffer =
            al_create_bitmap(
                al_get_bitmap_width(bmp_output),
                al_get_bitmap_height(bmp_output)
            );
        custom_wall_offset_effect_buffer =
            al_create_bitmap(
                al_get_bitmap_width(bmp_output),
                al_get_bitmap_height(bmp_output)
            );
        updateOffsetEffectBuffer(
            Point(-FLT_MAX), Point(FLT_MAX),
            game.liquidLimitEffectCaches,
            custom_liquid_limit_effect_buffer,
            true
        );
        updateOffsetEffectBuffer(
            Point(-FLT_MAX), Point(FLT_MAX),
            game.wallSmoothingEffectCaches,
            custom_wall_offset_effect_buffer,
            true
        );
        updateOffsetEffectBuffer(
            Point(-FLT_MAX), Point(FLT_MAX),
            game.wallShadowEffectCaches,
            custom_wall_offset_effect_buffer,
            false
        );
        
    }
    
    vector<WorldComponent> components;
    //Let's reserve some space. We might need more or less,
    //but this is a nice estimate.
    components.reserve(
        game.curAreaData->sectors.size() + //Sectors.
        mobs.all.size() + //Mob shadows.
        mobs.all.size() + //Mobs.
        particles.getCount() //Particles.
    );
    
    //Sectors.
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* s_ptr = game.curAreaData->sectors[s];
        
        if(
            !bmp_output &&
            !rectanglesIntersect(
                s_ptr->bbox[0], s_ptr->bbox[1],
                game.view.box[0], game.view.box[1]
            )
        ) {
            //Off-camera.
            continue;
        }
        
        WorldComponent c;
        c.sector_ptr = s_ptr;
        c.z = s_ptr->z;
        components.push_back(c);
    }
    
    //Particles.
    particles.fillComponentList(components, game.view.box[0], game.view.box[1]);
    
    //Mobs.
    for(size_t m = 0; m < mobs.all.size(); m++) {
        Mob* mob_ptr = mobs.all[m];
        
        if(!bmp_output && mob_ptr->isOffCamera()) {
            //Off-camera.
            continue;
        }
        
        if(hasFlag(mob_ptr->flags, MOB_FLAG_HIDDEN)) continue;
        if(mob_ptr->isStoredInsideMob()) continue;
        
        //Shadows.
        if(
            mob_ptr->type->castsShadow &&
            !hasFlag(mob_ptr->flags, MOB_FLAG_SHADOW_INVISIBLE)
        ) {
            WorldComponent c;
            c.mob_shadow_ptr = mob_ptr;
            if(mob_ptr->standingOnMob) {
                c.z =
                    mob_ptr->standingOnMob->z +
                    mob_ptr->standingOnMob->getDrawingHeight();
            } else {
                c.z = mob_ptr->groundSector->z;
            }
            c.z += mob_ptr->getDrawingHeight() - 1;
            components.push_back(c);
        }
        
        //Limbs.
        if(mob_ptr->parent && mob_ptr->parent->limb_anim.animDb) {
            unsigned char method = mob_ptr->parent->limb_draw_method;
            WorldComponent c;
            c.mob_limb_ptr = mob_ptr;
            
            switch(method) {
            case LIMB_DRAW_METHOD_BELOW_BOTH: {
                c.z = std::min(mob_ptr->z, mob_ptr->parent->m->z);
                break;
            } case LIMB_DRAW_METHOD_BELOW_CHILD: {
                c.z = mob_ptr->z;
                break;
            } case LIMB_DRAW_METHOD_BELOW_PARENT: {
                c.z = mob_ptr->parent->m->z;
                break;
            } case LIMB_DRAW_METHOD_ABOVE_PARENT: {
                c.z =
                    mob_ptr->parent->m->z +
                    mob_ptr->parent->m->getDrawingHeight() +
                    0.001;
                break;
            } case LIMB_DRAW_METHOD_ABOVE_CHILD: {
                c.z = mob_ptr->z + mob_ptr->getDrawingHeight() + 0.001;
                break;
            } case LIMB_DRAW_METHOD_ABOVE_BOTH: {
                c.z =
                    std::max(
                        mob_ptr->parent->m->z +
                        mob_ptr->parent->m->getDrawingHeight() +
                        0.001,
                        mob_ptr->z + mob_ptr->getDrawingHeight() +
                        0.001
                    );
                break;
            }
            }
            
            components.push_back(c);
        }
        
        //The mob proper.
        WorldComponent c;
        c.mob_ptr = mob_ptr;
        c.z = mob_ptr->z + mob_ptr->getDrawingHeight();
        if(mob_ptr->holder.m && mob_ptr->holder.forceAboveHolder) {
            c.z += mob_ptr->holder.m->getDrawingHeight() + 1;
        }
        components.push_back(c);
    }
    
    //Time to draw!
    for(size_t c = 0; c < components.size(); c++) {
        components[c].idx = c;
    }
    
    sort(
        components.begin(), components.end(),
    [] (const WorldComponent & c1, const WorldComponent & c2) -> bool {
        if(c1.z == c2.z) {
            return c1.idx < c2.idx;
        }
        return c1.z < c2.z;
    }
    );
    
    float mob_shadow_stretch = 0;
    
    if(dayMinutes < 60 * 5 || dayMinutes > 60 * 20) {
        mob_shadow_stretch = 1;
    } else if(dayMinutes < 60 * 12) {
        mob_shadow_stretch = 1 - ((dayMinutes - 60 * 5) / (60 * 12 - 60 * 5));
    } else {
        mob_shadow_stretch = (dayMinutes - 60 * 12) / (60 * 20 - 60 * 12);
    }
    
    for(size_t c = 0; c < components.size(); c++) {
        WorldComponent* c_ptr = &components[c];
        
        if(c_ptr->sector_ptr) {
        
            bool has_liquid = false;
            if(c_ptr->sector_ptr->hazard && c_ptr->sector_ptr->hazard->associatedLiquid) {
                drawLiquid(
                    c_ptr->sector_ptr,
                    c_ptr->sector_ptr->hazard->associatedLiquid,
                    Point(),
                    1.0f,
                    areaTimePassed
                );
                has_liquid = true;
            }
            if(!has_liquid) {
                drawSectorTexture(c_ptr->sector_ptr, Point(), 1.0f, 1.0f);
            }
            float liquid_opacity_mult = 1.0f;
            if(c_ptr->sector_ptr->drainingLiquid) {
                liquid_opacity_mult =
                    c_ptr->sector_ptr->liquidDrainLeft /
                    GEOMETRY::LIQUID_DRAIN_DURATION;
            }
            drawSectorEdgeOffsets(
                c_ptr->sector_ptr,
                bmp_output ?
                custom_liquid_limit_effect_buffer :
                game.liquidLimitEffectBuffer,
                liquid_opacity_mult
            );
            drawSectorEdgeOffsets(
                c_ptr->sector_ptr,
                bmp_output ?
                custom_wall_offset_effect_buffer :
                game.wallOffsetEffectBuffer,
                1.0f
            );
            
        } else if(c_ptr->mob_shadow_ptr) {
        
            float delta_z = 0;
            if(!c_ptr->mob_shadow_ptr->standingOnMob) {
                delta_z =
                    c_ptr->mob_shadow_ptr->z -
                    c_ptr->mob_shadow_ptr->groundSector->z;
            }
            drawMobShadow(
                c_ptr->mob_shadow_ptr,
                delta_z,
                mob_shadow_stretch
            );
            
        } else if(c_ptr->mob_limb_ptr) {
        
            if(!hasFlag(c_ptr->mob_limb_ptr->flags, MOB_FLAG_HIDDEN)) {
                c_ptr->mob_limb_ptr->drawLimb();
            }
            
        } else if(c_ptr->mob_ptr) {
        
            if(!hasFlag(c_ptr->mob_ptr->flags, MOB_FLAG_HIDDEN)) {
                c_ptr->mob_ptr->drawMob();
                if(c_ptr->mob_ptr->type->drawMobCallback) {
                    c_ptr->mob_ptr->type->drawMobCallback(c_ptr->mob_ptr);
                }
            }
            
        } else if(c_ptr->particle_ptr) {
        
            c_ptr->particle_ptr->draw();
            
        }
    }
    
    if(bmp_output) {
        al_destroy_bitmap(custom_wall_offset_effect_buffer);
    }
}
