/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the area class and related functions.
 */

#pragma once

#include <memory>

#include "../../core/pathing.h"
#include "../../util/drawing_utils.h"
#include "../../util/general_utils.h"
#include "../content.h"
#include "../other/weather.h"
#include "mission.h"
#include "sector.h"


//Types of areas that can be played.
enum AREA_TYPE {

    //A simple area with no goal.
    AREA_TYPE_SIMPLE,
    
    //An area that likely has a goal, constraints, and/or scoring.
    AREA_TYPE_MISSION,
    
    //Total number of area types.
    N_AREA_TYPES,
    
};


namespace AREA {
extern const float DEF_DAY_TIME_SPEED;
extern const size_t DEF_DAY_TIME_START;
extern const unsigned char DEF_DIFFICULTY;
};


/**
 * @brief Info about dividing the area in a grid.
 *
 * The blockmap divides the entire area
 * in a grid, so that collision detections only
 * happen between stuff in the same grid cell.
 * This is to avoid having, for instance, a Pikmin
 * on the lake part of TIS check for collisions with
 * a wall on the landing site part of TIS.
 * It's also used when checking sectors in a certain spot.
 */
struct Blockmap {

    //--- Public members ---
    
    //Top-left corner of the blockmap.
    Point topLeftCorner;
    
    //Specifies a list of edges in each block.
    vector<vector<vector<Edge*> > > edges;
    
    //Specifies a list of sectors in each block.
    vector<vector<unordered_set<Sector*> > >  sectors;
    
    //Number of columns.
    size_t nCols = 0;
    
    //Number of rows.
    size_t nRows = 0;
    
    
    //--- Public function declarations ---
    
    size_t getCol(float x) const;
    size_t getRow(float y) const;
    bool getEdgesInRegion(
        const Point& tl, const Point& br, set<Edge*>& edges
    ) const;
    Point getTopLeftCorner(size_t col, size_t row) const;
    void clear();
    
};


/**
 * @brief Info for a mob's generation.
 *
 * It is a mob's representation on the editor and in the area file,
 * but it doesn't have the data of a LIVING mob. This only holds its
 * position and type data, plus some other tiny things.
 */
struct MobGen {

    //--- Public members ---
    
    //Mob type.
    MobType* type = nullptr;
    
    //Position.
    Point pos;
    
    //Angle.
    float angle = 0.0f;
    
    //Is it a boss encounter?
    bool isBoss = false;
    
    //Script vars.
    string vars;
    
    //Indexes of linked objects.
    vector<size_t> linkIdxs;
    
    //Index to the mob storing this one inside, if any.
    size_t storedInside = INVALID;
    
    //Linked objects. Cache for performance.
    vector<MobGen*> links;
    
    
    //--- Public function declarations ---
    
    explicit MobGen(
        const Point& pos = Point(),
        MobType* type = nullptr, float angle = 0, const string& vars = "",
        bool boss = false
    );
    void clone(MobGen* destination, bool includePosition = true) const;
    
};


/**
 * @brief Info about the shadows cast onto the area by a tree
 * (or whatever the game maker desires).
 */
struct TreeShadow {

    //--- Public members ---
    
    //Internal name of the tree shadow texture.
    string bmpName;
    
    //Tree shadow texture.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //Positional data.
    Pose2d pose;
    
    //Opacity.
    unsigned char alpha = 255;
    
    //Swaying is multiplied by this.
    Point sway;
    
    
    //--- Public function declarations ---
    
    explicit TreeShadow(
        const Point& center = Point(), const Point& size = Point(100.0f),
        float angle = 0, unsigned char alpha = 255,
        const string& bmpName = "", const Point& sway = Point(1.0f)
    );
    ~TreeShadow();
    
};


/**
 * @brief A rectangular region within the area. Used for all sorts of purposes.
 */
struct AreaRegion {

    //--- Public members ---

    //Center.
    Point center;

    //Size.
    Point size;
    
};


/**
 * @brief Info about an area.
 *
 * This structure is so that the sectors know how to communicate with
 * the edges, the edges with the vertexes, etc.
 */
struct Area : public Content {

    //--- Public members ---
    
    //Type of area.
    AREA_TYPE type = AREA_TYPE_SIMPLE;
    
    //Blockmap.
    Blockmap bmap;
    
    //List of vertexes.
    vector<Vertex*> vertexes;
    
    //List of edges.
    vector<Edge*> edges;
    
    //List of sectors.
    vector<Sector*> sectors;
    
    //List of mob generators.
    vector<MobGen*> mobGenerators;
    
    //List of path stops.
    vector<PathStop*> pathStops;
    
    //List of tree shadows.
    vector<TreeShadow*> treeShadows;

    //List of regions.
    vector<AreaRegion*> regions;
    
    //Bitmap of the background.
    ALLEGRO_BITMAP* bgBmp = nullptr;
    
    //Internal name of the background bitmap.
    string bgBmpName;
    
    //Zoom the background by this much.
    float bgBmpZoom = 1.0f;
    
    //How far away the background is.
    float bgDist = 2.0f;
    
    //Tint the background with this color.
    ALLEGRO_COLOR bgColor = COLOR_BLACK;
    
    //Area subtitle, if any.
    string subtitle;
    
    //Thumbnail, if any.
    std::shared_ptr<ALLEGRO_BITMAP> thumbnail = nullptr;
    
    //Area difficulty, if applicable. [1 - 5], or 0 for undefined.
    unsigned char difficulty = AREA::DEF_DIFFICULTY;
    
    //String representing the starting amounts of each spray.
    string sprayAmounts;
    
    //Song to play.
    string songName;
    
    //Weather condition to use.
    Weather weatherCondition;
    
    //Name of the weather condition to use.
    string weatherName;
    
    //Area day time at the start of gameplay. This is in minutes.
    size_t dayTimeStart = AREA::DEF_DAY_TIME_START;
    
    //Area day time speed, in game-minutes per real-minutes.
    float dayTimeSpeed = AREA::DEF_DAY_TIME_SPEED;
    
    //Known geometry problems.
    GeometryProblems problems;
    
    //Mission data.
    MissionData mission;
    
    //Mission data.
    MissionDataOld missionOld;
    
    //Path to the user data folder for this area.
    string userDataPath;
    
    
    //--- Public function declarations ---
    
    void checkStability();
    void cleanup(bool* outdeleted_sectors = nullptr);
    void clone(Area& other);
    void connectEdgeToSector(Edge* ePtr, Sector* sPtr, size_t side);
    void connectEdgeToVertex(Edge* ePtr, Vertex* vPtr, size_t endpoint);
    void connectSectorEdges(Sector* sPtr);
    void connectVertexEdges(Vertex* vPtr);
    size_t findEdgeIdx(const Edge* ePtr) const;
    size_t findMobGenIdx(const MobGen* mPtr) const;
    size_t findSectorIdx(const Sector* sPtr) const;
    size_t findVertexIdx(const Vertex* vPtr) const;
    void fixEdgeIdxs(Edge* ePtr);
    void fixEdgePointers(Edge* ePtr);
    void fixPathStopIdxs(PathStop* sPtr);
    void fixPathStopPointers(PathStop* sPtr);
    void fixSectorIdxs(Sector* sPtr);
    void fixSectorPointers(Sector* sPtr);
    void fixVertexIdxs(Vertex* vPtr);
    void fixVertexPointers(Vertex* vPtr);
    void generateBlockmap();
    void generateEdgesBlockmap(const vector<Edge*>& edges);
    size_t getNrPathLinks();
    void loadMainDataFromDataNode(
        DataNode* node, CONTENT_LOAD_LEVEL level
    );
    void loadMissionDataFromDataNode(DataNode* node);
    void loadGeometryFromDataNode(
        DataNode* node, CONTENT_LOAD_LEVEL level
    );
    void loadThumbnail(const string& thumbnailPath);
    Edge* newEdge();
    Sector* newSector();
    Vertex* newVertex();
    void removeVertex(size_t vIdx);
    void removeVertex(const Vertex* vPtr);
    void removeEdge(size_t eIdx);
    void removeEdge(const Edge* ePtr);
    void removeSector(size_t sIdx);
    void removeSector(const Sector* sPtr);
    void saveGeometryToDataNode(DataNode* node);
    void saveMainDataToDataNode(DataNode* node);
    void saveMissionDataToDataNode(DataNode* node);
    void saveThumbnail(bool toBackup);
    void clear();
    
};
