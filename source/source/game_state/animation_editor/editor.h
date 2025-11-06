/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general animation editor-related functions.
 */

#pragma once

#include <string>

#include "../../lib/imgui/imgui_impl_allegro5.h"
#include "../../util/general_utils.h"
#include "../editor.h"


namespace ANIM_EDITOR {
extern const float FLOOD_FILL_ALPHA_THRESHOLD;
extern const float GRID_INTERVAL;
extern const float HITBOX_MIN_RADIUS;
extern const float KEYBOARD_PAN_AMOUNT;
extern const size_t TIMELINE_HEADER_HEIGHT;
extern const size_t TIMELINE_HEIGHT;
extern const size_t TIMELINE_LOOP_TRI_SIZE;
extern const size_t TIMELINE_PADDING;
extern const float TOP_MIN_SIZE;
extern const float ZOOM_MAX_LEVEL;
extern const float ZOOM_MIN_LEVEL;
}


/**
 * @brief Info about the animation editor.
 */
class AnimationEditor : public Editor {

public:

    //--- Members ---
    
    //Automatically load this file upon boot-up of the editor, if any.
    string autoLoadFile;
    
    
    //--- Function declarations ---
    
    AnimationEditor();
    void doLogic() override;
    void doDrawing() override;
    void load() override;
    void unload() override;
    string getName() const override;
    string getNameForHistory() const;
    void drawCanvas();
    string getOpenedContentPath() const;
    
private:

    //--- Misc. declarations ---
    
    //Editor states.
    enum EDITOR_STATE {
    
        //Main menu.
        EDITOR_STATE_MAIN,
        
        //Animation editing.
        EDITOR_STATE_ANIMATION,
        
        //Sprite editing.
        EDITOR_STATE_SPRITE,
        
        //Body part editing.
        EDITOR_STATE_BODY_PART,
        
        //Hitbox editing.
        EDITOR_STATE_HITBOXES,
        
        //Sprite bitmap editing.
        EDITOR_STATE_SPRITE_BITMAP,
        
        //Sprite transformations editing.
        EDITOR_STATE_SPRITE_TRANSFORM,
        
        //Top editing.
        EDITOR_STATE_TOP,
        
        //Info.
        EDITOR_STATE_INFO,
        
        //Tools.
        EDITOR_STATE_TOOLS,
        
    };
    
    
    //--- Members ---
    
    //Currently loaded animation database.
    AnimationDatabase db;
    
    //Is the current animation playing?
    bool animPlaying = false;
    
    //Whether to use a background texture, if any.
    ALLEGRO_BITMAP* bg = nullptr;
    
    //Is the sprite comparison mode on?
    bool comparison = false;
    
    //Is the comparison sprite above the working sprite?
    bool comparisonAbove = true;
    
    //Is the comparison sprite meant to blink?
    bool comparisonBlink = true;
    
    //Is the blinking comparison sprite currently visible?
    bool comparisonBlinkShow = true;
    
    //Time left until the blinking comparison sprite's visibility is swapped.
    Timer comparisonBlinkTimer;
    
    //Comparison sprite to use in sprite comparison mode.
    Sprite* comparisonSprite = nullptr;
    
    //Is the comparison sprite mode tinting the sprites?
    bool comparisonTint = true;
    
    //Animation instance, for when the user is editing animations.
    AnimationInstance curAnimInst;
    
    //Current hitbox.
    Hitbox* curHitbox = nullptr;
    
    //The alpha is calculated using the sine of this value.
    float curHitboxAlpha = 0.0f;
    
    //Index number of the current hitbox.
    size_t curHitboxIdx = INVALID;
    
    //Current maturity to display on the Pikmin's top.
    unsigned char curMaturity = 0;
    
    //Current sprite, for when the user is editing sprites.
    Sprite* curSprite = nullptr;
    
    //Keep the aspect ratio when resizing the current sprite?
    bool curSpriteKeepAspectRatio = true;
    
    //Keep the total area when resizing the current sprite?
    bool curSpriteKeepArea = false;
    
    //The current transformation widget.
    TransformationWidget curTransformationWidget;
    
    //Is the grid visible?
    bool gridVisible = true;
    
    //Are the hitboxes currently visible?
    bool hitboxesVisible = true;
    
    //Last file used as for a spritesheet.
    string lastSpritesheetUsed;
    
    //Picker info for the picker in the "load" dialog.
    Picker loadDialogPicker;
    
    //Mob type of the currently loaded animation database, if any.
    MobType* loadedMobType = nullptr;
    
    //Is the mob radius visible?
    bool mobRadiusVisible = false;
    
    //Is the leader silhouette visible?
    bool leaderSilhouetteVisible = false;
    
    //Before entering the sprite bitmap state, this was the camera position.
    Point preSpriteBmpCamPos;
    
    //Before entering the sprite bitmap state, this was the camera zoom.
    float preSpriteBmpCamZoom = 1.0f;
    
    //When entering the sprite bitmap state, this was the bitmap position
    //of the current sprite.
    Point matchingSpriteBmpPos;
    
    //When entering the sprite bitmap state, this was the bitmap size
    //of the current sprite.
    Point matchingSpriteBmpSize;
    
    //Is side view on?
    bool sideView = false;
    
    //Is the add mode on in the sprite bitmap state?
    bool spriteBmpAddMode = false;
    
    //Top bitmaps for the current Pikmin type.
    ALLEGRO_BITMAP* topBmp[N_MATURITIES] = { nullptr, nullptr, nullptr };
    
    //Keep the aspect ratio when resizing the Pikmin top?
    bool topKeepAspectRatio = true;
    
    //Animation sounds being played.
    vector<size_t> animSoundIds;
    
    //Whether to use a background texture.
    bool useBg = false;
    
    //Position of the load widget.
    Point loadWidgetPos;
    
    //Position of the reload widget.
    Point reloadWidgetPos;
    
    //Position of the quit widget.
    Point quitWidgetPos;
    
    //Info about the "new" dialog.
    struct {
    
        //Selected pack.
        string pack;
        
        //Selected animation database type.
        int type = 0;
        
        //Selected custom mob category, when picking a mob type.
        string customMobCat;
        
        //Selected mob type, when picking a mob type.
        MobType* mobTypePtr = nullptr;
        
        //Internal name of the new animation database.
        string internalName = "my_animation";
        
        //Path to the new animation database.
        string animPath;
        
        //Last time we checked if the new database path existed, it was this.
        string lastCheckedAnimPath;
        
        //Does a file already exist under the new animation database's path?
        bool animPathExists = false;
        
        //Whether we need to focus on the text input widget.
        bool needsTextFocus = true;
        
    } newDialog;
    
    
    //--- Function declarations ---
    
    void applyChangesToAllMatchingSprites(
        const Point& oldPos, const Point& oldSize,
        const Point& newPos, const Point& newSize
    );
    void centerCameraOnSpriteBitmap(bool instant);
    void changeState(const EDITOR_STATE newState);
    void closeLoadDialog();
    void closeOptionsDialog();
    void createAnimDb(const string& path);
    void deleteCurrentAnimDb();
    float getCursorTimelineTime();
    string getFileTooltip(const string& path) const;
    void handleLmbDragInTimeline();
    void importAnimationData(const string& name);
    void importSpriteBmpData(const string& name);
    void importSpriteHitboxData(const string& name);
    void importSpriteTopData(const string& name);
    void importSpriteTransformationData(const string& name);
    bool isCursorInTimeline();
    void loadAnimDbFile(
        const string& path, bool shouldUpdateHistory
    );
    void pickAnimDbFile(
        const string& name, const string& topCat, const string& secCat,
        void* info, bool isNew
    );
    void playSound(size_t soundIdx);
    void reloadAnimDbs();
    void renameAnimation(Animation* anim, const string& newName);
    void renameBodyPart(BodyPart* part, const string& newName);
    void renameSprite(Sprite* spr, const string& newName);
    void resizeEverything(float mult);
    void resizeSprite(Sprite* s, float mult);
    bool saveAnimDb();
    void setupForNewAnimDbPost();
    void setupForNewAnimDbPre();
    void setAllSpriteScales(float scale);
    void setBestFrameSprite();
    void spriteBmpFloodFill(
        ALLEGRO_BITMAP* bmp, bool* selectionPixels, int x, int y
    );
    void stopSounds();
    void updateCurHitbox();
    void updateHitboxes();
    static void drawCanvasDearImGuiCallback(
        const ImDrawList* parentList, const ImDrawCmd* cmd
    );
    void drawComparison();
    void drawSideViewHitbox(
        Hitbox* hPtr, const ALLEGRO_COLOR& color,
        const ALLEGRO_COLOR& outlineColor, float outlineThickness
    );
    void drawSideViewLeaderSilhouette(float xOffset);
    void drawSideViewSprite(const Sprite* s);
    void drawTimeline();
    void drawTopDownViewHitbox(
        Hitbox* hPtr, const ALLEGRO_COLOR& color,
        const ALLEGRO_COLOR& outlineColor, float outlineThickness
    );
    void drawTopDownViewLeaderSilhouette(float xOffset);
    void drawTopDownViewMobRadius(MobType* mt);
    void drawTopDownViewSprite(Sprite* s);
    void openLoadDialog();
    void openNewDialog();
    void openOptionsDialog();
    void pickAnimation(
        const string& name, const string& topCat, const string& secCat,
        void* info, bool isNew
    );
    void pickSprite(
        const string& name, const string& topCat, const string& secCat,
        void* info, bool isNew
    );
    void deleteAnimDbCmd(float inputValue);
    void gridToggleCmd(float inputValue);
    void hitboxesToggleCmd(float inputValue);
    void leaderSilhouetteToggleCmd(float inputValue);
    void loadCmd(float inputValue);
    void mobRadiusToggleCmd(float inputValue);
    void openExternallyCmd(float inputValue);
    void playPauseAnimCmd(float inputValue);
    void quickPlayCmd(float inputValue);
    void quitCmd(float inputValue);
    void reloadCmd(float inputValue);
    void restartAnimCmd(float inputValue);
    void saveCmd(float inputValue);
    void zoomAndPosResetCmd(float inputValue);
    void zoomEverythingCmd(float inputValue);
    void zoomInCmd(float inputValue);
    void zoomOutCmd(float inputValue);
    void processGui();
    void processGuiControlPanel();
    void processGuiDeleteAnimDbDialog();
    void processGuiHitboxHazards();
    void processGuiLoadDialog();
    void processGuiNewDialog();
    void processGuiOptionsDialog();
    void processGuiPanelAnimation();
    void processGuiPanelAnimationData();
    void processGuiPanelAnimationHeader();
    void processGuiPanelBodyPart();
    void processGuiPanelFrame(Frame*& framePtr);
    void processGuiPanelFrameHeader(Frame*& framePtr);
    void processGuiPanelInfo();
    void processGuiPanelMain();
    void processGuiPanelSprite();
    void processGuiPanelSpriteBitmap();
    void processGuiPanelSpriteHitboxes();
    void processGuiPanelSpriteTop();
    void processGuiPanelSpriteTransform();
    void processGuiPanelTools();
    void processGuiMenuBar();
    void processGuiStatusBar();
    void processGuiToolbar();
    void handleKeyCharCanvas(const ALLEGRO_EVENT& ev) override;
    void handleKeyDownAnywhere(const ALLEGRO_EVENT& ev) override;
    void handleKeyDownCanvas(const ALLEGRO_EVENT& ev) override;
    void handleLmbDoubleClick(const ALLEGRO_EVENT& ev) override;
    void handleLmbDown(const ALLEGRO_EVENT& ev) override;
    void handleLmbDrag(const ALLEGRO_EVENT& ev) override;
    void handleLmbUp(const ALLEGRO_EVENT& ev) override;
    void handleMmbDoubleClick(const ALLEGRO_EVENT& ev) override;
    void handleMmbDown(const ALLEGRO_EVENT& ev) override;
    void handleMmbDrag(const ALLEGRO_EVENT& ev) override;
    void handleMouseUpdate(const ALLEGRO_EVENT& ev) override;
    void handleMouseWheel(const ALLEGRO_EVENT& ev) override;
    void handleRmbDoubleClick(const ALLEGRO_EVENT& ev) override;
    void handleRmbDown(const ALLEGRO_EVENT& ev) override;
    void handleRmbDrag(const ALLEGRO_EVENT& ev) override;
    void panCam(const ALLEGRO_EVENT& ev);
    void resetCamXY();
    void resetCamZoom();
    
};
