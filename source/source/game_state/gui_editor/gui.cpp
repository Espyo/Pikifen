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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../lib/imgui/imgui_stdlib.h"
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
    for(const auto& f : game.content.guiDefs.manifests) {
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
        "Load a GUI definition",
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
    ImGui::GetWindowDrawList()->AddCallback(
        drawCanvasDearImGuiCallback, nullptr
    );
    
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
    ImGui::BeginChild("panel");
    
    //Basically, just show the correct panel for the current state.
    switch(state) {
    case EDITOR_STATE_MAIN: {
        processGuiPanelMain();
        break;
    } case EDITOR_STATE_HARDCODED: {
        processGuiPanelHardcoded();
        break;
    } case EDITOR_STATE_CUSTOM: {
        processGuiPanelCustom();
        break;
    } case EDITOR_STATE_INFO: {
        processGuiPanelInfo();
        break;
    }
    }
    
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
            "You have never saved this GUI definition to your disk, so if you\n"
            "delete, you will only lose your unsaved progress.";
    } else {
        explanationStr =
            "If you delete, you will lose all unsaved progress, and the\n"
            "GUI definition's files in your disk will be gone FOREVER!";
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
    [this](const string& path) -> string {
        return path;
    },
    [this](const string& path) {
        closeTopDialog();
        loadGuiDefFile(path, true);
    },
    [this](const string& path) {
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
                "Pick a GUI definition to load.",
                "Ctrl + L"
            );
            
            //Reload current file item.
            if(ImGui::MenuItem("Reload current GUI definition")) {
                reloadWidgetPos = getLastWidgetPost();
                reloadCmd(1.0f);
            }
            setTooltip(
                "Lose all changes and reload the current definition "
                "from your disk."
            );
            
            //Save file item.
            if(ImGui::MenuItem("Save current GUI definition", "Ctrl+S")) {
                saveCmd(1.0f);
            }
            setTooltip(
                "Save the GUI definition to your disk.",
                "Ctrl + S"
            );
            
            //Delete current GUI definition item.
            if(ImGui::MenuItem("Delete current GUI definition")) {
                deleteGuiDefCmd(1.0f);
            }
            setTooltip(
                "Delete the current GUI definition from your disk."
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
    for(const auto& g : game.content.guiDefs.manifests) {
        guiFiles.push_back(g.first);
    }
    ImGui::Spacer();
    newDialog.mustUpdate |=
        monoCombo("Definition", &newDialog.internalName, guiFiles);
        
    //Check if everything's ok.
    if(newDialog.mustUpdate) {
        newDialog.problem.clear();
        if(newDialog.internalName.empty()) {
            newDialog.problem =
                "You have to select a definition!";
        } else if(!isInternalNameGood(newDialog.internalName)) {
            newDialog.problem =
                "The internal name should only have lowercase letters,\n"
                "numbers, and underscores!";
        } else if(newDialog.pack == FOLDER_NAMES::BASE_PACK) {
            newDialog.problem =
                "All the GUI definitions already live in the\n"
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
                    "for that GUI in that pack!";
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
    
    //Misc. node.
    if(saveableTreeNode("options", "Misc.")) {
    
        //Quick play area combo.
        vector<string> areaNames;
        vector<string> areaPaths;
        int selectedAreaIdx = -1;
        getQuickPlayAreaList(
            game.options.guiEd.quickPlayAreaPath,
            &areaNames, &areaPaths, &selectedAreaIdx
        );
        if(ImGui::Combo("Quick play area", &selectedAreaIdx, areaNames)) {
            if(selectedAreaIdx == -1) {
                game.options.guiEd.quickPlayAreaPath.clear();
            } else {
                game.options.guiEd.quickPlayAreaPath =
                    areaPaths[selectedAreaIdx];
            }
        }
        setTooltip("Area to play on when choosing the quick play feature.");
        
        ImGui::TreePop();
    }
    
    ImGui::Spacer();
    
    processGuiEditorStyle();
}


/**
 * @brief Processes the custom items panel for this frame.
 */
void GuiEditor::processGuiPanelCustom() {
    ImGui::BeginChild("custom");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("CUSTOM ITEMS");
    
    processGuiPanelItems();
    
    if(curItemIdx != INVALID) {
        processGuiPanelItem();
        processGuiPanelCustomItem();
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the custom GUI item data panel for this frame.
 */
void GuiEditor::processGuiPanelCustomItem() {
    if(curItemIdx == INVALID) return;
    
    CustomGuiItemDef* curItemPtr = (CustomGuiItemDef*) allItems[curItemIdx];
    
    if(curItemPtr->size.x == 0.0f) return;
    
    //Custom data header text.
    ImGui::Spacer();
    ImGui::Text("Custom data:");
    
    //Type combobox.
    vector<string> typesList {
        "Bitmap",
        "9-slice texture",
        "Text",
        "Rectangle",
        "Filled rectangle",
        "Square",
        "Filled square",
        "Ellipse",
        "Filled ellipse",
        "Circle",
        "Filled circle",
    };
    int typeInt = (int) curItemPtr->type;
    if(ImGui::Combo("Type", &typeInt, typesList)) {
        typeInt = std::max(typeInt, 0);
        curItemPtr->type = (CUSTOM_GUI_ITEM_TYPE) typeInt;
        curItemPtr->clearBitmap();
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Type of content that will be drawn inside the GUI item."
    );
    
    //Color picker.
    if(
        ImGui::ColorEdit4(
            "Color",
            (float*) &curItemPtr->color
        )
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Color to tint the bitmap with, or color of the text or shape to draw."
    );
    
    //Draw before hardcoded checkbox.
    if(
        ImGui::Checkbox(
            "Draw before hardcoded items",
            &curItemPtr->drawBeforeHardcoded
        )
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "If checked, this item will be drawn before the hardcoded items.\n"
        "Otherwise, it will be drawn after all the hardcoded items.\n"
        "Whether other custom items that also have this checked will be drawn\n"
        "before or after this one depends on the order in the list above."
    );

    //Description input.
    if(ImGui::InputText("Description", &curItemPtr->description)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Optional description.\n"
        "This shows up when your mouse is over the item\n"
        "in the list of items above."
    );
    
    if(
        curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_BITMAP ||
        curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_9_SLICE
    ) {
        //Choose the image button.
        if(ImGui::Button("Choose image...")) {
            openBitmapDialog(
            [this, curItemPtr] (const string& bmp) {
                if(bmp != curItemPtr->bitmapName) {
                    //New image, delete the old one.
                    if(curItemPtr->bitmap != game.bmpError) {
                        game.content.bitmaps.list.free(
                            curItemPtr->bitmapName
                        );
                    }
                    curItemPtr->bitmapName = bmp;
                    curItemPtr->bitmap =
                        game.content.bitmaps.list.get(
                            curItemPtr->bitmapName, nullptr, false
                        );
                    changesMgr.markAsChanged();
                }
                setStatus("Picked an image successfully.");
            }
            );
        }
        setTooltip(
            "Choose which image to use from the game's content."
        );
        
        //Image name text.
        ImGui::SameLine();
        monoText("%s", curItemPtr->bitmapName.c_str());
        setTooltip("Internal name:\n" + curItemPtr->bitmapName);
        
    } else if(curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_TEXT) {
        //Text input.
        if(ImGui::InputText("Text", &curItemPtr->text)) {
            changesMgr.markAsChanged();
        }
        setTooltip("Text to write in the GUI item.");
        
        //Font combobox.
        vector<string> fontsList {
            "Area name",
            "Counter",
            "Leader cursor counter",
            "Slim",
            "Standard",
            "Value",
        };
        int fontInt = (int) curItemPtr->fontType;
        if(ImGui::Combo("Font", &fontInt, fontsList)) {
            fontInt = std::max(fontInt, 0);
            curItemPtr->fontType = (ENGINE_FONT) fontInt;
            changesMgr.markAsChanged();
        }
        setTooltip("Font to use for the text.");
        
        //Alignment combobox.
        vector<string> alignmentsList {
            "Left",
            "Center",
            "Right",
        };
        int alignmentInt = (int) curItemPtr->textAlignment;
        if(ImGui::Combo("Alignment", &alignmentInt, alignmentsList)) {
            alignmentInt = std::max(alignmentInt, 0);
            curItemPtr->textAlignment = alignmentInt;
            changesMgr.markAsChanged();
        }
        setTooltip("Text alignment.");
        
    } else {
        if(
            curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_RECTANGLE ||
            curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_SQUARE ||
            curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_ELLIPSE ||
            curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_CIRCLE
        ) {
            //Thickness value.
            if(
                ImGui::DragFloat(
                    "Thickness", &curItemPtr->thickness, 0.05f, 0.001f, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Thickness of the line that makes up the shape.",
                "", WIDGET_EXPLANATION_DRAG
            );
        }
        
        if(
            curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_RECTANGLE ||
            curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_FILLED_RECTANGLE ||
            curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_SQUARE ||
            curItemPtr->type == CUSTOM_GUI_ITEM_TYPE_FILLED_SQUARE
        ) {
            //Rounding value.
            if(
                ImGui::DragFloat(
                    "Rounding", &curItemPtr->rectangleRounding, 0.05f
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Radius of the rounding of the corners.",
                "", WIDGET_EXPLANATION_DRAG
            );
        }
        
    }
}


/**
 * @brief Processes the hardcoded items panel for this frame.
 */
void GuiEditor::processGuiPanelHardcoded() {
    ImGui::BeginChild("hardcoded");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("HARDCODED ITEMS");
    
    processGuiPanelItems();
    
    if(curItemIdx != INVALID) {
        processGuiPanelItem();
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui GUI definition info control panel
 * for this frame.
 */
void GuiEditor::processGuiPanelInfo() {
    ImGui::BeginChild("info");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("INFO");
    
    //Name input.
    if(ImGui::InputText("Name", &contentMd.name)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Name of this GUI definition. Optional."
    );
    
    //Description input.
    if(ImGui::InputText("Description", &contentMd.description)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Description of this GUI definition. Optional."
    );
    
    //Version input.
    if(monoInputText("Version", &contentMd.version)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Version of the definition, preferably in the \"X.Y.Z\" format. "
        "Optional."
    );
    
    //Maker input.
    if(ImGui::InputText("Maker", &contentMd.maker)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Name (or nickname) of who made this definition. "
        "Optional."
    );
    
    //Maker notes input.
    if(ImGui::InputText("Maker notes", &contentMd.makerNotes)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Extra notes or comments about the definition for other makers to see. "
        "Optional."
    );
    
    //Notes input.
    if(ImGui::InputText("Notes", &contentMd.notes)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Extra notes or comments of any kind. "
        "Optional."
    );
    
    ImGui::EndChild();
}


/**
 * @brief Processes the GUI item info panel for this frame.
 */
void GuiEditor::processGuiPanelItem() {
    if(curItemIdx == INVALID) return;
    
    GuiItemDef* curItemPtr = allItems[curItemIdx];
    
    if(curItemPtr->size.x == 0.0f) return;
    
    //Item's name text.
    ImGui::Spacer();
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
            "itemsList", ImVec2(0.0f, 200.0f), ImGuiChildFlags_Borders
        )
    ) {
        for(size_t i = 0; i < allItems.size(); i++) {
            GuiItemDef* item = allItems[i];
            
            bool isCustom = i >= hardcodedItems.size();
            if(state == EDITOR_STATE_HARDCODED && isCustom) continue;
            if(state == EDITOR_STATE_CUSTOM && !isCustom) continue;
            
            //Item checkbox.
            bool visible = item->size.x != 0.0f;
            if(
                ImGui::Checkbox(("##v" + item->name).c_str(), &visible)
            ) {
                if(visible) {
                    setToDefaults(item);
                } else {
                    item->center.x = 0.0f;
                    item->center.y = 0.0f;
                    item->size.x = 0.0f;
                    item->size.y = 0.0f;
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
            bool selected = curItemIdx == i;
            ImGui::SameLine();
            if(
                monoSelectable(item->name.c_str(), &selected)
            ) {
                curItemIdx = i;
            }
            if(!item->description.empty()) {
                setTooltip(wordWrap(item->description, 50));
            }
            
            if(mustFocusOnCurItem && selected) {
                ImGui::SetScrollHereY(0.5f);
                mustFocusOnCurItem = false;
            }
            
        }
        ImGui::EndChild();
    }
    
    if(state == EDITOR_STATE_CUSTOM) {
    
        CustomGuiItemDef* curItemPtr = nullptr;
        if(curItemIdx != INVALID) {
            curItemPtr = (CustomGuiItemDef*) allItems[curItemIdx];
        }
        
        //New item button.
        if(
            ImGui::ImageButton(
                "newItemButton", editorIcons[EDITOR_ICON_ADD],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            CustomGuiItemDef newItem;
            newItem.name = "new_item";
            setToDefaults(&newItem);
            customItems.push_back(newItem);
            rebuildAllItemsCache();
            curItemIdx = allItems.size() - 1;
            curItemPtr = (CustomGuiItemDef*) allItems[curItemIdx];;
            setStatus("Created a new custom GUI item.");
        }
        setTooltip(
            "Add a new custom GUI item."
        );
        
        if(curItemPtr) {
            //Delete item button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "delItemButton", editorIcons[EDITOR_ICON_REMOVE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                string deletedItemName = curItemPtr->name;
                size_t customIdx = curItemIdx - hardcodedItems.size();
                curItemPtr->clearBitmap();
                customItems.erase(customItems.begin() + customIdx);
                rebuildAllItemsCache();
                curItemIdx = INVALID;
                changesMgr.markAsChanged();
                setStatus("Deleted item \"" + deletedItemName + "\".");
            }
            setTooltip("Delete the current item.");
            
            //Rename item button.
            static string renameItemName;
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "renameItemButton", editorIcons[EDITOR_ICON_INFO],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                duplicateString(curItemPtr->name, renameItemName);
                openInputPopup("renameItem");
            }
            setTooltip(
                "Rename the current GUI item."
            );
            
            //Rename item popup.
            if(
                processGuiInputPopup(
                    "renameItem", "New name:", &renameItemName, true
                )
            ) {
                renameItem(curItemPtr, renameItemName);
            }
            
            //Move item up button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "moveItemUpButton", editorIcons[EDITOR_ICON_MOVE_LEFT],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                size_t customIdx = curItemIdx - hardcodedItems.size();
                if(customIdx > 0) {
                    std::swap(
                        customItems[customIdx], customItems[customIdx - 1]
                    );
                    rebuildAllItemsCache();
                    curItemIdx--;
                    changesMgr.markAsChanged();
                    setStatus("Moved item up.");
                } else {
                    setStatus("This is already the topmost item.");
                }
            }
            setTooltip(
                "Move the current item up in the list.\n"
                "Items are drawn in order from top to bottom."
            );
            
            //Move item down button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "moveItemDownButton", editorIcons[EDITOR_ICON_MOVE_RIGHT],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                size_t customIdx = curItemIdx - hardcodedItems.size();
                if(customIdx < customItems.size() - 1) {
                    std::swap(
                        customItems[customIdx], customItems[customIdx + 1]
                    );
                    rebuildAllItemsCache();
                    curItemIdx++;
                    changesMgr.markAsChanged();
                    setStatus("Moved item down.");
                } else {
                    setStatus("This is already the bottommost item.");
                }
            }
            setTooltip(
                "Move the current item down in the list.\n"
                "Items are drawn in order from top to bottom."
            );
            
        }
        
    }
}


/**
 * @brief Processes the Dear ImGui main control panel for this frame.
 */
void GuiEditor::processGuiPanelMain() {
    if(manifest.internalName.empty()) return;
    
    ImGui::BeginChild("main");
    
    //Current definition header text.
    ImGui::Text("Definition: ");
    
    //Current definition text.
    ImGui::SameLine();
    monoText("%s", manifest.internalName.c_str());
    string fileTooltip =
        getFileTooltip(manifest.path) + "\n\n"
        "File state: ";
    if(!changesMgr.existsOnDisk()) {
        fileTooltip += "Doesn't exist in your disk yet!";
    } else if(changesMgr.hasUnsavedChanges()) {
        fileTooltip += "You have unsaved changes.";
    } else {
        fileTooltip += "Everything ok.";
    }
    setTooltip(fileTooltip);
    
    //Hardcoded items button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "hardcodedButton", editorIcons[EDITOR_ICON_MOB_RADIUS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Hardcoded items"
        )
    ) {
        changeState(EDITOR_STATE_HARDCODED);
    }
    setTooltip(
        "Change the layout of the hardcoded GUI items the engine needs."
    );
    
    //Custom items button.
    if(
        ImGui::ImageButtonAndText(
            "customButton", editorIcons[EDITOR_ICON_DETAILS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Custom items"
        )
    ) {
        changeState(EDITOR_STATE_CUSTOM);
    }
    setTooltip(
        "Make entirely custom GUI items for added decoration."
    );
    
    //Information button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "infoButton", editorIcons[EDITOR_ICON_INFO],
            Point(EDITOR::ICON_BMP_SIZE),
            8.0f, "Info"
        )
    ) {
        changeState(EDITOR_STATE_INFO);
    }
    setTooltip(
        "Set the GUI definition's information here, if you want."
    );
    
    //Stats node.
    ImGui::Spacer();
    if(saveableTreeNode("main", "Stats")) {
    
        //Hardcoded item amount text.
        ImGui::BulletText(
            "Hardcoded items: %i", (int) hardcodedItems.size()
        );
        
        //Custom item amount text.
        ImGui::BulletText(
            "Custom items: %i", (int) customItems.size()
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
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
            resizeString(
                f2s(game.editorsView.mouseCursorWorldPos.x),
                7, true, true, false, ' ', "%"
            ).c_str(),
            resizeString(
                f2s(game.editorsView.mouseCursorWorldPos.y),
                7, true, true, false, ' ', "%"
            ).c_str()
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
        "Pick a GUI definition to load.",
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
        "Save the GUI definition to your disk.",
        "Ctrl + S"
    );
    
    //Quick play button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "playButton", editorIcons[EDITOR_ICON_PLAY],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        quickPlayCmd(1.0f);
    }
    if(ImGui::BeginPopupContextItem()) {
        vector<string> areaNames;
        vector<string> areaPaths;
        int selectedAreaIdx = -1;
        getQuickPlayAreaList(
            game.options.guiEd.quickPlayAreaPath,
            &areaNames, &areaPaths, &selectedAreaIdx
        );
        for(int a = 0; a < (int) areaNames.size(); a++) {
            if(ImGui::Selectable(areaNames[a].c_str(), a == selectedAreaIdx)) {
                game.options.guiEd.quickPlayAreaPath = areaPaths[a];
                saveOptions();
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::EndPopup();
    }
    setTooltip(
        "Save, quit, and start playing the area chosen in the options.\n"
        "Leaving will return to the editor.\n"
        "This button will not do anything if the area is not set properly.\n"
        "You can also right-click the button to choose the area.",
        "Ctrl + P"
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
