/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gameplay state class and
 * gameplay state-related functions.
 */

#include <algorithm>

#include <allegro5/allegro_native_dialog.h>

#include "gameplay.h"

#include "../../content/mob/converter.h"
#include "../../content/mob/pile.h"
#include "../../content/mob/resource.h"
#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_functions.h"
#include "../../core/misc_structs.h"
#include "../../lib/data_file/data_file.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


namespace GAMEPLAY {

//How long the HUD moves for when the area is entered.
const float AREA_INTRO_HUD_MOVE_TIME = 3.0f;

//How long it takes for the area name to fade away, in-game.
const float AREA_TITLE_FADE_DURATION = 1.0f;

//How long it takes for the area name to fade away, in-game, with quick play.
const float AREA_TITLE_FADE_DURATION_FAST = 0.3f;

//How long the "Go!" big message lasts for.
const float BIG_MSG_GO_DUR = 1.5f;

//What text to show in the "Go!" big message.
const string BIG_MSG_GO_TEXT = "GO!";

//How long the "Mission clear!" big message lasts for.
const float BIG_MSG_MISSION_CLEAR_DUR = 4.5f;

//What text to show in the "Mission clear!" big message.
const string BIG_MSG_MISSION_CLEAR_TEXT = "MISSION CLEAR!";

//How long the "Mission failed..." big message lasts for.
const float BIG_MSG_MISSION_FAILED_DUR = 4.5f;

//What text to show in the "Mission failed..." big message.
const string BIG_MSG_MISSION_FAILED_TEXT = "MISSION FAILED...";

//How long the "1 minute left!" big message lasts for.
const float BIG_MSG_ONE_MIN_LEFT_DUR = 4.0f;

//What text to show in the "1 minute left!" big message.
const string BIG_MSG_ONE_MIN_LEFT_TEXT = "1 minute left!";

//How long the "Ready?" big message lasts for.
const float BIG_MSG_READY_DUR = 2.5f;

//What text to show in the "Ready?" big message.
const string BIG_MSG_READY_TEXT = "READY?";

//How long the "Time's up!" big message lasts for.
const float BIG_MSG_TIMES_UP_DUR = 4.5f;

//What text to show in the "Time's up!" big message.
const string BIG_MSG_TIMES_UP_TEXT = "TIME'S UP!";

//Distance between current leader and boss before the boss music kicks in.
const float BOSS_MUSIC_DISTANCE = 300.0f;

//Something is only considered off-camera if it's beyond this extra margin.
const float CAMERA_BOX_MARGIN = 128.0f;

//Smoothen the camera's movements by this factor.
const float CAMERA_SMOOTHNESS_FACTOR = 4.5f;

//Opacity of the collision bubbles in the maker tool.
const unsigned char COLLISION_OPACITY = 192;

//If an enemy is this close to the active leader, turn on the song's enemy mix.
const float ENEMY_MIX_DISTANCE = 150.0f;

//Width and height of the fog bitmap.
const int FOG_BITMAP_SIZE = 128;

//When a leader lands, this is the maximum size of the particles.
extern const float LEADER_LAND_PART_MAX_SIZE = 64.0f;

//When a leader lands, scale the particles by the fall distance and this factor.
extern const float LEADER_LAND_PART_SIZE_MULT = 0.1f;

//How far an analog stick must be held before a leader starts moving.
extern const float LEADER_MOVEMENT_MAGNITUDE_THRESHOLD = 0.75f;

//Multiply a leader's speed by this, when the analog stick is at the threshold.
extern const float LEADER_MOVEMENT_MIN_SPEED_MULT = 0.50f;

//How long the HUD moves for when a menu is entered.
const float MENU_ENTRY_HUD_MOVE_TIME = 0.4f;

//How long the HUD moves for when a menu is exited.
const float MENU_EXIT_HUD_MOVE_TIME = 0.5f;

//Opacity of the throw preview.
const unsigned char PREVIEW_OPACITY = 160;

//Scale of the throw preview's effect texture.
const float PREVIEW_TEXTURE_SCALE = 20.0f;

//Time multiplier for the throw preview's effect texture animation.
const float PREVIEW_TEXTURE_TIME_MULT = 20.0f;

//How frequently should a replay state be saved.
const float REPLAY_SAVE_FREQUENCY = 1.0f;

//Swarming arrows move these many units per second.
const float SWARM_ARROW_SPEED = 400.0f;

//Tree shadows sway this much away from their neutral position.
const float TREE_SHADOW_SWAY_AMOUNT = 8.0f;

//Tree shadows sway this much per second (TAU = full back-and-forth cycle).
const float TREE_SHADOW_SWAY_SPEED = TAU / 8;

}


#pragma region Big message


/**
 * @brief Gets the current big message's ID.
 *
 * @return The ID.
 */
BIG_MESSAGE BigMessageInfo::get() {
    return curId;
}


/**
 * @brief Gets the current big message's time spent.
 *
 * @return The time.
 */
float BigMessageInfo::getTime() {
    return curTime;
}


/**
 * @brief Overrides the time spent in the current big message to be
 * the specified amount.
 *
 * @param time The new time.
 */
void BigMessageInfo::overrideTime(float time) {
    curTime = time;
}


/**
 * @brief Sets the current big message to be this one.
 *
 * @param id ID of the new big message.
 */
void BigMessageInfo::set(BIG_MESSAGE id) {
    curId = id;
    curTime = 0.0f;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void BigMessageInfo::tick(float deltaT) {
    if(curId != BIG_MESSAGE_NONE) {
        curTime += deltaT;
    }
}


#pragma endregion
#pragma region Gameplay message box


/**
 * @brief Constructs a new message box info object.
 *
 * @param text Text to display.
 * @param speakerIcon If not nullptr, use this bitmap to represent who
 * is talking.
 */
GameplayMessageBox::GameplayMessageBox(
    const string& text, ALLEGRO_BITMAP* speakerIcon
):
    speakerIcon(speakerIcon) {
    
    string message = unescapeString(text);
    if(message.size() && message.back() == '\n') {
        message.pop_back();
    }
    vector<StringToken> tokens = tokenizeString(message);
    setStringTokenWidths(
        tokens, game.sysContent.fntStandard, game.sysContent.fntSlim,
        al_get_font_line_height(game.sysContent.fntStandard), true
    );
    
    vector<StringToken> line;
    for(size_t t = 0; t < tokens.size(); t++) {
        if(tokens[t].type == STRING_TOKEN_LINE_BREAK) {
            tokensPerLine.push_back(line);
            line.clear();
        } else {
            line.push_back(tokens[t]);
        }
    }
    if(!line.empty()) {
        tokensPerLine.push_back(line);
    }
}


/**
 * @brief Handles the user having pressed the button to continue the message,
 * or to skip to showing everything in the current section.
 */
void GameplayMessageBox::advance() {
    if(
        transitionTimer > 0.0f ||
        misinputProtectionTimer > 0.0f ||
        swipeTimer > 0.0f
    ) return;
    
    size_t lastToken = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t lineIdx = curSection * 3 + l;
        if(lineIdx >= tokensPerLine.size()) break;
        lastToken += tokensPerLine[lineIdx].size();
    }
    
    if(curToken >= lastToken + 1) {
        if(curSection >= ceil(tokensPerLine.size() / 3.0f) - 1) {
            //End of the message. Start closing the message box.
            close();
        } else {
            //Start swiping to go to the next section.
            swipeTimer = GAMEPLAY_MSG_BOX::TOKEN_SWIPE_DURATION;
        }
    } else {
        //Skip the text typing and show everything in this section.
        skippedAtToken = curToken;
        curToken = lastToken + 1;
    }
}


/**
 * @brief Closes the message box, even if it is still writing something.
 */
void GameplayMessageBox::close() {
    if(!transitionIn && transitionTimer > 0.0f) return;
    transitionIn = false;
    transitionTimer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayMessageBox::tick(float deltaT) {
    size_t tokensInSection = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t lineIdx = curSection * 3 + l;
        if(lineIdx >= tokensPerLine.size()) break;
        tokensInSection += tokensPerLine[lineIdx].size();
    }
    
    //Animate the swipe animation.
    if(swipeTimer > 0.0f) {
        swipeTimer -= deltaT;
        if(swipeTimer <= 0.0f) {
            //Go to the next section.
            swipeTimer = 0.0f;
            curSection++;
            totalTokenAnimTime = 0.0f;
            totalSkipAnimTime = 0.0f;
            skippedAtToken = INVALID;
        }
    }
    
    if(!transitionIn || transitionTimer == 0.0f) {
    
        //Animate the text.
        if(game.config.aestheticGen.gameplayMsgChInterval == 0.0f) {
            skippedAtToken = 0;
            curToken = tokensInSection + 1;
        } else {
            totalTokenAnimTime += deltaT;
            if(skippedAtToken == INVALID) {
                size_t prevToken = curToken;
                curToken =
                    totalTokenAnimTime /
                    game.config.aestheticGen.gameplayMsgChInterval;
                curToken =
                    std::min(curToken, tokensInSection + 1);
                if(prevToken != curToken) {
                    game.audio.addNewUiSoundSource(
                    game.sysContent.sndGameplayMsgChar, {
                        .stackMinPos = 0.05f,
                        .volume = 0.5f,
                        .volumeDeviation = 0.1f,
                        .speedDeviation = 0.1f,
                    }
                    );
                    if(curToken == tokensInSection + 1) {
                        //We've reached the last token organically.
                        //Start a misinput protection timer, so the player
                        //doesn't accidentally go to the next section when they
                        //were just trying to skip the text.
                        misinputProtectionTimer =
                            GAMEPLAY_MSG_BOX::MISINPUT_PROTECTION_DURATION;
                    }
                }
            } else {
                totalSkipAnimTime += deltaT;
            }
        }
        
    }
    
    //Animate the transition.
    transitionTimer -= deltaT;
    transitionTimer = std::max(0.0f, transitionTimer);
    if(!transitionIn && transitionTimer == 0.0f) {
        toDelete = true;
    }
    
    //Misinput protection logic.
    misinputProtectionTimer -= deltaT;
    misinputProtectionTimer = std::max(0.0f, misinputProtectionTimer);
    
    //Button opacity logic.
    if(
        transitionTimer == 0.0f &&
        misinputProtectionTimer == 0.0f &&
        swipeTimer == 0.0f &&
        curToken >= tokensInSection + 1
    ) {
        advanceButtonAlpha =
            std::min(
                advanceButtonAlpha +
                GAMEPLAY_MSG_BOX::ADVANCE_BUTTON_FADE_SPEED * deltaT,
                1.0f
            );
    } else {
        advanceButtonAlpha =
            std::max(
                0.0f,
                advanceButtonAlpha -
                GAMEPLAY_MSG_BOX::ADVANCE_BUTTON_FADE_SPEED * deltaT
            );
    }
}


#pragma endregion
#pragma region Gameplay state


/**
 * @brief Changes the amount of sprays of a certain type the player owns.
 * It also animates the correct HUD item, if any.
 *
 * @param team Which team's spray counts to change.
 * @param typeIdx Index number of the spray type.
 * @param amount Amount to change by.
 */
void GameplayState::changeSprayCount(
    PlayerTeam* team, size_t typeIdx, signed int amount
) {
    team->sprayStats[typeIdx].nrSprays =
        std::max(
            (signed int) team->sprayStats[typeIdx].nrSprays + amount,
            (signed int) 0
        );
}


/**
 * @brief Draws the gameplay.
 */
void GameplayState::doDrawing() {
    doGameDrawing();
    
    if(game.perfMon) {
        game.perfMon->leaveState();
    }
}


/**
 * @brief Tick the gameplay logic by one frame.
 */
void GameplayState::doLogic() {
    if(game.perfMon) {
        if(isInputAllowed) {
            //The first frame will have its speed all broken,
            //because of the long loading time that came before it.
            game.perfMon->setPaused(false);
            game.perfMon->enterState(PERF_MON_STATE_FRAME);
        } else {
            game.perfMon->setPaused(true);
        }
    }
    
    float regularDeltaT = game.deltaT;
    
    if(game.makerTools.changeSpeed) {
        game.deltaT *=
            game.makerTools.changeSpeedSettings[
                game.makerTools.changeSpeedSettingIdx
            ];
    } else if(game.makerTools.frameAdvanceMode) {
        if(game.makerTools.mustAdvanceOneFrame) {
            game.makerTools.mustAdvanceOneFrame = false;
            game.deltaT = 1.0f / game.options.advanced.targetFps;
        } else {
            //Let's not make it exactly 0 otherwise we'll get divisions-by-zero.
            game.deltaT = FLT_MIN;
        }
    }
    
    for(Player& player : players) {
        player.view.updateMouseCursor(game.mouseCursor.winPos);
    }
    
    //Controls.
    for(size_t a = 0; a < game.controls.actionQueue.size(); a++) {
        handlePlayerAction(game.controls.actionQueue[a]);
        if(onionMenu) {
            onionMenu->handlePlayerAction(game.controls.actionQueue[a]);
        }
        if(pauseMenu) {
            pauseMenu->handlePlayerAction(game.controls.actionQueue[a]);
        }
        game.makerTools.handleGameplayPlayerAction(
            game.controls.actionQueue[a]
        );
    }
    
    //Game logic.
    if(!paused) {
        game.statistics.gameplayTime += regularDeltaT;
        doGameplayLogic(game.deltaT * deltaTMult);
        doAestheticLogic(game.deltaT * deltaTMult);
    }
    doMenuLogic();
}


/**
 * @brief Ends the currently ongoing mission.
 *
 * @param clear Is it a clear or a failure?
 * @param showTimesUpMsg Whether to show a "Time's up!" message, or one of the
 * normal mission end messages.
 * @param ev Mission event responsible for this end, if any.
 * @return Whether it was able to end the mission.
 */
bool GameplayState::endMission(
    bool clear, bool showTimesUpMsg, MissionEvent* ev
) {
    if(interlude.get() != INTERLUDE_NONE) return false;
    
    interlude.set(INTERLUDE_MISSION_END, false);
    deltaTMult = 0.5f;
    stopAllLeaders();
    
    //Zoom in on the reason, if possible.
    for(Player& player : players) {
        Point newCamPos = player.view.cam.targetPos;
        float newCamZoom = player.view.cam.targetZoom;
        
        if(ev) {
            MissionEvType* evTypePtr = game.missionEvTypes[ev->type];
            if(
                evTypePtr->getZoomData(
                    ev, &game.curAreaData->mission, this,
                    &newCamPos, &newCamZoom
                )
            ) {
                player.view.cam.targetPos = newCamPos;
                player.view.cam.targetZoom = newCamZoom;
            }
        }
    }
    
    BIG_MESSAGE bigMsgToShow;
    ALLEGRO_SAMPLE* sndToPlay;
    if(clear) {
        bigMsgToShow = BIG_MESSAGE_MISSION_CLEAR;
        sndToPlay = game.sysContent.sndMissionClear;
    } else {
        bigMsgToShow = BIG_MESSAGE_MISSION_FAILED;
        sndToPlay = game.sysContent.sndMissionFailed;
    }
    if(showTimesUpMsg) {
        bigMsgToShow = BIG_MESSAGE_TIMES_UP;
    }
    
    bigMsg.set(bigMsgToShow);
    game.audio.addNewUiSoundSource(sndToPlay);
    game.audio.setCurrentSong("");
    
    for(Player& player : players) {
        player.hud->gui.startAnimation(
            GUI_MANAGER_ANIM_IN_TO_OUT,
            GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
        );
        player.inventory->close();
    }
    
    return true;
}


/**
 * @brief Code to run when the state is entered, be it from the area menu, be it
 * from the result menu's "keep playing" option.
 */
void GameplayState::enter() {
    particles.viewports.clear();
    
    for(Player& player : players) {
        player.view.size.x = game.winW;
        player.view.size.y = game.winH;
        player.view.center.x = game.winW / 2.0f;
        player.view.center.y = game.winH / 2.0f;
        player.view.boxMargin.x = GAMEPLAY::CAMERA_BOX_MARGIN;
        player.view.boxMargin.y = GAMEPLAY::CAMERA_BOX_MARGIN;
        player.view.updateTransformations();
    }
    
    float zoomReaches[3] = {
        game.config.rules.zoomClosestReach,
        game.options.advanced.zoomMediumReach,
        game.config.rules.zoomFarthestReach
    };
    float viewportReach = sqrt(players[0].view.size.x * players[0].view.size.y);
    for(int z = 0; z < 3; z++) {
        zoomLevels[z] = viewportReach / zoomReaches[z];
    }
    
    for(Player& player : players) {
        if(player.leaderPtr) {
            player.view.cam.setPos(player.leaderPtr->pos);
        } else {
            player.view.cam.setPos(Point());
        }
        player.view.cam.setZoom(zoomLevels[1]);
        player.view.updateTransformations();
        player.view.updateMouseCursor(game.mouseCursor.winPos);
        particles.viewports.push_back(&player.view);
        player.radarZoom = zoomLevels[1] * 0.4f;
    }
    
    lastMobClearedPos = Point(LARGE_FLOAT);
    lastHurtLeaderPos = Point(LARGE_FLOAT);
    lastPikminBornPos = Point(LARGE_FLOAT);
    lastPikminDeathPos = Point(LARGE_FLOAT);
    lastShipThatGotTreasurePos = Point(LARGE_FLOAT);
    
    missionFailReason = (MISSION_FAIL_COND) INVALID;
    goalIndicatorRatio = 0.0f;
    fail1IndicatorRatio = 0.0f;
    fail2IndicatorRatio = 0.0f;
    scoreFlapper = 0.0f;
    
    paused = false;
    interlude.set(INTERLUDE_READY, true);
    bigMsg.set(BIG_MESSAGE_READY);
    deltaTMult = 0.5f;
    bossMusicState = BOSS_MUSIC_STATE_NEVER_PLAYED;
    bool skipReadyInterlude = false;
    
    if(!game.quickPlay.areaPath.empty()) {
        //If this is an editor quick play, skip the "Ready..." interlude.
        skipReadyInterlude = true;
    }
    
    if(wentToResults) {
        game.fadeMgr.startFade(true, nullptr);
        if(pauseMenu) {
            pauseMenu->toDelete = true;
        }
    }
    
    readyForInput = false;
    game.mouseCursor.reset();
    
    for(Player& player : players) {
        player.hud->gui.hideItems();
        player.inventory->close();
        player.leaderPrompt.reset();
        if(game.mouseCursor.onWindow) {
            player.leaderCursorWorld = player.view.mouseCursorWorldPos;
            player.leaderCursorWin = game.mouseCursor.winPos;
        } else if(player.leaderPtr) {
            player.leaderCursorWorld =
                player.leaderPtr->pos +
                angleToCoordinates(
                    player.leaderPtr->angle,
                    game.config.rules.leaderCursorMaxDist / 2.0f
                );
            player.leaderCursorWin = player.leaderCursorWorld;
            al_transform_coordinates(
                &player.view.worldToWindowTransform,
                &player.leaderCursorWin.x, &player.leaderCursorWin.y
            );
        }
        if(player.leaderPtr) {
            player.leaderPtr->stopWhistling();
        }
        updateClosestGroupMembers(&player);
        
        player.whistle.nextDotTimer.onEnd = [&player] () {
            player.whistle.nextDotTimer.start();
            unsigned char dot = 255;
            for(unsigned char d = 0; d < 6; d++) { //Find WHAT dot to create.
                if(player.whistle.dotRadius[d] == -1) {
                    dot = d;
                    break;
                }
            }
            
            if(dot != 255) player.whistle.dotRadius[dot] = 0;
        };
        
        player.whistle.nextRingTimer.onEnd = [&player] () {
            player.whistle.nextRingTimer.start();
            player.whistle.rings.push_back(0);
            player.whistle.ringColors.push_back(player.whistle.ringPrevColor);
            player.whistle.ringPrevColor =
                sumAndWrap(
                    player.whistle.ringPrevColor, 1, WHISTLE::N_RING_COLORS
                );
        };
    }
    
    if(skipReadyInterlude) {
        interlude.overrideTime(GAMEPLAY::BIG_MSG_READY_DUR);
        bigMsg.overrideTime(GAMEPLAY::BIG_MSG_READY_DUR);
    } else {
        game.audio.addNewUiSoundSource(game.sysContent.sndReady);
    }
}


/**
 * @brief Generates the bitmap that'll draw the fog fade effect.
 *
 * @param nearRadius Until this radius, the fog is not present.
 * @param farRadius From this radius on, the fog is fully dense.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* GameplayState::generateFogBitmap(
    float nearRadius, float farRadius
) {
    if(farRadius == 0) return nullptr;
    
    ALLEGRO_BITMAP* bmp =
        al_create_bitmap(GAMEPLAY::FOG_BITMAP_SIZE, GAMEPLAY::FOG_BITMAP_SIZE);
        
    ALLEGRO_LOCKED_REGION* region =
        al_lock_bitmap(
            bmp, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_WRITEONLY
        );
    unsigned char* row = (unsigned char*) region->data;
    
    //We need to draw a radial gradient to represent the fog.
    //Between the center and the "near" radius, the opacity is 0%.
    //From there to the edge, the opacity fades to 100%.
    //Because the every quadrant of the image is the same, just mirrored,
    //we only need to process the pixels on the top-left quadrant and then
    //apply them to the respective pixels on the other quadrants as well.
    
    //This is where the "near" section of the fog is.
    float nearRatio = nearRadius / farRadius;
    
#define fillPixel(x, row) \
    row[(x) * 4 + 0] = 255; \
    row[(x) * 4 + 1] = 255; \
    row[(x) * 4 + 2] = 255; \
    row[(x) * 4 + 3] = curA; \
    
    for(int y = 0; y < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); y++) {
        for(int x = 0; x < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); x++) {
            //First, get how far this pixel is from the center.
            //Center = 0, radius or beyond = 1.
            float curRatio =
                Distance(
                    Point(x, y),
                    Point(
                        GAMEPLAY::FOG_BITMAP_SIZE / 2.0,
                        GAMEPLAY::FOG_BITMAP_SIZE / 2.0
                    )
                ).toFloat() / (GAMEPLAY::FOG_BITMAP_SIZE / 2.0);
            curRatio = std::min(curRatio, 1.0f);
            //Then, map that ratio to a different ratio that considers
            //the start of the "near" section as 0.
            curRatio =
                interpolateNumber(curRatio, nearRatio, 1.0f, 0.0f, 1.0f);
            //Finally, clamp the value and get the alpha.
            curRatio = std::clamp(curRatio, 0.0f, 1.0f);
            unsigned char curA = 255 * curRatio;
            
            //Save the memory location of the opposite row's pixels.
            unsigned char* oppositeRow =
                row + region->pitch * (GAMEPLAY::FOG_BITMAP_SIZE - y - y - 1);
            fillPixel(x, row);
            fillPixel(GAMEPLAY::FOG_BITMAP_SIZE - x - 1, row);
            fillPixel(x, oppositeRow);
            fillPixel(GAMEPLAY::FOG_BITMAP_SIZE - x - 1, oppositeRow);
        }
        row += region->pitch;
    }
    
#undef fillPixel
    
    al_unlock_bitmap(bmp);
    bmp = recreateBitmap(bmp); //Refresh mipmaps.
    return bmp;
}


/**
 * @brief Returns how many Pikmin are on the field in the current area.
 * This also checks inside converters.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t GameplayState::getAmountOfFieldPikmin(const PikminType* filter) {
    size_t total = 0;
    
    //Check the Pikmin mobs.
    for(size_t p = 0; p < mobs.pikmin.size(); p++) {
        Pikmin* pPtr = mobs.pikmin[p];
        if(filter && pPtr->pikType != filter) continue;
        total++;
    }
    
    //Check Pikmin inside converters.
    for(size_t c = 0; c < mobs.converters.size(); c++) {
        Converter* cPtr = mobs.converters[c];
        if(filter && cPtr->currentType != filter) continue;
        total += cPtr->amountInBuffer;
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are in the group.
 *
 * @param player The player responsible.
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t GameplayState::getAmountOfGroupPikmin(
    Player* player, const PikminType* filter
) {
    if(!player->leaderPtr) return 0;
    
    size_t total = 0;
    
    for(size_t m = 0; m < player->leaderPtr->group->members.size(); m++) {
        Mob* mPtr = player->leaderPtr->group->members[m];
        if(mPtr->type->category->id != MOB_CATEGORY_PIKMIN) continue;
        if(filter && mPtr->type != filter) continue;
        total++;
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are idling in the area.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t GameplayState::getAmountOfIdlePikmin(const PikminType* filter) {
    size_t total = 0;
    
    for(size_t p = 0; p < mobs.pikmin.size(); p++) {
        Pikmin* pPtr = mobs.pikmin[p];
        if(filter && pPtr->type != filter) continue;
        if(
            pPtr->fsm.curState->id == PIKMIN_STATE_IDLING ||
            pPtr->fsm.curState->id == PIKMIN_STATE_IDLING_H
        ) {
            total++;
        }
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are inside of Onions in the current area.
 * This also checks ships.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
long GameplayState::getAmountOfOnionPikmin(const PikminType* filter) {
    long total = 0;
    
    //Check Onions proper.
    for(size_t o = 0; o < mobs.onions.size(); o++) {
        Onion* oPtr = mobs.onions[o];
        for(
            size_t t = 0;
            t < oPtr->oniType->nest->pikTypes.size();
            t++
        ) {
            if(filter && oPtr->oniType->nest->pikTypes[t] != filter) {
                continue;
            }
            total +=
                (long)
                oPtr->nest->getAmountByType(oPtr->oniType->nest->pikTypes[t]);
        }
    }
    
    //Check ships.
    for(size_t s = 0; s < mobs.ships.size(); s++) {
        Ship* sPtr = mobs.ships[s];
        if(!sPtr->nest) continue;
        for(
            size_t t = 0;
            t < sPtr->shiType->nest->pikTypes.size();
            t++
        ) {
            if(filter && sPtr->shiType->nest->pikTypes[t] != filter) {
                continue;
            }
            total +=
                (long)
                sPtr->nest->getAmountByType(sPtr->shiType->nest->pikTypes[t]);
        }
    }
    return total;
}


/**
 * @brief Returns the total amount of Pikmin the player has.
 * This includes Pikmin on the field as well as the Onions, and also
 * Pikmin inside converters.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
long GameplayState::getAmountOfTotalPikmin(const PikminType* filter) {
    long total = 0;
    
    //Check Pikmin on the field and inside converters.
    total += (long) getAmountOfFieldPikmin(filter);
    
    //Check Pikmin inside Onions and ships.
    total += getAmountOfOnionPikmin(filter);
    
    //Return the final sum.
    return total;
}


/**
 * @brief Returns the closest group member of a given standby subgroup.
 * In the case all candidate members are out of reach,
 * this returns the closest. Otherwise, it returns the closest
 * and more mature one.
 *
 * @param player The player responsible.
 * @param type Type to search for.
 * @param distant If not nullptr, whether all members are unreachable is
 * returned here.
 * @return The closest member, or nullptr if there is no member
 * of that subgroup available to grab.
 */
Mob* GameplayState::getClosestGroupMember(
    Player* player, const SubgroupType* type, bool* distant
) {
    if(!player->leaderPtr) return nullptr;
    
    Mob* result = nullptr;
    
    //Closest members so far for each maturity.
    Distance closestDists[N_MATURITIES];
    Mob* closestPtrs[N_MATURITIES];
    bool canGrabClosest[N_MATURITIES];
    for(unsigned char m = 0; m < N_MATURITIES; m++) {
        closestPtrs[m] = nullptr;
        canGrabClosest[m] = false;
    }
    
    //Fetch the closest, for each maturity.
    size_t nMembers = player->leaderPtr->group->members.size();
    for(size_t m = 0; m < nMembers; m++) {
    
        Mob* memberPtr = player->leaderPtr->group->members[m];
        if(memberPtr->subgroupTypePtr != type) {
            continue;
        }
        
        unsigned char maturity = 0;
        if(memberPtr->type->category->id == MOB_CATEGORY_PIKMIN) {
            maturity = ((Pikmin*) memberPtr)->maturity;
        }
        bool canGrab = player->leaderPtr->canGrabGroupMember(memberPtr);
        
        if(!canGrab && canGrabClosest[maturity]) {
            //Skip if we'd replace a grabbable Pikmin with a non-grabbable one.
            continue;
        }
        
        Distance d(player->leaderPtr->pos, memberPtr->pos);
        
        if(
            (canGrab && !canGrabClosest[maturity]) ||
            !closestPtrs[maturity] ||
            d < closestDists[maturity]
        ) {
            closestDists[maturity] = d;
            closestPtrs[maturity] = memberPtr;
            canGrabClosest[maturity] = canGrab;
            
        }
    }
    
    //Now, try to get the one with the highest maturity within reach.
    Distance closestDist;
    for(unsigned char m = 0; m < N_MATURITIES; m++) {
        if(!closestPtrs[2 - m]) continue;
        if(!canGrabClosest[2 - m]) continue;
        result = closestPtrs[2 - m];
        closestDist = closestDists[2 - m];
        break;
    }
    
    if(distant) *distant = !result;
    
    if(!result) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < N_MATURITIES; m++) {
            if(!closestPtrs[m]) continue;
            
            if(!result || closestDists[m] < closestDist) {
                result = closestPtrs[m];
                closestDist = closestDists[m];
            }
        }
    }
    
    return result;
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string GameplayState::getName() const {
    return "gameplay";
}


/**
 * @brief Returns a mob on the leader cursor that either has enemy
 * or treasure points.
 *
 * @param player The player responsible.
 * @return The mob, or nullptr if none.
 */
Mob* GameplayState::getPointMobOnLeaderCursor(Player* player) const {
    if(!player || !player->leaderPtr) return nullptr;
    
    Mob* closest = nullptr;
    Distance closestDist;
    for(size_t m = 0; m < mobs.all.size(); m++) {
        Mob* mPtr = mobs.all[m];
        
        if(mPtr->isStoredInsideMob()) continue;
        if(!mPtr->fsm.curState) continue;
        
        Distance d(player->leaderCursorWorld, mPtr->pos);
        if(d > mPtr->radius) continue;
        if(closest && d > closestDist) continue;
        
        if(
            mPtr->type->category->id != MOB_CATEGORY_ENEMIES &&
            mPtr->type->category->id != MOB_CATEGORY_TREASURES &&
            mPtr->type->category->id != MOB_CATEGORY_PILES &&
            mPtr->type->category->id != MOB_CATEGORY_RESOURCES
        ) continue;
        
        closest = mPtr;
        closestDist = d;
    }
    
    return closest;
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void GameplayState::handleAllegroEvent(ALLEGRO_EVENT& ev) {
    //Handle the Onion menu first so events don't bleed from gameplay to it.
    if(onionMenu) {
        onionMenu->handleAllegroEvent(ev);
    } else if(pauseMenu) {
        pauseMenu->handleAllegroEvent(ev);
    }
    
    //Finally, let the HUD handle events.
    for(Player& player : players) {
        player.hud->gui.handleAllegroEvent(ev);
        player.inventory->gui.handleAllegroEvent(ev);
    }
    
}


/**
 * @brief Leaves the gameplay state and enters the title screen,
 * or annex screen, or etc.
 *
 * @param target Where to leave to.
 */
void GameplayState::leave(const GAMEPLAY_LEAVE_TARGET target) {
    if(unloading) return;
    
    if(game.perfMon) {
        //Don't register the final frame, since it won't draw anything.
        game.perfMon->setPaused(true);
    }
    
    game.audio.stopAllPlaybacks();
    game.audio.setCurrentSong("");
    game.controls.setGameState(CONTROLS_GAME_STATE_MENUS);
    bossMusicState = BOSS_MUSIC_STATE_NEVER_PLAYED;
    saveStatistics();
    
    switch(target) {
    case GAMEPLAY_LEAVE_TARGET_RETRY: {
        game.changeState(game.states.gameplay);
        break;
    } case GAMEPLAY_LEAVE_TARGET_END: {
        wentToResults = true;
        //Change state, but don't unload this one, since the player
        //may pick the "keep playing" option in the results screen.
        game.changeState(game.states.results, false);
        break;
    } case GAMEPLAY_LEAVE_TARGET_AREA_SELECT: {
        if(game.quickPlay.areaPath.empty()) {
            game.states.annexScreen->areaMenuAreaType =
                game.curAreaData->type;
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.changeState(game.states.annexScreen);
        } else {
            game.changeState(game.quickPlay.editor);
        }
        break;
    }
    }
}


/**
 * @brief Loads the "gameplay" state into memory.
 */
void GameplayState::load() {
    if(game.perfMon) {
        game.perfMon->reset();
        game.perfMon->enterState(PERF_MON_STATE_LOADING);
        game.perfMon->setPaused(false);
    }
    
    loading = true;
    game.errors.prepareAreaLoad();
    wentToResults = false;
    
    drawLoadingScreen("", "", "", 1.0f);
    al_flip_display();
    
    game.statistics.areaEntries++;
    
    //Game content.
    loadGameContent();
    
    //Initialize some important things.
    for(size_t t = 0; t < MAX_PLAYER_TEAMS; t++) {
        for(size_t s = 0; s < game.content.sprayTypes.list.size(); s++) {
            playerTeams[t].sprayStats.push_back(SprayStats());
        }
    }
    players[0].team = &playerTeams[0];
    
    areaTitleFadeTimer.start(
        game.quickPlay.areaPath.empty() ?
        GAMEPLAY::AREA_TITLE_FADE_DURATION :
        GAMEPLAY::AREA_TITLE_FADE_DURATION_FAST
    );
    areaTimePassed = 0.0f;
    gameplayTimePassed = 0.0f;
    game.makerTools.resetForGameplay();
    
    afterHours = false;
    pikminBorn = 0;
    pikminDeaths = 0;
    treasuresCollected = 0;
    treasuresTotal = 0;
    goalTreasuresCollected = 0;
    goalTreasuresTotal = 0;
    treasurePointsObtained = 0;
    treasurePointsTotal = 0;
    enemyDefeats = 0;
    enemyTotal = 0;
    enemyPointsObtained = 0;
    enemyPointsTotal = 0;
    curLeadersInMissionExit = 0;
    missionRequiredMobAmount = 0;
    missionScore = 0;
    oldMissionScore = 0;
    oldMissionGoalCur = 0;
    oldMissionFail1Cur = 0;
    oldMissionFail2Cur = 0;
    nrLivingLeaders = 0;
    leadersKod = 0;
    medalGotItJuiceTimer = 0.0f;
    lastCarryingTieBreaker = nullptr;
    pikminBornPerType.clear();
    pikminDeathsPerType.clear();
    printActionLogLines.clear();
    
    game.framerateLastAvgPoint = 0;
    game.framerateHistory.clear();
    
    bossMusicState = BOSS_MUSIC_STATE_NEVER_PLAYED;
    game.audio.setCurrentSong("");
    game.audio.onSongFinished = [this] (const string& name) {
        if(name == game.sysContentNames.sngBossVictory) {
            switch(bossMusicState) {
            case BOSS_MUSIC_STATE_VICTORY: {
                game.audio.setCurrentSong(game.curAreaData->songName, false);
                bossMusicState = BOSS_MUSIC_STATE_PAUSED;
            } default: {
                break;
            }
            }
        }
    };
    
    auto sparkAnimDbIt =
        game.content.globalAnimDbs.list.find(
            game.sysContentNames.anmSparks
        );
    if(sparkAnimDbIt == game.content.globalAnimDbs.list.end()) {
        game.errors.report(
            "Unknown global animation \"" + game.sysContentNames.anmSparks +
            "\" when trying to load the leader damage sparks!"
        );
    } else {
        game.sysContent.anmSparks.initToFirstAnim(
            &sparkAnimDbIt->second
        );
    }
    
    //Load the area.
    if(
        !game.content.loadAreaAsCurrent(
            pathOfAreaToLoad, nullptr,
            CONTENT_LOAD_LEVEL_FULL, false
        )
    ) {
        leave(GAMEPLAY_LEAVE_TARGET_AREA_SELECT);
        return;
    }
    
    if(!game.curAreaData->weatherCondition.blackoutStrength.empty()) {
        lightmapBmp = al_create_bitmap(game.winW, game.winH);
    }
    if(!game.curAreaData->weatherCondition.fogColor.empty()) {
        bmpFog =
            generateFogBitmap(
                game.curAreaData->weatherCondition.fogNear,
                game.curAreaData->weatherCondition.fogFar
            );
    }
    
    //Generate mobs.
    nextMobId = 0;
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object generation");
    }
    
    vector<Mob*> mobsPerGen;
    
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* mPtr = game.curAreaData->mobGenerators[m];
        bool valid = true;
        
        if(!mPtr->type) {
            valid = false;
        } else if(
            mPtr->type->category->id == MOB_CATEGORY_PIKMIN &&
            game.states.gameplay->mobs.pikmin.size() >=
            game.curAreaData->getMaxPikminInField()
        ) {
            valid = false;
        }
        
        if(valid) {
            Mob* newMob = createMob(mPtr);
            mobsPerGen.push_back(newMob);
        } else {
            mobsPerGen.push_back(nullptr);
        }
    }
    
    //Mob links.
    //Because mobs can create other mobs when loaded, mob gen index X
    //does not necessarily correspond to mob index X. Hence, we need
    //to keep the pointers to the created mobs in a vector, and use this
    //to link the mobs by (generator) index.
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* genPtr = game.curAreaData->mobGenerators[m];
        Mob* mobPtr = mobsPerGen[m];
        if(!mobPtr) continue;
        
        for(size_t l = 0; l < genPtr->linkIdxs.size(); l++) {
            size_t linkTargetGenIdx = genPtr->linkIdxs[l];
            Mob* linkTargetMobPtr = mobsPerGen[linkTargetGenIdx];
            mobPtr->links.push_back(linkTargetMobPtr);
        }
    }
    
    //Mobs stored inside other. Same logic as mob links.
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* holdeeGenPtr = game.curAreaData->mobGenerators[m];
        if(holdeeGenPtr->storedInside == INVALID) continue;
        Mob* holdeePtr = mobsPerGen[m];
        Mob* holderMobPtr = mobsPerGen[holdeeGenPtr->storedInside];
        holderMobPtr->storeMobInside(holdeePtr);
    }
    
    //Save each path stop's sector.
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        game.curAreaData->pathStops[s]->sectorPtr =
            getSector(game.curAreaData->pathStops[s]->pos, nullptr, true);
    }
    
    //Create liquids.
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* sPtr = game.curAreaData->sectors[s];
        if(!sPtr->hazard) continue;
        if(!sPtr->hazard->associatedLiquid) continue;
        if(sPtr->liquid) continue;
        
        vector<Sector*> liquidSectors;
        
        sPtr->getNeighborSectorsConditionally(
        [] (Sector * s2) -> bool {
            return s2->hazard && s2->hazard->associatedLiquid;
        },
        liquidSectors
        );
        
        liquids.push_back(new Liquid(sPtr->hazard, liquidSectors));
        for(size_t s = 0; s < liquidSectors.size(); s++) {
            sPtr->liquid = liquids.back();
        }
    }
    
    //Sort leaders.
    sort(
        mobs.leaders.begin(), mobs.leaders.end(),
    [] (Leader * l1, Leader * l2) -> bool {
        size_t priorityL1 =
        find(
            game.config.leaders.order.begin(),
            game.config.leaders.order.end(), l1->leaType
        ) -
        game.config.leaders.order.begin();
        size_t priorityL2 =
        find(
            game.config.leaders.order.begin(),
            game.config.leaders.order.end(), l2->leaType
        ) -
        game.config.leaders.order.begin();
        return priorityL1 < priorityL2;
    }
    );
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //In case a leader is stored in another mob,
    //update the available list.
    updateAvailableLeaders();
    startingNrOfLeaders = mobs.leaders.size();
    
    for(Player& player : players) {
        player.leaderIdx = INVALID;
        player.leaderPtr = nullptr;
        
        if(!mobs.leaders.empty()) {
            changeToNextLeader(&player, true, false, false);
        }
        
        player.whistle.nextDotTimer.start();
        player.whistle.nextRingTimer.start();
    }
    
    //Memorize mobs required by the mission.
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
        unordered_set<size_t> missionRequiredMobGenIdxs;
        
        if(game.curAreaData->missionOld.goalAllMobs) {
            for(size_t m = 0; m < mobsPerGen.size(); m++) {
                if(
                    mobsPerGen[m] &&
                    game.missionGoals[game.curAreaData->missionOld.goal]->
                    isMobApplicable(mobsPerGen[m]->type)
                ) {
                    missionRequiredMobGenIdxs.insert(m);
                }
            }
            
        } else {
            missionRequiredMobGenIdxs =
                game.curAreaData->missionOld.goalMobIdxs;
        }
        
        for(size_t i : missionRequiredMobGenIdxs) {
            missionRemainingMobIds.insert(mobsPerGen[i]->id);
        }
        missionRequiredMobAmount = missionRemainingMobIds.size();
        
        missionEventsTriggered.clear();
        missionEventsTriggered.insert(
            missionEventsTriggered.begin(),
            game.curAreaData->mission.events.size(),
            false
        );
        
        missionMobChecklists.clear();
        for(
            size_t c = 0;
            c < game.curAreaData->mission.mobChecklists.size(); c++
        ) {
            missionMobChecklists.push_back(MissionMobChecklistStatus());
            vector<size_t> idxs =
                game.curAreaData->mission.mobChecklists[c].calculateList();
            missionMobChecklists.back().remaining.reserve(idxs.size());
            for(size_t i = 0; i < idxs.size(); i++) {
                missionMobChecklists.back().remaining.insert(
                    mobsPerGen[idxs[i]]
                );
            }
            missionMobChecklists.back().startingAmount =
                missionMobChecklists.back().remaining.size();
            missionMobChecklists.back().requiredAmount =
                game.curAreaData->mission.mobChecklists[c].requiredAmount;
            if(missionMobChecklists.back().requiredAmount == 0) {
                missionMobChecklists.back().requiredAmount =
                    missionMobChecklists.back().startingAmount;
            }
        }
        
        if(game.curAreaData->missionOld.goal == MISSION_GOAL_COLLECT_TREASURE) {
            //Since the collect treasure goal can accept piles and resources
            //meant to add treasure points, we'll need some special treatment.
            for(size_t i : missionRequiredMobGenIdxs) {
                if(
                    mobsPerGen[i]->type->category->id ==
                    MOB_CATEGORY_PILES
                ) {
                    Pile* pilPtr = (Pile*) mobsPerGen[i];
                    goalTreasuresTotal += pilPtr->amount;
                } else {
                    goalTreasuresTotal++;
                }
            }
        }
    }
    
    //Figure out the total amount of treasures and their points.
    for(size_t t = 0; t < mobs.treasures.size(); t++) {
        treasuresTotal++;
        treasurePointsTotal +=
            mobs.treasures[t]->treType->points;
    }
    for(size_t p = 0; p < mobs.piles.size(); p++) {
        Pile* pPtr = mobs.piles[p];
        ResourceType* resType = pPtr->pilType->contents;
        if(
            resType->deliveryResult !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        treasuresTotal += pPtr->amount;
        treasurePointsTotal +=
            pPtr->amount * resType->pointAmount;
    }
    for(size_t r = 0; r < mobs.resources.size(); r++) {
        Resource* rPtr = mobs.resources[r];
        if(
            rPtr->resType->deliveryResult !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        treasuresTotal++;
        treasurePointsTotal += rPtr->resType->pointAmount;
    }
    
    //Figure out the total amount of enemies and their points.
    enemyTotal = 0;
    for(size_t e = 0; e < mobs.enemies.size(); e++) {
        if(!mobs.enemies[e]->parent) {
            enemyTotal++;
            enemyPointsTotal += mobs.enemies[e]->eneType->points;
        }
    }
    
    //Initialize the area's active cells.
    float areaWidth =
        game.curAreaData->bmap.nCols * GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    float areaHeight =
        game.curAreaData->bmap.nRows * GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    size_t nrAreaCellCols =
        ceil(areaWidth / GEOMETRY::AREA_CELL_SIZE) + 1;
    size_t nrAreaCellRows =
        ceil(areaHeight / GEOMETRY::AREA_CELL_SIZE) + 1;
        
    areaActiveCells.clear();
    areaActiveCells.assign(
        nrAreaCellCols, vector<bool>(nrAreaCellRows, false)
    );
    
    //Initialize some other things.
    areaRegions.clear();
    areaRegions.insert(
        areaRegions.begin(),
        game.curAreaData->regions.size(), AreaRegionStatus()
    );
    
    pathMgr.handleAreaLoad();
    
    for(Player& player : players) {
        player.hud = new Hud();
        player.hud->player = &player;
        player.inventory = new Inventory(&player);
    }
    
    dayMinutes = game.curAreaData->dayTimeStart;
    
    map<string, string> sprayStrs =
        getVarMap(game.curAreaData->sprayAmounts);
        
    for(auto& s : sprayStrs) {
        size_t sprayIdx = 0;
        for(; sprayIdx < game.config.misc.sprayOrder.size(); sprayIdx++) {
            if(
                game.config.misc.sprayOrder[sprayIdx]->manifest->internalName ==
                s.first
            ) {
                break;
            }
        }
        if(sprayIdx == game.content.sprayTypes.list.size()) {
            game.errors.report(
                "Unknown spray type \"" + s.first + "\", "
                "while trying to set the starting number of sprays for "
                "area \"" + game.curAreaData->name + "\"!", nullptr
            );
            continue;
        }
        
        for(size_t t = 0; t < MAX_PLAYER_TEAMS; t++) {
            playerTeams[t].sprayStats[sprayIdx].nrSprays = s2i(s.second);
        }
    }
    
    //Effect caches.
    game.liquidLimitEffectCaches.clear();
    game.liquidLimitEffectCaches.insert(
        game.liquidLimitEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.liquidLimitEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveLiquidLimit,
        getLiquidLimitLength,
        getLiquidLimitColor
    );
    game.wallSmoothingEffectCaches.clear();
    game.wallSmoothingEffectCaches.insert(
        game.wallSmoothingEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.wallSmoothingEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveLedgeSmoothing,
        getLedgeSmoothingLength,
        getLedgeSmoothingColor
    );
    game.wallShadowEffectCaches.clear();
    game.wallShadowEffectCaches.insert(
        game.wallShadowEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.wallShadowEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveWallShadow,
        getWallShadowLength,
        getWallShadowColor
    );
    
    //TODO Uncomment this when replays are implemented.
    /*
    replayTimer = timer(
        GAMEPLAY::REPLAY_SAVE_FREQUENCY,
    [this] () {
        this->replayTimer.start();
        vector<mob*> obstacles; //TODO
        gameplayReplay.addNewState(
            leaders, pikminList, enemies, treasures, onions, obstacles,
            curLeaderIdx
        );
    }
    );
    replayTimer.start();
    gameplayReplay.clear();*/
    
    //Report any errors with the loading process.
    game.errors.reportAreaLoadErrors();
    
    if(game.perfMon) {
        game.perfMon->setAreaName(game.curAreaData->name);
        game.perfMon->leaveState();
        game.console.write(
            "The performance monitor maker tool is running.", 10
        );
    }
    
    enter();
    
    loading = false;
}


/**
 * @brief Loads all of the game's content.
 */
void GameplayState::loadGameContent() {
    game.content.reloadPacks();
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_WEATHER_CONDITION,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Area manifests.
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    //Mob types.
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_MOB_TYPE,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Register leader sub-group types.
    for(size_t p = 0; p < game.config.pikmin.order.size(); p++) {
        subgroupTypes.registerType(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, game.config.pikmin.order[p],
            game.config.pikmin.order[p]->bmpIcon
        );
    }
    
    vector<string> toolTypesVector;
    for(auto& t : game.content.mobTypes.list.tool) {
        toolTypesVector.push_back(t.first);
    }
    sort(toolTypesVector.begin(), toolTypesVector.end());
    for(size_t t = 0; t < toolTypesVector.size(); t++) {
        ToolType* ttPtr = game.content.mobTypes.list.tool[toolTypesVector[t]];
        subgroupTypes.registerType(
            SUBGROUP_TYPE_CATEGORY_TOOL, ttPtr, ttPtr->bmpIcon
        );
    }
    
    subgroupTypes.registerType(SUBGROUP_TYPE_CATEGORY_LEADER);
}


/**
 * @brief Starts the fade out to leave the gameplay state.
 *
 * @param target Where to leave to.
 */
void GameplayState::startLeaving(const GAMEPLAY_LEAVE_TARGET target) {
    game.fadeMgr.startFade( false, [this, target] () { leave(target); });
}


/**
 * @brief Stops all leaders in their tracks, as far as player controls
 * are concerned.
 */
void GameplayState::stopAllLeaders() {
    for(Player& player : players) {
        player.leaderMovement.reset();
        player.swarmMovement.reset();
        player.leaderCursorMov.reset();
    }
}


/**
 * @brief Tries to pause the game.
 */
void GameplayState::tryPause() {
    if(!loaded) return;
    if(pauseMenu) return;
    if(paused) return;
    if(players.empty()) return;
    doPlayerActionPause(&players[0], true, false);
}


/**
 * @brief Unloads the "gameplay" state from memory.
 */
void GameplayState::unload() {
    unloading = true;
    
    for(Player& player : players) {
        if(player.hud) {
            player.hud->gui.destroy();
            delete player.hud;
            player.hud = nullptr;
            player.inventory->gui.destroy();
            delete player.inventory;
            player.inventory = nullptr;
        }
        
        player.leaderIdx = INVALID;
        player.leaderPtr = nullptr;
        
        player.closeToInteractableToUse = nullptr;
        player.closeToNestToOpen = nullptr;
        player.closeToPikminToPluck = nullptr;
        player.closeToShipToHeal = nullptr;
        
        player.view.cam.setPos(Point());
        player.view.cam.setZoom(1.0f);
        
        stopAllLeaders();
    }
    
    while(!mobs.all.empty()) {
        deleteMob(*mobs.all.begin(), true);
    }
    
    if(lightmapBmp) {
        al_destroy_bitmap(lightmapBmp);
        lightmapBmp = nullptr;
    }
    
    missionMobChecklists.clear();
    missionRemainingMobIds.clear();
    pathMgr.clear();
    particles.clear();
    
    for(size_t l = 0; l < liquids.size(); l++) {
        delete liquids[l];
    }
    liquids.clear();
    
    for(size_t t = 0; t < MAX_PLAYER_TEAMS; t++) {
        playerTeams[t].sprayStats.clear();
    }
    
    game.sysContent.anmSparks.clear();
    unloadGameContent();
    game.content.unloadCurrentArea(CONTENT_LOAD_LEVEL_FULL);
    
    if(bmpFog) {
        al_destroy_bitmap(bmpFog);
        bmpFog = nullptr;
    }
    
    if(msgBox) {
        delete msgBox;
        msgBox = nullptr;
    }
    if(onionMenu) {
        delete onionMenu;
        onionMenu = nullptr;
    }
    if(pauseMenu) {
        delete pauseMenu;
        pauseMenu = nullptr;
    }
    game.console.clear();
    
    unloading = false;
}


/**
 * @brief Unloads loaded game content.
 */
void GameplayState::unloadGameContent() {
    subgroupTypes.clear();
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
        CONTENT_TYPE_WEATHER_CONDITION,
        CONTENT_TYPE_MOB_TYPE,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_GUI,
    }
    );
}


/**
 * @brief Updates the list of leaders available to be controlled.
 */
void GameplayState::updateAvailableLeaders() {
    //Build the list.
    availableLeaders.clear();
    for(size_t l = 0; l < mobs.leaders.size(); l++) {
        if(mobs.leaders[l]->health <= 0.0f) continue;
        if(mobs.leaders[l]->toDelete) continue;
        if(mobs.leaders[l]->isStoredInsideMob()) continue;
        availableLeaders.push_back(mobs.leaders[l]);
    }
    
    if(availableLeaders.empty()) {
        return;
    }
    
    //Sort it so that it follows the expected leader order.
    //If there are multiple leaders of the same type, leaders with a lower
    //mob index number come first.
    std::sort(
        availableLeaders.begin(), availableLeaders.end(),
    [] (const Leader * l1, const Leader * l2) -> bool {
        size_t l1OrderIdx = INVALID;
        size_t l2OrderIdx = INVALID;
        for(size_t t = 0; t < game.config.leaders.order.size(); t++) {
            if(game.config.leaders.order[t] == l1->type) l1OrderIdx = t;
            if(game.config.leaders.order[t] == l2->type) l2OrderIdx = t;
        }
        if(l1OrderIdx == l2OrderIdx) {
            return l1->id < l2->id;
        }
        return l1OrderIdx < l2OrderIdx;
    }
    );
    
    //Update the current leader's index, which could've changed.
    for(Player& player : players) {
        for(size_t l = 0; l < availableLeaders.size(); l++) {
            if(availableLeaders[l] == player.leaderPtr) {
                player.leaderIdx = l;
                break;
            }
        }
    }
}


/**
 * @brief Updates the variables that indicate what the closest
 * group member of the standby subgroup is, for the current
 * standby subgroup, the previous, and the next.
 *
 * In the case all candidate members are out of reach,
 * this gets set to the closest. Otherwise, it gets set to the closest
 * and more mature one.
 * Sets to nullptr if there is no member of that subgroup available.
 *
 * @param player The player responsible.
 */
void GameplayState::updateClosestGroupMembers(Player* player) {
    player->closestGroupMember[BUBBLE_RELATION_PREVIOUS] = nullptr;
    player->closestGroupMember[BUBBLE_RELATION_CURRENT] = nullptr;
    player->closestGroupMember[BUBBLE_RELATION_NEXT] = nullptr;
    player->closestGroupMemberDistant = false;
    
    if(!player->leaderPtr) return;
    if(player->leaderPtr->group->members.empty()) {
        player->leaderPtr->updateThrowVariables();
        return;
    }
    
    //Get the closest group members for the three relevant subgroup types.
    SubgroupType* prevType;
    player->leaderPtr->group->getNextStandbyType(true, &prevType);
    
    if(prevType) {
        player->closestGroupMember[BUBBLE_RELATION_PREVIOUS] =
            getClosestGroupMember(player, prevType);
    }
    
    if(player->leaderPtr->group->curStandbyType) {
        player->closestGroupMember[BUBBLE_RELATION_CURRENT] =
            getClosestGroupMember(
                player,
                player->leaderPtr->group->curStandbyType,
                &player->closestGroupMemberDistant
            );
    }
    
    SubgroupType* nextType;
    player->leaderPtr->group->getNextStandbyType(false, &nextType);
    
    if(nextType) {
        player->closestGroupMember[BUBBLE_RELATION_NEXT] =
            getClosestGroupMember(player, nextType);
    }
    
    if(player->closestGroupMember[BUBBLE_RELATION_CURRENT]) {
        player->leaderPtr->updateThrowVariables();
    }
}


#pragma endregion
#pragma region Interlude info


/**
 * @brief Gets the current interlude's ID.
 *
 * @return The ID.
 */
INTERLUDE InterludeInfo::get() {
    return curId;
}


/**
 * @brief Gets the current interlude's time spent.
 *
 * @return The time.
 */
float InterludeInfo::getTime() {
    return curTime;
}


/**
 * @brief Overrides the time spent in the current interlude to be
 * the specified amount.
 *
 * @param time The new time.
 */
void InterludeInfo::overrideTime(float time) {
    curTime = time;
}


/**
 * @brief Sets the current interlude to be this one.
 *
 * @param id ID of the new interlude.
 * @param instantVolumeChange Whether the volume of sound effects should
 * change instantly or gradually.
 */
void InterludeInfo::set(INTERLUDE id, bool instantVolumeChange) {
    bool wasInInterlude = curId != INTERLUDE_NONE;
    
    curId = id;
    curTime = 0.0f;
    
    bool isInInterlude = curId != INTERLUDE_NONE;
    
    if(!wasInInterlude && isInInterlude) {
        game.audio.handleInterludeStart(instantVolumeChange);
        game.controls.setGameState(CONTROLS_GAME_STATE_INTERLUDE);
    } else if(wasInInterlude && !isInInterlude) {
        game.audio.handleInterludeEnd(instantVolumeChange);
        game.controls.setGameState(CONTROLS_GAME_STATE_GAMEPLAY);
    }
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void InterludeInfo::tick(float deltaT) {
    if(curId != INTERLUDE_NONE) {
        curTime += deltaT;
    }
}


#pragma endregion
#pragma region Mission mob checklist status


/**
 * @brief Marks a mob as cleared by removing it from the list, if it's there.
 *
 * @param m The mob.
 * @return Whether the mob is in the list.
 */
bool MissionMobChecklistStatus::remove(Mob* m) {
    auto it = std::find(remaining.begin(), remaining.end(), m);
    if(it == remaining.end()) {
        return false;
    }
    game.states.gameplay->lastMobClearedPos = m->pos;
    remaining.erase(it);
    return true;
}


#pragma endregion
