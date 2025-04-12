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

    //--- Misc. declarations ---
    
    /**
     * @brief Represents a GUI item.
     */
    struct Item {
    
        //--- Members ---
        
        //Its name in the file.
        string name;
        
        //Center coordinates.
        Point center;
        
        //Width and height.
        Point size;
        
    };
    
    
    //--- Members ---
    
    //Currently selected item, or INVALID for none.
    size_t curItem = INVALID;
    
    //Data node for the contents of the current GUI definition.
    DataNode fileNode;
    
    //List of items for the current GUI definition.
    vector<Item> items;
    
    //Picker info for the picker in the "load" dialog.
    Picker loadDialogPicker;
    
    //Position of the load widget.
    Point loadWidgetPos;
    
    //The list of items must focus on the currently selected item.
    bool mustFocusOnCurItem = false;
    
    //Small hack -- does the camera need recentering in processGui()?
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
    void createGuiDef(const string &internal_name, const string &pack);
    void deleteCurrentGuiDef();
    void loadGuiDefFile(const string &path, bool should_update_history);
    void openLoadDialog();
    void openNewDialog();
    void openOptionsDialog();
    void pickGuiDefFile(
        const string &name, const string &top_cat, const string &sec_cat, void* info, bool is_new
    );
    bool saveGuiDef();
    void setupForNewGuiDef();
    Point snapPoint(const Point &p);
    static void drawCanvasDearImGuiCallback(
        const ImDrawList* parent_list, const ImDrawCmd* cmd
    );
    string getFileTooltip(const string &path) const;
    void gridIntervalDecreaseCmd(float input_value);
    void gridIntervalIncreaseCmd(float input_value);
    void deleteGuiDefCmd(float input_value);
    void loadCmd(float input_value);
    void quitCmd(float input_value);
    void reloadCmd(float input_value);
    void reloadGuiDefs();
    void saveCmd(float input_value);
    void snapModeCmd(float input_value);
    void zoomAndPosResetCmd(float input_value);
    void zoomInCmd(float input_value);
    void zoomOutCmd(float input_value);
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
    void resetCam(bool instantaneous);
    
};
