/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle editor Dear ImGui logic.
 */

#include "editor.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../lib/imgui/imgui_stdlib.h"
#include "../../util/allegro_utils.h"
#include "../../util/imgui_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Opens the "load" dialog.
 */
void ParticleEditor::openLoadDialog() {
    reloadPartGens();
    
    //Set up the picker's behavior and data.
    vector<PickerItem> file_items;
    for(const auto &g : game.content.particleGens.list) {
        ContentManifest* man = g.second.manifest;
        file_items.push_back(
            PickerItem(
                g.second.name,
                "Pack: " + game.content.packs.list[man->pack].name, "",
                (void*) man,
                getFileTooltip(man->path)
            )
        );
    }
    
    loadDialogPicker = Picker(this);
    loadDialogPicker.items = file_items;
    loadDialogPicker.pickCallback =
        std::bind(
            &ParticleEditor::pickPartGenFile, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );
        
    //Open the dialog that will contain the picker and history.
    openDialog(
        "Load a particle generator file",
        std::bind(&ParticleEditor::processGuiLoadDialog, this)
    );
    dialogs.back()->closeCallback =
        std::bind(&ParticleEditor::closeLoadDialog, this);
}


/**
 * @brief Opens the "new" dialog.
 */
void ParticleEditor::openNewDialog() {
    openDialog(
        "Create a new particle generator",
        std::bind(&ParticleEditor::processGuiNewDialog, this)
    );
    dialogs.back()->customSize = Point(400, 0);
    dialogs.back()->closeCallback = [this] () {
        newDialog.pack.clear();
        newDialog.internalName = "my_particle_generator";
        newDialog.partGenPath.clear();
        newDialog.lastCheckedPartGenPath.clear();
        newDialog.partGenPathExists = false;
    };
    
}


/**
 * @brief Opens the options dialog.
 */
void ParticleEditor::openOptionsDialog() {
    openDialog(
        "Options",
        std::bind(&ParticleEditor::processGuiOptionsDialog, this)
    );
    dialogs.back()->closeCallback =
        std::bind(&ParticleEditor::closeOptionsDialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void ParticleEditor::processGui() {
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.winW, game.winH));
    ImGui::Begin(
        "Particle editor", nullptr,
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
void ParticleEditor::processGuiControlPanel() {
    if(manifest.internalName.empty()) return;
    
    ImGui::BeginChild("panel");
    
    //Current file header text.
    ImGui::Text("File: ");
    
    //Current file text.
    ImGui::SameLine();
    monoText("%s", manifest.internalName.c_str());
    string file_tooltip =
        getFileTooltip(manifest.path) + "\n\n"
        "File state: ";
    if(!changesMgr.existsOnDisk()) {
        file_tooltip += "Not saved to disk yet!";
    } else if(changesMgr.hasUnsavedChanges()) {
        file_tooltip += "You have unsaved changes.";
    } else {
        file_tooltip += "Everything ok.";
    }
    setTooltip(file_tooltip);
    
    ImGui::Spacer();
    
    //Process the particle generator info.
    processGuiPanelGenerator();
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui particle generator deletion dialog
 * for this frame.
 */
void ParticleEditor::processGuiDeletePartGenDialog() {
    //Explanation text.
    string explanation_str;
    if(!changesMgr.existsOnDisk()) {
        explanation_str =
            "You have never saved this particle generator to disk, so if you\n"
            "delete, you will only lose your unsaved progress.";
    } else {
        explanation_str =
            "If you delete, you will lose all unsaved progress, and the\n"
            "particle generator's files on the disk will be gone FOREVER!";
    }
    ImGui::SetupCentering(ImGui::CalcTextSize(explanation_str.c_str()).x);
    ImGui::Text("%s", explanation_str.c_str());
    
    //Final warning text.
    string final_warning_str =
        "Are you sure you want to delete the current particle generator?";
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
        deleteCurrentPartGen();
    }
    ImGui::PopStyleColor(3);
}


/**
 * @brief Processes the "load" dialog for this frame.
 */
void ParticleEditor::processGuiLoadDialog() {
    //History node.
    processGuiHistory(
        game.options.partEd.history,
    [this](const string &path) -> string {
        return path;
    },
    [this](const string &path) {
        closeTopDialog();
        loadPartGenFile(path, true);
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
        "Creates a new particle generator."
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
void ParticleEditor::processGuiMenuBar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load file item.
            if(ImGui::MenuItem("Load or create...", "Ctrl+L")) {
                loadWidgetPos = getLastWidgetPost();
                loadCmd(1.0f);
            }
            setTooltip(
                "Pick a particle generator file to load.",
                "Ctrl + L"
            );
            
            //Reload current file item.
            if(ImGui::MenuItem("Reload current particle generator")) {
                reloadWidgetPos = getLastWidgetPost();
                reloadCmd(1.0f);
            }
            setTooltip(
                "Lose all changes and reload the current file from the disk."
            );
            
            //Save file item.
            if(ImGui::MenuItem("Save current particle generator", "Ctrl+S")) {
                saveCmd(1.0f);
            }
            setTooltip(
                "Save the particle generator into the file on disk.",
                "Ctrl + S"
            );
            
            //Delete current particle generator item.
            if(ImGui::MenuItem("Delete current particle generator")) {
                deletePartGenCmd(1.0f);
            }
            setTooltip(
                "Delete the current particle generator from the disk."
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
                "Quit the particle editor.",
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
                    "The particle editor allows you to change how each "
                    "particle generator works. In-game, particle generators "
                    "are responsible for generating particles, and each one "
                    "emits particles differently. Each generator also has "
                    "information about its particles' sizes, colors, movement, "
                    "etc."
                    "\n\n"
                    "If you need more help on how to use the particle editor, "
                    "check out the tutorial in the manual, located "
                    "in the engine's folder.";
                openHelpDialog(help_str, "particle.html");
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
void ParticleEditor::processGuiNewDialog() {
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
        "Internal name of the new particle generator.\n"
        "Remember to keep it simple, type in lowercase, and use underscores!"
    );
    
    //Check if everything's ok.
    ContentManifest temp_man;
    temp_man.pack = newDialog.pack;
    temp_man.internalName = newDialog.internalName;
    newDialog.partGenPath =
        game.content.particleGens.manifestToPath(temp_man);
    if(newDialog.lastCheckedPartGenPath != newDialog.partGenPath) {
        newDialog.partGenPathExists =
            fileExists(newDialog.partGenPath);
        newDialog.lastCheckedPartGenPath = newDialog.partGenPath;
    }
    
    if(newDialog.internalName.empty()) {
        problem = "You have to type an internal name first!";
    } else if(!isInternalNameGood(newDialog.internalName)) {
        problem =
            "The internal name should only have lowercase letters,\n"
            "numbers, and underscores!";
    } else {
        if(newDialog.partGenPathExists) {
            problem =
                "There is already a particle generator with\n"
                "that internal name in that pack!";
        }
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::SetupCentering(200);
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create particle generator", ImVec2(200, 40))) {
        hit_create_button = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    setTooltip(problem.empty() ? "Create the particle generator!" : problem);
    
    //Creation logic.
    if(hit_create_button) {
        if(!problem.empty()) return;
        auto really_create = [this] () {
            createPartGen(newDialog.partGenPath);
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
 * @brief Processes the options dialog for this frame.
 */
void ParticleEditor::processGuiOptionsDialog() {
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
            "Grid interval: %f", game.options.partEd.gridInterval
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
            "Default: " + i2s(OPTIONS::PART_ED_D::GRID_INTERVAL) +
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
            "Default: " + i2s(OPTIONS::PART_ED_D::GRID_INTERVAL) +
            ".",
            "Shift + Minus"
        );
        
        ImGui::TreePop();
        
    }
    
    ImGui::Spacer();
    
    processGuiEditorStyle();
    
    ImGui::Spacer();
    
    //Misc. node.
    if(saveableTreeNode("options", "Misc.")) {
    
        //Background texture checkbox.
        if(ImGui::Checkbox("Use background texture", &useBg)) {
            if(!useBg) {
                if(bg) {
                    al_destroy_bitmap(bg);
                    bg = nullptr;
                }
                game.options.partEd.bgPath.clear();
            }
        }
        setTooltip(
            "Check this to use a repeating texture on the background\n"
            "of the editor."
        );
        
        if(useBg) {
            ImGui::Indent();
            
            //Remove background texture button.
            unsigned char rem_bg_opacity =
                game.options.partEd.bgPath.empty() ? 50 : 255;
            if(
                ImGui::ImageButton(
                    "remBgButton", editorIcons[EDITOR_ICON_REMOVE],
                    Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                    COLOR_EMPTY, mapAlpha(rem_bg_opacity)
                )
            ) {
                game.options.partEd.bgPath.clear();
                if(bg) {
                    al_destroy_bitmap(bg);
                    bg = nullptr;
                }
            }
            setTooltip(
                "Remove the background image.\n"
                "This does not delete the file on your disk."
            );
            
            //Background texture browse button.
            ImGui::SameLine();
            if(ImGui::Button("Browse...")) {
                vector<string> f =
                    promptFileDialog(
                        FOLDER_PATHS_FROM_ROOT::BASE_PACK + "/" +
                        FOLDER_PATHS_FROM_PACK::TEXTURES,
                        "Please choose a background texture.",
                        "*.*", 0, game.display
                    );
                    
                if(!f.empty() && !f[0].empty()) {
                    game.options.partEd.bgPath = f[0];
                    if(bg) {
                        al_destroy_bitmap(bg);
                        bg = nullptr;
                    }
                    bg =
                        loadBmp(
                            game.options.partEd.bgPath,
                            nullptr, false, false, false
                        );
                }
            }
            setTooltip(
                "Browse for which texture file on your disk to use."
            );
            
            //Background texture name text.
            string bg_file_name =
                getPathLastComponent(game.options.partEd.bgPath);
            ImGui::SameLine();
            monoText("%s", bg_file_name.c_str());
            setTooltip("Full path:\n" + game.options.partEd.bgPath);
            
            ImGui::Unindent();
        }
        
        ImGui::TreePop();
        
    }
}


/**
 * @brief Processes the particle generator panel for this frame.
 */
void ParticleEditor::processGuiPanelGenerator() {
    //Particle system text.
    ImGui::Text("Particle system:");
    
    //Particle count text.
    ImGui::Indent();
    ImGui::Text(
        "Particles: %lu / %lu",
        partMgr.getCount(), game.options.advanced.maxParticles
    );
    
    //Play/pause particle system button.
    if(
        ImGui::ImageButton(
            "playSystemButton",
            mgrRunning ?
            editorIcons[EDITOR_ICON_STOP] :
            editorIcons[EDITOR_ICON_PLAY],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        partMgrPlaybackToggleCmd(1.0f);
    }
    setTooltip(
        "Play or pause the particle system.",
        "Shift + Spacebar"
    );
    
    ImGui::SameLine();
    
    //Clear particles button.
    if(
        ImGui::ImageButton(
            "clearParticlesButton", editorIcons[EDITOR_ICON_REMOVE],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        clearParticlesCmd(1.0f);
    }
    setTooltip(
        "Delete all existing particles.", "D"
    );
    ImGui::Unindent();
    
    //Particle generator text.
    ImGui::Text("Generator:");
    
    //Play/pause particle generator button.
    ImGui::Indent();
    if(
        ImGui::ImageButton(
            "playGeneratorButton",
            genRunning ?
            editorIcons[EDITOR_ICON_STOP] :
            editorIcons[EDITOR_ICON_PLAY],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        partGenPlaybackToggleCmd(1.0f);
    }
    setTooltip(
        loadedGen.emission.interval == 0.0f ?
        "Emit particles now." :
        "Play or pause the particle generator's emission.",
        "Spacebar"
    );
    
    //Particle generator angle value.
    ImGui::SameLine();
    ImGui::SetNextItemWidth(85);
    ImGui::SliderAngle("Angle", &generatorAngleOffset, 0.0f);
    setTooltip(
        "Rotate the generator's facing angle in the editor by this much.\n"
        "You can move the generator by just dragging the mouse in the canvas.",
        "", WIDGET_EXPLANATION_SLIDER
    );
    ImGui::Unindent();
    
    //Emission node.
    ImGui::Spacer();
    bool open_emission_node =
        saveableTreeNode("generator", "Emission");
    setTooltip(
        "Everything about how the particle generator emits new particles."
    );
    if(open_emission_node) {
    
        //Basics node.
        bool open_basics_node =
            saveableTreeNode("generatorEmission", "Basics");
        setTooltip("Edit basic information about emission here.");
        if(open_basics_node) {
        
            //Emit mode text.
            ImGui::Text("Mode:");
            
            //Emit once radio.
            int emit_mode = loadedGen.emission.interval == 0.0f ? 0 : 1;
            ImGui::SameLine();
            if(ImGui::RadioButton("Once", &emit_mode, 0)) {
                if(loadedGen.emission.interval != 0.0f) {
                    loadedGen.emission.interval = 0.0f;
                    loadedGen.emission.intervalDeviation = 0.0f;
                    loadedGen.restartTimer();
                }
                changesMgr.markAsChanged();
            }
            setTooltip("The particles are created just once.");
            
            //Emit continuously radio.
            ImGui::SameLine();
            if(ImGui::RadioButton("Interval", &emit_mode, 1)) {
                if(loadedGen.emission.interval == 0.0f) {
                    loadedGen.emission.interval = 0.01f;
                    loadedGen.emission.intervalDeviation = 0.0f;
                    loadedGen.restartTimer();
                }
                changesMgr.markAsChanged();
            }
            setTooltip(
                "The particles are constantly being created\n"
                "over time, with a set interval."
            );
            
            if(emit_mode == 1) {
                //Emission interval value.
                ImGui::Indent();
                ImGui::SetNextItemWidth(85);
                if(
                    ImGui::DragFloat(
                        "##interval", &loadedGen.emission.interval,
                        0.01f, 0.01f, FLT_MAX
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "How long between particle emissions, in seconds.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Emission interval deviation text.
                ImGui::SameLine();
                ImGui::Text(" +-");
                
                //Emission interval deviation value.
                ImGui::SameLine();
                ImGui::SetNextItemWidth(70);
                if(
                    ImGui::DragFloat(
                        "##intervalDeviation",
                        &loadedGen.emission.intervalDeviation,
                        0.01f, 0.0f, FLT_MAX
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "The emission interval varies randomly up or down "
                    "by this amount.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                ImGui::Unindent();
            }
            
            //Emission number text.
            ImGui::Spacer();
            ImGui::Text("Number:");
            
            //Emission number value.
            int number_int = (int) loadedGen.emission.number;
            ImGui::Indent();
            ImGui::SetNextItemWidth(85);
            if(
                ImGui::DragInt(
                    "##number", &number_int, 1, 1, (int) game.options.advanced.maxParticles
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "How many particles are created per emission.",
                "", WIDGET_EXPLANATION_DRAG
            );
            loadedGen.emission.number = number_int;
            
            //Emission number deviation text.
            ImGui::SameLine();
            ImGui::Text(" +-");
            
            //Emission number deviation value.
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            int number_dev_int = (int) loadedGen.emission.numberDeviation;
            if(
                ImGui::DragInt(
                    "##numberDeviation",
                    &number_dev_int, 1, 0, (int) game.options.advanced.maxParticles
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "The creation amount varies randomly up or down by this "
                "amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            loadedGen.emission.numberDeviation = number_dev_int;
            
            ImGui::Unindent();
            
            ImGui::TreePop();
            
        }
        
        //Shape node.
        ImGui::Spacer();
        bool open_shape_node =
            saveableTreeNode("generatorEmission", "Shape");
        setTooltip(
            "If you want the particles to appear within a specific shape\n"
            "around the generator, edit these properties."
        );
        if(open_shape_node) {
        
            //Circle emission shape radio.
            int shape = loadedGen.emission.shape;
            if(
                ImGui::RadioButton(
                    "Circle", &shape, PARTICLE_EMISSION_SHAPE_CIRCLE
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Makes it so particles are created in a circle or \n"
                "ring shape around the origin."
            );
            
            //Rectangle emission shape radio.
            ImGui::SameLine();
            if(
                ImGui::RadioButton(
                    "Rectangle", &shape, PARTICLE_EMISSION_SHAPE_RECTANGLE
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Makes it so particles are created in a rectangle or \n"
                "rectangular ring shape around the origin."
            );
            loadedGen.emission.shape = (PARTICLE_EMISSION_SHAPE)shape;
            
            ImGui::Indent();
            switch (loadedGen.emission.shape) {
            case PARTICLE_EMISSION_SHAPE_CIRCLE: {
                //Circle emission inner distance value.
                ImGui::SetNextItemWidth(75);
                if(
                    ImGui::DragFloat(
                        "Inner distance",
                        &loadedGen.emission.circleInnerDist,
                        0.1f, 0.0f, FLT_MAX
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "Minimum emission distance for particle creation.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Circle emission outer distance value.
                ImGui::SetNextItemWidth(75);
                if(
                    ImGui::DragFloat(
                        "Outer distance",
                        &loadedGen.emission.circleOuterDist,
                        0.1f, 0.0f, FLT_MAX
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "Maximum emission distance for particle creation.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                loadedGen.emission.circleInnerDist =
                    std::max(
                        loadedGen.emission.circleInnerDist,
                        0.0f
                    );
                loadedGen.emission.circleOuterDist =
                    std::max(
                        loadedGen.emission.circleInnerDist,
                        loadedGen.emission.circleOuterDist
                    );
                    
                //Circle emission arc value.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::SliderAngle(
                        "Arc", &loadedGen.emission.circleArc, 0
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "Arc of the circle for particle creation.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Circle emission arc rotation value.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::SliderAngle(
                        "Arc rotation", &loadedGen.emission.circleArcRot,
                        0.0f
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "Rotate the emission arc by these many degrees.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
                
                //Evenly spread checkbox.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::Checkbox(
                        "Evenly spread", &loadedGen.emission.evenlySpread
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "Evenly spread the particles throughout the emission\n"
                    "area, instead of placing them randomly."
                );
                
                break;
                
            } case PARTICLE_EMISSION_SHAPE_RECTANGLE: {
                //Rectangle emission inner distance values.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::DragFloat2(
                        "Inner distance",
                        (float*) &loadedGen.emission.rectInnerDist,
                        0.1f, 0.0f, FLT_MAX
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "Minimum emission distance (X and Y) for particle "
                    "creation.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                //Rectangle emission outer distance values.
                ImGui::SetNextItemWidth(150);
                if(
                    ImGui::DragFloat2(
                        "Outer distance",
                        (float*) &loadedGen.emission.rectOuterDist,
                        0.1f, 0.0f, FLT_MAX
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "Maximum emission distance (X and Y) for particle "
                    "creation.",
                    "", WIDGET_EXPLANATION_DRAG
                );
                
                loadedGen.emission.rectInnerDist.x =
                    std::max(
                        loadedGen.emission.rectInnerDist.x,
                        0.0f
                    );
                loadedGen.emission.rectInnerDist.y =
                    std::max(
                        loadedGen.emission.rectInnerDist.y,
                        0.0f
                    );
                loadedGen.emission.rectOuterDist.x =
                    std::max(
                        loadedGen.emission.rectOuterDist.x,
                        loadedGen.emission.rectInnerDist.x
                    );
                loadedGen.emission.rectOuterDist.y =
                    std::max(
                        loadedGen.emission.rectOuterDist.y,
                        loadedGen.emission.rectInnerDist.y
                    );
                    
                break;
            }
            }
            ImGui::Unindent();
            
            ImGui::TreePop();
            
        }
        
        ImGui::TreePop();
    }
    
    //Particle appearance node.
    ImGui::Spacer();
    bool open_appearance_node =
        saveableTreeNode("generator", "Particle appearance");
    setTooltip(
        "Everything about how a particle looks."
    );
    if(open_appearance_node) {
    
        //Image node.
        bool open_image_node =
            saveableTreeNode("generatorAppearance", "Image");
        setTooltip(
            "Edit information about the image (if any) to draw\n"
            "on a particle here."
        );
        if(open_image_node) {
        
            //Remove bitmap button.
            unsigned char rem_bmp_opacity =
                loadedGen.baseParticle.bmpName.empty() ? 50 : 255;
            if(
                ImGui::ImageButton(
                    "remBmpButton", editorIcons[EDITOR_ICON_REMOVE],
                    Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                    COLOR_EMPTY, mapAlpha(rem_bmp_opacity)
                )
            ) {
                //We can't have living particles with destroyed bitmaps,
                //so clear them all.
                partMgr.clear();
                loadedGen.baseParticle.setBitmap("");
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Remove the particles' image.\n"
                "This makes the particles be circles."
            );
            
            //Choose image button.
            ImGui::SameLine();
            if(ImGui::Button("Choose image...")) {
                openBitmapDialog(
                [this] (const string &bmp) {
                    //We can't have living particles with destroyed bitmaps,
                    //so clear them all.
                    partMgr.clear();
                    loadedGen.baseParticle.setBitmap(bmp);
                    changesMgr.markAsChanged();
                    setStatus("Picked an image successfully.");
                },
                "."
                );
            }
            setTooltip("Choose which image to use from the game's content.");
            
            //Image name text.
            ImGui::SameLine();
            monoText("%s", loadedGen.baseParticle.bmpName.c_str());
            setTooltip("Internal name:\n" + loadedGen.baseParticle.bmpName);
            
            if(loadedGen.baseParticle.bitmap) {
            
                //Image angle text.
                ImGui::Spacer();
                ImGui::Text("Angle:");
                
                //Fixed angle radio.
                int angle_type_int = loadedGen.baseParticle.bmpAngleType;
                ImGui::SameLine();
                if(
                    ImGui::RadioButton(
                        "Fixed", &angle_type_int,
                        PARTICLE_ANGLE_TYPE_FIXED
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "A particle's image angle is fixed all throughout."
                );
                
                //Direction angle radio.
                ImGui::SameLine();
                if(
                    ImGui::RadioButton(
                        "Direction", &angle_type_int,
                        PARTICLE_ANGLE_TYPE_DIRECTION
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "A particle's image angle matches the direction it's "
                    "traveling."
                );
                loadedGen.baseParticle.bmpAngleType =
                    (PARTICLE_ANGLE_TYPE) angle_type_int;
                    
                if(
                    loadedGen.baseParticle.bmpAngleType ==
                    PARTICLE_ANGLE_TYPE_FIXED
                ) {
                
                    //Image angle value.
                    ImGui::Indent();
                    ImGui::SetNextItemWidth(85);
                    if(
                        ImGui::SliderAngle(
                            "##imgAngle",
                            &loadedGen.baseParticle.bmpAngle, 0.0f
                        )
                    ) {
                        changesMgr.markAsChanged();
                    }
                    setTooltip(
                        "Angle of the image.",
                        "", WIDGET_EXPLANATION_SLIDER
                    );
                    
                    //Image angle deviation text.
                    ImGui::SameLine();
                    ImGui::Text(" +-");
                    
                    //Angle deviation value.
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(70);
                    if(
                        ImGui::SliderAngle(
                            "##imgAngleDev",
                            &loadedGen.bmpAngleDeviation, 0, 180
                        )
                    ) {
                        changesMgr.markAsChanged();
                    }
                    setTooltip(
                        "A particle's image angle varies randomly up or down\n"
                        "by this amount.",
                        "", WIDGET_EXPLANATION_SLIDER
                    );
                    ImGui::Unindent();
                }
            }
            
            ImGui::TreePop();
            
        }
        
        //Particle color node.
        ImGui::Spacer();
        bool open_color_node =
            saveableTreeNode("generatorAppearance", "Color");
        setTooltip(
            "Control the color a particle has and how it changes over time "
            "here."
        );
        if(open_color_node) {
        
            //Color keyframe editor.
            if(
                keyframeEditor(
                    "Color", loadedGen.baseParticle.color,
                    selectedColorKeyframe
                )
            ) {
                changesMgr.markAsChanged();
            }
            
            //Blend mode text.
            ImGui::Spacer();
            ImGui::Text("Blend:");
            
            //Normal blending radio.
            int blend_int = loadedGen.baseParticle.blendType;
            ImGui::SameLine();
            if(
                ImGui::RadioButton(
                    "Normal", &blend_int, PARTICLE_BLEND_TYPE_NORMAL
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Particles appear on top of other particles like normal."
            );
            
            //Additive blending radio.
            ImGui::SameLine();
            if(
                ImGui::RadioButton(
                    "Additive", &blend_int, PARTICLE_BLEND_TYPE_ADDITIVE
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Particle colors add onto the color of particles underneath\n"
                "them. This makes it so the more particles there are,\n"
                "the brighter the color gets."
            );
            loadedGen.baseParticle.blendType =
                (PARTICLE_BLEND_TYPE) blend_int;
                
            ImGui::TreePop();
            
        }
        
        //Particle size node.
        ImGui::Spacer();
        bool open_size_node =
            saveableTreeNode("generatorAppearance", "Size");
        setTooltip(
            "Control a particle's size and how it changes over time here."
        );
        if(open_size_node) {
        
            //Size keyframe editor.
            if(
                keyframeEditor(
                    "Size", loadedGen.baseParticle.size,
                    selectedSizeKeyframe
                )
            ) {
                changesMgr.markAsChanged();
            }
            loadedGen.baseParticle.size.setKeyframeValue(
                selectedSizeKeyframe,
                std::max(
                    0.0f,
                    loadedGen.baseParticle.size.getKeyframe(
                        selectedSizeKeyframe
                    ).second
                )
            );
            
            //Size deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(70);
            if(
                ImGui::DragFloat(
                    "Size deviation", &loadedGen.sizeDeviation,
                    0.5f, 0.0f, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "A particle's size varies randomly up or down by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            ImGui::TreePop();
            
        }
        
        ImGui::TreePop();
    }
    
    //Particle behavior node.
    ImGui::Spacer();
    bool open_behavior_node =
        saveableTreeNode("generator", "Particle behavior");
    setTooltip(
        "Everything about how a particle behaves."
    );
    if(open_behavior_node) {
    
        //Basics node.
        bool open_basics_node =
            saveableTreeNode("generatorBehavior", "Basics");
        setTooltip(
            "Control how long a particle lasts for, and more, here."
        );
        if(open_basics_node) {
        
            //Duration text.
            ImGui::Text("Duration:");
            
            //Duration value.
            ImGui::SetNextItemWidth(85);
            if(
                ImGui::DragFloat(
                    "##particleDur", &loadedGen.baseParticle.duration,
                    0.01f, 0.01f, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "How long each particle lives for, in seconds.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Duration deviation text.
            ImGui::SameLine();
            ImGui::Text(" +-");
            
            //Duration deviation value.
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            if(
                ImGui::DragFloat(
                    "##particleDurDev",
                    &loadedGen.durationDeviation, 0.01f, 0.0f, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "A particle's lifespan varies randomly up or down by this "
                "amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Absolute angles checkbox.
            ImGui::Spacer();
            if(
                ImGui::Checkbox(
                    "Absolute angles", &loadedGen.anglesAreAbsolute
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "If unchecked, the angles the particles move at are relative\n"
                "to the angle of the object, if the particle generator\n"
                "is attached to an object. If checked, the angles are\n"
                "always the same no matter what."
            );
            
            ImGui::TreePop();
            
        }
        
        //Linear speed node.
        ImGui::Spacer();
        bool open_linear_speed_node =
            saveableTreeNode("generatorBehavior", "Linear speed");
        setTooltip(
            "Control a particle's linear (simple) X and Y speed here."
        );
        if(open_linear_speed_node) {
        
            //Linear speed keyframe editor.
            if(
                keyframeEditor(
                    "Speed", loadedGen.baseParticle.linearSpeed,
                    selectedLinearSpeedKeyframe
                )
            ) {
                changesMgr.markAsChanged();
            }
            
            //Linear speed deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(150);
            if(
                ImGui::DragFloat2(
                    "Speed deviation",
                    (float*) &loadedGen.linearSpeedDeviation,
                    0.5f, 0.0f, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "A particle's linear speed varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Angle deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(75);
            if(
                ImGui::SliderAngle(
                    "Angle deviation",
                    &loadedGen.linearSpeedAngleDeviation, 0, 180
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "A particle's movement angle varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            ImGui::TreePop();
            
        }
        
        //Outwards speed node.
        ImGui::Spacer();
        bool open_outwards_speed_node =
            saveableTreeNode("generatorBehavior", "Outwards speed");
        setTooltip(
            "Control the speed at which a particle moves out from\n"
            "the center here. Use negative values to make them move\n"
            "towards the center instead."
        );
        if(open_outwards_speed_node) {
        
            //Outwards speed keyframe editor.
            if(
                keyframeEditor(
                    "Speed", loadedGen.baseParticle.outwardsSpeed,
                    selectedOutwardVelocityKeyframe
                )
            ) {
                changesMgr.markAsChanged();
            }
            
            //Outward speed deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(150);
            if(
                ImGui::DragFloat(
                    "Speed deviation",
                    &loadedGen.outwardsSpeedDeviation,
                    0.5f, 0.0f, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "A particle's outward speed varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            ImGui::TreePop();
        }
        
        //Orbital speed node.
        ImGui::Spacer();
        bool open_orbital_speed_node =
            saveableTreeNode("generatorBehavior", "Orbital speed");
        setTooltip(
            "Control the speed at which a particle orbits around the center "
            "here."
        );
        if(open_orbital_speed_node) {
        
            //Orbital speed keyframe editor.
            if(
                keyframeEditor(
                    "Speed", loadedGen.baseParticle.orbitalSpeed,
                    selectedOrbitalVelocityKeyframe
                )
            ) {
                changesMgr.markAsChanged();
            }
            
            //Orbital speed deviation value.
            ImGui::Spacer();
            ImGui::SetNextItemWidth(150);
            if(
                ImGui::DragFloat(
                    "Speed deviation",
                    &loadedGen.orbitalSpeedDeviation,
                    0.5f, 0.0f, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "A particle's orbital speed varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            ImGui::TreePop();
        }
        
        //Friction node.
        ImGui::Spacer();
        bool open_friction_node =
            saveableTreeNode("generatorBehavior", "Friction");
        setTooltip(
            "Control how a particle loses speed here."
        );
        if(open_friction_node) {
        
            //Friction value.
            ImGui::SetNextItemWidth(85);
            if(
                ImGui::DragFloat(
                    "##particleFriction",
                    &loadedGen.baseParticle.friction, 0.1f, -FLT_MAX, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Slowing factor applied to a particle.\n"
                "Negative values make it speed up.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Friction deviation text.
            ImGui::SameLine();
            ImGui::Text(" +-");
            
            //Friction deviation value.
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            if(
                ImGui::DragFloat(
                    "##particleFrictionDev",
                    &loadedGen.frictionDeviation, 0.1f, 0.0f, FLT_MAX
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "A particle's friction varies randomly up or down\n"
                "by this amount.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            ImGui::TreePop();
            
        }
        
        ImGui::TreePop();
        
    }
    
    //Info node.
    ImGui::Spacer();
    bool open_info_node =
        saveableTreeNode("generator", "Info");
    setTooltip(
        "Optional information about the particle generator."
    );
    if(open_info_node) {
    
        //Name input.
        if(
            ImGui::InputText("Name", &loadedGen.name)
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Name of this particle generator. Optional."
        );
        
        //Description input.
        if(
            ImGui::InputText("Description", &loadedGen.description)
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Description of this particle generator. Optional."
        );
        
        //Version input.
        if(
            monoInputText("Version", &loadedGen.version)
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Version of the file, preferably in the \"X.Y.Z\" format. "
            "Optional."
        );
        
        //Maker input.
        if(
            ImGui::InputText("Maker", &loadedGen.maker)
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Name (or nickname) of who made this file. "
            "Optional."
        );
        
        //Maker notes input.
        if(
            ImGui::InputText("Maker notes", &loadedGen.makerNotes)
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Extra notes or comments about the file for other makers to see. "
            "Optional."
        );
        
        //Notes input.
        if(ImGui::InputText("Notes", &loadedGen.notes)) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Extra notes or comments of any kind. "
            "Optional."
        );
        
        ImGui::TreePop();
        
    }
    
}


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void ParticleEditor::processGuiStatusBar() {
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
            boxString(f2s(game.view.cursorWorldPos.x), 7).c_str(),
            boxString(f2s(game.view.cursorWorldPos.y), 7).c_str()
        );
    }
}


/**
 * @brief Processes the Dear ImGui toolbar for this frame.
 */
void ParticleEditor::processGuiToolbar() {
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
        "Quit the particle editor.",
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
        "Pick a particle generator file to load.",
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
        "Save the particle generator into the file on disk.",
        "Ctrl + S"
    );
    
    //Toggle grid button.
    ImGui::SameLine(0, 16);
    if(
        ImGui::ImageButton(
            "gridButton", editorIcons[EDITOR_ICON_GRID],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        gridToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of the grid.",
        "Ctrl + G"
    );
    
    //Leader silhouette button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "silhouetteButton", editorIcons[EDITOR_ICON_LEADER_SILHOUETTE],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        leaderSilhouetteToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of a leader silhouette.",
        "Ctrl + P"
    );
    
    //Emission shape button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "emissionShapeButton", editorIcons[EDITOR_ICON_MOB_RADIUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        emissionShapeToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of the emission shape.",
        "Ctrl + R"
    );
}
