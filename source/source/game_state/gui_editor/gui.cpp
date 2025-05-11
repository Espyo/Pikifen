/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * GUI editor Dear ImGui logic.
 */

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/imgui_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Opens the "load" dialog.
 */
void GuiEditor::openLoadDialog() {
    reloadGuiDefs();
    
    //Set up the picker's behavior and data.
    vector<PickerItem> fileItems;
    for(const auto &f : game.content.guiDefs.manifests) {
        fileItems.push_back(
            PickerItem(
                f.first,
                "Pack: " + game.content.packs.list[f.second.pack].name, "",
                (void*) &f.second,
                getFileTooltip(f.second.path)
            )
        );
    }
    
    loadDialogPicker = Picker(this);
    loadDialogPicker.items = fileItems;
    loadDialogPicker.pickCallback =
        std::bind(
            &GuiEditor::pickGuiDefFile, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );
        
    //Open the dialog that will contain the picker and history.
    openDialog(
        "Load a GUI definition file",
        std::bind(&GuiEditor::processGuiLoadDialog, this)
    );
    dialogs.back()->closeCallback =
        std::bind(&GuiEditor::closeLoadDialog, this);
}


/**
 * @brief Opens the "new" dialog.
 */
void GuiEditor::openNewDialog() {
    newDialog.mustUpdate = true;
    openDialog(
        "Create a new GUI definition",
        std::bind(&GuiEditor::processGuiNewDialog, this)
    );
    dialogs.back()->customSize = Point(400, 0);
    dialogs.back()->closeCallback = [this] () {
        newDialog.pack.clear();
        newDialog.internalName.clear();
        newDialog.problem.clear();
        newDialog.defPath.clear();
        newDialog.mustUpdate = true;
    };
}


/**
 * @brief Opens the options dialog.
 */
void GuiEditor::openOptionsDialog() {
    openDialog(
        "Options",
        std::bind(&GuiEditor::processGuiOptionsDialog, this)
    );
    dialogs.back()->closeCallback =
        std::bind(&GuiEditor::closeOptionsDialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void GuiEditor::processGui() {
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.winW, game.winH));
    ImGui::Begin(
        "GUI editor", nullptr,
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
    ImGui::GetWindowDrawList()->AddCallback(drawCanvasDearImGuiCallback, nullptr);
    
    //Small hack. Recenter the camera, if necessary.
    if(mustRecenterCam) {
        resetCam(true);
        mustRecenterCam = false;
    }
    
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
void GuiEditor::processGuiControlPanel() {
    if(manifest.internalName.empty()) return;
    
    ImGui::BeginChild("panel");
    
    //Current file header text.
    ImGui::Text("File: ");
    
    //Current file text.
    ImGui::SameLine();
    monoText("%s", manifest.internalName.c_str());
    string fileTooltip =
        getFileTooltip(manifest.path) + "\n\n"
        "File state: ";
    if(!changesMgr.existsOnDisk()) {
        fileTooltip += "Not saved to disk yet!";
    } else if(changesMgr.hasUnsavedChanges()) {
        fileTooltip += "You have unsaved changes.";
    } else {
        fileTooltip += "Everything ok.";
    }
    setTooltip(fileTooltip);
    
    ImGui::Spacer();
    
    //Process the list of items.
    processGuiPanelItems();
    
    ImGui::Spacer();
    
    //Process the currently selected item.
    processGuiPanelItem();
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui GUI definition deletion dialog
 * for this frame.
 */
void GuiEditor::processGuiDeleteGuiDefDialog() {
    //Explanation text.
    string explanationStr;
    if(!changesMgr.existsOnDisk()) {
        explanationStr =
            "You have never saved this GUI definition to disk, so if you\n"
            "delete, you will only lose your unsaved progress.";
    } else {
        explanationStr =
            "If you delete, you will lose all unsaved progress, and the\n"
            "GUI definition's files on the disk will be gone FOREVER!";
    }
    ImGui::SetupCentering(ImGui::CalcTextSize(explanationStr.c_str()).x);
    ImGui::Text("%s", explanationStr.c_str());
    
    //Final warning text.
    string finalWarningStr =
        "Are you sure you want to delete the current GUI definition?";
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
        deleteCurrentGuiDef();
    }
    ImGui::PopStyleColor(3);
}


/**
 * @brief Processes the "load" dialog for this frame.
 */
void GuiEditor::processGuiLoadDialog() {
    //History node.
    processGuiHistory(
        game.options.guiEd.history,
    [this](const string &path) -> string {
        return path;
    },
    [this](const string &path) {
        closeTopDialog();
        loadGuiDefFile(path, true);
    },
    [this](const string &path) {
        return getFileTooltip(path);
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
    setTooltip(
        "Creates a new GUI definition.\n"
        "This works by copying an existing one to a new pack."
    );
    
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
void GuiEditor::processGuiMenuBar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load file item.
            if(ImGui::MenuItem("Load or create...", "Ctrl+L")) {
                loadWidgetPos = getLastWidgetPost();
                loadCmd(1.0f);
            }
            setTooltip(
                "Pick a GUI definition file to load.",
                "Ctrl + L"
            );
            
            //Reload current file item.
            if(ImGui::MenuItem("Reload current GUI definition")) {
                reloadWidgetPos = getLastWidgetPost();
                reloadCmd(1.0f);
            }
            setTooltip(
                "Lose all changes and reload the current file from the disk."
            );
            
            //Save file item.
            if(ImGui::MenuItem("Save current GUI definition", "Ctrl+S")) {
                saveCmd(1.0f);
            }
            setTooltip(
                "Save the GUI definition into the file on disk.",
                "Ctrl + S"
            );
            
            //Delete current GUI definition item.
            if(ImGui::MenuItem("Delete current GUI definition")) {
                deleteGuiDefCmd(1.0f);
            }
            setTooltip(
                "Delete the current GUI definition from the disk."
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
            
            //Quit editor item.
            if(ImGui::MenuItem("Quit", "Ctrl+Q")) {
                quitWidgetPos = getLastWidgetPost();
                quitCmd(1.0f);
            }
            setTooltip(
                "Quit the GUI editor.",
                "Ctrl + Q"
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
            if(ImGui::MenuItem("Reset", "0")) {
                zoomAndPosResetCmd(1.0f);
            }
            setTooltip(
                "Reset the zoom level and camera position.",
                "0"
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
                    "This editor allows you to change where each item "
                    "in a graphical user interface is, and how big it is. "
                    "It works both for the gameplay HUD and any menu's items. "
                    "In the canvas you can find the \"game window\", but in "
                    "reality, it's just some square. This is because the "
                    "coordinates you work in go from 0% to 100%, instead of "
                    "using a real window size, since the player can choose "
                    "whatever window size they want. In addition, for the sake "
                    "of simplicity, the editor won't show what each GUI item "
                    "looks like. So you will have to use your imagination to "
                    "visualize how everything will really look in-game."
                    "\n\n"
                    "If you need more help on how to use the GUI editor, "
                    "check out the tutorial in the manual, located "
                    "in the engine's folder.";
                openHelpDialog(helpStr, "gui.html");
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
 * @brief Processes the Dear ImGui "new" dialog for this frame.
 */
void GuiEditor::processGuiNewDialog() {
    //Pack widgets.
    newDialog.mustUpdate |=
        processGuiNewDialogPackWidgets(&newDialog.pack);
        
    //GUI definition combo.
    vector<string> guiFiles;
    for(const auto &g : game.content.guiDefs.manifests) {
        guiFiles.push_back(g.first);
    }
    ImGui::Spacer();
    newDialog.mustUpdate |=
        monoCombo("File", &newDialog.internalName, guiFiles);
        
    //Check if everything's ok.
    if(newDialog.mustUpdate) {
        newDialog.problem.clear();
        if(newDialog.internalName.empty()) {
            newDialog.problem =
                "You have to select a file!";
        } else if(!isInternalNameGood(newDialog.internalName)) {
            newDialog.problem =
                "The internal name should only have lowercase letters,\n"
                "numbers, and underscores!";
        } else if(newDialog.pack == FOLDER_NAMES::BASE_PACK) {
            newDialog.problem =
                "All the GUI definition files already live in the\n"
                "base pack! The idea is you pick one of those so it'll\n"
                "be copied onto a different pack for you to edit.";
        } else {
            ContentManifest tempMan;
            tempMan.internalName = newDialog.internalName;
            tempMan.pack = newDialog.pack;
            newDialog.defPath =
                game.content.guiDefs.manifestToPath(tempMan);
            if(fileExists(newDialog.defPath)) {
                newDialog.problem =
                    "There is already a GUI definition\n"
                    "file for that GUI in that pack!";
            }
        }
        newDialog.mustUpdate = false;
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::SetupCentering(180);
    if(!newDialog.problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create GUI definition", ImVec2(180, 40))) {
        auto reallyCreate = [this] () {
            createGuiDef(string(newDialog.internalName), newDialog.pack);
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
    if(!newDialog.problem.empty()) {
        ImGui::EndDisabled();
    }
    setTooltip(
        newDialog.problem.empty() ?
        "Create the GUI definition!" :
        newDialog.problem
    );
}


/**
 * @brief Processes the options dialog for this frame.
 */
void GuiEditor::processGuiOptionsDialog() {
    //Controls node.
    if(saveableTreeNode("options", "Controls")) {
    
        //Middle mouse button pans checkbox.
        ImGui::Checkbox("Use MMB to pan", &game.options.editors.mmbPan);
        setTooltip(
            "Use the middle mouse button to pan the camera\n"
            "(and RMB to reset camera/zoom).\n"
            "Default: " +
            b2s(OPTIONS::EDITORS_D::MMB_PAN) + "."
        );
        
        //Grid interval text.
        ImGui::Text(
            "Grid interval: %f", game.options.guiEd.gridInterval
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
            "Default: " + i2s(OPTIONS::GUI_ED_D::GRID_INTERVAL) +
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
            "Default: " + i2s(OPTIONS::GUI_ED_D::GRID_INTERVAL) +
            ".",
            "Shift + Minus"
        );
        
        ImGui::TreePop();
        
    }
    
    ImGui::Spacer();
    
    processGuiEditorStyle();
}


/**
 * @brief Processes the GUI item info panel for this frame.
 */
void GuiEditor::processGuiPanelItem() {
    if(curItem == INVALID) return;
    
    Item* curItemPtr = &items[curItem];
    
    if(curItemPtr->size.x == 0.0f) return;
    
    //Item's name text.
    ImGui::Text("Item \"%s\" data:", curItemPtr->name.c_str());
    
    //Center values.
    if(
        ImGui::DragFloat2("Center", (float*) &curItemPtr->center, 0.10f)
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Center coordinates of the item. e.g. 32,100 is 32% of the\n"
        "width horizontally and very bottom vertically.",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    //Size values.
    if(
        processGuiSizeWidgets(
            "Size", curItemPtr->size, 0.10f, false, false, 0.10f
        )
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Width and height of the item. e.g. 40,90 is 40% of the window width,\n"
        "and 90% of the window height.",
        "",
        WIDGET_EXPLANATION_DRAG
    );
    
    Point topLeft(
        curItemPtr->center.x - curItemPtr->size.x / 2.0f,
        curItemPtr->center.y - curItemPtr->size.y / 2.0f
    );
    Point bottomRight(
        curItemPtr->center.x + curItemPtr->size.x / 2.0f,
        curItemPtr->center.y + curItemPtr->size.y / 2.0f
    );
    bool updateFromCorners = false;
    
    //Top-left coordinates values.
    ImGui::Spacer();
    if(ImGui::DragFloat2("Top-left", (float*) &topLeft, 0.10f)) {
        updateFromCorners = true;
    }
    
    //Bottom-right coordinates values.
    if(ImGui::DragFloat2("Bottom-right", (float*) &bottomRight, 0.10f)) {
        updateFromCorners = true;
    }
    
    if(updateFromCorners) {
        Point newSize(
            bottomRight.x - topLeft.x,
            bottomRight.y - topLeft.y
        );
        if(newSize.x > 0.0f && newSize.y > 0.0f) {
            Point newCenter(
                (topLeft.x + bottomRight.x) / 2.0f,
                (topLeft.y + bottomRight.y) / 2.0f
            );
            curItemPtr->center = newCenter;
            curItemPtr->size = newSize;
        }
        changesMgr.markAsChanged();
    }
}


/**
 * @brief Processes the GUI item list panel for this frame.
 */
void GuiEditor::processGuiPanelItems() {
    //Items text.
    ImGui::Text("Items:");
    
    //Item list.
    if(
        ImGui::BeginChild(
            "itemsList", ImVec2(0.0f, 300.0f), ImGuiChildFlags_Borders
        )
    ) {
        for(size_t i = 0; i < items.size(); i++) {
        
            //Item checkbox.
            bool visible = items[i].size.x != 0.0f;
            if(
                ImGui::Checkbox(("##v" + items[i].name).c_str(), &visible)
            ) {
                if(visible) {
                    items[i].center.x = 50.0f;
                    items[i].center.y = 50.0f;
                    items[i].size.x = 10.0f;
                    items[i].size.y = 10.0f;
                } else {
                    items[i].center.x = 0.0f;
                    items[i].center.y = 0.0f;
                    items[i].size.x = 0.0f;
                    items[i].size.y = 0.0f;
                }
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Whether this item is visible in-game or not."
            );
            
            //Separator text.
            ImGui::SameLine();
            ImGui::Text("  ");
            
            //Item selectable.
            bool selected = curItem == i;
            ImGui::SameLine();
            if(
                monoSelectable(items[i].name.c_str(), &selected)
            ) {
                curItem = i;
            }
            
            if(mustFocusOnCurItem && selected) {
                ImGui::SetScrollHereY(0.5f);
                mustFocusOnCurItem = false;
            }
            
        }
        ImGui::EndChild();
    }
}


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void GuiEditor::processGuiStatusBar() {
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
            boxString(f2s(game.editorsView.cursorWorldPos.x), 7, "%").c_str(),
            boxString(f2s(game.editorsView.cursorWorldPos.y), 7, "%").c_str()
        );
    }
}


/**
 * @brief Processes the Dear ImGui toolbar for this frame.
 */
void GuiEditor::processGuiToolbar() {
    if(manifest.internalName.empty()) return;
    
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
        "Quit the GUI editor.",
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
        "Pick a GUI definition file to load.",
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
        "Save the GUI definition into the file on disk.",
        "Ctrl + S"
    );
    
    //Snap mode button.
    ALLEGRO_BITMAP* snapModeBmp = nullptr;
    string snapModeDescription;
    if(game.options.guiEd.snap) {
        snapModeBmp = editorIcons[EDITOR_ICON_SNAP_GRID];
        snapModeDescription = "grid. Holding Shift disables snapping.";
    } else {
        snapModeBmp = editorIcons[EDITOR_ICON_SNAP_NOTHING];
        snapModeDescription = "nothing. Holding Shift snaps to grid.";
    }
    
    ImGui::SameLine(0, 16);
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
        "X"
    );
}
