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
extern const float FINAL_SCORE_LABEL_SWAY_TIME_OFFSET;
extern const float FINAL_SCORE_SWAY;
extern const float FINAL_SCORE_SWAY_TIME_SCALE;
extern const string GUI_FILE_PATH;
extern const float MEDAL_SCALE;
extern const float MEDAL_SHINE_SCALE;
extern const float MEDAL_SHINE_ROT_TIME_SCALE;
}


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

    //--- Private members ---
    
    //GUI manager.
    GuiManager gui;
    
    //Time spent on this state.
    float guiTimeSpent = 0.0f;
    
    //GUI items that need to grow during the periodic text animation.
    vector<GuiItem*> textToAnimate;
    
    //Stats box GUI item.
    ListGuiItem* statsList = nullptr;
    
    //Final mission score. Cache for convenience.
    int finalMissionScore = 0;
    
    
    //--- Private function declarations ---
    
    void addNewScoreStat(size_t criterionIdx);
    void addNewStat(
        const string& label, const string& value,
        const ALLEGRO_COLOR& color = COLOR_WHITE
    );
    void continuePlaying();
    void leave();
    void populateStatsList(const MissionRecord& oldRecord);
    void retryArea();
    
};
