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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../lib/imgui/imgui_stdlib.h"
#include "../../util/allegro_utils.h"
#include "../../util/enum_utils.h"
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
    
    for(
        size_t a = 0; a < game.content.areas.list[AREA_TYPE_SIMPLE].size(); a++
    ) {
        Area* areaPtr = game.content.areas.list[AREA_TYPE_SIMPLE][a];
        ContentManifest* man = areaPtr->manifest;
        areas.push_back(
            PickerItem(
                areaPtr->name,
                "Pack: " + game.content.packs.list[man->pack].name,
                "Simple", (void*) man,
                getFolderTooltip(man->path, ""),
                areaPtr->thumbnail.get()
            )
        );
    }
    for(
        size_t a = 0; a < game.content.areas.list[AREA_TYPE_MISSION].size(); a++
    ) {
        Area* areaPtr = game.content.areas.list[AREA_TYPE_MISSION][a];
        ContentManifest* man = areaPtr->manifest;
        areas.push_back(
            PickerItem(
                areaPtr->name,
                "Pack: " + game.content.packs.list[man->pack].name,
                "Mission", (void*) man,
                getFolderTooltip(man->path, ""),
                areaPtr->thumbnail.get()
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
    processGuiCanvas();
    ImGui::GetWindowDrawList()->AddCallback(
        drawCanvasDearImGuiCallback, nullptr
    );
    
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
    string explanationStr;
    if(!changesMgr.existsOnDisk()) {
        explanationStr =
            "You have never saved this area to your disk, so if you\n"
            "delete, you will only lose your unsaved progress.";
    } else {
        explanationStr =
            "If you delete, you will lose all unsaved progress,\n"
            "and the area's files in your disk will be gone FOREVER!";
    }
    ImGui::SetupCentering(ImGui::CalcTextSize(explanationStr.c_str()).x);
    ImGui::Text("%s", explanationStr.c_str());
    
    //Final warning text.
    string finalWarningStr =
        "Are you sure you want to delete the current area?";
    ImGui::SetupCentering(ImGui::CalcTextSize(finalWarningStr.c_str()).x);
    ImGui::TextColored(
        ImVec4(0.8, 0.6, 0.6, 1.0),
        "%s", finalWarningStr.c_str()
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
 * @param valuePtr Points to the value of the points value.
 * @param criterionIdx Criterion index.
 * @param widgetLabel Label for the main value widget.
 * @param tooltip Start of the tooltip for this criterion's value widget.
 */
void AreaEditor::processGuiGradingCriterionWidgets(
    int* valuePtr, MISSION_SCORE_CRITERIA criterionIdx,
    const string& widgetLabel, const string& tooltip
) {
    //Main value.
    ImGui::SetNextItemWidth(50);
    int pointsInt = *valuePtr;
    if(ImGui::DragInt(widgetLabel.c_str(), &pointsInt, 0.1f)) {
        registerChange("mission grading change");
        *valuePtr = pointsInt;
    }
    setTooltip(
        tooltip + "\n"
        "Negative numbers means the player loses points.\n"
        "0 means this criterion doesn't count.",
        "", WIDGET_EXPLANATION_DRAG
    );
    if(*valuePtr != 0) {
        ImGui::Indent();
        
        //Loss on fail checkbox.
        int flags = game.curAreaData->missionOld.pointLossData;
        if(
            ImGui::CheckboxFlags(
                ("0 points on fail##zpof" + i2s(criterionIdx)).c_str(),
                &flags,
                getIdxBitmask(criterionIdx)
            )
        ) {
            registerChange("mission grading change");
            game.curAreaData->missionOld.pointLossData = flags;
        }
        setTooltip(
            "If checked, the player will receive 0 points for\n"
            "this criterion if they fail the mission."
        );
        
        //Use in HUD checkbox.
        flags = game.curAreaData->missionOld.pointHudData;
        if(
            ImGui::CheckboxFlags(
                ("Use in HUD counter##uihc" + i2s(criterionIdx)).c_str(),
                &flags, getIdxBitmask(MISSION_SCORE_CRITERIA_PIKMIN_BORN)
            )
        ) {
            registerChange("mission grading change");
            game.curAreaData->missionOld.pointHudData = flags;
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
 * @param requirementPtr Points to the requirement value for this medal.
 * @param widgetLabel Label for the value widget.
 * @param widgetMinValue Minimum value for the value widget.
 * @param widgetMaxValue Maximum value for the value widget.
 * @param tooltip Tooltip for the value widget.
 */
void AreaEditor::processGuiGradingMedalWidgets(
    int* requirementPtr, const string& widgetLabel,
    int widgetMinValue, int widgetMaxValue,
    const string& tooltip
) {
    //Requirement value.
    int req = *requirementPtr;
    ImGui::SetNextItemWidth(90);
    if(
        ImGui::DragInt(
            widgetLabel.c_str(), &req, 1.0f, widgetMinValue, widgetMaxValue
        )
    ) {
        registerChange("mission grading change");
        *requirementPtr = req;
    }
    setTooltip(tooltip, "", WIDGET_EXPLANATION_DRAG);
}


/**
 * @brief Processes the Dear ImGui widgets regarding a grading mode
 * for this frame.
 *
 * @param value Internal value for this mode's radio button.
 * @param widgetLabel Label for the radio widget.
 * @param tooltip Tooltip for the radio widget.
 */
void AreaEditor::processGuiGradingModeWidgets(
    int value, const string& widgetLabel, const string& tooltip
) {
    //Radio button.
    int mode = game.curAreaData->mission.gradingMode;
    if(ImGui::RadioButton(widgetLabel.c_str(), &mode, value)) {
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
    [this](const string& name) -> string {
        return name;
    },
    [this](const string& path) {
        closeTopDialog();
        loadAreaFolder(path, false, true);
    },
    [this](const string& path) {
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
                "Lose all changes and reload the current area from your disk."
            );
            
            //Save current area item.
            if(ImGui::MenuItem("Save current area", "Ctrl+S")) {
                saveCmd(1.0f);
            }
            setTooltip(
                "Save the GUI definition to your disk.",
                "Ctrl + S"
            );
            
            //Delete current area item.
            if(ImGui::MenuItem("Delete current area")) {
                deleteAreaCmd(1.0f);
            }
            setTooltip(
                "Delete the current area from your disk."
            );
            
            //Open externally item.
            if(ImGui::MenuItem("Open externally")) {
                openExternallyCmd(1.0f);
            }
            setTooltip(
                "Open the folder with the area's data in your "
                "operative system.\n"
                "Useful if you need to edit things by hand."
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
            string undoText;
            if(undoHistory.empty()) {
                undoText = "Nothing to undo.";
            } else {
                undoText = "Undo: " + undoHistory.front().second + ".";
            }
            setTooltip(
                undoText,
                "Ctrl + Z"
            );
            
            //Redo item.
            if(ImGui::MenuItem("Redo", "Ctrl+Y")) {
                redoCmd(1.0f);
            }
            string redoText;
            if(redoHistory.empty()) {
                redoText =
                    "Nothing to redo.";
            } else {
                redoText =
                    "Redo: " + redoHistory.front().second + ".";
            }
            setTooltip(
                redoText,
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
                string stateStr =
                    game.options.editors.showTooltips ? "Enabled" : "Disabled";
                setStatus(stateStr + " tooltips.");
                saveOptions();
            }
            setTooltip(
                "Whether tooltips should appear when you place your mouse on\n"
                "top of something in the GUI. Like the tooltip you are\n"
                "reading right now."
            );
            
            //General help item.
            if(ImGui::MenuItem("Help...")) {
                string helpStr =
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
                openHelpDialog(helpStr, "area.html");
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
 * @brief Processes the Dear ImGui "change mission preset" dialog
 * for this frame.
 */
void AreaEditor::processGuiMissionPresetDialog() {
    //Explanation text.
    string explanationStr =
        "If you change the preset, whatever mission data\n"
        "the area had before will be LOST.\n"
        "If you choose \"custom\", whatever was there before\n"
        "will be kept, and you can then customize the mission in depth.";
    ImGui::SetupCentering(ImGui::CalcTextSize(explanationStr.c_str()).x);
    ImGui::Text("%s", explanationStr.c_str());
    
    //New preset combobox.
    int presetInt = missionPresetDialogPreset;
    if(
        ImGui::Combo(
            "New preset", &presetInt, enumGetNames(missionPresetNames), 15
        )
    ) {
        missionPresetDialogPreset = (MISSION_PRESET) presetInt;
    }
    setTooltip("The new preset.");
    
    //Cancel button.
    ImGui::Spacer();
    ImGui::SetupCentering(100 + 100 + 30);
    if(ImGui::Button("Cancel", ImVec2(100, 40))) {
        closeTopDialog();
    }
    setTooltip("Cancel.");
    
    //Change button.
    ImGui::SameLine(0.0f, 30);
    if(ImGui::Button("Change", ImVec2(100, 40))) {
        registerChange("mission preset change");
        game.curAreaData->mission.applyPreset(missionPresetDialogPreset);
        closeTopDialog();
    }
    setTooltip("Apply the new preset.");
}


/**
 * @brief Processes the Dear ImGui mob script vars for this frame.
 *
 * @param mPtr Mob to process.
 */
void AreaEditor::processGuiMobScriptVars(MobGen* mPtr) {
    if(!mPtr->type) return;
    
    map<string, string> varsMap = getVarMap(mPtr->vars);
    map<string, string> newVarsMap;
    map<string, bool> varsInWidgets;
    
    //Start with the properties that apply to all objects.
    
    //Team property.
    string teamVar;
    if(isInMap(varsMap, "team")) {
        teamVar = varsMap["team"];
    }
    
    vector<string> teamNames = enumGetNames(mobTeamNames);
    teamNames.insert(teamNames.begin(), "(Default)");
    
    int teamNr;
    if(teamVar.empty()) {
        teamNr = 0;
    } else {
        bool found;
        MOB_TEAM team = enumGetValue(mobTeamINames, teamVar, &found);
        if(!found) {
            teamNr = 0;
        } else {
            //0 is reserved in this widget for "default".
            //Increase it by one to get the widget's team index number.
            teamNr = ((int) team) + 1;
        }
    }
    
    if(ImGui::Combo("Team", &teamNr, teamNames, 15)) {
        registerChange("object script vars change");
        if(teamNr > 0) {
            //0 is reserved in this widget for "default".
            //Decrease it by one to get the real team index number.
            teamNr--;
            teamVar = enumGetName(mobTeamINames, teamNr);
        } else {
            teamVar.clear();
        }
    }
    setTooltip(
        "What sort of team this object belongs to.\n"
        "(Variable name: \"team\".)"
    );
    
    if(!teamVar.empty()) newVarsMap["team"] = teamVar;
    varsInWidgets["team"] = true;
    
    //Health property.
    float maxHealth = mPtr->type->maxHealth;
    if(isInMap(varsMap, "max_health")) {
        maxHealth = s2f(varsMap["max_health"]);
    }
    float health = maxHealth;
    if(isInMap(varsMap, "health")) {
        health = s2f(varsMap["health"]);
    }
    
    if(ImGui::DragFloat("Health", &health, 0.25f, 0.0f, maxHealth)) {
        registerChange("object script vars change");
    }
    setTooltip(
        "Starting health for this specific object.\n"
        "(Variable name: \"health\".)",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    if(health != maxHealth) {
        newVarsMap["health"] = f2s(health);
    }
    varsInWidgets["health"] = true;
    
    //Max health property.
    if(ImGui::DragFloat("Max health", &maxHealth, 0.25f, 0.0f, FLT_MAX)) {
        registerChange("object script vars change");
    }
    setTooltip(
        "Maximum health for this specific object.\n"
        "The object type's default is " + f2s(mPtr->type->maxHealth) + ".\n"
        "(Variable name: \"max_health\".)",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    if(maxHealth != mPtr->type->maxHealth) {
        newVarsMap["max_health"] = f2s(maxHealth);
    }
    varsInWidgets["max_health"] = true;
    
    //Now, dynamically create widgets for all properties this mob type has.
    
    for(size_t p = 0; p < mPtr->type->areaEditorProps.size(); p++) {
    
        MobType::AreaEditorProp* pPtr =
            &mPtr->type->areaEditorProps[p];
            
        string value;
        if(!isInMap(varsMap, pPtr->var)) {
            value = pPtr->defValue;
        } else {
            value = varsMap[pPtr->var];
        }
        
        switch(pPtr->type) {
        case AEMP_TYPE_TEXT: {
    
            string valueS = value;
            if(ImGui::InputText(pPtr->name.c_str(), &valueS)) {
                registerChange("object script vars change");
                value = valueS;
            }
            
            break;
            
        } case AEMP_TYPE_INT: {
    
            int valueI = s2i(value);
            if(
                ImGui::DragInt(
                    pPtr->name.c_str(), &valueI, 0.02f,
                    pPtr->minValue, pPtr->maxValue
                )
            ) {
                registerChange("object script vars change");
                value = i2s(valueI);
            }
            
            break;
            
        } case AEMP_TYPE_FLOAT: {
    
            float valueF = s2f(value);
            if(
                ImGui::DragFloat(
                    pPtr->name.c_str(), &valueF, 0.1f,
                    pPtr->minValue, pPtr->maxValue
                )
            ) {
                registerChange("object script vars change");
                value = f2s(valueF);
            }
            
            break;
            
        } case AEMP_TYPE_BOOL: {
    
            bool valueB = s2b(value);
            if(ImGui::Checkbox(pPtr->name.c_str(), &valueB)) {
                registerChange("object script vars change");
                value = b2s(valueB);
            }
            
            break;
            
        } case AEMP_TYPE_LIST: {
    
            string valueS = value;
            if(ImGui::Combo(pPtr->name, &valueS, pPtr->valueList, 15)) {
                registerChange("object script vars change");
                value = valueS;
            }
            
            break;
            
        } case AEMP_TYPE_NR_LIST: {
    
            int itemIdx = s2i(value);
            if(ImGui::Combo(pPtr->name, &itemIdx, pPtr->valueList, 15)) {
                registerChange("object script vars change");
                value = i2s(itemIdx);
            }
            
            break;
            
        }
        }
        
        setTooltip(
            wordWrap(pPtr->tooltip, 50) +
            (pPtr->tooltip.empty() ? "" : "\n") +
            "(Variable name: \"" + pPtr->var + "\".)",
            "",
            (pPtr->type == AEMP_TYPE_INT || pPtr->type == AEMP_TYPE_FLOAT) ?
            WIDGET_EXPLANATION_DRAG :
            WIDGET_EXPLANATION_NONE
        );
        
        if(value != pPtr->defValue) {
            newVarsMap[pPtr->var] = value;
        }
        
        varsInWidgets[pPtr->var] = true;
        
    }
    
    string otherVarsStr;
    for(auto const& v : varsMap) {
        if(!varsInWidgets[v.first]) {
            otherVarsStr += v.first + "=" + v.second + ";";
        }
    }
    
    mPtr->vars.clear();
    for(auto const& v : newVarsMap) {
        mPtr->vars += v.first + "=" + v.second + ";";
    }
    mPtr->vars += otherVarsStr;
    
    if(!mPtr->vars.empty() && mPtr->vars[mPtr->vars.size() - 1] == ';') {
        mPtr->vars.pop_back();
    }
    
    //Finally, a widget for the entire list.
    string mobVars = mPtr->vars;
    ImGui::Spacer();
    if(monoInputText("Full list", &mobVars)) {
        registerChange("object script vars change");
        mPtr->vars = mobVars;
    }
    setTooltip(
        "This is the full list of script variables to use.\n"
        "You can add variables here, though variables in the "
        "wrong format will be removed.\n"
        "Format example: \"sleep=y;jumping=n\"."
    );
}


/**
 * @brief Processes the Dear ImGui "new" dialog for this frame.
 */
void AreaEditor::processGuiNewDialog() {
    string problem;
    bool hitCreateButton = false;
    
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
        hitCreateButton = true;
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
    ContentManifest tempMan;
    tempMan.pack = newDialog.pack;
    tempMan.internalName = newDialog.internalName;
    newDialog.areaPath =
        game.content.areas.manifestToPath(
            tempMan, (AREA_TYPE) newDialog.type
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
        hitCreateButton = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    setTooltip(
        problem.empty() ? "Create the area!" : problem
    );
    
    //Creation logic.
    if(hitCreateButton) {
        if(!problem.empty()) return;
        auto reallyCreate = [this] () {
            createArea(newDialog.areaPath);
            closeTopDialog();
            closeTopDialog(); //Close the load dialog.
        };
        
        if(
            newDialog.pack == FOLDER_NAMES::BASE_PACK &&
            !game.options.advanced.engineDev
        ) {
            openBaseContentWarningDialog(reallyCreate);
        } else {
            reallyCreate();
        }
    }
}


/**
 * @brief Processes the options dialog for this frame.
 */
void AreaEditor::processGuiOptionsDialog() {
    //Controls node.
    if(saveableTreeNode("options", "Controls")) {
    
        //Snap threshold value.
        int snapThreshold = (int) game.options.areaEd.snapThreshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Snap threshold", &snapThreshold,
            0.1f, 0, INT_MAX
        );
        setTooltip(
            "Mouse cursor must be these many pixels close\n"
            "to a vertex/edge in order to snap there.\n"
            "Default: " +
            i2s(OPTIONS::AREA_ED_D::SNAP_THRESHOLD) + ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.areaEd.snapThreshold = snapThreshold;
        
        //Middle mouse button pans checkbox.
        ImGui::Checkbox("Use MMB to pan", &game.options.editors.mmbPan);
        setTooltip(
            "Use the middle mouse button to pan the camera\n"
            "(and RMB to reset camera/zoom).\n"
            "Default: " +
            b2s(OPTIONS::EDITORS_D::MMB_PAN) + "."
        );
        
        //Drag threshold value.
        int dragThreshold = (int) game.options.editors.mouseDragThreshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Drag threshold", &dragThreshold,
            0.1f, 0, INT_MAX
        );
        setTooltip(
            "Mouse cursor must move these many pixels "
            "to be considered a drag.\n"
            "Default: " + i2s(OPTIONS::EDITORS_D::MOUSE_DRAG_THRESHOLD) +
            ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.editors.mouseDragThreshold = dragThreshold;
        
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
        int viewMode = game.options.areaEd.viewMode;
        ImGui::Text("View mode:");
        
        ImGui::Indent();
        
        //Textures view mode radio button.
        ImGui::RadioButton("Textures", &viewMode, VIEW_MODE_TEXTURES);
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
        ImGui::RadioButton("Wireframe", &viewMode, VIEW_MODE_WIREFRAME);
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
        ImGui::RadioButton("Heightmap", &viewMode, VIEW_MODE_HEIGHTMAP);
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
        ImGui::RadioButton("Brightness", &viewMode, VIEW_MODE_BRIGHTNESS);
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
        game.options.areaEd.viewMode = (VIEW_MODE) viewMode;
        
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
        int interfaceModeI = (int) game.options.areaEd.advancedMode;
        ImGui::Indent();
        ImGui::RadioButton("Basic", &interfaceModeI, 0);
        setTooltip(
            "Only shows basic GUI items. Recommended for starters\n"
            "so that the interface isn't overwhelming. See the\n"
            "\"Advanced\" option's description for a list of such items."
        );
        
        //Advanced interface button.
        ImGui::RadioButton("Advanced", &interfaceModeI, 1);
        setTooltip(
            "Shows and enables some advanced GUI items:\n"
            "- Toolbar buttons (and shortcut keys) to quickly swap "
            "modes with.\n"
            "- Toolbar button to toggle preview mode with."
        );
        ImGui::Unindent();
        game.options.areaEd.advancedMode = (bool) interfaceModeI;
        
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
        int backupInterval = game.options.areaEd.backupInterval;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Auto-backup interval", &backupInterval, 1, 0, INT_MAX
        );
        setTooltip(
            "Interval between auto-backup saves, in seconds. 0 = off.\n"
            "Default: " + i2s(OPTIONS::AREA_ED_D::BACKUP_INTERVAL) +
            ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.areaEd.backupInterval = backupInterval;
        
        //Undo limit value.
        size_t oldUndoLimit = game.options.areaEd.undoLimit;
        int undoLimit = (int) game.options.areaEd.undoLimit;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Undo limit", &undoLimit, 0.1, 0, INT_MAX
        );
        setTooltip(
            "Maximum number of operations that can be undone. 0 = off.\n"
            "Default: " + i2s(OPTIONS::AREA_ED_D::UNDO_LIMIT) + ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.areaEd.undoLimit = undoLimit;
        
        if(game.options.areaEd.undoLimit != oldUndoLimit) {
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
        
            //Setup.
            processGuiListNavSetup(
                &selectedShadowIdx, game.curAreaData->treeShadows.size(), true
            );
            
            //Current shadow text.
            processGuiListNavCurWidget(
                selectedShadowIdx, game.curAreaData->treeShadows.size(),
                "Tree shadow"
            );
            
            //New shadow button.
            if(
                processGuiListNavNewWidget(
                    &selectedShadowIdx, game.curAreaData->treeShadows.size(),
                    "Start creating a new tree shadow.\n"
                    "Click on the canvas where you want the shadow to be.",
                    false, "", 1.0f, "N"
                )
            ) {
                addNewTreeShadowCmd(1.0f);
            }
            
            //Delete shadow button.
            size_t prevSelectedShadowIdx = selectedShadowIdx;
            if(
                processGuiListNavDelWidget(
                    &selectedShadowIdx, game.curAreaData->treeShadows.size(),
                    "Delete the selected tree shadow.", true, "", 1.0f, "Delete"
                )
            ) {
                selectedShadowIdx = prevSelectedShadowIdx;
                deleteTreeShadowCmd(1.0f);
            }
            
            //Previous shadow button.
            if(
                processGuiListNavPrevWidget(
                    &selectedShadowIdx, game.curAreaData->treeShadows.size(),
                    "Select the previous tree shadow.", true
                )
            ) {
                selectedShadow =
                    game.curAreaData->treeShadows[selectedShadowIdx];
            }
            
            //Next shadow button.
            if(
                processGuiListNavNextWidget(
                    &selectedShadowIdx, game.curAreaData->treeShadows.size(),
                    "Select the next tree shadow.", true
                )
            ) {
                selectedShadow =
                    game.curAreaData->treeShadows[selectedShadowIdx];
            }
            
            ImGui::Spacer();
            
            if(selectedShadow) {
            
                //Choose the tree shadow image button.
                if(ImGui::Button("Choose image...")) {
                    openBitmapDialog(
                    [this] (const string& bmp) {
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
                Point shadowCenter = selectedShadow->pose.pos;
                if(
                    ImGui::DragFloat2("Center", (float*) &shadowCenter)
                ) {
                    registerChange("tree shadow center change");
                    selectedShadow->pose.pos = shadowCenter;
                }
                setTooltip(
                    "Center coordinates of the tree shadow.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Tree shadow size value.
                Point shadowSize = selectedShadow->pose.size;
                if(
                    processGuiSizeWidgets(
                        "Size", shadowSize,
                        1.0f, selectedShadowKeepAspectRatio, false,
                        -FLT_MAX
                    )
                ) {
                    registerChange("tree shadow size change");
                    selectedShadow->pose.size = shadowSize;
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
                float shadowAngle =
                    normalizeAngle(selectedShadow->pose.angle);
                if(
                    ImGui::SliderAngleWithContext(
                        "Angle", &shadowAngle, 0, 360, "%.2f"
                    )
                ) {
                    registerChange("tree shadow angle change");
                    selectedShadow->pose.angle = shadowAngle;
                }
                setTooltip(
                    "Angle of the tree shadow.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Tree shadow opacity value.
                int shadowOpacity = selectedShadow->alpha;
                if(ImGui::SliderInt("Opacity", &shadowOpacity, 0, 255)) {
                    registerChange("tree shadow opacity change");
                    selectedShadow->alpha = shadowOpacity;
                }
                setTooltip(
                    "How opaque the tree shadow is.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Tree shadow sway value.
                Point shadowSway = selectedShadow->sway;
                if(ImGui::DragFloat2("Sway", (float*) &shadowSway, 0.1)) {
                    registerChange("tree shadow sway change");
                    selectedShadow->sway = shadowSway;
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
        
        ImGui::Spacer();
        
        //Regions node.
        if(saveableTreeNode("details", "Regions")) {
        
            //Setup.
            processGuiListNavSetup(
                &selectedRegionIdx, game.curAreaData->regions.size(), true
            );
            
            //Current region text.
            processGuiListNavCurWidget(
                selectedRegionIdx, game.curAreaData->regions.size(),
                "Region"
            );
            
            //New region button.
            if(
                processGuiListNavNewWidget(
                    &selectedRegionIdx, game.curAreaData->regions.size(),
                    "Create a new area region."
                )
            ) {
                addNewRegionCmd(1.0f);
            }
            
            //Delete region button.
            size_t prevSelectedRegionIdx = selectedRegionIdx;
            if(
                processGuiListNavDelWidget(
                    &selectedRegionIdx, game.curAreaData->regions.size(),
                    "Delete the selected area region.", true, "", 1.0, "Delete"
                )
            ) {
                selectedRegionIdx = prevSelectedRegionIdx;
                deleteRegionCmd(1.0f);
            }
            
            //Previous region button.
            if(
                processGuiListNavPrevWidget(
                    &selectedRegionIdx, game.curAreaData->regions.size(),
                    "Select the previous region.", true
                )
            ) {
                selectedRegion =
                    game.curAreaData->regions[selectedRegionIdx];
            }
            
            //Next region button.
            if(
                processGuiListNavNextWidget(
                    &selectedRegionIdx, game.curAreaData->regions.size(),
                    "Select the next tree region.", true
                )
            ) {
                selectedRegion =
                    game.curAreaData->regions[selectedRegionIdx];
            }
            
            ImGui::Spacer();
            
            if(selectedRegion) {
            
                //Region center value.
                Point regionCenter = selectedRegion->center;
                if(
                    ImGui::DragFloat2("Center", (float*) &regionCenter)
                ) {
                    registerChange("region center change");
                    selectedRegion->center = regionCenter;
                };
                setTooltip(
                    "Center coordinates of the region.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Region size value.
                Point regionSize = selectedRegion->size;
                if(
                    ImGui::DragFloat2("Size", (float*) &regionSize)
                ) {
                    registerChange("region size change");
                    selectedRegion->size = regionSize;
                };
                setTooltip(
                    "Width and height of the region.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
            } else {
            
                //"No region selected" text.
                ImGui::TextDisabled("(No region selected)");
                
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
    Edge* ePtr = *selectedEdges.begin();
    
    //Wall shadow node.
    if(saveableTreeNode("layout", "Wall shadow")) {
    
        //Length/presence text.
        ImGui::Text("Length and presence:");
        
        //Automatic length radio button.
        bool autoLength = (ePtr->wallShadowLength == LARGE_FLOAT);
        if(ImGui::RadioButton("Automatic length", autoLength)) {
            if(!autoLength) {
                registerChange("edge shadow length change");
                ePtr->wallShadowLength = LARGE_FLOAT;
                quickPreviewTimer.start();
            }
            autoLength = true;
        }
        setTooltip(
            "The wall shadow's length will depend "
            "on the height of the wall.\n"
            "If it's too short, the wall shadow will also "
            "automatically disappear."
        );
        
        //Never show radio button.
        bool noLength = (ePtr->wallShadowLength == 0.0f);
        if(ImGui::RadioButton("Never show", noLength)) {
            if(!noLength) {
                registerChange("edge shadow length change");
                ePtr->wallShadowLength = 0.0f;
                quickPreviewTimer.start();
            }
            noLength = true;
        }
        setTooltip(
            "The wall shadow will never appear, no matter what."
        );
        
        //Fixed length radio button.
        bool fixedLength = (!noLength && !autoLength);
        if(ImGui::RadioButton("Fixed length", fixedLength)) {
            if(!fixedLength) {
                registerChange("edge shadow length change");
                ePtr->wallShadowLength = 30.0f;
                quickPreviewTimer.start();
            }
            fixedLength = true;
        }
        setTooltip(
            "The wall shadow will always appear, and will "
            "have a fixed length regardless of the wall's height."
        );
        
        //Length value.
        if(fixedLength) {
            float length = ePtr->wallShadowLength;
            if(
                ImGui::DragFloat(
                    "Length", &length, 0.2f,
                    GEOMETRY::SHADOW_MIN_LENGTH, GEOMETRY::SHADOW_MAX_LENGTH
                )
            ) {
                registerChange("edge shadow length change");
                ePtr->wallShadowLength = length;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Length of the shadow.",
                "", WIDGET_EXPLANATION_DRAG
            );
        }
        
        //Shadow color.
        ALLEGRO_COLOR color = ePtr->wallShadowColor;
        ImGui::Spacer();
        if(
            ImGui::ColorEdit4(
                "Color", (float*) &color,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            registerChange("edge shadow color change");
            ePtr->wallShadowColor = color;
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
        float length = ePtr->ledgeSmoothingLength;
        if(
            ImGui::DragFloat(
                "Length", &length, 0.2f,
                0.0f, GEOMETRY::SMOOTHING_MAX_LENGTH
            )
        ) {
            registerChange("edge ledge smoothing length change");
            ePtr->ledgeSmoothingLength = length;
            quickPreviewTimer.start();
        }
        setTooltip(
            "Length of the ledge smoothing effect.\n"
            "Use this to make a ledge leading into a wall look more rounded.\n"
            "0 means there will be no effect.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Smoothing color.
        ALLEGRO_COLOR color = ePtr->ledgeSmoothingColor;
        ImGui::Spacer();
        if(
            ImGui::ColorEdit4(
                "Color", (float*) &color,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            registerChange("edge ledge smoothing color change");
            ePtr->ledgeSmoothingColor = color;
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
    
    if(enableEdgeSectorPatching && selectedEdges.size() == 1) {
    
        //Sector patching node.
        ImGui::Spacer();
        if(saveableTreeNode("layout", "Sector patching")) {
        
            //Information text.
            ImGui::TextWrapped(
                "See: Main panel > Tools > Misc. > Enable edge sector patching."
            );
            
            for(size_t s = 0; s < 2; s++) {
                //Side value.
                int sInt = (int) ePtr->sectorIdxs[s];
                ImGui::SetNextItemWidth(80);
                string label = s == 0 ? "A-side" : "B-side";
                if(ImGui::DragInt(label.c_str(), &sInt)) {
                    if(
                        sInt >= 0 &&
                        sInt <= (int) game.curAreaData->sectors.size()
                    ) {
                        registerChange("edge sector patch");
                        Sector* oldSector = ePtr->sectors[s];
                        size_t newSectorIdx = (size_t) sInt;
                        Sector* newSector =
                            game.curAreaData->sectors[newSectorIdx];
                        size_t edgeIdx = game.curAreaData->findEdgeIdx(ePtr);
                        ePtr->transferSector(
                            oldSector, newSector,
                            newSectorIdx, edgeIdx
                        );
                        updateAffectedSectors({oldSector, newSector});
                    }
                }
                setTooltip(
                    s == 0 ?
                    "Index of the sector on the A-side." :
                    "Index of the sector on the B-side.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Angle arrow widget.
                float angle =
                    getAngle(v2p(ePtr->vertexes[0]), v2p(ePtr->vertexes[1]));
                angle +=
                    s == 0 ? -(TAU / 4.0f) : (TAU / 4.0f);
                ImGui::SameLine();
                angleVisualizer(angle);
                
            }
            
            //Swap sides button.
            if(ImGui::Button("Swap sides")) {
                registerChange("edge sector patch");
                Sector* sector0 = ePtr->sectors[0];
                Sector* sector1 = ePtr->sectors[1];
                std::swap(ePtr->sectorIdxs[0], ePtr->sectorIdxs[1]);
                game.curAreaData->fixEdgePointers(ePtr);
                updateAffectedSectors({sector0, sector1});
            }
            setTooltip("Swap the two sides.");
            
            ImGui::TreePop();
        }
        
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
            f2s(game.curAreaData->missionOld.goalExitCenter.x).c_str(),
            f2s(game.curAreaData->missionOld.goalExitCenter.y).c_str()
        );
        
        //Region center text.
        ImGui::Text(
            "Exit region size: %s x %s",
            f2s(game.curAreaData->missionOld.goalExitSize.x).c_str(),
            f2s(game.curAreaData->missionOld.goalExitSize.y).c_str()
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
        
            map<string, string> sprayStrs =
                getVarMap(game.curAreaData->sprayAmounts);
            for(size_t s = 0; s < game.config.misc.sprayOrder.size(); s++) {
                string sprayInternalName =
                    game.config.misc.sprayOrder[s]->manifest->internalName;
                int amount = s2i(sprayStrs[sprayInternalName]);
                ImGui::SetNextItemWidth(50);
                if(
                    ImGui::DragInt(
                        game.config.misc.sprayOrder[s]->name.c_str(), &amount,
                        0.1, 0, INT_MAX
                    )
                ) {
                    registerChange("area spray amounts change");
                    sprayStrs[sprayInternalName] = i2s(amount);
                    game.curAreaData->sprayAmounts.clear();
                    for(auto const& v : sprayStrs) {
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
        
        //Rules node.
        ImGui::Spacer();
        if(saveableTreeNode("gameplay", "Game rules")) {
        
            //Max Pikmin in field override checkbox.
            bool overrideMaxPik = game.curAreaData->maxPikminInField != INVALID;
            if(
                ImGui::Checkbox(
                    "Override max Pikmin in field", &overrideMaxPik
                )
            ) {
                registerChange("Pikmin maximum override");
                if(overrideMaxPik) {
                    game.curAreaData->maxPikminInField =
                        game.config.rules.maxPikminInField;
                } else {
                    game.curAreaData->maxPikminInField = INVALID;
                }
            }
            setTooltip(
                "Whether to use a custom maximum of Pikmin on the field,\n"
                "or to use the game configuration default."
            );
            
            if(overrideMaxPik) {
            
                //Max Pikmin in field override value.
                int maxPik = (int) game.curAreaData->maxPikminInField;
                ImGui::Indent();
                ImGui::SetNextItemWidth(50);
                if(
                    ImGui::DragInt(
                        "Maximum", &maxPik,
                        0.1, 0, INT_MAX
                    )
                ) {
                    registerChange("Pikmin maximum override");
                    game.curAreaData->maxPikminInField = maxPik;
                }
                ImGui::Unindent();
                setTooltip(
                    "Maximum amount of Pikmin that can be out on the field.", "",
                    WIDGET_EXPLANATION_DRAG
                );
                
            }
            
            //Onions auto eject override checkbox.
            bool onionsAutoEject = game.curAreaData->onionsAutoEject;
            if(ImGui::Checkbox("Onions auto-eject", &onionsAutoEject)) {
                registerChange("Onion auto-eject override");
                game.curAreaData->onionsAutoEject = onionsAutoEject;
            }
            setTooltip(
                "If checked, all Onions will automatically eject Pikmin\n"
                "whenever there is enough free space in the field."
            );
            
            //Onions eject grown Pikmin override checkbox.
            bool onionsEjectGrown = game.curAreaData->onionsEjectGrownPikmin;
            if(ImGui::Checkbox("Onions eject grown Pikmin", &onionsEjectGrown)) {
                registerChange("Onion eject grown Pikmin override");
                game.curAreaData->onionsEjectGrownPikmin = onionsEjectGrown;
            }
            setTooltip(
                "If checked, all Onions will eject fully-grown Pikmin\n"
                "instead of seeds."
            );
            
            ImGui::TreePop();
        }
        
        ImGui::Spacer();
        
        if(game.curAreaData->type == AREA_TYPE_MISSION) {
            processGuiPanelMission();
            processGuiPanelMissionOld();
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
        
            string newTag;
            
            //Gameplay tags combo.
            vector<string> gameplayTags = {
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
            int gameplayTagIdx = -1;
            if(ImGui::Combo("Gameplay", &gameplayTagIdx, gameplayTags, 15)) {
                newTag = gameplayTags[gameplayTagIdx];
            }
            
            //Theme tags combo.
            vector<string> themeTags = {
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
            int themeTagIdx = -1;
            if(ImGui::Combo("Theme", &themeTagIdx, themeTags, 15)) {
                newTag = themeTags[themeTagIdx];
            }
            
            //Misc. tags combo.
            vector<string> miscTags = {
                "Art",
                "Technical",
                "Troll",
                "Tutorial",
            };
            int miscTagIdx = -1;
            if(ImGui::Combo("Misc.", &miscTagIdx, miscTags, 15)) {
                newTag = miscTags[miscTagIdx];
            }
            
            if(!newTag.empty()) {
                registerChange("area tags change");
                if(!game.curAreaData->tags.empty()) {
                    game.curAreaData->tags += "; ";
                }
                game.curAreaData->tags += newTag;
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
        
        //Difficulty combobox.
        int difficulty = game.curAreaData->difficulty;
        vector<string> difficultyOptions = {
            "Not specified", "1", "2", "3", "4", "5"
        };
        if(ImGui::Combo("Difficulty", &difficulty, difficultyOptions, 15)) {
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
        bool validSongSelected =
            !game.curAreaData->songName.empty() &&
            game.curAreaData->songName != NONE_OPTION;
        bool previewing =
            !previewSong.empty();
        bool canPreviewSelectedSong =
            validSongSelected &&
            previewSong != game.curAreaData->songName;
        bool canStopPreviewing =
            previewing &&
            (
                !validSongSelected ||
                previewSong == game.curAreaData->songName
            );
        bool previewButtonValid =
            canPreviewSelectedSong || canStopPreviewing;
            
        if(!previewButtonValid) ImGui::BeginDisabled();
        
        if(
            ImGui::ImageButton(
                "previewSongButton",
                canStopPreviewing ?
                editorIcons[EDITOR_ICON_STOP] :
                editorIcons[EDITOR_ICON_PLAY],
                Point(ImGui::GetTextLineHeight())
            )
        ) {
            if(canPreviewSelectedSong) {
                previewSong = game.curAreaData->songName;
                game.audio.setCurrentSong(previewSong);
                previewing = true;
            } else if(canStopPreviewing) {
                game.audio.setCurrentSong(
                    game.sysContentNames.sngEditors, false
                );
                previewSong.clear();
                previewing = false;
            }
        }
        
        if(!previewButtonValid) ImGui::EndDisabled();
        
        string previewTooltipStr;
        if(previewing) {
            previewTooltipStr +=
                "Currently previewing the song \"" +
                game.content.songs.list[previewSong].name +
                "\".\n";
        }
        if(canPreviewSelectedSong) {
            previewTooltipStr +=
                "Click here to preview the song \"" +
                game.content.songs.list[game.curAreaData->songName].name +
                "\".";
        } else if(canStopPreviewing) {
            previewTooltipStr +=
                "Click here to stop.";
        } else {
            previewTooltipStr +=
                "If you select a song, you can click here to preview it.";
        }
        setTooltip(previewTooltipStr);
        
        //Music combobox.
        ImGui::SameLine();
        vector<string> songInternals;
        vector<string> songNames;
        songInternals.push_back("");
        songNames.push_back(NONE_OPTION);
        for(auto& s : game.content.songs.list) {
            songInternals.push_back(s.first);
            songNames.push_back(s.second.name);
        }
        string songName = game.curAreaData->songName;
        if(ImGui::Combo("Song", &songName, songInternals, songNames, 15)) {
            registerChange("area song change");
            game.curAreaData->songName = songName;
        }
        setTooltip(
            "What song to play."
        );
        
        //Area weather combobox.
        vector<string> weatherCondInternals;
        vector<string> weatherCondNames;
        weatherCondInternals.push_back("");
        weatherCondNames.push_back(NONE_OPTION);
        for(auto& w : game.content.weatherConditions.list) {
            weatherCondInternals.push_back(w.first);
            weatherCondNames.push_back(w.second.name);
        }
        string weatherName = game.curAreaData->weatherName;
        if(
            ImGui::Combo(
                "Weather", &weatherName,
                weatherCondInternals, weatherCondNames, 15
            )
        ) {
            registerChange("area weather change");
            game.curAreaData->weatherName = weatherName;
        }
        setTooltip(
            "The weather condition to use."
        );
        
        ImGui::Spacer();
        
        bool hasTimeLimit = false;
        float missionMin = 0;
        if(game.curAreaData->type == AREA_TYPE_MISSION) {
            if(
                game.curAreaData->missionOld.goal == MISSION_GOAL_TIMED_SURVIVAL
            ) {
                hasTimeLimit = true;
                missionMin =
                    game.curAreaData->missionOld.goalAmount / 60.0f;
            } else if(
                hasFlag(
                    game.curAreaData->missionOld.failConditions,
                    getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
                )
            ) {
                hasTimeLimit = true;
                missionMin =
                    game.curAreaData->missionOld.failTimeLimit / 60.0f;
            }
        }
        int dayStartMin = (int) game.curAreaData->dayTimeStart;
        dayStartMin = wrapFloat(dayStartMin, 0, 60 * 24);
        float daySpeed = game.curAreaData->dayTimeSpeed;
        int dayEndMin = (int) (dayStartMin + missionMin * daySpeed);
        dayEndMin = wrapFloat(dayEndMin, 0, 60 * 24);
        
        //Area day time at start value.
        if(
            ImGui::DragTime2(
                "Start day time", &dayStartMin, "h", "m", 23, 59
            )
        ) {
            registerChange("day time change");
            game.curAreaData->dayTimeStart = dayStartMin;
            if(hasTimeLimit) {
                daySpeed =
                    calculateDaySpeed(
                        dayStartMin, dayEndMin, missionMin
                    );
                game.curAreaData->dayTimeSpeed = daySpeed;
            }
        }
        setTooltip(
            "Point of the (game world) day at which gameplay starts.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        if(hasTimeLimit) {
            //Area day time at end value.
            if(
                ImGui::DragTime2(
                    "End day time", &dayEndMin, "h", "m", 23, 59
                )
            ) {
                registerChange("day time change");
                daySpeed =
                    calculateDaySpeed(
                        dayStartMin, dayEndMin, missionMin
                    );
                game.curAreaData->dayTimeSpeed = daySpeed;
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
                    "Day time speed", &daySpeed, 0.1f, 0.0f, FLT_MAX
                )
            ) {
                registerChange("day time change");
                game.curAreaData->dayTimeSpeed = daySpeed;
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
        unsigned char remThumbOpacity =
            !game.curAreaData->thumbnail ? 50 : 255;
        if(
            ImGui::ImageButton(
                "remThumbButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                COLOR_EMPTY, mapAlpha(remThumbOpacity)
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
            "a file in your disk. When you save the area, the thumbnail\n"
            "gets saved into \"thumbnail.png\" in the area's folder, \n"
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
        unsigned char remBgOpacity =
            game.curAreaData->bgBmpName.empty() ? 50 : 255;
        if(
            ImGui::ImageButton(
                "remBgButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                COLOR_EMPTY, mapAlpha(remBgOpacity)
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
            [this] (const string& bmp) {
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
        ALLEGRO_COLOR bgColor = game.curAreaData->bgColor;
        if(
            ImGui::ColorEdit4(
                "Void color", (float*) &bgColor,
                ImGuiColorEditFlags_NoInputs
            )
        ) {
            registerChange("area background color change");
            game.curAreaData->bgColor = bgColor;
        }
        setTooltip(
            "Set the color of the void. If you have a background image,\n"
            "this will appear below it."
        );
        
        //Background distance value.
        float bgDist = game.curAreaData->bgDist;
        if(ImGui::DragFloat("Distance", &bgDist)) {
            registerChange("area background distance change");
            game.curAreaData->bgDist = bgDist;
        }
        setTooltip(
            "How far away the background texture is. "
            "Affects parallax scrolling.\n"
            "2 is a good value.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Background zoom value.
        float bgBmpZoom = game.curAreaData->bgBmpZoom;
        if(ImGui::DragFloat("Zoom", &bgBmpZoom, 0.01)) {
            registerChange("area background zoom change");
            game.curAreaData->bgBmpZoom = bgBmpZoom;
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
        string makerNotes = game.curAreaData->makerNotes;
        if(ImGui::InputText("Maker notes", &makerNotes)) {
            registerChange("area maker notes change");
            game.curAreaData->makerNotes = makerNotes;
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
                "newCircleButton", editorIcons[EDITOR_ICON_NEW_CIRCLE_SECTOR],
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
        ALLEGRO_BITMAP* selFilterBmp = nullptr;
        string selFilterDescription;
        switch(selectionFilter) {
        case SELECTION_FILTER_VERTEXES: {
            selFilterBmp = editorIcons[EDITOR_ICON_VERTEXES];
            selFilterDescription = "vertexes only";
            break;
        } case SELECTION_FILTER_EDGES: {
            selFilterBmp = editorIcons[EDITOR_ICON_EDGES];
            selFilterDescription = "edges + vertexes";
            break;
        } case SELECTION_FILTER_SECTORS: {
            selFilterBmp = editorIcons[EDITOR_ICON_SECTORS];
            selFilterDescription = "sectors + edges + vertexes";
            break;
        } case N_SELECTION_FILTERS: {
            break;
        }
        }
        
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "selFilterButton", selFilterBmp,
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            selectionFilterCmd(1.0f);
        }
        setTooltip(
            "Current selection filter: " + selFilterDescription + ".\n"
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
                    //selectionHomogenized is true. But the sectors aren't
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
    string folderTooltip =
        getFolderTooltip(manifest.path, game.curAreaData->userDataPath) +
        "\n\n"
        "Folder state: ";
    if(!changesMgr.existsOnDisk()) {
        folderTooltip += "Doesn't exist in your disk yet!";
    } else if(changesMgr.hasUnsavedChanges()) {
        folderTooltip += "You have unsaved changes.";
    } else {
        folderTooltip += "Everything ok.";
    }
    setTooltip(folderTooltip);
    
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
    float oldTimeLimit = game.curAreaData->mission.timeLimit;
    bool dayDurationNeedsUpdate = false;
    
    //Mission essentials node.
    if(saveableTreeNode("gameplay", "Mission essentials")) {
    
        //Preset text.
        ImGui::Text(
            "Preset: %s",
            enumGetName(missionPresetNames, game.curAreaData->mission.preset)
            .c_str()
        );
        
        //Change preset button.
        ImGui::SameLine();
        if(ImGui::Button("Change...")) {
            missionPresetDialogPreset = game.curAreaData->mission.preset;
            openDialog(
                "Change mission preset",
                std::bind(
                    &AreaEditor::processGuiMissionPresetDialog, this
                )
            );
            dialogs.back()->customSize = Point(400, 0);
        }
        setTooltip(
            "Change the mission's preset.\n"
            "By using one of the presets you can skip most of the setup,\n"
            "whereas by picking \"custom\" you can control all the details."
        );
        
        //Time limit values.
        int seconds = (int) game.curAreaData->mission.timeLimit;
        if(ImGui::DragTime2("Time limit", &seconds)) {
            registerChange("mission time limit change");
            game.curAreaData->mission.timeLimit = (size_t) seconds;
            dayDurationNeedsUpdate = true;
        }
        setTooltip(
            "Time limit for the mission. 0 means no time limit.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        ImGui::TreePop();
    }
    
    ImGui::Spacer();
    
    if(game.curAreaData->mission.preset == MISSION_PRESET_CUSTOM) {
        processGuiPanelMissionEv();
        processGuiPanelMissionMobChecklists();
        processGuiPanelMissionScoreCriteria();
        processGuiPanelMissionHudItems();
    }
    
    if(dayDurationNeedsUpdate) {
        if(
            game.curAreaData->mission.timeLimit == 0 &&
            oldTimeLimit > 0
        ) {
            game.curAreaData->dayTimeSpeed = AREA::DEF_DAY_TIME_SPEED;
        } else {
            float oldDayStartMin = game.curAreaData->dayTimeStart;
            oldDayStartMin = wrapFloat(oldDayStartMin, 0, 60 * 24);
            float oldDaySpeed = game.curAreaData->dayTimeSpeed;
            float oldTimeLimitMin = oldTimeLimit / 60.0f;
            size_t newTimeLimitSec = game.curAreaData->mission.timeLimit;
            float oldDayEndMin = oldDayStartMin + oldTimeLimitMin * oldDaySpeed;
            oldDayEndMin = wrapFloat(oldDayEndMin, 0, 60 * 24);
            newTimeLimitSec = std::max(newTimeLimitSec, (size_t) 1);
            float newTimeLimitMin = newTimeLimitSec / 60.0f;
            game.curAreaData->dayTimeSpeed =
                calculateDaySpeed(
                    oldDayStartMin, oldDayEndMin, newTimeLimitMin
                );
        }
    }
}


/**
 * @brief Processes the Dear ImGui mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionOld() {
    float oldMissionSurvivalMin =
        game.curAreaData->missionOld.goalAmount / 60.0f;
    float oldMissionTimeLimitMin =
        game.curAreaData->missionOld.failTimeLimit / 60.0f;
    bool dayDurationNeedsUpdate = false;
    
    //Mission goal node.
    if(saveableTreeNode("gameplay", "Mission goal")) {
    
        //Goal combobox.
        vector<string> goalsList;
        for(size_t g = 0; g < game.missionGoals.size(); g++) {
            goalsList.push_back(game.missionGoals[g]->getName());
        }
        int missionGoal = game.curAreaData->missionOld.goal;
        if(ImGui::Combo("Goal", &missionGoal, goalsList, 15)) {
            registerChange("mission requirements change");
            game.curAreaData->missionOld.goalMobIdxs.clear();
            game.curAreaData->missionOld.goalAmount = 1;
            game.curAreaData->missionOld.goal = (MISSION_GOAL) missionGoal;
            if(
                game.curAreaData->missionOld.goal ==
                MISSION_GOAL_TIMED_SURVIVAL
            ) {
                dayDurationNeedsUpdate = true;
            }
        }
        
        switch(game.curAreaData->missionOld.goal) {
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
            int totalSeconds =
                (int) game.curAreaData->missionOld.goalAmount;
            if(ImGui::DragTime2("Time", &totalSeconds)) {
                registerChange("mission requirements change");
                totalSeconds = std::max(totalSeconds, 1);
                game.curAreaData->missionOld.goalAmount =
                    (size_t) totalSeconds;
                dayDurationNeedsUpdate = true;
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
                (int) game.curAreaData->missionOld.goalAmount;
            ImGui::SetNextItemWidth(80);
            if(ImGui::DragInt("Amount", &amount, 0.1f, 1, INT_MAX)) {
                registerChange("mission requirements change");
                game.curAreaData->missionOld.goalAmount =
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
    
        processGuiPanelMissionFail(&dayDurationNeedsUpdate);
        ImGui::TreePop();
    }
    
    //Mission grading node.
    ImGui::Spacer();
    if(saveableTreeNode("gameplay", "Mission grading")) {
    
        processGuiPanelMissionGrading();
        ImGui::TreePop();
        
    }
    
    if(dayDurationNeedsUpdate) {
        float dayStartMin = game.curAreaData->dayTimeStart;
        dayStartMin = wrapFloat(dayStartMin, 0, 60 * 24);
        float daySpeed = game.curAreaData->dayTimeSpeed;
        float oldMissionMin = 0;
        size_t missionSeconds = 0;
        if(game.curAreaData->missionOld.goal == MISSION_GOAL_TIMED_SURVIVAL) {
            oldMissionMin = oldMissionSurvivalMin;
            missionSeconds = game.curAreaData->missionOld.goalAmount;
            game.curAreaData->missionOld.failTimeLimit = 0.0f;
            disableFlag(
                game.curAreaData->missionOld.failConditions,
                getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
            );
        } else {
            oldMissionMin = oldMissionTimeLimitMin;
            missionSeconds = game.curAreaData->missionOld.failTimeLimit;
        }
        float oldDayEndMin = dayStartMin + oldMissionMin * daySpeed;
        oldDayEndMin = wrapFloat(oldDayEndMin, 0, 60 * 24);
        missionSeconds = std::max(missionSeconds, (size_t) 1);
        float newMissionMin = missionSeconds / 60.0f;
        game.curAreaData->dayTimeSpeed =
            calculateDaySpeed(
                dayStartMin, oldDayEndMin, newMissionMin
            );
    }
    
}


/**
 * @brief Processes the Dear ImGui event part of the
 * mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionEv() {
    //Mission events node.
    if(saveableTreeNode("gameplay", "Mission events")) {
    
        static size_t curEventIdx = 0;
        
        processGuiListNavSetup(
            &curEventIdx, game.curAreaData->mission.events.size(), false
        );
        
        //Navigation count widget.
        processGuiListNavCurWidget(
            curEventIdx, game.curAreaData->mission.events.size(), "Event"
        );
        
        //Navigation add widget.
        if(
            processGuiListNavNewWidget(
                &curEventIdx, game.curAreaData->mission.events.size(),
                "Add a new mission event."
            )
        ) {
            registerChange("mission event creation");
            game.curAreaData->mission.events.insert(
                game.curAreaData->mission.events.begin() +
                curEventIdx,
                MissionEvent()
            );
            setStatus(
                "Created mission event #" + i2s(curEventIdx + 1) + "."
            );
        };
        
        //Navigation delete widget.
        size_t prevCurEventIdx = curEventIdx;
        if(
            processGuiListNavDelWidget(
                &curEventIdx, game.curAreaData->mission.events.size(),
                "Delete the current event.", true
            )
        ) {
            registerChange("mission event deletion");
            game.curAreaData->mission.events.erase(
                game.curAreaData->mission.events.begin() +
                prevCurEventIdx
            );
            setStatus(
                "Deleted mission event #" + i2s(prevCurEventIdx + 1) + "."
            );
        };
        
        //Navigation previous widget.
        processGuiListNavPrevWidget(
            &curEventIdx, game.curAreaData->mission.events.size(),
            "Change to the previous event.", true
        );
        
        //Navigation next widget.
        processGuiListNavNextWidget(
            &curEventIdx, game.curAreaData->mission.events.size(),
            "Change to the next event.", true
        );
        
        //Navigation trigger earlier widget.
        if(
            processGuiListNavMoveLeftWidget(
                &curEventIdx, game.curAreaData->mission.events.size(),
                "Make this event trigger earlier.\n"
                "Events are triggered in the order they're displayed here.",
                true
            )
        ) {
            registerChange("mission event reorder");
            std::swap(
                game.curAreaData->mission.events[curEventIdx],
                game.curAreaData->mission.events[curEventIdx - 1]
            );
            curEventIdx--;
            setStatus("Made the event trigger earlier.");
        }
        
        //Navigation trigger later widget.
        if(
            processGuiListNavMoveRightWidget(
                &curEventIdx, game.curAreaData->mission.events.size(),
                "Make this event trigger later.\n"
                "Events are triggered in the order they're displayed here.",
                true
            )
        ) {
            registerChange("mission event reorder");
            std::swap(
                game.curAreaData->mission.events[curEventIdx],
                game.curAreaData->mission.events[curEventIdx + 1]
            );
            curEventIdx++;
            setStatus("Made the event trigger later.");
        }
        
        if(!game.curAreaData->mission.events.empty()) {
        
            MissionEvent* evPtr =
                &game.curAreaData->mission.events[curEventIdx];
            MissionEvType::EditorInfo evEditorInfo =
                game.missionEvTypes[evPtr->type]->getEditorInfo();
                
            //Event type combobox.
            ImGui::Spacer();
            vector<string> evTypeNames;
            for(size_t e = 0; e < game.missionEvTypes.size(); e++) {
                evTypeNames.push_back(game.missionEvTypes[e]->getName());
            }
            int missionEvType = evPtr->type;
            if(ImGui::Combo("Type", &missionEvType, evTypeNames, 15)) {
                registerChange("mission event type change");
                evPtr->type = (MISSION_EV) missionEvType;
                evEditorInfo =
                    game.missionEvTypes[evPtr->type]->getEditorInfo();
                evPtr->indexParam = 0;
                evPtr->amountParam = 1;
            }
            setTooltip("What thing needs to happen for the event to trigger.");
            
            if(!evEditorInfo.description.empty()) {
            
                //Event description text.
                ImGui::TextWrapped("%s", evEditorInfo.description.c_str());
                
            }
            
            if(!evEditorInfo.indexParamName.empty()) {
            
                //Event index param value.
                int number = (int) evPtr->indexParam;
                number++;
                ImGui::SetNextItemWidth(50);
                if(
                    ImGui::DragInt(
                        (evEditorInfo.indexParamName + "##idxParam").c_str(),
                        &number, 0.1f, 1, INT_MAX
                    )
                ) {
                    registerChange("mission event number change");
                    number--;
                    evPtr->indexParam = (size_t) number;
                }
                setTooltip(
                    evEditorInfo.indexParamDescription,
                    "", WIDGET_EXPLANATION_DRAG
                );
                
            }
            
            if(!evEditorInfo.amountParamName.empty()) {
            
                //Event amount param value.
                int number = (int) evPtr->amountParam;
                ImGui::SetNextItemWidth(50);
                if(
                    ImGui::DragInt(
                        (evEditorInfo.amountParamName + "##amtParam").c_str(),
                        &number, 0.1f, 0, INT_MAX
                    )
                ) {
                    registerChange("mission event number change");
                    evPtr->amountParam = (size_t) number;
                }
                setTooltip(
                    evEditorInfo.amountParamDescription,
                    "", WIDGET_EXPLANATION_DRAG
                );
                
            }
            
            //Action combobox.
            vector<string> actionTypeNames;
            for(size_t a = 0; a < game.missionActionTypes.size(); a++) {
                actionTypeNames.push_back(
                    game.missionActionTypes[a]->getName()
                );
            }
            int missionActionType = evPtr->actionType;
            ImGui::Spacer();
            if(
                ImGui::Combo("Action", &missionActionType, actionTypeNames, 15)
            ) {
                registerChange("mission event action change");
                evPtr->actionType = (MISSION_ACTION) missionActionType;
            }
            setTooltip(
                "What action to perform when the event is triggered."
            );
            
            MissionActionType::EditorInfo actionEditorInfo =
                game.missionActionTypes[evPtr->actionType]->getEditorInfo();
                
            if(!actionEditorInfo.description.empty()) {
            
                //Action description text.
                ImGui::TextWrapped("%s", actionEditorInfo.description.c_str());
                
            }
            
            if(evPtr->actionType == MISSION_ACTION_SEND_MESSAGE) {
            
                //Action message input.
                string message = evPtr->actionMessage;
                if(monoInputText("Message", &message)) {
                    registerChange("mission event action message change");
                    evPtr->actionMessage = message;
                }
                setTooltip(
                    "Specify what message you want to be sent to the script."
                );
                
            } else {
            
                //Zero time for scoring checkbox.
                bool zeroTime = evPtr->zeroTimeForScore;
                if(
                    ImGui::Checkbox(
                        "Zero time for score", &zeroTime
                    )
                ) {
                    registerChange("mission event action time rule change");
                    evPtr->zeroTimeForScore = zeroTime;
                }
                setTooltip(
                    "If true, the time remaining in the time limit will\n"
                    "be considered 0 for the sake of scoring."
                );
                
            }
            
        }
        
        ImGui::TreePop();
    }
    
    ImGui::Spacer();
}


/**
 * @brief Processes the Dear ImGui fail conditions part of the
 * mission control panel for this frame.
 *
 * @param dayDurationNeedsUpdate The variable that dictates whether the
 * day duration widget data later in the panel needs to be updated.
 */
void AreaEditor::processGuiPanelMissionFail(
    bool* dayDurationNeedsUpdate
) {
    unsigned int failFlags =
        (unsigned int) game.curAreaData->missionOld.failConditions;
    bool failFlagsChanged = false;
    
    //Pause menu end checkbox.
    bool pauseMenuEndIsFail =
        game.curAreaData->missionOld.goal != MISSION_GOAL_END_MANUALLY;
    ImGui::BeginDisabled();
    ImGui::CheckboxFlags(
        "End from pause menu",
        &failFlags,
        getIdxBitmask(MISSION_FAIL_COND_PAUSE_MENU)
    );
    ImGui::EndDisabled();
    if(pauseMenuEndIsFail) {
        enableFlag(
            game.curAreaData->missionOld.failConditions,
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
            game.curAreaData->missionOld.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_PAUSE_MENU)
        );
        setTooltip(
            "The current mission goal is \"end whenever you want\", so\n"
            "ending from the pause menu is the goal, not a fail condition."
        );
    }
    
    //Time limit checkbox.
    if(game.curAreaData->missionOld.goal == MISSION_GOAL_TIMED_SURVIVAL) {
        disableFlag(
            failFlags,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
        disableFlag(
            game.curAreaData->missionOld.failConditions,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
        ImGui::BeginDisabled();
    }
    bool timeLimitChanged =
        ImGui::CheckboxFlags(
            "Reach the time limit",
            &failFlags,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        );
    failFlagsChanged |= timeLimitChanged;
    if(
        timeLimitChanged &&
        hasFlag(
            failFlags,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        *dayDurationNeedsUpdate = true;
    }
    if(game.curAreaData->missionOld.goal == MISSION_GOAL_TIMED_SURVIVAL) {
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
            failFlags,
            getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
        )
    ) {
        //Time limit values.
        int seconds =
            (int) game.curAreaData->missionOld.failTimeLimit;
        ImGui::Indent();
        if(ImGui::DragTime2("Time limit", &seconds)) {
            registerChange("mission fail conditions change");
            seconds = std::max(seconds, 1);
            game.curAreaData->missionOld.failTimeLimit = (size_t) seconds;
            *dayDurationNeedsUpdate = true;
        }
        setTooltip(
            "Time limit that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    //Reaching too few Pikmin checkbox.
    failFlagsChanged |=
        ImGui::CheckboxFlags(
            "Reach too few Pikmin",
            &failFlags,
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
            failFlags,
            getIdxBitmask(MISSION_FAIL_COND_TOO_FEW_PIKMIN)
        )
    ) {
        ImGui::Indent();
        
        //Pikmin amount value.
        int amount =
            (int) game.curAreaData->missionOld.failTooFewPikAmount;
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Amount##fctfpa", &amount, 0.1f, 0, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->missionOld.failTooFewPikAmount =
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
    failFlagsChanged |=
        ImGui::CheckboxFlags(
            "Reach too many Pikmin",
            &failFlags,
            getIdxBitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
        );
    setTooltip(
        "The mission ends as a fail if the total Pikmin count reaches\n"
        "a certain amount or higher."
    );
    
    if(
        hasFlag(
            failFlags,
            getIdxBitmask(MISSION_FAIL_COND_TOO_MANY_PIKMIN)
        )
    ) {
        ImGui::Indent();
        
        //Pikmin amount value.
        int amount =
            (int) game.curAreaData->missionOld.failTooManyPikAmount;
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Amount##fctmpa", &amount, 0.1f, 1, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->missionOld.failTooManyPikAmount =
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
    failFlagsChanged |=
        ImGui::CheckboxFlags(
            "Lose Pikmin",
            &failFlags,
            getIdxBitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
        );
    setTooltip(
        "The mission ends as a fail if a certain amount of Pikmin die."
    );
    
    if(
        hasFlag(
            failFlags,
            getIdxBitmask(MISSION_FAIL_COND_LOSE_PIKMIN)
        )
    ) {
        //Pikmin deaths value.
        int amount =
            (int) game.curAreaData->missionOld.failPikKilled;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Deaths", &amount, 0.1f, 1, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->missionOld.failPikKilled =
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
    failFlagsChanged |=
        ImGui::CheckboxFlags(
            "Take damage",
            &failFlags,
            getIdxBitmask(MISSION_FAIL_COND_TAKE_DAMAGE)
        );
    setTooltip(
        "The mission ends as a fail if any leader loses any health."
    );
    
    //Lose leaders checkbox.
    failFlagsChanged |=
        ImGui::CheckboxFlags(
            "Lose leaders",
            &failFlags,
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
            failFlags,
            getIdxBitmask(
                MISSION_FAIL_COND_LOSE_LEADERS
            )
        )
    ) {
        //Leader KOs value.
        int amount =
            (int) game.curAreaData->missionOld.failLeadersKod;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("KOs", &amount, 0.1f, 1, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->missionOld.failLeadersKod =
                (size_t) amount;
        }
        setTooltip(
            "Leader KO amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    //Defeat enemies checkbox.
    failFlagsChanged |=
        ImGui::CheckboxFlags(
            "Defeat enemies",
            &failFlags,
            getIdxBitmask(MISSION_FAIL_COND_DEFEAT_ENEMIES)
        );
    setTooltip(
        "The mission ends as a fail if a certain amount of\n"
        "enemies get defeated."
    );
    
    if(
        hasFlag(
            failFlags,
            getIdxBitmask(MISSION_FAIL_COND_DEFEAT_ENEMIES)
        )
    ) {
        //Enemy defeats value.
        int amount =
            (int) game.curAreaData->missionOld.failEnemiesDefeated;
        ImGui::Indent();
        ImGui::SetNextItemWidth(50);
        if(ImGui::DragInt("Defeats", &amount, 0.1f, 1, INT_MAX)) {
            registerChange("mission fail conditions change");
            game.curAreaData->missionOld.failEnemiesDefeated =
                (size_t) amount;
        }
        setTooltip(
            "Enemy defeat amount that, when reached, ends the mission\n"
            "as a fail.",
            "", WIDGET_EXPLANATION_DRAG
        );
        ImGui::Unindent();
    }
    
    if(failFlagsChanged) {
        registerChange("mission fail conditions change");
        game.curAreaData->missionOld.failConditions =
            (Bitmask8) failFlags;
    }
    
    vector<MISSION_FAIL_COND> activeConditions;
    for(size_t c = 0; c < game.missionFailConds.size(); c++) {
        if(
            hasFlag(
                game.curAreaData->missionOld.failConditions,
                getIdxBitmask(c)
            )
        ) {
            activeConditions.push_back((MISSION_FAIL_COND) c);
        }
    }
    
    if(!activeConditions.empty()) {
    
        //Primary HUD condition checkbox.
        ImGui::Spacer();
        bool showPrimary =
            game.curAreaData->missionOld.failHudPrimaryCond != INVALID;
        if(ImGui::Checkbox("Show primary HUD element", &showPrimary)) {
            registerChange("mission fail conditions change");
            game.curAreaData->missionOld.failHudPrimaryCond =
                showPrimary ?
                (size_t) activeConditions[0] :
                INVALID;
        }
        setTooltip(
            "If checked, a large HUD element will appear showing\n"
            "the most important fail condition's information."
        );
        
        if(showPrimary) {
            //Primary HUD condition combobox.
            int selected = 0;
            bool found = false;
            vector<string> condStrings;
            for(size_t c = 0; c < activeConditions.size(); c++) {
                size_t condId = activeConditions[c];
                condStrings.push_back(
                    game.missionFailConds[condId]->getName()
                );
                if(
                    condId ==
                    game.curAreaData->missionOld.failHudPrimaryCond
                ) {
                    found = true;
                    selected = (int) c;
                }
            }
            if(!found) {
                game.curAreaData->missionOld.failHudSecondaryCond = 0;
            }
            ImGui::Indent();
            if(
                ImGui::Combo(
                    "Primary condition", &selected, condStrings, 15
                )
            ) {
                registerChange("mission fail conditions change");
                game.curAreaData->missionOld.failHudPrimaryCond =
                    activeConditions[selected];
            }
            setTooltip(
                "Failure condition to show in the primary HUD element."
            );
            ImGui::Unindent();
        }
        
        //Secondary HUD condition checkbox.
        bool showSecondary =
            game.curAreaData->missionOld.failHudSecondaryCond != INVALID;
        if(ImGui::Checkbox("Show secondary HUD element", &showSecondary)) {
            registerChange("mission fail conditions change");
            game.curAreaData->missionOld.failHudSecondaryCond =
                showSecondary ?
                (size_t) activeConditions[0] :
                INVALID;
        }
        setTooltip(
            "If checked, a smaller HUD element will appear showing\n"
            "some other fail condition's information."
        );
        
        if(showSecondary) {
            //Secondary HUD condition combobox.
            bool found = false;
            int selected = 0;
            vector<string> condStrings;
            for(size_t c = 0; c < activeConditions.size(); c++) {
                size_t condId = activeConditions[c];
                condStrings.push_back(
                    game.missionFailConds[condId]->getName()
                );
                if(
                    condId ==
                    game.curAreaData->missionOld.failHudSecondaryCond
                ) {
                    found = true;
                    selected = (int) c;
                }
            }
            if(!found) {
                game.curAreaData->missionOld.failHudSecondaryCond = 0;
            }
            ImGui::Indent();
            if(
                ImGui::Combo(
                    "Secondary condition", &selected, condStrings, 15
                )
            ) {
                registerChange("mission fail conditions change");
                game.curAreaData->missionOld.failHudSecondaryCond =
                    activeConditions[selected];
            }
            setTooltip(
                "Failure condition to show in the secondary HUD element."
            );
            ImGui::Unindent();
        }
        
    } else {
        game.curAreaData->missionOld.failHudPrimaryCond = INVALID;
        game.curAreaData->missionOld.failHudSecondaryCond = INVALID;
        
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
    
    int requiresAllOption =
        game.curAreaData->missionOld.goalAllMobs ? 0 : 1;
        
    //All enemies requirement radio button.
    if(ImGui::RadioButton("All", &requiresAllOption, 0)) {
        registerChange("mission requirements change");
        game.curAreaData->missionOld.goalAllMobs =
            requiresAllOption == 0;
    }
    setTooltip(
        "Require the player to defeat all enemies "
        "in order to reach the goal."
    );
    
    //Specific enemies requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requiresAllOption, 1)
    ) {
        registerChange("mission requirements change");
        game.curAreaData->missionOld.goalAllMobs =
            requiresAllOption == 0;
    }
    setTooltip(
        "Require the player to defeat specific enemies "
        "in order to reach the goal.\n"
        "You must specify which enemies these are."
    );
    
    if(!game.curAreaData->missionOld.goalAllMobs) {
    
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
    size_t totalRequired = getMissionRequiredMobCount();
    ImGui::Text("Total objects required: %lu", totalRequired);
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
    
    int requiresAllOption =
        game.curAreaData->missionOld.goalAllMobs ? 0 : 1;
        
    //All treasures requirement radio button.
    if(ImGui::RadioButton("All", &requiresAllOption, 0)) {
        registerChange("mission requirements change");
        game.curAreaData->missionOld.goalAllMobs =
            requiresAllOption == 0;
    }
    setTooltip(
        "Require the player to collect all treasures "
        "in order to reach the goal."
    );
    
    //Specific treasures requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requiresAllOption, 1)
    ) {
        registerChange("mission requirements change");
        game.curAreaData->missionOld.goalAllMobs =
            requiresAllOption == 0;
    }
    setTooltip(
        "Require the player to collect specific treasures "
        "in order to reach the goal.\n"
        "You must specify which treasures these are."
    );
    
    if(!game.curAreaData->missionOld.goalAllMobs) {
    
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
    size_t totalRequired = getMissionRequiredMobCount();
    ImGui::Text("Total objects required: %lu", totalRequired);
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
        f2s(game.curAreaData->missionOld.goalExitCenter.x).c_str(),
        f2s(game.curAreaData->missionOld.goalExitCenter.y).c_str()
    );
    
    //Region center text.
    ImGui::Text(
        "Exit region size: %s x %s",
        f2s(game.curAreaData->missionOld.goalExitSize.x).c_str(),
        f2s(game.curAreaData->missionOld.goalExitSize.y).c_str()
    );
    
    //Leader requirements text.
    ImGui::Spacer();
    ImGui::Text("Leader requirements:");
    
    int requiresAllOption =
        game.curAreaData->missionOld.goalAllMobs ? 0 : 1;
        
    //All leaders requirement radio button.
    if(ImGui::RadioButton("All", &requiresAllOption, 0)) {
        registerChange("mission requirements change");
        game.curAreaData->missionOld.goalAllMobs =
            requiresAllOption == 0;
    }
    setTooltip(
        "Require the player to bring all leaders to the exit\n"
        "region in order to reach the mission's goal."
    );
    
    //Specific leaders requirement radio button.
    ImGui::SameLine();
    if(
        ImGui::RadioButton("Specific ones", &requiresAllOption, 1)
    ) {
        registerChange("mission requirements change");
        game.curAreaData->missionOld.goalAllMobs =
            requiresAllOption == 0;
    }
    setTooltip(
        "Require the player to bring specific leaders to the exit\n"
        "region in order to reach the mission's goal.\n"
        "You must specify which leaders these are."
    );
    
    if(!game.curAreaData->missionOld.goalAllMobs) {
    
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
    size_t totalRequired = getMissionRequiredMobCount();
    ImGui::Text("Total objects required: %lu", totalRequired);
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
        game.curAreaData->missionOld.gradingMode == MISSION_GRADING_MODE_POINTS
    ) {
    
        ImGui::Spacer();
        processGuiGradingCriterionWidgets(
            &game.curAreaData->missionOld.pointsPerPikminBorn,
            MISSION_SCORE_CRITERIA_PIKMIN_BORN,
            "Points per Pikmin born",
            "Amount of points that the player receives for each\n"
            "Pikmin born."
        );
        
        processGuiGradingCriterionWidgets(
            &game.curAreaData->missionOld.pointsPerPikminDeath,
            MISSION_SCORE_CRITERIA_PIKMIN_DEATH,
            "Points per Pikmin death",
            "Amount of points that the player receives for each\n"
            "Pikmin lost."
        );
        
        if(
            hasFlag(
                game.curAreaData->missionOld.failConditions,
                getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
            )
        ) {
            processGuiGradingCriterionWidgets(
                &game.curAreaData->missionOld.pointsPerSecLeft,
                MISSION_SCORE_CRITERIA_SEC_LEFT,
                "Points per second left",
                "Amount of points that the player receives for each\n"
                "second of time left, from the mission's time limit."
            );
        }
        
        processGuiGradingCriterionWidgets(
            &game.curAreaData->missionOld.pointsPerSecPassed,
            MISSION_SCORE_CRITERIA_SEC_PASSED,
            "Points per second passed",
            "Amount of points that the player receives for each\n"
            "second of time that has passed."
        );
        
        processGuiGradingCriterionWidgets(
            &game.curAreaData->missionOld.pointsPerTreasurePoint,
            MISSION_SCORE_CRITERIA_TREASURE_POINTS,
            "Points per treasure point",
            "Amount of points that the player receives for each\n"
            "point gathered from treasures. Different treasures are worth\n"
            "different treasure points."
        );
        
        processGuiGradingCriterionWidgets(
            &game.curAreaData->missionOld.pointsPerEnemyPoint,
            MISSION_SCORE_CRITERIA_ENEMY_POINTS,
            "Points per enemy point",
            "Amount of points that the player receives for each\n"
            "enemy point. Different enemies are worth different\n"
            "points."
        );
        
        //Award points on collection checkbox.
        if(game.curAreaData->missionOld.pointsPerEnemyPoint != 0) {
            bool enemyPointsOnCollection =
                game.curAreaData->missionOld.enemyPointsOnCollection;
            ImGui::Indent();
            if(
                ImGui::Checkbox(
                    "Award points on collection", &enemyPointsOnCollection
                )
            ) {
                registerChange("mission grading change");
                game.curAreaData->missionOld.enemyPointsOnCollection =
                    enemyPointsOnCollection;
            }
            setTooltip(
                "If checked, enemy points will be awarded on enemy\n"
                "collection. If unchecked, enemy points will be awarded\n"
                "on enemy defeat."
            );
            ImGui::Unindent();
        }
        
        //Starting score value.
        ImGui::Spacer();
        int startingPoints = game.curAreaData->missionOld.startingPoints;
        ImGui::SetNextItemWidth(60);
        if(ImGui::DragInt("Starting points", &startingPoints, 1.0f)) {
            registerChange("mission grading change");
            game.curAreaData->missionOld.startingPoints = startingPoints;
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
            &game.curAreaData->missionOld.bronzeReq, "Bronze",
            INT_MIN, game.curAreaData->missionOld.silverReq - 1,
            "To get a bronze medal, the player needs at least these\n"
            "many points. Fewer than this, and the player gets no medal."
        );
        
        processGuiGradingMedalWidgets(
            &game.curAreaData->missionOld.silverReq, "Silver",
            game.curAreaData->missionOld.bronzeReq + 1,
            game.curAreaData->missionOld.goldReq - 1,
            "To get a silver medal, the player needs at least these\n"
            "many points."
        );
        
        processGuiGradingMedalWidgets(
            &game.curAreaData->missionOld.goldReq, "Gold",
            game.curAreaData->missionOld.silverReq + 1,
            game.curAreaData->missionOld.platinumReq - 1,
            "To get a gold medal, the player needs at least these\n"
            "many points."
        );
        
        processGuiGradingMedalWidgets(
            &game.curAreaData->missionOld.platinumReq, "Platinum",
            game.curAreaData->missionOld.goldReq + 1, INT_MAX,
            "To get a platinum medal, the player needs at least these\n"
            "many points."
        );
        
        //Maker record value.
        ImGui::Spacer();
        int makerRecord = game.curAreaData->missionOld.makerRecord;
        ImGui::SetNextItemWidth(60);
        if(ImGui::DragInt("Maker's record", &makerRecord, 1.0f)) {
            registerChange("maker record change");
            game.curAreaData->missionOld.makerRecord = makerRecord;
        }
        setTooltip(
            "Specify your best score here, if you want.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Maker record date input.
        string makerRecordDate =
            game.curAreaData->missionOld.makerRecordDate;
        ImGui::SetNextItemWidth(120);
        if(
            monoInputText(
                "Date (YYYY/MM/DD)", &makerRecordDate
            )
        ) {
            registerChange("maker record change");
            game.curAreaData->missionOld.makerRecordDate = makerRecordDate;
        }
        setTooltip(
            "Specify the date in which you got your best score here,\n"
            "if you want. Your record will only be saved if you write a date.\n"
            "The format must be YYYY/MM/DD."
        );
    }
}


/**
 * @brief Processes the Dear ImGui HUD items part of the
 * mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionHudItems() {
    //Mission HUD items node.
    if(saveableTreeNode("gameplay", "Mission HUD items")) {
    
        static size_t curHudItemIdx = 0;
        
        //Setup.
        processGuiListNavSetup(
            &curHudItemIdx, enumGetCount(missionHudItemIdNames), false
        );
        
        //Current item text.
        processGuiListNavCurWidget(
            curHudItemIdx, enumGetCount(missionHudItemIdNames), "Item",
            enumGetName(missionHudItemIdNames, curHudItemIdx) + " (typically)"
        );
        
        //Previous item button.
        processGuiListNavPrevWidget(
            &curHudItemIdx, enumGetCount(missionHudItemIdNames),
            "Select the previous HUD item."
        );
        
        //Next item button.
        processGuiListNavNextWidget(
            &curHudItemIdx, enumGetCount(missionHudItemIdNames),
            "Select the next HUD item.", true
        );
        
        MissionHudItem* itemPtr =
            &game.curAreaData->mission.hudItems[curHudItemIdx];
            
        //Enabled checkbox.
        bool enabled = itemPtr->enabled;
        ImGui::Spacer();
        if(
            ImGui::Checkbox("Enabled", &enabled)
        ) {
            registerChange("mission HUD item toggle");
            itemPtr->enabled = enabled;
        }
        setTooltip(
            "Whether this HUD item is enabled and visible in this mission."
        );
        
        if(itemPtr->enabled) {
        
            //Content type combobox.
            int contentType = itemPtr->contentType;
            if(
                ImGui::Combo(
                    "Content type", &contentType,
                    enumGetNames(missionHudItemContentTypeNames), 15
                )
            ) {
                registerChange("mission HUD item content type change");
                itemPtr->contentType = (MISSION_HUD_ITEM_CONTENT) contentType;
            }
            setTooltip(
                "What sort of content will be shown inside."
            );
            
            switch(itemPtr->contentType) {
            case MISSION_HUD_ITEM_CONTENT_TEXT: {
        
                //Text input.
                string text = itemPtr->text;
                if(ImGui::InputText("Text", &text)) {
                    registerChange("mission HUD item text change");
                    itemPtr->text = text;
                }
                setTooltip(
                    "The HUD item won't have anything other than this text."
                );
                
                break;
                
            } case MISSION_HUD_ITEM_CONTENT_CLOCK_DOWN: {
        
                break;
                
            } case MISSION_HUD_ITEM_CONTENT_CLOCK_UP: {
        
                break;
                
            } case MISSION_HUD_ITEM_CONTENT_SCORE: {
        
                break;
                
            } case MISSION_HUD_ITEM_CONTENT_CUR_TOT:
            case MISSION_HUD_ITEM_CONTENT_REM_TOT:
            case MISSION_HUD_ITEM_CONTENT_CUR_AMT:
            case MISSION_HUD_ITEM_CONTENT_REM_AMT:
            case MISSION_HUD_ITEM_CONTENT_TOT_AMT: {
        
                const auto processIdxsListWidgets =
                    [this] (
                        vector<size_t>* idxs,
                        const string& label, const string& descriptor
                ) {
                
                    if(idxs->empty()) idxs->push_back(0);
                    
                    for(size_t i = 0; i < idxs->size(); i++) {
                    
                        //Add button.
                        if(
                            ImGui::ImageButton(
                                "addIdxButton",
                                editorIcons[EDITOR_ICON_ADD],
                                Point(EDITOR::ICON_BMP_SIZE)
                            )
                        ) {
                            registerChange(
                                "mission HUD item " + descriptor +
                                " addition"
                            );
                            idxs->insert(idxs->begin() + i, 0);
                        }
                        
                        //Remove button.
                        ImGui::SameLine();
                        if(idxs->size() != 1) {
                        
                            if(
                                ImGui::ImageButton(
                                    "remIdxButton",
                                    editorIcons[EDITOR_ICON_REMOVE],
                                    Point(EDITOR::ICON_BMP_SIZE)
                                )
                            ) {
                                registerChange(
                                    "mission HUD item " + descriptor +
                                    " removal"
                                );
                                idxs->erase(idxs->begin() + i);
                            }
                            
                        } else {
                        
                            ImGui::Dummy(
                                ImVec2(
                                    EDITOR::ICON_BMP_SIZE,
                                    EDITOR::ICON_BMP_SIZE
                                )
                            );
                            
                        }
                        
                        //Number input.
                        int idx = idxs->operator[](i);
                        idx++;
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(50);
                        if(
                            ImGui::DragInt(
                                (label + "##idx" + i2s(i)).c_str(),
                                &idx, 0.1f, 1, INT_MAX
                            )
                        ) {
                            registerChange(
                                "mission HUD item " + descriptor + " change"
                            );
                            idx--;
                            idxs->operator[](i) = (size_t) idx;
                        }
                        setTooltip(
                            "Number of the " + descriptor + " to get the\n"
                            "amounts from. If you specify multiple ones,\n"
                            "it combines all of them."
                        );
                        
                    }
                    
                };
                
                //Label input.
                string text = itemPtr->text;
                if(ImGui::InputText("Label", &text)) {
                    registerChange("mission HUD item text change");
                    itemPtr->text = text;
                }
                setTooltip(
                    "Text to accompany the amounts, if any."
                );
                
                //Amount type combobox.
                int amountType = itemPtr->amountType;
                if(
                    ImGui::Combo(
                        "Amount type", &amountType,
                        enumGetNames(missionHudItemAmountTypeNames), 15
                    )
                ) {
                    registerChange("mission HUD item amount type change");
                    itemPtr->amountType = (MISSION_HUD_ITEM_AMT) amountType;
                }
                setTooltip(
                    "What type of information the amount "
                    "should be calculated from."
                );
                
                if(
                    itemPtr->amountType ==
                    MISSION_HUD_ITEM_AMT_MOB_CHECKLIST
                ) {
                
                    //Mob checklist number widgets.
                    processIdxsListWidgets(
                        &itemPtr->idxsList,
                        "Mob checklist number", "mob checklist"
                    );
                    
                    break;
                    
                }
                
                if(
                    itemPtr->amountType ==
                    MISSION_HUD_ITEM_AMT_LEADERS_IN_REGION
                ) {
                
                    //Region number widgets.
                    processIdxsListWidgets(
                        &itemPtr->idxsList,
                        "Region number", "region"
                    );
                    
                    break;
                    
                }
                
                if(
                    itemPtr->contentType !=
                    MISSION_HUD_ITEM_CONTENT_CUR_AMT &&
                    (
                        itemPtr->amountType ==
                        MISSION_HUD_ITEM_AMT_LEADERS_IN_REGION ||
                        itemPtr->amountType ==
                        MISSION_HUD_ITEM_AMT_PIKMIN ||
                        itemPtr->amountType ==
                        MISSION_HUD_ITEM_AMT_LEADERS ||
                        itemPtr->amountType ==
                        MISSION_HUD_ITEM_AMT_PIKMIN_DEATHS ||
                        itemPtr->amountType ==
                        MISSION_HUD_ITEM_AMT_LEADER_KOS
                    )
                ) {
                
                    //Total amount value.
                    int total = itemPtr->totalAmount;
                    if(
                        ImGui::DragInt(
                            "Total", &total, 0.1f, 1, INT_MAX
                        )
                    ) {
                        registerChange("mission HUD item amount change");
                        itemPtr->totalAmount = total;
                    }
                    setTooltip(
                        "Amount to use as the total."
                    );
                    
                    break;
                    
                }
                
                break;
                
            }
            }
            
        }
        
        ImGui::TreePop();
        
    }
    
    ImGui::Spacer();
}


/**
 * @brief Processes the Dear ImGui mob checklists part of the
 * mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionMobChecklists() {
    //Mission mob checklists node.
    if(saveableTreeNode("gameplay", "Mission mob checklists")) {
    
        //Setup.
        processGuiListNavSetup(
            &curMobChecklistIdx, game.curAreaData->mission.mobChecklists.size(),
            false
        );
        
        //Current checklist text.
        processGuiListNavCurWidget(
            curMobChecklistIdx, game.curAreaData->mission.mobChecklists.size(),
            "Checklist"
        );
        
        //Create checklist button.
        size_t prevCurMobChecklistIdx = curMobChecklistIdx;
        if(
            processGuiListNavNewWidget(
                &curMobChecklistIdx,
                game.curAreaData->mission.mobChecklists.size(),
                "Add a new mission mob checklist."
            )
        ) {
            registerChange("mission mob checklist creation");
            game.curAreaData->mission.mobChecklists.insert(
                game.curAreaData->mission.mobChecklists.begin() +
                prevCurMobChecklistIdx,
                MissionMobChecklist()
            );
            for(
                size_t e = 0; e < game.curAreaData->mission.events.size(); e++
            ) {
                MissionEvent* ePtr = &game.curAreaData->mission.events[e];
                if(ePtr->type != MISSION_EV_MOB_CHECKLIST) continue;
                if(ePtr->indexParam == 0) continue;
                adjustMisalignedIndex(
                    ePtr->indexParam, prevCurMobChecklistIdx, true
                );
            }
            setStatus(
                "Created mission mob checklist #" +
                i2s(curMobChecklistIdx + 1) + "."
            );
        }
        
        //Delete checklist button.
        prevCurMobChecklistIdx = curMobChecklistIdx;
        if(
            processGuiListNavDelWidget(
                &curMobChecklistIdx,
                game.curAreaData->mission.mobChecklists.size(),
                "Delete the current mission mob checklist.", true
            )
        ) {
            registerChange("mission mob checklist deletion");
            game.curAreaData->mission.mobChecklists.erase(
                game.curAreaData->mission.mobChecklists.begin() +
                prevCurMobChecklistIdx
            );
            for(
                size_t e = 0; e < game.curAreaData->mission.events.size(); e++
            ) {
                MissionEvent* ePtr = &game.curAreaData->mission.events[e];
                if(ePtr->type != MISSION_EV_MOB_CHECKLIST) continue;
                if(ePtr->indexParam == 0) continue;
                adjustMisalignedIndex(
                    ePtr->indexParam, prevCurMobChecklistIdx, false
                );
            }
            setStatus(
                "Deleted mission event #" +
                i2s(prevCurMobChecklistIdx + 1) + "."
            );
        }
        
        //Previous checklist button.
        processGuiListNavPrevWidget(
            &curMobChecklistIdx,
            game.curAreaData->mission.mobChecklists.size(),
            "Change to the previous mission mob checklist.", true
        );
        
        //Next checklist button.
        processGuiListNavNextWidget(
            &curMobChecklistIdx,
            game.curAreaData->mission.mobChecklists.size(),
            "Change to the next mission mob checklist.", true
        );
        
        if(!game.curAreaData->mission.mobChecklists.empty()) {
        
            MissionMobChecklist* checklistPtr =
                &game.curAreaData->mission.mobChecklists[curMobChecklistIdx];
                
            //Checklist type combobox.
            ImGui::Spacer();
            int checklistType = checklistPtr->type;
            if(
                ImGui::Combo(
                    "Type", &checklistType,
                    enumGetNames(missionMobChecklistTypeNames), 15
                )
            ) {
                registerChange("mission mob checklist type change");
                checklistPtr->type = (MISSION_MOB_CHECKLIST) checklistType;
            }
            setTooltip(
                "The checklist type controls how the objects that are\n"
                "a part of it are determined."
            );
            
            //All checkbox.
            bool amountIsAll = checklistPtr->requiredAmount == 0;
            if(
                ImGui::Checkbox("All matching mobs", &amountIsAll)
            ) {
                registerChange("mission mob checklist amount change");
                if(amountIsAll) {
                    checklistPtr->requiredAmount = 0;
                } else {
                    checklistPtr->requiredAmount = 1;
                }
            }
            setTooltip(
                "If checked, then the checklist is cleared when all of the\n"
                "matching objects in the area are cleared. Otherwise,\n"
                "the checklist is cleared when any X of the matching\n"
                "objects are cleared."
            );
            
            //Amount value.
            if(!amountIsAll) {
                int amount = (int) checklistPtr->requiredAmount;
                ImGui::Indent();
                ImGui::SetNextItemWidth(50);
                if(
                    ImGui::DragInt("Amount", &amount, 0.1f, 1, INT_MAX)
                ) {
                    registerChange("mission mob checklist amount change");
                    checklistPtr->requiredAmount = (size_t) amount;
                }
                setTooltip(
                    "How many matching objects within the checklist need to\n"
                    "be cleared in order for the checklist to be cleared."
                    , "", WIDGET_EXPLANATION_DRAG
                );
                ImGui::Unindent();
            }
            
            if(
                checklistPtr->type == MISSION_MOB_CHECKLIST_CUSTOM ||
                checklistPtr->type == MISSION_MOB_CHECKLIST_ENEMIES ||
                checklistPtr->type == MISSION_MOB_CHECKLIST_TREASURES_ENEMIES
            ) {
            
                //Enemies need collection checkbox.
                bool enemiesNeedCollection =
                    checklistPtr->enemiesNeedCollection;
                if(
                    ImGui::Checkbox(
                        "Enemies need collection", &enemiesNeedCollection
                    )
                ) {
                    registerChange("mission mob checklist requirement change");
                    checklistPtr->enemiesNeedCollection = enemiesNeedCollection;
                }
                setTooltip(
                    "If true, enemies need to be defeated and\n"
                    "collected in order to be checked.\n"
                    "If false, they only need to be defeated."
                );
                
            }
            
            if(checklistPtr->type == MISSION_MOB_CHECKLIST_CUSTOM) {
            
                //Choose mobs button.
                if(ImGui::Button("Pick objects...")) {
                    changeState(EDITOR_STATE_MOBS);
                    subState = EDITOR_SUB_STATE_MISSION_MOBS;
                }
                setTooltip(
                    "Click here to start picking which objects do and\n"
                    "do not belong to the checklist."
                );
                
                //Mob amount text.
                ImGui::SameLine();
                ImGui::Text(
                    "(%u chosen)", (unsigned int) checklistPtr->mobIdxs.size()
                );
                
            }
            
        }
        
        ImGui::TreePop();
    }
    
    ImGui::Spacer();
}


/**
 * @brief Processes the Dear ImGui score criteria part of the
 * mission control panel for this frame.
 */
void AreaEditor::processGuiPanelMissionScoreCriteria() {
    //Mission score criteria node.
    if(saveableTreeNode("gameplay", "Mission scoring")) {
    
        //Setup.
        static size_t curCriterionIdx = 0;
        processGuiListNavSetup(
            &curCriterionIdx, game.curAreaData->mission.scoreCriteria.size(),
            false
        );
        
        //Current criterion text.
        processGuiListNavCurWidget(
            curCriterionIdx, game.curAreaData->mission.scoreCriteria.size(),
            "Criterion"
        );
        
        //Add criterion button.
        size_t prevCurCriterionIdx = curCriterionIdx;
        if(
            processGuiListNavNewWidget(
                &curCriterionIdx,
                game.curAreaData->mission.scoreCriteria.size(),
                "Add a new mission score criterion."
            )
        ) {
            registerChange("mission score criterion creation");
            game.curAreaData->mission.scoreCriteria.insert(
                game.curAreaData->mission.scoreCriteria.begin() +
                prevCurCriterionIdx,
                MissionScoreCriterion()
            );
            setStatus(
                "Created mission score criterion #" +
                i2s(curCriterionIdx + 1) + "."
            );
        }
        
        //Delete criterion button.
        prevCurCriterionIdx = curCriterionIdx;
        if(
            processGuiListNavDelWidget(
                &curCriterionIdx,
                game.curAreaData->mission.scoreCriteria.size(),
                "Delete the current mission score criterion.", true
            )
        ) {
            registerChange("mission score criterion deletion");
            game.curAreaData->mission.scoreCriteria.erase(
                game.curAreaData->mission.scoreCriteria.begin() +
                prevCurCriterionIdx
            );
            setStatus(
                "Deleted mission score criterion #" +
                i2s(prevCurCriterionIdx + 1) + "."
            );
        }
        
        //Previous criterion button.
        processGuiListNavPrevWidget(
            &curCriterionIdx, game.curAreaData->mission.scoreCriteria.size(),
            "Select the previous mission score criterion.", true
        );
        
        //Next criterion button.
        processGuiListNavNextWidget(
            &curCriterionIdx, game.curAreaData->mission.scoreCriteria.size(),
            "Select the next mission score criterion.", true
        );
        
        if(!game.curAreaData->mission.scoreCriteria.empty()) {
        
            MissionScoreCriterion* criterionPtr =
                &game.curAreaData->mission.scoreCriteria[curCriterionIdx];
                
            //Criterion type combobox.
            ImGui::Spacer();
            int criterionType = criterionPtr->type;
            if(
                ImGui::Combo(
                    "Type", &criterionType,
                    enumGetNames(missionScoreCriterionTypeNames), 15
                )
            ) {
                registerChange("mission score criterion type change");
                criterionPtr->type = (MISSION_SCORE_CRITERION) criterionType;
            }
            setTooltip(
                "What aspect of gameplay gets judged for this criterion."
            );
            
            //Point multiplier value.
            int points = criterionPtr->points;
            if(
                ImGui::DragInt(
                    "Points", &points, 0.1f, 1, INT_MAX
                )
            ) {
                registerChange("mission score criterion point change");
                criterionPtr->points = points;
            }
            setTooltip(
                "The player receives these many points per criterion item.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Applies to HUD checkbox.
            bool hud = criterionPtr->affectsHud;
            if(
                ImGui::Checkbox(
                    "Applies to HUD", &hud
                )
            ) {
                registerChange("mission score criterion option change");
                criterionPtr->affectsHud = hud;
            }
            setTooltip(
                "If unchecked, this criterion will only affect the score\n"
                "received at the end of the mission.\n"
                "If checked, it will also affect the score items in the HUD\n"
                "in real time.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            if(criterionPtr->type == MISSION_SCORE_CRITERION_MOB_CHECKLIST) {
            
                //Mob checklist number value.
                int number = (int) criterionPtr->indexParam;
                ImGui::SetNextItemWidth(50);
                if(
                    ImGui::DragInt(
                        "Mob checklist number",
                        &number, 0.1f, 0, INT_MAX
                    )
                ) {
                    registerChange("mission score criterion checklist change");
                    criterionPtr->indexParam = (size_t) number;
                }
                setTooltip(
                    "Number of the mob checklist to check the mobs of.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
            }
        }
        
        ImGui::TreePop();
        
    }
    
    ImGui::Spacer();
}


/**
 * @brief Processes the Dear ImGui mob control panel for this frame.
 */
void AreaEditor::processGuiPanelMob() {

    MobGen* mPtr = *selectedMobs.begin();
    
    //Category and type comboboxes.
    string customCatName = "";
    if(mPtr->type) customCatName = mPtr->type->customCategoryName;
    MobType* type = mPtr->type;
    
    if(processGuiMobTypeWidgets(&customCatName, &type)) {
        registerChange("object type change");
        mPtr->type = type;
        lastMobCustomCatName = "";
        if(mPtr->type) {
            lastMobCustomCatName = mPtr->type->customCategoryName;
        }
        lastMobType = mPtr->type;
    }
    
    if(mPtr->type) {
        //Tips text.
        ImGui::TextDisabled("(%s info & tips)", mPtr->type->name.c_str());
        string fullStr =
            "Internal object category: " + mPtr->type->category->name + "\n" +
            wordWrap(mPtr->type->description, 50);
        if(!mPtr->type->areaEditorTips.empty()) {
            fullStr +=
                "\n\n" +
                wordWrap(mPtr->type->areaEditorTips, 50);
        }
        setTooltip(fullStr);
        
        if(mPtr->type->areaEditorRecommendLinksFrom) {
            if(mPtr->links.empty()) {
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
        
        if(mPtr->type->areaEditorRecommendLinksTo) {
            bool hasLinksTo = false;
            for(
                size_t m = 0;
                m < game.curAreaData->mobGenerators.size();
                m++
            ) {
                MobGen* otherMPtr = game.curAreaData->mobGenerators[m];
                for(size_t l = 0; l < otherMPtr->links.size(); l++) {
                    if(otherMPtr->links[l] == mPtr) {
                        hasLinksTo = true;
                        break;
                    }
                }
                if(hasLinksTo) break;
            }
            if(!hasLinksTo) {
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
    float mobAngle = normalizeAngle(mPtr->angle);
    ImGui::Spacer();
    if(ImGui::SliderAngleWithContext("Angle", &mobAngle, 0, 360, "%.2f")) {
        registerChange("object angle change");
        mPtr->angle = mobAngle;
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
    
        processGuiMobScriptVars(mPtr);
        
        ImGui::TreePop();
        
    }
    
    //Object advanced node.
    ImGui::Spacer();
    if(saveableTreeNode("mobs", "Advanced")) {
    
        if(mPtr->type && mPtr->type->category->id == MOB_CATEGORY_ENEMIES) {
            bool isBoss = mPtr->isBoss;
            if(ImGui::Checkbox("Boss", &isBoss)) {
                registerChange("Enemy boss setting");
                mPtr->isBoss = isBoss;
            }
            setTooltip(
                "If this enemy should be considered a boss.\n"
                "Boss enemies will trigger boss music when nearby."
            );
        }
        
        if(mPtr->storedInside == INVALID) {
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
                registerChange("Object in object storing");
                mPtr->storedInside = INVALID;
            }
            setTooltip(
                "This object is currently stored inside another. Click here\n"
                "to unstore it and make it a regular object instead."
            );
        }
        
        //Object link amount text.
        ImGui::Spacer();
        ImGui::Text(
            "%i link%s", (int) mPtr->links.size(),
            mPtr->links.size() == 1 ? "" : "s"
        );
        
        //Object new link button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "newLinkButton", editorIcons[EDITOR_ICON_ADD],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            if(subState == EDITOR_SUB_STATE_NEW_MOB_LINK) {
                subState = EDITOR_SUB_STATE_NONE;
            } else {
                subState = EDITOR_SUB_STATE_NEW_MOB_LINK;
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
                    mPtr->links.erase(mPtr->links.begin());
                    mPtr->linkIdxs.erase(mPtr->linkIdxs.begin());
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
        
    } else if(subState == EDITOR_SUB_STATE_NEW_MOB_LINK) {
    
        //Link creation explanation text.
        ImGui::TextWrapped(
            "Use the canvas to link to an object. Click on the object you "
            "want this one to link to."
        );
        
        //Link creation cancel button.
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
    
        //Instructions text.
        ImGui::TextWrapped(
            "Click an object to mark or unmark it as part of the checklist. "
            "Objects flashing yellow are a part of the checklist. "
            "Click the finish button when you are done."
        );
        
        //Total objects chosen text.
        ImGui::Text(
            "Total objects chosen: %lu",
            game.curAreaData->mission.mobChecklists[
                curMobChecklistIdx
            ].mobIdxs.size()
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
            addNewMobCmd(1.0f);
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
                    "delMobButton", editorIcons[EDITOR_ICON_REMOVE],
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
    PathLink* lPtr = *selectedPathLinks.begin();
    
    //Type combobox.
    vector<string> linkTypeNames;
    linkTypeNames.push_back("Normal");
    linkTypeNames.push_back("Ledge");
    
    int typeI = lPtr->type;
    if(ImGui::Combo("Type", &typeI, linkTypeNames, 15)) {
        registerChange("path link type change");
        lPtr->type = (PATH_LINK_TYPE) typeI;
    }
    setTooltip(
        "What type of link this is."
    );
    
    homogenizeSelectedPathLinks();
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
        
        int oneWayMode = pathDrawingNormals;
        
        //One-way links radio button.
        ImGui::RadioButton("Draw one-way links", &oneWayMode, 0);
        setTooltip(
            "When drawing, new links drawn will be one-way links.",
            "1"
        );
        
        //Normal links radio button.
        ImGui::RadioButton("Draw normal links", &oneWayMode, 1);
        setTooltip(
            "When drawing, new links drawn will be normal (two-way) links.",
            "2"
        );
        
        pathDrawingNormals = oneWayMode;
        
        //Type combobox.
        vector<string> linkTypeNames;
        linkTypeNames.push_back("Normal");
        linkTypeNames.push_back("Ledge");
        
        int typeI = pathDrawingType;
        if(ImGui::Combo("Type", &typeI, linkTypeNames, 15)) {
            pathDrawingType = (PATH_LINK_TYPE) typeI;
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
        int flagsI = pathDrawingFlags;
        if(
            ImGui::CheckboxFlags(
                "Script use only",
                &flagsI,
                PATH_STOP_FLAG_SCRIPT_ONLY
            )
        ) {
            pathDrawingFlags = flagsI;
        }
        setTooltip(
            "Can only be used by objects if their script tells them to."
        );
        
        //Light load only checkbox.
        if(
            ImGui::CheckboxFlags(
                "Light load only",
                &flagsI,
                PATH_STOP_FLAG_LIGHT_LOAD_ONLY
            )
        ) {
            pathDrawingFlags = flagsI;
        }
        setTooltip(
            "Can only be used by objects that are not carrying anything, "
            "or by objects that only have a weight of 1."
        );
        
        //Airborne only checkbox.
        if(
            ImGui::CheckboxFlags(
                "Airborne only",
                &flagsI,
                PATH_STOP_FLAG_AIRBORNE_ONLY
            )
        ) {
            pathDrawingFlags = flagsI;
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
            addNewPathCmd(1.0f);
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
        
            bool okToEdit =
                (selectedPathStops.size() == 1) || selectionHomogenized;
                
            if(selectedPathStops.empty()) {
            
                //"No stop selected" text.
                ImGui::TextDisabled("(No path stop selected)");
                
            } else if(okToEdit) {
            
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
        
            bool okToEdit =
                (selectedPathLinks.size() == 1) || selectionHomogenized;
            if(!okToEdit && selectedPathLinks.size() == 2) {
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
                    okToEdit = true;
                }
            }
            
            if(selectedPathLinks.empty()) {
            
                //"No link selected" text.
                ImGui::TextDisabled("(No path link selected)");
                
            } else if(okToEdit) {
            
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
                        game.editorsView.cam.pos.x - AREA_EDITOR::COMFY_DIST;
                    pathPreviewCheckpoints[0].y =
                        game.editorsView.cam.pos.y;
                    pathPreviewCheckpoints[1].x =
                        game.editorsView.cam.pos.x + AREA_EDITOR::COMFY_DIST;
                    pathPreviewCheckpoints[1].y =
                        game.editorsView.cam.pos.y;
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
            
                unsigned int flagsI = pathPreviewSettings.flags;
                
                //Is from script checkbox.
                if(
                    ImGui::CheckboxFlags(
                        "Is from script",
                        &flagsI,
                        PATH_FOLLOW_FLAG_SCRIPT_USE
                    )
                ) {
                    pathPreviewSettings.flags = flagsI;
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
                        &flagsI,
                        PATH_FOLLOW_FLAG_LIGHT_LOAD
                    )
                ) {
                    pathPreviewSettings.flags = flagsI;
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
                        &flagsI,
                        PATH_FOLLOW_FLAG_AIRBORNE
                    )
                ) {
                    pathPreviewSettings.flags = flagsI;
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
                float totalDist = 0.0f;
                size_t totalNrStops = 0;
                bool success = false;
                
                if(pathPreviewResult > 0) {
                    totalDist = pathPreviewDist;
                    totalNrStops = pathPreview.size();
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
                        "Total travel distance: %f", totalDist
                    );
                } else {
                    ImGui::Text(" ");
                }
                
                //Path total stops visited text.
                if(success) {
                    ImGui::BulletText(
                        "Total stops visited: %lu", totalNrStops
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
            static string labelName;
            if(
                processGuiInputPopup("selectStops", "Label:", &labelName, true)
            ) {
                selectPathStopsWithLabel(labelName);
            }
            
            ImGui::TreePop();
            
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui path stop control panel for this frame.
 */
void AreaEditor::processGuiPanelPathStop() {
    PathStop* sPtr = *selectedPathStops.begin();
    
    //Radius value.
    float radius = sPtr->radius;
    if(ImGui::DragFloat("Radius", &radius, 0.5f, PATHS::MIN_STOP_RADIUS)) {
        radius = std::max(PATHS::MIN_STOP_RADIUS, radius);
        registerChange("path stop radius change");
        sPtr->radius = radius;
        pathPreviewTimer.start(false);
    }
    setTooltip(
        "Radius of the stop. Used when mobs want to find the closest\n"
        "start/end stop.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Script use only checkbox.
    int flagsI = sPtr->flags;
    if(
        ImGui::CheckboxFlags(
            "Script use only",
            &flagsI,
            PATH_STOP_FLAG_SCRIPT_ONLY
        )
    ) {
        registerChange("path stop property change");
        sPtr->flags = flagsI;
    }
    setTooltip(
        "Can only be used by objects if their script tells them to."
    );
    
    //Light load only checkbox.
    if(
        ImGui::CheckboxFlags(
            "Light load only",
            &flagsI,
            PATH_STOP_FLAG_LIGHT_LOAD_ONLY
        )
    ) {
        registerChange("path stop property change");
        sPtr->flags = flagsI;
    }
    setTooltip(
        "Can only be used by objects that are not carrying anything, "
        "or by objects that only have a weight of 1."
    );
    
    //Airborne only checkbox.
    if(
        ImGui::CheckboxFlags(
            "Airborne only",
            &flagsI,
            PATH_STOP_FLAG_AIRBORNE_ONLY
        )
    ) {
        registerChange("path stop property change");
        sPtr->flags = flagsI;
    }
    setTooltip(
        "Can only be used by objects that can fly."
    );
    
    //Label text.
    monoInputText("Label", &sPtr->label);
    setTooltip(
        "If this stop is part of a path that you want\n"
        "to address in a script, write the name here."
    );
    
    homogenizeSelectedPathStops();
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
            "area editor's components in the way.",
            "Shift + P"
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
                crossSectionWindowStart = game.editorsView.getTopLeft();
                crossSectionWindowEnd = game.editorsView.size / 2.0f;
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
                    game.editorsView.cam.pos.x - AREA_EDITOR::COMFY_DIST;
                crossSectionCheckpoints[0].y =
                    game.editorsView.cam.pos.y;
                crossSectionCheckpoints[1].x =
                    game.editorsView.cam.pos.x + AREA_EDITOR::COMFY_DIST;
                crossSectionCheckpoints[1].y =
                    game.editorsView.cam.pos.y;
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
    Sector* sPtr = *selectedSectors.begin();
    
    //Sector behavior node.
    if(saveableTreeNode("layout", "Behavior")) {
    
        //Sector height value.
        float sectorZ = sPtr->z;
        if(ImGui::DragFloat("Height", &sectorZ)) {
            registerChange("sector height change");
            sPtr->z = sectorZ;
            updateAllEdgeOffsetCaches();
        }
        if(ImGui::BeginPopupContextItem()) {
            //-50 height selectable.
            if(ImGui::Selectable("-50")) {
                registerChange("sector height change");
                sPtr->z -= 50.0f;
                updateAllEdgeOffsetCaches();
                ImGui::CloseCurrentPopup();
            }
            
            //+50 height selectable.
            if(ImGui::Selectable("+50")) {
                registerChange("sector height change");
                sPtr->z += 50.0f;
                updateAllEdgeOffsetCaches();
                ImGui::CloseCurrentPopup();
            }
            
            //Set to zero selectable.
            if(ImGui::Selectable("Set to 0")) {
                registerChange("sector height change");
                sPtr->z = 0.0f;
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
        
            string hazardIname;
            if(sPtr->hazard) {
                hazardIname = sPtr->hazard->manifest->internalName;
            }
            if(processGuiHazardManagementWidgets(hazardIname)) {
                registerChange("sector hazard changes");
                sPtr->hazard =
                    hazardIname.empty() ?
                    nullptr :
                    &game.content.hazards.list[hazardIname];
            }
            setTooltip("This sector's hazard, if any.");
            
            if(!hazardIname.empty()) {
                //Sector hazard floor only checkbox.
                bool sectorHazardFloor = sPtr->hazardFloor;
                ImGui::Indent();
                if(ImGui::Checkbox("Floor only", &sectorHazardFloor)) {
                    registerChange("sector hazard floor option change");
                    sPtr->hazardFloor = sectorHazardFloor;
                }
                ImGui::Unindent();
                setTooltip(
                    "Do the hazards only affects objects on the floor,\n"
                    "or do they affect airborne objects in the sector too?"
                );
            }
            
            //Sector bottomless pit checkbox.
            bool sectorBottomlessPit = sPtr->isBottomlessPit;
            if(ImGui::Checkbox("Bottomless pit", &sectorBottomlessPit)) {
                registerChange("sector bottomless pit change");
                sPtr->isBottomlessPit = sectorBottomlessPit;
                if(!sectorBottomlessPit) {
                    updateSectorTexture(sPtr, sPtr->textureInfo.bmpName);
                }
            }
            setTooltip(
                "Is this sector's floor a bottomless pit?\n"
                "Pikmin die when they fall in pits,\n"
                "and you can see the background (or void)."
            );
            
            if(
                sPtr->hazard && sPtr->hazard->associatedLiquid &&
                sPtr->hazard->associatedLiquid->canFreeze
            ) {
            
                //Freezing point override.
                int freezingPointVar = 0;
                map<string, string> sectorVars = getVarMap(sPtr->vars);
                if(!sPtr->vars.empty()) {
                    auto var =
                        sectorVars.find(LIQUID::FREEZING_POINT_SECTOR_VAR);
                    if(var != sectorVars.end()) {
                        freezingPointVar = s2i(var->second);
                    }
                }
                ImGui::SetNextItemWidth(50);
                if(ImGui::DragInt("Freezing point", &freezingPointVar, 0.1f)) {
                    registerChange("sector vars change");
                    if(freezingPointVar <= 0) {
                        sectorVars.erase(LIQUID::FREEZING_POINT_SECTOR_VAR);
                    } else {
                        sectorVars[LIQUID::FREEZING_POINT_SECTOR_VAR] =
                            i2s(freezingPointVar);
                    }
                    sPtr->vars = saveVarMap(sectorVars);
                }
                setTooltip(
                    "Normally, a liquid's freezing point is determined\n"
                    "automatically from its surface area. The closest\n"
                    "multiple of 5 is used so the freezing point is a\n"
                    "nice round number. You can override it with a manual\n"
                    "value here. Use 0 to not override.",
                    "", WIDGET_EXPLANATION_DRAG
                );
            }
            
            ImGui::TreePop();
        }
        
        //Sector advanced behavior node.
        ImGui::Spacer();
        if(saveableTreeNode("layout", "Advanced")) {
        
            //Sector type combobox.
            vector<string> typesList;
            for(size_t t = 0; t < enumGetCount(sectorTypeINames); t++) {
                typesList.push_back(
                    strToSentence(
                        enumGetName(sectorTypeINames, (SECTOR_TYPE) t)
                    )
                );
            }
            int sectorType = sPtr->type;
            if(ImGui::Combo("Type", &sectorType, typesList, 15)) {
                registerChange("sector type change");
                sPtr->type = (SECTOR_TYPE) sectorType;
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
    
        int textureType = !sPtr->fade;
        
        //Sector texture fader radio button.
        ImGui::RadioButton("Texture fader", &textureType, 0);
        setTooltip(
            "Makes the surrounding textures fade into each other."
        );
        
        //Sector regular texture radio button.
        ImGui::RadioButton("Regular texture", &textureType, 1);
        setTooltip(
            "Makes the sector use a regular texture."
        );
        
        if(sPtr->fade != (textureType == 0)) {
            registerChange("sector texture type change");
            sPtr->fade = textureType == 0;
            if(!sPtr->fade) {
                updateSectorTexture(sPtr, sPtr->textureInfo.bmpName);
            }
        }
        
        if(!sPtr->fade) {
        
            ImGui::Indent();
            
            //Sector texture button.
            if(ImGui::Button("Choose image...")) {
                vector<PickerItem> pickerButtons;
                
                pickerButtons.push_back(PickerItem("Choose another..."));
                
                for(size_t s = 0; s < textureSuggestions.size(); s++) {
                    pickerButtons.push_back(
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
                    pickerButtons,
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
            monoText("%s", sPtr->textureInfo.bmpName.c_str());
            setTooltip("Internal name:\n" + sPtr->textureInfo.bmpName);
            
            ImGui::Unindent();
            
        }
        
        //Sector texture effects node.
        ImGui::Spacer();
        if(saveableTreeNode("layout", "Texture effects")) {
        
            //Sector texture offset value.
            Point textureTranslation = sPtr->textureInfo.tf.trans;
            if(ImGui::DragFloat2("Offset", (float*) &textureTranslation)) {
                registerChange("sector texture offset change");
                sPtr->textureInfo.tf.trans = textureTranslation;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Offset the texture horizontally or vertically "
                "by this much.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Sector texture scale value.
            Point textureScale = sPtr->textureInfo.tf.scale;
            if(ImGui::DragFloat2("Scale", (float*) &textureScale, 0.01)) {
                registerChange("sector texture scale change");
                sPtr->textureInfo.tf.scale = textureScale;
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
            float textureRotation = normalizeAngle(sPtr->textureInfo.tf.rot);
            if(
                ImGui::SliderAngleWithContext(
                    "Angle", &textureRotation, 0, 360, "%.2f"
                )
            ) {
                registerChange("sector texture angle change");
                sPtr->textureInfo.tf.rot = textureRotation;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Rotate the texture by these many degrees.\n"
                "The rotation's center point is at the origin "
                "of the area, at coordinates 0,0.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            //Sector texture tint value.
            ALLEGRO_COLOR textureTint = sPtr->textureInfo.tint;
            if(
                ImGui::ColorEdit4(
                    "Tint color", (float*) &textureTint,
                    ImGuiColorEditFlags_NoInputs
                )
            ) {
                registerChange("sector texture tint change");
                sPtr->textureInfo.tint = textureTint;
                quickPreviewTimer.start();
            }
            setTooltip(
                "Tint the texture with this color. White means no tint."
            );
            
            //On-canvas texture effect editing checkbox.
            bool octeeOn =
                subState == EDITOR_SUB_STATE_OCTEE;
            if(ImGui::Checkbox("On-canvas editing", &octeeOn)) {
                subState =
                    octeeOn ?
                    EDITOR_SUB_STATE_OCTEE :
                    EDITOR_SUB_STATE_NONE;
            }
            setTooltip(
                "Enable on-canvas texture effect editing.\n"
                "With this, you can click and drag on the canvas "
                "to adjust the texture,\n"
                "based on whatever mode is currently active."
            );
            
            if(octeeOn) {
            
                ImGui::Indent();
                
                int octeeModeInt = (int) octeeMode;
                
                //On-canvas texture effect editing offset radio button.
                ImGui::RadioButton(
                    "Change offset", &octeeModeInt,
                    (int) OCTEE_MODE_OFFSET
                );
                setTooltip(
                    "Dragging will change the texture's offset.",
                    "1"
                );
                
                //On-canvas texture effect editing scale radio button.
                ImGui::RadioButton(
                    "Change scale", &octeeModeInt,
                    (int) OCTEE_MODE_SCALE
                );
                setTooltip(
                    "Dragging will change the texture's scale.",
                    "2"
                );
                
                //On-canvas texture effect editing angle radio button.
                ImGui::RadioButton(
                    "Change angle", &octeeModeInt,
                    (int) OCTEE_MODE_ANGLE
                );
                setTooltip(
                    "Dragging will change the texture's angle.",
                    "3"
                );
                
                octeeMode = (OCTEE_MODE) octeeModeInt;
                
                ImGui::Unindent();
                
            }
            
            ImGui::TreePop();
        }
        
        //Sector mood node.
        ImGui::Spacer();
        if(saveableTreeNode("layout", "Sector mood")) {
        
            //Sector brightness value.
            int sectorBrightness = sPtr->brightness;
            ImGui::SetNextItemWidth(180);
            if(ImGui::SliderInt("Brightness", &sectorBrightness, 0, 255)) {
                registerChange("sector brightness change");
                sPtr->brightness = sectorBrightness;
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
        unsigned char remRefOpacity = referenceFilePath.empty() ? 50 : 255;
        if(
            ImGui::ImageButton(
                "remRefButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                COLOR_EMPTY, mapAlpha(remRefOpacity)
            )
        ) {
            referenceFilePath.clear();
            updateReference();
        }
        setTooltip(
            "Remove the reference image.\n"
            "This does not delete the file in your disk."
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
            "Browse for a file in your disk to use."
        );
        
        //Reference image name text.
        string refFileName =
            getPathLastComponent(referenceFilePath);
        ImGui::SameLine();
        monoText("%s", refFileName.c_str());
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
                bool backupExists = false;
                if(!manifest.internalName.empty()) {
                    string filePath =
                        game.curAreaData->userDataPath + "/" +
                        FILE_NAMES::AREA_GEOMETRY;
                    if(al_filename_exists(filePath.c_str())) {
                        backupExists = true;
                    }
                }
                
                if(backupExists) {
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
        static float resizeMults[2] = { 1.0f, 1.0f };
        ImGui::SetNextItemWidth(128.0f);
        ImGui::DragFloat2("##resizeMult", resizeMults, 0.01);
        setTooltip(
            "Resize multipliers, vertically and horizontally.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Resize everything button.
        ImGui::SameLine();
        if(ImGui::Button("Resize everything")) {
            if(resizeMults[0] == 0.0f || resizeMults[1] == 0.0f) {
                setStatus(
                    "Can't resize everything to size 0!",
                    true
                );
            } else if(resizeMults[0] == 1.0f && resizeMults[1] == 1.0f) {
                setStatus(
                    "Resizing everything by 1 wouldn't make a difference!",
                    true
                );
            } else {
                registerChange("global resize");
                resizeEverything(resizeMults);
                setStatus(
                    "Resized everything by " + f2s(resizeMults[0]) + ", " +
                    f2s(resizeMults[1]) + "."
                );
                resizeMults[0] = 1.0f;
                resizeMults[1] = 1.0f;
            }
        }
        setTooltip(
            "Resize everything in the area by the specified multiplier.\n"
            "0.5 will resize everything to half size, 2.0 to double, etc."
        );
        
        ImGui::Spacer();
        
        //Enable edge sector patching checkbox.
        ImGui::Checkbox(
            "Enable edge sector patching",
            &enableEdgeSectorPatching
        );
        setTooltip(
            "If checked, the edge tab of the layout panel will contain\n"
            "widgets that let you correct what the sector on each side is.\n"
            "Sectors are defined by their edges, so it's important that the\n"
            "edges store the correct sector numbers. Use this feature in case\n"
            "a sector becomes broken and you can't fix it otherwise.\n"
            "See also: the Editor > Debug > Show sector indexes menu option."
        );
        
        
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
            resizeString(
                f2s(game.editorsView.mouseCursorWorldPos.x), 7
            ).c_str(),
            resizeString(
                f2s(game.editorsView.mouseCursorWorldPos.y), 7
            ).c_str()
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
        "Save the area to your disk.",
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
    unsigned char undoOpacity = undoHistory.empty() ? 50 : 255;
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            "undoButton", editorIcons[EDITOR_ICON_UNDO],
            Point(EDITOR::ICON_BMP_SIZE),
            Point(0.0f), Point(1.0f),
            COLOR_EMPTY, mapAlpha(undoOpacity)
        )
    ) {
        undoCmd(1.0f);
    }
    string undoText;
    if(undoHistory.empty()) {
        undoText = "Nothing to undo.";
    } else {
        undoText = "Undo: " + undoHistory.front().second + ".";
    }
    setTooltip(
        undoText,
        "Ctrl + Z"
    );
    
    //Redo button.
    unsigned redoOpacity = redoHistory.empty() ? 50 : 255;
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "redoButton", editorIcons[EDITOR_ICON_UNDO],
            Point(EDITOR::ICON_BMP_SIZE),
            Point(1.0f, 0.0f), Point(0.0f, 1.0f),
            COLOR_EMPTY, mapAlpha(redoOpacity)
        )
    ) {
        redoCmd(1.0f);
    }
    string redoText;
    if(redoHistory.empty()) {
        redoText =
            "Nothing to redo.";
    } else {
        redoText =
            "Redo: " + redoHistory.front().second + ".";
    }
    setTooltip(
        redoText,
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
        int referenceAlphaInt = referenceAlpha;
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(0.0f, 0.0f));
        ImGui::SetNextItemWidth(48.0f);
        ImGui::SliderInt("##refAlpha", &referenceAlphaInt, 0, 255, "");
        setTooltip(
            "Opacity of the reference image.",
            "", WIDGET_EXPLANATION_SLIDER
        );
        ImGui::EndGroup();
        referenceAlpha = referenceAlphaInt;
        
    }
    
    //Snap mode button.
    ALLEGRO_BITMAP* snapModeBmp = nullptr;
    string snapModeDescription;
    switch(game.options.areaEd.snapMode) {
    case SNAP_MODE_GRID: {
        snapModeBmp = editorIcons[EDITOR_ICON_SNAP_GRID];
        snapModeDescription = "grid. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_VERTEXES: {
        snapModeBmp = editorIcons[EDITOR_ICON_SNAP_VERTEXES];
        snapModeDescription = "vertexes. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_EDGES: {
        snapModeBmp = editorIcons[EDITOR_ICON_SNAP_EDGES];
        snapModeDescription = "edges. Holding Shift disables snapping.";
        break;
    } case SNAP_MODE_NOTHING: {
        snapModeBmp = editorIcons[EDITOR_ICON_SNAP_NOTHING];
        snapModeDescription = "off. Holding Shift snaps to grid.";
        break;
    } case N_SNAP_MODES: {
        break;
    }
    }
    
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "snapButton", snapModeBmp,
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        snapModeCmd(1.0f);
    }
    setTooltip(
        "Current snap mode: " + snapModeDescription,
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
