/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally used functions.
 */

#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "../core/game.h"
#include "../content/area/area.h"
#include "../content/area/sector.h"
#include "../content/mob/leader.h"
#include "../content/mob/onion.h"
#include "../content/mob/pikmin.h"
#include "../content/other/gui.h"
#include "../content/other/mob_script.h"
#include "../game_state/editor.h"
#include "../lib/data_file/data_file.h"
#include "controls_mediator.h"


//A custom-made assertion.
#define engineAssert(expr, message) \
    if(!(expr)) { \
        string info = "\"" #expr "\", in "; \
        info += __FUNCTION__; \
        info += " ("; \
        info += __FILE__; \
        info += ":"; \
        info += std::to_string((long long) (__LINE__)); \
        info += "). Extra info: "; \
        info += (message); \
        crash("Assert", info, 1); \
    }

//Returns the task range for whether the Pikmin is idling or being C-sticked.
#define taskRange(p) \
    (((p)->followingGroup == curLeaderPtr && swarmMagnitude) ? \
     game.config.pikmin.swarmTaskRange : game.config.pikmin.idleTaskRange)


/**
 * @brief Function that checks if an edge should use a given edge offset effect.
 *
 * The first parameter is the edge to check.
 * The second parameter is where the affected sector gets returned to.
 * The third parameter is where the unaffected sector gets returned to.
 * Returns whether it should receive the effect.
 */
typedef bool (*OffsetEffectChecker)(Edge*, Sector**, Sector**);

/**
 * @brief Function that returns an edge's edge offset effect color.
 *
 * The first parameter is the edge to check.
 * Returns the color.
 */
typedef ALLEGRO_COLOR (*OffsetEffectColorGetter)(Edge*);

/**
 * @brief Function that returns an edge's edge offset effect length.
 *
 * The first parameter is the edge to check.
 * Returns the length.
 */
typedef float (*OffsetEffectLengthGetter)(Edge*);



bool areaWallsBetween(
    const Point &p1, const Point &p2,
    float ignoreWallsBelowZ = -FLT_MAX, bool* outImpassableWalls = nullptr
);
void clearAreaTextures();
void crash(const string &reason, const string &info, int exitStatus);
bool doesEdgeHaveLedgeSmoothing(
    Edge* ePtr, Sector** outAffectedSector, Sector** outUnaffectedSector
);
bool doesEdgeHaveLiquidLimit(
    Edge* ePtr, Sector** outAffectedSector, Sector** outUnaffectedSector
);
bool doesEdgeHaveWallShadow(
    Edge* ePtr, Sector** outAffectedSector, Sector** outUnaffectedSector
);
void drawEdgeOffsetOnBuffer(
    const vector<EdgeOffsetCache> &caches, size_t eIdx
);
Mob* getClosestMobToCursor(bool mustHaveHealth = false);
void getEdgeOffsetEdgeInfo(
    Edge* ePtr, Vertex* endVertex, unsigned char endIdx,
    float edgeProcessAngle,
    OffsetEffectChecker checker,
    OffsetEffectLengthGetter lengthGetter,
    OffsetEffectColorGetter colorGetter,
    float* outAngle, float* outLength, ALLEGRO_COLOR* outColor,
    float* outElbowAngle, float* outElbowLength
);
void getEdgeOffsetIntersection(
    const Edge* e1, const Edge* e2, const Vertex* commonVertex,
    float baseShadowAngle1, float baseShadowAngle2,
    float shadowLength,
    float* outAngle, float* outLength
);
ALLEGRO_COLOR getLedgeSmoothingColor(Edge* ePtr);
ALLEGRO_COLOR getLiquidLimitColor(Edge* ePtr);
float getLedgeSmoothingLength(Edge* ePtr);
float getLiquidLimitLength(Edge* ePtr);
string getMissionRecordEntryName(Area* areaPtr);
void getNextEdge(
    Vertex* vPtr, float pivotAngle, bool clockwise,
    const Edge* ignore, Edge** outEdge, float* outAngle, float* outDiff
);
Mob* getNextMobNearCursor(
    Mob* pivot, bool mustHaveHealth = false
);
void getNextOffsetEffectEdge(
    Vertex* vPtr, float pivotAngle, bool clockwise,
    const Edge* ignore, OffsetEffectChecker edgeChecker,
    Edge** outEdge, float* outAngle, float* outDiff,
    float* outBaseShadowAngle,
    bool* outShadowCw
);
string getSubtitleOrMissionGoal(
    const string &subtitle, const AREA_TYPE areaType,
    const MISSION_GOAL goal
);
unsigned char getThrowPreviewVertexes(
    ALLEGRO_VERTEX* vertexes,
    float start, float end,
    const Point &leaderPos, const Point &cursorPos,
    const ALLEGRO_COLOR &color,
    float uOffset, float uScale,
    bool varyThickness
);
map<string, string> getVarMap(const string &varsString);
string getEngineVersionString();
ALLEGRO_COLOR getWallShadowColor(Edge* ePtr);
float getWallShadowLength(Edge* ePtr);
vector<std::pair<int, string> > getWeatherTable(DataNode* node);
void guiAddBackInputIcon(
    GuiManager* gui, const string &itemName = "back_input"
);
bool monoCombo(
    const string &label, int* currentItem, const vector<string> &items,
    int popupMaxHeightInItems = -1
);
bool monoCombo(
    const string &label, string* currentItem, const vector<string> &items,
    int popupMaxHeightInItems = -1
);
bool monoCombo(
    const string &label, string* currentItem,
    const vector<string> &itemInternalValues,
    const vector<string> &itemDisplayNames,
    int popupMaxHeightInItems = -1
);
bool monoButton(
    const char* label, const ImVec2 &size = ImVec2(0, 0)
);
bool monoInputText(
    const char* label, string* str, ImGuiInputTextFlags flags = 0,
    ImGuiInputTextCallback callback = nullptr, void* userData = nullptr
);
bool monoInputTextWithHint(
    const char* label, const char* hint, string* str,
    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
    void* userData = nullptr
);
bool monoListBox(
    const string &label, int* currentItem, const vector<string> &items,
    int heightInItems = -1
);
bool monoSelectable(
    const char* label, bool selected = false, ImGuiSelectableFlags flags = 0,
    const ImVec2 &size = ImVec2(0, 0)
);
bool monoSelectable(
    const char* label, bool* pSelected, ImGuiSelectableFlags flags = 0,
    const ImVec2 &size = ImVec2(0, 0)
);
bool openManual(const string &page);
void printInfo(
    const string &text,
    float totalDuration = 5.0f,
    float fadeDuration = 3.0f
);
void reportFatalError(const string &s, const DataNode* dn = nullptr);
void saveMakerTools();
void saveOptions();
void saveScreenshot();
void saveStatistics();
void setStringTokenWidths(
    vector<StringToken> &tokens,
    const ALLEGRO_FONT* textFont, const ALLEGRO_FONT* controlFont,
    float maxControlBitmapHeight = 0, bool controlCondensed = false
);
void signalHandler(int signum);
void spewPikminSeed(
    const Point pos, float z, PikminType* pikType,
    float angle, float horizontalSpeed, float verticalSpeed
);
vector<vector<StringToken> > splitLongStringWithTokens(
    const vector<StringToken> &tokens, int maxWidth
);
ParticleGenerator standardParticleGenSetup(
    const string &internalName, Mob* targetMob
);
void startGameplayMessage(const string &text, ALLEGRO_BITMAP* speakerBmp);
vector<StringToken> tokenizeString(const string &s);
string unescapeString(const string &s);
void updateOffsetEffectBuffer(
    const Point &camTL, const Point &camBR,
    const vector<EdgeOffsetCache> &caches, ALLEGRO_BITMAP* buffer,
    bool clearFirst
);
void updateOffsetEffectCaches (
    vector<EdgeOffsetCache> &caches,
    const unordered_set<Vertex*> &vertexesToUpdate,
    OffsetEffectChecker checker,
    OffsetEffectLengthGetter lengthGetter,
    OffsetEffectColorGetter colorGetter
);
Point v2p(const Vertex* v);



/**
 * @brief Goes through all keyframes in a keyframe interpolator, and lets you
 * adjust the value in each one, by running the "predicate" function for each.
 *
 * @tparam t Value type for the interpolator.
 * @param interpolator Interpolator to adjust.
 * @param predicate Function whose argument is the original value at that
 * keyframe, and whose return value is the new value.
 * @return Whether the operation succeeded.
 */
template<typename t>
bool adjustKeyframeInterpolatorValues(
    KeyframeInterpolator<t> &interpolator,
    std::function<t(const t &)> predicate
) {
    bool result = false;
    size_t nKeyframes = interpolator.getKeyframeCount();
    for(size_t k = 0; k < nKeyframes; k++) {
        const auto &origKeyframe = interpolator.getKeyframe(k);
        interpolator.setKeyframeValue(k, predicate(origKeyframe.second));
        result = true;
    }
    return result;
}


/**
 * @brief Processes a Dear ImGui text widget, but sets the font to be
 * monospaced.
 *
 * @tparam ArgsT Function argument type.
 * @param args Function arguments to pass to ImGui::Text().
 */
template <typename ...Args_t>
void monoText(Args_t && ...args) {
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    ImGui::Text(std::forward<Args_t>(args)...);
    ImGui::PopFont();
}
