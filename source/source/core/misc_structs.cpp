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
 * @brief Loads an audio stream for the manager.
 *
 * @param name Name of the audio stream to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param reportErrors Only issues errors if this is true.
 * @return The audio stream.
 */
ALLEGRO_AUDIO_STREAM* AudioStreamManager::doLoad(
    const string &name, DataNode* node, bool reportErrors
) {
    const auto &it = game.content.songTracks.manifests.find(name);
    string path =
        it != game.content.songTracks.manifests.end() ?
        it->second.path :
        name;
    ALLEGRO_AUDIO_STREAM* stream = loadAudioStream(path, node, reportErrors);
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
 * @param reportErrors Only issues errors if this is true.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* BitmapManager::doLoad(
    const string &name, DataNode* node, bool reportErrors
) {
    const auto &it = game.content.bitmaps.manifests.find(name);
    string path =
        it != game.content.bitmaps.manifests.end() ?
        it->second.path :
        name;
    return loadBmp(path, node, reportErrors);
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
 * @param newPos Coordinates to place the camera at.
 */
void Camera::setPos(const Point &newPos) {
    pos = newPos;
    targetPos = newPos;
}


/**
 * @brief Instantly places the camera at the specified zoom.
 *
 * @param newZoom Zoom to set to.
 */
void Camera::setZoom(float newZoom) {
    zoom = newZoom;
    targetZoom = newZoom;
}


/**
 * @brief Ticks camera movement by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Camera::tick(float deltaT) {
    pos.x +=
        (targetPos.x - pos.x) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * deltaT);
    pos.y +=
        (targetPos.y - pos.y) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * deltaT);
    zoom +=
        (targetZoom - zoom) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * deltaT);
}


/**
 * @brief Emits an error in the gameplay "info" window.
 *
 * @param s Full error description.
 */
void ErrorManager::emitInGameplay(const string &s) {
    string infoStr =
        "\n\n\n"
        "ERROR: " + s + "\n\n"
        "(Saved to \"" + FILE_PATHS_FROM_ROOT::ERROR_LOG + "\".)\n\n";
    printInfo(infoStr, 30.0f, 3.0f);
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
    string prevErrorLog;
    string output = "";
    
    //Get the previous contents of the log file, if any.
    ALLEGRO_FILE* fileI =
        al_fopen(FILE_PATHS_FROM_ROOT::ERROR_LOG.c_str(), "r");
    if(fileI) {
        while(!al_feof(fileI)) {
            string line;
            getline(fileI, line);
            prevErrorLog += line + "\n";
        }
        prevErrorLog.erase(prevErrorLog.size() - 1);
        al_fclose(fileI);
    }
    
    //Write this session's header, if necessary.
    if(nrSessionErrors == 0) {
        string header;
        if(!prevErrorLog.empty()) {
            header += "\n\n";
        }
        header += "Pikifen version " + getEngineVersionString();
        if(!game.config.general.version.empty()) {
            header +=
                ", " + game.config.general.name +
                " version " + game.config.general.version;
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
    ALLEGRO_FILE* fileO =
        al_fopen(FILE_PATHS_FROM_ROOT::ERROR_LOG.c_str(), "w");
    if(fileO) {
        al_fwrite(fileO, prevErrorLog + output);
        al_fclose(fileO);
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
    string fullError = s;
    if(d) {
        fullError += " (" + d->filePath;
        if(d->lineNr != 0) fullError += " line " + i2s(d->lineNr);
        fullError += ")";
    }
    
    if(firstAreaLoadError.empty()) firstAreaLoadError = fullError;
    
    logToConsole(fullError);
    logToFile(fullError);
    emitInGameplay(fullError);
    
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
    
    size_t nrErrorsFound =
        nrSessionErrors - nrErrorsOnAreaLoad;
        
    string infoStr =
        "\n\n\n"
        "ERROR: " + firstAreaLoadError + "\n\n";
    if(nrErrorsFound > 1) {
        infoStr += "(+" + i2s(nrErrorsFound - 1) + " more) ";
    }
    infoStr +=
        "(Saved to \"" + FILE_PATHS_FROM_ROOT::ERROR_LOG + "\".)\n\n";
        
    printInfo(infoStr, 30.0f, 3.0f);
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
 * @param isFadeIn If true, this fades in. If false, fades out.
 * @param onEnd Code to run when the fade finishes.
 */
void FadeManager::startFade(
    bool isFadeIn, const std::function<void()> &onEnd
) {
    float curDuration = durationOverride == 0.0f ? duration : durationOverride;
    timeLeft = curDuration;
    fadeIn = isFadeIn;
    this->onEnd = onEnd;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void FadeManager::tick(float deltaT) {
    if(timeLeft == 0) return;
    timeLeft -= deltaT;
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
 * @param childName Name of the child node.
 * @param var The var to get. This is an Allegro color.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const ALLEGRO_COLOR &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, c2s(var));
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is a string.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const string &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, var);
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is a string.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const char* var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, var);
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is an integer.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const size_t &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, i2s(var));
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is an integer.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const int &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, i2s(var));
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is an unsigned integer.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const unsigned int &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, i2s(var));
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is an unsigned char.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const unsigned char &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, i2s(var));
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is a boolean.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const bool &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, b2s(var));
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is a float.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const float &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, f2s(var));
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is a double.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const double &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, f2s(var));
    if(outChildNode) *outChildNode = newNode;
}


/**
 * @brief Gets a variable's value, and writes it to a child node's value.
 *
 * @param childName Name of the child node.
 * @param var The var to get. This is a point.
 * @param outChildNode If not nullptr, the new node is returned here.
 */
void GetterWriter::write(
    const string &childName, const Point &var, DataNode** outChildNode
) {
    DataNode* newNode = node->addNew(childName, p2s(var));
    if(outChildNode) *outChildNode = newNode;
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
    ALLEGRO_MOUSE_STATE mouseState;
    al_get_mouse_state(&mouseState);
    winPos.x = al_get_mouse_state_axis(&mouseState, 0);
    winPos.y = al_get_mouse_state_axis(&mouseState, 1);
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
 */
void MouseCursor::updatePos(const ALLEGRO_EVENT &ev) {
    winPos.x = ev.mouse.x;
    winPos.y = ev.mouse.y;
}


/**
 * @brief Draws the notification.
 *
 * @param view Viewport to draw to.
 */
void Notification::draw(const Viewport &view) const {
    if(visibility == 0.0f) return;
    
    float scale = ease(EASE_METHOD_OUT, visibility);
    
    ALLEGRO_TRANSFORM tra, oldTra;
    al_identity_transform(&tra);
    al_scale_transform(&tra, scale, scale);
    al_translate_transform(
        &tra, pos.x * view.cam.zoom, pos.y * view.cam.zoom
    );
    al_scale_transform(
        &tra, 1.0f / view.cam.zoom, 1.0f / view.cam.zoom
    );
    al_copy_transform(&oldTra, al_get_current_transform());
    al_compose_transform(&tra, &oldTra);
    al_use_transform(&tra);
    
    int bmpW = al_get_bitmap_width(game.sysContent.bmpNotification);
    int bmpH = al_get_bitmap_height(game.sysContent.bmpNotification);
    
    float textBoxX1 = -bmpW * 0.5 + DRAWING::NOTIFICATION_PADDING;
    float textBoxX2 = bmpW * 0.5 - DRAWING::NOTIFICATION_PADDING;
    float textBoxY1 = -bmpH - DRAWING::NOTIFICATION_PADDING;
    float textBoxY2 = DRAWING::NOTIFICATION_PADDING;
    
    drawBitmap(
        game.sysContent.bmpNotification,
        Point(0, -bmpH * 0.5),
        Point(bmpW, bmpH),
        0,
        mapAlpha(DRAWING::NOTIFICATION_ALPHA * visibility)
    );
    
    if(inputSource.type != INPUT_SOURCE_TYPE_NONE) {
        textBoxX1 +=
            DRAWING::NOTIFICATION_INPUT_SIZE + DRAWING::NOTIFICATION_PADDING;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, inputSource,
            true,
            Point(
                -bmpW * 0.5 + DRAWING::NOTIFICATION_PADDING +
                DRAWING::NOTIFICATION_INPUT_SIZE * 0.5,
                -bmpH * 0.5
            ),
            Point(
                DRAWING::NOTIFICATION_INPUT_SIZE,
                DRAWING::NOTIFICATION_INPUT_SIZE
            ),
            visibility * 255
        );
    }
    
    drawText(
        text, game.sysContent.fntStandard,
        Point(
            (textBoxX1 + textBoxX2) * 0.5,
            (textBoxY1 + textBoxY2) * 0.5
        ),
        Point(
            textBoxX2 - textBoxX1,
            textBoxY2 - textBoxY1
        ),
        mapAlpha(DRAWING::NOTIFICATION_ALPHA * visibility),
        ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, TEXT_SETTING_FLAG_CANT_GROW
    );
    
    al_use_transform(&oldTra);
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
 * @param inputSource Player input source icon to show.
 * @param text Text to show.
 * @param pos Where to show it in the game world.
 */
void Notification::setContents(
    const PlayerInputSource &inputSource, const string &text, const Point &pos
) {
    this->inputSource = inputSource;
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
 * @param deltaT How long the frame's tick is, in seconds.
 */
void Notification::tick(float deltaT) {
    if(enabled) {
        visibility += NOTIFICATION::FADE_SPEED * deltaT;
    } else {
        visibility -= NOTIFICATION::FADE_SPEED * deltaT;
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
    bool isNew = true;
    
    for(size_t m = 0; m < curPage.measurements.size(); m++) {
        if(curPage.measurements[m].first == curMeasurementName) {
            curPage.measurements[m].second += dur;
            isNew = false;
            break;
        }
    }
    if(isNew) {
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
                bool isNew = true;
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
                        isNew = false;
                        break;
                    }
                }
                if(isNew) {
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
    string prevLog;
    ALLEGRO_FILE* fileI =
        al_fopen(FILE_PATHS_FROM_ROOT::PERFORMANCE_LOG.c_str(), "r");
    if(fileI) {
        string line;
        while(!al_feof(fileI)) {
            getline(fileI, line);
            prevLog += line + "\n";
        }
        prevLog.erase(prevLog.size() - 1);
        al_fclose(fileI);
    }
    
    ALLEGRO_FILE* fileO =
        al_fopen(FILE_PATHS_FROM_ROOT::PERFORMANCE_LOG.c_str(), "w");
    if(fileO) {
        al_fwrite(fileO, prevLog + s);
        al_fclose(fileO);
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
    double totalMeasuredTime = 0.0;
    for(size_t m = 0; m < measurements.size(); m++) {
        totalMeasuredTime += measurements[m].second;
    }
    
    //Write each measurement into the string.
    for(size_t m = 0; m < measurements.size(); m++) {
        writeMeasurement(
            s, measurements[m].first,
            measurements[m].second,
            totalMeasuredTime
        );
    }
    
    //Write the total.
    s +=
        "  TOTAL: " + std::to_string(duration) + "s (" +
        std::to_string(totalMeasuredTime) + "s measured, " +
        std::to_string(duration - totalMeasuredTime) + "s not measured).\n";
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
 * @param childName Name of the child node.
 * @param var The var to set. This is an Allegro color.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, ALLEGRO_COLOR &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2c(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is a string.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, string &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = n->value;
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is an integer.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, size_t &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2i(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is an integer.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, int &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2i(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is an unsigned integer.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, unsigned int &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2i(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is an unsigned char.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, unsigned char &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2i(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is a boolean.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, bool &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2b(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is a float.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, float &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2f(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is a double.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, double &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2f(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param childName Name of the child node.
 * @param var The var to set. This is a point.
 * @param outChildNode If not nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &childName, Point &var, DataNode** outChildNode
) {
    DataNode* n = node->getChildByName(childName);
    if(!n->value.empty()) {
        if(outChildNode) *outChildNode = n;
        var = s2p(n->value);
    } else {
        if(outChildNode) *outChildNode = nullptr;
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
        (float) generateGoodInt() /
        ((float) INT32_MAX / (maximum - minimum)) + minimum;
}


/**
 * @brief Calls the PRNG in order to get a decent random number.
 *
 * @return The generated number.
 */
int32_t RngManager::generateGoodInt() {
    //Let's generate two, get their top 16 bits, and merge them.
    //This is because relying on the least significant bits of an LCG is
    //a bad idea (for instance, the numbers always come out alternating
    //between odd and even).
    int32_t n1 = linearCongruentialGenerator(&state);
    int32_t n2 = linearCongruentialGenerator(&state);
    
    int16_t msb1 = (n1 >> 16) & 0xFFFF;
    int16_t msb2 = (n2 >> 16) & 0xFFFF;
    
    return ((int32_t) msb1 << 16) | msb2;
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
            (generateGoodInt()) %
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
 * 
 * @param initialSeed The seed.
 */
void RngManager::init(int32_t initialSeed) {
    state = initialSeed;
}



/**
 * @brief Loads an audio sample for the manager.
 *
 * @param name Name of the audio sample to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param reportErrors Only issues errors if this is true.
 * @return The audio sample.
 */
ALLEGRO_SAMPLE* SampleManager::doLoad(
    const string &name, DataNode* node, bool reportErrors
) {
    const auto &it = game.content.sounds.manifests.find(name);
    string path =
        it != game.content.sounds.manifests.end() ?
        it->second.path :
        name;
    return loadSample(path, node, reportErrors);
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
 * @param specificType Specific type of mob, if you want to specify further.
 * @return The type, or nullptr if not found.
 */
SubgroupType* SubgroupTypeManager::getType(
    const SUBGROUP_TYPE_CATEGORY category,
    const MobType* specificType
) const {
    for(size_t t = 0; t < types.size(); t++) {
        SubgroupType* tPtr = types[t];
        if(
            tPtr->category == category &&
            tPtr->specificType == specificType
        ) {
            return tPtr;
        }
    }
    return nullptr;
}


/**
 * @brief Registers a new type of subgroup.
 *
 * @param category The category of subgroup type. Pikmin, leader,
 * bomb-rock, etc.
 * @param specificType Specific type of mob, if you want to specify further.
 * @param icon If not nullptr, use this icon to represent this subgroup.
 */
void SubgroupTypeManager::registerType(
    const SUBGROUP_TYPE_CATEGORY category,
    MobType* specificType,
    ALLEGRO_BITMAP* icon
) {
    SubgroupType* newSgType = new SubgroupType();
    
    newSgType->category = category;
    newSgType->specificType = specificType;
    newSgType->icon = icon;
    
    types.push_back(newSgType);
}


/**
 * @brief Loads the system content names from a file.
 *
 * @param file File to load from.
 */
void SystemContentNames::load(DataNode* file) {
    ReaderSetter graRS(file->getChildByName("graphics"));
    
    graRS.set("bright_circle", bmpBrightCircle);
    graRS.set("bright_ring", bmpBrightRing);
    graRS.set("bubble_box", bmpBubbleBox);
    graRS.set("button_box", bmpButtonBox);
    graRS.set("checkbox_check", bmpCheckboxCheck);
    graRS.set("checkbox_no_check", bmpCheckboxNoCheck);
    graRS.set("cursor", bmpCursor);
    graRS.set("discord_icon", bmpDiscordIcon);
    graRS.set("editor_icons", bmpEditorIcons);
    graRS.set("enemy_spirit", bmpEnemySpirit);
    graRS.set("focus_box", bmpFocusBox);
    graRS.set("frame_box", bmpFrameBox);
    graRS.set("github_icon", bmpGithubIcon);
    graRS.set("hard_bubble", bmpHardBubble);
    graRS.set("icon", bmpIcon);
    graRS.set("idle_glow", bmpIdleGlow);
    graRS.set("key_box", bmpKeyBox);
    graRS.set("leader_silhouette_side", bmpLeaderSilhouetteSide);
    graRS.set("leader_silhouette_top", bmpLeaderSilhouetteTop);
    graRS.set("medal_bronze", bmpMedalBronze);
    graRS.set("medal_gold", bmpMedalGold);
    graRS.set("medal_none", bmpMedalNone);
    graRS.set("medal_platinum", bmpMedalPlatinum);
    graRS.set("medal_silver", bmpMedalSilver);
    graRS.set("menu_icons", bmpMenuIcons);
    graRS.set("mission_clear", bmpMissionClear);
    graRS.set("mission_fail", bmpMissionFail);
    graRS.set("more", bmpMore);
    graRS.set("mouse_cursor", bmpMouseCursor);
    graRS.set("notification", bmpNotification);
    graRS.set("pikmin_spirit", bmpPikminSpirit);
    graRS.set("player_input_icons", bmpPlayerInputIcons);
    graRS.set("random", bmpRandom);
    graRS.set("rock", bmpRock);
    graRS.set("shadow", bmpShadow);
    graRS.set("shadow_square", bmpShadowSquare);
    graRS.set("smack", bmpSmack);
    graRS.set("smoke", bmpSmoke);
    graRS.set("sparkle", bmpSparkle);
    graRS.set("spotlight", bmpSpotlight);
    graRS.set("swarm_arrow", bmpSwarmArrow);
    graRS.set("throw_invalid", bmpThrowInvalid);
    graRS.set("throw_preview", bmpThrowPreview);
    graRS.set("throw_preview_dashed", bmpThrowPreviewDashed);
    graRS.set("title_screen_bg", bmpTitleScreenBg);
    graRS.set("wave_ring", bmpWaveRing);
    
    ReaderSetter fntRS(file->getChildByName("fonts"));
    
    fntRS.set("area_name", fntAreaName);
    fntRS.set("counter", fntCounter);
    fntRS.set("cursor_counter", fntCursorCounter);
    fntRS.set("editor_header", fntEditorHeader);
    fntRS.set("editor_monospace", fntEditorMonospace);
    fntRS.set("editor_standard", fntEditorStandard);
    fntRS.set("slim", fntSlim);
    fntRS.set("standard", fntStandard);
    fntRS.set("value", fntValue);
    
    ReaderSetter sndRS(file->getChildByName("sounds"));
    
    sndRS.set("attack", sndAttack);
    sndRS.set("camera", sndCamera);
    sndRS.set("menu_activate", sndMenuActivate);
    sndRS.set("menu_back", sndMenuBack);
    sndRS.set("menu_select", sndMenuSelect);
    sndRS.set("switch_pikmin", sndSwitchPikmin);
    
    ReaderSetter sngRS(file->getChildByName("songs"));
    
    sngRS.set("boss", sngBoss);
    sngRS.set("boss_victory", sngBossVictory);
    sngRS.set("editors", sngEditors);
    sngRS.set("menus", sngMenus);
    
    ReaderSetter aniRS(file->getChildByName("animations"));
    
    aniRS.set("sparks", anmSparks);
    
    ReaderSetter parRS(file->getChildByName("particle_generators"));
    
    parRS.set("converter_insertion", parConverterInsertion);
    parRS.set("ding", parDing);
    parRS.set("enemy_defeat", parEnemyDefeat);
    parRS.set("leader_heal", parLeaderHeal);
    parRS.set("leader_land", parLeaderLand);
    parRS.set("onion_generating_inside", parOnionGenInside);
    parRS.set("onion_insertion", parOnionInsertion);
    parRS.set("pikmin_pluck_dirt", parPikminPluckDirt);
    parRS.set("pikmin_seed", parPikminSeed);
    parRS.set("pikmin_seed_landed", parPikminSeedLanded);
    parRS.set("smack", parSmack);
    parRS.set("spray", parSpray);
    parRS.set("sprout_evolution", parSproutEvolution);
    parRS.set("sprout_regression", parSproutRegression);
    parRS.set("throw_trail", parThrowTrail);
    parRS.set("treasure", parTreasure);
    parRS.set("wave_ring", parWaveRing);
}


/**
 * @brief Returns the bottom-right corner's coordinates, in window coordinates.
 *
 * @return The coordinates.
 */
Point Viewport::getBottomRight() {
    return center + size / 2.0f;
}


/**
 * @brief Returns the top-left corner's coordinates, in window coordinates.
 *
 * @return The coordinates.
 */
Point Viewport::getTopLeft() {
    return center - size / 2.0f;
}


/**
 * @brief Updates the viewport's visibility box,
 * based on the windowToWorldTransform transformation.
 */
void Viewport::updateBox() {
    box[0] = center - size / 2.0f;
    box[1] = center + size / 2.0f;
    al_transform_coordinates(
        &windowToWorldTransform,
        &box[0].x, &box[0].y
    );
    al_transform_coordinates(
        &windowToWorldTransform,
        &box[1].x, &box[1].y
    );
    
    box[0].x -= boxMargin.x;
    box[0].y -= boxMargin.y;
    box[1].x += boxMargin.x;
    box[1].y += boxMargin.y;
}


/**
 * @brief Updates the mouse cursor position, given the game window
 * cursor coordinates.
 *
 * @param windowPos Window coordinates.
 */
void Viewport::updateCursor(const Point &windowPos) {
    cursorWorldPos = windowPos;
    al_transform_coordinates(
        &windowToWorldTransform,
        &cursorWorldPos.x, &cursorWorldPos.y
    );
}


/**
 * @brief Updates the transformations with the current camera coordinates,
 * zoom, etc.
 */
void Viewport::updateTransformations() {
    //World coordinates to window coordinates.
    worldToWindowTransform = game.identityTransform;
    al_translate_transform(
        &worldToWindowTransform,
        -cam.pos.x + center.x / cam.zoom,
        -cam.pos.y + center.y / cam.zoom
    );
    al_scale_transform(
        &worldToWindowTransform, cam.zoom, cam.zoom
    );
    
    //Window coordinates to world coordinates.
    windowToWorldTransform = worldToWindowTransform;
    al_invert_transform(&windowToWorldTransform);
}


/**
 * @brief Constructs a new whistle struct object.
 */
Whistle::Whistle() {
    dotRadius[0] = -1;
    dotRadius[1] = -1;
    dotRadius[2] = -1;
    dotRadius[3] = -1;
    dotRadius[4] = -1;
    dotRadius[5] = -1;
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
 * @param deltaT How long the frame's tick is, in seconds.
 * @param center What its center is on this frame.
 * @param whistleRange How far the whistle can reach from the cursor center.
 * @param leaderToCursorDist Distance between the leader and the cursor.
 */
void Whistle::tick(
    float deltaT, const Point &center,
    float whistleRange, float leaderToCursorDist
) {
    this->center = center;
    
    fadeTimer.tick(deltaT);
    
    if(whistling) {
        //Create rings.
        nextRingTimer.tick(deltaT);
        nextDotTimer.tick(deltaT);
        
        for(unsigned char d = 0; d < 6; d++) {
            if(dotRadius[d] == -1) continue;
            
            dotRadius[d] += game.config.rules.whistleGrowthSpeed * deltaT;
            if(radius > 0 && dotRadius[d] > whistleRange) {
                dotRadius[d] = whistleRange;
                
            } else if(fadeRadius > 0 && dotRadius[d] > fadeRadius) {
                dotRadius[d] = fadeRadius;
            }
        }
    }
    
    for(size_t r = 0; r < rings.size(); ) {
        //Erase rings that go beyond the cursor.
        rings[r] += WHISTLE::RING_SPEED * deltaT;
        if(leaderToCursorDist < rings[r]) {
            rings.erase(rings.begin() + r);
            ringColors.erase(ringColors.begin() + r);
        } else {
            r++;
        }
    }
}
