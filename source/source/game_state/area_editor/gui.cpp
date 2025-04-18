/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor Dear ImGui logic.
 */

#include <algorithm>

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../lib/imgui/imgui_stdlib.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/imgui_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Opens the "load" dialog.
 */
void AreaEditor::openLoadDialog() {
    reloadAreas();
    
    //Set up the picker's behavior and data.
    vector<PickerItem> areas;
    
    for(size_t a = 0; a < game.content.areas.list[AREA_TYPE_SIMPLE].size(); a++) {
        Area* area_ptr = game.content.areas.list[AREA_TYPE_SIMPLE][a];
        ContentManifest* man = area_ptr->manifest;
        areas.push_back(
            PickerItem(
                area_ptr->name,
                "Pack: " + game.content.packs.list[man->pack].name,
                "Simple", (void*) man,
                getFolderTooltip(man->path, ""),
                area_ptr->thumbnail.get()
            )
        );
    }
    for(size_t a = 0; a < game.content.areas.list[AREA_TYPE_MISSION].size(); a++) {
        Area* area_ptr = game.content.areas.list[AREA_TYPE_MISSION][a];
        ContentManifest* man = area_ptr->manifest;
        areas.push_back(
            PickerItem(
                area_ptr->name,
                "Pack: " + game.content.packs.list[man->pack].name,
                "Mission", (void*) man,
                getFolderTooltip(man->path, ""),
                area_ptr->thumbnail.get()
            )
        );
    }
    
    loadDialogPicker = Picker(this);
    loadDialogPicker.items = areas;
    loadDialogPicker.pickCallback =
        std::bind(
            &AreaEditor::pickAreaFolder, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );
        
    //Open the dialog that will contain the picker and history.
    openDialog(
        "Load an area or create a new one",
        std::bind(&AreaEditor::processGuiLoadDialog, this)
    );
    dialogs.back()->closeCallback =
        std::bind(&AreaEditor::closeLoadDialog, this);
}


/**
 * @brief Opens the "new" dialog.
 */
void AreaEditor::openNewDialog() {
    openDialog(
        "Create a new area",
        std::bind(&AreaEditor::processGuiNewDialog, this)
    );
    dialogs.back()->customSize = Point(400, 0);
    dialogs.back()->closeCallback = [this] () {
        newDialog.pack.clear();
        newDialog.internalName = "my_area";
        newDialog.type = AREA_TYPE_SIMPLE;
        newDialog.areaPath.clear();
        newDialog.lastCheckedAreaPath.clear();
        newDialog.areaPathExists = false;
    };
}


/**
 * @brief Opens the options dialog.
 */
void AreaEditor::openOptionsDialog() {
    openDialog(
        "Options",
        std::bind(&AreaEditor::processGuiOptionsDialog, this)
    );
    dialogs.back()->closeCallback =
        std::bind(&AreaEditor::closeOptionsDialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void AreaEditor::processGui() {
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.winW, game.winH));
    ImGui::Begin(
        "Area editor", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse
    );
    
    //The menu bar.
    processGuiMenuBar();
    
    //The two main columns that split the canvas (+ toolbar + status bar)
    //and control panel.
    ImGui::Columns(2, "colMain");
    
    //Do the toolbar.
    processGuiToolbar();
    
    //Draw the canvas now.
    ImGui::BeginChild("canvas", ImVec2(0, -EDITOR::STATUS_BAR_HEIGHT));
    ImGui::EndChild();
    isMouseInGui =
        !ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    ImVec2 tl = ImGui::GetItemRectMin();
    canvasTL.x = tl.x;
    canvasTL.y = tl.y;
    ImVec2 br = ImGui::GetItemRectMax();
    canvasBR.x = br.x;
    canvasBR.y = br.y;
    ImGui::GetWindowDrawList()->AddCallback(drawCanvasDearImGuiCallback, nullptr);
    
    //Status bar.
    processGuiStatusBar();
    
    //Set up the separator for the control panel.
    ImGui::NextColumn();
    
    if(canvasSeparatorX == -1) {
        canvasSeparatorX = game.winW * 0.675;
        ImGui::SetColumnWidth(0, canvasSeparatorX);
    } else {
        canvasSeparatorX = ImGui::GetColumnOffset(1);
    }
    
    //Do the control panel now.
    processGuiControlPanel();
    ImGui::NextColumn();
    
    //Finish the main window.
    ImGui::Columns(1);
    ImGui::End();
    
    //Process any dialogs.
    processDialogs();
}


/**
 * @brief Processes the Dear ImGui control panel for this frame.
 */
void AreaEditor::processGuiControlPanel() {
    ImGui::BeginChild("panel");
    
    //Basically, just show the correct panel for the current state.
    switch(state) {
    case EDITOR_STATE_MAIN: {
        processGuiPanelMain();
        break;
    } case EDITOR_STATE_INFO: {
        processGuiPanelInfo();
        break;
    } case EDITOR_STATE_GAMEPLAY: {
        processGuiPanelGameplay();
        break;
    } case EDITOR_STATE_LAYOUT: {
        processGuiPanelLayout();
        break;
    } case EDITOR_STATE_MOBS: {
        processGuiPanelMobs();
        break;
    } case EDITOR_STATE_PATHS: {
        processGuiPanelPaths();
        break;
    } case EDITOR_STATE_DETAILS: {
        processGuiPanelDetails();
        break;
    } case EDITOR_STATE_REVIEW: {
        processGuiPanelReview();
        break;
    } case EDITOR_STATE_TOOLS: {
        processGuiPanelTools();
        break;
    }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui area deletion dialog for this frame.
 */
void AreaEditor::processGuiDeleteAreaDialog() {
    //Explanation text.
    string explanation_str;
    if(!changesMgr.existsOnDisk()) {
        explanation_str =
            "You have never saved this area to disk, so if you\n"
            "delete, you will only lose your unsaved progress.";
    } else {
        explanation_str =
            "If you delete, you will lose all unsaved progress,\n"
            "and the area's files on the disk will be gone FOREVER!";
    }
    ImGui::SetupCentering(ImGui::CalcTextSize(explanation_str.c_str()).x);
    ImGui::Text("%s", explanation_str.c_str());
    
    //Final warning text.
    string final_warning_str =
        "Are you sure you want to delete the current area?";
    ImGui::SetupCentering(ImGui::CalcTextSize(final_warning_str.c_str()).x);
    ImGui::TextColored(
        ImVec4(0.8, 0.6, 0.6, 1.0),
        "%s", final_warning_str.c_str()
    );
    
    //Cancel button.
    ImGui::Spacer();
    ImGui::SetupCentering(100 + 100 + 30);
    if(ImGui::Button("Cancel", ImVec2(100, 40))) {
        closeTopDialog();
    }
    
    //Delete button.
    ImGui::SameLine(0.0f, 30);
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImVec4(0.3, 0.1, 0.1, 1.0)
    );
    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered, ImVec4(0.5, 0.1, 0.1, 1.0)
    );
    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive, ImVec4(0.4, 0.1, 0.1, 1.0)
    );
    if(ImGui::Button("Delete", ImVec2(100, 40))) {
        closeTopDialog();
        deleteCurrentArea();
    }
    ImGui::PopStyleColor(3);
}


/**
 * @brief Processes the Dear ImGui widgets regarding a grading criterion
 * for this frame.
 *
 * @param value_ptr Points to the value of the points value.
 * @param criterion_idx Criterion index.
 * @param widget_label Label for the main value widget.
 * @param tooltip Start of the tooltip for this criterion's value widget.
 */
void AreaEditor::processGuiGradingCriterionWidgets(
    int* value_ptr, MISSION_SCORE_CRITERIA criterion_idx,
    const string &widget_label, const string &tooltip
) {
    //Main value.
    ImGui::SetNextItemWidth(50);
    int points_int = *value_ptr;
    if(ImGui::DragInt(widget_label.c_str(), &points_int, 0.1f)) {
        registerChange("mission grading change");
        *value_ptr = points_int;
    }
    setTooltip(
        tooltip + "\n"
        "Negative numbers means the player loses points.\n"
        "0 means this criterion doesn't count.",
        "", WIDGET_EXPLANATION_DRAG
    );
    if(*value_ptr != 0) {
        ImGui::Indent();
        
        //Loss on fail checkbox.
        int flags = game.curAreaData->mission.pointLossData;
        if(
            ImGui::CheckboxFlags(
                ("0 points on fail##zpof" + i2s(criterion_idx)).c_str(),
                &flags,
                getIdxBitmask(criterion_idx)
            )
        ) {
            registerChange("mission grading change");
            game.curAreaData->mission.pointLossData = flags;
        }
        setTooltip(
            "If checked, the player will receive 0 points for\n"
            "this criterion if they fail the mission."
        );
        
        //Use in HUD checkbox.
        flags = game.curAreaData->mission.pointHudData;
        if(
            ImGui::CheckboxFlags(
                ("Use in HUD counter##uihc" + i2s(criterion_idx)).c_str(),
                &flags, getIdxBitmask(MISSION_SCORE_CRITERIA_PIKMIN_BORN)
            )
        ) {
            registerChange("mission grading change");
            game.curAreaData->mission.pointHudData = flags;
        }
        setTooltip(
            "If checked, the HUD item for the score counter will\n"
            "use this criterion in its calculation. If none of\n"
            "the criteria are used for the HUD item, then it\n"
            "won't even show up."
        );
        
        ImGui::Unindent();
    }
}


/**
 * @brief Processes the Dear ImGui widgets regarding a grading medal
 * requirements for this frame.
 *
 * @param requirement_ptr Points to the requirement value for this medal.
 * @param widget_label Label for the value widget.
 * @param widget_min_value Minimum value for the value widget.
 * @param widget_max_value Maximum value for the value widget.
 * @param tooltip Tooltip for the value widget.
 */
void AreaEditor::processGuiGradingMedalWidgets(
    int* requirement_ptr, const string &widget_label,
    int widget_min_value, int widget_max_value,
    const string &tooltip
) {
    //Requirement value.
    int req = *requirement_ptr;
    ImGui::SetNextItemWidth(90);
    if(
        ImGui::DragInt(
            widget_label.c_str(), &req, 1.0f, widget_min_value, widget_max_value
        )
    ) {
        registerChange("mission grading change");
        *requirement_ptr = req;
    }
    setTooltip(tooltip, "", WIDGET_EXPLANATION_DRAG);
}


/**
 * @brief Processes the Dear ImGui widgets regarding a grading mode
 * for this frame.
 *
 * @param value Internal value for this mode's radio button.
 * @param widget_label Label for the radio widget.
 * @param tooltip Tooltip for the radio widget.
 */
void AreaEditor::processGuiGradingModeWidgets(
    int value, const string &widget_label, const string &tooltip
) {
    //Radio button.
    int mode = game.curAreaData->mission.gradingMode;
    if(ImGui::RadioButton(widget_label.c_str(), &mode, value)) {
        registerChange("mission grading change");
        game.curAreaData->mission.gradingMode =
            (MISSION_GRADING_MODE) mode;
    }
    setTooltip(tooltip);
}


/**
 * @brief Processes the Dear ImGui "load" dialog for this frame.
 */
void AreaEditor::processGuiLoadDialog() {
    //History node.
    processGuiHistory(
        game.options.areaEd.history,
    [this](const string &name) -> string {
        return name;
    },
    [this](const string &path) {
        closeTopDialog();
        loadAreaFolder(path, false, true);
    },
    [this](const string &path) {
        return getFolderTooltip(path, "");
    }
    );
    
    //New node.
    ImGui::Spacer();
    if(saveableTreeNode("load", "New")) {
        if(ImGui::Button("Create new...", ImVec2(168.0f, 32.0f))) {
            openNewDialog();
        }
        
        ImGui::TreePop();
    }
    setTooltip("Create a new area.");
    
    //Load node.
    ImGui::Spacer();
    if(saveableTreeNode("load", "Load")) {
        loadDialogPicker.process();
        
        ImGui::TreePop();
    }
}


/**
 * @brief Processes the Dear ImGui "new" dialog for this frame.
 */
void AreaEditor::processGuiNewDialog() {
    string problem;
    bool hit_create_button = false;
    
    //Pack widgets.
    processGuiNewDialogPackWidgets(&newDialog.pack);
    
    //Internal name input.
    ImGui::Spacer();
    ImGui::FocusOnInputText(newDialog.needsTextFocus);
    if(
        monoInputText(
            "Internal name", &newDialog.internalName,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        hit_create_button = true;
    }
    setTooltip(
        "Internal name of the new area.\n"
        "Remember to keep it simple, type in lowercase, and use underscores!"
    );
    
    //Simple area radio.
    ImGui::Spacer();
    ImGui::RadioButton("Simple area", &newDialog.type, AREA_TYPE_SIMPLE);
    setTooltip("Choose this to make your area a simple area.");
    
    //Mission area radio.
    ImGui::SameLine();
    ImGui::RadioButton("Mission", &newDialog.type, AREA_TYPE_MISSION);
    setTooltip("Choose this to make your area a mission area.");
    
    //Check if everything's ok.
    ContentManifest temp_man;
    temp_man.pack = newDialog.pack;
    temp_man.internalName = newDialog.internalName;
    newDialog.areaPath =
        game.content.areas.manifestToPath(
            temp_man, (AREA_TYPE) newDialog.type
        );
    if(newDialog.lastCheckedAreaPath != newDialog.areaPath) {
        newDialog.areaPathExists = folderExists(newDialog.areaPath);
        newDialog.lastCheckedAreaPath = newDialog.areaPath;
    }
    
    if(newDialog.internalName.empty()) {
        problem = "You have to type an internal name first!";
    } else if(!isInternalNameGood(newDialog.internalName)) {
        problem =
            "The internal name should only have lowercase letters,\n"
            "numbers, and underscores!";
    } else if(newDialog.areaPathExists) {
        problem =
            "There is already an area of that type with\n"
            "that internal name in that pack!";
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::SetupCentering(100);
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create area", ImVec2(100, 40))) {
        hit_create_button = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    setTooltip(
        problem.empty() ? "Create the area!" : problem
    );
    
    //Creation logic.
    if(hit_create_button) {
        if(!problem.empty()) return;
        auto really_create = [this] () {
            createArea(newDialog.areaPath);
            closeTopDialog();
            closeTopDialog(); //Close the load dialog.
        };
        
        if(
            newDialog.pack == FOLDER_NAMES::BASE_PACK &&
            !game.options.advanced.engineDev
        ) {
            openBaseContentWarningDialog(really_create);
        } else {
            really_create();
        }
    }
}


/**
 * @brief Processes the Dear ImGui menu bar for this frame.
 */
void AreaEditor::processGuiMenuBar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load or create area item.
            if(ImGui::MenuItem("Load or create...", "Ctrl+L")) {
                loadWidgetPos = getLastWidgetPost();
                loadCmd(1.0f);
            }
            setTooltip(
                "Pick an area to load, or create a new one.",
                "Ctrl + L"
            );
            
            //Reload current area item.
            if(ImGui::MenuItem("Reload current area")) {
                reloadWidgetPos = getLastWidgetPost();
                reloadCmd(1.0f);
            }
            setTooltip(
                "Lose all changes and reload the current area from the disk."
            );
            
            //Save current area item.
            if(ImGui::MenuItem("Save current area", "Ctrl+S")) {
                saveCmd(1.0f);
            }
            setTooltip(
                "Save the area into the files on disk.",
                "Ctrl + S"
            );
            
            //Delete current area item.
            if(ImGui::MenuItem("Delete current area")) {
                deleteAreaCmd(1.0f);
            }
            setTooltip(
                "Delete the current area from the disk."
            );
            
            //Quick play item.
            if(ImGui::MenuItem("Quick play", "Ctrl+P")) {
                quickPlayCmd(1.0f);
            }
            setTooltip(
                "Save, quit, and start playing the area. Leaving will return "
                "to the editor.",
                "Ctrl + P"
            );
            
            //Separator item.
            ImGui::Separator();
            
            //Options menu item.
            if(ImGui::MenuItem("Options...")) {
                openOptionsDialog();
            }
            setTooltip(
                "Open the options menu, so you can tweak your preferences."
            );
            
            //Debug menu.
            if(ImGui::BeginMenu("Debug")) {
            
                //Show edge indexes item.
                if(
                    ImGui::MenuItem(
                        "Show edge indexes", "F2", &debugEdgeIdxs
                    )
                ) {
                    if(debugEdgeIdxs) {
                        setStatus("Enabled debug edge index display.");
                    } else {
                        setStatus("Disabled debug edge index display.");
                    }
                }
                setTooltip(
                    "Shows what index each edge is.\n"
                    "Mostly useful for debugging the engine."
                );
                
                //Show sector indexes item.
                if(
                    ImGui::MenuItem(
                        "Show sector indexes", "F3", &debugSectorIdxs
                    )
                ) {
                    if(debugSectorIdxs) {
                        setStatus("Enabled debug sector index display.");
                    } else {
                        setStatus("Disabled debug sector index display.");
                    }
                }
                setTooltip(
                    "Shows the sector index on either side of an edge.\n"
                    "Mostly useful for debugging the engine."
                );
                
                //Show vertex indexes item.
                if(
                    ImGui::MenuItem(
                        "Show vertex indexes", "F4", &debugVertexIdxs
                    )
                ) {
                    if(debugVertexIdxs) {
                        setStatus("Enabled debug vertex index display.");
                    } else {
                        setStatus("Disabled debug vertex index display.");
                    }
                }
                setTooltip(
                    "Shows what index each vertex is.\n"
                    "Mostly useful for debugging the engine."
                );
                
                //Show sector triangulation item.
                if(
                    ImGui::MenuItem(
                        "Show sector triangulation", "F5", &debugTriangulation
                    )
                ) {
                    if(debugTriangulation) {
                        setStatus("Enabled debug triangulation display.");
                    } else {
                        setStatus("Disabled debug triangulation display.");
                    }
                }
                setTooltip(
                    "Shows what triangles make up the selected sector.\n"
                    "Mostly useful for debugging the engine."
                );
                
                //Show path indexes item.
                if(
                    ImGui::MenuItem(
                        "Show path indexes", "F6", &debugPathIdxs
                    )
                ) {
                    if(debugPathIdxs) {
                        setStatus("Enabled debug path index display.");
                    } else {
                        setStatus("Disabled debug path index display.");
                    }
                }
                setTooltip(
                    "Shows what index each path stop is.\n"
                    "Mostly useful for debugging the engine."
                );
                
                ImGui::EndMenu();
                
            }
            
            //Quit editor item.
            if(ImGui::MenuItem("Quit", "Ctrl+Q")) {
                quitWidgetPos = getLastWidgetPost();
                quitCmd(1.0f);
            }
            setTooltip(
                "Quit the area editor.",
                "Ctrl + Q"
            );
            
            ImGui::EndMenu();
            
        }
        
        //Edit menu.
        if(ImGui::BeginMenu("Edit")) {
        
            //Undo item.
            if(ImGui::MenuItem("Undo", "Ctrl+Z")) {
                undoCmd(1.0f);
            }
            string undo_text;
            if(undoHistory.empty()) {
                undo_text = "Nothing to undo.";
            } else {
                undo_text = "Undo: " + undoHistory.front().second + ".";
            }
            setTooltip(
                undo_text,
                "Ctrl + Z"
            );
            
            //Redo item.
            if(ImGui::MenuItem("Redo", "Ctrl+Y")) {
                redoCmd(1.0f);
            }
            string redo_text;
            if(redoHistory.empty()) {
                redo_text =
                    "Nothing to redo.";
            } else {
                redo_text =
                    "Redo: " + redoHistory.front().second + ".";
            }
            setTooltip(
                redo_text,
                "Ctrl + Y"
            );
            
            //Separator.
            ImGui::Separator();
            
            //Copy properties item.
            if(ImGui::MenuItem("Copy properties", "Ctrl+C")) {
                copyPropertiesCmd(1.0f);
            }
            setTooltip(
                "Copies the properties of what you selected, if applicable.",
                "Ctrl + C"
            );
            
            //Paste properties item.
            if(ImGui::MenuItem("Paste properties", "Ctrl+V")) {
                pastePropertiesCmd(1.0f);
            }
            setTooltip(
                "Pastes previously-copied properties onto what you selected, "
                "if applicable.",
                "Ctrl + V"
            );
            
            if(
                state == EDITOR_STATE_LAYOUT &&
                subState == EDITOR_SUB_STATE_NONE
            ) {
                //Paste texture item.
                if(ImGui::MenuItem("Paste texture", "Ctrl+T")) {
                    pasteTextureCmd(1.0f);
                }
                setTooltip(
                    "Pastes a previously-copied sector's texture onto "
                    "the sector you selected.",
                    "Ctrl + T"
                );
            }
            
            //Separator.
            ImGui::Separator();
            
            //Select all item.
            if(ImGui::MenuItem("Select all", "Ctrl+A")) {
                selectAllCmd(1.0f);
            }
            setTooltip(
                "Selects everything in the current mode, if applicable.",
                "Ctrl + A"
            );
            
            //Delete item.
            if(ImGui::MenuItem("Delete", "Delete")) {
                deleteCmd(1.0f);
            }
            setTooltip(
                "Deletes the selected things, if applicable.",
                "Delete"
            );
            
            ImGui::EndMenu();
            
        }
        
        //View menu.
        if(ImGui::BeginMenu("View")) {
        
            //Zoom in item.
            if(ImGui::MenuItem("Zoom in", "Plus")) {
                zoomInCmd(1.0f);
            }
            setTooltip(
                "Zooms the camera in a bit.",
                "Plus"
            );
            
            //Zoom out item.
            if(ImGui::MenuItem("Zoom out", "Minus")) {
                zoomOutCmd(1.0f);
            }
            setTooltip(
                "Zooms the camera out a bit.",
                "Minus"
            );
            
            //Zoom and position reset item.
            if(ImGui::MenuItem("Zoom/position reset", "0")) {
                zoomAndPosResetCmd(1.0f);
            }
            setTooltip(
                "Reset the zoom level, and if pressed again,\n"
                "reset the camera position.",
                "0"
            );
            
            //Zoom everything item.
            if(ImGui::MenuItem("Zoom onto everything", "Home")) {
                zoomEverythingCmd(1.0f);
            }
            setTooltip(
                "Move and zoom the camera so that everything in the area\n"
                "fits nicely into view.",
                "Home"
            );
            
            ImGui::EndMenu();
            
        }
        
        //Help menu.
        if(ImGui::BeginMenu("Help")) {
        
            //Show tooltips item.
            if(
                ImGui::MenuItem(
                    "Show tooltips", "", &game.options.editors.showTooltips
                )
            ) {
                string state_str =
                    game.options.editors.showTooltips ? "Enabled" : "Disabled";
                setStatus(state_str + " tooltips.");
                saveOptions();
            }
            setTooltip(
                "Whether tooltips should appear when you place your mouse on\n"
                "top of something in the GUI. Like the tooltip you are\n"
                "reading right now."
            );
            
            //General help item.
            if(ImGui::MenuItem("Help...")) {
                string help_str =
                    "To create an area, start by drawing its layout. "
                    "For this, you draw the polygons that make up the "
                    "geometry of the area. These polygons cannot overlap, "
                    "and a polygon whose floor is higher than its neighbor's "
                    "makes a wall. After that, place objects where you want, "
                    "specify the carrying paths, add details, and try it out."
                    "\n\n"
                    "If you need more help on how to use the area editor, "
                    "check out the tutorial in the manual, located "
                    "in the engine's folder.";
                openHelpDialog(help_str, "area.html");
            }
            setTooltip(
                "Opens a general help message for this editor."
            );
            
            ImGui::EndMenu();
            
        }
        
        ImGui::EndMenuBar();
        
    }
}


/**
 * @brief Processes the Dear ImGui mob script vars for this frame.
 *
 * @param m_ptr Mob to process.
 */
void AreaEditor::processGuiMobScriptVars(MobGen* m_ptr) {
    if(!m_ptr->type) return;
    
    map<string, string> vars_map = getVarMap(m_ptr->vars);
    map<string, string> new_vars_map;
    map<string, bool> vars_in_widgets;
    
    //Start with the properties that apply to all objects.
    
    //Team property.
    string team_value;
    if(vars_map.find("team") != vars_map.end()) {
        team_value = vars_map["team"];
    }
    
    vector<string> team_names;
    team_names.push_back("(Default)");
    for(unsigned char t = 0; t < N_MOB_TEAMS; t++) {
        team_names.push_back(game.teamNames[t]);
    }
    
    int team_nr;
    if(team_value.empty()) {
        team_nr = 0;
    } else {
        size_t team_nr_st = stringToTeamNr(team_value);
        if(team_nr_st == INVALID) {
            team_nr = 0;
        } else {
            //0 is reserved in this widget for "default".
            //Increase it by one to get the widget's team index number.
            team_nr = (int) team_nr_st + 1;
        }
    }
    
    if(ImGui::Combo("Team", &team_nr, team_names, 15)) {
        registerChange("object script vars change");
        if(team_nr > 0) {
            //0 is reserved in this widget for "default".
            //Decrease it by one to get the real team index number.
            team_nr--;
            team_value = game.teamInternalNames[team_nr];
        } else {
            team_value.clear();
        }
    }
    setTooltip(
        "What sort of team this object belongs to.\n"
        "(Variable name: \"team\".)"
    );
    
    if(!team_value.empty()) new_vars_map["team"] = team_value;
    vars_in_widgets["team"] = true;
    
    //Health property.
    float max_health = m_ptr->type->maxHealth;
    if(vars_map.find("max_health") != vars_map.end()) {
        max_health = s2f(vars_map["max_health"]);
    }
    float health = max_health;
    if(vars_map.find("health") != vars_map.end()) {
        health = s2f(vars_map["health"]);
    }
    
    if(ImGui::DragFloat("Health", &health, 0.25f, 0.0f, max_health)) {
        registerChange("object script vars change");
    }
    setTooltip(
        "Starting health for this specific object.\n"
        "(Variable name: \"health\".)",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    if(health != max_health) {
        new_vars_map["health"] = f2s(health);
    }
    vars_in_widgets["health"] = true;
    
    //Max health property.
    if(ImGui::DragFloat("Max health", &max_health, 0.25f, 0.0f, FLT_MAX)) {
        registerChange("object script vars change");
    }
    setTooltip(
        "Maximum health for this specific object.\n"
        "The object type's default is " + f2s(m_ptr->type->maxHealth) + ".\n"
        "(Variable name: \"max_health\".)",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    if(max_health != m_ptr->type->maxHealth) {
        new_vars_map["max_health"] = f2s(max_health);
    }
    vars_in_widgets["max_health"] = true;
    
    //Now, dynamically create widgets for all properties this mob type has.
    
    for(size_t p = 0; p < m_ptr->type->areaEditorProps.size(); p++) {
    
        MobType::AreaEditorProp* p_ptr =
            &m_ptr->type->areaEditorProps[p];
            
        string value;
        if(vars_map.find(p_ptr->var) == vars_map.end()) {
            value = p_ptr->defValue;
        } else {
            value = vars_map[p_ptr->var];
        }
        
        switch(p_ptr->type) {
        case AEMP_TYPE_TEXT: {
    
            string value_s = value;
            if(ImGui::InputText(p_ptr->name.c_str(), &value_s)) {
                registerChange("object script vars change");
                value = value_s;
            }
            
            break;
            
        } case AEMP_TYPE_INT: {
    
            int value_i = s2i(value);
            if(
                ImGui::DragInt(
                    p_ptr->name.c_str(), &value_i, 0.02f,
                    p_ptr->minValue, p_ptr->maxValue
                )
            ) {
                registerChange("object script vars change");
                value = i2s(value_i);
            }
            
            break;
            
        } case AEMP_TYPE_FLOAT: {
    
            float value_f = s2f(value);
            if(
                ImGui::DragFloat(
                    p_ptr->name.c_str(), &value_f, 0.1f,
                    p_ptr->minValue, p_ptr->maxValue
                )
            ) {
                registerChange("object script vars change");
                value = f2s(value_f);
            }
            
            break;
            
        } case AEMP_TYPE_BOOL: {
    
            bool value_b = s2b(value);
            if(ImGui::Checkbox(p_ptr->name.c_str(), &value_b)) {
                registerChange("object script vars change");
                value = b2s(value_b);
            }
            
            break;
            
        } case AEMP_TYPE_LIST: {
    
            string value_s = value;
            if(ImGui::Combo(p_ptr->name, &value_s, p_ptr->valueList, 15)) {
                registerChange("object script vars change");
                value = value_s;
            }
            
            break;
            
        } case AEMP_TYPE_NR_LIST: {
    
            int item_idx = s2i(value);
            if(ImGui::Combo(p_ptr->name, &item_idx, p_ptr->valueList, 15)) {
                registerChange("object script vars change");
                value = i2s(item_idx);
            }
            
            break;
            
        }
        }
        
        setTooltip(
            wordWrap(p_ptr->tooltip, 50) +
            (p_ptr->tooltip.empty() ? "" : "\n") +
            "(Variable name: \"" + p_ptr->var + "\".)",
            "",
            (p_ptr->type == AEMP_TYPE_INT || p_ptr->type == AEMP_TYPE_FLOAT) ?
            WIDGET_EXPLANATION_DRAG :
            WIDGET_EXPLANATION_NONE
        );
        
        if(value != p_ptr->defValue) {
            new_vars_map[p_ptr->var] = value;
        }
        
        vars_in_widgets[p_ptr->var] = true;
        
    }
    
    string other_vars_str;
    for(auto const &v : vars_map) {
        if(!vars_in_widgets[v.first]) {
            other_vars_str += v.first + "=" + v.second + ";";
        }
    }
    
    m_ptr->vars.clear();
    for(auto const &v : new_vars_map) {
        m_ptr->vars += v.first + "=" + v.second + ";";
    }
    m_ptr->vars += other_vars_str;
    
    if(!m_ptr->vars.empty() && m_ptr->vars[m_ptr->vars.size() - 1] == ';') {
        m_ptr->vars.pop_back();
    }
    
    //Finally, a widget for the entire list.
    string mob_vars = m_ptr->vars;
    ImGui::Spacer();
    if(monoInputText("Full list", &mob_vars)) {
        registerChange("object script vars change");
        m_ptr->vars = mob_vars;
    }
    setTooltip(
        "This is the full list of script variables to use.\n"
        "You can add variables here, though variables in the "
        "wrong format will be removed.\n"
        "Format example: \"sleep=y;jumping=n\"."
    );
}


/**
 * @brief Processes the options dialog for this frame.
 */
void AreaEditor::processGuiOptionsDialog() {
    //Controls node.
    if(saveableTreeNode("options", "Controls")) {
    
        //Snap threshold value.
        int snap_threshold = (int) game.options.areaEd.snapThreshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Snap threshold", &snap_threshold,
            0.1f, 0, INT_MAX
        );
        setTooltip(
            "Cursor must be these many pixels close\n"
            "to a vertex/edge in order to snap there.\n"
            "Default: " +
            i2s(OPTIONS::AREA_ED_D::SNAP_THRESHOLD) + ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.areaEd.snapThreshold = snap_threshold;
        
        //Middle mouse button pans checkbox.
        ImGui::Checkbox("Use MMB to pan", &game.options.editors.mmbPan);
        setTooltip(
            "Use the middle mouse button to pan the camera\n"
            "(and RMB to reset camera/zoom).\n"
            "Default: " +
            b2s(OPTIONS::EDITORS_D::MMB_PAN) + "."
        );
        
        //Drag threshold value.
        int drag_threshold = (int) game.options.editors.mouseDragThreshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Drag threshold", &drag_threshold,
            0.1f, 0, INT_MAX
        );
        setTooltip(
            "Cursor must move these many pixels to be considered a drag.\n"
            "Default: " + i2s(OPTIONS::EDITORS_D::MOUSE_DRAG_THRESHOLD) +
            ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.editors.mouseDragThreshold = drag_threshold;
        
        ImGui::TreePop();
        
    }
    
    //View node.
    ImGui::Spacer();
    if(saveableTreeNode("options", "View")) {
    
        //Show edge length checkbox.
        ImGui::Checkbox(
            "Show edge length", &game.options.areaEd.showEdgeLength
        );
        setTooltip(
            "Show the length of nearby edges when drawing or moving vertexes.\n"
            "Default: " +
            b2s(OPTIONS::AREA_ED_D::SHOW_EDGE_LENGTH) + "."
        );
        
        //Show circular sector info checkbox.
        ImGui::Checkbox(
            "Show circular sector info",
            &game.options.areaEd.showCircularInfo
        );
        setTooltip(
            "Show the radius and number of vertexes of a circular sector\n"
            "when drawing one.\n"
            "Default: " +
            b2s(OPTIONS::AREA_ED_D::SHOW_CIRCULAR_INFO) + "."
        );
        
        //Show path link length checkbox.
        ImGui::Checkbox(
            "Show path link length",
            &game.options.areaEd.showPathLinkLength
        );
        setTooltip(
            "Show the length of nearby path links when drawing or\n"
            "moving path stops.\n"
            "Default: " +
            b2s(OPTIONS::AREA_ED_D::SHOW_PATH_LINK_LENGTH) + "."
        );
        
        //Show territory checkbox.
        ImGui::Checkbox(
            "Show territory/terrain radius",
            &game.options.areaEd.showTerritory
        );
        setTooltip(
            "Show the territory radius and terrain radius\n"
            "of the selected objects, when applicable.\n"
            "Default: " + b2s(OPTIONS::AREA_ED_D::SHOW_TERRITORY) +
            "."
        );
        
        //View mode text.
        int view_mode = game.options.areaEd.viewMode;
        ImGui::Text("View mode:");
        
        ImGui::Indent();
        
        //Textures view mode radio button.
        ImGui::RadioButton("Textures", &view_mode, VIEW_MODE_TEXTURES);
        setTooltip(
            "Draw textures on the sectors." +
            (string) (
                (
                    OPTIONS::AREA_ED_D::VIEW_MODE ==
                    VIEW_MODE_TEXTURES
                ) ?
                "\nThis is the default." :
                ""
            )
        );
        
        //Wireframe view mode radio button.
        ImGui::RadioButton("Wireframe", &view_mode, VIEW_MODE_WIREFRAME);
        setTooltip(
            "Do not draw sectors, only edges and vertexes.\n"
            "Best for performance." +
            (string) (
                (
                    OPTIONS::AREA_ED_D::VIEW_MODE ==
                    VIEW_MODE_WIREFRAME
                ) ?
                "This is the default." :
                ""
            )
        );
        
        //Heightmap view mode radio button.
        ImGui::RadioButton("Heightmap", &view_mode, VIEW_MODE_HEIGHTMAP);
        setTooltip(
            "Draw sectors as heightmaps. Lighter means taller." +
            (string) (
                (
                    OPTIONS::AREA_ED_D::VIEW_MODE ==
                    VIEW_MODE_HEIGHTMAP
                ) ?
                "This is the default." :
                ""
            )
        );
        
        //Brightness view mode radio button.
        ImGui::RadioButton("Brightness", &view_mode, VIEW_MODE_BRIGHTNESS);
        setTooltip(
            "Draw sectors as solid grays based on their brightness." +
            (string) (
                (
                    OPTIONS::AREA_ED_D::VIEW_MODE ==
                    VIEW_MODE_BRIGHTNESS
                ) ?
                "This is the default." :
                ""
            )
        );
        game.options.areaEd.viewMode = (VIEW_MODE) view_mode;
        
        ImGui::Unindent();
        
        ImGui::TreePop();
        
    }
    
    ImGui::Spacer();
    
    processGuiEditorStyle();
    
    ImGui::Spacer();
    
    //Misc. node.
    if(saveableTreeNode("options", "Misc.")) {
    
        //Interface mode text.
        ImGui::Text("Interface mode:");
        
        //Basic interface button.
        int interface_mode_i = (int) game.options.areaEd.advancedMode;
        ImGui::Indent();
        ImGui::RadioButton("Basic", &interface_mode_i, 0);
        setTooltip(
            "Only shows basic GUI items. Recommended for starters\n"
            "so that the interface isn't overwhelming. See the\n"
            "\"Advanced\" option's description for a list of such items."
        );
        
        //Advanced interface button.
        ImGui::RadioButton("Advanced", &interface_mode_i, 1);
        setTooltip(
            "Shows and enables some advanced GUI items:\n"
            "- Toolbar buttons (and shortcut keys) to quickly swap "
            "modes with.\n"
            "- Toolbar button to toggle preview mode with."
        );
        ImGui::Unindent();
        game.options.areaEd.advancedMode = (bool) interface_mode_i;
        
        //Selection transformation checkbox.
        ImGui::Checkbox(
            "Selection transformation", &game.options.areaEd.selTrans
        );
        setTooltip(
            "If true, when you select two or more vertexes, some handles\n"
            "will appear, allowing you to scale or rotate them together.\n"
            "Default: " + b2s(OPTIONS::AREA_ED_D::SEL_TRANS) + "."
        );
        
        //Grid interval text.
        ImGui::Text(
            "Grid interval: %i", (int) game.options.areaEd.gridInterval
        );
        
        //Increase grid interval button.
        ImGui::SameLine();
        if(
            ImGui::Button(
                "+",
                ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())
            )
        ) {
            gridIntervalIncreaseCmd(1.0f);
        }
        setTooltip(
            "Increase the spacing on the grid.\n"
            "Default: " + i2s(OPTIONS::AREA_ED_D::GRID_INTERVAL) +
            ".",
            "Shift + Plus"
        );
        
        //Decrease grid interval button.
        ImGui::SameLine();
        if(
            ImGui::Button(
                "-",
                ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())
            )
        ) {
            gridIntervalDecreaseCmd(1.0f);
        }
        setTooltip(
            "Decrease the spacing on the grid.\n"
            "Default: " + i2s(OPTIONS::AREA_ED_D::GRID_INTERVAL) +
            ".",
            "Shift + Minus"
        );
        
        //Auto-backup interval value.
        int backup_interval = game.options.areaEd.backupInterval;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Auto-backup interval", &backup_interval, 1, 0, INT_MAX
        );
        setTooltip(
            "Interval between auto-backup saves, in seconds. 0 = off.\n"
            "Default: " + i2s(OPTIONS::AREA_ED_D::BACKUP_INTERVAL) +
            ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.areaEd.backupInterval = backup_interval;
        
        //Undo limit value.
        size_t old_undo_limit = game.options.areaEd.undoLimit;
        int undo_limit = (int) game.options.areaEd.undoLimit;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Undo limit", &undo_limit, 0.1, 0, INT_MAX
        );
        setTooltip(
            "Maximum number of operations that can be undone. 0 = off.\n"
            "Default: " + i2s(OPTIONS::AREA_ED_D::UNDO_LIMIT) + ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.areaEd.undoLimit = undo_limit;
        
        if(game.options.areaEd.undoLimit != old_undo_limit) {
            updateUndoHistory();
        }
        
        ImGui::Spacer();
        
        ImGui::TreePop();
        
    }
}


/**
 * @brief Processes the Dear ImGui area details control panel for this frame.
 */
void AreaEditor::processGuiPanelDetails() {
    ImGui::BeginChild("details");
    
    if(subState == EDITOR_SUB_STATE_NEW_SHADOW) {
    
        //Creation explanation text.
        ImGui::TextWrapped(
            "Use the canvas to place a tree shadow. It'll appear where "
            "you click."
        );
        
        //Creation cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            setStatus();
            subState = EDITOR_SUB_STATE_NONE;
        }
        setTooltip(
            "Cancel the creation.",
            "Escape"
        );
        
    } else {
    
        //Back button.
        if(ImGui::Button("Back")) {
            changeState(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panelTitle("DETAILS");
        
        //Tree shadows node.
        if(saveableTreeNode("details", "Tree shadows")) {
        
            //New tree shadow button.
            if(
                ImGui::ImageButton(
                    "newShadowButton", editorIcons[EDITOR_ICON_ADD],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                newTreeShadowCmd(1.0f);
            }
            setTooltip(
                "Start creating a new tree shadow.\n"
                "Click on the canvas where you want the shadow to be.",
                "N"
            );
            
            //Delete shadow button.
            if(selectedShadow) {
                ImGui::SameLine();
                if(
                    ImGui::ImageButton(
                        "delShadowButton", editorIcons[EDITOR_ICON_REMOVE],
                        Point(EDITOR::ICON_BMP_SIZE)
                    )
                ) {
                    deleteTreeShadowCmd(1.0f);
                }
                setTooltip(
                    "Delete the selected tree shadow.",
                    "Delete"
                );
            }
            
            ImGui::Spacer();
            
            if(selectedShadow) {
            
                //Choose the tree shadow image button.
                if(ImGui::Button("Choose image...")) {
                    openBitmapDialog(
                    [this] (const string &bmp) {
                        if(bmp != selectedShadow->bmpName) {
                            //New image, delete the old one.
                            registerChange("tree shadow image change");
                            if(selectedShadow->bitmap != game.bmpError) {
                                game.content.bitmaps.list.free(
                                    selectedShadow->bmpName
                                );
                            }
                            selectedShadow->bmpName = bmp;
                            selectedShadow->bitmap =
                                game.content.bitmaps.list.get(
                                    selectedShadow->bmpName, nullptr, false
                                );
                        }
                        setStatus("Picked a tree shadow image successfully.");
                    },
                    FOLDER_NAMES::TEXTURES
                    );
                }
                setTooltip(
                    "Choose which texture to use from the game's content."
                );
                
                //Tree shadow image name text.
                ImGui::SameLine();
                monoText("%s", selectedShadow->bmpName.c_str());
                setTooltip("Internal name:\n" + selectedShadow->bmpName);
                
                //Tree shadow center value.
                Point shadow_center = selectedShadow->center;
                if(
                    ImGui::DragFloat2("Center", (float*) &shadow_center)
                ) {
                    registerChange("tree shadow center change");
                    selectedShadow->center = shadow_center;
                }
                setTooltip(
                    "Center coordinates of the tree shadow.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Tree shadow size value.
                Point shadow_size = selectedShadow->size;
                if(
                    processGuiSizeWidgets(
                        "Size", shadow_size,
                        1.0f, selectedShadowKeepAspectRatio, false,
                        -FLT_MAX
                    )
                ) {
                    registerChange("tree shadow size change");
                    selectedShadow->size = shadow_size;
                };
                setTooltip(
                    "Width and height of the tree shadow.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Tree shadow aspect ratio checkbox.
                ImGui::Indent();
                ImGui::Checkbox(
                    "Keep aspect ratio",
                    &selectedShadowKeepAspectRatio
                );
                ImGui::Unindent();
                setTooltip("Keep the aspect ratio when resizing the image.");
                
                //Tree shadow angle value.
                float shadow_angle = normalizeAngle(selectedShadow->angle);
                if(ImGui::SliderAngle("Angle", &shadow_angle, 0, 360, "%.2f")) {
                    registerChange("tree shadow angle change");
                    selectedShadow->angle = shadow_angle;
                }
                setTooltip(
                    "Angle of the tree shadow.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Tree shadow opacity value.
                int shadow_opacity = selectedShadow->alpha;
                if(ImGui::SliderInt("Opacity", &shadow_opacity, 0, 255)) {
                    registerChange("tree shadow opacity change");
                    selectedShadow->alpha = shadow_opacity;
                }
                setTooltip(
                    "How opaque the tree shadow is.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Tree shadow sway value.
                Point shadow_sway = selectedShadow->sway;
                if(ImGui::DragFloat2("Sway", (float*) &shadow_sway, 0.1)) {
                    registerChange("tree shadow sway change");
                    selectedShadow->sway = shadow_sway;
                }
                setTooltip(
                    "Multiply the amount of swaying by this much. 0 means "
                    "no swaying in that direction.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
            } else {
            
                //"No tree shadow selected" text.
                ImGui::TextDisabled("(No tree shadow selected)");
                
            }
            
            ImGui::TreePop();
            
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui edge control panel for this frame.
 */
void AreaEditor::processGuiPanelEdge() {
    Edge* e_ptr = *selectedEdges.begin();
    
    //Wall shadow node.
    if(saveableTreeNode("layout", "Wall shadow")) {
    
        //Length/presence text.
        ImGui::Text("Length and presence:");
        
        //Automatic length radio button.
        bool auto_length = (e_ptr->wallShadowLength == LARGE_FLOAT);
        if(ImGui::RadioButton("Automatic length", auto_length)) {
            if(!auto_length) {
                registerChange("edge shadow length change");
                e_ptr->wallShadowLength = LARGE_FLOAT;
                quickPreviewTimer.start();
            }
            auto_length = true;
        }
        setTooltip(
            "The wall shadow's length will depend "
            "on the height of the wall.\n"
            "If it's too short, the wall shadow will also "
            "automatically disappear."
        );
        
        //Never show radio button.
        bool no_length = (e_ptr->wallShadowLength == 0.0f);
        if(ImGui::RadioButton("Never show", no_length)) {
            if(!no_length) {
                registerChange("edge shadow length change");
                e_ptr->wallShadowLength = 0.0f;
                quickPreviewTimer.start();
            }
            no_length = true;
        }
        setTooltip(
            "The wall shadow will never appear, no matter what."
        );
        
        //Fixed length radio button.
        bool fixed_length = (!no_length && !auto_length);
        if(ImGui::RadioButton("Fixed length", fixed_length)) {
            if(!fixed_length) {
                registerChange("edge shadow length change");
                e_ptr->wallShadowLength = 30.0f;
                quickPreviewTimer.start();
            }
            fixed_length = true;
        }
        setTooltip(
            "The wall shadow will always appear, and will "
            "have a fixed length regardless of the wall's height."
        );
        
        //Length value.
        if(fixed_length) {
            float length = e_ptr->wallShadowLength;
            if(
                ImGui::DragFloat(
                    "Length", &length, 0.2f,
                    GEOMETRY::SHADOW_MIN_LENGTH, GEOMETRY::SHADOW_MAX_LENGTH
                )
            ) {
                registerChange("edge shadow length change");
                e_ptr->wallShadowLength = length;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Length of the shadow.",
                "", WIDGET_EXPLANATION_DRAG
            );
        }
        
        //Shadow color.
        ALLEGRO_COLOR color = e_ptr->wallShadowColor;
        ImGui::Spacer();
        if(
            ImGui::ColorEdit4(
                "Color", (float*) &color,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            registerChange("edge shadow color change");
            e_ptr->wallShadowColor = color;
            quickPreviewTimer.start();
        }
        setTooltip(
            "Color of the shadow, opacity included. "
            "This is the color\n"
            "closest to the wall, since it becomes more "
            "transparent as it goes out."
        );
        
        ImGui::TreePop();
    }
    
    //Ledge smoothing node.
    ImGui::Spacer();
    if(saveableTreeNode("layout", "Ledge smoothing")) {
    
        //Length value.
        float length = e_ptr->ledgeSmoothingLength;
        if(
            ImGui::DragFloat(
                "Length", &length, 0.2f,
                0.0f, GEOMETRY::SMOOTHING_MAX_LENGTH
            )
        ) {
            registerChange("edge ledge smoothing length change");
            e_ptr->ledgeSmoothingLength = length;
            quickPreviewTimer.start();
        }
        setTooltip(
            "Length of the ledge smoothing effect.\n"
            "Use this to make a ledge leading into a wall look more rounded.\n"
            "0 means there will be no effect.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Smoothing color.
        ALLEGRO_COLOR color = e_ptr->ledgeSmoothingColor;
        ImGui::Spacer();
        if(
            ImGui::ColorEdit4(
                "Color", (float*) &color,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            registerChange("edge ledge smoothing color change");
            e_ptr->ledgeSmoothingColor = color;
            quickPreviewTimer.start();
        }
        setTooltip(
            "Color of the ledge smoothing effect, opacity included. "
            "This is the color\n"
            "closest to the edge, since it becomes more "
            "transparent as it goes out."
        );
        
        ImGui::TreePop();
    }
    
    homogenizeSelectedEdges();
    updateAllEdgeOffsetCaches();
    
}


/**
 * @brief Processes the Dear ImGui area gameplay settings control panel for
 * this frame.
 */
void AreaEditor::processGuiPanelGameplay() {
    ImGui::BeginChild("gameplay");
    
    switch(subState) {
    case EDITOR_SUB_STATE_MISSION_EXIT: {

        //Instructions text.
        ImGui::TextWrapped(
            "Use the handles on the canvas to control where the exit region is."
        );
        
        //Region center text.
        ImGui::Text(
            "Exit region center: %s,%s",
            f2s(game.curAreaData->mission.goalExitCenter.x).c_str(),
            f2s(game.curAreaData->mission.goalExitCenter.y).c_str()
        );
        
        //Region center text.
        ImGui::Text(
            "Exit region size: %s x %s",
            f2s(game.curAreaData->mission.goalExitSize.x).c_str(),
            f2s(game.curAreaData->mission.goalExitSize.y).c_str()
        );
        
        //Finish button.
        if(ImGui::Button("Finish")) {
            subState = EDITOR_SUB_STATE_NONE;
        }
        setTooltip("Click here to finish.");
        
        break;
        
    }
    default: {

        //Back button.
        if(ImGui::Button("Back")) {
            changeState(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panelTitle("GAMEPLAY");
        
        //Sprays node.
        ImGui::Spacer();
        if(saveableTreeNode("gameplay", "Starting sprays")) {
        
            map<string, string> spray_strs =
                getVarMap(game.curAreaData->sprayAmounts);
            for(size_t s = 0; s < game.config.misc.sprayOrder.size(); s++) {
                string spray_internal_name =
                    game.config.misc.sprayOrder[s]->manifest->internalName;
                int amount = s2i(spray_strs[spray_internal_name]);
                ImGui::SetNextItemWidth(50);
                if(
                    ImGui::DragInt(
                        game.config.misc.sprayOrder[s]->name.c_str(), &amount,
                        0.1, 0, INT_MAX
                    )
                ) {
                    registerChange("area spray amounts change");
                    spray_strs[spray_internal_name] = i2s(amount);
                    game.curAreaData->sprayAmounts.clear();
                    for(auto const &v : spray_strs) {
                        game.curAreaData->sprayAmounts +=
                            v.first + "=" + v.second + ";";
                    }
                }
                setTooltip(
                    "Starting amount of spray dosages to give the player.", "",
                    WIDGET_EXPLANATION_DRAG
                );
                
            }
            
            ImGui::TreePop();
        }
        
        ImGui::Spacer();
        
        if(game.curAreaData->type == AREA_TYPE_MISSION) {
            processGuiPanelMission();
        }
        
        break;
    }
    
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui area info control panel for this frame.
 */
void AreaEditor::processGuiPanelInfo() {
    ImGui::BeginChild("info");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("INFO");
    
    //General node.
    if(saveableTreeNode("info", "General")) {
    
        //Area name input.
        string name = game.curAreaData->name;
        if(ImGui::InputText("Name", &name)) {
            registerChange("area name change");
            game.curAreaData->name = name;
        }
        setTooltip(
            "Name of the area."
        );
        
        //Area subtitle input.
        string subtitle = game.curAreaData->subtitle;
        if(ImGui::InputText("Subtitle", &subtitle)) {
            registerChange("area subtitle change");
            game.curAreaData->subtitle = subtitle;
        }
        setTooltip(
            "Subtitle, if any. Appears on the loading screen."
        );
        
        //Area description input.
        string description = game.curAreaData->description;
        if(ImGui::InputText("Description", &description)) {
            registerChange("area description change");
            game.curAreaData->description = description;
        }
        setTooltip(
            "A general description about the area, like what the player "
            "does here."
        );
        
        //Add area tags button.
        if(
            ImGui::Button(
                "+",
                ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())
            )
        ) {
            ImGui::OpenPopup("addTags");
        }
        setTooltip(
            "Add tags from a list of recommended tags.\n"
            "You can still add your own, of course."
        );
        
        //Add area tags popup.
        if(popup("addTags")) {
        
            string new_tag;
            
            //Gameplay tags combo.
            vector<string> gameplay_tags = {
                "Standard",
                "Puzzle",
                "Short and sweet",
                "Exploration",
                "Battle",
                "Challenge",
                "Gimmick",
                "Role-playing",
                "Custom game mode",
            };
            int gameplay_tag_idx = -1;
            if(ImGui::Combo("Gameplay", &gameplay_tag_idx, gameplay_tags, 15)) {
                new_tag = gameplay_tags[gameplay_tag_idx];
            }
            
            //Theme tags combo.
            vector<string> theme_tags = {
                "Autumn",
                "Beach",
                "Cave",
                "Concrete",
                "Desert",
                "Forest",
                "Garden",
                "House",
                "Lakeside",
                "Man-made",
                "Metal",
                "Snow",
                "Swamp",
                "Tiles",
                "Toys",
            };
            int theme_tag_idx = -1;
            if(ImGui::Combo("Theme", &theme_tag_idx, theme_tags, 15)) {
                new_tag = theme_tags[theme_tag_idx];
            }
            
            //Misc. tags combo.
            vector<string> misc_tags = {
                "Art",
                "Technical",
                "Troll",
                "Tutorial",
            };
            int misc_tag_idx = -1;
            if(ImGui::Combo("Misc.", &misc_tag_idx, misc_tags, 15)) {
                new_tag = misc_tags[misc_tag_idx];
            }
            
            if(!new_tag.empty()) {
                registerChange("area tags change");
                if(!game.curAreaData->tags.empty()) {
                    game.curAreaData->tags += "; ";
                }
                game.curAreaData->tags += new_tag;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        
        //Area tags input.
        ImGui::SameLine();
        string tags = game.curAreaData->tags;
        if(ImGui::InputText("Tags", &tags)) {
            registerChange("area tags change");
            game.curAreaData->tags = tags;
        }
        setTooltip(
            "Short keywords that describe the area, separated by semicolon.\n"
            "Example: \"Beach; Gimmick; Short and sweet\""
        );
        
        //Difficulty value.
        int difficulty = game.curAreaData->difficulty;
        vector<string> difficulty_options = {
            "Not specified",
            "1 - Very easy",
            "2 - Easy",
            "3 - Medium",
            "4 - Hard",
            "5 - Very hard"
        };
        if(ImGui::Combo("Difficulty", &difficulty, difficulty_options, 15)) {
            registerChange("difficulty change");
            game.curAreaData->difficulty = difficulty;
        }
        setTooltip(
            "How hard this area is. This is very subjective, and only\n"
            "serves as a way to tell players if this area is something\n"
            "relaxed and easy (1), or if it's something that only the\n"
            "most experienced Pikmin veterans can handle (5).\n"
            "Or anything in between.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::TreePop();
    }
    
    //Ambiance node.
    ImGui::Spacer();
    if(saveableTreeNode("info", "Ambiance")) {
    
        //Preview song button.
        bool valid_song_selected =
            !game.curAreaData->songName.empty() &&
            game.curAreaData->songName != NONE_OPTION;
        bool previewing =
            !previewSong.empty();
        bool can_preview_selected_song =
            valid_song_selected &&
            previewSong != game.curAreaData->songName;
        bool can_stop_previewing =
            previewing &&
            (
                !valid_song_selected ||
                previewSong == game.curAreaData->songName
            );
        bool preview_button_valid =
            can_preview_selected_song || can_stop_previewing;
            
        if(!preview_button_valid) ImGui::BeginDisabled();
        
        if(
            ImGui::ImageButton(
                "previewSongButton",
                can_stop_previewing ?
                editorIcons[EDITOR_ICON_STOP] :
                editorIcons[EDITOR_ICON_PLAY],
                Point(ImGui::GetTextLineHeight())
            )
        ) {
            if(can_preview_selected_song) {
                previewSong = game.curAreaData->songName;
                game.audio.setCurrentSong(previewSong);
                previewing = true;
            } else if(can_stop_previewing) {
                game.audio.setCurrentSong(game.sysContentNames.sngEditors, false);
                previewSong.clear();
                previewing = false;
            }
        }
        
        if(!preview_button_valid) ImGui::EndDisabled();
        
        string preview_tooltip_str;
        if(previewing) {
            preview_tooltip_str +=
                "Currently previewing the song \"" +
                game.content.songs.list[previewSong].name +
                "\".\n";
        }
        if(can_preview_selected_song) {
            preview_tooltip_str +=
                "Click here to preview the song \"" +
                game.content.songs.list[game.curAreaData->songName].name +
                "\".";
        } else if(can_stop_previewing) {
            preview_tooltip_str +=
                "Click here to stop.";
        } else {
            preview_tooltip_str +=
                "If you select a song, you can click here to preview it.";
        }
        setTooltip(preview_tooltip_str);
        
        //Music combobox.
        ImGui::SameLine();
        vector<string> song_internals;
        vector<string> song_names;
        song_internals.push_back("");
        song_names.push_back(NONE_OPTION);
        for(auto &s : game.content.songs.list) {
            song_internals.push_back(s.first);
            song_names.push_back(s.second.name);
        }
        string song_name = game.curAreaData->songName;
        if(ImGui::Combo("Song", &song_name, song_internals, song_names, 15)) {
            registerChange("area song change");
            game.curAreaData->songName = song_name;
        }
        setTooltip(
            "What song to play."
        );
        
        //Area weather combobox.
        vector<string> weather_cond_internals;
        vector<string> weather_cond_names;
        weather_cond_internals.push_back("");
        weather_cond_names.push_back(NONE_OPTION);
        for(auto &w : game.content.weatherConditions.list) {
            weather_cond_internals.push_back(w.first);
            weather_cond_names.push_back(w.second.name);
        }
        string weather_name = game.curAreaData->weatherName;
        if(
            ImGui::Combo(
                "Weather", &weather_name,
                weather_cond_internals, weather_cond_names, 15
            )
        ) {
            registerChange("area weather change");
            game.curAreaData->weatherName = weather_name;
        }
        setTooltip(
            "The weather condition to use."
        );
        
        ImGui::Spacer();
        
        bool has_time_limit = false;
        float mission_min = 0;
        if(game.curAreaData->type == AREA_TYPE_MISSION) {
            if(
                game.curAreaData->mission.goal == MISSION_GOAL_TIMED_SURVIVAL
            ) {
                has_time_limit = true;
                mission_min =
                    game.curAreaData->mission.goalAmount / 60.0f;
            } else if(
                hasFlag(
                    game.curAreaData->mission.failConditions,
                    getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
                )
            ) {
                has_time_limit = true;
                mission_min =
                    game.curAreaData->mission.failTimeLimit / 60.0f;
            }
        }
        int day_start_min = (int) game.curAreaData->dayTimeStart;
        day_start_min = wrapFloat(day_start_min, 0, 60 * 24);
        float day_speed = game.curAreaData->dayTimeSpeed;
        int day_end_min = (int) (day_start_min + mission_min * day_speed);
        day_end_min = wrapFloat(day_end_min, 0, 60 * 24);
        
        //Area day time at start value.
        if(
            ImGui::DragTime2(
                "Start day time", &day_start_min, "h", "m", 23, 59
            )
        ) {
            registerChange("day time change");
            game.curAreaData->dayTimeStart = day_start_min;
            if(has_time_limit) {
                day_speed =
                    calculateDaySpeed(
                        day_start_min, day_end_min, mission_min
                    );
                game.curAreaData->dayTimeSpeed = day_speed;
            }
        }
        setTooltip(
            "Point of the (game world) day at which gameplay starts.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        if(has_time_limit) {
            //Area day time at end value.
            if(
                ImGui::DragTime2(
                    "End day time", &day_end_min, "h", "m", 23, 59
                )
            ) {
                registerChange("day time change");
                day_speed =
                    calculateDaySpeed(
                        day_start_min, day_end_min, mission_min
                    );
                game.curAreaData->dayTimeSpeed = day_speed;
            }
            setTooltip(
                "Point of the (game world) day at which gameplay ends.\n"
                "Only applicable in missions with some sort of time limits.\n"
                "Set this to the same as the area start time to make\n"
                "the day time frozen.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
        } else {
        
            //Area day time speed value.
            ImGui::SetNextItemWidth(165);
            if(
                ImGui::DragFloat(
                    "Day time speed", &day_speed, 0.1f, 0.0f, FLT_MAX
                )
            ) {
                registerChange("day time change");
                game.curAreaData->dayTimeSpeed = day_speed;
            }
            setTooltip(
                "Speed at which the (game world) day passes.\n"
                "60 means 1 game-world-hour goes by in 1 real-world-minute.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
        }
        
        ImGui::TreePop();
    }
    
    //Thumbnail node.
    ImGui::Spacer();
    if(saveableTreeNode("info", "Thumbnail")) {
    
        //Remove thumbnail button.
        unsigned char rem_thumb_opacity =
            !game.curAreaData->thumbnail ? 50 : 255;
        if(
            ImGui::ImageButton(
                "remThumbButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                COLOR_EMPTY, mapAlpha(rem_thumb_opacity)
            ) &&
            game.curAreaData->thumbnail
        ) {
            registerChange("area thumbnail removal");
            removeThumbnail();
            thumbnailNeedsSaving = true;
            thumbnailBackupNeedsSaving = true;
        }
        setTooltip(
            "Remove the current thumbnail, if any."
        );
        
        //Thumbnail browse button.
        ImGui::SameLine();
        if(ImGui::Button("Browse...")) {
            vector<string> f =
                promptFileDialog(
                    "",
                    "Please choose an image to copy over and "
                    "use as the thumbnail.",
                    "*.jpg;*.png",
                    ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                    ALLEGRO_FILECHOOSER_PICTURES,
                    game.display
                );
                
            if(!f.empty() && !f[0].empty()) {
                registerChange("area thumbnail change");
                removeThumbnail();
                game.curAreaData->loadThumbnail(f[0]);
                thumbnailNeedsSaving = true;
                thumbnailBackupNeedsSaving = true;
            }
            
        }
        setTooltip(
            "Press the Browse... button to set the area's thumbnail from\n"
            "a file on your disk. When you save the area, the thumbnail\n"
            "gets saved into thumbnail.png in the area's folder, \n"
            "but the original file you selected with the\n"
            "'Browse...' button will be left untouched."
        );
        
        //Current thumbnail text.
        //This needs to come after everything else, because the previous buttons
        //could delete the bitmap after we already told Dear ImGui that it
        //would be drawing it.
        ImGui::Text("Current thumbnail:");
        
        if(!game.curAreaData->thumbnail) {
            //No thumbnail text.
            ImGui::Text("None");
        } else {
            //Thumbnail image.
            Point size =
                resizeToBoxKeepingAspectRatio(
                    getBitmapDimensions(game.curAreaData->thumbnail.get()),
                    Point(200.0f)
                );
            ImGui::Image(game.curAreaData->thumbnail.get(), size);
        }
        
        ImGui::TreePop();
    }
    
    //Background node.
    ImGui::Spacer();
    if(saveableTreeNode("info", "Background")) {
    
        //Remove background texture button.
        unsigned char rem_bg_opacity =
            game.curAreaData->bgBmpName.empty() ? 50 : 255;
        if(
            ImGui::ImageButton(
                "remBgButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                COLOR_EMPTY, mapAlpha(rem_bg_opacity)
            ) &&
            !game.curAreaData->bgBmpName.empty()
        ) {
            registerChange("area background removal");
            game.curAreaData->bgBmpName.clear();
            setStatus("Removed the background image successfully.");
        }
        setTooltip(
            "Remove the background image for the area."
        );
        
        //Choose background texture button.
        ImGui::SameLine();
        if(ImGui::Button("Choose image...")) {
            openBitmapDialog(
            [this] (const string &bmp) {
                registerChange("area background change");
                game.curAreaData->bgBmpName = bmp;
                setStatus("Picked a background image successfully.");
            },
            FOLDER_NAMES::TEXTURES
            );
        }
        setTooltip(
            "Choose which background image to use from the game's content.\n"
            "This repeating texture can be seen when looking at the void."
        );
        
        //Background image name text.
        ImGui::SameLine();
        monoText("%s", game.curAreaData->bgBmpName.c_str());
        setTooltip("Internal name:\n" + game.curAreaData->bgBmpName);
        
        //Background color value.
        ALLEGRO_COLOR bg_color = game.curAreaData->bgColor;
        if(
            ImGui::ColorEdit4(
                "Void color", (float*) &bg_color,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            registerChange("area background color change");
            game.curAreaData->bgColor = bg_color;
        }
        setTooltip(
            "Set the color of the void. If you have a background image,\n"
            "this will appear below it."
        );
        
        //Background distance value.
        float bg_dist = game.curAreaData->bgDist;
        if(ImGui::DragFloat("Distance", &bg_dist)) {
            registerChange("area background distance change");
            game.curAreaData->bgDist = bg_dist;
        }
        setTooltip(
            "How far away the background texture is. "
            "Affects paralax scrolling.\n"
            "2 is a good value.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Background zoom value.
        float bg_bmp_zoom = game.curAreaData->bgBmpZoom;
        if(ImGui::DragFloat("Zoom", &bg_bmp_zoom, 0.01)) {
            registerChange("area background zoom change");
            game.curAreaData->bgBmpZoom = bg_bmp_zoom;
        }
        setTooltip(
            "Scale the texture by this amount.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::TreePop();
    }
    
    //Metadata node.
    ImGui::Spacer();
    if(saveableTreeNode("info", "Metadata")) {
    
        //Maker input.
        string maker = game.curAreaData->maker;
        if(ImGui::InputText("Maker", &maker)) {
            registerChange("area maker change");
            game.curAreaData->maker = maker;
        }
        setTooltip(
            "Name (or nickname) of who made this area. "
            "Optional."
        );
        
        //Version input.
        string version = game.curAreaData->version;
        if(monoInputText("Version", &version)) {
            registerChange("area version change");
            game.curAreaData->version = version;
        }
        setTooltip(
            "Version of the area, preferably in the \"X.Y.Z\" format. "
            "Optional."
        );
        
        //Maker notes input.
        string maker_notes = game.curAreaData->makerNotes;
        if(ImGui::InputText("Maker notes", &maker_notes)) {
            registerChange("area maker notes change");
            game.curAreaData->makerNotes = maker_notes;
        }
        setTooltip(
            "Extra notes or comments about the area for other makers to see. "
            "Optional."
        );
        
        //Notes input.
        string notes = game.curAreaData->notes;
        if(ImGui::InputText("Notes", &notes)) {
            registerChange("area notes change");
            game.curAreaData->notes = notes;
        }
        setTooltip(
            "Extra notes or comments of any kind. "
            "Optional."
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui layout control panel for this frame.
 */
void AreaEditor::processGuiPanelLayout() {
    ImGui::BeginChild("main");
    
    if(subState == EDITOR_SUB_STATE_DRAWING) {
        //Drawing explanation text.
        ImGui::TextWrapped(
            "Use the canvas to draw your layout. Each click places a vertex. "
            "You either draw edges from one edge/vertex to another "
            "edge/vertex, or draw a sector's shape and finish on the "
            "starting vertex."
        );
        
        //Drawing cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            clearLayoutDrawing();
            cancelLayoutDrawing();
        }
        setTooltip(
            "Cancel the drawing.",
            "Escape"
        );
        
    } else if(subState == EDITOR_SUB_STATE_CIRCLE_SECTOR) {
        //Drawing explanation text.
        ImGui::TextWrapped(
            "Use the canvas to draw a circle sector. First, click to choose "
            "the sector's center. Then, choose how large the circle is. "
            "Finally, choose how many edges it'll have."
        );
        
        //Drawing cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            clearCircleSector();
            cancelCircleSector();
        }
        setTooltip(
            "Cancel the drawing.",
            "Escape"
        );
        
    } else if(subState == EDITOR_SUB_STATE_QUICK_HEIGHT_SET) {
        //Explanation text.
        ImGui::TextWrapped(
            "Move the cursor up or down to change the sector's height. "
            "Release the key to return to normal."
        );
        
    } else {
    
        //Back button.
        if(ImGui::Button("Back")) {
            changeState(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panelTitle("LAYOUT");
        
        //New sector button.
        if(
            ImGui::ImageButton(
                "newSectorButton", editorIcons[EDITOR_ICON_ADD],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            layoutDrawingCmd(1.0f);
        }
        setTooltip(
            "Start drawing a new sector.\n"
            "Click on the canvas to draw the lines that make up the sector.",
            "N"
        );
        
        //New circle sector button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "newCircleButton", editorIcons[EDITOR_ICON_ADD_CIRCLE_SECTOR],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            circleSectorCmd(1.0f);
        }
        setTooltip(
            "Start creating a new circular sector.\n"
            "Click on the canvas to set the center, then radius, then the "
            "number of edges.",
            "C"
        );
        
        //Delete edges button.
        if(!selectedEdges.empty()) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delEdgesButton", editorIcons[EDITOR_ICON_REMOVE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                deleteEdgeCmd(1.0f);
            }
            setTooltip(
                "Delete the selected edges.\n"
                "Sectors without any edges left get deleted too.\n"
                "Sectors that would end up with edge gaps also get deleted.\n"
                "If you delete an edge between two sectors,\n"
                "the smallest will merge into the largest.",
                "Delete"
            );
        }
        
        //Selection filter button.
        ALLEGRO_BITMAP* sel_filter_bmp = nullptr;
        string sel_filter_description;
        switch(selectionFilter) {
        case SELECTION_FILTER_VERTEXES: {
            sel_filter_bmp = editorIcons[EDITOR_ICON_VERTEXES];
            sel_filter_description = "vertexes only";
            break;
        } case SELECTION_FILTER_EDGES: {
            sel_filter_bmp = editorIcons[EDITOR_ICON_EDGES];
            sel_filter_description = "edges + vertexes";
            break;
        } case SELECTION_FILTER_SECTORS: {
            sel_filter_bmp = editorIcons[EDITOR_ICON_SECTORS];
            sel_filter_description = "sectors + edges + vertexes";
            break;
        } case N_SELECTION_FILTERS: {
            break;
        }
        }
        
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "selFilterButton", sel_filter_bmp,
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            selectionFilterCmd(1.0f);
        }
        setTooltip(
            "Current selection filter: " + sel_filter_description + ".\n"
            "When selecting things in the canvas, only these will "
            "become selected.",
            "F or Shift + F"
        );
        
        //Clear selection button.
        if(
            !selectedSectors.empty() ||
            !selectedEdges.empty() ||
            !selectedVertexes.empty()
        ) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "clearSelButton", editorIcons[EDITOR_ICON_SELECT_NONE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                clearSelection();
            }
            setTooltip(
                "Clear the selection.",
                "Escape"
            );
        }
        
        //Sectors/edges tabs.
        ImGui::Spacer();
        if(ImGui::BeginTabBar("tabTabs")) {
        
            //Sectors tab.
            if(ImGui::BeginTabItem("Sectors")) {
            
                if(layoutMode == LAYOUT_MODE_EDGES) {
                    //If the user homogenized the edges, then
                    //selection_homogenized is true. But the sectors aren't
                    //homogenized, so reset the variable back to false.
                    selectionHomogenized = false;
                }
                
                layoutMode = LAYOUT_MODE_SECTORS;
                
                if(selectedSectors.size() == 1 || selectionHomogenized) {
                    processGuiPanelSector();
                    
                } else if(selectedSectors.empty()) {
                
                    //"No sector selected" text.
                    ImGui::TextDisabled("(No sector selected)");
                    
                } else {
                
                    //Non-homogenized sectors warning.
                    ImGui::TextWrapped(
                        "Multiple different sectors selected. "
                        "To make all their properties the same "
                        "and edit them all together, click here:"
                    );
                    
                    //Homogenize sectors button.
                    if(ImGui::Button("Edit all together")) {
                        registerChange("sector combining");
                        selectionHomogenized = true;
                        homogenizeSelectedSectors();
                    }
                }
                
                ImGui::EndTabItem();
            }
            
            //Edges tab.
            if(ImGui::BeginTabItem("Edges", nullptr)) {
            
                layoutMode = LAYOUT_MODE_EDGES;
                
                if(selectedEdges.size() == 1 || selectionHomogenized) {
                    processGuiPanelEdge();
                    
                } else if(selectedEdges.empty()) {
                
                    //"No edge selected" text.
                    ImGui::TextDisabled("(No edge selected)");
                    
                } else {
                
                    //Non-homogenized edges warning.
                    ImGui::TextWrapped(
                        "Multiple different edges selected. "
                        "To make all their properties the same "
                        "and edit them all together, click here:"
                    );
                    
                    //Homogenize edges button.
                    if(ImGui::Button("Edit all together")) {
                        registerChange("edge combining");
                        selectionHomogenized = true;
                        homogenizeSelectedEdges();
                    }
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui main control panel for this frame.
 */
void AreaEditor::processGuiPanelMain() {
    if(manifest.internalName.empty() || !game.curAreaData) return;
    
    ImGui::BeginChild("main");
    
    //Current folder header text.
    ImGui::Text("Folder: ");
    
    //Current folder text.
    ImGui::SameLine();
    monoText("%s", manifest.internalName.c_str());
    string folder_tooltip =
        getFolderTooltip(manifest.path, game.curAreaData->userDataPath) +
        "\n\n"
        "Folder state: ";
    if(!changesMgr.existsOnDisk()) {
        folder_tooltip += "Not saved to disk yet!";
    } else if(changesMgr.hasUnsavedChanges()) {
        folder_tooltip += "You have unsaved changes.";
    } else {
        folder_tooltip += "Everything ok.";
    }
    setTooltip(folder_tooltip);
    
    //Layout button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "layoutButton", editorIcons[EDITOR_ICON_SECTORS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Layout"
        )
    ) {
        changeState(EDITOR_STATE_LAYOUT);
    }
    setTooltip(
        "Draw sectors (polygons) to create the area's layout."
    );
    
    //Objects button.
    if(
        ImGui::ImageButtonAndText(
            "mobsButton", editorIcons[EDITOR_ICON_MOBS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Objects"
        )
    ) {
        changeState(EDITOR_STATE_MOBS);
    }
    setTooltip(
        "Change object settings and placements."
    );
    
    //Paths button.
    if(
        ImGui::ImageButtonAndText(
            "pathsButton", editorIcons[EDITOR_ICON_PATHS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Paths"
        )
    ) {
        changeState(EDITOR_STATE_PATHS);
    }
    setTooltip(
        "Draw movement paths, and their stops."
    );
    
    //Details button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "detailsButton", editorIcons[EDITOR_ICON_DETAILS],
            Point(EDITOR::ICON_BMP_SIZE),
            12.0f, "Details"
        )
    ) {
        changeState(EDITOR_STATE_DETAILS);
    }
    setTooltip(
        "Edit misc. details, like tree shadows."
    );
    
    //Area info button.
    if(
        ImGui::ImageButtonAndText(
            "infoButton", editorIcons[EDITOR_ICON_INFO],
            Point(EDITOR::ICON_BMP_SIZE),
            12.0f, "Info"
        )
    ) {
        changeState(EDITOR_STATE_INFO);
    }
    setTooltip(
        "Set the area's name, weather, and other basic information here."
    );
    
    //Area gameplay settings button.
    if(
        ImGui::ImageButtonAndText(
            "gameplayButton", editorIcons[EDITOR_ICON_GAMEPLAY],
            Point(EDITOR::ICON_BMP_SIZE),
            12.0f, "Gameplay settings"
        )
    ) {
        changeState(EDITOR_STATE_GAMEPLAY);
    }
    setTooltip(
        "Specify how the player's gameplay experience in this area will be."
    );
    
    //Review button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "reviewButton", editorIcons[EDITOR_ICON_REVIEW],
            Point(EDITOR::ICON_BMP_SIZE),
            8.0f, "Review"
        )
    ) {
        changeState(EDITOR_STATE_REVIEW);
    }
    setTooltip(
        "Use this to make sure everything is okay with the area."
    );
    
    //Tools button.
    if(
        ImGui::ImageButtonAndText(
            "toolsButton", editorIcons[EDITOR_ICON_TOOLS],
            Point(EDITOR::ICON_BMP_SIZE),
            8.0f, "Tools"
        )
    ) {
        changeState(EDITOR_STATE_TOOLS);
    }
    setTooltip(
        "Special tools to help you make the area."
    );
    
    ImGui::Spacer();
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMission() {

    float old_mission_survival_min =
        game.curAreaData->mission.goalAmount / 60.0f;
    float old_mission_time_limit_min =
        game.curAreaData->mission.failTimeLimit / 60.0f;
    bool day_duration_needs_update = false;
    
    //Mission goal node.
    if(saveableTreeNode("gameplay", "Mission goal")) {
    
        //Goal combobox.
        vector<string> goals_list;
        for(size_t g = 0; g < game.missionGoals.size(); g++) {
            goals_list.push_back(game.missionGoals[g]->getName());
        }
        int mission_goal = game.curAreaData->mission.goal;
        if(ImGui::Combo("Goal", &mission_goal, goals_list, 15)) {
            registerChange("mission requirements change");
            game.curAreaData->mission.goalMobIdxs.clear();
            game.curAreaData->mission.goalAmount = 1;
            game.curAreaData->mission.goal = (MISSION_GOAL) mission_goal;
            if(
                game.curAreaData->mission.goal ==
                MISSION_GOAL_TIMED_SURVIVAL
            ) {
                day_duration_needs_update = true;
            }
        }
        
        switch(game.curAreaData->mission.goal) {
        case MISSION_GOAL_END_MANUALLY: {
    
            //Explanation text.
            ImGui::TextWrapped(
                "The player has no real goal. They just play until they have "
                "had enough, at which point they must end from the pause menu."
            );
            
            break;
            
        }
        case MISSION_GOAL_COLLECT_TREASURE: {
    
            processGuiPanelMissionGoalCt();
            break;
            
        }
        case MISSION_GOAL_BATTLE_ENEMIES: {
    
            processGuiPanelMissionGoalBe();
            break;
            
        }
        case MISSION_GOAL_TIMED_SURVIVAL: {
    
            //Explanation text.
            ImGui::TextWrapped(
                "The player must survive for a certain amount of time."
            );
            
            //Time values.
            ImGui::Spacer();
            int total_seconds =
                (int) game.curAreaData->mission.goalAmount;
            if(ImGui::DragTime2("Time", &total_seconds)) {
                registerChange("mission requirements change");
                total_seconds = std::max(total_seconds, 1);
                game.curAreaData->mission.goalAmount =
                    (size_t) total_seconds;
                day_duration_needs_update = true;
            }
            setTooltip(
                "The total survival time.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            break;
            
        }
        case MISSION_GOAL_GET_TO_EXIT: {
    
            processGuiPanelMissionGoalGte();
            break;
            
        }
        case MISSION_GOAL_GROW_PIKMIN: {
    
            //Explanation text.
            ImGui::TextWrapped(
                "The player must reach or surpass a certain number of "
                "total Pikmin."
            );
            
            //Pikmin amount value.
            ImGui::Spacer();
            int amount =
                (int) game.curAreaData->mission.goalAmount;
            ImGui::SetNextItemWidth(80);
            if(ImGui::DragInt("Amount", &amount, 0.1f, 1, INT_MAX)) {
                registerChange("mission requirements change");
                game.curAreaData->mission.goalAmount =
                    (size_t) amount;
            }
            setTooltip(
                "The total Pikmin amount requirement.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            break;
            
        }
        }
        
        ImGui::TreePop();
        
    }
    
    //Mission fail conditions node.
    ImGui::Spacer();
    if(saveableTreeNode("gameplay", "Mission fail conditions")) {
    
        processGuiPanelMissionFail(&day_duration_needs_update);
        ImGui::TreePop();
    }
    
    //Mission grading node.
    ImGui::Spacer();
    if(saveableTreeNode("gameplay", "Mission grading")) {
    
        processGuiPanelMissionGrading();
        ImGui::TreePop();
        
    }
    
    if(day_duration_needs_update) {
        float day_start_min = game.curAreaData->dayTimeStart;
        day_start_min = wrapFloat(day_start_min, 0, 60 * 24);
        float day_speed = game.curAreaData->dayTimeSpeed;
        float old_mission_min = 0;
        size_t mission_seconds = 0;
        if(game.curAreaData->mission.goal == MISSION_GOAL_TIMED_SURVIVAL) {
            old_mission_min = old_mission_survival_min;
            mission_seconds = game.curAreaData->mission.goalAmount;
        } else {
            old_mission_min = old_mission_time_limit_min;
            mission_seconds = game.curAreaData->mission.failTimeLimit;
        }
        float old_day_end_min = day_start_min + old_mission_min * day_speed;
        old_day_end_min = wrapFloat(old_day_end_min, 0, 60 * 24);
        mission_seconds = std::max(mission_seconds, (size_t) 1);
        float new_mission_min = mission_seconds / 60.0f;
        game.curAreaData->dayTimeSpeed =
            calculateDaySpeed(
                day_start_min, old_day_end_min, new_mission_min
            );
    }
    
}


/**
 * @brief Processes the Dear ImGui fail conditions part of the
 * mission control panel for this frame.
 *
 * @param day_duration_needs_update The variable that dictates whether the
 * day duration widget data later in the panel needs to be updated.
 */
void AreaEditor::processGuiPanelMissionFail(
    bool* day_duration_needs_update
) {
    unsigned int fail_flags =
        (unsigned int) game.curAreaData->mission.failConditions;
    bool fail_flags_changed = false;
    
    //Pause menu end checkbox.
    bool pause_menu_end_is_fail =
        game.curAreaData->mission.goal != MISSION_GOAL_END_MANUALLY;
    ImGui::BeginDisabled();
    ImGui::CheckboxFlags(
        "End from pause menu",
        &fail_flags,
        getIdxBitmask(MISSION_FAIL_COND_PAUSE_MENU)
    );
    ImGui::EndDisabled();
    if(pause_menu_end_is_fail) {
        enableFlag(
            game.curAreaData->mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
        setTooltip(
            "Since reaching the mission goal automatically ends the\n"
            "mission as a clear, if the player can go to the pause menu\n"
            "and end there, then naturally they haven't reached the\n"
            "goal yet. So this method of ending has to always be a fail."
        );
    } else {
        disableFlag(
            game.curAreaData->mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
        setTooltip(
            "The current mission goal is \"end whenever you want\", so\n"
            "ending from the pause menu is the goal, not a fail condition."
        );
    }
    
    //Time limit checkbox.
    if(game.curAreaData->mission.goal == MISSION_GOAL_TIMED_SURVIVAL) {
        disableFlag(
            fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
        disableFlag(
            game.curAreaData->mission.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
        ImGui::BeginDisabled();
    }
    bool time_limit_changed =
        ImGui::CheckboxFlags(
            "Reach the time limit",
            &fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
    fail_flags_changed |= time_limit_changed;
    if(
        time_limit_changed &&
        hasFlag(
            fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        *day_duration_needs_update = true;
    }
    if(game.curAreaData->mission.goal == MISSION_GOAL_TIMED_SURVIVAL) {
        ImGui::EndDisabled();
        setTooltip(
            "The mission's goal is to survive for a certain amount of\n"
            "time, so it doesn't make sense to have a time limit to\n"
            "fail with."
        );
    } else {
        setTooltip(
            "The mission ends as a fail if the player spends a certain\n"
            "amount of time in the mission."
        );
    }
    
    if(
        hasFlag(
            fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        //Time limit values.
        int seconds =
            (int) game.curAreaData->mission.failTimeLimit;
        ImGui::Indent();
        if(ImGui::DragTime2("Time limit", &seconds)) {
            registerChange("mission fail conditions change");
            seconds = std::max(seconds, 1);
            game.curAreaData->mission.failTimeLimit = (size_t) seconds;
            *day_duration_needs_update = true;
        }
        setTooltip(
            "Time limit that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    //Reaching too few Pikmin checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Reach too few Pikmin",
            &fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TOO_FEW_PIKMIN)
        );
    setTooltip(
        "The mission ends as a fail if the total Pikmin count reaches\n"
        "a certain amount or lower. 0 means this only happens with a\n"
        "total Pikmin extinction. This fail condition isn't forced\n"
        "because the player might still be able to reach the mission\n"
        "goal using leaders. Or because you may want to make a mission\n"
        "with no Pikmin in the first place (like a puzzle stage)."
    );
    
    if(
        hasFlag(
            fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TOO_FEW_PIKMIN)
        )
    ) {
        ImGui::Indent();
        
        //Pikmin amount value.
        int amount =
            (int) game.curAreaData->mission.failTooFewPikAmount;
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Amount##fctfpa", &amount, 0.1f, 0, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->mission.failTooFewPikAmount =
                (size_t) amount;
        }
        setTooltip(
            "Pikmin amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::Unindent();
    }
    
    //Reaching too many Pikmin checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Reach too many Pikmin",
            &fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
        );
    setTooltip(
        "The mission ends as a fail if the total Pikmin count reaches\n"
        "a certain amount or higher."
    );
    
    if(
        hasFlag(
            fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
        )
    ) {
        ImGui::Indent();
        
        //Pikmin amount value.
        int amount =
            (int) game.curAreaData->mission.failTooManyPikAmount;
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Amount##fctmpa", &amount, 0.1f, 1, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->mission.failTooManyPikAmount =
                (size_t) amount;
        }
        setTooltip(
            "Pikmin amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::Unindent();
    }
    
    //Losing Pikmin checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Lose Pikmin",
            &fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
        );
    setTooltip(
        "The mission ends as a fail if a certain amount of Pikmin die."
    );
    
    if(
        hasFlag(
            fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
        )
    ) {
        //Pikmin deaths value.
        int amount =
            (int) game.curAreaData->mission.failPikKilled;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Deaths", &amount, 0.1f, 1, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->mission.failPikKilled =
                (size_t) amount;
        }
        setTooltip(
            "Pikmin death amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    //Taking damage checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Take damage",
            &fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_TAKE_DAMAGE)
        );
    setTooltip(
        "The mission ends as a fail if any leader loses any health."
    );
    
    //Lose leaders checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Lose leaders",
            &fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_LOSE_LEADERS)
        );
    setTooltip(
        "The mission ends as a fail if a certain amount of leaders get\n"
        "KO'd. This fail condition isn't forced because the\n"
        "player might still be able to reach the mission goal with the\n"
        "Pikmin. Or because you may want to make a really gimmicky\n"
        "automatic mission with no leaders."
    );
    
    if(
        hasFlag(
            fail_flags,
            getIdxBitmask(
                MISSION_FAIL_COND_LOSE_LEADERS
            )
        )
    ) {
        //Leader KOs value.
        int amount =
            (int) game.curAreaData->mission.failLeadersKod;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("KOs", &amount, 0.1f, 1, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->mission.failLeadersKod =
                (size_t) amount;
        }
        setTooltip(
            "Leader KO amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    //Kill enemies checkbox.
    fail_flags_changed |=
        ImGui::CheckboxFlags(
            "Kill enemies",
            &fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_KILL_ENEMIES)
        );
    setTooltip(
        "The mission ends as a fail if a certain amount of\n"
        "enemies get killed."
    );
    
    if(
        hasFlag(
            fail_flags,
            getIdxBitmask(MISSION_FAIL_COND_KILL_ENEMIES)
        )
    ) {
        //Enemy kills value.
        int amount =
            (int) game.curAreaData->mission.failEnemiesKilled;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Kills", &amount, 0.1f, 1, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->mission.failEnemiesKilled =
                (size_t) amount;
        }
        setTooltip(
            "Enemy kill amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    if(fail_flags_changed) {
        registerChange("mission fail conditions change");
        game.curAreaData->mission.failConditions =
            (bitmask_8_t) fail_flags;
    }
    
    vector<MISSION_FAIL_COND> active_conditions;
    for(size_t c = 0; c < game.missionFailConds.size(); c++) {
        if(
            hasFlag(
                game.curAreaData->mission.failConditions,
                getIdxBitmask(c)
            )
        ) {
            active_conditions.push_back((MISSION_FAIL_COND) c);
        }
    }
    
    if(!active_conditions.empty()) {
    
        //Primary HUD condition checkbox.
        ImGui::Spacer();
        bool show_primary =
            game.curAreaData->mission.failHudPrimaryCond != INVALID;
        if(ImGui::Checkbox("Show primary HUD element", &show_primary)) {
            registerChange("mission fail conditions change");
            game.curAreaData->mission.failHudPrimaryCond =
                show_primary ?
                (size_t) active_conditions[0] :
                INVALID;
        }
        setTooltip(
            "If checked, a large HUD element will appear showing\n"
            "the most important fail condition's information."
        );
        
        if(show_primary) {
            //Primary HUD condition combobox.
            int selected = 0;
            bool found = false;
            vector<string> cond_strings;
            for(size_t c = 0; c < active_conditions.size(); c++) {
                size_t cond_id = active_conditions[c];
                cond_strings.push_back(
                    game.missionFailConds[cond_id]->getName()
                );
                if(
                    cond_id ==
                    game.curAreaData->mission.failHudPrimaryCond
                ) {
                    found = true;
                    selected = (int) c;
                }
            }
            if(!found) {
                game.curAreaData->mission.failHudSecondaryCond = 0;
            }
            ImGui::Indent();
            if(
                ImGui::Combo(
                    "Primary condition", &selected, cond_strings, 15
                )
            ) {
                registerChange("mission fail conditions change");
                game.curAreaData->mission.failHudPrimaryCond =
                    active_conditions[selected];
            }
            setTooltip(
                "Failure condition to show in the primary HUD element."
            );
            ImGui::Unindent();
        }
        
        //Secondary HUD condition checkbox.
        bool show_secondary =
            game.curAreaData->mission.failHudSecondaryCond != INVALID;
        if(ImGui::Checkbox("Show secondary HUD element", &show_secondary)) {
            registerChange("mission fail conditions change");
            game.curAreaData->mission.failHudSecondaryCond =
                show_secondary ?
                (size_t) active_conditions[0] :
                INVALID;
        }
        setTooltip(
            "If checked, a smaller HUD element will appear showing\n"
            "some other fail condition's information."
        );
        
        if(show_secondary) {
            //Secondary HUD condition combobox.
            bool found = false;
            int selected = 0;
            vector<string> cond_strings;
            for(size_t c = 0; c < active_conditions.size(); c++) {
                size_t cond_id = active_conditions[c];
                cond_strings.push_back(
                    game.missionFailConds[cond_id]->getName()
                );
                if(
                    cond_id ==
                    game.curAreaData->mission.failHudSecondaryCond
                ) {
                    found = true;
                    selected = (int) c;
                }
            }
            if(!found) {
                game.curAreaData->mission.failHudSecondaryCond = 0;
            }
            ImGui::Indent();
            if(
                ImGui::Combo(
                    "Secondary condition", &selected, cond_strings, 15
                )
            ) {
                registerChange("mission fail conditions change");
                game.curAreaData->mission.failHudSecondaryCond =
                    active_conditions[selected];
            }
            setTooltip(
                "Failure condition to show in the secondary HUD element."
            );
            ImGui::Unindent();
        }
        
    } else {
        game.curAreaData->mission.failHudPrimaryCond = INVALID;
        game.curAreaData->mission.failHudSecondaryCond = INVALID;
        
    }
}


/**
 * @brief Processes the Dear ImGui battle enemies goal part of the
 * mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionGoalBe() {
    //Explanation text.
    ImGui::TextWrapped(
        "The player must defeat certain enemies, or all of them."
    );
    
    //Enemy requirements text.
    ImGui::Spacer();
    ImGui::Text("Enemy requirements:");
    
    int requires_all_option =
        game.curAreaData->mission.goalAllMobs ? 0 : 1;
        
    //All enemies requirement radio button.
    if(ImGui::RadioButton("All", &requires_all_option, 0)) {
        registerChange("mission requirements change");
        game.curAreaData->mission.goalAllMobs =
            requires_all_option == 0;
    }
    setTooltip(
        "Require the player to defeat all enemies "
        "in order to reach the goal."
    );
    
    //Specific enemies requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requires_all_option, 1)
    ) {
        registerChange("mission requirements change");
        game.curAreaData->mission.goalAllMobs =
            requires_all_option == 0;
    }
    setTooltip(
        "Require the player to defeat specific enemies "
        "in order to reach the goal.\n"
        "You must specify which enemies these are."
    );
    
    if(!game.curAreaData->mission.goalAllMobs) {
    
        //Start mob selector mode button.
        if(ImGui::Button("Pick enemies...")) {
            changeState(EDITOR_STATE_MOBS);
            subState = EDITOR_SUB_STATE_MISSION_MOBS;
        }
        setTooltip(
            "Click here to start picking which enemies do and\n"
            "do not belong to the required enemy list."
        );
        
    }
    
    //Total objects required text.
    size_t total_required = getMissionRequiredMobCount();
    ImGui::Text("Total objects required: %lu", total_required);
}


/**
 * @brief Processes the Dear ImGui collect treasures goal part of the
 * mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionGoalCt() {
    //Explanation text.
    ImGui::TextWrapped(
        "The player must collect certain treasures, or all of them."
    );
    
    //Treasure requirements text.
    ImGui::Spacer();
    ImGui::Text("Treasure requirements:");
    
    int requires_all_option =
        game.curAreaData->mission.goalAllMobs ? 0 : 1;
        
    //All treasures requirement radio button.
    if(ImGui::RadioButton("All", &requires_all_option, 0)) {
        registerChange("mission requirements change");
        game.curAreaData->mission.goalAllMobs =
            requires_all_option == 0;
    }
    setTooltip(
        "Require the player to collect all treasures "
        "in order to reach the goal."
    );
    
    //Specific treasures requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requires_all_option, 1)
    ) {
        registerChange("mission requirements change");
        game.curAreaData->mission.goalAllMobs =
            requires_all_option == 0;
    }
    setTooltip(
        "Require the player to collect specific treasures "
        "in order to reach the goal.\n"
        "You must specify which treasures these are."
    );
    
    if(!game.curAreaData->mission.goalAllMobs) {
    
        //Start mob selector mode button.
        if(ImGui::Button("Pick treasures...")) {
            changeState(EDITOR_STATE_MOBS);
            subState = EDITOR_SUB_STATE_MISSION_MOBS;
        }
        setTooltip(
            "Click here to start picking which treasures, piles, and\n"
            "resources do and do not belong to the required\n"
            "treasure list."
        );
        
    }
    
    //Total objects required text.
    size_t total_required = getMissionRequiredMobCount();
    ImGui::Text("Total objects required: %lu", total_required);
}


/**
 * @brief Processes the Dear ImGui get to exit goal part of the
 * mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionGoalGte() {
    //Explanation text.
    ImGui::TextWrapped(
        "The player must get a leader or all of them "
        "to the exit point."
    );
    
    //Start exit region selector mode button.
    ImGui::Spacer();
    if(ImGui::Button("Pick region...")) {
        subState = EDITOR_SUB_STATE_MISSION_EXIT;
    }
    setTooltip(
        "Click here to start picking where the exit region is.\n"
    );
    
    //Region center text.
    ImGui::Text(
        "Exit region center: %s,%s",
        f2s(game.curAreaData->mission.goalExitCenter.x).c_str(),
        f2s(game.curAreaData->mission.goalExitCenter.y).c_str()
    );
    
    //Region center text.
    ImGui::Text(
        "Exit region size: %s x %s",
        f2s(game.curAreaData->mission.goalExitSize.x).c_str(),
        f2s(game.curAreaData->mission.goalExitSize.y).c_str()
    );
    
    //Leader requirements text.
    ImGui::Spacer();
    ImGui::Text("Leader requirements:");
    
    int requires_all_option =
        game.curAreaData->mission.goalAllMobs ? 0 : 1;
        
    //All leaders requirement radio button.
    if(ImGui::RadioButton("All", &requires_all_option, 0)) {
        registerChange("mission requirements change");
        game.curAreaData->mission.goalAllMobs =
            requires_all_option == 0;
    }
    setTooltip(
        "Require the player to bring all leaders to the exit\n"
        "region in order to reach the mission's goal."
    );
    
    //Specific leaders requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requires_all_option, 1)
    ) {
        registerChange("mission requirements change");
        game.curAreaData->mission.goalAllMobs =
            requires_all_option == 0;
    }
    setTooltip(
        "Require the player to bring specific leaders to the exit\n"
        "region in order to reach the mission's goal.\n"
        "You must specify which leaders these are."
    );
    
    if(!game.curAreaData->mission.goalAllMobs) {
    
        //Start mob selector mode button.
        if(ImGui::Button("Pick leaders...")) {
            changeState(EDITOR_STATE_MOBS);
            subState = EDITOR_SUB_STATE_MISSION_MOBS;
        }
        setTooltip(
            "Click here to start picking which leaders do and\n"
            "do not belong to the required leader list."
        );
        
    }
    
    //Total objects required text.
    size_t total_required = getMissionRequiredMobCount();
    ImGui::Text("Total objects required: %lu", total_required);
}


/**
 * @brief Processes the Dear ImGui mission grading part of the
 * mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionGrading() {
    //Grading mode text.
    ImGui::Text("Grading mode:");
    
    //Grading mode widgets.
    processGuiGradingModeWidgets(
        0, "Points",
        "The player's final grade depends on how many points they\n"
        "got in different criteria."
    );
    
    ImGui::SameLine();
    processGuiGradingModeWidgets(
        1, "Goal",
        "The player's final grade depends on whether they have reached\n"
        "the mission goal (platinum) or not (nothing)."
    );
    
    ImGui::SameLine();
    processGuiGradingModeWidgets(
        2, "Participation",
        "The player's final grade depends on whether they have played\n"
        "the mission (platinum) or not (nothing)."
    );
    
    //Grading criterion widgets.
    if(
        game.curAreaData->mission.gradingMode == MISSION_GRADING_MODE_POINTS
    ) {
    
        ImGui::Spacer();
        processGuiGradingCriterionWidgets(
            &game.curAreaData->mission.pointsPerPikminBorn,
            MISSION_SCORE_CRITERIA_PIKMIN_BORN,
            "Points per Pikmin born",
            "Amount of points that the player receives for each\n"
            "Pikmin born."
        );
        
        processGuiGradingCriterionWidgets(
            &game.curAreaData->mission.pointsPerPikminDeath,
            MISSION_SCORE_CRITERIA_PIKMIN_DEATH,
            "Points per Pikmin death",
            "Amount of points that the player receives for each\n"
            "Pikmin lost."
        );
        
        if(
            hasFlag(
                game.curAreaData->mission.failConditions,
                getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
            )
        ) {
            processGuiGradingCriterionWidgets(
                &game.curAreaData->mission.pointsPerSecLeft,
                MISSION_SCORE_CRITERIA_SEC_LEFT,
                "Points per second left",
                "Amount of points that the player receives for each\n"
                "second of time left, from the mission's time limit."
            );
        }
        
        processGuiGradingCriterionWidgets(
            &game.curAreaData->mission.pointsPerSecPassed,
            MISSION_SCORE_CRITERIA_SEC_PASSED,
            "Points per second passed",
            "Amount of points that the player receives for each\n"
            "second of time that has passed."
        );
        
        processGuiGradingCriterionWidgets(
            &game.curAreaData->mission.pointsPerTreasurePoint,
            MISSION_SCORE_CRITERIA_TREASURE_POINTS,
            "Points per treasure point",
            "Amount of points that the player receives for each\n"
            "point gathered from treasures. Different treasures are worth\n"
            "different treasure points."
        );
        
        processGuiGradingCriterionWidgets(
            &game.curAreaData->mission.pointsPerEnemyPoint,
            MISSION_SCORE_CRITERIA_ENEMY_POINTS,
            "Points per enemy point",
            "Amount of points that the player receives for each\n"
            "enemy point. Different enemies are worth different\n"
            "points."
        );
        
        //Award points on collection checkbox.
        if(game.curAreaData->mission.pointsPerEnemyPoint != 0) {
            bool enemy_points_on_collection =
                game.curAreaData->mission.enemyPointsOnCollection;
            ImGui::Indent();
            if(
                ImGui::Checkbox(
                    "Award points on collection", &enemy_points_on_collection
                )
            ) {
                registerChange("mission grading change");
                game.curAreaData->mission.enemyPointsOnCollection =
                    enemy_points_on_collection;
            }
            setTooltip(
                "If checked, enemy points will be awarded on enemy\n"
                "collection. If unchecked, enemy points will be awarded\n"
                "on enemy death."
            );
            ImGui::Unindent();
        }
        
        //Starting score value.
        ImGui::Spacer();
        int starting_points = game.curAreaData->mission.startingPoints;
        ImGui::SetNextItemWidth(60);
        if(ImGui::DragInt("Starting points", &starting_points, 1.0f)) {
            registerChange("mission grading change");
            game.curAreaData->mission.startingPoints = starting_points;
        }
        setTooltip(
            "Starting amount of points. It can be positive or negative.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Medal point requirements text.
        ImGui::Spacer();
        ImGui::Text("Medal point requirements:");
        
        //Medal point requirement widgets.
        processGuiGradingMedalWidgets(
            &game.curAreaData->mission.bronzeReq, "Bronze",
            INT_MIN, game.curAreaData->mission.silverReq - 1,
            "To get a bronze medal, the player needs at least these\n"
            "many points. Fewer than this, and the player gets no medal."
        );
        
        processGuiGradingMedalWidgets(
            &game.curAreaData->mission.silverReq, "Silver",
            game.curAreaData->mission.bronzeReq + 1,
            game.curAreaData->mission.goldReq - 1,
            "To get a silver medal, the player needs at least these\n"
            "many points."
        );
        
        processGuiGradingMedalWidgets(
            &game.curAreaData->mission.goldReq, "Gold",
            game.curAreaData->mission.silverReq + 1,
            game.curAreaData->mission.platinumReq - 1,
            "To get a gold medal, the player needs at least these\n"
            "many points."
        );
        
        processGuiGradingMedalWidgets(
            &game.curAreaData->mission.platinumReq, "Platinum",
            game.curAreaData->mission.goldReq + 1, INT_MAX,
            "To get a platinum medal, the player needs at least these\n"
            "many points."
        );
        
        //Maker record value.
        ImGui::Spacer();
        int maker_record = game.curAreaData->mission.makerRecord;
        ImGui::SetNextItemWidth(60);
        if(ImGui::DragInt("Maker's record", &maker_record, 1.0f)) {
            registerChange("maker record change");
            game.curAreaData->mission.makerRecord = maker_record;
        }
        setTooltip(
            "Specify your best score here, if you want.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Maker record date input.
        string maker_record_date =
            game.curAreaData->mission.makerRecordDate;
        ImGui::SetNextItemWidth(120);
        if(
            monoInputText(
                "Date (YYYY/MM/DD)", &maker_record_date
            )
        ) {
            registerChange("maker record change");
            game.curAreaData->mission.makerRecordDate = maker_record_date;
        }
        setTooltip(
            "Specify the date in which you got your best score here,\n"
            "if you want. Your record will only be saved if you write a date.\n"
            "The format must be YYYY/MM/DD."
        );
    }
}


/**
 * @brief Processes the Dear ImGui mob control panel for this frame.
 */
void AreaEditor::processGuiPanelMob() {

    MobGen* m_ptr = *selectedMobs.begin();
    
    //Category and type comboboxes.
    string custom_cat_name = "";
    if(m_ptr->type) custom_cat_name = m_ptr->type->customCategoryName;
    MobType* type = m_ptr->type;
    
    if(processGuiMobTypeWidgets(&custom_cat_name, &type)) {
        registerChange("object type change");
        m_ptr->type = type;
        lastMobCustomCatName = "";
        if(m_ptr->type) {
            lastMobCustomCatName = m_ptr->type->customCategoryName;
        }
        lastMobType = m_ptr->type;
    }
    
    if(m_ptr->type) {
        //Tips text.
        ImGui::TextDisabled("(%s info & tips)", m_ptr->type->name.c_str());
        string full_str =
            "Internal object category: " + m_ptr->type->category->name + "\n" +
            wordWrap(m_ptr->type->description, 50);
        if(!m_ptr->type->areaEditorTips.empty()) {
            full_str +=
                "\n\n" +
                wordWrap(m_ptr->type->areaEditorTips, 50);
        }
        setTooltip(full_str);
        
        if(m_ptr->type->areaEditorRecommendLinksFrom) {
            if(m_ptr->links.empty()) {
                //No outgoing links warning.
                ImGui::PushStyleColor(
                    ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.05f, 1.0f)
                );
                ImGui::Text("Warning: no links from this mob!");
                ImGui::PopStyleColor();
                setTooltip(
                    "Warning: you need to link this object to a different one\n"
                    "in order for it to work as intended!"
                );
            }
        }
        
        if(m_ptr->type->areaEditorRecommendLinksTo) {
            bool has_links_to = false;
            for(
                size_t m = 0;
                m < game.curAreaData->mobGenerators.size();
                m++
            ) {
                MobGen* other_m_ptr = game.curAreaData->mobGenerators[m];
                for(size_t l = 0; l < other_m_ptr->links.size(); l++) {
                    if(other_m_ptr->links[l] == m_ptr) {
                        has_links_to = true;
                        break;
                    }
                }
                if(has_links_to) break;
            }
            if(!has_links_to) {
                //No incoming links warning.
                ImGui::PushStyleColor(
                    ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.05f, 1.0f)
                );
                ImGui::Text("Warning: no links to this mob!");
                ImGui::PopStyleColor();
                setTooltip(
                    "Warning: you need to link a different object to this one\n"
                    "in order for it to work as intended!"
                );
            }
        }
        
        //If the mob type exists, obviously the missing mob type problem is
        //gone, if it was active.
        if(problemType == EPT_TYPELESS_MOB) {
            clearProblems();
        }
    }
    
    //Object angle value.
    float mob_angle = normalizeAngle(m_ptr->angle);
    ImGui::Spacer();
    if(ImGui::SliderAngle("Angle", &mob_angle, 0, 360, "%.2f")) {
        registerChange("object angle change");
        m_ptr->angle = mob_angle;
    }
    setTooltip(
        "Angle that the object is facing.\n"
        "You can also press R in the canvas to "
        "make it face the cursor.",
        "", WIDGET_EXPLANATION_SLIDER
    );
    
    //Object script vars node.
    ImGui::Spacer();
    if(saveableTreeNode("mobs", "Script vars")) {
    
        processGuiMobScriptVars(m_ptr);
        
        ImGui::TreePop();
        
    }
    
    //Object advanced node.
    ImGui::Spacer();
    if(saveableTreeNode("mobs", "Advanced")) {
    
        if(m_ptr->storedInside == INVALID) {
            //Store inside another mob button.
            if(ImGui::Button("Store inside...")) {
                subState = EDITOR_SUB_STATE_STORE_MOB_INSIDE;
            }
            setTooltip(
                "If you want to store this object inside another object,\n"
                "click here to choose which object will do the storing.\n"
                "When that object dies, this one pops out."
            );
            
        } else {
        
            //Unstore button.
            if(ImGui::Button("Unstore")) {
                m_ptr->storedInside = INVALID;
            }
            setTooltip(
                "This object is currently stored inside another. Click here\n"
                "to unstore it and make it a regular object instead."
            );
        }
        
        //Object link amount text.
        ImGui::Spacer();
        ImGui::Text(
            "%i link%s", (int) m_ptr->links.size(),
            m_ptr->links.size() == 1 ? "" : "s"
        );
        
        //Object new link button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "newLinkButton", editorIcons[EDITOR_ICON_ADD],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            if(subState == EDITOR_SUB_STATE_ADD_MOB_LINK) {
                subState = EDITOR_SUB_STATE_NONE;
            } else {
                subState = EDITOR_SUB_STATE_ADD_MOB_LINK;
            }
        }
        setTooltip(
            "Start creating a new object link.\n"
            "Click on the other object you want to link to.",
            "Shift+L"
        );
        
        //Object delete link button.
        if(!(*selectedMobs.begin())->links.empty()) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delLinkButton", editorIcons[EDITOR_ICON_REMOVE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                if((*selectedMobs.begin())->links.size() == 1) {
                    registerChange("Object link deletion");
                    m_ptr->links.erase(m_ptr->links.begin());
                    m_ptr->linkIdxs.erase(m_ptr->linkIdxs.begin());
                    homogenizeSelectedMobs();
                } else if(subState == EDITOR_SUB_STATE_DEL_MOB_LINK) {
                    subState = EDITOR_SUB_STATE_NONE;
                } else {
                    subState = EDITOR_SUB_STATE_DEL_MOB_LINK;
                }
            }
            setTooltip(
                "Delete an object link.\n"
                "If there is only one, it gets deleted automatically.\n"
                "Otherwise, you must click on the other object whose\n"
                "link you want to delete, or click the link proper."
            );
        }
        
        ImGui::TreePop();
    }
    
    homogenizeSelectedMobs();
    
}


/**
 * @brief Processes the Dear ImGui mobs control panel for this frame.
 */
void AreaEditor::processGuiPanelMobs() {
    ImGui::BeginChild("mobs");
    
    if(subState == EDITOR_SUB_STATE_NEW_MOB) {
    
        //Creation explanation text.
        ImGui::TextWrapped(
            "Use the canvas to place an object. It'll appear where you click."
        );
        
        //Creation cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            setStatus();
            subState = EDITOR_SUB_STATE_NONE;
        }
        setTooltip(
            "Cancel the creation.",
            "Escape"
        );
        
    } else if(subState == EDITOR_SUB_STATE_DUPLICATE_MOB) {
    
        //Duplication explanation text.
        ImGui::TextWrapped(
            "Use the canvas to place the new duplicated object(s). "
            "It/they will appear where you click."
        );
        
        //Duplication cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            setStatus();
            subState = EDITOR_SUB_STATE_NONE;
        }
        setTooltip(
            "Cancel the duplication.",
            "Escape"
        );
        
    } else if(subState == EDITOR_SUB_STATE_STORE_MOB_INSIDE) {
    
        //Storing process explanation text.
        ImGui::TextWrapped(
            "Use the canvas to link to an object. Click on the object you "
            "want this one to be stored inside of."
        );
        
        //Storing process cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            setStatus();
            subState = EDITOR_SUB_STATE_NONE;
        }
        setTooltip(
            "Cancel the storing process.",
            "Escape"
        );
        
    } else if(subState == EDITOR_SUB_STATE_ADD_MOB_LINK) {
    
        //Link addition explanation text.
        ImGui::TextWrapped(
            "Use the canvas to link to an object. Click on the object you "
            "want this one to link to."
        );
        
        //Link addition cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            setStatus();
            subState = EDITOR_SUB_STATE_NONE;
        }
        setTooltip(
            "Cancel the linking.",
            "Escape"
        );
        
    } else if(subState == EDITOR_SUB_STATE_DEL_MOB_LINK) {
    
        //Link deletion explanation text.
        ImGui::TextWrapped(
            "Use the canvas to delete an object link. Click on a linked object "
            "or on its link to delete the corresponding link."
        );
        
        //Link deletion cancel button.
        if(ImGui::Button("Cancel", ImVec2(-1.0f, 32.0f))) {
            setStatus();
            subState = EDITOR_SUB_STATE_NONE;
        }
        setTooltip(
            "Cancel the link removal.",
            "Escape"
        );
        
    } else if(subState == EDITOR_SUB_STATE_MISSION_MOBS) {
    
        string cat_name =
            game.curAreaData->mission.goal ==
            MISSION_GOAL_COLLECT_TREASURE ?
            "treasure/pile/resource" :
            game.curAreaData->mission.goal ==
            MISSION_GOAL_BATTLE_ENEMIES ?
            "enemy" :
            "leader";
            
        //Instructions text.
        ImGui::TextWrapped(
            "Click a %s object to mark or unmark it as a required "
            "object for the mission. Objects flashing yellow are considered "
            "required. Click the finish button when you are done.",
            cat_name.c_str()
        );
        
        //Total objects required text.
        ImGui::Text(
            "Total objects required: %lu",
            game.curAreaData->mission.goalMobIdxs.size()
        );
        
        //Finish button.
        if(ImGui::Button("Finish")) {
            changeState(EDITOR_STATE_GAMEPLAY);
        }
        setTooltip("Click here to finish.");
        
    } else {
    
        //Back button.
        if(ImGui::Button("Back")) {
            changeState(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panelTitle("OBJECTS");
        
        //New object button.
        if(
            ImGui::ImageButton(
                "newMobButton", editorIcons[EDITOR_ICON_ADD],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            newMobCmd(1.0f);
        }
        setTooltip(
            "Start creating a new object.\n"
            "Click on the canvas where you want the object to be.",
            "N"
        );
        
        if(!selectedMobs.empty()) {
        
            //Delete object button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delMobButtonn", editorIcons[EDITOR_ICON_REMOVE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                deleteMobCmd(1.0f);
            }
            setTooltip(
                "Delete all selected objects.\n",
                "Delete"
            );
            
            //Duplicate object button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "dupMobButton", editorIcons[EDITOR_ICON_DUPLICATE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                duplicateMobsCmd(1.0f);
            }
            setTooltip(
                "Start duplicating the selected objects.\n"
                "Click on the canvas where you want the copied objects to be.",
                "Ctrl+D"
            );
            
        }
        
        ImGui::Spacer();
        
        if(selectedMobs.size() == 1 || selectionHomogenized) {
        
            processGuiPanelMob();
            
        } else if(selectedMobs.empty()) {
        
            //"No object selected" text.
            ImGui::TextDisabled("(No object selected)");
            
        } else {
        
            //Non-homogenized objects warning.
            ImGui::TextWrapped(
                "Multiple different objects selected. To make all their "
                "properties the same and edit them all together, click here:"
            );
            
            //Homogenize objects button.
            if(ImGui::Button("Edit all together")) {
                registerChange("object combining");
                selectionHomogenized = true;
                homogenizeSelectedMobs();
            }
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui path link control panel for this frame.
 */
void AreaEditor::processGuiPanelPathLink() {
    PathLink* l_ptr = *selectedPathLinks.begin();
    
    //Type combobox.
    vector<string> link_type_names;
    link_type_names.push_back("Normal");
    link_type_names.push_back("Ledge");
    
    int type_i = l_ptr->type;
    if(ImGui::Combo("Type", &type_i, link_type_names, 15)) {
        registerChange("path link type change");
        l_ptr->type = (PATH_LINK_TYPE) type_i;
    }
    setTooltip(
        "What type of link this is."
    );
    
    homogenizeSelectedPathLinks();
}


/**
 * @brief Processes the Dear ImGui path stop control panel for this frame.
 */
void AreaEditor::processGuiPanelPathStop() {
    PathStop* s_ptr = *selectedPathStops.begin();
    
    //Radius value.
    float radius = s_ptr->radius;
    if(ImGui::DragFloat("Radius", &radius, 0.5f, PATHS::MIN_STOP_RADIUS)) {
        radius = std::max(PATHS::MIN_STOP_RADIUS, radius);
        registerChange("path stop radius change");
        s_ptr->radius = radius;
        pathPreviewTimer.start(false);
    }
    setTooltip(
        "Radius of the stop. Used when mobs want to find the closest\n"
        "start/end stop.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Script use only checkbox.
    int flags_i = s_ptr->flags;
    if(
        ImGui::CheckboxFlags(
            "Script use only",
            &flags_i,
            PATH_STOP_FLAG_SCRIPT_ONLY
        )
    ) {
        registerChange("path stop property change");
        s_ptr->flags = flags_i;
    }
    setTooltip(
        "Can only be used by objects if their script tells them to."
    );
    
    //Light load only checkbox.
    if(
        ImGui::CheckboxFlags(
            "Light load only",
            &flags_i,
            PATH_STOP_FLAG_LIGHT_LOAD_ONLY
        )
    ) {
        registerChange("path stop property change");
        s_ptr->flags = flags_i;
    }
    setTooltip(
        "Can only be used by objects that are not carrying anything, "
        "or by objects that only have a weight of 1."
    );
    
    //Airborne only checkbox.
    if(
        ImGui::CheckboxFlags(
            "Airborne only",
            &flags_i,
            PATH_STOP_FLAG_AIRBORNE_ONLY
        )
    ) {
        registerChange("path stop property change");
        s_ptr->flags = flags_i;
    }
    setTooltip(
        "Can only be used by objects that can fly."
    );
    
    //Label text.
    monoInputText("Label", &s_ptr->label);
    setTooltip(
        "If this stop is part of a path that you want\n"
        "to address in a script, write the name here."
    );
    
    homogenizeSelectedPathStops();
}


/**
 * @brief Processes the Dear ImGui paths control panel for this frame.
 */
void AreaEditor::processGuiPanelPaths() {
    ImGui::BeginChild("paths");
    
    if(subState == EDITOR_SUB_STATE_PATH_DRAWING) {
    
        //Drawing explanation text.
        ImGui::TextWrapped(
            "Use the canvas to draw path links and path stops. "
            "Each click places a stop and/or connects to a stop. "
            "Use the following widgets the change how new links will be."
        );
        
        //Link settings text.
        ImGui::Spacer();
        ImGui::Text("New path link settings:");
        ImGui::Indent();
        
        int one_way_mode = pathDrawingNormals;
        
        //One-way links radio button.
        ImGui::RadioButton("Draw one-way links", &one_way_mode, 0);
        setTooltip(
            "When drawing, new links drawn will be one-way links.",
            "1"
        );
        
        //Normal links radio button.
        ImGui::RadioButton("Draw normal links", &one_way_mode, 1);
        setTooltip(
            "When drawing, new links drawn will be normal (two-way) links.",
            "2"
        );
        
        pathDrawingNormals = one_way_mode;
        
        //Type combobox.
        vector<string> link_type_names;
        link_type_names.push_back("Normal");
        link_type_names.push_back("Ledge");
        
        int type_i = pathDrawingType;
        if(ImGui::Combo("Type", &type_i, link_type_names, 15)) {
            pathDrawingType = (PATH_LINK_TYPE) type_i;
        }
        setTooltip(
            "What type of link to draw."
        );
        ImGui::Unindent();
        
        //Stop settings text.
        ImGui::Spacer();
        ImGui::Text("New path stop settings:");
        
        //Script use only checkbox.
        ImGui::Indent();
        int flags_i = pathDrawingFlags;
        if(
            ImGui::CheckboxFlags(
                "Script use only",
                &flags_i,
                PATH_STOP_FLAG_SCRIPT_ONLY
            )
        ) {
            pathDrawingFlags = flags_i;
        }
        setTooltip(
            "Can only be used by objects if their script tells them to."
        );
        
        //Light load only checkbox.
        if(
            ImGui::CheckboxFlags(
                "Light load only",
                &flags_i,
                PATH_STOP_FLAG_LIGHT_LOAD_ONLY
            )
        ) {
            pathDrawingFlags = flags_i;
        }
        setTooltip(
            "Can only be used by objects that are not carrying anything, "
            "or by objects that only have a weight of 1."
        );
        
        //Airborne only checkbox.
        if(
            ImGui::CheckboxFlags(
                "Airborne only",
                &flags_i,
                PATH_STOP_FLAG_AIRBORNE_ONLY
            )
        ) {
            pathDrawingFlags = flags_i;
        }
        setTooltip(
            "Can only be used by objects that can fly."
        );
        
        //Label text.
        monoInputText("Label", &pathDrawingLabel);
        setTooltip(
            "If the new stop is part of a path that you want\n"
            "to address in a script, write the name here."
        );
        ImGui::Unindent();
        
        //Drawing stop button.
        ImGui::Spacer();
        if(ImGui::Button("Done", ImVec2(-1.0f, 32.0f))) {
            setStatus();
            subState = EDITOR_SUB_STATE_NONE;
        }
        setTooltip(
            "Stop drawing.",
            "Escape"
        );
        
    } else {
    
        //Back button.
        if(ImGui::Button("Back")) {
            changeState(EDITOR_STATE_MAIN);
        }
        
        //Panel title text.
        panelTitle("PATHS");
        
        //New path button.
        if(
            ImGui::ImageButton(
                "newPathButton", editorIcons[EDITOR_ICON_ADD],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            newPathCmd(1.0f);
        }
        setTooltip(
            "Start drawing a new path.\n"
            "Click on a path stop to start there, or click somewhere empty "
            "to start on a new stop.\n"
            "Then, click a path stop or somewhere empty to create a "
            "link there.",
            "N"
        );
        
        //Delete path button.
        if(!selectedPathLinks.empty() || !selectedPathStops.empty()) {
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delPathButton", editorIcons[EDITOR_ICON_REMOVE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                deletePathCmd(1.0f);
            }
            setTooltip(
                "Delete all selected path stops and/or path links.\n",
                "Delete"
            );
        }
        
        //Stop properties node.
        ImGui::Spacer();
        if(saveableTreeNode("paths", "Stop properties")) {
        
            bool ok_to_edit =
                (selectedPathStops.size() == 1) || selectionHomogenized;
                
            if(selectedPathStops.empty()) {
            
                //"No stop selected" text.
                ImGui::TextDisabled("(No path stop selected)");
                
            } else if(ok_to_edit) {
            
                processGuiPanelPathStop();
                
            } else {
            
                //Non-homogenized stops warning.
                ImGui::TextWrapped(
                    "Multiple different path stops selected. "
                    "To make all their properties the same and "
                    "edit them all together, click here:"
                );
                
                //Homogenize stops button.
                if(ImGui::Button("Edit all together")) {
                    registerChange("path stop combining");
                    selectionHomogenized = true;
                    //Unselect path links otherwise those will be considered
                    //homogenized too.
                    selectedPathLinks.clear();
                    homogenizeSelectedPathStops();
                }
            }
            
            
            ImGui::TreePop();
            
        }
        
        //Link properties node.
        ImGui::Spacer();
        if(saveableTreeNode("paths", "Link properties")) {
        
            bool ok_to_edit =
                (selectedPathLinks.size() == 1) || selectionHomogenized;
            if(!ok_to_edit && selectedPathLinks.size() == 2) {
                auto it = selectedPathLinks.begin();
                PathLink* l1 = *it;
                it++;
                PathLink* l2 = *it;
                if(
                    l1->startPtr == l2->endPtr &&
                    l1->endPtr == l2->startPtr
                ) {
                    //The only things we have selected are a link,
                    //and also the opposite link. As far as the user cares,
                    //this is all just one link that is of the "normal" type.
                    //And if they edit the properties, we want both links to
                    //be edited together.
                    ok_to_edit = true;
                }
            }
            
            if(selectedPathLinks.empty()) {
            
                //"No link selected" text.
                ImGui::TextDisabled("(No path link selected)");
                
            } else if(ok_to_edit) {
            
                processGuiPanelPathLink();
                
            } else {
            
                //Non-homogenized links warning.
                ImGui::TextWrapped(
                    "Multiple different path links selected. "
                    "To make all their properties the same and "
                    "edit them all together, click here:"
                );
                
                //Homogenize links button.
                if(ImGui::Button("Edit all together")) {
                    registerChange("path link combining");
                    selectionHomogenized = true;
                    //Unselect path stops otherwise those will be considered
                    //homogenized too.
                    selectedPathStops.clear();
                    homogenizeSelectedPathLinks();
                }
            }
            
            
            ImGui::TreePop();
            
        }
        
        //Path preview node.
        ImGui::Spacer();
        if(saveableTreeNode("paths", "Path preview")) {
        
            //Show preview path checkbox.
            if(ImGui::Checkbox("Show preview path", &showPathPreview)) {
                if(
                    showPathPreview &&
                    pathPreviewCheckpoints[0].x == LARGE_FLOAT
                ) {
                    //No previous location. Place them on-camera.
                    pathPreviewCheckpoints[0].x =
                        game.cam.pos.x - AREA_EDITOR::COMFY_DIST;
                    pathPreviewCheckpoints[0].y =
                        game.cam.pos.y;
                    pathPreviewCheckpoints[1].x =
                        game.cam.pos.x + AREA_EDITOR::COMFY_DIST;
                    pathPreviewCheckpoints[1].y =
                        game.cam.pos.y;
                }
                pathPreviewDist = calculatePreviewPath();
            }
            setTooltip(
                "Show the path objects will take to travel from point A\n"
                "to point B. These points can be dragged in the canvas.\n"
                "Hazards and obstacles will not be taken into consideration\n"
                "when calculating the preview path."
            );
            
            ImGui::Spacer();
            
            if(showPathPreview) {
            
                unsigned int flags_i = pathPreviewSettings.flags;
                
                //Is from script checkbox.
                if(
                    ImGui::CheckboxFlags(
                        "Is from script",
                        &flags_i,
                        PATH_FOLLOW_FLAG_SCRIPT_USE
                    )
                ) {
                    pathPreviewSettings.flags = flags_i;
                    pathPreviewDist = calculatePreviewPath();
                }
                setTooltip(
                    "Whether the path preview feature is considered to be\n"
                    "from a script, meaning it can use script-only stops."
                );
                
                //Has light load checkbox.
                if(
                    ImGui::CheckboxFlags(
                        "Has light load",
                        &flags_i,
                        PATH_FOLLOW_FLAG_LIGHT_LOAD
                    )
                ) {
                    pathPreviewSettings.flags = flags_i;
                    pathPreviewDist = calculatePreviewPath();
                }
                setTooltip(
                    "Whether the path preview feature is considered to have\n"
                    "a light load, meaning it can use light load-only stops."
                );
                
                //Is airborne checkbox.
                if(
                    ImGui::CheckboxFlags(
                        "Is airborne",
                        &flags_i,
                        PATH_FOLLOW_FLAG_AIRBORNE
                    )
                ) {
                    pathPreviewSettings.flags = flags_i;
                    pathPreviewDist = calculatePreviewPath();
                }
                setTooltip(
                    "Whether the path preview feature is considered to be\n"
                    "airborne, meaning it can use airborne-only stops\n"
                    "and go up ledges."
                );
                
                //Use stops with this label input.
                if(
                    ImGui::InputText(
                        "Label",
                        &pathPreviewSettings.label
                    )
                ) {
                    pathPreviewDist = calculatePreviewPath();
                }
                setTooltip(
                    "To limit the path preview feature to only use stops with\n"
                    "a given label, write its name here, or leave it empty\n"
                    "for no label enforcement."
                );
                
                string result;
                float total_dist = 0.0f;
                size_t total_nr_stops = 0;
                bool success = false;
                
                if(pathPreviewResult > 0) {
                    total_dist = pathPreviewDist;
                    total_nr_stops = pathPreview.size();
                    success = true;
                }
                
                result = pathResultToString(pathPreviewResult);
                
                //Path result header text.
                ImGui::Spacer();
                ImGui::Text("Result:");
                
                //Path result text.
                ImGui::BulletText("%s", result.c_str());
                
                //Path total travel distance text.
                if(success) {
                    ImGui::BulletText(
                        "Total travel distance: %f", total_dist
                    );
                } else {
                    ImGui::Text(" ");
                }
                
                //Path total stops visited text.
                if(success) {
                    ImGui::BulletText(
                        "Total stops visited: %lu", total_nr_stops
                    );
                } else {
                    ImGui::Text(" ");
                }
                
            }
            
            ImGui::TreePop();
            
        }
        
        //Path tools node.
        ImGui::Spacer();
        if(saveableTreeNode("paths", "Tools")) {
        
            //Show closest stop checkbox.
            ImGui::Checkbox("Show closest stop", &showClosestStop);
            setTooltip(
                "Show the closest stop to the cursor.\n"
                "Useful to know which stop "
                "Pikmin will go to when starting to carry."
            );
            
            //Select stops with label button.
            if(ImGui::Button("Select all stops with label...")) {
                openInputPopup("selectStops");
            }
            setTooltip(
                "Selects all stops that have the specified label.\n"
                "The search is case-sensitive."
            );
            
            //Select stops with label popup.
            static string label_name;
            if(processGuiInputPopup("selectStops", "Label:", &label_name, true)) {
                selectPathStopsWithLabel(label_name);
            }
            
            ImGui::TreePop();
            
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui review control panel for this frame.
 */
void AreaEditor::processGuiPanelReview() {
    ImGui::BeginChild("review");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("REVIEW");
    
    //Problem search node.
    if(saveableTreeNode("review", "Problem search")) {
    
        //Problem search button.
        if(ImGui::Button("Search for problems")) {
            findProblems();
        }
        setTooltip(
            "Search for problems with the area."
        );
        
        //Problem texts.
        ImGui::Text("Problem found:");
        
        ImGui::Indent();
        if(problemType == EPT_NONE_YET) {
            ImGui::TextDisabled("Haven't searched yet.");
        } else {
            ImGui::TextWrapped("%s", problemTitle.c_str());
        }
        ImGui::Unindent();
        
        if(!problemDescription.empty()) {
        
            ImGui::Indent();
            ImGui::TextWrapped("%s", problemDescription.c_str());
            ImGui::Unindent();
            
            //Go to problem button.
            if(ImGui::Button("Go to problem")) {
                goToProblem();
            }
            setTooltip(
                "Focus the camera on the problem found, if applicable."
            );
            
        }
        
        ImGui::TreePop();
        
    }
    
    //Preview node.
    ImGui::Spacer();
    if(saveableTreeNode("review", "Preview")) {
    
        //Area preview checkbox.
        ImGui::Checkbox("Preview area", &previewMode);
        setTooltip(
            "Preview how the area will look like, without any of the "
            "area editor's components in the way."
        );
        
        //Tree shadows checkbox.
        if(!previewMode) {
            ImGui::BeginDisabled();
        }
        ImGui::Indent();
        ImGui::Checkbox("Show tree shadows", &showShadows);
        ImGui::Unindent();
        if(!previewMode) {
            ImGui::EndDisabled();
        }
        
        ImGui::TreePop();
        
    }
    
    //Cross-section node.
    ImGui::Spacer();
    if(saveableTreeNode("review", "Cross-section")) {
    
        //Show cross-section checkbox.
        if(ImGui::Checkbox("Show cross-section", &showCrossSection)) {
            if(showCrossSection) {
                crossSectionWindowStart = canvasTL;
                crossSectionWindowEnd =
                    Point(canvasBR.x * 0.5, canvasBR.y * 0.5);
                crossSectionZWindowStart =
                    Point(
                        crossSectionWindowEnd.x,
                        crossSectionWindowStart.y
                    );
                crossSectionZWindowEnd =
                    Point(
                        crossSectionWindowEnd.x + 48,
                        crossSectionWindowEnd.y
                    );
            }
            
            if(
                showCrossSection &&
                crossSectionCheckpoints[0].x == LARGE_FLOAT
            ) {
                //No previous location. Place them on-camera.
                crossSectionCheckpoints[0].x =
                    game.cam.pos.x - AREA_EDITOR::COMFY_DIST;
                crossSectionCheckpoints[0].y =
                    game.cam.pos.y;
                crossSectionCheckpoints[1].x =
                    game.cam.pos.x + AREA_EDITOR::COMFY_DIST;
                crossSectionCheckpoints[1].y =
                    game.cam.pos.y;
            }
        }
        setTooltip(
            "Show a 2D cross-section between points A and B."
        );
        
        //Show height grid checkbox.
        if(showCrossSection) {
            ImGui::Indent();
            ImGui::Checkbox("Show height grid", &showCrossSectionGrid);
            setTooltip(
                "Show a height grid in the cross-section window."
            );
            ImGui::Unindent();
        }
        
        ImGui::Spacer();
        
        ImGui::TreePop();
        
    }
    
    //Tools node.
    if(saveableTreeNode("review", "Tools")) {
    
        //Show blocking sectors checkbox.
        ImGui::Checkbox("Show blocking sectors", &showBlockingSectors);
        setTooltip(
            "Show which sectors are blocking (red) and which\n"
            "are not (green). Useful to make sure the radar works as\n"
            "intended, and that players can't go or throw out-of-bounds."
        );
        
        //Show height grid checkbox.
        if(showCrossSection) {
            ImGui::Indent();
            ImGui::Checkbox("Show height grid", &showCrossSectionGrid);
            setTooltip(
                "Show a height grid in the cross-section window."
            );
            ImGui::Unindent();
        }
        
        ImGui::Spacer();
        
        ImGui::TreePop();
        
    }
    
    //Stats node.
    if(saveableTreeNode("main", "Stats")) {
    
        //Sector amount text.
        ImGui::BulletText(
            "Sectors: %i", (int) game.curAreaData->sectors.size()
        );
        
        //Edge amount text.
        ImGui::BulletText(
            "Edges: %i", (int) game.curAreaData->edges.size()
        );
        
        //Vertex amount text.
        ImGui::BulletText(
            "Vertexes: %i", (int) game.curAreaData->vertexes.size()
        );
        
        //Object amount text.
        ImGui::BulletText(
            "Objects: %i", (int) game.curAreaData->mobGenerators.size()
        );
        
        //Path stop amount text.
        ImGui::BulletText(
            "Path stops: %i", (int) game.curAreaData->pathStops.size()
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sector control panel for this frame.
 */
void AreaEditor::processGuiPanelSector() {
    Sector* s_ptr = *selectedSectors.begin();
    
    //Sector behavior node.
    if(saveableTreeNode("layout", "Behavior")) {
    
        //Sector height value.
        float sector_z = s_ptr->z;
        if(ImGui::DragFloat("Height", &sector_z)) {
            registerChange("sector height change");
            s_ptr->z = sector_z;
            updateAllEdgeOffsetCaches();
        }
        if(ImGui::BeginPopupContextItem()) {
            //-50 height selectable.
            if(ImGui::Selectable("-50")) {
                registerChange("sector height change");
                s_ptr->z -= 50.0f;
                updateAllEdgeOffsetCaches();
                ImGui::CloseCurrentPopup();
            }
            
            //+50 height selectable.
            if(ImGui::Selectable("+50")) {
                registerChange("sector height change");
                s_ptr->z += 50.0f;
                updateAllEdgeOffsetCaches();
                ImGui::CloseCurrentPopup();
            }
            
            //Set to zero selectable.
            if(ImGui::Selectable("Set to 0")) {
                registerChange("sector height change");
                s_ptr->z = 0.0f;
                updateAllEdgeOffsetCaches();
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        setTooltip(
            "Height of the floor. Positive numbers are higher.\n"
            "Right-click for some shortcuts.\n"
            "You can also hold H in the canvas to set a sector's height\n"
            "by moving the cursor up or down.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Sector hazard node.
        ImGui::Spacer();
        if(saveableTreeNode("layout", "Hazard")) {
        
            string hazard_iname;
            if(s_ptr->hazard) {
                hazard_iname = s_ptr->hazard->manifest->internalName;
            }
            if(processGuiHazardManagementWidgets(hazard_iname)) {
                registerChange("sector hazard changes");
                s_ptr->hazard =
                    hazard_iname.empty() ?
                    nullptr :
                    &game.content.hazards.list[hazard_iname];
            }
            setTooltip("This sector's hazard, if any.");
            
            if(!hazard_iname.empty()) {
                //Sector hazard floor only checkbox.
                bool sector_hazard_floor = s_ptr->hazardFloor;
                ImGui::Indent();
                if(ImGui::Checkbox("Floor only", &sector_hazard_floor)) {
                    registerChange("sector hazard floor option change");
                    s_ptr->hazardFloor = sector_hazard_floor;
                }
                ImGui::Unindent();
                setTooltip(
                    "Do the hazards only affects objects on the floor,\n"
                    "or do they affect airborne objects in the sector too?"
                );
            }
            
            //Sector bottomless pit checkbox.
            bool sector_bottomless_pit = s_ptr->isBottomlessPit;
            if(ImGui::Checkbox("Bottomless pit", &sector_bottomless_pit)) {
                registerChange("sector bottomless pit change");
                s_ptr->isBottomlessPit = sector_bottomless_pit;
                if(!sector_bottomless_pit) {
                    updateSectorTexture(s_ptr, s_ptr->textureInfo.bmpName);
                }
            }
            setTooltip(
                "Is this sector's floor a bottomless pit?\n"
                "Pikmin die when they fall in pits,\n"
                "and you can see the background (or void)."
            );
            
            ImGui::TreePop();
        }
        
        //Sector advanced behavior node.
        ImGui::Spacer();
        if(saveableTreeNode("layout", "Advanced")) {
        
            //Sector type combobox.
            vector<string> types_list;
            for(
                size_t t = 0; t < game.sectorTypes.getNrOfItems(); t++
            ) {
                types_list.push_back(
                    strToSentence(game.sectorTypes.getName((SECTOR_TYPE) t))
                );
            }
            int sector_type = s_ptr->type;
            if(ImGui::Combo("Type", &sector_type, types_list, 15)) {
                registerChange("sector type change");
                s_ptr->type = (SECTOR_TYPE) sector_type;
            }
            setTooltip(
                "What type of sector this is."
            );
            
            ImGui::Spacer();
            
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }
    
    //Sector appearance node.
    ImGui::Spacer();
    if(saveableTreeNode("layout", "Appearance")) {
    
        int texture_type = !s_ptr->fade;
        
        //Sector texture fader radio button.
        ImGui::RadioButton("Texture fader", &texture_type, 0);
        setTooltip(
            "Makes the surrounding textures fade into each other."
        );
        
        //Sector regular texture radio button.
        ImGui::RadioButton("Regular texture", &texture_type, 1);
        setTooltip(
            "Makes the sector use a regular texture."
        );
        
        if(s_ptr->fade != (texture_type == 0)) {
            registerChange("sector texture type change");
            s_ptr->fade = texture_type == 0;
            if(!s_ptr->fade) {
                updateSectorTexture(s_ptr, s_ptr->textureInfo.bmpName);
            }
        }
        
        if(!s_ptr->fade) {
        
            ImGui::Indent();
            
            //Sector texture button.
            if(ImGui::Button("Choose image...")) {
                vector<PickerItem> picker_buttons;
                
                picker_buttons.push_back(PickerItem("Choose another..."));
                
                for(size_t s = 0; s < textureSuggestions.size(); s++) {
                    picker_buttons.push_back(
                        PickerItem(
                            textureSuggestions[s].name,
                            "", "", nullptr,
                            "",
                            textureSuggestions[s].bmp
                        )
                    );
                }
                openPickerDialog(
                    "Pick a texture",
                    picker_buttons,
                    std::bind(
                        &AreaEditor::pickTexture, this,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3,
                        std::placeholders::_4,
                        std::placeholders::_5
                    ),
                    "Suggestions:", false, true
                );
            }
            setTooltip("Pick a texture to use.");
            
            //Sector texture name text.
            ImGui::SameLine();
            monoText("%s", s_ptr->textureInfo.bmpName.c_str());
            setTooltip("Internal name:\n" + s_ptr->textureInfo.bmpName);
            
            ImGui::Unindent();
            
        }
        
        //Sector texture effects node.
        ImGui::Spacer();
        if(saveableTreeNode("layout", "Texture effects")) {
        
            //Sector texture offset value.
            Point texture_translation = s_ptr->textureInfo.translation;
            if(ImGui::DragFloat2("Offset", (float*) &texture_translation)) {
                registerChange("sector texture offset change");
                s_ptr->textureInfo.translation = texture_translation;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Offset the texture horizontally or vertically "
                "by this much.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Sector texture scale value.
            Point texture_scale = s_ptr->textureInfo.scale;
            if(ImGui::DragFloat2("Scale", (float*) &texture_scale, 0.01)) {
                registerChange("sector texture scale change");
                s_ptr->textureInfo.scale = texture_scale;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Scale the texture horizontally or vertically "
                "by this much.\n"
                "The scale's anchor point is at the origin "
                "of the area, at coordinates 0,0.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Sector texture rotation value.
            float texture_rotation = normalizeAngle(s_ptr->textureInfo.rot);
            if(ImGui::SliderAngle("Angle", &texture_rotation, 0, 360, "%.2f")) {
                registerChange("sector texture angle change");
                s_ptr->textureInfo.rot = texture_rotation;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Rotate the texture by these many degrees.\n"
                "The rotation's center point is at the origin "
                "of the area, at coordinates 0,0.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            //Sector texture tint value.
            ALLEGRO_COLOR texture_tint = s_ptr->textureInfo.tint;
            if(
                ImGui::ColorEdit4(
                    "Tint color", (float*) &texture_tint,
                    ImGuiColorEditFlags_NoInputs
                )
            ) {
                registerChange("sector texture tint change");
                s_ptr->textureInfo.tint = texture_tint;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Tint the texture with this color. White means no tint."
            );
            
            //On-canvas texture effect editing checkbox.
            bool octee_on =
                subState == EDITOR_SUB_STATE_OCTEE;
            if(ImGui::Checkbox("On-canvas editing", &octee_on)) {
                subState =
                    octee_on ?
                    EDITOR_SUB_STATE_OCTEE :
                    EDITOR_SUB_STATE_NONE;
            }
            setTooltip(
                "Enable on-canvas texture effect editing.\n"
                "With this, you can click and drag on the canvas "
                "to adjust the texture,\n"
                "based on whatever mode is currently active."
            );
            
            if(octee_on) {
            
                ImGui::Indent();
                
                int octee_mode_int = (int) octeeMode;
                
                //On-canvas texture effect editing offset radio button.
                ImGui::RadioButton(
                    "Change offset", &octee_mode_int,
                    (int) OCTEE_MODE_OFFSET
                );
                setTooltip(
                    "Dragging will change the texture's offset.",
                    "1"
                );
                
                //On-canvas texture effect editing scale radio button.
                ImGui::RadioButton(
                    "Change scale", &octee_mode_int,
                    (int) OCTEE_MODE_SCALE
                );
                setTooltip(
                    "Dragging will change the texture's scale.",
                    "2"
                );
                
                //On-canvas texture effect editing angle radio button.
                ImGui::RadioButton(
                    "Change angle", &octee_mode_int,
                    (int) OCTEE_MODE_ANGLE
                );
                setTooltip(
                    "Dragging will change the texture's angle.",
                    "3"
                );
                
                octeeMode = (OCTEE_MODE) octee_mode_int;
                
                ImGui::Unindent();
                
            }
            
            ImGui::TreePop();
        }
        
        //Sector mood node.
        ImGui::Spacer();
        if(saveableTreeNode("layout", "Sector mood")) {
        
            //Sector brightness value.
            int sector_brightness = s_ptr->brightness;
            ImGui::SetNextItemWidth(180);
            if(ImGui::SliderInt("Brightness", &sector_brightness, 0, 255)) {
                registerChange("sector brightness change");
                s_ptr->brightness = sector_brightness;
            }
            setTooltip(
                "How bright the sector is. Affects not just the sector's "
                "appearance, but everything inside it.\n"
                "0 is fully dark, 255 is fully lit.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            ImGui::Spacer();
            
            ImGui::TreePop();
        }
        
        ImGui::Spacer();
        
        ImGui::TreePop();
    }
    
    homogenizeSelectedSectors();
}


/**
 * @brief Processes the Dear ImGui tools control panel for this frame.
 */
void AreaEditor::processGuiPanelTools() {
    ImGui::BeginChild("tools");
    
    //Back button.
    if(ImGui::Button("Back")) {
        saveReference();
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("TOOLS");
    
    //Reference image node.
    if(saveableTreeNode("tools", "Reference image")) {
    
        //Remove reference image button.
        unsigned char rem_ref_opacity = referenceFilePath.empty() ? 50 : 255;
        if(
            ImGui::ImageButton(
                "remRefButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                COLOR_EMPTY, mapAlpha(rem_ref_opacity)
            )
        ) {
            referenceFilePath.clear();
            updateReference();
        }
        setTooltip(
            "Remove the reference image.\n"
            "This does not delete the file on your disk."
        );
        
        //Browse for a reference image button.
        ImGui::SameLine();
        if(ImGui::Button("Browse...")) {
            vector<string> f =
                promptFileDialog(
                    "",
                    "Please choose the bitmap to use for a reference.",
                    "*.*",
                    ALLEGRO_FILECHOOSER_FILE_MUST_EXIST |
                    ALLEGRO_FILECHOOSER_PICTURES,
                    game.display
                );
                
            if(!f.empty() && !f[0].empty()) {
                referenceFilePath = f[0];
            }
            updateReference();
        }
        setTooltip(
            "Browse for a file on your disk to use."
        );
        
        //Reference image name text.
        string ref_file_name =
            getPathLastComponent(referenceFilePath);
        ImGui::SameLine();
        monoText("%s", ref_file_name.c_str());
        setTooltip("Full path:\n" + referenceFilePath);
        
        //Reference center value.
        ImGui::DragFloat2("Center", (float*) &referenceCenter);
        setTooltip(
            "Center coordinates.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Reference size value.
        processGuiSizeWidgets(
            "Size", referenceSize, 1.0f,
            referenceKeepAspectRatio, false,
            AREA_EDITOR::REFERENCE_MIN_SIZE
        );
        setTooltip(
            "Width and height.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Reference keep aspect ratio checkbox.
        ImGui::Indent();
        ImGui::Checkbox(
            "Keep aspect ratio",
            &referenceKeepAspectRatio
        );
        ImGui::Unindent();
        setTooltip("Keep the aspect ratio when resizing the image.");
        
        //Reference opacity value.
        int opacity = referenceAlpha;
        ImGui::SliderInt("Opacity", &opacity, 0, 255);
        referenceAlpha = opacity;
        setTooltip(
            "How opaque it is.",
            "", WIDGET_EXPLANATION_SLIDER
        );
        
        ImGui::TreePop();
        
    }
    
    //Misc. node.
    ImGui::Spacer();
    if(saveableTreeNode("tools", "Misc.")) {
    
        //Load auto-backup button.
        if(ImGui::Button("Load auto-backup")) {
            changesMgr.askIfUnsaved(
                Point(),
                "loading the auto-backup", "load",
            [this] () {
                bool backup_exists = false;
                if(!manifest.internalName.empty()) {
                    string file_path =
                        game.curAreaData->userDataPath + "/" + FILE_NAMES::AREA_GEOMETRY;
                    if(al_filename_exists(file_path.c_str())) {
                        backup_exists = true;
                    }
                }
                
                if(backup_exists) {
                    loadBackup();
                } else {
                    setStatus("There is no backup available.");
                }
            },
            [this] () { return saveArea(false); }
            );
        }
        setTooltip(
            "Discard all changes made and load the auto-backup, if any exists."
        );
        
        //Resize everything multiplier value.
        static float resize_mults[2] = { 1.0f, 1.0f };
        ImGui::SetNextItemWidth(128.0f);
        ImGui::DragFloat2("##resizeMult", resize_mults, 0.01);
        setTooltip(
            "Resize multipliers, vertically and horizontally.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Resize everything button.
        ImGui::SameLine();
        if(ImGui::Button("Resize everything")) {
            if(resize_mults[0] == 0.0f || resize_mults[1] == 0.0f) {
                setStatus(
                    "Can't resize everything to size 0!",
                    true
                );
            } else if(resize_mults[0] == 1.0f && resize_mults[1] == 1.0f) {
                setStatus(
                    "Resizing everything by 1 wouldn't make a difference!",
                    true
                );
            } else {
                registerChange("global resize");
                resizeEverything(resize_mults);
                setStatus(
                    "Resized everything by " + f2s(resize_mults[0]) + ", " +
                    f2s(resize_mults[1]) + "."
                );
                resize_mults[0] = 1.0f;
                resize_mults[1] = 1.0f;
            }
        }
        setTooltip(
            "Resize everything in the area by the specified multiplier.\n"
            "0.5 will resize everything to half size, 2.0 to double, etc."
        );
        
        ImGui::Spacer();
        
        ImGui::TreePop();
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void AreaEditor::processGuiStatusBar() {
    //Status bar text.
    processGuiStatusBarText();
    
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        canvasSeparatorX - ImGui::GetItemRectSize().x -
        EDITOR::MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    //Mouse coordinates text.
    if(!isMouseInGui || isM1Pressed) {
        ImGui::SameLine();
        monoText(
            "%s, %s",
            boxString(f2s(game.mouseCursor.worldPos.x), 7).c_str(),
            boxString(f2s(game.mouseCursor.worldPos.y), 7).c_str()
        );
    }
}


/**
 * @brief Processes the Dear ImGui toolbar for this frame.
 */
void AreaEditor::processGuiToolbar() {
    if(manifest.internalName.empty() || !game.curAreaData) return;
    
    //Quit button.
    if(
        ImGui::ImageButton(
            "quitButton", editorIcons[EDITOR_ICON_QUIT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        quitWidgetPos = getLastWidgetPost();
        quitCmd(1.0f);
    }
    setTooltip(
        "Quit the area editor.",
        "Ctrl + Q"
    );
    
    //Load button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "loadButton", editorIcons[EDITOR_ICON_LOAD],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        loadWidgetPos = getLastWidgetPost();
        loadCmd(1.0f);
    }
    setTooltip(
        "Pick an area to load, or create a new one.",
        "Ctrl + L"
    );
    
    //Save button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "saveButton",
            changesMgr.hasUnsavedChanges() ?
            editorIcons[EDITOR_ICON_SAVE_UNSAVED] :
            editorIcons[EDITOR_ICON_SAVE],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        saveCmd(1.0f);
    }
    setTooltip(
        "Save the area into the files on disk.",
        "Ctrl + S"
    );
    
    //Play button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "playButton", editorIcons[EDITOR_ICON_PLAY],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        quickPlayCmd(1.0f);
    }
    setTooltip(
        "Save, quit, and start playing the area. Leaving will return "
        "to the editor.",
        "Ctrl + P"
    );
    
    //Undo button.
    unsigned char undo_opacity = undoHistory.empty() ? 50 : 255;
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            "undoButton", editorIcons[EDITOR_ICON_UNDO],
            Point(EDITOR::ICON_BMP_SIZE),
            Point(0.0f), Point(1.0f),
            COLOR_EMPTY, mapAlpha(undo_opacity)
        )
    ) {
        undoCmd(1.0f);
    }
    string undo_text;
    if(undoHistory.empty()) {
        undo_text = "Nothing to undo.";
    } else {
        undo_text = "Undo: " + undoHistory.front().second + ".";
    }
    setTooltip(
        undo_text,
        "Ctrl + Z"
    );
    
    //Redo button.
    unsigned redo_opacity = redoHistory.empty() ? 50 : 255;
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "redoButton", editorIcons[EDITOR_ICON_UNDO],
            Point(EDITOR::ICON_BMP_SIZE),
            Point(1.0f, 0.0f), Point(0.0f, 1.0f),
            COLOR_EMPTY, mapAlpha(redo_opacity)
        )
    ) {
        redoCmd(1.0f);
    }
    string redo_text;
    if(redoHistory.empty()) {
        redo_text =
            "Nothing to redo.";
    } else {
        redo_text =
            "Redo: " + redoHistory.front().second + ".";
    }
    setTooltip(
        redo_text,
        "Ctrl + Y"
    );
    
    if(!referenceFilePath.empty()) {
    
        //Reference image toggle button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "refToggleButton", editorIcons[EDITOR_ICON_REFERENCE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            referenceToggleCmd(1.0f);
        }
        setTooltip(
            "Toggle the visibility of the reference image.",
            "Ctrl + R"
        );
        
        //Reference image opacity value.
        int reference_alpha_int = referenceAlpha;
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(0.0f, 0.0f));
        ImGui::SetNextItemWidth(48.0f);
        ImGui::SliderInt("##refAlpha", &reference_alpha_int, 0, 255, "");
        setTooltip(
            "Opacity of the reference image.",
            "", WIDGET_EXPLANATION_SLIDER
        );
        ImGui::EndGroup();
        referenceAlpha = reference_alpha_int;
        
    }
    
    //Snap mode button.
    ALLEGRO_BITMAP* snap_mode_bmp = nullptr;
    string snap_mode_description;
    switch(game.options.areaEd.snapMode) {
    case SNAP_MODE_GRID: {
        snap_mode_bmp = editorIcons[EDITOR_ICON_SNAP_GRID];
        snap_mode_description = "grid. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_VERTEXES: {
        snap_mode_bmp = editorIcons[EDITOR_ICON_SNAP_VERTEXES];
        snap_mode_description = "vertexes. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_EDGES: {
        snap_mode_bmp = editorIcons[EDITOR_ICON_SNAP_EDGES];
        snap_mode_description = "edges. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_NOTHING: {
        snap_mode_bmp = editorIcons[EDITOR_ICON_SNAP_NOTHING];
        snap_mode_description = "off. Holding Shift snaps to grid.";
        break;
    } case N_SNAP_MODES: {
        break;
    }
    }
    
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "snapButton", snap_mode_bmp,
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        snapModeCmd(1.0f);
    }
    setTooltip(
        "Current snap mode: " + snap_mode_description,
        "X or Shift + X"
    );
    
    if(game.options.areaEd.advancedMode) {
    
        //Layout mode button.
        ImGui::SameLine(0, 16);
        if(
            ImGui::ImageButton(
                "layoutButton", editorIcons[EDITOR_ICON_SECTORS],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            changeState(EDITOR_STATE_LAYOUT);
        }
        setTooltip(
            "Swaps to the layout editing mode.",
            "L"
        );
        
        //Mobs mode button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "mobsButton", editorIcons[EDITOR_ICON_MOBS],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            changeState(EDITOR_STATE_MOBS);
        }
        setTooltip(
            "Swaps to the objects editing mode.",
            "O"
        );
        
        //Paths mode button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "pathsButton", editorIcons[EDITOR_ICON_PATHS],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            changeState(EDITOR_STATE_PATHS);
        }
        setTooltip(
            "Swaps to the paths editing mode.",
            "P"
        );
        
        //Details mode button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "detailsButton", editorIcons[EDITOR_ICON_DETAILS],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            changeState(EDITOR_STATE_DETAILS);
        }
        setTooltip(
            "Swaps to the details editing mode.",
            "D"
        );
        
        //Toggle preview mode button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "previewButton", editorIcons[EDITOR_ICON_REVIEW],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            previewMode = !previewMode;
        }
        setTooltip(
            "Toggles area preview mode. More info in the review panel.",
            "Shift + P"
        );
        
    }
}
