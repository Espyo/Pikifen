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
 * @param path Path to the bitmap file.
 * @param node If present, it will be used to report errors, if any.
 * @param reportError If false, omits error reporting.
 * @param errorBmpOnError If true, returns the error bitmap in the case of an
 * error. Otherwise, returns nullptr.
 * @param errorBmpOnEmpty If true, returns the error bitmap in the case of an
 * empty file name. Otherwise, returns nullptr.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* loadBmp(
    const string& path, DataNode* node,
    bool reportError, bool errorBmpOnError,
    bool errorBmpOnEmpty
) {
    if(path.empty()) {
        if(errorBmpOnEmpty) {
            return game.bmpError;
        } else {
            return nullptr;
        }
    }
    
    ALLEGRO_BITMAP* b = al_load_bitmap((path).c_str());
    
    if(!b) {
        if(reportError) {
            game.errors.report(
                "Could not open image \"" + path + "\"!",
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
 */
DataNode loadDataFile(const string& filePath) {
    DataNode n = DataNode(filePath);
    if(!n.fileWasOpened) {
        game.errors.report(
            "Could not open data file \"" + filePath + "\"!"
        );
    }
    
    return n;
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
        
    //Cursor counter font.
    game.sysContent.fntCursorCounter =
        loadFont(
            game.sysContentNames.fntCursorCounter,
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
    DataNode file(FILE_PATHS_FROM_ROOT::MAKER_TOOLS);
    if(!file.fileWasOpened) return;
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
    game.sysContent.bmpCursor =
        game.content.bitmaps.list.get(game.sysContentNames.bmpCursor);
    game.sysContent.bmpDiscordIcon =
        game.content.bitmaps.list.get(game.sysContentNames.bmpDiscordIcon);
    game.sysContent.bmpEnemySpirit =
        game.content.bitmaps.list.get(game.sysContentNames.bmpEnemySpirit);
    game.sysContent.bmpFocusBox =
        game.content.bitmaps.list.get(game.sysContentNames.bmpFocusBox);
    game.sysContent.bmpFrameBox =
        game.content.bitmaps.list.get(game.sysContentNames.bmpFrameBox);
    game.sysContent.bmpGithubIcon =
        game.content.bitmaps.list.get(game.sysContentNames.bmpGithubIcon);
    game.sysContent.bmpHardBubble =
        game.content.bitmaps.list.get(game.sysContentNames.bmpHardBubble);
    game.sysContent.bmpIdleGlow =
        game.content.bitmaps.list.get(game.sysContentNames.bmpIdleGlow);
    game.sysContent.bmpKeyBox =
        game.content.bitmaps.list.get(game.sysContentNames.bmpKeyBox);
    game.sysContent.bmpLeaderSilhouetteSide =
        game.content.bitmaps.list.get(
            game.sysContentNames.bmpLeaderSilhouetteSide
        );
    game.sysContent.bmpLeaderSilhouetteTop =
        game.content.bitmaps.list.get(
            game.sysContentNames.bmpLeaderSilhouetteTop
        );
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
    game.sysContent.bmpMore =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMore);
    game.sysContent.bmpMouseCursor =
        game.content.bitmaps.list.get(game.sysContentNames.bmpMouseCursor);
    game.sysContent.bmpNotification =
        game.content.bitmaps.list.get(game.sysContentNames.bmpNotification);
    game.sysContent.bmpPikminSpirit =
        game.content.bitmaps.list.get(game.sysContentNames.bmpPikminSpirit);
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
    game.sysContent.bmpWaveRing =
        game.content.bitmaps.list.get(game.sysContentNames.bmpWaveRing);
}


/**
 * @brief Loads miscellaneous fixed sound effects.
 */
void loadMiscSounds() {
    game.audio.init(
        game.options.audio.masterVol,
        game.options.audio.gameplaySoundVol,
        game.options.audio.musicVol,
        game.options.audio.ambianceSoundVol,
        game.options.audio.uiSoundVol
    );
    
    //Sound effects.
    game.sysContent.sndAttack =
        game.content.sounds.list.get(game.sysContentNames.sndAttack);
    game.sysContent.sndCamera =
        game.content.sounds.list.get(game.sysContentNames.sndCamera);
    game.sysContent.sndGo =
        game.content.sounds.list.get(game.sysContentNames.sndGo);
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
    game.sysContent.sndReady =
        game.content.sounds.list.get(game.sysContentNames.sndReady);
    game.sysContent.sndMenuSelect =
        game.content.sounds.list.get(game.sysContentNames.sndMenuSelect);
    game.sysContent.sndSwitchPikmin =
        game.content.sounds.list.get(game.sysContentNames.sndSwitchPikmin);
}


/**
 * @brief Loads the player's options.
 */
void loadOptions() {
    DataNode file = DataNode(FILE_PATHS_FROM_ROOT::OPTIONS);
    if(!file.fileWasOpened) return;
    
    //Init game controllers.
    game.controllerNumbers.clear();
    int nJoysticks = al_get_num_joysticks();
    for(int j = 0; j < nJoysticks; j++) {
        game.controllerNumbers[al_get_joystick(j)] = j;
    }
    
    //Read the main options.
    game.options.loadFromDataNode(&file);
    
    //Final setup.
    game.winFullscreen = game.options.graphics.intendedWinFullscreen;
    game.winW = game.options.graphics.intendedWinW;
    game.winH = game.options.graphics.intendedWinH;
    
    ControlsManagerOptions controlsMgrOptions;
    controlsMgrOptions.stickMinDeadzone =
        game.options.advanced.joystickMinDeadzone;
    controlsMgrOptions.stickMaxDeadzone =
        game.options.advanced.joystickMaxDeadzone;
    game.controls.setOptions(controlsMgrOptions);
}


/**
 * @brief Loads an audio sample from the game's content.
 *
 * @param path Path to the file to load.
 * @param node If not nullptr, blame this data node if the file
 * doesn't exist.
 * @param reportErrors Only issues errors if this is true.
 * @return The sample.
 */
ALLEGRO_SAMPLE* loadSample(
    const string& path, DataNode* node, bool reportErrors
) {
    ALLEGRO_SAMPLE* sample = al_load_sample((path).c_str());
    
    if(!sample && reportErrors) {
        game.errors.report(
            "Could not open audio file \"" + path + "\"!",
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
    statsFile.loadFile(FILE_PATHS_FROM_ROOT::STATISTICS, true, false, true);
    if(!statsFile.fileWasOpened) return;
    
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
    game.content.bitmaps.list.free(game.sysContent.bmpBrightCircle);
    game.content.bitmaps.list.free(game.sysContent.bmpBrightRing);
    game.content.bitmaps.list.free(game.sysContent.bmpBubbleBox);
    game.content.bitmaps.list.free(game.sysContent.bmpButtonBox);
    game.content.bitmaps.list.free(game.sysContent.bmpCheckboxCheck);
    game.content.bitmaps.list.free(game.sysContent.bmpCheckboxNoCheck);
    game.content.bitmaps.list.free(game.sysContent.bmpCursor);
    game.content.bitmaps.list.free(game.sysContent.bmpDiscordIcon);
    game.content.bitmaps.list.free(game.sysContent.bmpEnemySpirit);
    game.content.bitmaps.list.free(game.sysContent.bmpFocusBox);
    game.content.bitmaps.list.free(game.sysContent.bmpFrameBox);
    game.content.bitmaps.list.free(game.sysContent.bmpGithubIcon);
    game.content.bitmaps.list.free(game.sysContent.bmpHardBubble);
    game.content.bitmaps.list.free(game.sysContent.bmpIcon);
    game.content.bitmaps.list.free(game.sysContent.bmpIdleGlow);
    game.content.bitmaps.list.free(game.sysContent.bmpKeyBox);
    game.content.bitmaps.list.free(game.sysContent.bmpLeaderSilhouetteSide);
    game.content.bitmaps.list.free(game.sysContent.bmpLeaderSilhouetteTop);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalBronze);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalGold);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalGotIt);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalNone);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalPlatinum);
    game.content.bitmaps.list.free(game.sysContent.bmpMedalSilver);
    game.content.bitmaps.list.free(game.sysContent.bmpMenuIcons);
    game.content.bitmaps.list.free(game.sysContent.bmpMissionClear);
    game.content.bitmaps.list.free(game.sysContent.bmpMissionFail);
    game.content.bitmaps.list.free(game.sysContent.bmpMore);
    game.content.bitmaps.list.free(game.sysContent.bmpMouseCursor);
    game.content.bitmaps.list.free(game.sysContent.bmpNotification);
    game.content.bitmaps.list.free(game.sysContent.bmpPikminSpirit);
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
    game.content.bitmaps.list.free(game.sysContent.bmpWaveRing);
    
    //Fonts.
    al_destroy_font(game.sysContent.fntAreaName);
    al_destroy_font(game.sysContent.fntCounter);
    al_destroy_font(game.sysContent.fntCursorCounter);
    al_destroy_font(game.sysContent.fntSlim);
    al_destroy_font(game.sysContent.fntStandard);
    al_destroy_font(game.sysContent.fntValue);
    
    //Sounds effects.
    game.content.sounds.list.free(game.sysContent.sndAttack);
    game.content.sounds.list.free(game.sysContent.sndCamera);
    game.content.sounds.list.free(game.sysContent.sndGo);
    game.content.sounds.list.free(game.sysContent.sndMenuActivate);
    game.content.sounds.list.free(game.sysContent.sndMenuBack);
    game.content.sounds.list.free(game.sysContent.sndMenuFail);
    game.content.sounds.list.free(game.sysContent.sndMenuSelect);
    game.content.sounds.list.free(game.sysContent.sndMissionClear);
    game.content.sounds.list.free(game.sysContent.sndMissionFailed);
    game.content.sounds.list.free(game.sysContent.sndReady);
    game.content.sounds.list.free(game.sysContent.sndSwitchPikmin);
}
