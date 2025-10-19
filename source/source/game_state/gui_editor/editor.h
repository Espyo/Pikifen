/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general GUI editor-related functions.
 */

#pragma once

#include <string>

#include "../../content/other/gui.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../editor.h"


namespace GUI_EDITOR {
extern const vector<float> GRID_INTERVALS;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the GUI editor.
 */
class GuiEditor : public Editor {

public:

    //--- Members ---
    
    //Automatically load this file upon boot-up of the editor, if any.
    string autoLoadFile;
    
    
    //--- Function declarations ---
    
    GuiEditor();
    void doLogic() override;
    void doDrawing() override;
    void load() override;
    void unload() override;
    string getName() const override;
    void drawCanvas();
    string getOpenedContentPath() const;
    
    
private:

    //--- Members ---
    
    //Currently selected item, or INVALID for none.
    size_t curItem = INVALID;
    
    //Data node for the contents of the current GUI definition.
    DataNode fileNode;
    
    //List of hardcoded item definitions for the current GUI definition.
    vector<GuiItemDef> hardcodedItems;
    
    //List of custom item definitions for the current GUI definition.
    vector<CustomGuiItemDef> customItems;
    
    //Picker info for the picker in the "load" dialog.
    Picker loadDialogPicker;
    
    //Position of the load widget.
    Point loadWidgetPos;
    
    //The list of items must focus on the currently selected item.
    bool mustFocusOnCurItem = false;
    
    //Small hack -- does the camera need re-centering in processGui()?
    bool mustRecenterCam = false;
    
    //Position of the reload widget.
    Point reloadWidgetPos;
    
    //Position of the quit widget.
    Point quitWidgetPos;
    
    //The current transformation widget.
    TransformationWidget curTransformationWidget;
    
    struct {
    
        //Selected pack.
        string pack;
        
        //Internal name of the new GUI definition.
        string internalName;
        
        //Problem found, if any.
        string problem;
        
        //Path to the new GUI definition.
        string defPath;
        
        //Whether the dialog needs updating.
        bool mustUpdate = true;
        
    } newDialog;
    
    
    //--- Function declarations ---
    
    void closeLoadDialog();
    void closeOptionsDialog();
    void createGuiDef(const string& internalName, const string& pack);
    void deleteCurrentGuiDef();
    void loadGuiDefFile(const string& path, bool shouldUpdateHistory);
    void openLoadDialog();
    void openNewDialog();
    void openOptionsDialog();
    void pickGuiDefFile(
        const string& name, const string& topCat, const string& secCat,
        void* info, bool isNew
    );
    bool saveGuiDef();
    void setupForNewGuiDef();
    Point snapPoint(const Point& p);
    static void drawCanvasDearImGuiCallback(
        const ImDrawList* parentList, const ImDrawCmd* cmd
    );
    string getFileTooltip(const string& path) const;
    void gridIntervalDecreaseCmd(float inputValue);
    void gridIntervalIncreaseCmd(float inputValue);
    void deleteGuiDefCmd(float inputValue);
    void loadCmd(float inputValue);
    void quitCmd(float inputValue);
    void reloadCmd(float inputValue);
    void reloadGuiDefs();
    void quickPlayCmd(float inputValue);
    void saveCmd(float inputValue);
    void snapModeCmd(float inputValue);
    void zoomAndPosResetCmd(float inputValue);
    void zoomInCmd(float inputValue);
    void zoomOutCmd(float inputValue);
    void processGui();
    void processGuiControlPanel();
    void processGuiDeleteGuiDefDialog();
    void processGuiLoadDialog();
    void processGuiMenuBar();
    void processGuiNewDialog();
    void processGuiOptionsDialog();
    void processGuiPanelItem();
    void processGuiPanelItems();
    void processGuiStatusBar();
    void processGuiToolbar();
    void handleKeyCharCanvas(const ALLEGRO_EVENT& ev) override;
    void handleKeyDownAnywhere(const ALLEGRO_EVENT& ev) override;
    void handleKeyDownCanvas(const ALLEGRO_EVENT& ev) override;
    void handleLmbDoubleClick(const ALLEGRO_EVENT& ev) override;
    void handleLmbDown(const ALLEGRO_EVENT& ev) override;
    void handleLmbDrag(const ALLEGRO_EVENT& ev) override;
    void handleLmbUp(const ALLEGRO_EVENT& ev) override;
    void handleMmbDown(const ALLEGRO_EVENT& ev) override;
    void handleMmbDrag(const ALLEGRO_EVENT& ev) override;
    void handleMouseUpdate(const ALLEGRO_EVENT& ev) override;
    void handleMouseWheel(const ALLEGRO_EVENT& ev) override;
    void handleRmbDown(const ALLEGRO_EVENT& ev) override;
    void handleRmbDrag(const ALLEGRO_EVENT& ev) override;
    void panCam(const ALLEGRO_EVENT& ev);
    void resetCam(bool instantaneous);
    
};
