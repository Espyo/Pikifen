/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general area editor-related functions.
 */

#pragma once

#include <deque>

#include "../editor.h"

#include "../../content/area/area.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../util/general_utils.h"


using std::deque;
using std::size_t;
using std::string;


namespace AREA_EDITOR {
extern const ALLEGRO_COLOR BLOCKING_COLOR;
extern const float COMFY_DIST;
extern const float CROSS_SECTION_POINT_RADIUS;
extern const float CURSOR_SNAP_DISTANCE;
extern const float CURSOR_SNAP_UPDATE_INTERVAL;
extern const float DEBUG_TEXT_SCALE;
extern const unsigned char DEF_REFERENCE_ALPHA;
extern const float KEYBOARD_PAN_AMOUNT;
extern const unsigned char MAX_CIRCLE_SECTOR_POINTS;
extern const float MAX_GRID_INTERVAL;
extern const size_t MAX_TEXTURE_SUGGESTIONS;
extern const ALLEGRO_COLOR MEASUREMENT_COLOR;
extern const unsigned char MIN_CIRCLE_SECTOR_POINTS;
extern const float MIN_GRID_INTERVAL;
extern const float MOB_LINK_THICKNESS;
extern const float NEW_SECTOR_ERROR_TINT_DURATION;
extern const ALLEGRO_COLOR NON_BLOCKING_COLOR;
extern const float PATH_LINK_THICKNESS;
extern const float PATH_PREVIEW_CHECKPOINT_RADIUS;
extern const float PATH_PREVIEW_TIMER_DUR;
extern const float POINT_LETTER_TEXT_SCALE;
extern const float REFERENCE_MIN_SIZE;
extern const float QUICK_PREVIEW_DURATION;
extern const unsigned char SELECTION_COLOR[3];
extern const float SELECTION_EFFECT_SPEED;
extern const float SELECTION_TW_PADDING;
extern const float UNDO_SAVE_LOCK_DURATION;
extern const float VERTEX_MERGE_RADIUS;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the area editor.
 */
class AreaEditor : public Editor {

public:

    //--- Misc. declarations ---
    
    //Ways for the cursor to snap.
    enum SNAP_MODE {
    
        //Snap to grid.
        SNAP_MODE_GRID,
        
        //Snap to vertexes.
        SNAP_MODE_VERTEXES,
        
        //Snap to edges.
        SNAP_MODE_EDGES,
        
        //Snap to nothing.
        SNAP_MODE_NOTHING,
        
        //Total amount of snap modes.
        N_SNAP_MODES,
        
    };
    
    //Ways for the area to be viewed in-editor.
    enum VIEW_MODE {
    
        //Textures.
        VIEW_MODE_TEXTURES,
        
        //Wireframe.
        VIEW_MODE_WIREFRAME,
        
        //Heightmap.
        VIEW_MODE_HEIGHTMAP,
        
        //Brightness map.
        VIEW_MODE_BRIGHTNESS,
        
        //Total amount of view modes.
        N_VIEW_MODES,
        
    };
    
    
    //--- Members ---
    
    //Automatically load this folder upon boot-up of the editor, if any.
    string autoLoadFolder;
    
    //Area being edited when using the quick-play button.
    string quickPlayAreaPath;
    
    //Position the camera was it in the editor before quick-play.
    Point quickPlayCamPos;
    
    //Editor camera zoom before quick-play.
    float quickPlayCamZ = 1.0f;
    
    //This hack fixes a glitch by skipping drawing for one frame.
    bool hackSkipDrawing = false;
    
    
    //--- Function declarations ---
    
    AreaEditor();
    void doLogic() override;
    void doDrawing() override;
    void load() override;
    void unload() override;
    string getName() const override;
    void drawCanvas();
    string getOpenedContentPath() const;
    
private:

    /**
     * @brief Represents a suggested texture.
     */
    struct TextureSuggestion {
    
        //--- Members ---
        
        //Bitmap of the texture.
        ALLEGRO_BITMAP* bmp = nullptr;
        
        //Internal name of the texture.
        string name;
        
        
        //--- Function declarations ---
        
        explicit TextureSuggestion(const string &n);
        void destroy();
        
    };
    
    /**
     * @brief Represents a point in the current layout drawing.
     */
    struct LayoutDrawingNode {
    
        //--- Members ---
        
        //Raw coordinates of the mouse click.
        Point rawSpot;
        
        //Final spot of the node, after snapping to an existing vertex/edge.
        Point snappedSpot;
        
        //Is this node on top of an existing vertex? This points to it if so.
        Vertex* onVertex = nullptr;
        
        //onVertex's vertex index.
        size_t onVertexIdx = INVALID;
        
        //Is this node on top of an existing edge? This points to it if so.
        Edge* onEdge = nullptr;
        
        //onEdge's edge index.
        size_t onEdgeIdx = INVALID;
        
        //Is this node just on top of a sector? This points to it if so.
        Sector* onSector = nullptr;
        
        //onSector's sector index.
        size_t onSectorIdx = INVALID;
        
        //Is onVertex a new vertex, created during the sector creation?
        bool isNewVertex = false;
        
        
        //--- Function declarations ---
        
        LayoutDrawingNode(
            const AreaEditor* aePtr, const Point &mouseClick
        );
        LayoutDrawingNode();
        
    };
    
    /**
     * @brief Info pertaining to a sector split operation.
     */
    struct SectorSplit {
    
        //--- Members ---
        
        //Area data from before the split.
        Area* preSplitAreaData = nullptr;
        
        //Sector being worked on in a sector split operation.
        Sector* workingSector = nullptr;
        
        //Edges of the sector split sector, before the split operation.
        vector<Edge*> workingSectorOldEdges;
        
        //Edges traversed in each step.
        vector<Edge*> traversedEdges[2];
        
        //Vertexes traversed in each step.
        vector<Vertex*> traversedVertexes[2];
        
        //During stage 1, was the working sector to the left?
        bool isWorkingAtStage1Left = false;
        
        //Number of drawing nodes before a useless split part 2. Or INVALID.
        size_t uselessSplitPart2Checkpoint = INVALID;
        
    };

    //Style of the different things to draw in the canvas.
    struct AreaEdCanvasStyle : CanvasStyle {

        //Texture alpha.
        float textureAlpha = 1.0f;

        //Wall shadow alpha.
        float wallShadowAlpha = 1.0f;

        //Edge line alpha.
        float edgeAlpha = 1.0f;

        //Mob alpha.
        float mobAlpha = 1.0f;

        //Z of the lowest sector.
        float lowestSectorZ = 0.0f;

        //Z of the highest sector.
        float highestSectorZ = 0.0f;

    };
    
    //Possible results after a line drawing operation.
    enum DRAWING_LINE_RESULT {
    
        //No error.
        DRAWING_LINE_RESULT_OK,
        
        //Hit an existing edge or vertex when drawing a new sector.
        DRAWING_LINE_RESULT_HIT_EDGE_OR_VERTEX,
        
        //Trying to draw along an existing edge.
        DRAWING_LINE_RESULT_ALONG_EDGE,
        
        //Crosses exsting edges.
        DRAWING_LINE_RESULT_CROSSES_EDGES,
        
        //Crosses previous parts of the drawing.
        DRAWING_LINE_RESULT_CROSSES_DRAWING,
        
        //Goes towards a sector different from the working sector.
        DRAWING_LINE_RESULT_WAYWARD_SECTOR,
        
    };
    
    //Possible errors for a sector split operation.
    enum SECTOR_SPLIT_RESULT {
    
        //Ok to split.
        SECTOR_SPLIT_RESULT_OK,
        
        //The split is invalid.
        SECTOR_SPLIT_RESULT_INVALID,
        
        //That wouldn't split in any useful way. e.g. Slice through a donut.
        SECTOR_SPLIT_RESULT_USELESS,
        
    };
    
    //Types of problems in the area (Editor Problem Type).
    enum EPT {
    
        //None found so far.
        EPT_NONE_YET,
        
        //No problems.
        EPT_NONE,
        
        //Two edges intersect.
        EPT_INTERSECTING_EDGES,
        
        //An edge is all by itself.
        EPT_LONE_EDGE,
        
        //Two vertexes in the same spot.
        EPT_OVERLAPPING_VERTEXES,
        
        //A sector is corrupted.
        EPT_BAD_SECTOR,
        
        //No leader mob found.
        EPT_MISSING_LEADER,
        
        //A texture is not found in the game content.
        EPT_UNKNOWN_TEXTURE,
        
        //Mob with no type.
        EPT_TYPELESS_MOB,
        
        //Mob out of bounds.
        EPT_MOB_OOB,
        
        //Mob stuck in a wall.
        EPT_MOB_IN_WALL,
        
        //Mob links to itself.
        EPT_MOB_LINKS_TO_SELF,
        
        //Mobs stored in a loop.
        EPT_MOB_STORED_IN_LOOP,
        
        //Pikmin amount goes over the limit.
        EPT_PIKMIN_OVER_LIMIT,
        
        //Bridge mob missing a bridge sector.
        EPT_SECTORLESS_BRIDGE,
        
        //A path stop is all by itself.
        EPT_LONE_PATH_STOP,
        
        //A path stop is out of bounds.
        EPT_PATH_STOP_OOB,
        
        //Two path stops are in the same place.
        EPT_PATH_STOPS_TOGETHER,
        
        //A path stop is on top of an unrelated link.
        EPT_PATH_STOP_ON_LINK,
        
        //Bridge blocks the path from pile to it.
        EPT_PILE_BRIDGE_PATH,
        
        //Unknown tree shadow image.
        EPT_UNKNOWN_SHADOW,
        
        //No active score criteria for this mission.
        EPT_NO_SCORE_CRITERIA,
        
        //No mission goal mobs.
        EPT_NO_GOAL_MOBS,
        
    };
    
    //Editor states.
    enum EDITOR_STATE {
    
        //Main menu.
        EDITOR_STATE_MAIN,
        
        //Area info editing.
        EDITOR_STATE_INFO,
        
        //Area gameplay settings editing.
        EDITOR_STATE_GAMEPLAY,
        
        //Layout editing.
        EDITOR_STATE_LAYOUT,
        
        //Mob editing.
        EDITOR_STATE_MOBS,
        
        //Path editing.
        EDITOR_STATE_PATHS,
        
        //Detail editing.
        EDITOR_STATE_DETAILS,
        
        //Review.
        EDITOR_STATE_REVIEW,
        
        //Tools.
        EDITOR_STATE_TOOLS,
        
    };
    
    //Editor sub-states.
    enum EDITOR_SUB_STATE {
    
        //None.
        EDITOR_SUB_STATE_NONE,
        
        //Picking a mission exit region.
        EDITOR_SUB_STATE_MISSION_EXIT,
        
        //Drawing the layout.
        EDITOR_SUB_STATE_DRAWING,
        
        //Drawing a circular sector.
        EDITOR_SUB_STATE_CIRCLE_SECTOR,
        
        //On-canvas texture effect editing.
        EDITOR_SUB_STATE_OCTEE,
        
        //Quick sector height set.
        EDITOR_SUB_STATE_QUICK_HEIGHT_SET,
        
        //Adding a new mob.
        EDITOR_SUB_STATE_NEW_MOB,
        
        //Duplicating a mob.
        EDITOR_SUB_STATE_DUPLICATE_MOB,
        
        //Storing a mob inside another.
        EDITOR_SUB_STATE_STORE_MOB_INSIDE,
        
        //Adding a mob link.
        EDITOR_SUB_STATE_ADD_MOB_LINK,
        
        //Deleting a mob link.
        EDITOR_SUB_STATE_DEL_MOB_LINK,
        
        //Picking required mobs for the mission.
        EDITOR_SUB_STATE_MISSION_MOBS,
        
        //Drawing paths.
        EDITOR_SUB_STATE_PATH_DRAWING,
        
        //Adding a new tree shadow.
        EDITOR_SUB_STATE_NEW_SHADOW,
        
    };
    
    //On-canvas texture effect editing modes.
    enum OCTEE_MODE {
    
        //Editing texture offset.
        OCTEE_MODE_OFFSET,
        
        //Editing texture scale.
        OCTEE_MODE_SCALE,
        
        //Editing texture angle.
        OCTEE_MODE_ANGLE,
        
    };
    
    //Filters for selecting.
    enum SELECTION_FILTER {
    
        //Select sectors, edges, and vertexes.
        SELECTION_FILTER_SECTORS,
        
        //Select edges and vertexes.
        SELECTION_FILTER_EDGES,
        
        //Select vertexes only.
        SELECTION_FILTER_VERTEXES,
        
        //Total amount of selection filters.
        N_SELECTION_FILTERS,
        
    };
    
    //Layout editing panel modes.
    enum LAYOUT_MODE {
    
        //Sector info.
        LAYOUT_MODE_SECTORS,
        
        //Edge info.
        LAYOUT_MODE_EDGES,
        
    };
    
    
    //--- Members ---
    
    //Time left until a backup is generated.
    Timer backupTimer;
    
    //When the player copies an edge's properties, they go here.
    Edge* copyBufferEdge = nullptr;
    
    //When the player copies a mob's properties, they go here.
    MobGen* copyBufferMob = nullptr;
    
    //When the player copies a path link's properties, they go here.
    PathLink* copyBufferPathLink = nullptr;
    
    //When the player copies a sector's properties, they go here.
    Sector* copyBufferSector = nullptr;
    
    //Where the cross-section tool points are.
    Point crossSectionCheckpoints[2];
    
    //Cross-section window's start coordinates.
    Point crossSectionWindowStart;
    
    //Cross-section window's end coordinates.
    Point crossSectionWindowEnd;
    
    //Cross-section Z legend window's start coordinates.
    Point crossSectionZWindowStart;
    
    //Cross-section Z legend window's end coordinates.
    Point crossSectionZWindowEnd;
    
    //When showing a hazard in the list, this is the index of the current one.
    size_t curHazardIdx = INVALID;
    
    //The current transformation widget.
    TransformationWidget curTransformationWidget;
    
    //Last known cursor snap position for heavy snap modes.
    Point cursorSnapCache;
    
    //Time left to update the cursor snap position for heavy snap modes.
    Timer cursorSnapTimer = Timer(AREA_EDITOR::CURSOR_SNAP_UPDATE_INTERVAL);
    
    //Debug tool -- show the edge indexes?
    bool debugEdgeIdxs = false;
    
    //Debug tool -- show the sector indexes?
    bool debugSectorIdxs = false;
    
    //Debug tool -- show the path indexes?
    bool debugPathIdxs = false;
    
    //Debug tool -- show the triangulation?
    bool debugTriangulation = false;
    
    //Debug tool -- show the vertex indexes?
    bool debugVertexIdxs = false;
    
    //Nodes of the drawing.
    vector<LayoutDrawingNode> drawingNodes;
    
    //Result of the current drawing line.
    DRAWING_LINE_RESULT drawingLineResult = DRAWING_LINE_RESULT_OK;
    
    //Currently highlighted edge, if any.
    Edge* highlightedEdge = nullptr;
    
    //Currently highlighted mob, if any.
    MobGen* highlightedMob = nullptr;
    
    //Currently highlighted path link, if any.
    PathLink* highlightedPathLink = nullptr;
    
    //Currently highlighted path stop, if any.
    PathStop* highlightedPathStop = nullptr;
    
    //Currently highlighted sector, if any.
    Sector* highlightedSector = nullptr;
    
    //Currently highlighted vertex, if any.
    Vertex* highlightedVertex = nullptr;
    
    //Category name of the last mob placed.
    string lastMobCustomCatName;
    
    //Mob type of the last mob placed.
    MobType* lastMobType = nullptr;
    
    //Are we editing sectors or edges?
    LAYOUT_MODE layoutMode = LAYOUT_MODE_SECTORS;
    
    //Picker info for the picker in the "load" dialog.
    Picker loadDialogPicker;
    
    //Closest mob to the mouse when moving.
    MobGen* moveClosestMob = nullptr;
    
    //Closest path stop to the mouse when moving.
    PathStop* moveClosestStop = nullptr;
    
    //Closest vertex to the mouse when moving.
    Vertex* moveClosestVertex = nullptr;
    
    //The moved thing was here when the move started (world coords).
    Point moveStartPos;
    
    //The mouse cursor was here when the move started (world coords).
    Point moveMouseStartPos;
    
    //Currently moving the selected vertexes, objects, etc.?
    bool moving = false;
    
    //Path preview checkpoint that is currently being moved, or -1 for none.
    signed char movingPathPreviewCheckpoint = -1;
    
    //Cross-section point that is currently being moved, or -1 for none.
    signed char movingCrossSectionPoint = -1;
    
    //New circle sector's second point.
    Point newCircleSectorAnchor;
    
    //New circle sector's center.
    Point newCircleSectorCenter;
    
    //Points where the new circle sector's vertexes will end up.
    vector<Point> newCircleSectorPoints;
    
    //What step of the circular sector building process are we in?
    unsigned char newCircleSectorStep = 0;
    
    //For each edge of the new circle sector, is it valid?
    vector<bool> newCircleSectorValidEdges;
    
    //Time left to keep the error-redness of the new sector's line(s) for.
    Timer newSectorErrorTintTimer = Timer(AREA_EDITOR::NEW_SECTOR_ERROR_TINT_DURATION);
    
    //Mouse drag start coordinates, when using on-canvas texture effect editing.
    Point octeeDragStart;
    
    //Texture's original angle, when using on-canvas texture effect editing.
    float octeeOrigAngle = 0.0f;
    
    //Texture's original offset, when using on-canvas texture effect editing.
    Point octeeOrigOffset;
    
    //Texture's original scale, when using on-canvas texture effect editing.
    Point octeeOrigScale;
    
    //Current on-canvas texture effect edit mode.
    OCTEE_MODE octeeMode = OCTEE_MODE_OFFSET;
    
    //When drawing a path, use these stop flags.
    Bitmask8 pathDrawingFlags = 0;
    
    //When drawing a path, use this label.
    string pathDrawingLabel;
    
    //When drawing a path, create normal links. False for one-way links.
    bool pathDrawingNormals = true;
    
    //When drawing a path, use this type.
    PATH_LINK_TYPE pathDrawingType = PATH_LINK_TYPE_NORMAL;
    
    //First stop of the next link when drawing a path.
    PathStop* pathDrawingStop1 = nullptr;
    
    //Path stops that make up the current path preview.
    vector<PathStop*> pathPreview;
    
    //Location of the two path preview checkpoints.
    Point pathPreviewCheckpoints[2];
    
    //The closest stop to the path preview start and end.
    PathStop* pathPreviewClosest[2] = { nullptr, nullptr };
    
    //Total distance of the previewed path.
    float pathPreviewDist = 0.0f;
    
    //Settings for the path preview.
    PathFollowSettings pathPreviewSettings;
    
    //Result of the path preview's calculation.
    PATH_RESULT pathPreviewResult = PATH_RESULT_NOT_CALCULATED;
    
    //Only calculate the preview path when this time is up.
    Timer pathPreviewTimer;
    
    //Area data before vertex movement.
    Area* preMoveAreaData = nullptr;
    
    //Position of the selected mobs before movement.
    map<MobGen*, Point> preMoveMobCoords;
    
    //Position of the selected path stops before movement.
    map<PathStop*, Point> preMoveStopCoords;
    
    //Position of the selected vertexes before movement.
    map<Vertex*, Point> preMoveVertexCoords;
    
    //Is preview mode on?
    bool previewMode = false;
    
    //Name of the area song we're previewing, if any.
    string previewSong;
    
    //Description of the current problem found.
    string problemDescription;
    
    //Information about the problematic intersecting edges, if any.
    EdgeIntersection problemEdgeIntersection = EdgeIntersection(nullptr, nullptr);
    
    //Pointer to the problematic mob, if any.
    MobGen* problemMobPtr = nullptr;
    
    //Pointer to the problematic path stop, if any.
    PathStop* problemPathStopPtr = nullptr;
    
    //Type of the current problem found in the review panel.
    EPT problemType = EPT_NONE_YET;
    
    //Pointer to the problematic sector, if any.
    Sector* problemSectorPtr = nullptr;
    
    //Pointer to the problematic tree shadow, if any.
    TreeShadow* problemShadowPtr = nullptr;
    
    //Title of the current problem found.
    string problemTitle;
    
    //Pointer to the problematic vertex, if any.
    Vertex* problemVertexPtr = nullptr;
    
    //Sector heights when the quick height set mode was entered.
    map<Sector*, float> quickHeightSetStartHeights;
    
    //Mouse coordinates (window) when the quick height set mode was entered.
    Point quickHeightSetStartPos;
    
    //Time left in the quick preview mode, including fade out.
    Timer quickPreviewTimer = Timer(AREA_EDITOR::QUICK_PREVIEW_DURATION);
    
    //Redo history, with the state of the area at each point. Front = latest.
    deque<std::pair<Area*, string> > redoHistory;
    
    //Opacity of the reference image.
    unsigned char referenceAlpha = 255;
    
    //Reference image center.
    Point referenceCenter;
    
    //Reference image dimensions.
    Point referenceSize;
    
    //Bitmap of the reference image.
    ALLEGRO_BITMAP* referenceBitmap = nullptr;
    
    //File path of the reference image.
    string referenceFilePath;
    
    //Keep the aspect ratio when resizing the reference?
    bool referenceKeepAspectRatio = true;
    
    //Info about the current sector split operation.
    SectorSplit sectorSplitInfo;
    
    //Currently selected edges.
    set<Edge*> selectedEdges;
    
    //Currently selected mobs.
    set<MobGen*> selectedMobs;
    
    //Currently selected path links.
    set<PathLink*> selectedPathLinks;
    
    //Currently selected path stops.
    set<PathStop*> selectedPathStops;
    
    //Currently selected sectors.
    set<Sector*> selectedSectors;
    
    //Currently selected tree shadow.
    TreeShadow* selectedShadow = nullptr;
    
    //Keep the aspect ratio of the currently selected shadow?
    bool selectedShadowKeepAspectRatio = false;
    
    //Currently selected vertexes.
    set<Vertex*> selectedVertexes;
    
    //Is the user currently performing a rectangle box?
    bool selecting = false;
    
    //Angle of the selection.
    float selectionAngle = 0.0f;
    
    //Center of the selection.
    Point selectionCenter;
    
    //The selection's alpha depends on this value.
    float selectionEffect = 0.0f;
    
    //Point where the selection is currently at.
    Point selectionEnd;
    
    //Current selection filter.
    SELECTION_FILTER selectionFilter = SELECTION_FILTER_SECTORS;
    
    //Has the user agreed to homogenize the selection?
    bool selectionHomogenized = false;
    
    //Angle of the selection, before it got transformed.
    float selectionOrigAngle = 0.0f;
    
    //Center of the selection, before it got transformed.
    Point selectionOrigCenter;
    
    //Size of the selection, before it got transformed.
    Point selectionOrigSize;
    
    //Size of the selection, padding included.
    Point selectionSize;
    
    //Point where the selection started.
    Point selectionStart;
    
    //Show blocking vs. non-blocking sectors?
    bool showBlockingSectors = false;
    
    //Show the path stop closest to the cursor?
    bool showClosestStop = false;
    
    //Use the cross-section view tool?
    bool showCrossSection = false;
    
    //When using the cross-section view tool, render the grid?
    bool showCrossSectionGrid = true;
    
    //Show the path preview and the checkpoints?
    bool showPathPreview = false;
    
    //Render the reference image?
    bool showReference = true;
    
    //Render the tree shadows?
    bool showShadows = false;
    
    //List of texture suggestions.
    vector<TextureSuggestion> textureSuggestions;
    
    //Position of the load widget.
    Point loadWidgetPos;
    
    //Position of the reload widget.
    Point reloadWidgetPos;
    
    //Position of the quit widget.
    Point quitWidgetPos;
    
    //Was the area's thumbnail changed in any way since the last save?
    bool thumbnailNeedsSaving = false;
    
    //Was the area's thumbnail changed in any way since the last backup save?
    bool thumbnailBackupNeedsSaving = false;
    
    //Undo history, with the state of the area at each point. Front = latest.
    deque<std::pair<Area*, string> > undoHistory;
    
    //Name of the undo operation responsible for the lock.
    string undoSaveLockOperation;
    
    //During this timer, don't save state for operations matching the last one.
    Timer undoSaveLockTimer;
    
    struct {
    
        //Selected pack.
        string pack;
        
        //Internal name of the new area.
        string internalName = "my_area";
        
        //Selected area type.
        int type = AREA_TYPE_SIMPLE;
        
        //Path to the new area.
        string areaPath;
        
        //Last time we checked if the new area path existed, it was this.
        string lastCheckedAreaPath;
        
        //Does a folder already exist under the new area's path?
        bool areaPathExists = false;
        
        //Whether we need to focus on the text input widget.
        bool needsTextFocus = true;
        
    } newDialog;
    
    
    //--- Function declarations ---
    
    bool areNodesTraversable(
        const LayoutDrawingNode &n1,
        const LayoutDrawingNode &n2
    ) const;
    float calculateDaySpeed(
        float dayStartMin, float dayEndMin,
        float missionMin
    );
    void cancelCircleSector();
    void cancelLayoutDrawing();
    void cancelLayoutMoving();
    float calculatePreviewPath();
    void changeState(const EDITOR_STATE newState);
    void checkDrawingLine(const Point &pos);
    void clearCircleSector();
    void clearCurrentArea();
    void clearLayoutDrawing();
    void clearLayoutMoving();
    void clearProblems();
    void clearSelection();
    void clearTextureSuggestions();
    void clearUndoHistory();
    void closeLoadDialog();
    void closeOptionsDialog();
    void copyEdgeProperties();
    void copyMobProperties();
    void copyPathLinkProperties();
    void copySectorProperties();
    void createArea(const string &requestedAreaPath);
    void createDrawingVertexes();
    void createMobUnderCursor();
    Sector* createSectorForLayoutDrawing(const Sector* copyFrom);
    void deleteCurrentArea();
    void deleteEdge(Edge* ePtr);
    bool deleteEdges(const set<Edge*> &which);
    void deleteMobs(const set<MobGen*> &which);
    void deletePathLinks(const set<PathLink*> &which);
    void deletePathStops(const set<PathStop*> &which);
    void doSectorSplit();
    void emitTriangulationErrorStatusBarMessage(
        const TRIANGULATION_ERROR error
    );
    string findGoodFirstTexture();
    void findProblems();
    void findProblemsBridgePath();
    void findProblemsIntersectingEdge();
    void findProblemsLoneEdge();
    void findProblemsLonePathStop();
    void findProblemsMissingLeader();
    void findProblemsMissingTexture();
    void findProblemsMobInsideWalls();
    void findProblemsMobLinksToSelf();
    void findProblemsMobStoredInLoop();
    void findProblemsNoGoalMob();
    void findProblemsNoScoreCriteria();
    void findProblemsNonSimpleSector();
    void findProblemsOobMob();
    void findProblemsOobPathStop();
    void findProblemsOverlappingVertex();
    void findProblemsPathStopOnLink();
    void findProblemsPathStopsIntersecting();
    void findProblemsPikminOverLimit();
    void findProblemsTypelessMob();
    void findProblemsUnknownTexture();
    void findProblemsUnknownTreeShadow();
    void finishCircleSector();
    void finishLayoutMoving();
    void finishNewSectorDrawing();
    void forgetPreparedState(Area* preparedChange);
    void getAffectedSectors(
        Sector* sPtr, unordered_set<Sector*> &list
    ) const;
    void getAffectedSectors(
        const set<Sector*> &sectors, unordered_set<Sector*> &list
    ) const;
    void getAffectedSectors(
        const set<Vertex*> &vertexes, unordered_set<Sector*> &list
    ) const;
    void getHoveredLayoutElement(
        Vertex** clickedVertex, Edge** clickedEdge, Sector** clickedSector
    ) const;
    Edge* getClosestEdgeToAngle(
        Vertex* vPtr, float angle, bool clockwise,
        float* outClosestEdgeAngle
    ) const;
    bool getCommonSector(
        vector<Vertex*> &vertexes, vector<Edge*> &edges, Sector** result
    ) const;
    Edge* getCorrectPostSplitEdge(
        const Vertex* vPtr, Edge* e1Ptr, Edge* e2Ptr
    ) const;
    bool getDrawingOuterSector(Sector** result) const;
    Edge* getEdgeUnderPoint(
        const Point &p, const Edge* after = nullptr
    ) const;
    string getFolderTooltip(
        const string &path, const string &userDataPath
    ) const;
    vector<EdgeIntersection> getIntersectingEdges() const;
    size_t getMissionRequiredMobCount() const;
    float getMobGenRadius(MobGen* m) const;
    bool getMobLinkUnderPoint(
        const Point &p,
        std::pair<MobGen*, MobGen*>* data1,
        std::pair<MobGen*, MobGen*>* data2
    ) const;
    MobGen* getMobUnderPoint(
        const Point &p, size_t* outIdx = nullptr
    ) const;
    bool getPathLinkUnderPoint(
        const Point &p, PathLink** link1, PathLink** link2
    ) const;
    PathStop* getPathStopUnderPoint(const Point &p) const;
    float getQuickHeightSetOffset() const;
    SECTOR_SPLIT_RESULT getSectorSplitEvaluation();
    Sector* getSectorUnderPoint(const Point &p) const;
    Vertex* getVertexUnderPoint(const Point &p) const;
    void goToUndoHistoryPoint(size_t p);
    void goToProblem();
    void handleLineError();
    void homogenizeSelectedEdges();
    void homogenizeSelectedMobs();
    void homogenizeSelectedPathLinks();
    void homogenizeSelectedPathStops();
    void homogenizeSelectedSectors();
    void loadAreaFolder(
        const string &requestedAreaPath,
        bool fromBackup, bool shouldUpdateHistory
    );
    void loadBackup();
    void loadReference();
    bool mergeSectors(Sector* s1, Sector* s2);
    void mergeVertex(
        const Vertex* v1, Vertex* v2, unordered_set<Sector*>* affectedSectors
    );
    void pasteEdgeProperties();
    void pasteMobProperties();
    void pastePathLinkProperties();
    void pasteSectorProperties();
    void pasteSectorTexture();
    Area* prepareState();
    void recreateDrawingNodes();
    void redo();
    void registerChange(
        const string &operationName, Area* prePreparedChange = nullptr
    );
    void reloadAreas();
    void removeThumbnail();
    void resizeEverything(float mults[2]);
    void rollbackToPreparedState(Area* preparedState);
    void rotateMobGensToPoint(const Point &pos);
    bool saveArea(bool toBackup);
    void saveBackup();
    void saveReference();
    void selectEdge(Edge* ePtr);
    void selectPathStopsWithLabel(const string &label);
    void selectSector(Sector* sPtr);
    void selectTreeShadow(TreeShadow* sPtr);
    void selectVertex(Vertex* vPtr);
    void setNewCircleSectorPoints();
    void setSelectionStatusText();
    void setStateFromUndoOrRedoHistory(Area* state);
    void setupForNewAreaPost();
    void setupForNewAreaPre();
    void setupSectorSplit();
    Point snapPoint(const Point &p, bool ignoreSelected = false);
    Vertex* splitEdge(Edge* ePtr, const Point &where);
    PathStop* splitPathLink(
        PathLink* l1, PathLink* l2,
        const Point &where
    );
    void startMobMove();
    void startPathStopMove();
    void startVertexMove();
    void traverseSectorForSplit(
        const Sector* sPtr, Vertex* begin, const Vertex* checkpoint,
        vector<Edge*>* edges, vector<Vertex*>* vertexes,
        bool* workingSectorLeft
    );
    void undo();
    void undoLayoutDrawingNode();
    void updateAffectedSectors(
        const unordered_set<Sector*> &affectedSectors
    );
    void updateAllEdgeOffsetCaches();
    void updateInnerSectorsOuterSector(
        const vector<Edge*> &edgesToCheck,
        const Sector* oldOuter, Sector* newOuter
    );
    void updateReference();
    void updateLayoutDrawingStatusText();
    void updateSectorTexture(Sector* sPtr, const string &internalName);
    void updateTextureSuggestions(const string &n);
    void updateUndoHistory();
    void updateVertexSelection();
    void drawArrow(
        const Point &start, const Point &end,
        float startOffset, float endOffset,
        float thickness, const ALLEGRO_COLOR &color
    );
    static void drawCanvasDearImGuiCallback(
        const ImDrawList* parentList, const ImDrawCmd* cmd
    );
    void drawCrossSectionGraph();
    void drawCrossSectionSector(
        float startRatio, float endRatio, float proportion,
        float lowestZ, const Sector* sectorPtr
    );
    void drawDebugText(
        const ALLEGRO_COLOR color, const Point &where, const string &text,
        unsigned char dots = 0
    );
    void drawEdges(const AreaEdCanvasStyle& style);
    void drawLineDist(
        const Point &focus, const Point &other, const string &prefix = ""
    );
    void drawMobs(const AreaEdCanvasStyle& style);
    void drawPaths(const AreaEdCanvasStyle& style);
    void drawSectors(const AreaEdCanvasStyle& style);
    void drawTreeShadows(const AreaEdCanvasStyle& style);
    void drawVertexes(const AreaEdCanvasStyle& style);
    void openLoadDialog();
    void openNewDialog();
    void openOptionsDialog();
    void pickAreaFolder(
        const string &name, const string &topCat, const string &secCat,
        void* info, bool isNew
    );
    void pickTexture(
        const string &name, const string &topCat, const string &secCat,
        void* info, bool isNew
    );
    void circleSectorCmd(float inputValue);
    void copyPropertiesCmd(float inputValue);
    void deleteAreaCmd(float inputValue);
    void deleteCmd(float inputValue);
    void deleteEdgeCmd(float inputValue);
    void deleteMobCmd(float inputValue);
    void deletePathCmd(float inputValue);
    void deleteTreeShadowCmd(float inputValue);
    void duplicateMobsCmd(float inputValue);
    void gridIntervalDecreaseCmd(float inputValue);
    void gridIntervalIncreaseCmd(float inputValue);
    void layoutDrawingCmd(float inputValue);
    void loadCmd(float inputValue);
    void newMobCmd(float inputValue);
    void newPathCmd(float inputValue);
    void newTreeShadowCmd(float inputValue);
    void pastePropertiesCmd(float inputValue);
    void pasteTextureCmd(float inputValue);
    void quickPlayCmd(float inputValue);
    void quitCmd(float inputValue);
    void redoCmd(float inputValue);
    void referenceToggleCmd(float inputValue);
    void reloadCmd(float inputValue);
    void saveCmd(float inputValue);
    void selectAllCmd(float inputValue);
    void selectionFilterCmd(float inputValue);
    void snapModeCmd(float inputValue);
    void undoCmd(float inputValue);
    void zoomAndPosResetCmd(float inputValue);
    void zoomEverythingCmd(float inputValue);
    void zoomInCmd(float inputValue);
    void zoomOutCmd(float inputValue);
    void processGui();
    void processGuiControlPanel();
    void processGuiDeleteAreaDialog();
    void processGuiGradingCriterionWidgets(
        int* valuePtr, MISSION_SCORE_CRITERIA criterionIdx,
        const string &widgetLabel, const string &tooltip
    );
    void processGuiGradingMedalWidgets(
        int* requirementPtr, const string &widgetLabel,
        int widgetMinValue, int widgetMaxValue,
        const string &tooltip
    );
    void processGuiGradingModeWidgets(
        int value, const string &widgetLabel, const string &tooltip
    );
    void processGuiLoadDialog();
    void processGuiNewDialog();
    void processGuiMenuBar();
    void processGuiMobScriptVars(MobGen* gen);
    void processGuiPanelDetails();
    void processGuiPanelEdge();
    void processGuiPanelGameplay();
    void processGuiPanelInfo();
    void processGuiPanelLayout();
    void processGuiPanelMain();
    void processGuiPanelMission();
    void processGuiPanelMissionFail(bool* dayDurationNeedsUpdate);
    void processGuiPanelMissionGoalBe();
    void processGuiPanelMissionGoalCt();
    void processGuiPanelMissionGoalGte();
    void processGuiPanelMissionGrading();
    void processGuiPanelMob();
    void processGuiPanelMobs();
    void processGuiPanelPathLink();
    void processGuiPanelPathStop();
    void processGuiPanelPaths();
    void processGuiPanelReview();
    void processGuiPanelSector();
    void processGuiPanelTools();
    void processGuiOptionsDialog();
    void processGuiStatusBar();
    void processGuiToolbar();
    void handleKeyCharAnywhere(const ALLEGRO_EVENT &ev) override;
    void handleKeyCharCanvas(const ALLEGRO_EVENT &ev) override;
    void handleKeyDownAnywhere(const ALLEGRO_EVENT &ev) override;
    void handleKeyDownCanvas(const ALLEGRO_EVENT &ev) override;
    void handleKeyUpAnywhere(const ALLEGRO_EVENT &ev) override;
    void handleLmbDoubleClick(const ALLEGRO_EVENT &ev) override;
    void handleLmbDown(const ALLEGRO_EVENT &ev) override;
    void handleLmbDrag(const ALLEGRO_EVENT &ev) override;
    void handleLmbUp(const ALLEGRO_EVENT &ev) override;
    void handleMmbDoubleClick(const ALLEGRO_EVENT &ev) override;
    void handleMmbDown(const ALLEGRO_EVENT &ev) override;
    void handleMmbDrag(const ALLEGRO_EVENT &ev) override;
    void handleMouseUpdate(const ALLEGRO_EVENT &ev) override;
    void handleMouseWheel(const ALLEGRO_EVENT &ev) override;
    void handleRmbDoubleClick(const ALLEGRO_EVENT &ev) override;
    void handleRmbDown(const ALLEGRO_EVENT &ev) override;
    void handleRmbDrag(const ALLEGRO_EVENT &ev) override;
    void panCam(const ALLEGRO_EVENT &ev);
    void resetCamXY();
    void resetCamZoom();
    
};
