/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally accessible functions.
 */

#define _USE_MATH_DEFINES
#undef _CMATH_

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdlib.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "misc_functions.h"

#include "../lib/imgui/imgui_stdlib.h"
#include "../util/allegro_utils.h"
#include "../util/backtrace.h"
#include "../util/general_utils.h"
#include "../util/imgui_utils.h"
#include "../util/os_utils.h"
#include "../util/string_utils.h"
#include "const.h"
#include "drawing.h"
#include "game.h"
#include "init.h"


/**
 * @brief Checks if there are any walls between two points.
 * i.e. any edges that a mob can't simply step up to.
 *
 * @param p1 First point.
 * @param p2 Second point.
 * @param ignoreWallsBelowZ Any walls whose sector Zs are below
 * this value get ignored. Use -FLT_MAX to not ignore any wall.
 * @param outImpassableWalls If not nullptr, true will be returned here if
 * any of the walls are impassable, i.e. the void or "blocking"-type
 * sectors. False otherwise.
 * @return Whether there are walls between.
 */
bool areaWallsBetween(
    const Point& p1, const Point& p2,
    float ignoreWallsBelowZ, bool* outImpassableWalls
) {
    Point bbTL = p1;
    Point bbBR = p1;
    updateMinMaxCoords(bbTL, bbBR, p2);
    
    set<Edge*> candidateEdges;
    if(
        !game.curAreaData->bmap.getEdgesInRegion(
            bbTL, bbBR,
            candidateEdges
        )
    ) {
        //Somehow out of bounds.
        if(outImpassableWalls) *outImpassableWalls = true;
        return true;
    }
    
    for(auto const& ePtr : candidateEdges) {
        if(
            !lineSegsIntersect(
                p1, p2,
                v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]),
                nullptr
            )
        ) {
            continue;
        }
        for(size_t s = 0; s < 2; s++) {
            if(!ePtr->sectors[s]) {
                //No sectors means there's out-of-bounds geometry in the way.
                if(outImpassableWalls) *outImpassableWalls = true;
                return true;
            }
            if(ePtr->sectors[s]->type == SECTOR_TYPE_BLOCKING) {
                //If a blocking sector is in the way, no clear line.
                if(outImpassableWalls) *outImpassableWalls = true;
                return true;
            }
        }
        if(
            ePtr->sectors[0]->z < ignoreWallsBelowZ &&
            ePtr->sectors[1]->z < ignoreWallsBelowZ
        ) {
            //This wall was chosen to be ignored.
            continue;
        }
        if(
            fabs(ePtr->sectors[0]->z - ePtr->sectors[1]->z) >
            GEOMETRY::STEP_HEIGHT
        ) {
            //The walls are more than stepping height in difference.
            //So it's a genuine wall in the way.
            if(outImpassableWalls) *outImpassableWalls = false;
            return true;
        }
    }
    
    if(outImpassableWalls) *outImpassableWalls = false;
    return false;
}


/**
 * @brief Clears the textures of the area's sectors from memory.
 */
void clearAreaTextures() {
    if(!game.curAreaData) return;
    
    for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
        Sector* sPtr = game.curAreaData->sectors[s];
        if(
            sPtr->textureInfo.bitmap &&
            sPtr->textureInfo.bitmap != game.bmpError
        ) {
            game.content.bitmaps.list.free(sPtr->textureInfo.bmpName);
            sPtr->textureInfo.bitmap = nullptr;
        }
    }
}


/**
 * @brief Purposely crashes the engine, reporting as much information
 * as possible to the logs. Used when a fatal problem occurs.
 *
 * @param reason Explanation of the type of crash (assert, SIGSEGV, etc.).
 * @param info Any extra information to report to the logs.
 * @param exitStatus Program exit status.
 */
void crash(const string& reason, const string& info, int exitStatus) {

    if(game.display) {
        ALLEGRO_BITMAP* backbuffer = al_get_backbuffer(game.display);
        if(backbuffer) {
            al_save_bitmap(
                (
                    FOLDER_PATHS_FROM_ROOT::USER_DATA + "/" +
                    "crash_" + getCurrentTime(true) + ".png"
                ).c_str(),
                backbuffer
            );
        }
    }
    
    string errorStr = "Program crash!\n";
    errorStr +=
        "  Reason: " + reason + ".\n"
        "  Info: " + info + "\n"
        "  Time: " + getCurrentTime(false) + ".\n";
    if(game.errors.sessionHasErrors()) {
        errorStr += "  Error log has messages!\n";
    }
    errorStr +=
        "  Game state: " + game.getCurStateName() + ". deltaT: " +
        (
            game.deltaT == 0.0f ? "0" :
            f2s(game.deltaT) + " (" + f2s(1.0f / game.deltaT) + " FPS)"
        ) + ".\n"
        "  Mob count: " +
        i2s(game.states.gameplay->mobs.all.size()) + ". Particle count: " +
        i2s(game.states.gameplay->particles.getCount()) + ".\n" +
        "  Bitmaps loaded: " + i2s(game.content.bitmaps.list.getListSize()) +
        " (" +
        i2s(game.content.bitmaps.list.getTotalUses()) + " total uses).\n" +
        "  Current area: ";
        
    if(game.curAreaData && !game.curAreaData->name.empty()) {
        errorStr +=
            game.curAreaData->name + ", version " +
            game.curAreaData->version + ".\n";
    } else {
        errorStr += "none.\n";
    }
    
    errorStr += "  Current leader: ";
    
    if(
        !game.states.gameplay->players.empty() &&
        game.states.gameplay->players[0].leaderPtr
    ) {
        errorStr +=
            game.states.gameplay->players[0].leaderPtr->type->name + ", at " +
            p2s(game.states.gameplay->players[0].leaderPtr->pos) +
            ", state history: " +
            game.states.gameplay->players[0].leaderPtr->fsm.curState->name;
        for(size_t h = 0; h < STATE_HISTORY_SIZE; h++) {
            errorStr +=
                " " +
                game.states.gameplay->players[0].leaderPtr->
                fsm.prevStateNames[h];
        }
        errorStr += "\n  10 closest Pikmin to that leader:\n";
        
        vector<Pikmin*> closestPikmin =
            game.states.gameplay->mobs.pikmin;
        sort(
            closestPikmin.begin(), closestPikmin.end(),
        [] (const Pikmin * p1, const Pikmin * p2) -> bool {
            return
            Distance(
                game.states.gameplay->players[0].leaderPtr->pos,
                p1->pos
            ).toFloat() <
            Distance(
                game.states.gameplay->players[0].leaderPtr->pos,
                p2->pos
            ).toFloat();
        }
        );
        
        size_t closestPAmount = std::min(closestPikmin.size(), (size_t) 10);
        for(size_t p = 0; p < closestPAmount; p++) {
            errorStr +=
                "    " + closestPikmin[p]->type->name + ", at " +
                p2s(closestPikmin[p]->pos) + ", history: " +
                closestPikmin[p]->fsm.curState->name;
            for(size_t h = 0; h < STATE_HISTORY_SIZE; h++) {
                errorStr += " " + closestPikmin[p]->fsm.prevStateNames[h];
            }
            errorStr += "\n";
        }
    } else {
        errorStr += "none.";
    }
    
    game.errors.report(errorStr);
    
    showSystemMessageBox(
        nullptr, "Program crash!",
        "Pikifen has crashed!",
        "Sorry about that! To help fix this problem, please read the "
        "troubleshooting section of the included manual. Thanks!",
        nullptr,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(exitStatus);
}


/**
 * @brief Checks whether a given edge should get a ledge smoothing
 * edge offset effect or not.
 *
 * @param ePtr Edge to check.
 * @param outAffectedSector If there should be an effect, the affected sector,
 * i.e. the one getting the smoothing, is returned here.
 * @param outUnaffectedSector If there should be an effect, the
 * unaffected sector, i.e. the lower one, is returned here.
 * @return Whether it has ledge smoothing.
 */
bool doesEdgeHaveLedgeSmoothing(
    Edge* ePtr, Sector** outAffectedSector, Sector** outUnaffectedSector
) {
    //Never-smooth walls don't have the effect.
    if(ePtr->ledgeSmoothingLength <= 0.0f) return false;
    
    if(
        (ePtr->sectors[0] && !ePtr->sectors[1]) ||
        ePtr->sectors[1]->isBottomlessPit
    ) {
        //If 0 exists but 1 doesn't.
        *outAffectedSector = ePtr->sectors[0];
        *outUnaffectedSector = ePtr->sectors[1];
        return true;
        
    } else if(
        (!ePtr->sectors[0] && ePtr->sectors[1]) ||
        ePtr->sectors[0]->isBottomlessPit
    ) {
        //If 1 exists but 0 doesn't.
        *outAffectedSector = ePtr->sectors[1];
        *outUnaffectedSector = ePtr->sectors[0];
        return true;
        
    } else {
        //Return whichever one is the tallest.
        if(ePtr->sectors[0]->z > ePtr->sectors[1]->z) {
            *outAffectedSector = ePtr->sectors[0];
            *outUnaffectedSector = ePtr->sectors[1];
            return true;
        } else if(ePtr->sectors[1]->z > ePtr->sectors[0]->z) {
            *outAffectedSector = ePtr->sectors[1];
            *outUnaffectedSector = ePtr->sectors[0];
            return true;
        } else {
            return false;
        }
        
    }
}


/**
 * @brief Checks whether a given edge should get a liquid limit
 * edge offset effect or not.
 *
 * @param ePtr Edge to check.
 * @param outAffectedSector If there should be an effect, the affected sector,
 * i.e. the one with the liquid, is returned here.
 * @param outUnaffectedSector If there should be an effect, the
 * unaffected sector, i.e. the one without the liquid, is returned here.
 * @return Whether it has a liquid limit.
 */
bool doesEdgeHaveLiquidLimit(
    Edge* ePtr, Sector** outAffectedSector, Sector** outUnaffectedSector
) {
    //Check if the sectors exist.
    if(!ePtr->sectors[0] || !ePtr->sectors[1]) return false;
    
    //Check which ones have liquid.
    bool hasLiquid[2] = {false, false};
    for(unsigned char s = 0; s < 2; s++) {
        hasLiquid[s] =
            ePtr->sectors[s]->hazard &&
            ePtr->sectors[s]->hazard->associatedLiquid;
    }
    
    //Return edges with liquid on one side only.
    if(hasLiquid[0] && !hasLiquid[1]) {
        *outAffectedSector = ePtr->sectors[0];
        *outUnaffectedSector = ePtr->sectors[1];
        return true;
    } else if(hasLiquid[1] && !hasLiquid[0]) {
        *outAffectedSector = ePtr->sectors[1];
        *outUnaffectedSector = ePtr->sectors[0];
        return true;
    } else {
        return false;
    }
}


/**
 * @brief Checks whether a given edge should get a wall shadow
 * edge offset effect or not.
 *
 * @param ePtr Edge to check.
 * @param outAffectedSector If there should be an effect, the affected sector,
 * i.e. the one getting shaded, is returned here.
 * @param outUnaffectedSector If there should be an effect, the
 * unaffected sector, i.e. the one casting the shadow, is returned here.
 * @return Whether it has a wall shadow.
 */
bool doesEdgeHaveWallShadow(
    Edge* ePtr, Sector** outAffectedSector, Sector** outUnaffectedSector
) {
    //Never-cast walls don't cast.
    if(ePtr->wallShadowLength <= 0.0f) return false;
    
    //Invalid sectors don't cast.
    if(!ePtr->sectors[0] || !ePtr->sectors[1]) return false;
    if(ePtr->sectors[0]->isBottomlessPit) return false;
    if(ePtr->sectors[1]->isBottomlessPit) return false;
    
    //Same-height sectors can't cast.
    if(ePtr->sectors[0]->z == ePtr->sectors[1]->z) return false;
    
    //We can already save which one is highest.
    if(ePtr->sectors[0]->z > ePtr->sectors[1]->z) {
        *outUnaffectedSector = ePtr->sectors[0];
        *outAffectedSector = ePtr->sectors[1];
    } else {
        *outUnaffectedSector = ePtr->sectors[1];
        *outAffectedSector = ePtr->sectors[0];
    }
    
    if(ePtr->wallShadowLength != LARGE_FLOAT) {
        //Fixed shadow length.
        return true;
    } else {
        //Auto shadow length.
        return
            (*outUnaffectedSector)->z >
            (*outAffectedSector)->z + GEOMETRY::STEP_HEIGHT;
    }
}


/**
 * @brief Returns the mob that is closest to the mouse cursor.
 *
 * @param view Viewport to calculate from.
 * @param mustHaveHealth If true, only count enemies that have health
 * (health and max health > 0).
 * @return The mob.
 */
Mob* getClosestMobToCursor(const Viewport& view, bool mustHaveHealth) {
    Distance closestMobToCursorDist;
    Mob* closestMobToCursor = nullptr;
    
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* mPtr = game.states.gameplay->mobs.all[m];
        
        bool hasHealth = mPtr->health > 0.0f && mPtr->maxHealth > 0.0f;
        if(mustHaveHealth && !hasHealth) continue;
        if(mPtr->isStoredInsideMob()) continue;
        if(!mPtr->fsm.curState) continue;
        
        Distance d = Distance(view.cursorWorldPos, mPtr->pos);
        if(!closestMobToCursor || d < closestMobToCursorDist) {
            closestMobToCursor = mPtr;
            closestMobToCursorDist = d;
        }
    }
    
    return closestMobToCursor;
}


/**
 * @brief Returns the engine's version as a string.
 *
 * @return The string.
 */
string getEngineVersionString() {
    return
        i2s(VERSION_MAJOR) + "." +
        i2s(VERSION_MINOR) + "." +
        i2s(VERSION_REV);
}


/**
 * @brief Returns the color a ledge's smoothing should be.
 *
 * @param ePtr Edge with the ledge.
 * @return The color.
 */
ALLEGRO_COLOR getLedgeSmoothingColor(Edge* ePtr) {
    return ePtr->ledgeSmoothingColor;
}


/**
 * @brief Returns the length a ledge's smoothing should be.
 *
 * @param ePtr Edge with the ledge.
 * @return The length.
 */
float getLedgeSmoothingLength(Edge* ePtr) {
    return ePtr->ledgeSmoothingLength;
}


/**
 * @brief Returns the color a liquid limit's effect should be.
 *
 * @param ePtr Edge with the liquid limit.
 * @return The color.
 */
ALLEGRO_COLOR getLiquidLimitColor(Edge* ePtr) {
    return {1.0f, 1.0f, 1.0f, 0.75f};
}


/**
 * @brief Returns the length a liquid's limit effect.
 *
 * @param ePtr Edge with the liquid limit.
 * @return The length.
 */
float getLiquidLimitLength(Edge* ePtr) {
    //Let's vary the length randomly by the topleftmost edge coordinates.
    //It's better to use this than using just the first edge, for instance,
    //because that would result in many cases of edges that share a first
    //vertex. So it wouldn't look as random.
    //It is much more rare for two edges to share a topleftmost vertex.
    Point minCoords = v2p(ePtr->vertexes[0]);
    updateMinCoords(minCoords, v2p(ePtr->vertexes[1]));
    float r =
        (hashNr2(minCoords.x, minCoords.y) / (float) UINT32_MAX) * 5.0f;
    return
        15.0f +
        12.0f * sin((game.states.gameplay->areaTimePassed * 2.0f) + r);
}


/**
 * @brief Returns the name of the entry in a player records data file that
 * refers to the given area.
 *
 * @param areaPtr The area.
 * @return The entry name.
 */
string getMissionRecordEntryName(Area* areaPtr) {
    return
        areaPtr->name + ";" +
        getSubtitleOrMissionGoal(
            areaPtr->subtitle, areaPtr->type,
            areaPtr->mission.goal
        ) + ";" +
        areaPtr->maker + ";" +
        areaPtr->version;
}


/**
 * @brief Scans a circle of radius 8 around the cursor, and finds the mob
 * that comes after this one. i.e. the one with the next
 * highest ID number. If it's already the highest, it loops back around
 * to the lowest.
 *
 * @param view Viewport to calculate from.
 * @param pivot Return the mob after this one, or if nullptr, return the lowest.
 * @param mustHaveHealth If true, only count enemies that have health
 * (health and max health > 0).
 * @return The mob, or nullptr if there is none nearby.
 */
Mob* getNextMobNearCursor(
    const Viewport& view, Mob* pivot, bool mustHaveHealth
) {
    vector<Mob*> mobsNearCursor;
    
    //First, get all mobs that are close to the cursor.
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
        Mob* mPtr = game.states.gameplay->mobs.all[m];
        
        bool hasHealth = mPtr->health > 0.0f && mPtr->maxHealth > 0.0f;
        if(mustHaveHealth && !hasHealth) continue;
        if(mPtr->isStoredInsideMob()) continue;
        if(!mPtr->fsm.curState) continue;
        
        Distance d = Distance(view.cursorWorldPos, mPtr->pos);
        if(d < 8.0f) {
            mobsNearCursor.push_back(mPtr);
        }
    }
    
    if(mobsNearCursor.empty()) return nullptr;
    
    //Sort them by ID.
    std::sort(
        mobsNearCursor.begin(), mobsNearCursor.end(),
    [] (const Mob * m1, const Mob * m2) -> bool {
        return m1->id < m2->id;
    }
    );
    
    //Find the pivot's index so we can go to the next one.
    size_t pivotIdx = INVALID;
    for(size_t m = 0; m < mobsNearCursor.size(); m++) {
        if(mobsNearCursor[m] == pivot) {
            pivotIdx = m;
            break;
        }
    }
    
    //Return the next one, looping, or just returns the first if not available.
    if(pivotIdx == INVALID) {
        return mobsNearCursor[0];
    } else {
        return getNextInVector(mobsNearCursor, pivotIdx);
    }
}


/**
 * @brief Returns an area's subtitle or, if none is specified,
 * the mission's goal.
 *
 * @param subtitle Area subtitle.
 * @param areaType Type of area.
 * @param goal Mission goal.
 * @return The subtitle or goal.
 */
string getSubtitleOrMissionGoal(
    const string& subtitle, const AREA_TYPE areaType,
    const MISSION_GOAL goal
) {
    if(subtitle.empty() && areaType == AREA_TYPE_MISSION) {
        return game.missionGoals[goal]->getName();
    }
    
    return subtitle;
}


/**
 * @brief Calculates the vertex info necessary to draw the throw preview line,
 * from a given start point to a given end point.
 *
 * The vertexes returned always come in groups of four, and each group
 * must be drawn individually with the ALLEGRO_PRIM_TRIANGLE_FAN type.
 *
 * @param vertexes The array of vertexes to fill.
 * Must have room for at least 16.
 * @param start Start the line at this point.
 * This is a ratio from the leader (0) to the cursor (1).
 * @param end Same as start, but for the end point.
 * @param leaderPos Position of the leader.
 * @param cursorPos Position of the cursor.
 * @param color Color of the line.
 * @param uOffset Offset the texture u by this much.
 * @param uScale Scale the texture u by this much.
 * @param varyThickness If true, thickness varies as the line goes
 * forward. False makes it use the same thickness (the minimal one) throughout.
 * @return The amount of vertexes needed.
 */
unsigned char getThrowPreviewVertexes(
    ALLEGRO_VERTEX* vertexes,
    float start, float end,
    const Point& leaderPos, const Point& cursorPos,
    const ALLEGRO_COLOR& color,
    float uOffset, float uScale,
    bool varyThickness
) {
    const float segmentPoints[] = {
        0.0f, LEADER::THROW_PREVIEW_FADE_IN_RATIO,
        0.5f, LEADER::THROW_PREVIEW_FADE_OUT_RATIO,
        1.0f
    };
    
    float maxThickness =
        varyThickness ?
        LEADER::THROW_PREVIEW_DEF_MAX_THICKNESS :
        LEADER::THROW_PREVIEW_MIN_THICKNESS;
        
    float leaderToCursorDist = Distance(leaderPos, cursorPos).toFloat();
    unsigned char curV = 0;
    
    auto getThickness =
    [maxThickness] (float n) -> float {
        if(n >= 0.5f) {
            n = 1 - n;
        }
        return
        interpolateNumber(
            n, 0.0f, 0.5f, LEADER::THROW_PREVIEW_MIN_THICKNESS, maxThickness
        );
    };
    auto getColor =
    [&color] (float n) -> ALLEGRO_COLOR {
        if(n >= 0.5f) {
            n = 1 - n;
        }
        if(n < LEADER::THROW_PREVIEW_FADE_IN_RATIO) {
            return
            interpolateColor(
                n, 0.0f, LEADER::THROW_PREVIEW_FADE_IN_RATIO,
                changeAlpha(color, 0),
                color
            );
        } else {
            return color;
        }
    };
    
    //Get the vertexes of each necessary segment.
    for(unsigned char segment = 0; segment < 4; segment++) {
        float segmentStart = std::max(segmentPoints[segment], start);
        float segmentEnd = std::min(segmentPoints[segment + 1], end);
        
        if(
            segmentStart > segmentPoints[segment + 1] ||
            segmentEnd < segmentPoints[segment]
        ) {
            continue;
        }
        
        vertexes[curV].x = leaderToCursorDist * segmentStart;
        vertexes[curV].y = -getThickness(segmentStart) / 2.0f;
        vertexes[curV].color = getColor(segmentStart);
        curV++;
        
        vertexes[curV] = vertexes[curV - 1];
        vertexes[curV].y = -vertexes[curV].y;
        curV++;
        
        vertexes[curV].x = leaderToCursorDist * segmentEnd;
        vertexes[curV].y = getThickness(segmentEnd) / 2.0f;
        vertexes[curV].color = getColor(segmentEnd);
        curV++;
        
        vertexes[curV] = vertexes[curV - 1];
        vertexes[curV].y = -vertexes[curV].y;
        curV++;
    }
    
    //Final setup on all points.
    for(unsigned char v = 0; v < curV; v++) {
        Point p(vertexes[v].x, vertexes[v].y);
        
        //Apply the texture UVs.
        vertexes[v].u = vertexes[v].x / uScale - uOffset;
        vertexes[v].v = vertexes[v].y;
        
        //Rotate and move all points. For the sake of simplicity, up until now,
        //they were assuming the throw is perfectly to the right (0 degrees),
        //and that it starts on the world origin.
        p = rotatePoint(p, getAngle(leaderPos, cursorPos));
        p += leaderPos;
        vertexes[v].x = p.x;
        vertexes[v].y = p.y;
        
        //Give Z a value.
        vertexes[v].z = 0.0f;
    }
    
    return curV;
}


/**
 * @brief Given a string representation of mob script variables,
 * returns a map, where every key is a variable, and every value is the
 * variable's value.
 *
 * @param varsString String with the variables.
 * @return The map.
 */
map<string, string> getVarMap(const string& varsString) {
    map<string, string> finalMap;
    vector<string> rawVars = semicolonListToVector(varsString);
    
    for(size_t v = 0; v < rawVars.size(); v++) {
        vector<string> rawParts = split(rawVars[v], "=");
        if(rawParts.size() < 2) {
            continue;
        }
        finalMap[trimSpaces(rawParts[0])] = trimSpaces(rawParts[1]);
    }
    return finalMap;
}


/**
 * @brief Returns the color a wall's shadow should be.
 *
 * @param ePtr Edge with the wall.
 * @return The color.
 */
ALLEGRO_COLOR getWallShadowColor(Edge* ePtr) {
    return ePtr->wallShadowColor;
}


/**
 * @brief Returns the length a wall's shadow should be.
 *
 * @param ePtr Edge with the wall.
 * @return The length.
 */
float getWallShadowLength(Edge* ePtr) {
    if(ePtr->wallShadowLength != LARGE_FLOAT) {
        return ePtr->wallShadowLength;
    }
    
    float heightDifference =
        fabs(ePtr->sectors[0]->z - ePtr->sectors[1]->z);
    return
        std::clamp(
            heightDifference * GEOMETRY::SHADOW_AUTO_LENGTH_MULT,
            GEOMETRY::SHADOW_MIN_AUTO_LENGTH,
            GEOMETRY::SHADOW_MAX_AUTO_LENGTH
        );
}


/**
 * @brief Auxiliary function that returns a table used in the weather configs.
 *
 * @param node Data node with the weather table.
 * @return The table.
 */
vector<std::pair<int, string> > getWeatherTable(DataNode* node) {
    vector<std::pair<int, string> > table;
    size_t nPoints = node->getNrOfChildren();
    
    for(size_t p = 0; p < nPoints; p++) {
        DataNode* pointNode = node->getChild(p);
        table.push_back(make_pair(s2i(pointNode->name), pointNode->value));
    }
    
    sort(
        table.begin(), table.end(),
        [] (
            std::pair<int, string> p1,
            std::pair<int, string> p2
    ) -> bool {
        return p1.first < p2.first;
    }
    );
    
    if(!table.empty()) {
        auto first = table.front();
        auto last = table.back();
        if(first.first > 0) {
            //If there is no data for midnight (0),
            //use the data from the last point
            //(this is because the day loops after 24:00;
            //needed for interpolation).
            table.insert(
                table.begin(),
                make_pair(last.first - 24 * 60, last.second)
            );
        }
        if(last.first < 24 * 60) {
            //If there is no data for midnight (24),
            //use the data from the first point
            //(this is because the day loops after 24:00;
            //needed for interpolation).
            table.push_back(
                make_pair(first.first + 24 * 60, first.second)
            );
        }
    }
    
    return table;
}


/**
 * @brief Adds a GUI item that shows the input icon for going back in a menu.
 *
 * @param gui GUI manager to add the item to.
 * @param itemName Internal name of the GUI item.
 */
void guiAddBackInputIcon(GuiManager* gui, const string& itemName) {
    GuiItem* backInput = new GuiItem();
    backInput->onDraw =
    [] (const GuiItem::DrawInfo & draw) {
        if(!game.options.misc.showHudInputIcons) return;
        const PlayerInputSource& s =
            game.controls.findBind(PLAYER_ACTION_TYPE_MENU_BACK).
            inputSource;
        if(s.type == INPUT_SOURCE_TYPE_NONE) return;
        drawPlayerInputSourceIcon(
            game.sysContent.fntSlim, s, true, draw.center, draw.size
        );
    };
    gui->addItem(backInput, itemName);
}


/**
 * @brief Processes a Dear ImGui button widget, but sets the button font
 * to be monospaced.
 *
 * @param label Same as you'd pass to ImGui::InputText().
 * @param size Same as you'd pass to ImGui::InputText().
 * @return Whether the button was activated.
 */
bool monoButton(const char* label, const ImVec2& size) {
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result = ImGui::Button(label, size);
    ImGui::PopFont();
    
    return result;
}


/**
 * @brief Processes a Dear ImGui combo widget, but sets the box font
 * to be monospaced.
 *
 * @param label Combo widget label.
 * @param currentItem Index number of the current selected item. -1 means none.
 * @param items List of items.
 * @param popupMaxHeightInItems Maximum height of the popup,
 * in number of items.
 * @return Whether the value was changed.
 */
bool monoCombo(
    const string& label, int* currentItem, const vector<string>& items,
    int popupMaxHeightInItems
) {
    bool hasText = label[0] != '#';
    ImGui::BeginGroup();
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result =
        ImGui::Combo(
            hasText ? "##cb" + label : label,
            currentItem, items, popupMaxHeightInItems
        );
    ImGui::PopFont();
    
    if(hasText) {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("%s", label.c_str());
    }
    ImGui::EndGroup();
    
    return result;
}


/**
 * @brief Wrapper for creating a Dear ImGui combo box widget, but
 * using a string to control the selection,
 * as well as a vector of strings for the list of items.
 *
 * @param label Combo widget label.
 * @param currentItem Name of the current selected item.
 * @param items List of items.
 * @param popupMaxHeightInItems Maximum height of the popup,
 * in number of items.
 * @return Whether the value was changed.
 */
bool monoCombo(
    const string& label, string* currentItem, const vector<string>& items,
    int popupMaxHeightInItems
) {
    bool hasText = label[0] != '#';
    ImGui::BeginGroup();
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result =
        ImGui::Combo(
            hasText ? "##cb" + label : label,
            currentItem, items, popupMaxHeightInItems
        );
    ImGui::PopFont();
    
    if(hasText) {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("%s", label.c_str());
    }
    ImGui::EndGroup();
    
    return result;
}


/**
 * @brief Wrapper for creating a Dear ImGui combo box widget, but
 * using a string to control the selection,
 * as well as two vector of strings for the list of items, one with
 * the internal values of each item, another with the names to display.
 *
 * @param label Combo widget label.
 * @param currentItem Internal value of the current selected item.
 * @param itemInternalValues List of internal values for each item.
 * @param itemDisplayNames List of names to show the user for each item.
 * @param popupMaxHeightInItems Maximum height of the popup,
 * in number of items.
 * @return Whether the value was changed.
 */
bool monoCombo(
    const string& label, string* currentItem,
    const vector<string>& itemInternalValues,
    const vector<string>& itemDisplayNames,
    int popupMaxHeightInItems
) {
    bool hasText = label[0] != '#';
    ImGui::BeginGroup();
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result =
        ImGui::Combo(
            hasText ? "##cb" + label : label,
            currentItem, itemInternalValues,
            itemDisplayNames, popupMaxHeightInItems
        );
    ImGui::PopFont();
    
    if(hasText) {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("%s", label.c_str());
    }
    ImGui::EndGroup();
    
    return result;
}


/**
 * @brief Processes a Dear ImGui input text widget, but sets the input font
 * to be monospaced.
 *
 * @param label Same as you'd pass to ImGui::InputText().
 * @param str Same as you'd pass to ImGui::InputText().
 * @param flags Same as you'd pass to ImGui::InputText().
 * @param callback Same as you'd pass to ImGui::InputText().
 * @param userData Same as you'd pass to ImGui::InputText().
 * @return Whether the text input in the was changed by the user.
 */
bool monoInputText(
    const char* label, string* str, ImGuiInputTextFlags flags,
    ImGuiInputTextCallback callback, void* userData
) {
    bool hasText = label[0] != '#';
    ImGui::BeginGroup();
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result =
        ImGui::InputText(
            hasText ? ("##ti" + string(label)).c_str() : label,
            str, flags, callback, userData
        );
    ImGui::PopFont();
    
    if(hasText) {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("%s", label);
    }
    ImGui::EndGroup();
    
    return result;
}


/**
 * @brief Processes a Dear ImGui input text with hint widget, but sets
 * the input font to be monospaced.
 *
 * @param label Same as you'd pass to ImGui::InputText().
 * @param hint Same as you'd pass to ImGui::InputText().
 * @param str Same as you'd pass to ImGui::InputText().
 * @param flags Same as you'd pass to ImGui::InputText().
 * @param callback Same as you'd pass to ImGui::InputText().
 * @param userData Same as you'd pass to ImGui::InputText().
 * @return Whether the text input was changed by the user.
 */
bool monoInputTextWithHint(
    const char* label, const char* hint, string* str,
    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback,
    void* userData
) {
    bool hasText = label[0] != '#';
    bool strEmpty = str->empty();
    ImGui::BeginGroup();
    if(!strEmpty) ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result =
        ImGui::InputTextWithHint(
            hasText ? ("##ti" + string(label)).c_str() : label,
            hint, str, flags, callback, userData
        );
    if(!strEmpty) ImGui::PopFont();
    
    if(hasText) {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("%s", label);
    }
    ImGui::EndGroup();
    
    return result;
}


/**
 * @brief Processes a Dear ImGui list box widget, but sets
 * the font to be monospaced.
 *
 * @param label Same as you'd pass to ImGui::InputText().
 * @param currentItem Same as you'd pass to ImGui::InputText().
 * @param items Same as you'd pass to ImGui::InputText().
 * @param heightInItems Same as you'd pass to ImGui::InputText().
 * @return Whether the value was changed.
 */
bool monoListBox(
    const string& label, int* currentItem, const vector<string>& items,
    int heightInItems
) {
    bool hasText = label[0] != '#';
    ImGui::BeginGroup();
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result =
        ImGui::ListBox(
            hasText ? ("##lb" + string(label)).c_str() : label,
            currentItem, items, heightInItems
        );
    ImGui::PopFont();
    
    if(hasText) {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("%s", label.c_str());
    }
    ImGui::EndGroup();
    
    return result;
}


/**
 * @brief Processes a Dear ImGui selectable widget, but sets
 * the font to be monospaced.
 *
 * @param label Same as you'd pass to ImGui::InputText().
 * @param selected Same as you'd pass to ImGui::InputText().
 * @param flags Same as you'd pass to ImGui::InputText().
 * @param size Same as you'd pass to ImGui::InputText().
 * @return Whether the text input was changed by the user.
 */
bool monoSelectable(
    const char* label, bool selected, ImGuiSelectableFlags flags,
    const ImVec2& size
) {
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result = ImGui::Selectable(label, selected, flags, size);
    ImGui::PopFont();
    return result;
}


/**
 * @brief Processes a Dear ImGui selectable widget, but sets
 * the font to be monospaced.
 *
 * @param label Same as you'd pass to ImGui::InputText().
 * @param pSelected Same as you'd pass to ImGui::InputText().
 * @param flags Same as you'd pass to ImGui::InputText().
 * @param size Same as you'd pass to ImGui::InputText().
 * @return Whether the text input was changed by the user.
 */
bool monoSelectable(
    const char* label, bool* pSelected, ImGuiSelectableFlags flags,
    const ImVec2& size
) {
    ImGui::PushFont(game.sysContent.fntDearImGuiMonospace);
    bool result = ImGui::Selectable(label, pSelected, flags, size);
    ImGui::PopFont();
    return result;
}


/**
 * @brief Opens the manual on the user's web browser in the specified page.
 *
 * @param page Page to open, with the .html extension and any anchors.
 * @return Whether it succeeded in opening the browser.
 */
bool openManual(const string& page) {
    //This function could have a page argument and an anchor argument,
    //and it could have included the .html extension automatically, but doing
    //it this way makes it so that the string, e.g. "page.html#anchor" is
    //present directly in code. This way, if the link to "page.html" or
    //"anchor" needs to be updated, a full-project text search will find
    //instances of it inside the code just as it will in the HTML of pages.
    string url =
        "file://" + std::filesystem::current_path().string() +
        "/manual/content/" + page;
    return openWebBrowser(url);
}


/**
 * @brief Prints a bit of info onto the game window, for some seconds.
 *
 * @param text Text to print. Can use line breaks.
 * @param totalDuration Total amount of time in which the text is present.
 * @param fadeDuration When closing, fade out in the last N seconds.
 */
void printInfo(
    const string& text, float totalDuration, float fadeDuration
) {
    game.makerTools.infoPrintText = text;
    game.makerTools.infoPrintDuration = totalDuration;
    game.makerTools.infoPrintFadeDuration = fadeDuration;
    game.makerTools.infoPrintTimer.start(totalDuration);
}


/**
 * @brief Reports a fatal error to the user and shuts down the program.
 *
 * @param s String explaining the error.
 * @param dn File to log the error into, if any.
 */
void reportFatalError(const string& s, const DataNode* dn) {
    game.errors.report(s, dn);
    
    showSystemMessageBox(
        nullptr, "Fatal error!",
        "Pikifen has encountered a fatal error!",
        s.c_str(),
        nullptr,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(-1);
    
}


/**
 * @brief Saves the maker tools settings.
 */
void saveMakerTools() {
    DataNode file("", "");
    game.makerTools.saveToDataNode(&file);
    file.saveFile(FILE_PATHS_FROM_ROOT::MAKER_TOOLS, true, true);
}


/**
 * @brief Saves the player's options.
 */
void saveOptions() {
    DataNode file("", "");
    game.options.saveToDataNode(&file);
    file.saveFile(FILE_PATHS_FROM_ROOT::OPTIONS, true, true);
}


/**
 * @brief Saves the current backbuffer onto a file.
 * In other words, dumps a screenshot.
 */
void saveScreenshot() {
    string baseFileName = "screenshot_" + getCurrentTime(true);
    
    //Check if a file with this name already exists.
    vector<string> files =
        folderToVector(FOLDER_PATHS_FROM_ROOT::USER_DATA, false);
    size_t variantNr = 1;
    string finalFileName = baseFileName;
    bool validName = false;
    
    do {
    
        if(!isInContainer(files, finalFileName + ".png")) {
            //File name not found.
            //Go ahead and create a screenshot with this name.
            validName = true;
        } else {
            variantNr++;
            finalFileName = baseFileName + " " + i2s(variantNr);
        }
        
    } while(!validName);
    
    //Before saving, let's set every pixel's alpha to 255.
    //This is because alpha operations on the backbuffer behave weirdly.
    //On some machines, when saving to a bitmap, it will use those weird
    //alpha values, which may be harmless on the backbuffer, but not so much
    //on a saved PNG file.
    ALLEGRO_BITMAP* screenshot =
        al_clone_bitmap(al_get_backbuffer(game.display));
    ALLEGRO_LOCKED_REGION* region =
        al_lock_bitmap(
            screenshot,
            ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_READWRITE
        );
        
    unsigned char* row = (unsigned char*) region->data;
    int bmpW = ceil(al_get_bitmap_width(screenshot));
    int bmpH = ceil(al_get_bitmap_height(screenshot));
    for(int y = 0; y < bmpH; y++) {
        for(int x = 0; x < bmpW; x++) {
            row[(x) * 4 + 3] = 255;
        }
        row += region->pitch;
    }
    
    al_unlock_bitmap(screenshot);
    
    al_save_bitmap(
        (
            FOLDER_PATHS_FROM_ROOT::USER_DATA + "/" + finalFileName + ".png"
        ).c_str(),
        screenshot
    );
    
    al_destroy_bitmap(screenshot);
}


/**
 * @brief Saves the engine's lifetime statistics.
 */
void saveStatistics() {
    DataNode statsFile("", "");
    const Statistics& s = game.statistics;
    GetterWriter sGW(&statsFile);
    
    sGW.write("startups", s.startups);
    sGW.write("runtime", s.runtime);
    sGW.write("gameplay_time", s.gameplayTime);
    sGW.write("area_entries", s.areaEntries);
    sGW.write("pikmin_births", s.pikminBirths);
    sGW.write("pikmin_deaths", s.pikminDeaths);
    sGW.write("pikmin_eaten", s.pikminEaten);
    sGW.write("pikmin_hazard_deaths", s.pikminHazardDeaths);
    sGW.write("pikmin_blooms", s.pikminBlooms);
    sGW.write("pikmin_saved", s.pikminSaved);
    sGW.write("enemy_defeats", s.enemyDefeats);
    sGW.write("pikmin_thrown", s.pikminThrown);
    sGW.write("whistle_uses", s.whistleUses);
    sGW.write("distance_walked", s.distanceWalked);
    sGW.write("leader_damage_suffered", s.leaderDamageSuffered);
    sGW.write("punch_damage_caused", s.punchDamageCaused);
    sGW.write("leader_kos", s.leaderKos);
    sGW.write("sprays_used", s.spraysUsed);
    
    statsFile.saveFile(FILE_PATHS_FROM_ROOT::STATISTICS, true, true, true);
}


/**
 * @brief Sets the width of all string tokens in a vector of tokens.
 *
 * @param tokens Vector of tokens to set the widths of.
 * @param textFont Text font.
 * @param controlFont Font for control bind icons.
 * @param maxControlBitmapHeight If bitmap icons need to be condensed
 * vertically to fit a certain space, then their width will be affected too.
 * Specify the maximum height here. Use 0 to indicate no maximum height.
 * @param controlCondensed If true, control bind player icons are condensed.
 */
void setStringTokenWidths(
    vector<StringToken>& tokens,
    const ALLEGRO_FONT* textFont, const ALLEGRO_FONT* controlFont,
    float maxControlBitmapHeight, bool controlCondensed
) {
    for(size_t t = 0; t < tokens.size(); t++) {
        switch(tokens[t].type) {
        case STRING_TOKEN_CHAR: {
            tokens[t].width =
                al_get_text_width(textFont, tokens[t].content.c_str());
            break;
        } case STRING_TOKEN_BIND_INPUT: {
            tokens[t].content = trimSpaces(tokens[t].content);
            tokens[t].width =
                getPlayerInputIconWidth(
                    controlFont,
                    game.controls.findBind(tokens[t].content).inputSource,
                    controlCondensed,
                    maxControlBitmapHeight
                );
        }
        default: {
            break;
        }
        }
    }
}


/**
 * @brief Handles a system signal.
 *
 * @param signum Signal number.
 */
void signalHandler(int signum) {
    volatile static bool alreadyHandlingSignal = false;
    
    if(alreadyHandlingSignal) {
        //This stops an infinite loop if there's a signal raise
        //inside this function. It shouldn't happen, but better be safe.
        exit(signum);
    }
    alreadyHandlingSignal = true;
    
    string btStr = "Backtrace:\n";
    vector<string> bt = getBacktrace();
    for(size_t s = 0; s < bt.size(); s++) {
        btStr += "    " + bt[s] + "\n";
    }
    if(btStr.back() == '\n') {
        btStr.pop_back();
    }
    string signalName(strsignal(signum));
    string typeStr = "Signal " + i2s(signum) + " (" + signalName + ")";
    
    crash(typeStr, btStr, signum);
}


/**
 * @brief Spews out a Pikmin from a given point. Used by Onions and converters.
 *
 * @param pos Point of origin.
 * @param z Z of the point of origin.
 * @param pikType Type of the Pikmin to spew out.
 * @param angle Direction in which to spew.
 * @param horizontalSpeed Horizontal speed in which to spew.
 * @param verticalSpeed Vertical speed in which to spew.
 */
void spewPikminSeed(
    const Point pos, float z, PikminType* pikType,
    float angle, float horizontalSpeed, float verticalSpeed
) {
    Pikmin* newPikmin =
        (
            (Pikmin*)
            createMob(
                game.mobCategories.get(MOB_CATEGORY_PIKMIN),
                pos, pikType, angle, "", nullptr, PIKMIN_STATE_SEED
            )
        );
    newPikmin->z = z;
    newPikmin->speed.x = cos(angle) * horizontalSpeed;
    newPikmin->speed.y = sin(angle) * horizontalSpeed;
    newPikmin->speedZ = verticalSpeed;
    newPikmin->maturity = 0;
}


/**
 * @brief Splits a long string, composed of string tokens,
 * into different line breaks, such that no line goes over the limit
 * unless necessary.
 *
 * @param tokens Tokens that make up the string.
 * @param maxWidth Maximum width of each line.
 * @return The lines.
 */
vector<vector<StringToken> > splitLongStringWithTokens(
    const vector<StringToken>& tokens, int maxWidth
) {
    vector<vector<StringToken> > tokensPerLine;
    if(tokens.empty()) return tokensPerLine;
    
    tokensPerLine.push_back(vector<StringToken>());
    size_t curLineIdx = 0;
    unsigned int caret = 0;
    vector<StringToken> wordBuffer;
    unsigned int wordBufferWidth = 0;
    
    for(size_t t = 0; t < tokens.size() + 1; t++) {
    
        bool tokenIsSpace =
            t != tokens.size() &&
            tokens[t].type == STRING_TOKEN_CHAR && tokens[t].content == " ";
        bool tokenIsLineBreak =
            t != tokens.size() &&
            tokens[t].type == STRING_TOKEN_LINE_BREAK;
            
        if(t == tokens.size() || tokenIsSpace || tokenIsLineBreak) {
            //Found a point where we can end a word.
            
            int caretAfterWord = caret + wordBufferWidth;
            bool lineWillBeTooLong =
                caret > 0 && caretAfterWord > maxWidth;
                
            if(lineWillBeTooLong) {
                //Break to a new line before comitting the word.
                tokensPerLine.push_back(vector<StringToken>());
                caret = 0;
                curLineIdx++;
                
                //Remove the previous line's trailing space, if any.
                StringToken& prevTail =
                    tokensPerLine[curLineIdx - 1].back();
                if(
                    prevTail.type == STRING_TOKEN_CHAR &&
                    prevTail.content == " "
                ) {
                    tokensPerLine[curLineIdx - 1].pop_back();
                }
            }
            
            //Add the word to the current line.
            if(t < tokens.size()) {
                wordBuffer.push_back(tokens[t]);
                wordBufferWidth += tokens[t].width;
            }
            tokensPerLine[curLineIdx].insert(
                tokensPerLine[curLineIdx].end(),
                wordBuffer.begin(), wordBuffer.end()
            );
            caret += wordBufferWidth;
            wordBuffer.clear();
            wordBufferWidth = 0;
            
            if(tokenIsLineBreak) {
                //Break the line after comitting the word.
                tokensPerLine.push_back(vector<StringToken>());
                caret = 0;
                curLineIdx++;
            }
            
            continue;
            
        }
        
        //Add the token to the word buffer.
        wordBuffer.push_back(tokens[t]);
        wordBufferWidth += tokens[t].width;
    }
    
    return tokensPerLine;
}


/**
 * @brief Sets up a typical particle generator called from code.
 *
 * @param internalName Internal name of the particle generator to make use of
 * in the game's content.
 * @param targetMob Mob to follow and such.
 * @return The prepared particle generator.
 */
ParticleGenerator standardParticleGenSetup(
    const string& internalName, Mob* targetMob
) {
    ParticleGenerator pg =
        game.content.particleGens.list[internalName];
    pg.restartTimer();
    pg.followMob = targetMob;
    pg.followAngle = targetMob ? &targetMob->angle : nullptr;
    pg.followZOffset =
        targetMob ? targetMob->getDrawingHeight() + 1.0f : 0.0f;
    return pg;
}


/**
 * @brief Starts the display of a text message.
 *
 * If the text is empty, it closes the message box.
 * Any newline characters or slashes followed by n ("\n") will be used to
 * separate the message into lines.
 *
 * @param text Text to display.
 * @param speakerBmp Bitmap representing the speaker.
 */
void startGameplayMessage(const string& text, ALLEGRO_BITMAP* speakerBmp) {
    if(!text.empty()) {
        string finalText = unescapeString(text);
        game.states.gameplay->msgBox =
            new GameplayMessageBox(finalText, speakerBmp);
        for(Player& player : game.states.gameplay->players) {
            player.hud->gui.startAnimation(
                GUI_MANAGER_ANIM_IN_TO_OUT,
                GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
            );
        }
    } else {
        delete game.states.gameplay->msgBox;
        game.states.gameplay->msgBox = nullptr;
        for(Player& player : game.states.gameplay->players) {
            player.hud->gui.startAnimation(
                GUI_MANAGER_ANIM_OUT_TO_IN,
                GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
            );
        }
    }
}


/**
 * @brief Returns the tokens that make up a string.
 * This does not set the tokens's width.
 *
 * @param s String to tokenize.
 * @return The tokens.
 */
vector<StringToken> tokenizeString(const string& s) {
    vector<StringToken> tokens;
    StringToken curToken;
    curToken.type = STRING_TOKEN_CHAR;
    
    for(size_t c = 0; c < s.size(); c++) {
        if(strPeek(s, c, "\\\\")) {
            curToken.content.push_back('\\');
            if(curToken.type == STRING_TOKEN_CHAR) {
                tokens.push_back(curToken);
                curToken.content.clear();
            }
            c++;
            
        } else if(strPeek(s, c, "\\k")) {
            if(!curToken.content.empty()) tokens.push_back(curToken);
            curToken.content.clear();
            if(curToken.type != STRING_TOKEN_BIND_INPUT) {
                curToken.type = STRING_TOKEN_BIND_INPUT;
            } else {
                curToken.type = STRING_TOKEN_CHAR;
            }
            c++;
            
        } else if(s[c] == '\n' || strPeek(s, c, "\\n")) {
            if(!curToken.content.empty()) tokens.push_back(curToken);
            curToken.content.clear();
            curToken.type = STRING_TOKEN_LINE_BREAK;
            tokens.push_back(curToken);
            curToken.type = STRING_TOKEN_CHAR;
            if(s[c] != '\n') c++;
            
        } else {
            curToken.content.push_back(s[c]);
            if(curToken.type == STRING_TOKEN_CHAR) {
                tokens.push_back(curToken);
                curToken.content.clear();
            }
            
        }
    }
    if(!curToken.content.empty()) tokens.push_back(curToken);
    
    return tokens;
}


/**
 * @brief Unescapes a user string. This converts two backslashes into one, and
 * converts backslash n into a newline character.
 *
 * @param s String to unescape.
 * @return The unescaped string.
 */
string unescapeString(const string& s) {
    if(s.empty()) return s;
    
    string ret;
    ret.reserve(s.size());
    for(size_t c = 0; c < s.size() - 1;) {
        if(s[c] == '\\') {
            switch(s[c + 1]) {
            case 'n': {
                ret.push_back('\n');
                c += 2;
                break;
            } case '\\': {
                ret.push_back('\\');
                c += 2;
                break;
            } default: {
                ret.push_back('\\');
                c++;
                break;
            }
            }
        } else {
            ret.push_back(s[c]);
            c++;
        }
    }
    ret.push_back(s.back());
    return ret;
}


/**
 * @brief Convertes a vertex to a point.
 *
 * @param v Vertex to convert.
 * @return The point.
 */
Point v2p(const Vertex* v) {
    return Point(v->x, v->y);
}
