/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general particle editor-related functions.
 */

#pragma once

#include <string>

#include "../editor.h"


namespace PARTICLE_EDITOR {
extern const vector<float> GRID_INTERVALS;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the particle editor.
 */
class ParticleEditor : public Editor {

public:

    //--- Members ---
    
    //Automatically load this file upon boot-up of the editor, if any.
    string autoLoadFile;
    
    
    //--- Function declarations ---
    
    ParticleEditor();
    void doLogic() override;
    void doDrawing() override;
    void load() override;
    void unload() override;
    string getName() const override;
    void drawCanvas();
    string getOpenedContentPath() const;
    
private:

    //--- Members ---
    
    //Currently loaded particle generator.
    ParticleGenerator loadedGen;
    
    //Particle manager.
    ParticleManager partMgr;
    
    //Whether to use a background texture, if any.
    ALLEGRO_BITMAP* bg = nullptr;
    
    //Is the grid visible?
    bool gridVisible = true;
    
    //Picker info for the picker in the "load" dialog.
    Picker loadDialogPicker;
    
    //Position of the load widget.
    Point loadWidgetPos;
    
    //Is the particle manager currently generating?
    bool mgrRunning = false;
    
    //Is the particle generator currently generating?
    bool genRunning = false;
    
    //Offset the generator's angle in the editor by this much.
    float generatorAngleOffset = 0.0f;
    
    //Offset the generator's position in the editor by this much.
    Point generatorPosOffset;
    
    //Is the leader silhouette visible?
    bool leaderSilhouetteVisible = false;
    
    //Is the emission shape visible?
    bool emissionShapeVisible = false;
    
    //Selected color keyframe.
    size_t selectedColorKeyframe = 0;
    
    //Selected size keyframe.
    size_t selectedSizeKeyframe = 0;
    
    //Selected linear speed keyframe.
    size_t selectedLinearSpeedKeyframe = 0;
    
    //Selected orbital velocity keyframe.
    size_t selectedOrbitalVelocityKeyframe = 0;
    
    //Selected outward velocity keyframe.
    size_t selectedOutwardVelocityKeyframe = 0;
    
    //Position of the reload widget.
    Point reloadWidgetPos;
    
    //Position of the quit widget.
    Point quitWidgetPos;
    
    //Whether to use a background texture.
    bool useBg = false;
    
    struct {
    
        //Selected pack.
        string pack;
        
        //Internal name of the new particle generator.
        string internalName = "my_particle_generator";
        
        //Path to the new generator.
        string partGenPath;
        
        //Last time we checked if the new generator path existed, it was this.
        string lastCheckedPartGenPath;
        
        //Does a file already exist under the new generator's path?
        bool partGenPathExists = false;
        
        //Whether we need to focus on the text input widget.
        bool needsTextFocus = true;
        
    } newDialog;
    
    
    //--- Function declarations ---
    
    void closeLoadDialog();
    void closeOptionsDialog();
    void createPartGen(const string &part_gen_path);
    void deleteCurrentPartGen();
    string getFileTooltip(const string &path) const;
    void loadPartGenFile(
        const string &path, const bool should_update_history
    );
    void openLoadDialog();
    void openNewDialog();
    void openOptionsDialog();
    void pickPartGenFile(
        const string &name, const string &top_cat, const string &sec_cat,
        void* info, bool is_new
    );
    void reloadPartGens();
    void setupForNewPartGenPost();
    void setupForNewPartGenPre();
    bool savePartGen();
    static void drawCanvasDearImGuiCallback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    void gridIntervalDecreaseCmd(float input_value);
    void gridIntervalIncreaseCmd(float input_value);
    void gridToggleCmd(float input_value);
    void deletePartGenCmd(float input_value);
    void loadCmd(float input_value);
    void quitCmd(float input_value);
    void reloadCmd(float input_value);
    void saveCmd(float input_value);
    void zoomAndPosResetCmd(float input_value);
    void zoomInCmd(float input_value);
    void zoomOutCmd(float input_value);
    void clearParticlesCmd(float input_value);
    void emissionShapeToggleCmd(float input_value);
    void leaderSilhouetteToggleCmd(float input_value);
    void partGenPlaybackToggleCmd(float input_value);
    void partMgrPlaybackToggleCmd(float input_value);
    void processGui();
    void processGuiControlPanel();
    void processGuiDeletePartGenDialog();
    void processGuiLoadDialog();
    void processGuiMenuBar();
    void processGuiNewDialog();
    void processGuiOptionsDialog();
    void processGuiPanelGenerator();
    void processGuiStatusBar();
    void processGuiToolbar();
    void handleKeyCharCanvas(const ALLEGRO_EVENT &ev) override;
    void handleKeyDownAnywhere(const ALLEGRO_EVENT &ev) override;
    void handleKeyDownCanvas(const ALLEGRO_EVENT &ev) override;
    void handleLmbDoubleClick(const ALLEGRO_EVENT &ev) override;
    void handleLmbDown(const ALLEGRO_EVENT &ev) override;
    void handleLmbDrag(const ALLEGRO_EVENT &ev) override;
    void handleLmbUp(const ALLEGRO_EVENT &ev) override;
    void handleMmbDown(const ALLEGRO_EVENT &ev) override;
    void handleMmbDrag(const ALLEGRO_EVENT &ev) override;
    void handleMouseUpdate(const ALLEGRO_EVENT &ev) override;
    void handleMouseWheel(const ALLEGRO_EVENT &ev) override;
    void handleRmbDown(const ALLEGRO_EVENT &ev) override;
    void handleRmbDrag(const ALLEGRO_EVENT &ev) override;
    void panCam(const ALLEGRO_EVENT &ev);
    void resetCamXY();
    void resetCamZoom();
    
};
