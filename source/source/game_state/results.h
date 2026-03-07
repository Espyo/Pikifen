/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the exploration/mission results game state.
 */

#pragma once

#include "../content/area/mission.h"
#include "../content/other/gui.h"
#include "../util/drawing_utils.h"
#include "game_state.h"


namespace RESULTS {
extern const float CHART_CIRCLE_TIME_SCALE;
extern const float CHART_CIRCLE_SIZE_OFFSET;
extern const float FINAL_SCORE_LABEL_SWAY_TIME_OFFSET;
extern const float FINAL_SCORE_SWAY;
extern const float FINAL_SCORE_SWAY_TIME_SCALE;
extern const string GUI_FILE_PATH;
extern const float MEDAL_SCALE;
extern const float MEDAL_SHINE_SCALE;
extern const float MEDAL_SHINE_ROT_TIME_SCALE;
}


using DrawInfo = GuiItem::DrawInfo;


//Pages.
enum RESULTS_MENU_PAGE {

    //Stats.
    RESULTS_MENU_PAGE_STATS,
    
    //Scoring.
    RESULTS_MENU_PAGE_SCORING,
    
    //Score chart.
    RESULTS_MENU_PAGE_SCORE_CHART,
    
};


/**
 * @brief Info about the area results menu.
 */
class Results : public GameState {

public:

    //--- Public function declarations ---
    
    void load() override;
    void unload() override;
    void handleAllegroEvent(ALLEGRO_EVENT& ev) override;
    void doLogic() override;
    void doDrawing() override;
    string getName() const override;
    
private:

    //--- Private misc. declarations ---
    
    enum SCORE_MARKER {
        SCORE_MARKER_BRONZE,
        SCORE_MARKER_SILVER,
        SCORE_MARKER_GOLD,
        SCORE_MARKER_PLATINUM,
        SCORE_MARKER_SCORE,
        SCORE_MARKER_OLD_RECORD,
        SCORE_MARKER_MAKER_RECORD,
    };
    
    //--- Private members ---
    
    //Main GUI manager.
    GuiManager gui;
    
    //Time spent on this state.
    float guiTimeSpent = 0.0f;
    
    //GUI items that need to grow during the periodic text animation.
    vector<GuiItem*> textToAnimate;
    
    //Stats page box GUI item.
    GuiItem* statsPageBox = nullptr;
    
    //Scoring page box GUI item.
    GuiItem* scoringPageBox = nullptr;
    
    //Score chart page box GUI item.
    GuiItem* scoreChartPageBox = nullptr;
    
    //Stats list GUI item.
    ListGuiItem* statsList = nullptr;
    
    //Scoring list GUI item.
    ListGuiItem* scoringList = nullptr;
    
    //Score chart items list GUI item.
    ListGuiItem* scoreChartList = nullptr;
    
    //Score chart graphic GUI item.
    ListGuiItem* scoreChartChart = nullptr;
    
    //Score amount at the bottom of the score chart.
    int scoreChartBottom = 0;
    
    //Score amount at the top of the score chart.
    int scoreChartTop = 0;
    
    //Relevant score markers, sorted. Cache for convenience.
    vector<std::pair<SCORE_MARKER, int> > scoreMarkers;
    
    //Bullet point GUI item for each score marker. Cache for convenience.
    vector<GuiItem*> scoreMarkerGuiItems;
    
    //Medal obtained. Cache for convenience.
    MISSION_MEDAL medal = MISSION_MEDAL_NONE;
    
    //Mission event that ended the mission, if any. Cache for convenience.
    MissionEvent* endEv = nullptr;
    
    //Final mission score. Cache for convenience.
    int finalMissionScore = 0;
    
    //The player's old record, if any. Cache for convenience.
    MissionRecord oldRecord;
    
    //Whether this was a new record or not. Cache for convenience.
    bool isNewRecord = false;
    
    //Whether the new record was saved successfully. Cache for convenience.
    bool savedSuccessfully = false;
    
    
    //--- Private function declarations ---
    
    ButtonGuiItem* addNewPageItem(
        RESULTS_MENU_PAGE targetPage, bool left, GuiItem* curBox
    );
    void addNewPageItems(
        RESULTS_MENU_PAGE curPage, GuiItem* curBox, const string& itemNamePrefix
    );
    void addNewBulletPoint(
        GuiItem* list, const string& label, const string& value,
        const ALLEGRO_COLOR& color = COLOR_WHITE
    );
    void addNewScoreMarkerBulletPoint(
        const string& label, const string& value,
        const size_t totalBulletPoints, const ALLEGRO_COLOR& color = COLOR_WHITE
    );
    void continuePlaying();
    void drawScoreChartConnections();
    void drawScoreChartGraphic(const DrawInfo& draw);
    float getScoreChartY(int score) const;
    void initGuiMain();
    void initGuiScoreChart();
    void initGuiScoring();
    void initGuiStats();
    void leave();
    void populateScoreChart();
    void populateScoringList();
    void populateStatsList();
    void switchPage(RESULTS_MENU_PAGE newPage);
    void retryArea();
    
};
