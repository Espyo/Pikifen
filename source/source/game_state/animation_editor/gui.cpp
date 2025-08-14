/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Animation editor Dear ImGui logic.
 */

#include <algorithm>

#include "editor.h"

#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_functions.h"
#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../lib/imgui/imgui_stdlib.h"
#include "../../util/allegro_utils.h"
#include "../../util/imgui_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Opens the "load" dialog.
 */
void AnimationEditor::openLoadDialog() {
    reloadAnimDbs();
    
    //Set up the picker's behavior and data.
    vector<PickerItem> fileItems;
    for(const auto& a : game.content.globalAnimDbs.list) {
        fileItems.push_back(
            PickerItem(
                a.second.name,
                "Pack: " +
                game.content.packs.list[a.second.manifest->pack].name,
                "Global animations",
                (void*) a.second.manifest,
                getFileTooltip(a.second.manifest->path)
            )
        );
    }
    for(size_t c = 0; c < customCatTypes.size(); c++) {
        for(size_t a = 0; a < customCatTypes[c].size(); a++) {
            MobType* mtPtr = customCatTypes[c][a];
            if(!mtPtr) continue;
            if(!mtPtr->manifest) continue;
            auto& catAnimDbs =
                game.content.mobAnimDbs.list[mtPtr->category->id];
            auto mtCatAnimIt =
                catAnimDbs.find(mtPtr->manifest->internalName);
            if(mtCatAnimIt == catAnimDbs.end()) continue;
            
            ContentManifest* manPtr =
                mtCatAnimIt->second.manifest;
            fileItems.push_back(
                PickerItem(
                    mtPtr->name,
                    "Pack: " + game.content.packs.list[manPtr->pack].name,
                    mtPtr->customCategoryName + " objects",
                    (void*) manPtr,
                    getFileTooltip(manPtr->path)
                )
            );
        }
    }
    loadDialogPicker = Picker(this);
    loadDialogPicker.items = fileItems;
    loadDialogPicker.pickCallback =
        std::bind(
            &AnimationEditor::pickAnimDbFile, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5
        );
        
    //Open the dialog that will contain the picker and history.
    openDialog(
        "Load an animation database or create a new one",
        std::bind(&AnimationEditor::processGuiLoadDialog, this)
    );
    dialogs.back()->closeCallback =
        std::bind(&AnimationEditor::closeLoadDialog, this);
}


/**
 * @brief Opens the "new" dialog.
 */
void AnimationEditor::openNewDialog() {
    openDialog(
        "Create a new animation database",
        std::bind(&AnimationEditor::processGuiNewDialog, this)
    );
    dialogs.back()->customSize = Point(400, 0);
    dialogs.back()->closeCallback = [this] () {
        newDialog.pack.clear();
        newDialog.type = 0;
        newDialog.customMobCat.clear();
        newDialog.mobTypePtr = nullptr;
        newDialog.internalName = "my_animation";
        newDialog.lastCheckedAnimPath.clear();
        newDialog.animPath.clear();
        newDialog.animPathExists = false;
    };
}


/**
 * @brief Opens the options dialog.
 *
 */
void AnimationEditor::openOptionsDialog() {
    openDialog(
        "Options",
        std::bind(&AnimationEditor::processGuiOptionsDialog, this)
    );
    dialogs.back()->closeCallback =
        std::bind(&AnimationEditor::closeOptionsDialog, this);
}


/**
 * @brief Processes Dear ImGui for this frame.
 */
void AnimationEditor::processGui() {
    //Set up the entire editor window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(game.winW, game.winH));
    ImGui::Begin(
        "Animation editor", nullptr,
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
void AnimationEditor::processGuiControlPanel() {
    ImGui::BeginChild("panel");
    
    //Basically, just show the correct panel for the current state.
    switch(state) {
    case EDITOR_STATE_MAIN: {
        processGuiPanelMain();
        break;
    } case EDITOR_STATE_ANIMATION: {
        processGuiPanelAnimation();
        break;
    } case EDITOR_STATE_SPRITE: {
        processGuiPanelSprite();
        break;
    } case EDITOR_STATE_BODY_PART: {
        processGuiPanelBodyPart();
        break;
    } case EDITOR_STATE_HITBOXES: {
        processGuiPanelSpriteHitboxes();
        break;
    } case EDITOR_STATE_SPRITE_BITMAP: {
        processGuiPanelSpriteBitmap();
        break;
    } case EDITOR_STATE_SPRITE_TRANSFORM: {
        processGuiPanelSpriteTransform();
        break;
    } case EDITOR_STATE_TOP: {
        processGuiPanelSpriteTop();
        break;
    } case EDITOR_STATE_INFO: {
        processGuiPanelInfo();
        break;
    } case EDITOR_STATE_TOOLS: {
        processGuiPanelTools();
        break;
    }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui animation database deletion dialog
 * for this frame.
 */
void AnimationEditor::processGuiDeleteAnimDbDialog() {
    //Explanation text.
    string explanationStr;
    if(!changesMgr.existsOnDisk()) {
        explanationStr =
            "You have never saved this animation database to your disk, so\n"
            "if you delete, you will only lose your unsaved progress.";
    } else {
        explanationStr =
            "If you delete, you will lose all unsaved progress, and the\n"
            "animation database's files in your disk will be gone FOREVER!";
    }
    ImGui::SetupCentering(ImGui::CalcTextSize(explanationStr.c_str()).x);
    ImGui::Text("%s", explanationStr.c_str());
    
    //Final warning text.
    string finalWarningStr =
        "Are you sure you want to delete the current animation database?";
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
        deleteCurrentAnimDb();
    }
    ImGui::PopStyleColor(3);
}


/**
 * @brief Processes the list of the current hitbox's hazards,
 * as well as the widgets necessary to control it, for this frame.
 */
void AnimationEditor::processGuiHitboxHazards() {
    string hazardIname;
    if(curHitbox->hazard) {
        hazardIname = curHitbox->hazard->manifest->internalName;
    }
    if(processGuiHazardManagementWidgets(hazardIname)) {
        changesMgr.markAsChanged();
        curHitbox->hazard =
            hazardIname.empty() ?
            nullptr :
            &game.content.hazards.list[hazardIname];
    }
    setTooltip("Hazard, if any.");
}


/**
 * @brief Processes the "load" dialog for this frame.
 */
void AnimationEditor::processGuiLoadDialog() {
    //History node.
    processGuiHistory(
        game.options.animEd.history,
    [this](const string& path) -> string {
        return path;
    },
    [this](const string& path) {
        closeTopDialog();
        loadAnimDbFile(path, true);
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
    setTooltip("Creates a new animation database.");
    
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
void AnimationEditor::processGuiMenuBar() {
    if(ImGui::BeginMenuBar()) {
    
        //Editor menu.
        if(ImGui::BeginMenu("Editor")) {
        
            //Load file item.
            if(ImGui::MenuItem("Load or create...", "Ctrl+L")) {
                loadWidgetPos = getLastWidgetPost();
                loadCmd(1.0f);
            }
            setTooltip(
                "Pick a database to load.",
                "Ctrl + L"
            );
            
            //Reload current file item.
            if(ImGui::MenuItem("Reload current animation database")) {
                reloadWidgetPos = getLastWidgetPost();
                reloadCmd(1.0f);
            }
            setTooltip(
                "Lose all changes and reload the current "
                "database from your disk."
            );
            
            //Save current file item.
            if(ImGui::MenuItem("Save current animation database", "Ctrl+S")) {
                saveCmd(1.0f);
            }
            setTooltip(
                "Save the animation database to your disk.",
                "Ctrl + S"
            );
            
            //Delete current animation database item.
            if(ImGui::MenuItem("Delete current animation database")) {
                deleteAnimDbCmd(1.0f);
            }
            setTooltip(
                "Delete the current animation database from your disk."
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
                "Quit the animation editor.",
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
                "Move and zoom the camera so that everything in the animation\n"
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
                    "To create an animation, first you need some image file "
                    "to get the animation frames from, featuring the object "
                    "you want to edit in the different poses. After that, "
                    "you define what sprites exist (what parts of the image "
                    "match what poses), and then create animations, populating "
                    "their frames with the sprites.\n\n"
                    "If you need more help on how to use the animation editor, "
                    "check out the tutorial in the manual, located "
                    "in the engine's folder.";
                openHelpDialog(helpStr, "animation.html");
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
void AnimationEditor::processGuiNewDialog() {
    string problem;
    bool hitCreateButton = false;
    
    //Pack widgets.
    processGuiNewDialogPackWidgets(&newDialog.pack);
    
    //Global animation radio.
    ImGui::Spacer();
    ImGui::RadioButton("Global animation", &newDialog.type, 0);
    
    //Mob type animation radio.
    ImGui::SameLine();
    ImGui::RadioButton("Object type", &newDialog.type, 1);
    
    ImGui::Spacer();
    
    if(newDialog.type == 0) {
        //Internal name input.
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
            "Internal name of the new animation database.\n"
            "Remember to keep it simple, type in lowercase, "
            "and use underscores!"
        );
        
        //Small spacer dummy widget.
        ImGui::Dummy(ImVec2(0, 19));
        
    } else {
        //Mob type widgets.
        processGuiMobTypeWidgets(
            &newDialog.customMobCat, &newDialog.mobTypePtr,
            newDialog.pack
        );
        
    }
    
    //Check if everything's ok.
    if(newDialog.type == 0) {
        ContentManifest tempMan;
        tempMan.internalName = newDialog.internalName;
        tempMan.pack = newDialog.pack;
        newDialog.animPath =
            game.content.globalAnimDbs.manifestToPath(tempMan);
    } else {
        ContentManifest tempMan;
        tempMan.internalName = FILE_NAMES::MOB_TYPE_ANIMATION;
        tempMan.pack = newDialog.pack;
        if(newDialog.mobTypePtr) {
            newDialog.animPath =
                game.content.mobAnimDbs.manifestToPath(
                    tempMan,
                    newDialog.mobTypePtr->category->folderName,
                    newDialog.mobTypePtr->manifest->internalName
                );
        }
    }
    if(newDialog.lastCheckedAnimPath != newDialog.animPath) {
        newDialog.animPathExists = fileExists(newDialog.animPath);
        newDialog.lastCheckedAnimPath = newDialog.animPath;
    }
    
    if(newDialog.type == 0) {
        if(newDialog.internalName.empty()) {
            problem = "You have to type an internal name first!";
        } else if(!isInternalNameGood(newDialog.internalName)) {
            problem =
                "The internal name should only have lowercase letters,\n"
                "numbers, and underscores!";
        } else {
            if(newDialog.animPathExists) {
                problem =
                    "There is already a global animation database\n"
                    "with that internal name in that pack!";
            }
        }
    } else {
        if(!newDialog.mobTypePtr) {
            problem = "You have to choose an object type first!";
        } else {
            if(newDialog.animPathExists) {
                problem =
                    "There is already an animation database\n"
                    "for that object type in that pack!";
            }
        }
    }
    
    //Create button.
    ImGui::Spacer();
    ImGui::SetupCentering(200);
    if(!problem.empty()) {
        ImGui::BeginDisabled();
    }
    if(ImGui::Button("Create animation database", ImVec2(200, 40))) {
        hitCreateButton = true;
    }
    if(!problem.empty()) {
        ImGui::EndDisabled();
    }
    setTooltip(problem.empty() ? "Create the animation database!" : problem);
    
    //Creation logic.
    if(hitCreateButton) {
        if(!problem.empty()) return;
        auto reallyCreate = [this] () {
            closeTopDialog();
            closeTopDialog(); //Close the load dialog.
            createAnimDb(newDialog.animPath);
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
void AnimationEditor::processGuiOptionsDialog() {
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
        
        //Drag threshold value.
        int dragThreshold = (int) game.options.editors.mouseDragThreshold;
        ImGui::SetNextItemWidth(64.0f);
        ImGui::DragInt(
            "Drag threshold", &dragThreshold,
            0.1f, 0, INT_MAX
        );
        setTooltip(
            "Cursor must move these many pixels to be considered a drag.\n"
            "Default: " + i2s(OPTIONS::EDITORS_D::MOUSE_DRAG_THRESHOLD) +
            ".",
            "", WIDGET_EXPLANATION_DRAG
        );
        game.options.editors.mouseDragThreshold = dragThreshold;
        
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
                game.options.animEd.bgPath.clear();
            }
        }
        setTooltip(
            "Check this to use a repeating texture on the background\n"
            "of the editor."
        );
        
        if(useBg) {
            ImGui::Indent();
            
            //Remove background texture button.
            unsigned char remBgOpacity =
                game.options.animEd.bgPath.empty() ? 50 : 255;
            if(
                ImGui::ImageButton(
                    "remBgButton", editorIcons[EDITOR_ICON_REMOVE],
                    Point(ImGui::GetTextLineHeight()), Point(), Point(1.0f),
                    COLOR_EMPTY, mapAlpha(remBgOpacity)
                )
            ) {
                game.options.animEd.bgPath.clear();
                if(bg) {
                    al_destroy_bitmap(bg);
                    bg = nullptr;
                }
            }
            setTooltip(
                "Remove the background image.\n"
                "This does not delete the file in your disk."
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
                    game.options.animEd.bgPath = f[0];
                    if(bg) {
                        al_destroy_bitmap(bg);
                        bg = nullptr;
                    }
                    bg =
                        loadBmp(
                            game.options.animEd.bgPath,
                            nullptr, false, false, false
                        );
                }
            }
            setTooltip(
                "Browse for which texture file in your disk to use."
            );
            
            //Background texture name text.
            string fileName =
                getPathLastComponent(game.options.animEd.bgPath);
            ImGui::SameLine();
            monoText("%s", fileName.c_str());
            setTooltip("Full path:\n" + game.options.animEd.bgPath);
            
            ImGui::Unindent();
        }
        
        ImGui::TreePop();
        
    }
}


/**
 * @brief Processes the Dear ImGui animation control panel for this frame.
 */
void AnimationEditor::processGuiPanelAnimation() {
    ImGui::BeginChild("animation");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("ANIMATIONS");
    
    processGuiPanelAnimationHeader();
    
    if(curAnimInst.curAnim) {
    
        //Animation data node.
        if(saveableTreeNode("animation", "Animation data")) {
            processGuiPanelAnimationData();
            ImGui::TreePop();
        }
        
        //Frames node.
        ImGui::Spacer();
        if(saveableTreeNode("animation", "Frames")) {
            Frame* framePtr = nullptr;
            if(
                curAnimInst.curFrameIdx == INVALID &&
                !curAnimInst.curAnim->frames.empty()
            ) {
                curAnimInst.curFrameIdx = 0;
                curAnimInst.curFrameTime = 0.0f;
            }
            if(curAnimInst.validFrame()) {
                framePtr =
                    &(curAnimInst.curAnim->frames[curAnimInst.curFrameIdx]);
            }
            
            processGuiPanelFrameHeader(framePtr);
            if(framePtr) {
                processGuiPanelFrame(framePtr);
            }
            
            ImGui::TreePop();
        }
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui animation control panel's animation
 * data for this frame.
 */
void AnimationEditor::processGuiPanelAnimationData() {
    //Loop frame value.
    int loopFrame = (int) curAnimInst.curAnim->loopFrame + 1;
    if(
        ImGui::DragInt(
            "Loop frame", &loopFrame, 0.1f, 1,
            curAnimInst.curAnim->frames.empty() ?
            1 :
            (int) curAnimInst.curAnim->frames.size()
        )
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "The animation loops back to this frame when it "
        "reaches the last one.",
        "", WIDGET_EXPLANATION_DRAG
    );
    loopFrame =
        std::clamp(
            loopFrame, 1,
            curAnimInst.curAnim->frames.empty() ?
            1 :
            (int) curAnimInst.curAnim->frames.size()
        );
    curAnimInst.curAnim->loopFrame = loopFrame - 1;
    
    //Hit rate slider.
    int hitRate = curAnimInst.curAnim->hitRate;
    if(ImGui::SliderInt("Hit rate", &hitRate, 0, 100)) {
        changesMgr.markAsChanged();
        curAnimInst.curAnim->hitRate = hitRate;
    }
    setTooltip(
        "If this attack can knock back Pikmin, this indicates "
        "the chance that it will hit.\n"
        "0 means it will always miss, 50 means it will hit "
        "half the time, etc.",
        "", WIDGET_EXPLANATION_SLIDER
    );
    
    //Animation information text.
    ImGui::TextDisabled("(Animation info)");
    string animInfoStr =
        "Total duration: " + f2s(curAnimInst.curAnim->getDuration()) + "s";
    if(curAnimInst.curAnim->loopFrame != 0) {
        animInfoStr +=
            "\nLoop segment duration: " +
            f2s(curAnimInst.curAnim->getLoopDuration()) + "s";
    }
    setTooltip(animInfoStr);
}


/**
 * @brief Processes the Dear ImGui animation control panel's animation
 * header for this frame.
 */
void AnimationEditor::processGuiPanelAnimationHeader() {
    //Current animation text.
    size_t curAnimIdx = INVALID;
    if(curAnimInst.curAnim) {
        curAnimIdx = db.findAnimation(curAnimInst.curAnim->name);
    }
    ImGui::Text(
        "Current animation: %s / %i",
        (curAnimIdx == INVALID ? "--" : i2s(curAnimIdx + 1).c_str()),
        (int) db.animations.size()
    );
    
    //Previous animation button.
    if(
        ImGui::ImageButton(
            "prevAnimButton", editorIcons[EDITOR_ICON_PREVIOUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.animations.empty()) {
            if(!curAnimInst.curAnim) {
                pickAnimation(db.animations[0]->name, "", "", nullptr, false);
            } else {
                size_t newIdx =
                    sumAndWrap(
                        (int) db.findAnimation(curAnimInst.curAnim->name),
                        -1,
                        (int) db.animations.size()
                    );
                pickAnimation(
                    db.animations[newIdx]->name, "", "", nullptr, false
                );
            }
        }
    }
    setTooltip(
        "Previous\nanimation."
    );
    
    //Change current animation button.
    string animButtonName =
        (
            curAnimInst.curAnim ?
            curAnimInst.curAnim->name :
            NONE_OPTION
        ) + "##anim";
    ImVec2 animButtonSize(
        -(EDITOR::ICON_BMP_SIZE + 16.0f), EDITOR::ICON_BMP_SIZE + 6.0f
    );
    ImGui::SameLine();
    if(monoButton(animButtonName.c_str(), animButtonSize)) {
        vector<PickerItem> animNames;
        for(size_t a = 0; a < db.animations.size(); a++) {
            ALLEGRO_BITMAP* animFrame1 = nullptr;
            if(!db.animations[a]->frames.empty()) {
                size_t sPos =
                    db.findSprite(
                        db.animations[a]->frames[0].spriteName
                    );
                if(sPos != INVALID) {
                    animFrame1 = db.sprites[sPos]->bitmap;
                }
            }
            animNames.push_back(
                PickerItem(
                    db.animations[a]->name, "", "", nullptr, "", animFrame1
                )
            );
        }
        openPickerDialog(
            "Pick an animation, or create a new one",
            animNames,
            std::bind(
                &AnimationEditor::pickAnimation, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5
            ),
            "", true, true
        );
    }
    setTooltip(
        "Pick an animation, or create a new one."
    );
    
    //Next animation button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "nextAnimButton", editorIcons[EDITOR_ICON_NEXT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.animations.empty()) {
            if(!curAnimInst.curAnim) {
                pickAnimation(db.animations[0]->name, "", "", nullptr, false);
            } else {
                size_t newIdx =
                    sumAndWrap(
                        (int) db.findAnimation(curAnimInst.curAnim->name),
                        1,
                        (int) db.animations.size()
                    );
                pickAnimation(
                    db.animations[newIdx]->name, "", "", nullptr, false
                );
            }
        }
    }
    setTooltip(
        "Next\nanimation."
    );
    
    ImGui::Spacer();
    
    if(curAnimInst.curAnim) {
    
        //Delete animation button.
        if(
            ImGui::ImageButton(
                "delAnimButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            string curAnimName = curAnimInst.curAnim->name;
            size_t nr = db.findAnimation(curAnimName);
            db.animations.erase(db.animations.begin() + nr);
            if(db.animations.empty()) {
                curAnimInst.clear();
            } else {
                nr = std::min(nr, db.animations.size() - 1);
                pickAnimation(db.animations[nr]->name, "", "", nullptr, false);
            }
            animPlaying = false;
            changesMgr.markAsChanged();
            setStatus("Deleted animation \"" + curAnimName + "\".");
        }
        setTooltip(
            "Delete the current animation."
        );
        
    }
    
    if(curAnimInst.curAnim) {
    
        if(db.animations.size() > 1) {
        
            //Import animation button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "importAnimButton", editorIcons[EDITOR_ICON_DUPLICATE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("importAnim");
            }
            setTooltip(
                "Import the data from another animation."
            );
            
            //Import animation popup.
            vector<string> importAnimNames;
            for(size_t a = 0; a < db.animations.size(); a++) {
                if(db.animations[a] == curAnimInst.curAnim) continue;
                importAnimNames.push_back(db.animations[a]->name);
            }
            string pickedAnim;
            if(
                listPopup("importAnim", importAnimNames, &pickedAnim, true)
            ) {
                importAnimationData(pickedAnim);
                setStatus(
                    "Imported animation data from \"" + pickedAnim + "\"."
                );
            }
            
        }
        
        //Rename animation button.
        static string renameAnimName;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "renameAnimButton", editorIcons[EDITOR_ICON_INFO],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicateString(curAnimInst.curAnim->name, renameAnimName);
            openInputPopup("renameAnim");
        }
        setTooltip(
            "Rename the current animation."
        );
        
        //Rename animation popup.
        if(
            processGuiInputPopup(
                "renameAnim", "New name:", &renameAnimName, true
            )
        ) {
            renameAnimation(curAnimInst.curAnim, renameAnimName);
        }
    }
}


/**
 * @brief Processes the Dear ImGui body part control panel for this frame.
 */
void AnimationEditor::processGuiPanelBodyPart() {
    ImGui::BeginChild("bodyPart");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("BODY PARTS");
    
    static string newPartName;
    static int selectedPart = 0;
    
    //Add body part button.
    if(
        ImGui::ImageButton(
            "addPartButton", editorIcons[EDITOR_ICON_ADD],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        newPartName.clear();
        openInputPopup("newPartName");
    }
    setTooltip(
        "Create a new body part."
        "It will be placed after the currently selected body part."
    );
    
    //Add body part popup.
    if(
        processGuiInputPopup(
            "newPartName", "New body part's name:", &newPartName, true
        )
    ) {
        if(!newPartName.empty()) {
            bool alreadyExists = false;
            for(size_t b = 0; b < db.bodyParts.size(); b++) {
                if(db.bodyParts[b]->name == newPartName) {
                    selectedPart = (int) b;
                    alreadyExists = true;
                }
            }
            if(!alreadyExists) {
                selectedPart = std::max(0, selectedPart);
                db.bodyParts.insert(
                    db.bodyParts.begin() + selectedPart +
                    (db.bodyParts.empty() ? 0 : 1),
                    new BodyPart(newPartName)
                );
                if(db.bodyParts.size() == 1) {
                    selectedPart = 0;
                } else {
                    selectedPart++;
                }
                updateHitboxes();
                changesMgr.markAsChanged();
                setStatus("Created body part \"" + newPartName + "\".");
                newPartName.clear();
            } else {
                setStatus(
                    "A body part by the name \"" + newPartName +
                    "\" already exists!",
                    true
                );
            }
        }
    }
    
    if(!db.bodyParts.empty()) {
    
        //Delete body part button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "delPartButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            if(selectedPart >= 0 && !db.bodyParts.empty()) {
                string deletedPartName =
                    db.bodyParts[selectedPart]->name;
                delete db.bodyParts[selectedPart];
                db.bodyParts.erase(
                    db.bodyParts.begin() + selectedPart
                );
                if(db.bodyParts.empty()) {
                    selectedPart = -1;
                } else if(selectedPart > 0) {
                    selectedPart--;
                }
                updateHitboxes();
                changesMgr.markAsChanged();
                setStatus(
                    "Deleted body part \"" + deletedPartName + "\"."
                );
            }
        }
        setTooltip(
            "Delete the currently selected body part from the list."
        );
        
        //Rename body part button.
        static string renamePartName;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "renamePartButton", editorIcons[EDITOR_ICON_INFO],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicateString(
                db.bodyParts[selectedPart]->name, renamePartName
            );
            openInputPopup("renamePart");
        }
        setTooltip(
            "Rename the current body part."
        );
        
        //Rename body part popup.
        if(
            processGuiInputPopup(
                "renamePart", "New name:", &renamePartName, true
            )
        ) {
            renameBodyPart(
                db.bodyParts[selectedPart], renamePartName
            );
        }
        
        //Body part list.
        if(
            ImGui::BeginChild(
                "partsList", ImVec2(0.0f, 80.0f), ImGuiChildFlags_Borders
            )
        ) {
        
            for(size_t p = 0; p < db.bodyParts.size(); p++) {
            
                //Body part selectable.
                bool isSelected = (p == (size_t) selectedPart);
                monoSelectable(
                    db.bodyParts[p]->name.c_str(), &isSelected
                );
                
                if(ImGui::IsItemActive()) {
                    selectedPart = (int) p;
                    if(!ImGui::IsItemHovered()) {
                        int p2 =
                            (int) p +
                            (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
                        if(p2 >= 0 && p2 < (int) db.bodyParts.size()) {
                            BodyPart* pPtr = db.bodyParts[p];
                            db.bodyParts[p] = db.bodyParts[p2];
                            db.bodyParts[p2] = pPtr;
                            ImGui::ResetMouseDragDelta();
                            updateHitboxes();
                            changesMgr.markAsChanged();
                        }
                    }
                }
                
            }
            
            ImGui::EndChild();
            
        }
        
    }
    
    if(db.bodyParts.size() > 1) {
    
        //Explanation text.
        ImGui::Spacer();
        ImGui::TextWrapped(
            "The higher on the list, the more priority that body "
            "part's hitboxes have when the game checks collisions. "
            "Drag and drop items in the list to sort them."
        );
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui animation control panel's frame info for this
 * frame.
 *
 * @param framePtr Pointer to the currently selected frame.
 */
void AnimationEditor::processGuiPanelFrame(Frame*& framePtr) {
    //Sprite combobox.
    vector<string> spriteNames;
    for(size_t s = 0; s < db.sprites.size(); s++) {
        spriteNames.push_back(db.sprites[s]->name);
    }
    if(
        monoCombo(
            "Sprite", &framePtr->spriteName, spriteNames, 15
        )
    ) {
        framePtr->spriteIdx =
            db.findSprite(framePtr->spriteName);
        framePtr->spritePtr =
            db.sprites[framePtr->spriteIdx];
        changesMgr.markAsChanged();
    }
    setTooltip(
        "The sprite to use for this frame."
    );
    
    //Duration value.
    if(
        ImGui::DragFloat(
            "Duration", &framePtr->duration, 0.0005, 0.0f, FLT_MAX
        )
    ) {
        curAnimInst.curFrameTime = 0.0f;
        changesMgr.markAsChanged();
    }
    setTooltip(
        "How long this frame lasts for, in seconds.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Interpolate checkbox.
    if(
        ImGui::Checkbox("Interpolate", &framePtr->interpolate)
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "If true, the transformation data (sprite translation,\n"
        "sprite rotation, etc.) on this frame will smoothly\n"
        "interpolate until it meets the transformation\n"
        "data of the next frame.\n"
        "This does not affect the bitmap or hitboxes."
    );
    
    //Signal checkbox.
    bool useSignal = (framePtr->signal != INVALID);
    if(ImGui::Checkbox("Signal", &useSignal)) {
        if(useSignal) {
            framePtr->signal = 0;
        } else {
            framePtr->signal = INVALID;
        }
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Whether a signal event should be sent to the script\n"
        "when this frame starts."
    );
    
    //Signal value.
    if(useSignal) {
        ImGui::SameLine();
        int fSignal = (int) framePtr->signal;
        if(
            ImGui::DragInt("##signal", &fSignal, 0.1, 0, INT_MAX)
        ) {
            changesMgr.markAsChanged();
            framePtr->signal = fSignal;
        }
        setTooltip(
            "Number of the signal.",
            "", WIDGET_EXPLANATION_DRAG
        );
    }
    
    if(loadedMobType) {
    
        //Sound checkbox.
        bool useSound = (!framePtr->sound.empty());
        if(ImGui::Checkbox("Sound", &useSound)) {
            if(useSound) {
                framePtr->sound = NONE_OPTION;
            } else {
                framePtr->sound.clear();
            }
            changesMgr.markAsChanged();
            db.fillSoundIdxCaches(loadedMobType);
        }
        setTooltip(
            "Whether a sound should play when this frame starts."
        );
        
        if(useSound) {
        
            //Sound combobox.
            ImGui::SameLine();
            vector<string> sounds = { NONE_OPTION };
            for(
                size_t s = 0;
                s < loadedMobType->sounds.size();
                s++
            ) {
                sounds.push_back(loadedMobType->sounds[s].name);
            }
            if(
                monoCombo(
                    "##sound",
                    &framePtr->sound,
                    sounds,
                    15
                )
            ) {
                db.fillSoundIdxCaches(loadedMobType);
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Name of the sound in the object's data."
            );
        }
        
    }
    
    //Apply duration to all button.
    ImGui::Spacer();
    if(ImGui::Button("Apply duration to all frames")) {
        float d =
            curAnimInst.curAnim->frames[
                curAnimInst.curFrameIdx
            ].duration;
        for(
            size_t i = 0;
            i < curAnimInst.curAnim->frames.size();
            i++
        ) {
            curAnimInst.curAnim->frames[i].duration = d;
        }
        curAnimInst.curFrameTime = 0.0f;
        changesMgr.markAsChanged();
        setStatus(
            "Applied the duration " + f2s(d) + " to all frames."
        );
    }
}


/**
 * @brief Processes the Dear ImGui animation control panel's frame
 * header for this frame.
 *
 * @param framePtr Pointer to the current frame.
 */
void AnimationEditor::processGuiPanelFrameHeader(Frame*& framePtr) {
    //Current frame text.
    ImGui::Text(
        "Current frame: %s / %i",
        framePtr ? i2s(curAnimInst.curFrameIdx + 1).c_str() : "--",
        (int) curAnimInst.curAnim->frames.size()
    );
    
    if(framePtr) {
        //Play/pause button.
        if(
            ImGui::ImageButton(
                "playButton", editorIcons[EDITOR_ICON_PLAY_PAUSE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            if(isShiftPressed) {
                restartAnimCmd(1.0f);
            } else {
                playPauseAnimCmd(1.0f);
            }
        }
        if(ImGui::BeginPopupContextItem()) {
            //From the beginning selectable.
            if(ImGui::Selectable("From the beginning")) {
                restartAnimCmd(1.0f);
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
        setTooltip(
            "Play or pause the animation.\n"
            "Hold Shift to start from the beginning.\n"
            "Right click for more options.",
            "Spacebar"
        );
        
        //Previous frame button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "prevFrameButton", editorIcons[EDITOR_ICON_PREVIOUS],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            animPlaying = false;
            if(!curAnimInst.curAnim->frames.empty()) {
                if(curAnimInst.curFrameIdx == INVALID) {
                    curAnimInst.curFrameIdx = 0;
                } else if(curAnimInst.curFrameIdx == 0) {
                    curAnimInst.curFrameIdx =
                        curAnimInst.curAnim->frames.size() - 1;
                } else {
                    curAnimInst.curFrameIdx--;
                }
                curAnimInst.curFrameTime = 0.0f;
            }
        }
        setTooltip(
            "Previous frame."
        );
        
        //Next frame button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "nextFrameButton", editorIcons[EDITOR_ICON_NEXT],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            animPlaying = false;
            if(!curAnimInst.curAnim->frames.empty()) {
                if(
                    curAnimInst.curFrameIdx ==
                    curAnimInst.curAnim->frames.size() - 1 ||
                    curAnimInst.curFrameIdx == INVALID
                ) {
                    curAnimInst.curFrameIdx = 0;
                } else {
                    curAnimInst.curFrameIdx++;
                }
                curAnimInst.curFrameTime = 0.0f;
            }
        }
        setTooltip(
            "Next frame."
        );
        
        ImGui::SameLine();
    }
    
    //Add frame button.
    if(
        ImGui::ImageButton(
            "addFrameButton", editorIcons[EDITOR_ICON_ADD],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(
            curAnimInst.curFrameIdx <
            curAnimInst.curAnim->loopFrame
        ) {
            //Let the loop frame stay the same.
            curAnimInst.curAnim->loopFrame++;
        }
        animPlaying = false;
        if(curAnimInst.curFrameIdx != INVALID) {
            curAnimInst.curFrameIdx++;
            curAnimInst.curFrameTime = 0.0f;
            curAnimInst.curAnim->frames.insert(
                curAnimInst.curAnim->frames.begin() +
                curAnimInst.curFrameIdx,
                Frame(
                    curAnimInst.curAnim->frames[
                        curAnimInst.curFrameIdx - 1
                    ]
                )
            );
        } else {
            curAnimInst.curAnim->frames.push_back(Frame());
            curAnimInst.curFrameIdx = 0;
            curAnimInst.curFrameTime = 0.0f;
            setBestFrameSprite();
        }
        framePtr =
            &(curAnimInst.curAnim->frames[curAnimInst.curFrameIdx]);
        changesMgr.markAsChanged();
        setStatus(
            "Added frame #" + i2s(curAnimInst.curFrameIdx + 1) + "."
        );
    }
    setTooltip(
        "Add a new frame after the current one, by copying "
        "data from the current one."
    );
    
    if(framePtr) {
    
        //Delete frame button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "delFrameButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            size_t deletedFrameIdx = curAnimInst.curFrameIdx;
            if(curAnimInst.curFrameIdx != INVALID) {
                curAnimInst.curAnim->deleteFrame(
                    curAnimInst.curFrameIdx
                );
            }
            if(curAnimInst.curAnim->frames.empty()) {
                curAnimInst.curFrameIdx = INVALID;
                framePtr = nullptr;
            } else if(
                curAnimInst.curFrameIdx >=
                curAnimInst.curAnim->frames.size()
            ) {
                curAnimInst.curFrameIdx =
                    curAnimInst.curAnim->frames.size() - 1;
                framePtr =
                    &(
                        curAnimInst.curAnim->frames[
                            curAnimInst.curFrameIdx
                        ]
                    );
            }
            animPlaying = false;
            curAnimInst.curFrameTime = 0.0f;
            changesMgr.markAsChanged();
            setStatus(
                "Deleted frame #" + i2s(deletedFrameIdx + 1) + "."
            );
        }
        setTooltip(
            "Delete the current frame."
        );
        
    }
}


/**
 * @brief Processes the Dear ImGui animation database info control panel
 * for this frame.
 */
void AnimationEditor::processGuiPanelInfo() {
    ImGui::BeginChild("info");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("INFO");
    
    //Name input.
    if(ImGui::InputText("Name", &db.name)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Name of this animation. Optional."
    );
    
    //Description input.
    if(ImGui::InputText("Description", &db.description)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Description of this animation. Optional."
    );
    
    //Version input.
    if(monoInputText("Version", &db.version)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Version of the database, preferably in the \"X.Y.Z\" format. "
        "Optional."
    );
    
    //Maker input.
    if(ImGui::InputText("Maker", &db.maker)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Name (or nickname) of who made this database. "
        "Optional."
    );
    
    //Maker notes input.
    if(ImGui::InputText("Maker notes", &db.makerNotes)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Extra notes or comments about the database for other makers to see. "
        "Optional."
    );
    
    //Notes input.
    if(ImGui::InputText("Notes", &db.notes)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Extra notes or comments of any kind. "
        "Optional."
    );
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui main control panel for this frame.
 */
void AnimationEditor::processGuiPanelMain() {
    if(manifest.internalName.empty()) return;
    
    ImGui::BeginChild("main");
    
    //Current database header text.
    ImGui::Text("Database: ");
    
    //Current database text.
    ImGui::SameLine();
    monoText(
        "%s",
        loadedMobType ?
        loadedMobType->manifest->internalName.c_str() :
        manifest.internalName.c_str()
    );
    string fileTooltip =
        getFileTooltip(manifest.path) + "\n\n"
        "File state: ";
    if(!changesMgr.existsOnDisk()) {
        fileTooltip += "Doesn't exit in your disk yet!";
    } else if(changesMgr.hasUnsavedChanges()) {
        fileTooltip += "You have unsaved changes.";
    } else {
        fileTooltip += "Everything ok.";
    }
    setTooltip(fileTooltip);
    
    //Animations button.
    ImGui::Spacer();
    if(
        ImGui::ImageButtonAndText(
            "animsButton", editorIcons[EDITOR_ICON_ANIMATIONS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Animations"
        )
    ) {
        if(!curAnimInst.curAnim && !db.animations.empty()) {
            pickAnimation(db.animations[0]->name, "", "", nullptr, false);
        }
        changeState(EDITOR_STATE_ANIMATION);
    }
    setTooltip(
        "Change the way the animations look like."
    );
    
    //Sprites button.
    if(
        ImGui::ImageButtonAndText(
            "spritesButton", editorIcons[EDITOR_ICON_SPRITES],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Sprites"
        )
    ) {
        if(!curSprite && !db.sprites.empty()) {
            curSprite = db.sprites[0];
        }
        changeState(EDITOR_STATE_SPRITE);
    }
    setTooltip(
        "Change how each individual sprite looks like."
    );
    
    //Body parts button.
    if(
        ImGui::ImageButtonAndText(
            "partsButton", editorIcons[EDITOR_ICON_BODY_PARTS],
            Point(EDITOR::ICON_BMP_SIZE),
            24.0f, "Body parts"
        )
    ) {
        changeState(EDITOR_STATE_BODY_PART);
    }
    setTooltip(
        "Change what body parts exist, and their order."
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
        "Set the animation database's information here, if you want."
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
        "Special tools to help with specific tasks."
    );
    
    //Stats node.
    ImGui::Spacer();
    if(saveableTreeNode("main", "Stats")) {
    
        //Animation amount text.
        ImGui::BulletText(
            "Animations: %i", (int) db.animations.size()
        );
        
        //Sprite amount text.
        ImGui::BulletText(
            "Sprites: %i", (int) db.sprites.size()
        );
        
        //Body part amount text.
        ImGui::BulletText(
            "Body parts: %i", (int) db.bodyParts.size()
        );
        
        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite control panel for this frame.
 */
void AnimationEditor::processGuiPanelSprite() {
    ImGui::BeginChild("sprite");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("SPRITES");
    
    //Current sprite text.
    size_t curSpriteIdx = INVALID;
    if(curSprite) {
        curSpriteIdx = db.findSprite(curSprite->name);
    }
    ImGui::Text(
        "Current sprite: %s / %i",
        (curSpriteIdx == INVALID ? "--" : i2s(curSpriteIdx + 1).c_str()),
        (int) db.sprites.size()
    );
    
    //Previous sprite button.
    if(
        ImGui::ImageButton(
            "prevSpriteButton", editorIcons[EDITOR_ICON_PREVIOUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.sprites.empty()) {
            if(!curSprite) {
                pickSprite(db.sprites[0]->name, "", "", nullptr, false);
            } else {
                size_t newIdx =
                    sumAndWrap(
                        (int) db.findSprite(curSprite->name),
                        -1,
                        (int) db.sprites.size()
                    );
                pickSprite(db.sprites[newIdx]->name, "", "", nullptr, false);
            }
        }
    }
    setTooltip(
        "Previous\nsprite."
    );
    
    //Change current sprite button.
    string spriteButtonName =
        (curSprite ? curSprite->name : NONE_OPTION) + "##sprite";
    ImVec2 spriteButtonSize(
        -(EDITOR::ICON_BMP_SIZE + 16.0f), EDITOR::ICON_BMP_SIZE + 6.0f
    );
    ImGui::SameLine();
    if(monoButton(spriteButtonName.c_str(), spriteButtonSize)) {
        vector<PickerItem> spriteNames;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            spriteNames.push_back(
                PickerItem(
                    db.sprites[s]->name,
                    "", "", nullptr, "",
                    db.sprites[s]->bitmap
                )
            );
        }
        openPickerDialog(
            "Pick a sprite, or create a new one",
            spriteNames,
            std::bind(
                &AnimationEditor::pickSprite, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5
            ),
            "", true, true
        );
    }
    setTooltip(
        "Pick a sprite, or create a new one."
    );
    
    //Next sprite button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "nextSpriteButton", editorIcons[EDITOR_ICON_NEXT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(!db.sprites.empty()) {
            if(!curSprite) {
                pickSprite(db.sprites[0]->name, "", "", nullptr, false);
            } else {
                size_t newIdx =
                    sumAndWrap(
                        (int) db.findSprite(curSprite->name),
                        1,
                        (int) db.sprites.size()
                    );
                pickSprite(db.sprites[newIdx]->name, "", "", nullptr, false);
            }
        }
    }
    setTooltip(
        "Next\nsprite."
    );
    
    ImGui::Spacer();
    
    if(curSprite) {
        //Delete sprite button.
        if(
            ImGui::ImageButton(
                "delSpriteButton", editorIcons[EDITOR_ICON_REMOVE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            string deletedSpriteName = curSprite->name;
            size_t nr = db.findSprite(deletedSpriteName);
            db.deleteSprite(nr);
            curAnimInst.curFrameIdx = 0;
            if(db.sprites.empty()) {
                curSprite = nullptr;
                curHitbox = nullptr;
                curHitboxIdx = INVALID;
            } else {
                nr = std::min(nr, db.sprites.size() - 1);
                pickSprite(db.sprites[nr]->name, "", "", nullptr, false);
            }
            changesMgr.markAsChanged();
            setStatus("Deleted sprite \"" + deletedSpriteName + "\".");
        }
        setTooltip(
            "Delete the current sprite.\n"
            "Any frame that makes use of this sprite\n"
            "will be deleted from its animation."
        );
    }
    
    if(curSprite) {
    
        if(db.sprites.size() > 1) {
        
            //Import sprite button.
            ImGui::SameLine();
            if(
                ImGui::ImageButton(
                    "importSpriteButton", editorIcons[EDITOR_ICON_DUPLICATE],
                    Point(EDITOR::ICON_BMP_SIZE)
                )
            ) {
                ImGui::OpenPopup("importSprite");
            }
            setTooltip(
                "Import the data from another sprite."
            );
            
            //Import sprite popup.
            vector<string> importSpriteNames;
            for(size_t s = 0; s < db.sprites.size(); s++) {
                if(db.sprites[s] == curSprite) continue;
                importSpriteNames.push_back(db.sprites[s]->name);
            }
            string pickedSprite;
            if(
                listPopup(
                    "importSprite", importSpriteNames, &pickedSprite, true
                )
            ) {
                importSpriteBmpData(pickedSprite);
                importSpriteTransformationData(pickedSprite);
                importSpriteHitboxData(pickedSprite);
                importSpriteTopData(pickedSprite);
                setStatus(
                    "Imported all sprite data from \"" + pickedSprite + "\"."
                );
            }
            
        }
        
        //Rename sprite button.
        static string renameSpriteName;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "renameSpriteButton", editorIcons[EDITOR_ICON_INFO],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            duplicateString(curSprite->name, renameSpriteName);
            openInputPopup("renameSprite");
        }
        setTooltip(
            "Rename the current sprite."
        );
        
        //Rename sprite popup.
        if(
            processGuiInputPopup(
                "renameSprite", "New name:", &renameSpriteName, true
            )
        ) {
            renameSprite(curSprite, renameSpriteName);
        }
        
        //Resize sprite button.
        static string resizeSpriteMult;
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "resizeSpriteButton", editorIcons[EDITOR_ICON_RESIZE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            resizeSpriteMult = "1.0";
            openInputPopup("resizeSprite");
        }
        setTooltip(
            "Resize the current sprite."
        );
        
        //Resize sprite popup.
        if(
            processGuiInputPopup(
                "resizeSprite", "Resize by:", &resizeSpriteMult
            )
        ) {
            resizeSprite(curSprite, s2f(resizeSpriteMult));
        }
        
        ImVec2 modeButtonsSize(-1.0f, 24.0f);
        
        //Sprite bitmap button.
        if(ImGui::Button("Bitmap", modeButtonsSize)) {
            preSpriteBmpCamPos = game.editorsView.cam.targetPos;
            preSpriteBmpCamZoom = game.editorsView.cam.targetZoom;
            centerCameraOnSpriteBitmap(true);
            changeState(EDITOR_STATE_SPRITE_BITMAP);
        }
        setTooltip(
            "Pick what part of an image makes up this sprite."
        );
        
        if(curSprite->bitmap) {
            //Sprite transformation button.
            if(ImGui::Button("Transformation", modeButtonsSize)) {
                changeState(EDITOR_STATE_SPRITE_TRANSFORM);
            }
            setTooltip(
                "Offset, scale, or rotate the sprite's image."
            );
        }
        
        if(!db.bodyParts.empty()) {
            //Sprite hitboxes button.
            if(ImGui::Button("Hitboxes", modeButtonsSize)) {
                if(curSprite && !curSprite->hitboxes.empty()) {
                    updateCurHitbox();
                    changeState(EDITOR_STATE_HITBOXES);
                }
            }
            setTooltip(
                "Edit this sprite's hitboxes."
            );
        }
        
        if(
            loadedMobType &&
            loadedMobType->category->id == MOB_CATEGORY_PIKMIN
        ) {
        
            //Sprite Pikmin top button.
            if(ImGui::Button("Pikmin top", modeButtonsSize)) {
                changeState(EDITOR_STATE_TOP);
            }
            setTooltip(
                "Edit the Pikmin's top (maturity) for this sprite."
            );
            
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite bitmap control panel for this frame.
 */
void AnimationEditor::processGuiPanelSpriteBitmap() {
    ImGui::BeginChild("spriteBitmap");
    
    //Back button.
    if(ImGui::Button("Back")) {
        game.editorsView.cam.setPos(preSpriteBmpCamPos);
        game.editorsView.cam.setZoom(preSpriteBmpCamZoom);
        changeState(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panelTitle("BITMAP");
    
    if(db.sprites.size() > 1) {
    
        //Import bitmap data button.
        if(
            ImGui::ImageButton(
                "importDataButton", editorIcons[EDITOR_ICON_DUPLICATE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteBitmap");
        }
        setTooltip(
            "Import the bitmap data from another sprite."
        );
        
        //Import bitmap popup.
        vector<string> importSpriteNames;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            if(db.sprites[s] == curSprite) continue;
            importSpriteNames.push_back(db.sprites[s]->name);
        }
        string pickedSprite;
        if(
            listPopup(
                "importSpriteBitmap", importSpriteNames, &pickedSprite, true
            )
        ) {
            importSpriteBmpData(pickedSprite);
            centerCameraOnSpriteBitmap(false);
            setStatus(
                "Imported bitmap data from \"" + pickedSprite + "\"."
            );
        }
        
    }
    
    //Choose spritesheet image button.
    ImGui::Spacer();
    if(ImGui::Button("Choose image...")) {
        openBitmapDialog(
        [this] (const string& bmp) {
            curSprite->setBitmap(
                bmp, curSprite->bmpPos, curSprite->bmpSize
            );
            lastSpritesheetUsed = bmp;
            centerCameraOnSpriteBitmap(true);
            changesMgr.markAsChanged();
            setStatus("Picked a spritesheet image successfully.");
        }
        );
    }
    setTooltip("Choose which spritesheet to use from the game's content.");
    
    //Spritesheet image name text.
    ImGui::SameLine();
    monoText("%s", curSprite->bmpName.c_str());
    setTooltip("Internal name:\n" + curSprite->bmpName);
    
    //Sprite top-left coordinates value.
    int topLeft[2] =
    { (int) curSprite->bmpPos.x, (int) curSprite->bmpPos.y };
    if(
        ImGui::DragInt2(
            "Top-left", topLeft, 0.05f, 0.0f, INT_MAX
        )
    ) {
        curSprite->setBitmap(
            curSprite->bmpName,
            Point(topLeft[0], topLeft[1]), curSprite->bmpSize
        );
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Top-left coordinates.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Sprite size value.
    int size[2] =
    { (int) curSprite->bmpSize.x, (int) curSprite->bmpSize.y };
    if(
        ImGui::DragInt2(
            "Size", size, 0.05f, 0.0f, INT_MAX
        )
    ) {
        curSprite->setBitmap(
            curSprite->bmpName,
            curSprite->bmpPos, Point(size[0], size[1])
        );
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Width and height.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Canvas explanation text.
    ImGui::Spacer();
    ImGui::TextWrapped(
        "Click parts of the image on the left to %s the selection limits.",
        spriteBmpAddMode ? "expand" : "set"
    );
    
    //Add to selection checkbox.
    ImGui::Checkbox("Add to selection", &spriteBmpAddMode);
    setTooltip(
        "Add to the existing selection instead of replacing it."
    );
    
    if(
        curSprite->bmpPos.x != 0.0f ||
        curSprite->bmpPos.y != 0.0f ||
        curSprite->bmpSize.x != 0.0f ||
        curSprite->bmpSize.y != 0.0f
    ) {
    
        //Clear selection button.
        if(
            ImGui::Button("Clear selection")
        ) {
            curSprite->bmpPos = Point();
            curSprite->bmpSize = Point();
            curSprite->setBitmap(
                curSprite->bmpName, curSprite->bmpPos, curSprite->bmpSize
            );
            changesMgr.markAsChanged();
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite hitboxes control panel for this frame.
 */
void AnimationEditor::processGuiPanelSpriteHitboxes() {
    ImGui::BeginChild("spriteHitboxes");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panelTitle("HITBOXES");
    
    //Hitbox name header text.
    ImGui::Text("Hitbox: ");
    
    //Hitbox name text.
    ImGui::SameLine();
    monoText(
        "%s",
        curHitbox ? curHitbox->bodyPartName.c_str() : NONE_OPTION.c_str()
    );
    
    //Previous hitbox button.
    if(
        ImGui::ImageButton(
            "prevHitboxButton", editorIcons[EDITOR_ICON_PREVIOUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(curSprite->hitboxes.size()) {
            if(!curHitbox) {
                curHitbox = &curSprite->hitboxes[0];
                curHitboxIdx = 0;
            } else {
                curHitboxIdx =
                    sumAndWrap(
                        (int) curHitboxIdx, -1,
                        (int) curSprite->hitboxes.size()
                    );
                curHitbox = &curSprite->hitboxes[curHitboxIdx];
            }
        }
    }
    setTooltip(
        "Select the previous hitbox."
    );
    
    //Next hitbox button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "nextHitboxButton", editorIcons[EDITOR_ICON_NEXT],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        if(curSprite->hitboxes.size()) {
            if(curHitboxIdx == INVALID) {
                curHitbox = &curSprite->hitboxes[0];
                curHitboxIdx = 0;
            } else {
                curHitboxIdx =
                    sumAndWrap(
                        (int) curHitboxIdx, 1,
                        (int) curSprite->hitboxes.size()
                    );
                curHitbox = &curSprite->hitboxes[curHitboxIdx];
            }
        }
    }
    setTooltip(
        "Select the next hitbox."
    );
    
    if(curHitbox && db.sprites.size() > 1) {
    
        //Import hitbox data button.
        ImGui::SameLine();
        if(
            ImGui::ImageButton(
                "importDataButton", editorIcons[EDITOR_ICON_DUPLICATE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteHitboxes");
        }
        setTooltip(
            "Import the hitbox data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> importSpriteNames;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            if(db.sprites[s] == curSprite) continue;
            importSpriteNames.push_back(db.sprites[s]->name);
        }
        string pickedSprite;
        if(
            listPopup(
                "importSpriteHitboxes", importSpriteNames,
                &pickedSprite, true
            )
        ) {
            importSpriteHitboxData(pickedSprite);
            setStatus(
                "Imported hitbox data from \"" + pickedSprite + "\"."
            );
        }
        
    }
    
    //Side view checkbox.
    ImGui::Spacer();
    ImGui::Checkbox("Use side view", &sideView);
    setTooltip(
        "Use a side view of the object, so you can adjust hitboxes "
        "horizontally."
    );
    
    if(curHitbox) {
        //Hitbox center value.
        if(ImGui::DragFloat2("Center", (float*) &curHitbox->pos, 0.05f)) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "X and Y coordinates of the center.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Hitbox radius value.
        if(
            ImGui::DragFloat(
                "Radius", &curHitbox->radius, 0.05f, 0.001f, FLT_MAX
            )
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Radius of the hitbox.",
            "", WIDGET_EXPLANATION_DRAG
        );
        curHitbox->radius =
            std::max(ANIM_EDITOR::HITBOX_MIN_RADIUS, curHitbox->radius);
            
        //Hitbox Z value.
        if(ImGui::DragFloat("Z", &curHitbox->z, 0.1f)) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Altitude of the hitbox's bottom.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        if(
            ImGui::DragFloat("Height", &curHitbox->height, 0.1f, 0.0f, FLT_MAX)
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Hitbox's height. 0 = spans infinitely vertically.",
            "", WIDGET_EXPLANATION_DRAG
        );
        curHitbox->height = std::max(0.0f, curHitbox->height);
        
        //Hitbox type text.
        ImGui::Spacer();
        ImGui::Text("Hitbox type:");
        
        //Normal hitbox radio button.
        int typeInt = curHitbox->type;
        if(ImGui::RadioButton("Normal", &typeInt, HITBOX_TYPE_NORMAL)) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Normal hitbox, one that can be damaged."
        );
        
        //Attack hitbox radio button.
        if(ImGui::RadioButton("Attack", &typeInt, HITBOX_TYPE_ATTACK)) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Attack hitbox, one that damages opponents."
        );
        
        //Disabled hitbox radio button.
        if(ImGui::RadioButton("Disabled", &typeInt, HITBOX_TYPE_DISABLED)) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Disabled hitbox, one that cannot be interacted with."
        );
        curHitbox->type = (HITBOX_TYPE) typeInt;
        
        ImGui::Indent();
        
        switch(curHitbox->type) {
        case HITBOX_TYPE_NORMAL: {
    
            //Defense multiplier value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat("Defense multiplier", &curHitbox->value, 0.01)
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Opponent attacks will have their damage divided "
                "by this amount.\n"
                "0 = invulnerable.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Pikmin latch checkbox.
            if(
                ImGui::Checkbox(
                    "Pikmin can latch", &curHitbox->canPikminLatch
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Can the Pikmin latch on to this hitbox?"
            );
            
            //Hazard.
            processGuiHitboxHazards();
            
            break;
        } case HITBOX_TYPE_ATTACK: {
    
            //Power value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat("Power", &curHitbox->value, 0.01)
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "Attack power, in hit points.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Outward knockback checkbox.
            if(
                ImGui::Checkbox(
                    "Outward knockback", &curHitbox->knockbackOutward
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "If true, opponents are knocked away from the hitbox's center."
            );
            
            //Knockback angle value.
            if(!curHitbox->knockbackOutward) {
                curHitbox->knockbackAngle =
                    normalizeAngle(curHitbox->knockbackAngle);
                ImGui::SetNextItemWidth(128.0f);
                if(
                    ImGui::SliderAngleWithContext(
                        "Knockback angle", &curHitbox->knockbackAngle,
                        0.0f, 360.0f, "%.2f"
                    )
                ) {
                    changesMgr.markAsChanged();
                }
                setTooltip(
                    "Angle to knock away towards.",
                    "", WIDGET_EXPLANATION_SLIDER
                );
            }
            
            //Knockback strength value.
            ImGui::SetNextItemWidth(128.0f);
            if(
                ImGui::DragFloat(
                    "Knockback value", &curHitbox->knockback, 0.01
                )
            ) {
                changesMgr.markAsChanged();
            }
            setTooltip(
                "How strong the knockback is. 3 is a good value.",
                "", WIDGET_EXPLANATION_DRAG
            );
            
            //Wither chance value.
            int witherChanceInt = curHitbox->witherChance;
            ImGui::SetNextItemWidth(128.0f);
            if(ImGui::SliderInt("Wither chance", &witherChanceInt, 0, 100)) {
                changesMgr.markAsChanged();
                curHitbox->witherChance = witherChanceInt;
            }
            setTooltip(
                "Chance of the attack lowering a Pikmin's maturity by one.",
                "", WIDGET_EXPLANATION_SLIDER
            );
            
            //Hazard.
            processGuiHitboxHazards();
            
            break;
            
        }  case HITBOX_TYPE_DISABLED: {
            break;
        }
        }
        
        ImGui::Unindent();
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite top control panel for this frame.
 */
void AnimationEditor::processGuiPanelSpriteTop() {
    ImGui::BeginChild("spriteTop");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panelTitle("TOP");
    
    if(db.sprites.size() > 1) {
    
        //Import top data button.
        if(
            ImGui::ImageButton(
                "importDataButton", editorIcons[EDITOR_ICON_DUPLICATE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteTop");
        }
        setTooltip(
            "Import the top data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> importSpriteNames;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            if(db.sprites[s] == curSprite) continue;
            importSpriteNames.push_back(db.sprites[s]->name);
        }
        string pickedSprite;
        if(
            listPopup(
                "importSpriteTop", importSpriteNames, &pickedSprite, true
            )
        ) {
            importSpriteTopData(pickedSprite);
            setStatus(
                "Imported Pikmin top data from \"" + pickedSprite + "\"."
            );
        }
        
    }
    
    //Visible checkbox.
    ImGui::Spacer();
    if(ImGui::Checkbox("Visible", &curSprite->topVisible)) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Is the top visible in this sprite?"
    );
    
    if(curSprite->topVisible) {
    
        //Top center value.
        if(
            ImGui::DragFloat2("Center", (float*) &curSprite->topPos, 0.05f)
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Center coordinates.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Top size value.
        if(
            processGuiSizeWidgets(
                "Size", curSprite->topSize, 0.01f,
                topKeepAspectRatio, false, ANIM_EDITOR::TOP_MIN_SIZE
            )
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Width and height.",
            "", WIDGET_EXPLANATION_DRAG
        );
        
        //Keep aspect ratio checkbox.
        ImGui::Indent();
        ImGui::Checkbox("Keep aspect ratio", &topKeepAspectRatio);
        ImGui::Unindent();
        setTooltip("Keep the aspect ratio when resizing the top.");
        
        
        //Top angle value.
        curSprite->topAngle = normalizeAngle(curSprite->topAngle);
        if(
            ImGui::SliderAngleWithContext(
                "Angle", &curSprite->topAngle, 0.0f, 360.0f, "%.2f"
            )
        ) {
            changesMgr.markAsChanged();
        }
        setTooltip(
            "Angle.",
            "", WIDGET_EXPLANATION_SLIDER
        );
        
        //Toggle maturity button.
        ImGui::Spacer();
        if(ImGui::Button("Toggle maturity")) {
            curMaturity = sumAndWrap(curMaturity, 1, N_MATURITIES);
        }
        setTooltip(
            "View a different maturity top."
        );
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui sprite transform control panel for
 * this frame.
 */
void AnimationEditor::processGuiPanelSpriteTransform() {
    ImGui::BeginChild("spriteTransform");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_SPRITE);
    }
    
    //Panel title text.
    panelTitle("TRANSFORM");
    
    if(db.sprites.size() > 1) {
    
        //Import transformation data button.
        if(
            ImGui::ImageButton(
                "importDataButton", editorIcons[EDITOR_ICON_DUPLICATE],
                Point(EDITOR::ICON_BMP_SIZE)
            )
        ) {
            ImGui::OpenPopup("importSpriteTransform");
        }
        setTooltip(
            "Import the transformation data from another sprite."
        );
        
        //Import sprite popup.
        vector<string> importSpriteNames;
        for(size_t s = 0; s < db.sprites.size(); s++) {
            if(db.sprites[s] == curSprite) continue;
            importSpriteNames.push_back(db.sprites[s]->name);
        }
        string pickedSprite;
        if(
            listPopup(
                "importSpriteTransform", importSpriteNames,
                &pickedSprite, true
            )
        ) {
            importSpriteTransformationData(pickedSprite);
            setStatus(
                "Imported transformation data from \"" + pickedSprite + "\"."
            );
        }
        
    }
    
    //Sprite offset value.
    ImGui::Spacer();
    if(
        ImGui::DragFloat2("Offset", (float*) &curSprite->offset, 0.05f)
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "X and Y offset.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Sprite scale value.
    if(
        processGuiSizeWidgets(
            "Scale", curSprite->scale,
            0.005f, curSpriteKeepAspectRatio, curSpriteKeepArea,
            -FLT_MAX
        )
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Horizontal and vertical scale.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Sprite flip X button.
    ImGui::Indent();
    if(
        ImGui::Button("Flip X")
    ) {
        curSprite->scale.x *= -1.0f;
        changesMgr.markAsChanged();
    }
    
    //Sprite flip Y button.
    ImGui::SameLine();
    if(
        ImGui::Button("Flip Y")
    ) {
        curSprite->scale.y *= -1.0f;
        changesMgr.markAsChanged();
    }
    
    //Keep aspect ratio checkbox.
    if(ImGui::Checkbox("Keep aspect ratio", &curSpriteKeepAspectRatio)) {
        curSpriteKeepArea = false;
    }
    setTooltip("Keep the aspect ratio when resizing the sprite.");
    
    //Keep area checkbox.
    if(ImGui::Checkbox("Keep area", &curSpriteKeepArea)) {
        curSpriteKeepAspectRatio = false;
    };
    ImGui::Unindent();
    setTooltip(
        "Keeps the same total area when resizing the sprite.\n"
        "Useful for squash and stretch effects."
    );
    
    //Sprite angle value.
    curSprite->angle = normalizeAngle(curSprite->angle);
    if(
        ImGui::SliderAngleWithContext(
            "Angle", &curSprite->angle, 0.0f, 360.0f, "%.2f"
        )
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip(
        "Angle.",
        "", WIDGET_EXPLANATION_SLIDER
    );
    
    //Sprite tint color.
    if(
        ImGui::ColorEdit4(
            "Tint color", (float*) &curSprite->tint,
            ImGuiColorEditFlags_NoInputs
        )
    ) {
        changesMgr.markAsChanged();
    }
    setTooltip("Color to tint it by. White makes it look normal.");
    
    ImGui::Spacer();
    
    if(db.sprites.size() > 1) {
    
        //Comparison sprite node.
        if(saveableTreeNode("transformation", "Comparison sprite")) {
        
            //Use comparison checkbox.
            ImGui::Checkbox("Use comparison", &comparison);
            setTooltip(
                "Show another sprite, to help you align and scale this one.",
                "Ctrl + C"
            );
            
            if(comparison) {
            
                //Comparison sprite combobox.
                vector<string> allSprites;
                for(size_t s = 0; s < db.sprites.size(); s++) {
                    if(curSprite == db.sprites[s]) continue;
                    allSprites.push_back(db.sprites[s]->name);
                }
                static string comparisonSpriteName;
                monoCombo(
                    "Sprite", &comparisonSpriteName, allSprites, 15
                );
                setTooltip(
                    "Choose another sprite to serve as a comparison."
                );
                size_t comparisonSpriteIdx =
                    db.findSprite(comparisonSpriteName);
                if(comparisonSpriteIdx != INVALID) {
                    comparisonSprite = db.sprites[comparisonSpriteIdx];
                } else {
                    comparisonSprite = nullptr;
                }
                
                //Comparison blinks checkbox.
                ImGui::Checkbox("Blink comparison", &comparisonBlink);
                setTooltip(
                    "Blink the comparison in and out?"
                );
                
                //Comparison above checkbox.
                ImGui::Checkbox("Comparison above", &comparisonAbove);
                setTooltip(
                    "Should the comparison appear above or below the working "
                    "sprite?"
                );
                
                //Tint both checkbox.
                ImGui::Checkbox("Tint both", &comparisonTint);
                setTooltip(
                    "Tint the working sprite blue, and the comparison "
                    "sprite orange."
                );
                
            }
            
            ImGui::TreePop();
            
        }
        
    }
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui tools control panel for this frame.
 */
void AnimationEditor::processGuiPanelTools() {
    ImGui::BeginChild("tools");
    
    //Back button.
    if(ImGui::Button("Back")) {
        changeState(EDITOR_STATE_MAIN);
    }
    
    //Panel title text.
    panelTitle("TOOLS");
    
    //Resize everything value.
    static float resizeMult = 1.0f;
    ImGui::SetNextItemWidth(96.0f);
    ImGui::DragFloat("##resizeMult", &resizeMult, 0.01);
    setTooltip(
        "Resize multiplier.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Resize everything button.
    ImGui::SameLine();
    if(ImGui::Button("Resize everything")) {
        resizeEverything(resizeMult);
        resizeMult = 1.0f;
    }
    setTooltip(
        "Resize everything by the given multiplier.\n"
        "0.5 resizes everything to half size, 2.0 to double, etc."
    );
    
    //Set sprite scales value.
    static float scalesValue = 1.0f;
    ImGui::SetNextItemWidth(96.0f);
    ImGui::DragFloat("##scalesValue", &scalesValue, 0.01);
    setTooltip(
        "Scales value.",
        "", WIDGET_EXPLANATION_DRAG
    );
    
    //Set sprite scales button.
    ImGui::SameLine();
    if(ImGui::Button("Set all scales")) {
        setAllSpriteScales(scalesValue);
    }
    setTooltip(
        "Set the X/Y scales of all sprites to the given value."
    );
    
    ImGui::EndChild();
}


/**
 * @brief Processes the Dear ImGui status bar for this frame.
 */
void AnimationEditor::processGuiStatusBar() {
    //Status bar text.
    processGuiStatusBarText();
    
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        canvasSeparatorX - ImGui::GetItemRectSize().x -
        EDITOR::MOUSE_COORDS_TEXT_WIDTH;
    ImGui::Dummy(ImVec2(size, 0));
    
    bool showingCoords = false;
    bool showingTime = false;
    float curTime = 0.0f;
    
    //Mouse coordinates text.
    if(
        (!isMouseInGui || isM1Pressed) &&
        !isCursorInTimeline() && !animPlaying &&
        state != EDITOR_STATE_SPRITE_BITMAP &&
        (state != EDITOR_STATE_HITBOXES || !sideView)
    ) {
        showingCoords = true;
        ImGui::SameLine();
        monoText(
            "%s, %s",
            boxString(f2s(game.editorsView.cursorWorldPos.x), 7).c_str(),
            boxString(f2s(game.editorsView.cursorWorldPos.y), 7).c_str()
        );
    }
    
    if(
        !showingCoords &&
        state == EDITOR_STATE_ANIMATION &&
        curAnimInst.validFrame()
    ) {
        if(isCursorInTimeline()) {
            curTime = getCursorTimelineTime();
        } else {
            curTime =
                curAnimInst.curAnim->getTime(
                    curAnimInst.curFrameIdx, curAnimInst.curFrameTime
                );
        }
        
        showingTime = true;
    }
    
    //Animation time text.
    if(showingTime) {
        ImGui::SameLine();
        monoText(
            "%ss",
            boxString(f2s(curTime), 7).c_str()
        );
    }
    
    
}


/**
 * @brief Processes the Dear ImGui toolbar for this frame.
 */
void AnimationEditor::processGuiToolbar() {
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
        "Quit the animation editor.",
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
        "Pick a database to load.",
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
        "Save the animation database to your disk.",
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
    
    //Toggle hitboxes button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "hitboxesButton", editorIcons[EDITOR_ICON_HITBOXES],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        hitboxesToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of the hitboxes, if any.",
        "Ctrl + H"
    );
    
    //Toggle mob radius button.
    ImGui::SameLine();
    if(
        ImGui::ImageButton(
            "mobRadiusButton", editorIcons[EDITOR_ICON_MOB_RADIUS],
            Point(EDITOR::ICON_BMP_SIZE)
        )
    ) {
        mobRadiusToggleCmd(1.0f);
    }
    setTooltip(
        "Toggle visibility of the mob's radius, if applicable.",
        "Ctrl + R"
    );
    
    //Toggle leader silhouette button.
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
}
