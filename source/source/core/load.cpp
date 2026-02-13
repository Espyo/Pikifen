/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Data loading and unloading functions.
 */

#include <algorithm>

#include <allegro5/allegro_ttf.h>

#include "load.h"

#include "../content/other/spike_damage.h"
#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "../util/string_utils.h"
#include "const.h"
#include "drawing.h"
#include "game.h"
#include "init.h"
#include "misc_functions.h"


using std::make_pair;
using std::set;


/**
 * @brief Loads a mission's record.
 *
 * @param file File data node to load from.
 * @param areaPtr The area's data.
 * @param record Record object to fill.
 */
void loadAreaMissionRecord(
    DataNode* file, Area* areaPtr, MissionRecord& record
) {
    string missionRecordEntryName =
        getMissionRecordEntryName(areaPtr);
        
    vector<string> recordParts =
        split(
            file->getChildByName(
                missionRecordEntryName
            )->value,
            ";", true
        );
        
    if(recordParts.size() == 3) {
        record.clear = recordParts[0] == "1";
        record.score = s2i(recordParts[1]);
        record.date = recordParts[2];
    }
}


/**
 * @brief Loads an audio stream from the game's content.
 *
 * @param filePath Name of the file to load.
 * @param node If not nullptr, blame this data node if the file
 * doesn't exist.
 * @param reportErrors Only issues errors if this is true.
 * @return The stream.
 */
ALLEGRO_AUDIO_STREAM* loadAudioStream(
    const string& filePath, DataNode* node, bool reportErrors
) {
    ALLEGRO_AUDIO_STREAM* stream =
        al_load_audio_stream((filePath).c_str(), 4, 2048);
        
    if(!stream && reportErrors) {
        game.errors.report(
            "Could not open audio stream file \"" + filePath + "\"!",
            node
        );
    }
    
    return stream;
}


/**
 * @brief Loads a bitmap from the game's content.
 *
 * @param filePath Path to the bitmap file.
 * @param node If present, it will be used to report errors, if any.
 * @param reportError If false, omits error reporting.
 * @param errorBmpOnError If true, returns the error bitmap in the case of an
 * error. Otherwise, returns nullptr.
 * @param errorBmpOnEmpty If true, returns the error bitmap in the case of an
 * empty file name. Otherwise, returns nullptr.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* loadBmp(
    const string& filePath, DataNode* node,
    bool reportError, bool errorBmpOnError,
    bool errorBmpOnEmpty
) {
    if(filePath.empty()) {
        if(errorBmpOnEmpty) {
            return game.bmpError;
        } else {
            return nullptr;
        }
    }
    
    ALLEGRO_BITMAP* b = al_load_bitmap((filePath).c_str());
    
    if(!b) {
        if(reportError) {
            game.errors.report(
                "Could not open image file \"" + filePath + "\"!",
                node
            );
        }
        if(errorBmpOnError) {
            b = game.bmpError;
        }
    }
    
    return b;
}


/**
 * @brief Loads a data file from the game's content.
 *
 * @param filePath Path to the file, relative to the program root folder.
 * @param outSuccess If not nullptr, whether the file was successfully
 * opened or not is returned here.
 */
DataNode loadDataFile(const string& filePath, bool* outSuccess) {
    bool fileWasOpened = false;
    DataNode node = DataNode(filePath, &fileWasOpened);
    if(!fileWasOpened) {
        game.errors.report(
            "Could not open data file \"" + filePath + "\"!"
        );
    }
    
    if(outSuccess) *outSuccess = fileWasOpened;
    return node;
}


/**
 * @brief Loads a font from the disk. If it's a bitmap it'll load it from
 * the bitmap and map the characters according to the ranges provided.
 * If it's a font file, it'll just load it directly.
 *
 * @param path Path to the file.
 * @param n Number of Unicode ranges in the bitmap, if it's a bitmap.
 * @param ranges "n" pairs of first and last Unicode point to map glyphs to
 * for each range, if it's a bitmap.
 * @param size Font size, if it's a font file.
 */
ALLEGRO_FONT* loadFont(
    const string& path, int n, const int ranges[], int size
) {
    const string& finalPath =
        game.content.bitmaps.manifests[path].path;
        
    ALLEGRO_FONT* result = nullptr;
    
    //First, try to load it as a TTF font.
    result =
        al_load_ttf_font(finalPath.c_str(), size, ALLEGRO_TTF_NO_KERNING);
        
    if(result) return result;
    
    //Now try as a bitmap.
    ALLEGRO_BITMAP* bmp = loadBmp(finalPath);
    result = al_grab_font_from_bitmap(bmp, n, ranges);
    al_destroy_bitmap(bmp);
    
    return result;
}


/**
 * @brief Loads the game's fonts.
 */
void loadFonts() {
    const int STANDARD_FONT_RANGES_SIZE = 2;
    const int STANDARD_FONT_RANGES[STANDARD_FONT_RANGES_SIZE] = {
        0x0020, 0x007E, //ASCII
        /*0x00A0, 0x00A1, //Non-breaking space and inverted !
        0x00BF, 0x00FF, //Inverted ? and European vowels and such*/
    };
    
    const int COUNTER_FONT_RANGES_SIZE = 6;
    const int COUNTER_FONT_RANGES[COUNTER_FONT_RANGES_SIZE] = {
        0x002D, 0x0039, //Dash, dot, slash, numbers
        0x003A, 0x003A, //Colon
        0x0078, 0x0078, //Lowercase x
    };
    
    const int JUST_NUMBERS_FONT_RANGES_SIZE = 2;
    const int JUST_NUMBERS_FONT_RANGES[JUST_NUMBERS_FONT_RANGES_SIZE] = {
        0x0030, 0x0039, //0 to 9
    };
    
    const int VALUE_FONT_RANGES_SIZE = 6;
    const int VALUE_FONT_RANGES[VALUE_FONT_RANGES_SIZE] = {
        0x0024, 0x0024, //Dollar sign
        0x002D, 0x002D, //Dash
        0x0030, 0x0039, //Numbers
    };
    
    //We can't load the fonts directly because we want to set the ranges.
    //So we load them into bitmaps first.
    
    //Area name font.
    game.sysContent.fntAreaName =
        loadFont(
            game.sysContentNames.fntAreaName,
            STANDARD_FONT_RANGES_SIZE / 2, STANDARD_FONT_RANGES,
            34
        );
        
    //Built-in font.
    game.sysContent.fntBuiltin = al_create_builtin_font();
    
    //Counter font.
    game.sysContent.fntCounter =
        loadFont(
            game.sysContentNames.fntCounter,
            COUNTER_FONT_RANGES_SIZE / 2, COUNTER_FONT_RANGES,
            32
        );
        
    //Leader cursor counter font.
    game.sysContent.fntLeaderCursorCounter =
        loadFont(
            game.sysContentNames.fntLeaderCursorCounter,
            JUST_NUMBERS_FONT_RANGES_SIZE / 2, JUST_NUMBERS_FONT_RANGES,
            16
        );
        
    //Slim font.
    game.sysContent.fntSlim =
        loadFont(
            game.sysContentNames.fntSlim,
            STANDARD_FONT_RANGES_SIZE / 2, STANDARD_FONT_RANGES,
            22
        );
        
    //Standard font.
    game.sysContent.fntStandard =
        loadFont(
            game.sysContentNames.fntStandard,
            STANDARD_FONT_RANGES_SIZE / 2, STANDARD_FONT_RANGES,
            22
        );
        
    //Value font.
    game.sysContent.fntValue =
        loadFont(
            game.sysContentNames.fntValue,
            VALUE_FONT_RANGES_SIZE / 2, VALUE_FONT_RANGES,
            16
        );
}


/**
 * @brief Loads the maker tools from the tool config file.
 */
void loadMakerTools() {
    bool fileWasOpened = false;
    DataNode file(FILE_PATHS_FROM_ROOT::MAKER_TOOLS, &fileWasOpened);
    if(!fileWasOpened) return;
    game.makerTools.loadFromDataNode(&file);
}


/**
 * @brief Loads miscellaneous fixed graphics.
 */
void loadMiscGraphics() {
    //Icon.
    game.sysContent.bmpIcon =
        game.content.bitmaps.list.get(game.sysContentNames.bmpIcon);
    al_set_display_icon(game.display, game.sysContent.bmpIcon);
    
    //Graphics.
    game.sysContent.bmpMenuIcons =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMenuIcons);
    game.sysContent.bmpArrowDown =
        game.content.bitmaps.list.get(game.sysContentNames.bmpArrowDown);
    game.sysContent.bmpArrowLeft =
        game.content.bitmaps.list.get(game.sysContentNames.bmpArrowLeft);
    game.sysContent.bmpArrowRight =
        game.content.bitmaps.list.get(game.sysContentNames.bmpArrowRight);
    game.sysContent.bmpArrowUp =
        game.content.bitmaps.list.get(game.sysContentNames.bmpArrowUp);
    game.sysContent.bmpBrightCircle =
        game.content.bitmaps.list.get(game.sysContentNames.bmpBrightCircle);
    game.sysContent.bmpBrightRing =
        game.content.bitmaps.list.get(game.sysContentNames.bmpBrightRing);
    game.sysContent.bmpBubbleBox =
        game.content.bitmaps.list.get(game.sysContentNames.bmpBubbleBox);
    game.sysContent.bmpButtonBox =
        game.content.bitmaps.list.get(game.sysContentNames.bmpButtonBox);
    game.sysContent.bmpCheckboxCheck =
        game.content.bitmaps.list.get(game.sysContentNames.bmpCheckboxCheck);
    game.sysContent.bmpCheckboxNoCheck =
        game.content.bitmaps.list.get(game.sysContentNames.bmpCheckboxNoCheck);
    game.sysContent.bmpChill =
        game.content.bitmaps.list.get(game.sysContentNames.bmpChill);
    game.sysContent.bmpClock =
        game.content.bitmaps.list.get(game.sysContentNames.bmpClock);
    game.sysContent.bmpClockHand =
        game.content.bitmaps.list.get(game.sysContentNames.bmpClockHand);
    game.sysContent.bmpDifficulty =
        game.content.bitmaps.list.get(game.sysContentNames.bmpDifficulty);
    game.sysContent.bmpDiscordIcon =
        game.content.bitmaps.list.get(game.sysContentNames.bmpDiscordIcon);
    game.sysContent.bmpEnemySoul =
        game.content.bitmaps.list.get(game.sysContentNames.bmpEnemySoul);
    game.sysContent.bmpFocusBox =
        game.content.bitmaps.list.get(game.sysContentNames.bmpFocusBox);
    game.sysContent.bmpFrameBox =
        game.content.bitmaps.list.get(game.sysContentNames.bmpFrameBox);
    game.sysContent.bmpFrozenLiquid =
        game.content.bitmaps.list.get(game.sysContentNames.bmpFrozenLiquid);
    game.sysContent.bmpFrozenLiquidCracked =
        game.content.bitmaps.list.get(
            game.sysContentNames.bmpFrozenLiquidCracked
        );
    game.sysContent.bmpGithubIcon =
        game.content.bitmaps.list.get(game.sysContentNames.bmpGithubIcon);
    game.sysContent.bmpHardBubble =
        game.content.bitmaps.list.get(game.sysContentNames.bmpHardBubble);
    game.sysContent.bmpIdleGlow =
        game.content.bitmaps.list.get(game.sysContentNames.bmpIdleGlow);
    game.sysContent.bmpKeyBox =
        game.content.bitmaps.list.get(game.sysContentNames.bmpKeyBox);
    game.sysContent.bmpLeaderCursor =
        game.content.bitmaps.list.get(game.sysContentNames.bmpLeaderCursor);
    game.sysContent.bmpLeaderPrompt =
        game.content.bitmaps.list.get(game.sysContentNames.bmpLeaderPrompt);
    game.sysContent.bmpLeaderSilhouetteSide =
        game.content.bitmaps.list.get(
            game.sysContentNames.bmpLeaderSilhouetteSide
        );
    game.sysContent.bmpLeaderSilhouetteTop =
        game.content.bitmaps.list.get(
            game.sysContentNames.bmpLeaderSilhouetteTop
        );
    game.sysContent.bmpLowHealthRing =
        game.content.bitmaps.list.get(game.sysContentNames.bmpLowHealthRing);
    game.sysContent.bmpMedalBronze =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMedalBronze);
    game.sysContent.bmpMedalGold =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMedalGold);
    game.sysContent.bmpMedalGotIt =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMedalGotIt);
    game.sysContent.bmpMedalNone =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMedalNone);
    game.sysContent.bmpMedalPlatinum =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMedalPlatinum);
    game.sysContent.bmpMedalSilver =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMedalSilver);
    game.sysContent.bmpMenuIcons =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMenuIcons);
    game.sysContent.bmpMissionClear =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMissionClear);
    game.sysContent.bmpMissionFail =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMissionFail);
    game.sysContent.bmpMissionMob =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMissionMob);
    game.sysContent.bmpMore =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMore);
    game.sysContent.bmpMouseCursor =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMouseCursor);
    game.sysContent.bmpNapsack =
        game.content.bitmaps.list.get(game.sysContentNames.bmpNapsack);
    game.sysContent.bmpOnionMenu1 =
        game.content.bitmaps.list.get(game.sysContentNames.bmpOnionMenu1);
    game.sysContent.bmpOnionMenu10 =
        game.content.bitmaps.list.get(game.sysContentNames.bmpOnionMenu10);
    game.sysContent.bmpOnionMenuAll =
        game.content.bitmaps.list.get(game.sysContentNames.bmpOnionMenuAll);
    game.sysContent.bmpOnionMenuSingle =
        game.content.bitmaps.list.get(game.sysContentNames.bmpOnionMenuSingle);
    game.sysContent.bmpPikminSoul =
        game.content.bitmaps.list.get(game.sysContentNames.bmpPikminSoul);
    game.sysContent.bmpPlayerInputIcons =
        game.content.bitmaps.list.get(game.sysContentNames.bmpPlayerInputIcons);
    game.sysContent.bmpRandom =
        game.content.bitmaps.list.get(game.sysContentNames.bmpRandom);
    game.sysContent.bmpRock =
        game.content.bitmaps.list.get(game.sysContentNames.bmpRock);
    game.sysContent.bmpShadow =
        game.content.bitmaps.list.get(game.sysContentNames.bmpShadow);
    game.sysContent.bmpShadowSquare =
        game.content.bitmaps.list.get(game.sysContentNames.bmpShadowSquare);
    game.sysContent.bmpSmack =
        game.content.bitmaps.list.get(game.sysContentNames.bmpSmack);
    game.sysContent.bmpSmoke =
        game.content.bitmaps.list.get(game.sysContentNames.bmpSmoke);
    game.sysContent.bmpSparkle =
        game.content.bitmaps.list.get(game.sysContentNames.bmpSparkle);
    game.sysContent.bmpSpotlight =
        game.content.bitmaps.list.get(game.sysContentNames.bmpSpotlight);
    game.sysContent.bmpSwarmArrow =
        game.content.bitmaps.list.get(game.sysContentNames.bmpSwarmArrow);
    game.sysContent.bmpThrowInvalid =
        game.content.bitmaps.list.get(game.sysContentNames.bmpThrowInvalid);
    game.sysContent.bmpThrowPreview =
        game.content.bitmaps.list.get(game.sysContentNames.bmpThrowPreview);
    game.sysContent.bmpThrowPreviewDashed =
        game.content.bitmaps.list.get(
            game.sysContentNames.bmpThrowPreviewDashed
        );
    game.sysContent.bmpVignette =
        game.content.bitmaps.list.get(game.sysContentNames.bmpVignette);
    game.sysContent.bmpWarning =
        game.content.bitmaps.list.get(game.sysContentNames.bmpWarning);
    game.sysContent.bmpWaveRing =
        game.content.bitmaps.list.get(game.sysContentNames.bmpWaveRing);
}


/**
 * @brief Loads miscellaneous fixed sound effects.
 */
void loadMiscSounds() {
    game.audio.baseMasterMixerVolume =
        game.options.audio.masterVol;
    game.audio.baseGameplaySoundMixerVolume =
        game.options.audio.gameplaySoundVol;
    game.audio.baseMusicMixerVolume =
        game.options.audio.musicVol;
    game.audio.baseAmbianceSoundMixerVolume =
        game.options.audio.ambianceSoundVol;
    game.audio.baseUiSoundMixerVolume =
        game.options.audio.uiSoundVol;
    game.audio.init();
    
    //Sound effects.
    game.sysContent.sndAttack =
        game.content.sounds.list.get(game.sysContentNames.sndAttack);
    game.sysContent.sndCamera =
        game.content.sounds.list.get(game.sysContentNames.sndCamera);
    game.sysContent.sndCountdownTick =
        game.content.sounds.list.get(game.sysContentNames.sndCountdownTick);
    game.sysContent.sndDing =
        game.content.sounds.list.get(game.sysContentNames.sndDing);
    game.sysContent.sndEnemySoul =
        game.content.sounds.list.get(game.sysContentNames.sndEnemySoul);
    game.sysContent.sndFrozenLiquid =
        game.content.sounds.list.get(game.sysContentNames.sndFrozenLiquid);
    game.sysContent.sndFrozenLiquidCrack =
        game.content.sounds.list.get(game.sysContentNames.sndFrozenLiquidCrack);
    game.sysContent.sndFrozenLiquidThaw =
        game.content.sounds.list.get(game.sysContentNames.sndFrozenLiquidThaw);
    game.sysContent.sndGameplayMsgChar =
        game.content.sounds.list.get(game.sysContentNames.sndGameplayMsgChar);
    game.sysContent.sndGo =
        game.content.sounds.list.get(game.sysContentNames.sndGo);
    game.sysContent.sndMedalGotIt =
        game.content.sounds.list.get(game.sysContentNames.sndMedalGotIt);
    game.sysContent.sndMenuActivate =
        game.content.sounds.list.get(game.sysContentNames.sndMenuActivate);
    game.sysContent.sndMenuBack =
        game.content.sounds.list.get(game.sysContentNames.sndMenuBack);
    game.sysContent.sndMenuFail =
        game.content.sounds.list.get(game.sysContentNames.sndMenuFail);
    game.sysContent.sndMissionClear =
        game.content.sounds.list.get(game.sysContentNames.sndMissionClear);
    game.sysContent.sndMissionFailed =
        game.content.sounds.list.get(game.sysContentNames.sndMissionFailed);
    game.sysContent.sndOneMinuteLeft =
        game.content.sounds.list.get(game.sysContentNames.sndOneMinuteLeft);
    game.sysContent.sndReady =
        game.content.sounds.list.get(game.sysContentNames.sndReady);
    game.sysContent.sndMenuFocus =
        game.content.sounds.list.get(game.sysContentNames.sndMenuFocus);
    game.sysContent.sndSwitchPikmin =
        game.content.sounds.list.get(game.sysContentNames.sndSwitchPikmin);
}


/**
 * @brief Loads the player's options.
 */
void loadOptions() {
    bool fileWasOpened = false;
    DataNode file = DataNode(FILE_PATHS_FROM_ROOT::OPTIONS, &fileWasOpened);
    if(!fileWasOpened) return;
    
    //Init game controllers.
    game.hardware.updateControllers(true);
    
    //Read the main options.
    game.options.loadFromDataNode(&file);
    
    //Final setup.
    game.winFullscreen = game.options.graphics.intendedWinFullscreen;
    game.winW = game.options.graphics.intendedWinW;
    game.winH = game.options.graphics.intendedWinH;
}


/**
 * @brief Loads an audio sample from the game's content.
 *
 * @param filePath Path to the file to load.
 * @param node If not nullptr, blame this data node if the file
 * doesn't exist.
 * @param reportErrors Only issues errors if this is true.
 * @return The sample.
 */
ALLEGRO_SAMPLE* loadSample(
    const string& filePath, DataNode* node, bool reportErrors
) {
    ALLEGRO_SAMPLE* sample = al_load_sample((filePath).c_str());
    
    if(!sample && reportErrors) {
        game.errors.report(
            "Could not open audio sample file \"" + filePath + "\"!",
            node
        );
    }
    
    return sample;
}


/**
 * @brief Loads the engine's lifetime statistics.
 */
void loadStatistics() {
    DataNode statsFile;
    bool fileWasOpened = false;
    statsFile.loadFile(
        FILE_PATHS_FROM_ROOT::STATISTICS, &fileWasOpened, true, false, true
    );
    if(!fileWasOpened) return;
    
    Statistics& s = game.statistics;
    
    ReaderSetter sRS(&statsFile);
    
    sRS.set("startups",               s.startups);
    sRS.set("runtime",                s.runtime);
    sRS.set("gameplay_time",          s.gameplayTime);
    sRS.set("area_entries",           s.areaEntries);
    sRS.set("pikmin_births",          s.pikminBirths);
    sRS.set("pikmin_deaths",          s.pikminDeaths);
    sRS.set("pikmin_eaten",           s.pikminEaten);
    sRS.set("pikmin_hazard_deaths",   s.pikminHazardDeaths);
    sRS.set("pikmin_blooms",          s.pikminBlooms);
    sRS.set("pikmin_saved",           s.pikminSaved);
    sRS.set("enemy_defeats",          s.enemyDefeats);
    sRS.set("pikmin_thrown",          s.pikminThrown);
    sRS.set("whistle_uses",           s.whistleUses);
    sRS.set("distance_walked",        s.distanceWalked);
    sRS.set("leader_damage_suffered", s.leaderDamageSuffered);
    sRS.set("punch_damage_caused",    s.punchDamageCaused);
    sRS.set("leader_kos",             s.leaderKos);
    sRS.set("sprays_used",            s.spraysUsed);
}


/**
 * @brief Unloads miscellaneous graphics, sounds, and other resources.
 */
void unloadMiscResources() {
    //Graphics.
    game.content.bitmaps.list.free(game.sysContent.bmpArrowDown);
    game.content.bitmaps.list.free(game.sysContent.bmpArrowLeft);
    game.content.bitmaps.list.free(game.sysContent.bmpArrowRight);
    game.content.bitmaps.list.free(game.sysContent.bmpArrowUp);
    game.content.bitmaps.list.free(game.sysContent.bmpBrightCircle);
    game.content.bitmaps.list.free(game.sysContent.bmpBrightRing);
    game.content.bitmaps.list.free(game.sysContent.bmpBubbleBox);
    game.content.bitmaps.list.free(game.sysContent.bmpButtonBox);
    game.content.bitmaps.list.free(game.sysContent.bmpCheckboxCheck);
    game.content.bitmaps.list.free(game.sysContent.bmpCheckboxNoCheck);
    game.content.bitmaps.list.free(game.sysContent.bmpChill);
    game.content.bitmaps.list.free(game.sysContent.bmpClock);
    game.content.bitmaps.list.free(game.sysContent.bmpClockHand);
    game.content.bitmaps.list.free(game.sysContent.bmpDifficulty);
    game.content.bitmaps.list.free(game.sysContent.bmpDiscordIcon);
    game.content.bitmaps.list.free(game.sysContent.bmpEnemySoul);
    game.content.bitmaps.list.free(game.sysContent.bmpFocusBox);
    game.content.bitmaps.list.free(game.sysContent.bmpFrameBox);
    game.content.bitmaps.list.free(game.sysContent.bmpFrozenLiquid);
    game.content.bitmaps.list.free(game.sysContent.bmpFrozenLiquidCracked);
    game.content.bitmaps.list.free(game.sysContent.bmpGithubIcon);
    game.content.bitmaps.list.free(game.sysContent.bmpHardBubble);
    game.content.bitmaps.list.free(game.sysContent.bmpIcon);
    game.content.bitmaps.list.free(game.sysContent.bmpIdleGlow);
    game.content.bitmaps.list.free(game.sysContent.bmpKeyBox);
    game.content.bitmaps.list.free(game.sysContent.bmpLeaderCursor);
    game.content.bitmaps.list.free(game.sysContent.bmpLeaderPrompt);
    game.content.bitmaps.list.free(game.sysContent.bmpLeaderSilhouetteSide);
    game.content.bitmaps.list.free(game.sysContent.bmpLeaderSilhouetteTop);
    game.content.bitmaps.list.free(game.sysContent.bmpLowHealthRing);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalBronze);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalGold);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalGotIt);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalNone);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalPlatinum);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalSilver);
    game.content.bitmaps.list.free(game.sysContent.bmpMenuIcons);
    game.content.bitmaps.list.free(game.sysContent.bmpMissionClear);
    game.content.bitmaps.list.free(game.sysContent.bmpMissionFail);
    game.content.bitmaps.list.free(game.sysContent.bmpMissionMob);
    game.content.bitmaps.list.free(game.sysContent.bmpMore);
    game.content.bitmaps.list.free(game.sysContent.bmpMouseCursor);
    game.content.bitmaps.list.free(game.sysContent.bmpNapsack);
    game.content.bitmaps.list.free(game.sysContent.bmpOnionMenu1);
    game.content.bitmaps.list.free(game.sysContent.bmpOnionMenu10);
    game.content.bitmaps.list.free(game.sysContent.bmpOnionMenuAll);
    game.content.bitmaps.list.free(game.sysContent.bmpOnionMenuSingle);
    game.content.bitmaps.list.free(game.sysContent.bmpPikminSoul);
    game.content.bitmaps.list.free(game.sysContent.bmpPlayerInputIcons);
    game.content.bitmaps.list.free(game.sysContent.bmpRandom);
    game.content.bitmaps.list.free(game.sysContent.bmpRock);
    game.content.bitmaps.list.free(game.sysContent.bmpShadow);
    game.content.bitmaps.list.free(game.sysContent.bmpShadowSquare);
    game.content.bitmaps.list.free(game.sysContent.bmpSmack);
    game.content.bitmaps.list.free(game.sysContent.bmpSmoke);
    game.content.bitmaps.list.free(game.sysContent.bmpSparkle);
    game.content.bitmaps.list.free(game.sysContent.bmpSpotlight);
    game.content.bitmaps.list.free(game.sysContent.bmpSwarmArrow);
    game.content.bitmaps.list.free(game.sysContent.bmpThrowInvalid);
    game.content.bitmaps.list.free(game.sysContent.bmpThrowPreview);
    game.content.bitmaps.list.free(game.sysContent.bmpThrowPreviewDashed);
    game.content.bitmaps.list.free(game.sysContent.bmpVignette);
    game.content.bitmaps.list.free(game.sysContent.bmpWarning);
    game.content.bitmaps.list.free(game.sysContent.bmpWaveRing);
    
    //Fonts.
    al_destroy_font(game.sysContent.fntAreaName);
    al_destroy_font(game.sysContent.fntCounter);
    al_destroy_font(game.sysContent.fntLeaderCursorCounter);
    al_destroy_font(game.sysContent.fntSlim);
    al_destroy_font(game.sysContent.fntStandard);
    al_destroy_font(game.sysContent.fntValue);
    
    //Sounds effects.
    game.content.sounds.list.free(game.sysContent.sndAttack);
    game.content.sounds.list.free(game.sysContent.sndCamera);
    game.content.sounds.list.free(game.sysContent.sndCountdownTick);
    game.content.sounds.list.free(game.sysContent.sndDing);
    game.content.sounds.list.free(game.sysContent.sndEnemySoul);
    game.content.sounds.list.free(game.sysContent.sndFrozenLiquid);
    game.content.sounds.list.free(game.sysContent.sndFrozenLiquidCrack);
    game.content.sounds.list.free(game.sysContent.sndFrozenLiquidThaw);
    game.content.sounds.list.free(game.sysContent.sndGameplayMsgChar);
    game.content.sounds.list.free(game.sysContent.sndGo);
    game.content.sounds.list.free(game.sysContent.sndMedalGotIt);
    game.content.sounds.list.free(game.sysContent.sndMenuActivate);
    game.content.sounds.list.free(game.sysContent.sndMenuBack);
    game.content.sounds.list.free(game.sysContent.sndMenuFail);
    game.content.sounds.list.free(game.sysContent.sndMenuFocus);
    game.content.sounds.list.free(game.sysContent.sndMissionClear);
    game.content.sounds.list.free(game.sysContent.sndMissionFailed);
    game.content.sounds.list.free(game.sysContent.sndOneMinuteLeft);
    game.content.sounds.list.free(game.sysContent.sndReady);
    game.content.sounds.list.free(game.sysContent.sndSwitchPikmin);
}
