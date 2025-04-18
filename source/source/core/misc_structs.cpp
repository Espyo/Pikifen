/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Miscellaneous structures, too small
 * to warrant their own files.
 */

#undef _CMATH_

#include <algorithm>
#include <climits>
#include <iostream>

#include "misc_structs.h"

#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "../util/string_utils.h"
#include "const.h"
#include "drawing.h"
#include "game.h"
#include "load.h"
#include "misc_functions.h"


namespace GAMEPLAY_MSG_BOX {

//How quickly the advance button icon fades, in alpha (0-1) per second.
const float ADVANCE_BUTTON_FADE_SPEED = 4.0f;

//How many pixels of margin between the message box and window borders.
const float MARGIN = 16.0f;

//How long to protect the player from misinputs for.
const float MISINPUT_PROTECTION_DURATION = 0.75f;

//How many pixels of padding between the message box borders and text.
const float PADDING = 8.0f;

//How long each token animates for when being shown.
const float TOKEN_ANIM_DURATION = 0.5f;

//How much to move a token in the X direction when animating it.
const float TOKEN_ANIM_X_AMOUNT = 7.0f;

//How much to move a token in the Y direction when animating it.
const float TOKEN_ANIM_Y_AMOUNT = 3.0f;

//How long to swipe the current section's tokens away for.
const float TOKEN_SWIPE_DURATION = 0.45f;

//How much to move a token in the X direction when swiping it away.
const float TOKEN_SWIPE_X_AMOUNT = -2.0f;

//How much to move a token in the Y direction when swiping it away.
const float TOKEN_SWIPE_Y_AMOUNT = -15.0f;

}


namespace NOTIFICATION {

//How quickly it fades, in alpha per second.
const float FADE_SPEED = 4.0f;

}


namespace WHISTLE {

//R, G, and B components for each dot color.
const unsigned char DOT_COLORS[N_DOT_COLORS][3] = {
    {214, 25,  25 }, //Red.
    {242, 134, 48 }, //Orange.
    {143, 227, 58 }, //Lime.
    {55,  222, 222}, //Cyan.
    {30,  30,  219}, //Blue.
    {133, 28,  237}, //Purple.
};

//Seconds that need to pass before another dot is added.
const float DOT_INTERVAL = 0.03;

//A whistle dot spins these many radians a second.
const float DOT_SPIN_SPEED = TAU / 4;

//Time the whistle animations take to fade out.
const float FADE_TIME = 0.13f;

//R, G, and B components for each ring color.
const unsigned char RING_COLORS[N_RING_COLORS][3] = {
    {255, 255, 0  },
    {255, 0,   0  },
    {255, 0,   255},
    {128, 0,   255},
    {0,   0,   255},
    {0,   255, 255},
    {0,   255, 0  },
    {128, 255, 0  }
};

//Whistle rings move these many units per second.
const float RING_SPEED = 600.0f;

//Seconds that need to pass before another whistle ring appears.
const float RINGS_INTERVAL = 0.1f;

}


namespace MAKER_TOOLS {

//Internal names of each maker tool.
const string NAMES[N_MAKER_TOOLS] = {
    "",
    "area_image",
    "change_speed",
    "collision",
    "geometry_info",
    "hitboxes",
    "hud",
    "hurt_mob",
    "mob_info",
    "new_pikmin",
    "path_info",
    "set_song_pos_near_loop",
    "teleport"
};

}


/**
 * @brief Loads the system content names from a file.
 *
 * @param file File to load from.
 */
void SystemContentNames::load(DataNode* file) {
    ReaderSetter gra_rs(file->getChildByName("graphics"));
    
    gra_rs.set("bright_circle", bmpBrightCircle);
    gra_rs.set("bright_ring", bmpBrightRing);
    gra_rs.set("bubble_box", bmpBubbleBox);
    gra_rs.set("button_box", bmpButtonBox);
    gra_rs.set("checkbox_check", bmpCheckboxCheck);
    gra_rs.set("checkbox_no_check", bmpCheckboxNoCheck);
    gra_rs.set("cursor", bmpCursor);
    gra_rs.set("discord_icon", bmpDiscordIcon);
    gra_rs.set("editor_icons", bmpEditorIcons);
    gra_rs.set("enemy_spirit", bmpEnemySpirit);
    gra_rs.set("focus_box", bmpFocusBox);
    gra_rs.set("frame_box", bmpFrameBox);
    gra_rs.set("github_icon", bmpGithubIcon);
    gra_rs.set("hard_bubble", bmpHardBubble);
    gra_rs.set("icon", bmpIcon);
    gra_rs.set("idle_glow", bmpIdleGlow);
    gra_rs.set("key_box", bmpKeyBox);
    gra_rs.set("leader_silhouette_side", bmpLeaderSilhouetteSide);
    gra_rs.set("leader_silhouette_top", bmpLeaderSilhouetteTop);
    gra_rs.set("medal_bronze", bmpMedalBronze);
    gra_rs.set("medal_gold", bmpMedalGold);
    gra_rs.set("medal_none", bmpMedalNone);
    gra_rs.set("medal_platinum", bmpMedalPlatinum);
    gra_rs.set("medal_silver", bmpMedalSilver);
    gra_rs.set("menu_icons", bmpMenuIcons);
    gra_rs.set("mission_clear", bmpMissionClear);
    gra_rs.set("mission_fail", bmpMissionFail);
    gra_rs.set("more", bmpMore);
    gra_rs.set("mouse_cursor", bmpMouseCursor);
    gra_rs.set("notification", bmpNotification);
    gra_rs.set("pikmin_spirit", bmpPikminSpirit);
    gra_rs.set("player_input_icons", bmpPlayerInputIcons);
    gra_rs.set("random", bmpRandom);
    gra_rs.set("rock", bmpRock);
    gra_rs.set("shadow", bmpShadow);
    gra_rs.set("shadow_square", bmpShadowSquare);
    gra_rs.set("smack", bmpSmack);
    gra_rs.set("smoke", bmpSmoke);
    gra_rs.set("sparkle", bmpSparkle);
    gra_rs.set("spotlight", bmpSpotlight);
    gra_rs.set("swarm_arrow", bmpSwarmArrow);
    gra_rs.set("throw_invalid", bmpThrowInvalid);
    gra_rs.set("throw_preview", bmpThrowPreview);
    gra_rs.set("throw_preview_dashed", bmpThrowPreviewDashed);
    gra_rs.set("title_screen_bg", bmpTitleScreenBg);
    gra_rs.set("wave_ring", bmpWaveRing);
    
    ReaderSetter fnt_rs(file->getChildByName("fonts"));
    
    fnt_rs.set("area_name", fntAreaName);
    fnt_rs.set("counter", fntCounter);
    fnt_rs.set("cursor_counter", fntCursorCounter);
    fnt_rs.set("editor_header", fntEditorHeader);
    fnt_rs.set("editor_monospace", fntEditorMonospace);
    fnt_rs.set("editor_standard", fntEditorStandard);
    fnt_rs.set("slim", fntSlim);
    fnt_rs.set("standard", fntStandard);
    fnt_rs.set("value", fntValue);
    
    ReaderSetter snd_rs(file->getChildByName("sounds"));
    
    snd_rs.set("attack", sndAttack);
    snd_rs.set("camera", sndCamera);
    snd_rs.set("menu_activate", sndMenuActivate);
    snd_rs.set("menu_back", sndMenuBack);
    snd_rs.set("menu_select", sndMenuSelect);
    snd_rs.set("switch_pikmin", sndSwitchPikmin);
    
    ReaderSetter sng_rs(file->getChildByName("songs"));
    
    sng_rs.set("boss", sngBoss);
    sng_rs.set("boss_victory", sngBossVictory);
    sng_rs.set("editors", sngEditors);
    sng_rs.set("menus", sngMenus);
    
    ReaderSetter ani_rs(file->getChildByName("animations"));
    
    ani_rs.set("sparks", anmSparks);
    
    ReaderSetter par_rs(file->getChildByName("particle_generators"));
    
    par_rs.set("converter_insertion", parConverterInsertion);
    par_rs.set("ding", parDing);
    par_rs.set("enemy_death", parEnemyDeath);
    par_rs.set("leader_heal", parLeaderHeal);
    par_rs.set("leader_land", parLeaderLand);
    par_rs.set("onion_generating_inside", parOnionGenInside);
    par_rs.set("onion_insertion", parOnionInsertion);
    par_rs.set("pikmin_pluck_dirt", parPikminPluckDirt);
    par_rs.set("pikmin_seed_landed", parPikminSeedLanded);
    par_rs.set("smack", parSmack);
    par_rs.set("spray", parSpray);
    par_rs.set("sprout_evolution", parSproutEvolution);
    par_rs.set("sprout_regression", parSproutRegression);
    par_rs.set("throw_trail", parThrowTrail);
    par_rs.set("treasure", parTreasure);
    par_rs.set("wave_ring", parWaveRing);
}


/**
 * @brief Loads an audio stream for the manager.
 *
 * @param name Name of the audio stream to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The audio stream.
 */
ALLEGRO_AUDIO_STREAM* AudioStreamManager::doLoad(
    const string &name, DataNode* node, bool report_errors
) {
    const auto &it = game.content.songTracks.manifests.find(name);
    string path =
        it != game.content.songTracks.manifests.end() ?
        it->second.path :
        name;
    ALLEGRO_AUDIO_STREAM* stream = loadAudioStream(path, node, report_errors);
    if(stream) {
        game.registerAudioStreamSource(stream);
    }
    return stream;
}


/**
 * @brief Unloads an audio stream for the manager.
 *
 * @param asset Audio stream to unload.
 */
void AudioStreamManager::doUnload(ALLEGRO_AUDIO_STREAM* asset) {
    al_drain_audio_stream(asset);
    game.unregisterAudioStreamSource(asset);
    al_destroy_audio_stream(asset);
}


/**
 * @brief Loads a bitmap for the manager.
 *
 * @param name Name of the bitmap to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* BitmapManager::doLoad(
    const string &name, DataNode* node, bool report_errors
) {
    const auto &it = game.content.bitmaps.manifests.find(name);
    string path =
        it != game.content.bitmaps.manifests.end() ?
        it->second.path :
        name;
    return loadBmp(path, node, report_errors);
}


/**
 * @brief Unloads a bitmap for the manager.
 *
 * @param asset Bitmap to unload.
 */
void BitmapManager::doUnload(ALLEGRO_BITMAP* asset) {
    if(asset != game.bmpError) {
        al_destroy_bitmap(asset);
    }
}


/**
 * @brief Instantly places the camera at the specified coordinates.
 *
 * @param new_pos Coordinates to place the camera at.
 */
void Camera::setPos(const Point &new_pos) {
    pos = new_pos;
    targetPos = new_pos;
}


/**
 * @brief Instantly places the camera at the specified zoom.
 *
 * @param new_zoom Zoom to set to.
 */
void Camera::set_zoom(float new_zoom) {
    zoom = new_zoom;
    targetZoom = new_zoom;
}


/**
 * @brief Ticks camera movement by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Camera::tick(float delta_t) {
    pos.x +=
        (targetPos.x - pos.x) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * delta_t);
    pos.y +=
        (targetPos.y - pos.y) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * delta_t);
    zoom +=
        (targetZoom - zoom) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * delta_t);
}


/**
 * @brief Updates the camera's visibility box,
 * based on the windowToWorldTransform transformation.
 */
void Camera::updateBox() {
    box[0] = Point(0.0f);
    box[1] = Point(game.winW, game.winH);
    al_transform_coordinates(
        &game.windowToWorldTransform,
        &box[0].x, &box[0].y
    );
    al_transform_coordinates(
        &game.windowToWorldTransform,
        &box[1].x, &box[1].y
    );
    
    game.audio.setCameraPos(box[0], box[1]);
    
    box[0].x -= GAMEPLAY::CAMERA_BOX_MARGIN;
    box[0].y -= GAMEPLAY::CAMERA_BOX_MARGIN;
    box[1].x += GAMEPLAY::CAMERA_BOX_MARGIN;
    box[1].y += GAMEPLAY::CAMERA_BOX_MARGIN;
}


/**
 * @brief Emits an error in the gameplay "info" window.
 *
 * @param s Full error description.
 */
void ErrorManager::emitInGameplay(const string &s) {
    string info_str =
        "\n\n\n"
        "ERROR: " + s + "\n\n"
        "(Saved to \"" + FILE_PATHS_FROM_ROOT::ERROR_LOG + "\".)\n\n";
    printInfo(info_str, 30.0f, 3.0f);
}


/**
 * @brief Logs an error to stdout (i.e. the console).
 *
 * @param s Full error description.
 */
void ErrorManager::logToConsole(const string &s) {
    std::cout << s << std::endl;
}


/**
 * @brief Logs an error to the log file.
 *
 * @param s Full error description.
 */
void ErrorManager::logToFile(const string &s) {
    string prev_error_log;
    string output = "";
    
    //Get the previous contents of the log file, if any.
    ALLEGRO_FILE* file_i =
        al_fopen(FILE_PATHS_FROM_ROOT::ERROR_LOG.c_str(), "r");
    if(file_i) {
        while(!al_feof(file_i)) {
            string line;
            getline(file_i, line);
            prev_error_log += line + "\n";
        }
        prev_error_log.erase(prev_error_log.size() - 1);
        al_fclose(file_i);
    }
    
    //Write this session's header, if necessary.
    if(nrSessionErrors == 0) {
        string header;
        if(!prev_error_log.empty()) {
            header += "\n\n";
        }
        header += "Pikifen version " + getEngineVersionString();
        if(!game.config.general.version.empty()) {
            header +=
                ", " + game.config.general.name + " version " + game.config.general.version;
        }
        header += ":\n";
        output += header;
    }
    
    //Log this error.
    vector<string> lines = split(s, "\n");
    output += "  " + getCurrentTime(false) + ": " + lines[0] + "\n";
    for(size_t l = 1; l < lines.size(); l++) {
        output += "  " + lines[l] + "\n";
    }
    
    //Save it.
    ALLEGRO_FILE* file_o =
        al_fopen(FILE_PATHS_FROM_ROOT::ERROR_LOG.c_str(), "w");
    if(file_o) {
        al_fwrite(file_o, prev_error_log + output);
        al_fclose(file_o);
    }
}


/**
 * @brief Prepares everything for an area load.
 */
void ErrorManager::prepareAreaLoad() {
    nrErrorsOnAreaLoad = nrSessionErrors;
    firstAreaLoadError.clear();
}


/**
 * @brief Reports an error to the user and logs it.
 *
 * @param s String that represents the error.
 * @param d If not null, this will be used to obtain the file name
 * and line that caused the error.
 */
void ErrorManager::report(const string &s, const DataNode* d) {
    string full_error = s;
    if(d) {
        full_error += " (" + d->filePath;
        if(d->lineNr != 0) full_error += " line " + i2s(d->lineNr);
        full_error += ")";
    }
    
    if(firstAreaLoadError.empty()) firstAreaLoadError = full_error;
    
    logToConsole(full_error);
    logToFile(full_error);
    emitInGameplay(full_error);
    
    nrSessionErrors++;
}


/**
 * @brief Reports to the gameplay "info" window if any errors happened during
 * area load.
 * This will override whatever is in the "info" window, which is likely
 * the latest error, but that's okay since this information is more important.
 */
void ErrorManager::reportAreaLoadErrors() {
    if(nrSessionErrors <= nrErrorsOnAreaLoad) return;
    
    size_t nr_errors_found =
        nrSessionErrors - nrErrorsOnAreaLoad;
        
    string info_str =
        "\n\n\n"
        "ERROR: " + firstAreaLoadError + "\n\n";
    if(nr_errors_found > 1) {
        info_str += "(+" + i2s(nr_errors_found - 1) + " more) ";
    }
    info_str +=
        "(Saved to \"" + FILE_PATHS_FROM_ROOT::ERROR_LOG + "\".)\n\n";
        
    printInfo(info_str, 30.0f, 3.0f);
}


/**
 * @brief Returns whether this session has had any error reports.
 *
 * @return Whether it had errors.
 */
bool ErrorManager::sessionHasErrors() {
    return nrSessionErrors > 0;
}


/**
 * @brief Constructs a new fade manager object.
 *
 * @param duration Standard duration of a fade in/out.
 */
FadeManager::FadeManager(float duration) :
    duration(duration) {
}


/**
 * @brief Draws the fade overlay, if there is a fade in progress.
 */
void FadeManager::draw() {
    if(isFading()) {
        unsigned char alpha = (game.fadeMgr.getPercLeft()) * 255;
        al_draw_filled_rectangle(
            0, 0, game.winW, game.winH,
            al_map_rgba(
                0, 0, 0, (game.fadeMgr.isFadeIn() ? alpha : 255 - alpha)
            )
        );
    }
}


/**
 * @brief Returns the percentage of progress left in the current fade.
 *
 * @return The percentage.
 */
float FadeManager::getPercLeft() const {
    float curDuration = durationOverride == 0.0f ? duration : durationOverride;
    if(curDuration == 0.0f) return 0.0f;
    return timeLeft / curDuration;
}


/**
 * @brief Returns whether the current fade is a fade in or fade out.
 *
 * @return Whether it is a fade in.
 */
bool FadeManager::isFadeIn() const {
    return fadeIn;
}


/**
 * @brief Returns whether or not a fade is currently in progress.
 *
 * @return Whether it is in progress.
 */
bool FadeManager::isFading() const {
    float curDuration = durationOverride == 0.0f ? duration : durationOverride;
    return timeLeft > 0 && curDuration != 0.0f;
}


/**
 * @brief Sets the duration of the next fade. After that one, it goes back to
 * the regular duration.
 *
 * @param duration The duration.
 */
void FadeManager::setNextFadeDuration(float duration) {
    durationOverride = duration;
}


/**
 * @brief Sets up the start of a fade.
 *
 * @param is_fade_in If true, this fades in. If false, fades out.
 * @param on_end Code to run when the fade finishes.
 */
void FadeManager::startFade(
    bool is_fade_in, const std::function<void()> &on_end
) {
    float curDuration = durationOverride == 0.0f ? duration : durationOverride;
    timeLeft = curDuration;
    fadeIn = is_fade_in;
    this->onEnd = on_end;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void FadeManager::tick(float delta_t) {
    if(timeLeft == 0) return;
    timeLeft -= delta_t;
    if(timeLeft <= 0) {
        timeLeft = 0;
        if(onEnd) onEnd();
        if(durationOverride > 0.0f) durationOverride = 0.0f;
    }
}


/**
 * @brief Constructs a new getter writer object.
 *
 * @param dn Pointer to the base data node.
 */
GetterWriter::GetterWriter(DataNode* dn) :
    node(dn) {
    
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is an Allegro color.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const ALLEGRO_COLOR &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, c2s(var));
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is a string.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const string &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, var);
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is a string.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const char* var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, var);
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is an integer.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const size_t &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, i2s(var));
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is an integer.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const int &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, i2s(var));
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is an unsigned integer.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const unsigned int &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, i2s(var));
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is an unsigned char.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const unsigned char &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, i2s(var));
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is a boolean.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const bool &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, b2s(var));
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is a float.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const float &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, f2s(var));
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is a double.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const double &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, f2s(var));
    if(out_child_node) *out_child_node = new_node;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param child_name Name of the child node.
 * @param var The var to get. This is a point.
 * @param out_child_node If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &child_name, const Point &var, DataNode** out_child_node
) {
    DataNode* new_node = node->addNew(child_name, p2s(var));
    if(out_child_node) *out_child_node = new_node;
}



/**
 * @brief Hides the OS mouse in the game window.
 */
void MouseCursor::hide() const {
    al_hide_mouse_cursor(game.display);
}


/**
 * @brief Initializes everything.
 */
void MouseCursor::init() {
    hide();
    reset();
    
    saveTimer.onEnd = [this] () {
        saveTimer.start();
        history.push_back(winPos);
        if(history.size() > GAME::CURSOR_TRAIL_SAVE_N_SPOTS) {
            history.erase(history.begin());
        }
    };
    saveTimer.start(GAME::CURSOR_TRAIL_SAVE_INTERVAL);
}


/**
 * @brief Resets the cursor's state.
 */
void MouseCursor::reset() {
    ALLEGRO_MOUSE_STATE mouse_state;
    al_get_mouse_state(&mouse_state);
    game.mouseCursor.winPos.x = al_get_mouse_state_axis(&mouse_state, 0);
    game.mouseCursor.winPos.y = al_get_mouse_state_axis(&mouse_state, 1);
    game.mouseCursor.worldPos = game.mouseCursor.winPos;
    al_transform_coordinates(
        &game.windowToWorldTransform,
        &game.mouseCursor.worldPos.x, &game.mouseCursor.worldPos.y
    );
    history.clear();
}


/**
 * @brief Shows the OS mouse in the game window.
 */
void MouseCursor::show() const {
    al_show_mouse_cursor(game.display);
}


/**
 * @brief Updates the coordinates from an Allegro mouse event.
 *
 * @param ev Event to handle.
 * @param window_to_world_transform Transformation to use to get the
 * window coordinates.
 */
void MouseCursor::updatePos(
    const ALLEGRO_EVENT &ev,
    ALLEGRO_TRANSFORM &window_to_world_transform
) {
    winPos.x = ev.mouse.x;
    winPos.y = ev.mouse.y;
    worldPos = winPos;
    al_transform_coordinates(
        &window_to_world_transform,
        &worldPos.x, &worldPos.y
    );
}


/**
 * @brief Draws the notification.
 */
void Notification::draw() const {
    if(visibility == 0.0f) return;
    
    float scale = ease(EASE_METHOD_OUT, visibility);
    
    ALLEGRO_TRANSFORM tra, old_tra;
    al_identity_transform(&tra);
    al_scale_transform(&tra, scale, scale);
    al_translate_transform(
        &tra,
        pos.x * game.cam.zoom,
        pos.y * game.cam.zoom
    );
    al_scale_transform(
        &tra,
        1.0f / game.cam.zoom,
        1.0f / game.cam.zoom
    );
    al_copy_transform(&old_tra, al_get_current_transform());
    al_compose_transform(&tra, &old_tra);
    al_use_transform(&tra);
    
    int bmp_w = al_get_bitmap_width(game.sysContent.bmpNotification);
    int bmp_h = al_get_bitmap_height(game.sysContent.bmpNotification);
    
    float text_box_x1 = -bmp_w * 0.5 + DRAWING::NOTIFICATION_PADDING;
    float text_box_x2 = bmp_w * 0.5 - DRAWING::NOTIFICATION_PADDING;
    float text_box_y1 = -bmp_h - DRAWING::NOTIFICATION_PADDING;
    float text_box_y2 = DRAWING::NOTIFICATION_PADDING;
    
    drawBitmap(
        game.sysContent.bmpNotification,
        Point(0, -bmp_h * 0.5),
        Point(bmp_w, bmp_h),
        0,
        mapAlpha(DRAWING::NOTIFICATION_ALPHA * visibility)
    );
    
    if(inputSource.type != INPUT_SOURCE_TYPE_NONE) {
        text_box_x1 +=
            DRAWING::NOTIFICATION_CONTROL_SIZE + DRAWING::NOTIFICATION_PADDING;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, inputSource,
            true,
            Point(
                -bmp_w * 0.5 + DRAWING::NOTIFICATION_PADDING +
                DRAWING::NOTIFICATION_CONTROL_SIZE * 0.5,
                -bmp_h * 0.5
            ),
            Point(
                DRAWING::NOTIFICATION_CONTROL_SIZE,
                DRAWING::NOTIFICATION_CONTROL_SIZE
            ),
            visibility * 255
        );
    }
    
    drawText(
        text, game.sysContent.fntStandard,
        Point(
            (text_box_x1 + text_box_x2) * 0.5,
            (text_box_y1 + text_box_y2) * 0.5
        ),
        Point(
            text_box_x2 - text_box_x1,
            text_box_y2 - text_box_y1
        ),
        mapAlpha(DRAWING::NOTIFICATION_ALPHA * visibility),
        ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, TEXT_SETTING_FLAG_CANT_GROW
    );
    
    al_use_transform(&old_tra);
}


/**
 * @brief Returns how "present" the notification is.
 * 0 means hidden, 1 is fully visible. Mid values are transition.
 *
 * @return The visibility.
 */
float Notification::getVisibility() const {
    return visibility;
}


/**
 * @brief Resets the whole thing.
 */
void Notification::reset() {
    enabled = true;
    inputSource.type = INPUT_SOURCE_TYPE_NONE;
    text.clear();
    pos = Point();
    visibility = 0.0f;
}


/**
 * @brief Sets the contents to show.
 *
 * @param input_source Player input source icon to show.
 * @param text Text to show.
 * @param pos Where to show it in the game world.
 */
void Notification::setContents(
    const PlayerInputSource &input_source, const string &text, const Point &pos
) {
    this->inputSource = input_source;
    this->text = text;
    this->pos = pos;
}


/**
 * @brief Sets whether the notification is meant to show or not.
 *
 * @param enabled Whether it's enabled or not.
 */
void Notification::setEnabled(bool enabled) {
    this->enabled = enabled;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Notification::tick(float delta_t) {
    if(enabled) {
        visibility += NOTIFICATION::FADE_SPEED * delta_t;
    } else {
        visibility -= NOTIFICATION::FADE_SPEED * delta_t;
    }
    visibility = std::clamp(visibility, 0.0f, 1.0f);
}


/**
 * @brief Constructs a new performance monitor struct object.
 */
PerformanceMonitor::PerformanceMonitor() :
    curState(PERF_MON_STATE_LOADING),
    paused(false),
    curStateStartTime(0.0),
    curMeasurementStartTime(0.0),
    frameSamples(0) {
    
    reset();
}


/**
 * @brief Enters the given state of the monitoring process.
 *
 * @param state New state.
 */
void PerformanceMonitor::enterState(const PERF_MON_STATE state) {
    if(paused) return;
    
    curState = state;
    curStateStartTime = al_get_time();
    curPage = Page();
    
    if(curState == PERF_MON_STATE_FRAME) {
        frameSamples++;
    }
}


/**
 * @brief Finishes the latest measurement.
 */
void PerformanceMonitor::finishMeasurement() {
    if(paused) return;
    
    //Check if we were measuring something.
    engineAssert(
        curMeasurementStartTime != 0.0,
        curPage.measurements.empty() ?
        "(No measurements)" :
        "Last measurement: " + curPage.measurements.back().first
    );
    
    double dur = al_get_time() - curMeasurementStartTime;
    bool is_new = true;
    
    for(size_t m = 0; m < curPage.measurements.size(); m++) {
        if(curPage.measurements[m].first == curMeasurementName) {
            curPage.measurements[m].second += dur;
            is_new = false;
            break;
        }
    }
    if(is_new) {
        curPage.measurements.push_back(
            std::make_pair(curMeasurementName, dur)
        );
    }
    
    curMeasurementStartTime = 0.0;
}


/**
 * @brief Leaves the current state of the monitoring process.
 */
void PerformanceMonitor::leaveState() {
    if(paused) return;
    
    curPage.duration = al_get_time() - curStateStartTime;
    
    switch(curState) {
    case PERF_MON_STATE_LOADING: {
        loadingPage = curPage;
        break;
    }
    case PERF_MON_STATE_FRAME: {
        if(
            frameFastestPage.duration == 0.0 ||
            curPage.duration < frameFastestPage.duration
        ) {
            frameFastestPage = curPage;
            
        } else if(
            frameSlowestPage.duration == 0.0 ||
            curPage.duration > frameSlowestPage.duration
        ) {
            frameSlowestPage = curPage;
            
        }
        
        if(frameAvgPage.duration == 0.0) {
            frameAvgPage = curPage;
        } else {
            frameAvgPage.duration += curPage.duration;
            for(size_t m = 0; m < curPage.measurements.size(); m++) {
                bool is_new = true;
                for(
                    size_t m2 = 0;
                    m2 < frameAvgPage.measurements.size(); m2++
                ) {
                    if(
                        curPage.measurements[m].first ==
                        frameAvgPage.measurements[m2].first
                    ) {
                        frameAvgPage.measurements[m2].second +=
                            curPage.measurements[m].second;
                        is_new = false;
                        break;
                    }
                }
                if(is_new) {
                    frameAvgPage.measurements.push_back(
                        curPage.measurements[m]
                    );
                }
            }
        }
        break;
        
    }
    }
}


/**
 * @brief Resets all of the performance monitor's information.
 */
void PerformanceMonitor::reset() {
    areaName.clear();
    curState = PERF_MON_STATE_LOADING;
    paused = false;
    curStateStartTime = 0.0;
    curMeasurementStartTime = 0.0;
    curMeasurementName.clear();
    curPage = Page();
    frameSamples = 0;
    loadingPage = Page();
    frameAvgPage = Page();
    frameFastestPage = Page();
    frameSlowestPage = Page();
}


/**
 * @brief Saves a log file with all known stats, if there is anything to save.
 */
void PerformanceMonitor::saveLog() {
    if(loadingPage.measurements.empty()) {
        //Nothing to save.
        return;
    }
    
    //Average out the frames of gameplay.
    frameAvgPage.duration /= (double) frameSamples;
    for(size_t m = 0; m < frameAvgPage.measurements.size(); m++) {
        frameAvgPage.measurements[m].second /= (double) frameSamples;
    }
    
    //Fill out the string.
    string s =
        "\n" +
        getCurrentTime(false) +
        "; Pikifen version " + getEngineVersionString();
    if(!game.config.general.version.empty()) {
        s += ", game version " + game.config.general.version;
    }
    
    s +=
        "\nData from the latest played area, " + areaName + ", with " +
        i2s(frameSamples) + " gameplay frames sampled.\n";
        
    s += "\nLoading times:\n";
    loadingPage.write(s);
    
    s += "\nAverage frame processing times:\n";
    frameAvgPage.write(s);
    
    s += "\nFastest frame processing times:\n";
    frameFastestPage.write(s);
    
    s += "\nSlowest frame processing times:\n";
    frameSlowestPage.write(s);
    
    //Finally, write the string to a file.
    string prev_log;
    ALLEGRO_FILE* file_i =
        al_fopen(FILE_PATHS_FROM_ROOT::PERFORMANCE_LOG.c_str(), "r");
    if(file_i) {
        string line;
        while(!al_feof(file_i)) {
            getline(file_i, line);
            prev_log += line + "\n";
        }
        prev_log.erase(prev_log.size() - 1);
        al_fclose(file_i);
    }
    
    ALLEGRO_FILE* file_o =
        al_fopen(FILE_PATHS_FROM_ROOT::PERFORMANCE_LOG.c_str(), "w");
    if(file_o) {
        al_fwrite(file_o, prev_log + s);
        al_fclose(file_o);
    }
}


/**
 * @brief Sets the name of the area that was monitored.
 *
 * @param name Name of the area.
 */
void PerformanceMonitor::setAreaName(const string &name) {
    areaName = name;
}


/**
 * @brief Sets whether monitoring is currently paused or not.
 *
 * @param paused Pause value.
 */
void PerformanceMonitor::setPaused(bool paused) {
    this->paused = paused;
}


/**
 * @brief Starts measuring a certain point in the loading procedure.
 *
 * @param name Name of the measurement.
 */
void PerformanceMonitor::startMeasurement(const string &name) {
    if(paused) return;
    
    //Check if we were already measuring something.
    engineAssert(
        curMeasurementStartTime == 0.0,
        curPage.measurements.empty() ?
        "(No measurements)" :
        "Last measurement: " + curPage.measurements.back().first
    );
    
    curMeasurementStartTime = al_get_time();
    curMeasurementName = name;
}


/**
 * @brief Writes a page of information to a string.
 *
 * @param s String to write to.
 */
void PerformanceMonitor::Page::write(string &s) {
    //Get the total measured time.
    double total_measured_time = 0.0;
    for(size_t m = 0; m < measurements.size(); m++) {
        total_measured_time += measurements[m].second;
    }
    
    //Write each measurement into the string.
    for(size_t m = 0; m < measurements.size(); m++) {
        writeMeasurement(
            s, measurements[m].first,
            measurements[m].second,
            total_measured_time
        );
    }
    
    //Write the total.
    s +=
        "  TOTAL: " + std::to_string(duration) + "s (" +
        std::to_string(total_measured_time) + "s measured, " +
        std::to_string(duration - total_measured_time) + "s not measured).\n";
}


/**
 * @brief Writes a measurement in a human-friendly format onto a string.
 *
 * @param str The string to write to.
 * @param name The name of this measurement.
 * @param dur How long it lasted for, in seconds.
 * @param total How long the entire procedure lasted for.
 */
void PerformanceMonitor::Page::writeMeasurement(
    string &str, const string &name, double dur, float total
) {
    float perc = dur / total * 100.0;
    str +=
        "  " + name + "\n" +
        "    " + boxString(std::to_string(dur), 8, "s") +
        " (" + f2s(perc) + "%)\n    ";
    for(unsigned char p = 0; p < 100; p++) {
        if(p < perc) {
            str.push_back('#');
        } else {
            str.push_back('_');
        }
    }
    str += "\n";
}


/**
 * @brief Constructs a new reader setter object.
 *
 * @param dn Pointer to the base data node.
 */
ReaderSetter::ReaderSetter(DataNode* dn) :
    node(dn) {
    
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is an Allegro color.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, ALLEGRO_COLOR &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2c(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is a string.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, string &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = n->value;
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is an integer.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, size_t &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2i(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is an integer.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, int &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2i(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is an unsigned integer.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, unsigned int &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2i(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is an unsigned char.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, unsigned char &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2i(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is a boolean.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, bool &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2b(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is a float.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, float &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2f(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is a double.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, double &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2f(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child_name Name of the child node.
 * @param var The var to set. This is a point.
 * @param out_child_node If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child_name, Point &var, DataNode** out_child_node
) {
    DataNode* n = node->getChildByName(child_name);
    if(!n->value.empty()) {
        if(out_child_node) *out_child_node = n;
        var = s2p(n->value);
    } else {
        if(out_child_node) *out_child_node = nullptr;
    }
}



/**
 * @brief Returns a random float between the provided range, inclusive.
 *
 * @param minimum Minimum value that can be generated, inclusive.
 * @param maximum Maximum value that can be generated, inclusive.
 * @return The random number.
 */
float RngManager::f(float minimum, float maximum) {
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    
    return
        (float) linearCongruentialGenerator(&state) /
        ((float) INT32_MAX / (maximum - minimum)) + minimum;
}


/**
 * @brief Returns a random integer between the provided range, inclusive.
 *
 * @param minimum Minimum value that can be generated, inclusive.
 * @param maximum Maximum value that can be generated, inclusive.
 * @return The random number.
 */
int32_t RngManager::i(int32_t minimum, int32_t maximum) {
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    return
        (
            (linearCongruentialGenerator(&state)) %
            (maximum - minimum + 1)
        ) + minimum;
}


/**
 * @brief Initializes the random number generator, using the current time as
 * the seed.
 */
void RngManager::init() {
    init(time(nullptr));
}


/**
 * @brief Initializes the random number generator with the given seed.
 */
void RngManager::init(int32_t initial_seed) {
    state = initial_seed;
}



/**
 * @brief Constructs a new script var reader object.
 *
 * @param vars Map of variables to read from.
 */
ScriptVarReader::ScriptVarReader(map<string, string> &vars) :
    vars(vars) {
    
}


/**
 * @brief Assigns an Allegro color to the value of a given variable,
 * if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, ALLEGRO_COLOR &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2c(v->second);
    return true;
}


/**
 * @brief Assigns a string to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, string &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = v->second;
    return true;
}


/**
 * @brief Assigns a size_t to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, size_t &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/**
 * @brief Assigns an int to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, int &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/**
 * @brief Assigns an unsigned char to the value of a given variable,
 * if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, unsigned char &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/**
 * @brief Assigns a bool to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, bool &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2b(v->second);
    return true;
}


/**
 * @brief Assigns a float to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, float &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2f(v->second);
    return true;
}


/**
 * @brief Assigns a point to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, Point &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2p(v->second);
    return true;
}


/**
 * @brief Loads an audio sample for the manager.
 *
 * @param name Name of the audio sample to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The audio sample.
 */
ALLEGRO_SAMPLE* SampleManager::doLoad(
    const string &name, DataNode* node, bool report_errors
) {
    const auto &it = game.content.sounds.manifests.find(name);
    string path =
        it != game.content.sounds.manifests.end() ?
        it->second.path :
        name;
    return loadSample(path, node, report_errors);
}


/**
 * @brief Unloads an audio sample for the manager.
 *
 * @param asset Audio sample to unload.
 */
void SampleManager::doUnload(ALLEGRO_SAMPLE* asset) {
    al_destroy_sample(asset);
}


/**
 * @brief Clears the list of registered subgroup types.
 */
void SubgroupTypeManager::clear() {
    for(size_t t = 0; t < types.size(); t++) {
        delete types[t];
    }
    types.clear();
}


/**
 * @brief Returns the first registered subgroup type.
 *
 * @return The first type.
 */
SubgroupType* SubgroupTypeManager::getFirstType() const {
    return types.front();
}


/**
 * @brief Returns the subgroup type that comes after the given type.
 *
 * @param sgt Subgroup type to iterate from.
 * @return The next type.
 */
SubgroupType* SubgroupTypeManager::getNextType(
    const SubgroupType* sgt
) const {
    for(size_t t = 0; t < types.size(); t++) {
        if(types[t] == sgt) {
            return getNextInVector(types, t);
        }
    }
    return nullptr;
}


/**
 * @brief Returns the subgroup type that comes before the given type.
 *
 * @param sgt Subgroup type to iterate from.
 * @return The previous type.
 */
SubgroupType* SubgroupTypeManager::getPrevType(
    const SubgroupType* sgt
) const {
    for(size_t t = 0; t < types.size(); t++) {
        if(types[t] == sgt) {
            return getPrevInVector(types, t);
        }
    }
    return nullptr;
}


/**
 * @brief Returns the type of subgroup corresponding to the parameters.
 *
 * @param category The category of subgroup type. Pikmin, leader,
 * bomb-rock, etc.
 * @param specific_type Specific type of mob, if you want to specify further.
 * @return The type, or nullptr if not found.
 */
SubgroupType* SubgroupTypeManager::getType(
    const SUBGROUP_TYPE_CATEGORY category,
    const MobType* specific_type
) const {
    for(size_t t = 0; t < types.size(); t++) {
        SubgroupType* t_ptr = types[t];
        if(
            t_ptr->category == category &&
            t_ptr->specificType == specific_type
        ) {
            return t_ptr;
        }
    }
    return nullptr;
}


/**
 * @brief Registers a new type of subgroup.
 *
 * @param category The category of subgroup type. Pikmin, leader,
 * bomb-rock, etc.
 * @param specific_type Specific type of mob, if you want to specify further.
 * @param icon If not nullptr, use this icon to represent this subgroup.
 */
void SubgroupTypeManager::registerType(
    const SUBGROUP_TYPE_CATEGORY category,
    MobType* specific_type,
    ALLEGRO_BITMAP* icon
) {
    SubgroupType* new_sg_type = new SubgroupType();
    
    new_sg_type->category = category;
    new_sg_type->specificType = specific_type;
    new_sg_type->icon = icon;
    
    types.push_back(new_sg_type);
}


/**
 * @brief Constructs a new whistle struct object.
 */
Whistle::Whistle() :
    radius(0.0f),
    fadeRadius(0.0f),
    fadeTimer(WHISTLE::FADE_TIME),
    nextDotTimer(WHISTLE::DOT_INTERVAL),
    nextRingTimer(WHISTLE::RINGS_INTERVAL),
    ringPrevColor(0),
    whistling(false) {
    
    dotRadius[0] = -1;
    dotRadius[1] = -1;
    dotRadius[2] = -1;
    dotRadius[3] = -1;
    dotRadius[4] = -1;
    dotRadius[5] = -1;
    
    nextDotTimer.onEnd = [this] () {
        nextDotTimer.start();
        unsigned char dot = 255;
        for(unsigned char d = 0; d < 6; d++) { //Find WHAT dot to add.
            if(dotRadius[d] == -1) {
                dot = d;
                break;
            }
        }
        
        if(dot != 255) dotRadius[dot] = 0;
    };
    
    nextRingTimer.onEnd = [this] () {
        nextRingTimer.start();
        rings.push_back(0);
        ringColors.push_back(ringPrevColor);
        ringPrevColor =
            sumAndWrap(ringPrevColor, 1, WHISTLE::N_RING_COLORS);
    };
    
}


/**
 * @brief Stuff to do when a leader starts whistling.
 */
void Whistle::startWhistling() {
    for(unsigned char d = 0; d < 6; d++) {
        dotRadius[d] = -1;
    }
    fadeTimer.start();
    fadeRadius = 0;
    whistling = true;
}


/**
 * @brief Stuff to do when a leader stops whistling.
 */
void Whistle::stopWhistling() {
    whistling = false;
    fadeTimer.start();
    fadeRadius = radius;
    radius = 0;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 * @param center What its center is on this frame.
 * @param whistle_range How far the whistle can reach from the cursor center.
 * @param leader_to_cursor_dist Distance between the leader and the cursor.
 */
void Whistle::tick(
    float delta_t, const Point &center,
    float whistle_range, float leader_to_cursor_dist
) {
    this->center = center;
    
    fadeTimer.tick(delta_t);
    
    if(whistling) {
        //Create rings.
        nextRingTimer.tick(delta_t);
        nextDotTimer.tick(delta_t);
        
        for(unsigned char d = 0; d < 6; d++) {
            if(dotRadius[d] == -1) continue;
            
            dotRadius[d] += game.config.rules.whistleGrowthSpeed * delta_t;
            if(radius > 0 && dotRadius[d] > whistle_range) {
                dotRadius[d] = whistle_range;
                
            } else if(fadeRadius > 0 && dotRadius[d] > fadeRadius) {
                dotRadius[d] = fadeRadius;
            }
        }
    }
    
    for(size_t r = 0; r < rings.size(); ) {
        //Erase rings that go beyond the cursor.
        rings[r] += WHISTLE::RING_SPEED * delta_t;
        if(leader_to_cursor_dist < rings[r]) {
            rings.erase(rings.begin() + r);
            ringColors.erase(ringColors.begin() + r);
        } else {
            r++;
        }
    }
}
