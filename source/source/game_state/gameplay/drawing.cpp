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
#include "../../content/mob/resource.h"
#include "../../content/mob/scale.h"
#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


#pragma warning(disable: 4701)


/**
 * @brief Does the drawing for the main game loop.
 *
 * @param bmpOutput If not nullptr, draw the area onto this.
 * @param bmpTransform Transformation to use when drawing to a bitmap.
 * @param bmpSettings Settings to use when drawing to a bitmap.
 */
void GameplayState::doGameDrawing(
    ALLEGRO_BITMAP* bmpOutput, const ALLEGRO_TRANSFORM* bmpTransform,
    const MakerTools::AreaImageSettings& bmpSettings
) {

    /*  ***************************************
      *** |  |                           |  | ***
    ***** |__|          DRAWING          |__| *****
      ***  \/                             \/  ***
        ***************************************/
    
    ALLEGRO_TRANSFORM oldWorldToWindowTransform;
    int blendOldOp, blendOldSrc, blendOldDst,
        blendOldAOp, blendOldASrc, blendOldADst;
        
    if(bmpOutput) {
        oldWorldToWindowTransform = players[0].view.worldToWindowTransform;
        players[0].view.worldToWindowTransform = *bmpTransform;
        al_set_target_bitmap(bmpOutput);
        al_get_separate_blender(
            &blendOldOp, &blendOldSrc, &blendOldDst,
            &blendOldAOp, &blendOldASrc, &blendOldADst
        );
        al_set_separate_blender(
            ALLEGRO_ADD, ALLEGRO_ALPHA,
            ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD,
            ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA
        );
    }
    
    for(size_t p = 0; p < players.size(); p++) {
        Player& player = players[p];
        al_clear_to_color(game.curAreaData->bgColor);
        
        //Layer 1 -- Background.
        if(game.perfMon) {
            game.perfMon->startMeasurement("Drawing -- Background");
        }
        drawBackground(player.view, bmpOutput);
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        //Layer 2 -- World components.
        if(game.perfMon) {
            game.perfMon->startMeasurement("Drawing -- World");
        }
        al_use_transform(&player.view.worldToWindowTransform);
        drawWorldComponents(player.view, bmpOutput);
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        //Layer 3 -- In-game text.
        if(game.perfMon) {
            game.perfMon->startMeasurement("Drawing -- In-game text");
        }
        if(!bmpOutput && game.makerTools.hud) {
            drawInGameText(&player);
        }
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        //Layer 4 -- Precipitation.
        if(game.perfMon) {
            game.perfMon->startMeasurement("Drawing -- precipitation");
        }
        if(!bmpOutput) {
            drawPrecipitation();
        }
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        //Layer 5 -- Tree shadows.
        if(game.perfMon) {
            game.perfMon->startMeasurement("Drawing -- Tree shadows");
        }
        if(!(bmpOutput && !bmpSettings.shadows)) {
            drawTreeShadows();
        }
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        //Finish dumping to a bitmap image here.
        if(bmpOutput) {
            al_set_separate_blender(
                blendOldOp, blendOldSrc, blendOldDst,
                blendOldAOp, blendOldASrc, blendOldADst
            );
            players[0].view.worldToWindowTransform = oldWorldToWindowTransform;
            al_set_target_backbuffer(game.display);
            return;
        }
        
        //Layer 6 -- Lighting filter.
        if(game.perfMon) {
            game.perfMon->startMeasurement("Drawing -- Lighting");
        }
        drawLightingFilter(player.view);
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        //Layer 7 -- Leader cursor.
        al_use_transform(&player.view.worldToWindowTransform);
        ALLEGRO_COLOR cursorColor = game.config.aestheticGen.noPikminColor;
        if(player.closestGroupMember[BUBBLE_RELATION_CURRENT]) {
            cursorColor =
                player.closestGroupMember[BUBBLE_RELATION_CURRENT]->
                type->mainColor;
        }
        if(player.leaderPtr && game.makerTools.hud) {
            cursorColor =
                changeColorLighting(cursorColor, player.leaderCursorHeightDiffLight);
            drawLeaderCursor(&player, cursorColor);
        }
        
        //Layer 8 -- HUD.
        al_use_transform(&game.identityTransform);
        
        if(game.perfMon) {
            game.perfMon->startMeasurement("Drawing -- HUD");
        }
        
        if(game.makerTools.hud) {
            player.hud->gui.draw();
            player.inventory->gui.draw();
            
            drawBigMsg();
            
            if(msgBox) {
                drawGameplayMessageBox();
            } else if(onionMenu) {
                drawOnionMenu();
            } else if(pauseMenu) {
                drawPauseMenu();
            } else {
                drawMouseCursor(cursorColor);
            }
        }
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
    }
    
    //Layer 9 -- System stuff.
    if(game.makerTools.hud) {
        if(areaTitleFadeTimer.timeLeft > 0) {
            drawLoadingScreen(
                game.curAreaData->name,
                getSubtitleOrMissionGoal(
                    game.curAreaData->subtitle,
                    game.curAreaData->type,
                    game.curAreaData->mission.goal
                ),
                game.curAreaData->maker,
                areaTitleFadeTimer.getRatioLeft()
            );
        }
        
    }
    
    drawDebugTools(&players[0]);
}


#pragma warning(default: 4701)


/**
 * @brief Draws the area background.
 *
 * @param view Viewport to draw to.
 * @param bmpOutput If not nullptr, draw the background onto this.
 */
void GameplayState::drawBackground(
    const Viewport& view, ALLEGRO_BITMAP* bmpOutput
) {
    if(!game.curAreaData->bgBmp) return;
    
    ALLEGRO_VERTEX bgV[4];
    for(unsigned char v = 0; v < 4; v++) {
        bgV[v].color = COLOR_WHITE;
        bgV[v].z = 0;
    }
    
    //Not gonna lie, this uses some fancy-shmancy numbers.
    //I mostly got here via trial and error.
    //I apologize if you're trying to understand what it means.
    int bmpW = bmpOutput ? al_get_bitmap_width(bmpOutput) : view.size.x;
    int bmpH = bmpOutput ? al_get_bitmap_height(bmpOutput) : view.size.y;
    float zoomToUse = bmpOutput ? 0.5 : view.cam.zoom;
    Point finalZoom(
        bmpW * 0.5 * game.curAreaData->bgDist / zoomToUse,
        bmpH * 0.5 * game.curAreaData->bgDist / zoomToUse
    );
    
    bgV[0].x =
        0;
    bgV[0].y =
        0;
    bgV[0].u =
        (view.cam.pos.x - finalZoom.x) / game.curAreaData->bgBmpZoom;
    bgV[0].v =
        (view.cam.pos.y - finalZoom.y) / game.curAreaData->bgBmpZoom;
    bgV[1].x =
        bmpW;
    bgV[1].y =
        0;
    bgV[1].u =
        (view.cam.pos.x + finalZoom.x) / game.curAreaData->bgBmpZoom;
    bgV[1].v =
        (view.cam.pos.y - finalZoom.y) / game.curAreaData->bgBmpZoom;
    bgV[2].x =
        bmpW;
    bgV[2].y =
        bmpH;
    bgV[2].u =
        (view.cam.pos.x + finalZoom.x) / game.curAreaData->bgBmpZoom;
    bgV[2].v =
        (view.cam.pos.y + finalZoom.y) / game.curAreaData->bgBmpZoom;
    bgV[3].x =
        0;
    bgV[3].y =
        bmpH;
    bgV[3].u =
        (view.cam.pos.x - finalZoom.x) / game.curAreaData->bgBmpZoom;
    bgV[3].v =
        (view.cam.pos.y + finalZoom.y) / game.curAreaData->bgBmpZoom;
        
    al_draw_prim(
        bgV, nullptr, game.curAreaData->bgBmp,
        0, 4, ALLEGRO_PRIM_TRIANGLE_FAN
    );
}


/**
 * @brief Draws the current big message, if any.
 */
void GameplayState::drawBigMsg() {
    switch(bigMsg.get()) {
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
        const float t = bigMsg.getTime() / GAMEPLAY::BIG_MSG_READY_DUR;
        
        KeyframeInterpolator<float> kiY(game.winH * (-0.2f));
        kiY.add(TEXT_START_T, game.winH * (-0.2f));
        kiY.add(TEXT_MOVE_MID_T, game.winH * 0.40f, EASE_METHOD_IN);
        kiY.add(TEXT_PAUSE_T, game.winH / 2.0f, EASE_METHOD_OUT_ELASTIC);
        kiY.add(TEXT_SHRINK_T, game.winH / 2.0f);
        KeyframeInterpolator<float> kiH(TEXT_INITIAL_HEIGHT);
        kiH.add(TEXT_SHRINK_T, TEXT_INITIAL_HEIGHT * 1.4f);
        kiH.add(1.0f, 0.0f, EASE_METHOD_IN);
        
        for(size_t c = 0; c < GAMEPLAY::BIG_MSG_READY_TEXT.size(); c++) {
            float charRatio =
                c / ((float) GAMEPLAY::BIG_MSG_READY_TEXT.size() - 1);
            charRatio = 1.0f - charRatio;
            float xOffset = (TEXT_W / 2.0f) - (TEXT_W * charRatio);
            float y = kiY.get(t + charRatio * TEXT_VARIATION_DUR);
            drawText(
                string(1, GAMEPLAY::BIG_MSG_READY_TEXT[c]),
                game.sysContent.fntAreaName,
                Point((game.winW / 2.0f) + xOffset, y),
                Point(LARGE_FLOAT, game.winH * kiH.get(t)), COLOR_GOLD
            );
        }
        break;
        
    } case BIG_MESSAGE_GO: {

        const float TEXT_GROW_STOP_T = 0.10f;
        const float t = bigMsg.getTime() / GAMEPLAY::BIG_MSG_GO_DUR;
        
        KeyframeInterpolator<float> kiH(0.0f);
        kiH.add(TEXT_GROW_STOP_T, 0.20f, EASE_METHOD_OUT_ELASTIC);
        kiH.add(1.0f, 0.22f);
        KeyframeInterpolator<float> kiA(1.0f);
        kiA.add(TEXT_GROW_STOP_T, 1.0f);
        kiA.add(1.0f, 0.0f);
        
        drawText(
            GAMEPLAY::BIG_MSG_GO_TEXT,
            game.sysContent.fntAreaName,
            Point(game.winW / 2.0f, game.winH / 2.0f),
            Point(LARGE_FLOAT, game.winH * kiH.get(t)),
            changeAlpha(COLOR_GOLD, 255 * kiA.get(t))
        );
        break;
        
    } case BIG_MESSAGE_ONE_MIN_LEFT: {
        const float TEXT_W = game.winW * 0.70f;
        const float TEXT_VARIATION_DUR = 0.04f;
        const float TEXT_MOVE_STOP_T = 0.25f;
        const float TEXT_MOVE_AGAIN_T = 0.66f;
        const float TEXT_DRIFT_START_X = game.winW * 0.005f;
        const float TEXT_DRIFT_END_X = game.winW * -0.005f;
        const float t = bigMsg.getTime() / GAMEPLAY::BIG_MSG_ONE_MIN_LEFT_DUR;
        
        KeyframeInterpolator<float> kiX(game.winW);
        kiX.add(TEXT_MOVE_STOP_T, TEXT_DRIFT_START_X, EASE_METHOD_IN_OUT_BACK);
        kiX.add(TEXT_MOVE_AGAIN_T, TEXT_DRIFT_END_X);
        kiX.add(1.0f, -(float) game.winW, EASE_METHOD_IN_OUT_BACK);
        
        for(size_t c = 0; c < GAMEPLAY::BIG_MSG_ONE_MIN_LEFT_TEXT.size(); c++) {
            float charRatio =
                c / ((float) GAMEPLAY::BIG_MSG_ONE_MIN_LEFT_TEXT.size() - 1);
            charRatio = 1.0f - charRatio;
            float xOffset = (TEXT_W / 2.0f) - (TEXT_W * charRatio);
            float x = kiX.get(t + charRatio * TEXT_VARIATION_DUR);
            drawText(
                string(1, GAMEPLAY::BIG_MSG_ONE_MIN_LEFT_TEXT[c]),
                game.sysContent.fntAreaName,
                Point((game.winW / 2.0f) + xOffset + x, game.winH / 2.0f),
                Point(LARGE_FLOAT, game.winH * 0.08f), COLOR_GOLD
            );
        }
        break;
        
    } case BIG_MESSAGE_MISSION_CLEAR:
    case BIG_MESSAGE_MISSION_FAILED: {
        const string& TEXT =
            bigMsg.get() == BIG_MESSAGE_MISSION_CLEAR ?
            GAMEPLAY::BIG_MSG_MISSION_CLEAR_TEXT :
            GAMEPLAY::BIG_MSG_MISSION_FAILED_TEXT;
        const float TEXT_W = game.winW * 0.80f;
        const float TEXT_INITIAL_HEIGHT = 0.05f;
        const float TEXT_VARIATION_DUR = 0.08f;
        const float TEXT_MOVE_MID_T = 0.30f;
        const float TEXT_PAUSE_T = 0.50f;
        const float TEXT_FADE_T = 0.90f;
        const float t =
            bigMsg.get() == BIG_MESSAGE_MISSION_CLEAR ?
            (bigMsg.getTime() / GAMEPLAY::BIG_MSG_MISSION_CLEAR_DUR) :
            (bigMsg.getTime() / GAMEPLAY::BIG_MSG_MISSION_FAILED_DUR);
            
        KeyframeInterpolator<float> kiY(game.winH * (-0.2f));
        kiY.add(TEXT_MOVE_MID_T, game.winH * 0.40f, EASE_METHOD_IN);
        kiY.add(TEXT_PAUSE_T, game.winH / 2.0f, EASE_METHOD_OUT_ELASTIC);
        KeyframeInterpolator<float> kiH(TEXT_INITIAL_HEIGHT);
        kiH.add(1.0f, TEXT_INITIAL_HEIGHT * 1.4f, EASE_METHOD_IN);
        KeyframeInterpolator<float> kiA(1.0f);
        kiA.add(TEXT_FADE_T, 1.0f);
        kiA.add(1.0f, 0.0f);
        
        float alpha = kiA.get(t);
        
        for(size_t c = 0; c < TEXT.size(); c++) {
            float charRatio = c / ((float) TEXT.size() - 1);
            charRatio = 1.0f - charRatio;
            float xOffset = (TEXT_W / 2.0f) - (TEXT_W * charRatio);
            float y = kiY.get(t + charRatio * TEXT_VARIATION_DUR);
            
            drawText(
                string(1, TEXT[c]), game.sysContent.fntAreaName,
                Point((game.winW / 2.0f) + xOffset, y),
                Point(LARGE_FLOAT, game.winH * kiH.get(t)),
                changeAlpha(COLOR_GOLD, 255 * alpha)
            );
        }
        break;
        
    }
    }
}


/**
 * @brief Draws any debug visualization tools useful for engine debugging.
 *
 * @param player Player that the view belongs to.
 */
void GameplayState::drawDebugTools(Player* player) {
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
    
    Point rawStickCoords;
    rawStickCoords.x = game.controls.mgr.rawSticks[0][0][0];
    rawStickCoords.y = game.controls.mgr.rawSticks[0][0][1];
    float rawStickAngle;
    float rawStickMag;
    coordinatesToAngle(
        rawStickCoords, &rawStickAngle, &rawStickMag
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
        rawStickMag >= 0.99f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    al_draw_line(
        RAW_STICK_VIEWER_X,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE / 2.0f,
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE / 2.0f,
        fabs(rawStickCoords.y) <= 0.01f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    al_draw_line(
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE / 2.0f,
        RAW_STICK_VIEWER_Y,
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE / 2.0f,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE,
        fabs(rawStickCoords.x) <= 0.01f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    Point rawDrawCoords =
        rawStickCoords * RAW_STICK_VIEWER_SIZE / 2.0f;
    al_draw_filled_circle(
        RAW_STICK_VIEWER_X + RAW_STICK_VIEWER_SIZE / 2.0f +
        rawDrawCoords.x,
        RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE / 2.0f +
        rawDrawCoords.y,
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
        game.sysContent.fntBuiltin,
        al_map_rgb(255, 64, 64),
        RAW_STICK_VIEWER_X, RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE + 1,
        ALLEGRO_ALIGN_LEFT,
        (
            resizeString(
                (rawStickCoords.x >= 0.0f ? " " : "") +
                f2s(rawStickCoords.x), 6
            ) + " " + resizeString(
                (rawStickCoords.y >= 0.0f ? " " : "") +
                f2s(rawStickCoords.y), 6
            )
        ).c_str()
    );
    al_draw_text(
        game.sysContent.fntBuiltin,
        al_map_rgb(255, 64, 64),
        RAW_STICK_VIEWER_X, RAW_STICK_VIEWER_Y + RAW_STICK_VIEWER_SIZE + 1 + 8,
        ALLEGRO_ALIGN_LEFT,
        (
            resizeString(
                (rawStickAngle >= 0.0f ? " " : "") +
                f2s(rawStickAngle), 6
            ) + " " + resizeString(
                (rawStickMag >= 0.0f ? " " : "") +
                f2s(rawStickMag), 6
            )
        ).c_str()
    );
    */
    
    //Clean analog stick viewer.
    /*
    const float CLEAN_STICK_VIEWER_X = 116;
    const float CLEAN_STICK_VIEWER_Y = 8;
    const float CLEAN_STICK_VIEWER_SIZE = 100;
    
    Point cleanStickCoords;
    cleanStickCoords.x =
        game.controls.getPlayerActionTypeValue(PLAYER_ACTION_TYPE_RIGHT) -
        game.controls.getPlayerActionTypeValue(PLAYER_ACTION_TYPE_LEFT);
    cleanStickCoords.y =
        game.controls.getPlayerActionTypeValue(PLAYER_ACTION_TYPE_DOWN) -
        game.controls.getPlayerActionTypeValue(PLAYER_ACTION_TYPE_UP);
    float cleanStickAngle;
    float cleanStickMag;
    coordinatesToAngle(
        cleanStickCoords, &cleanStickAngle, &cleanStickMag
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
        cleanStickMag >= 0.99f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    al_draw_line(
        CLEAN_STICK_VIEWER_X,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        fabs(cleanStickCoords.y) <= 0.01f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    al_draw_line(
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        CLEAN_STICK_VIEWER_Y,
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE / 2.0f,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE,
        fabs(cleanStickCoords.x) <= 0.01f ?
        al_map_rgba(240, 64, 64, 200) :
        al_map_rgba(240, 240, 240, 200),
        1
    );
    Point cleanDrawCoords =
        cleanStickCoords * CLEAN_STICK_VIEWER_SIZE / 2.0f;
    al_draw_filled_circle(
        CLEAN_STICK_VIEWER_X + CLEAN_STICK_VIEWER_SIZE / 2.0f +
        cleanDrawCoords.x,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE / 2.0f +
        cleanDrawCoords.y,
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
        game.sysContent.fntBuiltin,
        al_map_rgb(255, 64, 64),
        CLEAN_STICK_VIEWER_X,
        CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE + 1,
        ALLEGRO_ALIGN_LEFT,
        (
            resizeString(
                (cleanStickCoords.x >= 0.0f ? " " : "") +
                f2s(cleanStickCoords.x), 6
            ) + " " + resizeString(
                (cleanStickCoords.y >= 0.0f ? " " : "") +
                f2s(cleanStickCoords.y), 6
            )
        ).c_str()
    );
    al_draw_text(
        game.sysContent.fntBuiltin,
        al_map_rgb(255, 64, 64),
        CLEAN_STICK_VIEWER_X, CLEAN_STICK_VIEWER_Y + CLEAN_STICK_VIEWER_SIZE +
        1 + 8,
        ALLEGRO_ALIGN_LEFT,
        (
            resizeString(
                (cleanStickAngle >= 0.0f ? " " : "") +
                f2s(cleanStickAngle), 6
            ) + " " + resizeString(
                (cleanStickMag >= 0.0f ? " " : "") +
                f2s(cleanStickMag), 6
            )
        ).c_str()
    );
    */
    
    //Group stuff.
    if(game.debug.showGroupInfo && player->leaderPtr) {
        al_use_transform(&player->view.worldToWindowTransform);
        for(size_t m = 0; m < player->leaderPtr->group->members.size(); m++) {
            Point offset = player->leaderPtr->group->getSpotOffset(m);
            al_draw_filled_circle(
                player->leaderPtr->group->anchor.x + offset.x,
                player->leaderPtr->group->anchor.y + offset.y,
                3.0f,
                al_map_rgba(0, 0, 0, 192)
            );
        }
        al_draw_circle(
            player->leaderPtr->group->anchor.x,
            player->leaderPtr->group->anchor.y,
            3.0f,
            player->leaderPtr->group->mode == Group::MODE_SHUFFLE ?
            al_map_rgba(0, 255, 0, 192) :
            player->leaderPtr->group->mode == Group::MODE_FOLLOW_BACK ?
            al_map_rgba(255, 255, 0, 192) :
            al_map_rgba(255, 0, 0, 192),
            2.0f
        );
        
        Point groupMidPoint =
            player->leaderPtr->group->anchor +
            rotatePoint(
                Point(player->leaderPtr->group->radius, 0.0f),
                player->leaderPtr->group->anchorAngle
            );
        al_draw_filled_circle(
            groupMidPoint.x,
            groupMidPoint.y,
            3.0f,
            al_map_rgb(0, 0, 255)
        );
        al_use_transform(&game.identityTransform);
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
    float transitionRatio =
        msgBox->transitionIn ?
        msgBox->transitionTimer / GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME :
        (1 - msgBox->transitionTimer / GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME);
    int lineHeight = al_get_font_line_height(game.sysContent.fntStandard);
    float boxHeight = lineHeight * 4;
    float offset =
        boxHeight * ease(EASE_METHOD_IN, transitionRatio);
        
    //Draw a rectangle to darken gameplay.
    al_draw_filled_rectangle(
        0.0f, 0.0f,
        game.winW, game.winH,
        al_map_rgba(0, 0, 0, 64 * (1 - transitionRatio))
    );
    
    //Draw the message box proper.
    drawTexturedBox(
        Point(
            game.winW / 2,
            game.winH - (boxHeight / 2.0f) - 4 + offset
        ),
        Point(game.winW - 16, boxHeight),
        game.sysContent.bmpBubbleBox
    );
    
    //Draw the speaker's icon, if any.
    if(msgBox->speakerIcon) {
        drawBitmap(
            msgBox->speakerIcon,
            Point(
                40,
                game.winH - boxHeight - 16 + offset
            ),
            Point(48.0f)
        );
        drawBitmap(
            players[0].hud->bmpBubble,
            Point(
                40,
                game.winH - boxHeight - 16 + offset
            ),
            Point(64.0f)
        );
    }
    
    //Draw the button to advance, if it's time.
    float advanceButtonYOffset =
        sin(
            msgBox->totalTokenAnimTime *
            GAMEPLAY_MSG_BOX::BUTTON_OFFSET_TIME_MULT
        ) * GAMEPLAY_MSG_BOX::BUTTON_OFFSET_MULT;
    drawPlayerInputSourceIcon(
        game.sysContent.fntSlim,
        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).inputSource,
        true,
        Point(
            game.winW -
            (GAMEPLAY_MSG_BOX::MARGIN + GAMEPLAY_MSG_BOX::PADDING + 8.0f),
            game.winH -
            (GAMEPLAY_MSG_BOX::MARGIN + GAMEPLAY_MSG_BOX::PADDING + 8.0f) +
            offset + advanceButtonYOffset
        ),
        Point(32.0f),
        mapAlpha(msgBox->advanceButtonAlpha * 255)
    );
    
    //Draw the message's text.
    size_t tokenIdx = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t lineIdx = msgBox->curSection * 3 + l;
        if(lineIdx >= msgBox->tokensPerLine.size()) {
            break;
        }
        
        //Figure out what scaling is necessary, if any.
        unsigned int totalWidth = 0;
        float xScale = 1.0f;
        for(size_t t = 0; t < msgBox->tokensPerLine[lineIdx].size(); t++) {
            totalWidth += msgBox->tokensPerLine[lineIdx][t].width;
        }
        const float maxTextWidth =
            (GAMEPLAY_MSG_BOX::MARGIN + GAMEPLAY_MSG_BOX::PADDING) * 2;
        if(totalWidth > game.winW - maxTextWidth) {
            xScale = (game.winW - maxTextWidth) / totalWidth;
        }
        
        float caret =
            GAMEPLAY_MSG_BOX::MARGIN + GAMEPLAY_MSG_BOX::PADDING;
        float startY =
            game.winH - lineHeight * 4 + GAMEPLAY_MSG_BOX::PADDING + offset;
            
        for(size_t t = 0; t < msgBox->tokensPerLine[lineIdx].size(); t++) {
            tokenIdx++;
            if(tokenIdx >= msgBox->curToken) break;
            StringToken& curToken = msgBox->tokensPerLine[lineIdx][t];
            
            float x = caret;
            float y = startY + lineHeight * l;
            unsigned char alpha = 255;
            float thisTokenAnimTime;
            
            //Change the token's position and alpha, if it needs animating.
            //First, check for the typing animation.
            if(tokenIdx >= msgBox->skippedAtToken) {
                thisTokenAnimTime = msgBox->totalSkipAnimTime;
            } else {
                thisTokenAnimTime =
                    msgBox->totalTokenAnimTime -
                    (
                        (tokenIdx + 1) *
                        game.config.aestheticGen.gameplayMsgChInterval
                    );
            }
            if(
                thisTokenAnimTime > 0 &&
                thisTokenAnimTime < GAMEPLAY_MSG_BOX::TOKEN_ANIM_DURATION
            ) {
                float ratio =
                    thisTokenAnimTime / GAMEPLAY_MSG_BOX::TOKEN_ANIM_DURATION;
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
                    1.0f -
                    (
                        msgBox->swipeTimer /
                        GAMEPLAY_MSG_BOX::TOKEN_SWIPE_DURATION
                    );
                x += GAMEPLAY_MSG_BOX::TOKEN_SWIPE_X_AMOUNT * ratio;
                y += GAMEPLAY_MSG_BOX::TOKEN_SWIPE_Y_AMOUNT * ratio;
                alpha = std::max(0, (signed int) (alpha - ratio * 255));
            }
            
            //Actually draw it now.
            float tokenFinalWidth = curToken.width * xScale;
            switch(curToken.type) {
            case STRING_TOKEN_CHAR: {
                drawText(
                    curToken.content, game.sysContent.fntStandard,
                    Point(x, y),
                    Point(tokenFinalWidth, LARGE_FLOAT),
                    mapAlpha(alpha),
                    ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP, 0,
                    Point(xScale, 1.0f)
                );
                break;
            }
            case STRING_TOKEN_BIND_INPUT: {
                drawPlayerInputSourceIcon(
                    game.sysContent.fntSlim,
                    game.controls.findBind(curToken.content).inputSource,
                    true,
                    Point(
                        x + tokenFinalWidth / 2.0f,
                        y + lineHeight / 2.0f
                    ),
                    Point(tokenFinalWidth, lineHeight)
                );
                break;
            }
            default: {
                break;
            }
            }
            caret += tokenFinalWidth;
        }
    }
}


/**
 * @brief Draws the in-game text.
 *
 * @param player Player whose viewport to draw to.
 */
void GameplayState::drawInGameText(Player* player) {
    //Mob things.
    size_t nMobs = mobs.all.size();
    for(size_t m = 0; m < nMobs; m++) {
        Mob* mobPtr = mobs.all[m];
        
        //Fractions and health.
        if(mobPtr->healthWheel) {
            mobPtr->healthWheel->draw();
        }
        if(mobPtr->fraction) {
            mobPtr->fraction->draw();
        }
        
        //Maker tool -- draw hitboxes.
        if(game.makerTools.hitboxes) {
            Sprite* s;
            mobPtr->getSpriteData(&s, nullptr, nullptr);
            if(s) {
                for(size_t h = 0; h < s->hitboxes.size(); h++) {
                    Hitbox* hPtr = &s->hitboxes[h];
                    ALLEGRO_COLOR hc;
                    switch(hPtr->type) {
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
                        mobPtr->pos + rotatePoint(hPtr->pos, mobPtr->angle);
                    al_draw_filled_circle(p.x, p.y, hPtr->radius, hc);
                }
            }
        }
        
        //Maker tool -- draw collision.
        if(game.makerTools.collision) {
            if(mobPtr->type->pushesWithHitboxes) {
                Sprite* s;
                mobPtr->getSpriteData(&s, nullptr, nullptr);
                if(s) {
                    for(size_t h = 0; h < s->hitboxes.size(); h++) {
                        Hitbox* hPtr = &s->hitboxes[h];
                        Point p =
                            mobPtr->pos +
                            rotatePoint(hPtr->pos, mobPtr->angle);
                        al_draw_circle(
                            p.x, p.y,
                            hPtr->radius, COLOR_WHITE, 1
                        );
                    }
                }
            } else if(mobPtr->rectangularDim.x != 0) {
                Point tl(
                    -mobPtr->rectangularDim.x / 2.0f,
                    -mobPtr->rectangularDim.y / 2.0f
                );
                Point br(
                    mobPtr->rectangularDim.x / 2.0f,
                    mobPtr->rectangularDim.y / 2.0f
                );
                vector<Point> rectVertices {
                    rotatePoint(tl, mobPtr->angle) +
                    mobPtr->pos,
                    rotatePoint(Point(tl.x, br.y),  mobPtr->angle) +
                    mobPtr->pos,
                    rotatePoint(br, mobPtr->angle) +
                    mobPtr->pos,
                    rotatePoint(Point(br.x, tl.y), mobPtr->angle) +
                    mobPtr->pos
                };
                float vertices[] {
                    rectVertices[0].x,
                    rectVertices[0].y,
                    rectVertices[1].x,
                    rectVertices[1].y,
                    rectVertices[2].x,
                    rectVertices[2].y,
                    rectVertices[3].x,
                    rectVertices[3].y
                };
                
                al_draw_polygon(vertices, 4, 0, COLOR_WHITE, 1, 10);
            } else {
                al_draw_circle(
                    mobPtr->pos.x, mobPtr->pos.y,
                    mobPtr->radius, COLOR_WHITE, 1
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
        Point targetPos =
            hasFlag(path->settings.flags, PATH_FOLLOW_FLAG_FOLLOW_MOB) ?
            path->settings.targetMob->pos :
            path->settings.targetPoint;
            
        if(!path->path.empty()) {
        
            //Faint lines for the entire path.
            for(size_t s = 0; s < path->path.size() - 1; s++) {
                bool isBlocked = false;
                PathLink* lPtr = path->path[s]->getLink(path->path[s + 1]);
                auto lIt = pathMgr.obstructions.find(lPtr);
                if(lIt != pathMgr.obstructions.end()) {
                    isBlocked = !lIt->second.empty();
                }
                
                al_draw_line(
                    path->path[s]->pos.x,
                    path->path[s]->pos.y,
                    path->path[s + 1]->pos.x,
                    path->path[s + 1]->pos.y,
                    isBlocked ?
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
            path->isDirect() ||
            path->curPathStopIdx == path->path.size()
        ) {
            bool isBlocked = path->blockReason != PATH_BLOCK_REASON_NONE;
            //Line directly to the target.
            al_draw_line(
                game.makerTools.infoLock->pos.x,
                game.makerTools.infoLock->pos.y,
                targetPos.x,
                targetPos.y,
                isBlocked ?
                al_map_rgba(255, 0, 0, 200) :
                al_map_rgba(0, 0, 255, 200),
                4.0f
            );
        } else if(path->curPathStopIdx < path->path.size()) {
            bool isBlocked = path->blockReason != PATH_BLOCK_REASON_NONE;
            //Line to the next stop, and circle for the next stop in blue.
            al_draw_line(
                game.makerTools.infoLock->pos.x,
                game.makerTools.infoLock->pos.y,
                path->path[path->curPathStopIdx]->pos.x,
                path->path[path->curPathStopIdx]->pos.y,
                isBlocked ?
                al_map_rgba(255, 0, 0, 200) :
                al_map_rgba(0, 0, 255, 200),
                4.0f
            );
            al_draw_filled_circle(
                path->path[path->curPathStopIdx]->pos.x,
                path->path[path->curPathStopIdx]->pos.y,
                10.0f,
                isBlocked ?
                al_map_rgba(192, 0, 0, 200) :
                al_map_rgba(0, 0, 192, 200)
            );
        }
        
        //Square on the target spot, and target distance.
        al_draw_filled_rectangle(
            targetPos.x - 8.0f,
            targetPos.y - 8.0f,
            targetPos.x + 8.0f,
            targetPos.y + 8.0f,
            al_map_rgba(0, 192, 0, 200)
        );
        al_draw_circle(
            targetPos.x,
            targetPos.y,
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
    
    //Maker tool -- draw reaches.
    if(game.makerTools.infoLock && game.makerTools.reaches) {
        if(game.makerTools.infoLock->farReach != INVALID) {
            MobType::Reach* farReach =
                &game.makerTools.infoLock->type->reaches[
                    game.makerTools.infoLock->farReach
                ];
            ALLEGRO_COLOR color = al_map_rgba(192, 64, 64, 192);
            drawReach(
                game.makerTools.infoLock->pos,
                game.makerTools.infoLock->angle,
                game.makerTools.infoLock->radius,
                farReach->angle1, farReach->radius1, color
            );
            drawReach(
                game.makerTools.infoLock->pos,
                game.makerTools.infoLock->angle,
                game.makerTools.infoLock->radius,
                farReach->angle2, farReach->radius2, color
            );
        }
        if(game.makerTools.infoLock->nearReach != INVALID) {
            MobType::Reach* nearReach =
                &game.makerTools.infoLock->type->reaches[
                    game.makerTools.infoLock->nearReach
                ];
            ALLEGRO_COLOR color = al_map_rgba(64, 192, 64, 192);
            drawReach(
                game.makerTools.infoLock->pos,
                game.makerTools.infoLock->angle,
                game.makerTools.infoLock->radius,
                nearReach->angle1, nearReach->radius1, color
            );
            drawReach(
                game.makerTools.infoLock->pos,
                game.makerTools.infoLock->angle,
                game.makerTools.infoLock->radius,
                nearReach->angle2, nearReach->radius2, color
            );
        }
    }
    
    //Player notification.
    player->notification.draw(player->view);
    
    //Mission exit region.
    if(
        game.curAreaData->type == AREA_TYPE_MISSION &&
        game.curAreaData->mission.goal == MISSION_GOAL_GET_TO_EXIT
    ) {
        drawHighlightedRectRegion(
            game.curAreaData->mission.goalExitCenter,
            game.curAreaData->mission.goalExitSize,
            changeAlpha(COLOR_GOLD, 192), areaTimePassed
        );
    }
}


/**
 * @brief Draws the leader's cursor and associated effects.
 *
 * @param player Player whose viewport to draw to.
 * @param color Color to tint it by.
 */
void GameplayState::drawLeaderCursor(
    Player* player, const ALLEGRO_COLOR& color
) {
    if(!player->leaderPtr) return;
    
    //Swarm arrows.
    size_t nArrows = player->leaderPtr->swarmArrows.size();
    for(size_t a = 0; a < nArrows; a++) {
        Point pos(
            cos(player->swarmAngle) * player->leaderPtr->swarmArrows[a],
            sin(player->swarmAngle) * player->leaderPtr->swarmArrows[a]
        );
        float alpha =
            64 + std::min(
                191,
                (int) (
                    191 *
                    (player->leaderPtr->swarmArrows[a] /
                     (game.config.rules.leaderCursorMaxDist * 0.4))
                )
            );
        drawBitmap(
            game.sysContent.bmpSwarmArrow,
            player->leaderPtr->pos + pos,
            Point(
                16 * (1 + player->leaderPtr->swarmArrows[a] /
                      game.config.rules.leaderCursorMaxDist),
                -1
            ),
            player->swarmAngle,
            mapAlpha(alpha)
        );
    }
    
    //Whistle rings.
    size_t nRings = player->whistle.rings.size();
    float leaderCursorAngle =
        getAngle(player->leaderPtr->pos, player->leaderCursorWorld);
    float leaderCursorDist =
        Distance(player->leaderPtr->pos, player->leaderCursorWorld).toFloat();
    for(size_t r = 0; r < nRings; r++) {
        Point pos(
            player->leaderPtr->pos.x + cos(leaderCursorAngle) *
            player->whistle.rings[r],
            player->leaderPtr->pos.y + sin(leaderCursorAngle) *
            player->whistle.rings[r]
        );
        float ringToWhistleDist = leaderCursorDist - player->whistle.rings[r];
        float scale =
            interpolateNumber(
                ringToWhistleDist,
                0, leaderCursorDist,
                player->whistle.radius * 2, 0
            );
        float alpha =
            interpolateNumber(
                ringToWhistleDist,
                0, leaderCursorDist,
                0, 100
            );
        unsigned char n = player->whistle.ringColors[r];
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
    
    //Whistle dots.
    if(
        player->whistle.radius > 0 ||
        player->whistle.fadeTimer.timeLeft > 0.0f
    ) {
        al_draw_filled_circle(
            player->whistle.center.x, player->whistle.center.y,
            player->whistle.radius,
            al_map_rgba(48, 128, 120, 64)
        );
        
        unsigned char nDots = 16 * WHISTLE::N_DOT_COLORS;
        for(unsigned char d = 0; d < WHISTLE::N_DOT_COLORS; d++) {
            for(unsigned char d2 = 0; d2 < 16; d2++) {
                unsigned char currentDot = d2 * WHISTLE::N_DOT_COLORS + d;
                float angle =
                    TAU / nDots *
                    currentDot -
                    WHISTLE::DOT_SPIN_SPEED * areaTimePassed;
                    
                Point dotPos(
                    player->whistle.center.x +
                    cos(angle) * player->whistle.dotRadius[d],
                    player->whistle.center.y +
                    sin(angle) * player->whistle.dotRadius[d]
                );
                
                ALLEGRO_COLOR dotColor =
                    al_map_rgb(
                        WHISTLE::DOT_COLORS[d][0],
                        WHISTLE::DOT_COLORS[d][1],
                        WHISTLE::DOT_COLORS[d][2]
                    );
                unsigned char dotAlpha = 255;
                if(player->whistle.fadeTimer.timeLeft > 0.0f) {
                    dotAlpha = 255 * player->whistle.fadeTimer.getRatioLeft();
                }
                
                drawBitmap(
                    game.sysContent.bmpBrightCircle,
                    dotPos, Point(5.0f),
                    0.0f, changeAlpha(dotColor, dotAlpha)
                );
            }
        }
    }
    
    //Leader cursor.
    Point bmpCursorSize = getBitmapDimensions(game.sysContent.bmpLeaderCursor);
    
    drawBitmap(
        game.sysContent.bmpLeaderCursor,
        player->leaderCursorWorld,
        bmpCursorSize / 2.0f,
        leaderCursorAngle,
        changeColorLighting(
            color,
            player->leaderCursorHeightDiffLight
        )
    );
    
    //Throw preview.
    drawThrowPreview(player);
    
    //Standby type count.
    size_t nStandbyPikmin = 0;
    if(
        game.options.misc.showLeaderCursorCounter &&
        player->leaderPtr->group->curStandbyType
    ) {
        for(
            size_t m = 0; m < player->leaderPtr->group->members.size(); m++
        ) {
            Mob* mPtr = player->leaderPtr->group->members[m];
            if(
                mPtr->subgroupTypePtr ==
                player->leaderPtr->group->curStandbyType
            ) {
                nStandbyPikmin++;
            }
        }
    }
    
    al_use_transform(&game.identityTransform);
    
    float extrasXOffset =
        std::max(bmpCursorSize.x, bmpCursorSize.y) * 0.18f *
        player->view.cam.zoom;
    float extrasYOffset = extrasXOffset;
    float standbyCountHeight = 0.0f;
    
    if(nStandbyPikmin > 0) {
        standbyCountHeight = game.winH * 0.02f;
        drawText(
            i2s(nStandbyPikmin), game.sysContent.fntLeaderCursorCounter,
            player->leaderCursorWin +
            Point(extrasXOffset, extrasYOffset),
            Point(LARGE_FLOAT, game.winH * 0.02f), color,
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
        );
    }
    
    if(player->leaderCursorMobPointsAlpha != 0) {
        drawText(
            "$" + i2s(player->leaderCursorMobPoints),
            game.sysContent.fntValue,
            player->leaderCursorWin +
            Point(extrasXOffset, extrasYOffset + standbyCountHeight),
            Point(LARGE_FLOAT, game.winH * 0.02f),
            changeAlpha(COLOR_GOLD, player->leaderCursorMobPointsAlpha * 255),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP
        );
    }
    
    al_use_transform(&player->view.worldToWindowTransform);
}


/**
 * @brief Draws the full-window effects that will represent lighting.
 *
 * @param view Viewport to draw to.
 */
void GameplayState::drawLightingFilter(const Viewport& view) {
    al_use_transform(&game.identityTransform);
    
    //Draw the fog effect.
    ALLEGRO_COLOR fogC = game.curAreaData->weatherCondition.getFogColor();
    if(fogC.a > 0) {
        //Start by drawing the central fog fade out effect.
        Point fogTL =
            view.cam.pos -
            Point(game.curAreaData->weatherCondition.fogFar);
        Point fogBR =
            view.cam.pos +
            Point(game.curAreaData->weatherCondition.fogFar);
        al_transform_coordinates(
            &view.worldToWindowTransform, &fogTL.x, &fogTL.y
        );
        al_transform_coordinates(
            &view.worldToWindowTransform, &fogBR.x, &fogBR.y
        );
        
        if(bmpFog) {
            drawBitmap(bmpFog, (fogTL + fogBR) / 2, (fogBR - fogTL), 0, fogC);
        }
        
        //Now draw the fully opaque fog around the central fade.
        //Top-left and top-center.
        al_draw_filled_rectangle(
            0, 0, fogBR.x, fogTL.y, fogC
        );
        //Top-right and center-right.
        al_draw_filled_rectangle(
            fogBR.x, 0, view.size.x, fogBR.y, fogC
        );
        //Bottom-right and bottom-center.
        al_draw_filled_rectangle(
            fogTL.x, fogBR.y, view.size.x, view.size.y, fogC
        );
        //Bottom-left and center-left.
        al_draw_filled_rectangle(
            0, fogTL.y, fogTL.x, view.size.y, fogC
        );
        
    }
    
    //Draw the daylight.
    ALLEGRO_COLOR daylightC =
        game.curAreaData->weatherCondition.getDaylightColor();
    if(daylightC.a > 0) {
        al_draw_filled_rectangle(0, 0, view.size.x, view.size.y, daylightC);
    }
    
    //Draw the blackout effect.
    unsigned char blackoutS =
        game.curAreaData->weatherCondition.getBlackoutStrength();
    if(blackoutS > 0) {
        //First, we'll create the lightmap.
        //This is inverted (white = darkness, black = light), because we'll
        //apply it to the window using a subtraction operation.
        al_set_target_bitmap(lightmapBmp);
        
        //For starters, the whole window is dark (white in the map).
        al_clear_to_color(mapGray(blackoutS));
        
        int oldOp, oldSrc, oldDst, oldAOp, oldASrc, oldADst;
        al_get_separate_blender(
            &oldOp, &oldSrc, &oldDst, &oldAOp, &oldASrc, &oldADst
        );
        al_set_separate_blender(
            ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ONE, ALLEGRO_ONE,
            ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE
        );
        
        //Then, find out spotlights, and draw
        //their lights on the map (as black).
        al_hold_bitmap_drawing(true);
        for(size_t m = 0; m < mobs.all.size(); m++) {
            Mob* mPtr = mobs.all[m];
            if(
                hasFlag(mPtr->flags, MOB_FLAG_HIDDEN) ||
                mPtr->type->blackoutRadius == 0.0f
            ) {
                continue;
            }
            
            Point pos = mPtr->pos;
            al_transform_coordinates(
                &view.worldToWindowTransform, &pos.x, &pos.y
            );
            float radius = 4.0f * view.cam.zoom;
            
            if(mPtr->type->blackoutRadius > 0.0f) {
                radius *= mPtr->type->blackoutRadius;
            } else {
                radius *= mPtr->radius;
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
            oldOp, oldSrc, oldDst, oldAOp, oldASrc, oldADst
        );
        
    }
    
}


/**
 * @brief Draws the current Onion menu.
 */
void GameplayState::drawOnionMenu() {
    ALLEGRO_SHADER* bgShader = game.shaders.getShader(SHADER_TYPE_ONION);
    
    if (bgShader) {
        al_use_shader(bgShader);
        al_set_shader_sampler(
            "colormap", onionMenu->nestPtr->nestType->menuColormap, 1
        );
        al_set_shader_float("area_time", game.timePassed);
        al_set_shader_float("brightness", 0.4f);
        al_set_shader_float("opacity", 0.8f * onionMenu->bgAlphaMult);
        
        drawPrimRect(Point(), Point(game.winW, game.winH), COLOR_WHITE);
        al_use_shader(nullptr);
        
    } else {
        al_draw_filled_rectangle(
            0, 0, game.winW, game.winH,
            al_map_rgba(24, 64, 60, 220 * onionMenu->bgAlphaMult)
        );
        
    }
    
    onionMenu->gui.draw();
    
    drawMouseCursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Draws the current pause menu.
 */
void GameplayState::drawPauseMenu() {
    al_draw_filled_rectangle(
        0, 0, game.winW, game.winH,
        al_map_rgba(24, 48, 70, 200 * pauseMenu->bgAlphaMult)
    );
    drawBitmap(
        game.sysContent.bmpVignette,
        Point(game.winW, game.winH) / 2.0f, Point(game.winW, game.winH),
        0.0f, al_map_rgba(140, 182, 224, 44 * pauseMenu->bgAlphaMult)
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
        size_t nPrecipitationParticles = precipitation.size();
        for(size_t p = 0; p < nPrecipitationParticles; p++) {
            al_draw_filled_circle(
                precipitation[p].x, precipitation[p].y,
                3, COLOR_WHITE
            );
        }
    }
}


/**
 * @brief Draws a mob's reach, for content debugging purposes.
 *
 * @param center Center point, i.e. position of the mob.
 * @param angle Facing angle, i.e. the mob's angle.
 * @param radius Center radius, i.e. the mob's radius.
 * @param reachAngle Angle of the reach. Must be above 0 to be drawn.
 * @param reachRadius Radius of the reach. Must be above 0 to be drawn.
 * @param color Color to draw with.
 */
void GameplayState::drawReach(
    const Point& center, float angle, float radius,
    float reachAngle, float reachRadius,
    const ALLEGRO_COLOR& color
) {
    const float THICKNESS = 3.0f;
    
    if(reachAngle <= 0.0f || reachRadius <= 0.0f) return;
    
    float angle1 = angle - reachAngle / 2.0f;
    float angle2 = angle + reachAngle / 2.0f;
    al_draw_arc(
        center.x, center.y, radius + reachRadius, angle1, angle2 - angle1,
        color, THICKNESS
    );
    if(reachAngle < TAU) {
        Point p1 =
            center + rotatePoint(Point(radius + reachRadius, 0.0f), angle1);
        Point p2 =
            center + rotatePoint(Point(radius + reachRadius, 0.0f), angle2);
        al_draw_line(
            center.x, center.y, p1.x, p1.y, color, THICKNESS
        );
        al_draw_line(
            center.x, center.y, p2.x, p2.y, color, THICKNESS
        );
    }
}


/**
 * @brief Draws a leader's throw preview.
 *
 * @param player Player whose viewport to draw to.
 */
void GameplayState::drawThrowPreview(Player* player) {
    if(!player->leaderPtr) return;
    
    ALLEGRO_VERTEX vertexes[16];
    
    if(!player->leaderPtr->throwee) {
        //Just draw a simple line and leave.
        unsigned char nVertexes =
            getThrowPreviewVertexes(
                vertexes, 0.0f, 1.0f,
                player->leaderPtr->pos, player->throwDest,
                changeAlpha(
                    game.config.aestheticGen.noPikminColor,
                    GAMEPLAY::PREVIEW_OPACITY / 2.0f
                ),
                0.0f, 1.0f, false
            );
            
        for(unsigned char v = 0; v < nVertexes; v += 4) {
            al_draw_prim(
                vertexes, nullptr, nullptr,
                v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
            );
        }
        
        return;
    }
    
    //Check which edges exist near the throw.
    set<Edge*> candidateEdges;
    
    game.curAreaData->bmap.getEdgesInRegion(
        Point(
            std::min(player->leaderPtr->pos.x, player->throwDest.x),
            std::min(player->leaderPtr->pos.y, player->throwDest.y)
        ),
        Point(
            std::max(player->leaderPtr->pos.x, player->throwDest.x),
            std::max(player->leaderPtr->pos.y, player->throwDest.y)
        ),
        candidateEdges
    );
    
    float wallCollisionR = 2.0f;
    bool wallIsBlockingSector = false;
    Distance leaderToDestDist(
        player->leaderPtr->pos, player->throwDest
    );
    float throwHAngle = 0.0f;
    float throwVAngle = 0.0f;
    float throwSpeed = 0.0f;
    float throwHSpeed = 0.0f;
    coordinatesToAngle(
        player->leaderPtr->throweeSpeed, &throwHAngle, &throwHSpeed
    );
    coordinatesToAngle(
        Point(throwHSpeed, player->leaderPtr->throweeSpeedZ),
        &throwVAngle, &throwSpeed
    );
    float textureOffset =
        fmod(
            areaTimePassed * GAMEPLAY::PREVIEW_TEXTURE_TIME_MULT,
            al_get_bitmap_width(game.sysContent.bmpThrowPreview) *
            GAMEPLAY::PREVIEW_TEXTURE_SCALE
        );
        
    //For each edge, check if it crosses the throw line.
    for(Edge* e : candidateEdges) {
        if(!e->sectors[0] || !e->sectors[1]) {
            continue;
        }
        
        float r = 0.0f;
        if(
            !lineSegsIntersect(
                player->leaderPtr->pos, player->throwDest,
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
            r < wallCollisionR
        ) {
            wallCollisionR = r;
            wallIsBlockingSector = true;
            continue;
        }
        
        //Otherwise, let's check for walls.
        
        if(e->sectors[0]->z == e->sectors[1]->z) {
            //Edges where both sectors have the same height have no wall.
            continue;
        }
        
        //Calculate the throwee's vertical position at that point.
        float edgeZ = std::max(e->sectors[0]->z, e->sectors[1]->z);
        float xAtEdge =
            leaderToDestDist.toFloat() * r;
        float yAtEdge =
            tan(throwVAngle) * xAtEdge -
            (
                -MOB::GRAVITY_ADDER /
                (
                    2 * throwSpeed * throwSpeed *
                    cos(throwVAngle) * cos(throwVAngle)
                )
            ) * xAtEdge * xAtEdge;
        yAtEdge += player->leaderPtr->z;
        
        //If the throwee would hit the wall at these coordinates, collision.
        if(edgeZ >= yAtEdge && r < wallCollisionR) {
            wallCollisionR = r;
            wallIsBlockingSector = false;
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
    
    if(wallCollisionR > 1.0f) {
        //No collision. Free throw.
        
        unsigned char nVertexes =
            getThrowPreviewVertexes(
                vertexes, 0.0f, 1.0f,
                player->leaderPtr->pos, player->throwDest,
                changeAlpha(
                    player->leaderPtr->throwee->type->mainColor,
                    GAMEPLAY::PREVIEW_OPACITY
                ),
                textureOffset, GAMEPLAY::PREVIEW_TEXTURE_SCALE, true
            );
            
        for(unsigned char v = 0; v < nVertexes; v += 4) {
            al_draw_prim(
                vertexes, nullptr, game.sysContent.bmpThrowPreview,
                v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
            );
        }
        
    } else {
        //Wall collision.
        
        Point collisionPoint(
            player->leaderPtr->pos.x +
            (player->throwDest.x - player->leaderPtr->pos.x) *
            wallCollisionR,
            player->leaderPtr->pos.y +
            (player->throwDest.y - player->leaderPtr->pos.y) *
            wallCollisionR
        );
        
        if(!player->leaderPtr->throweeCanReach || wallIsBlockingSector) {
            //It's impossible to reach.
            
            unsigned char nVertexes =
                getThrowPreviewVertexes(
                    vertexes, 0.0f, wallCollisionR,
                    player->leaderPtr->pos, player->throwDest,
                    changeAlpha(
                        player->leaderPtr->throwee->type->mainColor,
                        GAMEPLAY::PREVIEW_OPACITY
                    ),
                    textureOffset, GAMEPLAY::PREVIEW_TEXTURE_SCALE, true
                );
                
            for(unsigned char v = 0; v < nVertexes; v += 4) {
                al_draw_prim(
                    vertexes, nullptr, game.sysContent.bmpThrowPreview,
                    v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            drawBitmap(
                game.sysContent.bmpThrowInvalid,
                collisionPoint, Point(32.0f), throwHAngle,
                changeAlpha(
                    player->leaderPtr->throwee->type->mainColor,
                    GAMEPLAY::PREVIEW_OPACITY
                )
            );
            
        } else {
            //Trajectory is unknown after collision. Can theoretically reach.
            
            unsigned char nVertexes =
                getThrowPreviewVertexes(
                    vertexes, 0.0f, wallCollisionR,
                    player->leaderPtr->pos, player->throwDest,
                    changeAlpha(
                        player->leaderPtr->throwee->type->mainColor,
                        GAMEPLAY::COLLISION_OPACITY
                    ),
                    textureOffset, GAMEPLAY::PREVIEW_TEXTURE_SCALE, true
                );
                
            for(unsigned char v = 0; v < nVertexes; v += 4) {
                al_draw_prim(
                    vertexes, nullptr, game.sysContent.bmpThrowPreview,
                    v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            nVertexes =
                getThrowPreviewVertexes(
                    vertexes, wallCollisionR, 1.0f,
                    player->leaderPtr->pos, player->throwDest,
                    changeAlpha(
                        player->leaderPtr->throwee->type->mainColor,
                        GAMEPLAY::PREVIEW_OPACITY
                    ),
                    0.0f, 1.0f, true
                );
                
            for(unsigned char v = 0; v < nVertexes; v += 4) {
                al_draw_prim(
                    vertexes, nullptr, game.sysContent.bmpThrowPreviewDashed,
                    v, v + 4, ALLEGRO_PRIM_TRIANGLE_FAN
                );
            }
            
            drawBitmap(
                game.sysContent.bmpThrowInvalid,
                collisionPoint, Point(16.0f), throwHAngle,
                changeAlpha(
                    player->leaderPtr->throwee->type->mainColor,
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
    const MakerTools::AreaImageSettings& settings
) {
    //First, get the full dimensions of the map.
    Point minCoords(FLT_MAX, FLT_MAX);
    Point maxCoords(-FLT_MAX, -FLT_MAX);
    
    for(size_t v = 0; v < game.curAreaData->vertexes.size(); v++) {
        Vertex* vPtr = game.curAreaData->vertexes[v];
        updateMinMaxCoords(
            minCoords, maxCoords, v2p(vPtr)
        );
    }
    
    //Figure out the scale that will fit on the image.
    float areaW = maxCoords.x - minCoords.x + settings.padding;
    float areaH = maxCoords.y - minCoords.y + settings.padding;
    float finalBmpW = settings.size;
    float finalBmpH = settings.size;
    float scale;
    
    if(areaW > areaH) {
        scale = settings.size / areaW;
        finalBmpH *= areaH / areaW;
    } else {
        scale = settings.size / areaH;
        finalBmpW *= areaW / areaH;
    }
    
    //Create the bitmap.
    ALLEGRO_BITMAP* bmp = al_create_bitmap(finalBmpW, finalBmpH);
    
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_translate_transform(
        &t,
        -minCoords.x + settings.padding / 2.0f,
        -minCoords.y + settings.padding / 2.0f
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
        TreeShadow* sPtr = game.curAreaData->treeShadows[s];
        
        unsigned char alpha =
            (
                (sPtr->alpha / 255.0) *
                game.curAreaData->weatherCondition.getSunStrength()
            ) * 255;
            
        drawBitmap(
            sPtr->bitmap,
            Point(
                sPtr->center.x + GAMEPLAY::TREE_SHADOW_SWAY_AMOUNT*
                cos(GAMEPLAY::TREE_SHADOW_SWAY_SPEED * areaTimePassed) *
                sPtr->sway.x,
                sPtr->center.y + GAMEPLAY::TREE_SHADOW_SWAY_AMOUNT*
                sin(GAMEPLAY::TREE_SHADOW_SWAY_SPEED * areaTimePassed) *
                sPtr->sway.y
            ),
            sPtr->size,
            sPtr->angle, mapAlpha(alpha)
        );
    }
}


/**
 * @brief Draws the components that make up the game world:
 * layout, objects, etc.
 *
 * @param view Viewport to draw to.
 * @param bmpOutput If not nullptr, draw the area onto this.
 */
void GameplayState::drawWorldComponents(
    const Viewport& view, ALLEGRO_BITMAP* bmpOutput
) {
    ALLEGRO_BITMAP* customWallOffsetEffectBuffer = nullptr;
    ALLEGRO_BITMAP* customLiquidLimitEffectBuffer = nullptr;
    if(!bmpOutput) {
        updateOffsetEffectBuffer(
            view.box[0], view.box[1],
            game.liquidLimitEffectCaches,
            game.liquidLimitEffectBuffer,
            true, view
        );
        updateOffsetEffectBuffer(
            view.box[0], view.box[1],
            game.wallSmoothingEffectCaches,
            game.wallOffsetEffectBuffer,
            true, view
        );
        updateOffsetEffectBuffer(
            view.box[0], view.box[1],
            game.wallShadowEffectCaches,
            game.wallOffsetEffectBuffer,
            false, view
        );
        
    } else {
        customLiquidLimitEffectBuffer =
            al_create_bitmap(
                al_get_bitmap_width(bmpOutput),
                al_get_bitmap_height(bmpOutput)
            );
        customWallOffsetEffectBuffer =
            al_create_bitmap(
                al_get_bitmap_width(bmpOutput),
                al_get_bitmap_height(bmpOutput)
            );
        updateOffsetEffectBuffer(
            Point(-FLT_MAX), Point(FLT_MAX),
            game.liquidLimitEffectCaches,
            customLiquidLimitEffectBuffer,
            true, view
        );
        updateOffsetEffectBuffer(
            Point(-FLT_MAX), Point(FLT_MAX),
            game.wallSmoothingEffectCaches,
            customWallOffsetEffectBuffer,
            true, view
        );
        updateOffsetEffectBuffer(
            Point(-FLT_MAX), Point(FLT_MAX),
            game.wallShadowEffectCaches,
            customWallOffsetEffectBuffer,
            false, view
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
        Sector* sPtr = game.curAreaData->sectors[s];
        
        if(
            !bmpOutput &&
            !rectanglesIntersect(
                sPtr->bbox[0], sPtr->bbox[1],
                view.box[0], view.box[1]
            )
        ) {
            //Off-camera.
            continue;
        }
        
        WorldComponent c;
        c.sectorPtr = sPtr;
        c.z = sPtr->z;
        components.push_back(c);
    }
    
    //Particles.
    particles.fillComponentList(components, view.box[0], view.box[1]);
    
    //Mobs.
    for(size_t m = 0; m < mobs.all.size(); m++) {
        Mob* mobPtr = mobs.all[m];
        
        if(!bmpOutput && mobPtr->isOffCamera(view)) {
            //Off-camera.
            continue;
        }
        
        if(hasFlag(mobPtr->flags, MOB_FLAG_HIDDEN)) continue;
        if(mobPtr->isStoredInsideMob()) continue;
        
        //Shadows.
        if(
            mobPtr->type->castsShadow &&
            !hasFlag(mobPtr->flags, MOB_FLAG_SHADOW_INVISIBLE)
        ) {
            WorldComponent c;
            c.mobShadowPtr = mobPtr;
            if(mobPtr->standingOnMob) {
                c.z =
                    mobPtr->standingOnMob->z +
                    mobPtr->standingOnMob->getDrawingHeight();
            } else {
                c.z = mobPtr->groundSector->z;
            }
            c.z += mobPtr->getDrawingHeight() - 1;
            components.push_back(c);
        }
        
        //Limbs.
        if(mobPtr->parent && mobPtr->parent->limbAnim.animDb) {
            unsigned char method = mobPtr->parent->limbDrawMethod;
            WorldComponent c;
            c.mobLimbPtr = mobPtr;
            
            switch(method) {
            case LIMB_DRAW_METHOD_BELOW_BOTH: {
                c.z = std::min(mobPtr->z, mobPtr->parent->m->z);
                break;
            } case LIMB_DRAW_METHOD_BELOW_CHILD: {
                c.z = mobPtr->z;
                break;
            } case LIMB_DRAW_METHOD_BELOW_PARENT: {
                c.z = mobPtr->parent->m->z;
                break;
            } case LIMB_DRAW_METHOD_ABOVE_PARENT: {
                c.z =
                    mobPtr->parent->m->z +
                    mobPtr->parent->m->getDrawingHeight() +
                    0.001;
                break;
            } case LIMB_DRAW_METHOD_ABOVE_CHILD: {
                c.z = mobPtr->z + mobPtr->getDrawingHeight() + 0.001;
                break;
            } case LIMB_DRAW_METHOD_ABOVE_BOTH: {
                c.z =
                    std::max(
                        mobPtr->parent->m->z +
                        mobPtr->parent->m->getDrawingHeight() +
                        0.001,
                        mobPtr->z + mobPtr->getDrawingHeight() +
                        0.001
                    );
                break;
            }
            }
            
            components.push_back(c);
        }
        
        //The mob proper.
        WorldComponent c;
        c.mobPtr = mobPtr;
        c.z = mobPtr->z + mobPtr->getDrawingHeight();
        if(mobPtr->holder.m && mobPtr->holder.forceAboveHolder) {
            c.z += mobPtr->holder.m->getDrawingHeight() + 1;
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
    
    float mobShadowStretch = 0;
    
    if(dayMinutes < 60 * 5 || dayMinutes > 60 * 20) {
        mobShadowStretch = 1;
    } else if(dayMinutes < 60 * 12) {
        mobShadowStretch = 1 - ((dayMinutes - 60 * 5) / (60 * 12 - 60 * 5));
    } else {
        mobShadowStretch = (dayMinutes - 60 * 12) / (60 * 20 - 60 * 12);
    }
    
    for(size_t c = 0; c < components.size(); c++) {
        WorldComponent* cPtr = &components[c];
        
        if(cPtr->sectorPtr) {
        
            bool hasLiquid = false;
            if(
                cPtr->sectorPtr->hazard &&
                cPtr->sectorPtr->hazard->associatedLiquid
            ) {
                drawLiquid(
                    cPtr->sectorPtr,
                    cPtr->sectorPtr->hazard->associatedLiquid,
                    Point(),
                    1.0f,
                    areaTimePassed
                );
                hasLiquid = true;
            }
            if(!hasLiquid) {
                drawSectorTexture(cPtr->sectorPtr, Point(), 1.0f, 1.0f);
            }
            float liquidOpacityMult = 1.0f;
            if(cPtr->sectorPtr->drainingLiquid) {
                liquidOpacityMult =
                    cPtr->sectorPtr->liquidDrainLeft /
                    GEOMETRY::LIQUID_DRAIN_DURATION;
            }
            drawSectorEdgeOffsets(
                cPtr->sectorPtr,
                bmpOutput ?
                customLiquidLimitEffectBuffer :
                game.liquidLimitEffectBuffer,
                liquidOpacityMult, view
            );
            drawSectorEdgeOffsets(
                cPtr->sectorPtr,
                bmpOutput ?
                customWallOffsetEffectBuffer :
                game.wallOffsetEffectBuffer,
                1.0f, view
            );
            
        } else if(cPtr->mobShadowPtr) {
        
            float deltaZ = 0;
            if(!cPtr->mobShadowPtr->standingOnMob) {
                deltaZ =
                    cPtr->mobShadowPtr->z -
                    cPtr->mobShadowPtr->groundSector->z;
            }
            drawMobShadow(
                cPtr->mobShadowPtr,
                deltaZ,
                mobShadowStretch
            );
            
        } else if(cPtr->mobLimbPtr) {
        
            if(!hasFlag(cPtr->mobLimbPtr->flags, MOB_FLAG_HIDDEN)) {
                cPtr->mobLimbPtr->drawLimb();
            }
            
        } else if(cPtr->mobPtr) {
        
            if(!hasFlag(cPtr->mobPtr->flags, MOB_FLAG_HIDDEN)) {
                cPtr->mobPtr->drawMob();
                if(cPtr->mobPtr->type->drawMobCallback) {
                    cPtr->mobPtr->type->drawMobCallback(cPtr->mobPtr);
                }
            }
            
        } else if(cPtr->particlePtr) {
        
            cPtr->particlePtr->draw();
            
        }
    }
    
    if(bmpOutput) {
        al_destroy_bitmap(customWallOffsetEffectBuffer);
    }
}
