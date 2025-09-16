/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the miscellaneous structures,
 * too simple to warrant their own files.
 */

#pragma once

#include <functional>
#include <map>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>

#include "../content/animation/animation.h"
#include "../content/mob_category/mob_category.h"
#include "../content/mob/mob_enums.h"
#include "../content/other/hazard.h"
#include "../content/other/particle.h"
#include "../lib/data_file/data_file.h"
#include "../lib/imgui/imgui.h"
#include "../util/container_utils.h"
#include "../util/drawing_utils.h"
#include "../util/general_utils.h"
#include "../util/geometry_utils.h"
#include "../util/math_utils.h"
#include "controls_mediator.h"


class PikminType;

using std::map;
using std::size_t;
using std::string;
using std::vector;

namespace GAMEPLAY_MSG_BOX {
extern const float ADVANCE_BUTTON_FADE_SPEED;
extern const float BUTTON_OFFSET_MULT;
extern const float BUTTON_OFFSET_TIME_MULT;
extern const float MARGIN;
extern const float PADDING;
extern const float MISINPUT_PROTECTION_DURATION;
extern const float TOKEN_ANIM_DURATION;
extern const float TOKEN_ANIM_X_AMOUNT;
extern const float TOKEN_ANIM_Y_AMOUNT;
extern const float TOKEN_SWIPE_DURATION;
extern const float TOKEN_SWIPE_X_AMOUNT;
extern const float TOKEN_SWIPE_Y_AMOUNT;
}


namespace NOTIFICATION {
extern const float FADE_SPEED;
}


namespace WHISTLE {
constexpr unsigned char N_RING_COLORS = 8;
constexpr unsigned char N_DOT_COLORS = 6;
extern const unsigned char DOT_COLORS[N_DOT_COLORS][3];
extern const float DOT_INTERVAL;
extern const float DOT_SPIN_SPEED;
extern const float FADE_TIME;
extern const unsigned char RING_COLORS[N_RING_COLORS][3];
extern const float RING_SPEED;
extern const float RINGS_INTERVAL;
}


//Types of string token.
enum STRING_TOKEN {

    //None.
    STRING_TOKEN_NONE,
    
    //A regular character.
    STRING_TOKEN_CHAR,
    
    //A line break.
    STRING_TOKEN_LINE_BREAK,
    
    //A control bind icon.
    STRING_TOKEN_BIND_INPUT,
    
};


/**
 * @brief Info about the game camera. Where it is, where it wants
 * to go, etc.
 */
struct Camera {

    //--- Members ---
    
    //Current position.
    Point pos;
    
    //Position it wants to be at.
    Point targetPos;
    
    //Zoom it wants to be at.
    float targetZoom = 1.0f;
    
    //Current zoom.
    float zoom = 1.0f;
    
    
    //--- Function declarations ---
    
    void setPos(const Point& newPos);
    void setZoom(float newZoom);
    void tick(float deltaT);
    
};


/**
 * @brief A segment of the game window that shows a view into the game world.
 * For instance, regular gameplay, the radar, the area editor's canvas, etc.
 */
struct Viewport {

    //--- Members ---
    
    //Center, in game window coordinates.
    Point center;
    
    //Width and height, in game window coordinates.
    Point size;
    
    //Allegro transformation for converting window to world coordinates.
    ALLEGRO_TRANSFORM windowToWorldTransform;
    
    //Allegro transformation for converting world to window coordinates.
    ALLEGRO_TRANSFORM worldToWindowTransform;
    
    //Camera.
    Camera cam;
    
    //Camera shakiness manager.
    Shaker shaker;
    
    //Top-left and bottom-right world coordinates that this camera can see.
    //These also include the specified margin.
    Point box[2];
    
    //Margin for the box coordinates.
    Point boxMargin;
    
    //Mouse cursor position, in world coordinates, if applicable.
    Point mouseCursorWorldPos;
    
    
    //--- Function declarations ---
    
    Viewport();
    Point getBottomRight();
    Point getTopLeft();
    void updateBox();
    void updateMouseCursor(const Point& windowPos);
    void updateTransformations();
    
};


/**
 * @brief Represents the non-interactive "console" that shows up at the
 * top of the screen, mostly in maker tool and system contexts.
 */
struct Console {

    //--- Function declarations ---
    
    Console();
    void clear();
    void draw() const;
    void tick(float deltaT);
    void write(
        const string& text,
        float totalDuration = 5.0f, float fadeDuration = 3.0f
    );
    
    
    private:
    
    //--- Members ---
    
    //Timer that controls visibility.
    Timer visibilityTimer;
    
    //If it's visible, this is how long it stays visible for.
    float visibilityDuration = 5.0f;
    
    //How long its fade period lasts.
    float fadeDuration = 3.0f;
    
    //Text it is showing, if any.
    string text;
    
};


/**
 * @brief Manages any errors that occur with the engine's content or logic.
 */
struct ErrorManager {

    //--- Function declarations ---
    
    void report(const string& s, const DataNode* d = nullptr);
    void reportAreaLoadErrors();
    void prepareAreaLoad();
    bool sessionHasErrors();
    
    private:
    
    //--- Members ---
    
    //How many errors have been reported this application session.
    size_t nrSessionErrors = 0;
    
    //Errors reported by the time the area load started.
    size_t nrErrorsOnAreaLoad = 0;
    
    //First error reported during area load.
    string firstAreaLoadError;
    
    
    //--- Function declarations ---
    
    void emitInGameplay(const string& s);
    void logToConsole(const string& s);
    void logToFile(const string& s);
    
};


/**
 * @brief Info about a token in a string.
 */
struct StringToken {

    //--- Members ---
    
    //Type of token.
    STRING_TOKEN type = STRING_TOKEN_CHAR;
    
    //Its content.
    string content;
    
    //Width that it takes up, in pixels.
    int width = 0;
    
};


/**
 * @brief Info about the operative system's mouse cursor.
 */
struct MouseCursor {

    //--- Members ---
    
    //Position, in window coordinates.
    Point winPos;
    
    //Whether it was moved this frame.
    bool movedThisFrame = false;
    
    //Whether it is on the game window.
    bool onWindow = true;
    
    //Spots the cursor has been through, in window coordinates.
    //Used for the faint trail left behind.
    vector<Point> history;
    
    //Time left until the position of the cursor is saved on the history.
    Timer saveTimer;
    
    
    //--- Function declarations ---
    
    void hide() const;
    void init();
    void reset();
    void show() const;
    void updatePos(const ALLEGRO_EVENT& ev);
    
};



/**
 * @brief Manages random number generation.
 */
struct RngManager {
    //--- Members ---
    
    //The current randomness generator's state.
    int32_t state = 0;
    
    
    //--- Function declarations ---
    
    void init();
    void init(int32_t initialSeed);
    int32_t i(int32_t minimum, int32_t maximum);
    float f(float minimum, float maximum);
    
    
    private:
    
    //--- Function declarations ---
    
    int32_t generateGoodInt();
};



/**
 * @brief This structure makes getting values in variables and
 * writing them in data files much easier.
 * On the get functions, specify the name of the child and the variable.
 */
struct GetterWriter {

    //--- Members ---
    
    //Node that this getter-writer pertains to.
    DataNode* node = nullptr;
    
    
    //--- Function declarations ---
    
    explicit GetterWriter(DataNode* dn = nullptr);
    void write(
        const string& childName, const string& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const char* var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const size_t& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const int& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const unsigned int& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const unsigned char& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const bool& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const float& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const double& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const ALLEGRO_COLOR& var,
        DataNode** outChildNode = nullptr
    );
    void write(
        const string& childName, const Point& var,
        DataNode** outChildNode = nullptr
    );
    
};



/**
 * @brief This structure makes reading values in data files
 * and setting them to variables much easier.
 * On the set functions, specify the name of the child and the variable.
 * If the child is empty, the variable will not be set.
 */
struct ReaderSetter {

    //--- Members ---
    
    //Node that this reader-setter pertains to.
    DataNode* node = nullptr;
    
    
    //--- Function declarations ---
    
    explicit ReaderSetter(DataNode* dn = nullptr);
    void set(
        const string& childName, string& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, size_t& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, int& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, unsigned int& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, unsigned char& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, bool& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, float& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, double& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, ALLEGRO_COLOR& var,
        DataNode** outChildNode = nullptr
    );
    void set(
        const string& childName, Point& var,
        DataNode** outChildNode = nullptr
    );
    
};



/**
 * @brief Makes it easy to read script variables, and make changes
 * based on which
 * ones exist, and what values they have.
 */
struct ScriptVarReader {

    //--- Members ---
    
    //Reference to the list of script variables it pertains to.
    map<string, string>& vars;
    
    
    //--- Function declarations ---
    
    explicit ScriptVarReader(map<string, string>& vars);
    bool get(const string& name, string& dest) const;
    bool get(const string& name, size_t& dest) const;
    bool get(const string& name, int& dest) const;
    bool get(const string& name, unsigned char& dest) const;
    bool get(const string& name, bool& dest) const;
    bool get(const string& name, float& dest) const;
    bool get(const string& name, ALLEGRO_COLOR& dest) const;
    bool get(const string& name, Point& dest) const;
    
};



/**
 * @brief List of content that is needed system-wide.
 */
struct SystemContentList {

    //--- Members ---
    
    //Graphics.
    
    //Bright circle.
    ALLEGRO_BITMAP* bmpBrightCircle = nullptr;
    
    //Bright ring.
    ALLEGRO_BITMAP* bmpBrightRing = nullptr;
    
    //Bubble 9-slice texture.
    ALLEGRO_BITMAP* bmpBubbleBox = nullptr;
    
    //9-slice texture for player input buttons.
    ALLEGRO_BITMAP* bmpButtonBox = nullptr;
    
    //Checkbox with a checkmark.
    ALLEGRO_BITMAP* bmpCheckboxCheck = nullptr;
    
    //Checkbox without a checkmark.
    ALLEGRO_BITMAP* bmpCheckboxNoCheck = nullptr;
    
    //Mission difficulty.
    ALLEGRO_BITMAP* bmpDifficulty = nullptr;
    
    //Discord icon.
    ALLEGRO_BITMAP* bmpDiscordIcon = nullptr;
    
    //Enemy soul.
    ALLEGRO_BITMAP* bmpEnemySoul = nullptr;
    
    //9-slice texture for the focused GUI item.
    ALLEGRO_BITMAP* bmpFocusBox = nullptr;
    
    //9-slice texture for GUI frames.
    ALLEGRO_BITMAP* bmpFrameBox = nullptr;
    
    //GitHub icon.
    ALLEGRO_BITMAP* bmpGithubIcon = nullptr;
    
    //A hard bubble.
    ALLEGRO_BITMAP* bmpHardBubble = nullptr;
    
    //Pikifen icon.
    ALLEGRO_BITMAP* bmpIcon = nullptr;
    
    //Idle glow.
    ALLEGRO_BITMAP* bmpIdleGlow = nullptr;
    
    //9-slice texture for player input keys.
    ALLEGRO_BITMAP* bmpKeyBox = nullptr;
    
    //Leader cursor.
    ALLEGRO_BITMAP* bmpLeaderCursor = nullptr;
    
    //Leader silhouette from the side.
    ALLEGRO_BITMAP* bmpLeaderSilhouetteSide = nullptr;
    
    //Leader silhouette from the top.
    ALLEGRO_BITMAP* bmpLeaderSilhouetteTop = nullptr;
    
    //Leader low health ring.
    ALLEGRO_BITMAP* bmpLowHealthRing = nullptr;
    
    //Bronze mission medal.
    ALLEGRO_BITMAP* bmpMedalBronze = nullptr;
    
    //Gold mission medal.
    ALLEGRO_BITMAP* bmpMedalGold = nullptr;
    
    //"Got it!" blurb for mission medals.
    ALLEGRO_BITMAP* bmpMedalGotIt = nullptr;
    
    //No mission medal.
    ALLEGRO_BITMAP* bmpMedalNone = nullptr;
    
    //Platinum mission medal.
    ALLEGRO_BITMAP* bmpMedalPlatinum = nullptr;
    
    //Silver mission medal.
    ALLEGRO_BITMAP* bmpMedalSilver = nullptr;
    
    //Icons for menu buttons.
    ALLEGRO_BITMAP* bmpMenuIcons = nullptr;
    
    //Mission clear stamp.
    ALLEGRO_BITMAP* bmpMissionClear = nullptr;
    
    //Mission fail stamp.
    ALLEGRO_BITMAP* bmpMissionFail = nullptr;
    
    //"More..." icon.
    ALLEGRO_BITMAP* bmpMore = nullptr;
    
    //Mouse cursor.
    ALLEGRO_BITMAP* bmpMouseCursor = nullptr;
    
    //Notification.
    ALLEGRO_BITMAP* bmpNotification = nullptr;
    
    //Onion menu change 1 icon.
    ALLEGRO_BITMAP* bmpOnionMenu1 = nullptr;
    
    //Onion menu change 10 icon.
    ALLEGRO_BITMAP* bmpOnionMenu10 = nullptr;
    
    //Onion menu all types icon.
    ALLEGRO_BITMAP* bmpOnionMenuAll = nullptr;
    
    //Onion menu single type icon.
    ALLEGRO_BITMAP* bmpOnionMenuSingle = nullptr;
    
    //Pikmin soul.
    ALLEGRO_BITMAP* bmpPikminSoul = nullptr;
    
    //Misc. specific player input icons.
    ALLEGRO_BITMAP* bmpPlayerInputIcons = nullptr;
    
    //Randomness symbol.
    ALLEGRO_BITMAP* bmpRandom = nullptr;
    
    //A rock.
    ALLEGRO_BITMAP* bmpRock = nullptr;
    
    //Mob shadow.
    ALLEGRO_BITMAP* bmpShadow = nullptr;
    
    //Rectangular mob shadow.
    ALLEGRO_BITMAP* bmpShadowSquare = nullptr;
    
    //Smack effect.
    ALLEGRO_BITMAP* bmpSmack = nullptr;
    
    //Smoke.
    ALLEGRO_BITMAP* bmpSmoke = nullptr;
    
    //Sparkle effect.
    ALLEGRO_BITMAP* bmpSparkle = nullptr;
    
    //Spotlight for blackout.
    ALLEGRO_BITMAP* bmpSpotlight = nullptr;
    
    //Swarm arrow.
    ALLEGRO_BITMAP* bmpSwarmArrow = nullptr;
    
    //Invalid throw marker.
    ALLEGRO_BITMAP* bmpThrowInvalid = nullptr;
    
    //Throw preview texture.
    ALLEGRO_BITMAP* bmpThrowPreview = nullptr;
    
    //Dashed throw preview texture.
    ALLEGRO_BITMAP* bmpThrowPreviewDashed = nullptr;
    
    //Wave ring.
    ALLEGRO_BITMAP* bmpWaveRing = nullptr;
    
    //Fonts.
    
    //Font for area names.
    ALLEGRO_FONT* fntAreaName = nullptr;
    
    //Allegro's built-in font.
    ALLEGRO_FONT* fntBuiltin = nullptr;
    
    //Font for HUD counters.
    ALLEGRO_FONT* fntCounter = nullptr;
    
    //Counter displayed next to the leader cursor.
    ALLEGRO_FONT* fntLeaderCursorCounter = nullptr;
    
    //Font for slim text.
    ALLEGRO_FONT* fntSlim = nullptr;
    
    //Font for standard text.
    ALLEGRO_FONT* fntStandard = nullptr;
    
    //Font for the carrying / money values.
    ALLEGRO_FONT* fntValue = nullptr;
    
    //Specifically Dear ImGui fonts.
    
    //Header editor font for Dear ImGui.
    ImFont* fntDearImGuiHeader = nullptr;
    
    //Monospace editor font for Dear ImGui.
    ImFont* fntDearImGuiMonospace = nullptr;
    
    //Standard editor font for Dear ImGui.
    ImFont* fntDearImGuiStandard = nullptr;
    
    //Sound effects.
    
    //Attack.
    ALLEGRO_SAMPLE* sndAttack = nullptr;
    
    //Camera zoom level.
    ALLEGRO_SAMPLE* sndCamera = nullptr;
    
    //10 second countdown second tick.
    ALLEGRO_SAMPLE* sndCountdownTick = nullptr;
    
    //Useless attack ding.
    ALLEGRO_SAMPLE* sndDing = nullptr;
    
    //Enemy soul.
    ALLEGRO_SAMPLE* sndEnemySoul = nullptr;
    
    //Gameplay message character being typed out.
    ALLEGRO_SAMPLE* sndGameplayMsgChar = nullptr;
    
    //"Go!" big message.
    ALLEGRO_SAMPLE* sndGo = nullptr;
    
    //"Got it!" medal blurb.
    ALLEGRO_SAMPLE* sndMedalGotIt = nullptr;
    
    //Menu item activation.
    ALLEGRO_SAMPLE* sndMenuActivate = nullptr;
    
    //Menu item back.
    ALLEGRO_SAMPLE* sndMenuBack = nullptr;
    
    //Menu item activation fail.
    ALLEGRO_SAMPLE* sndMenuFail = nullptr;
    
    //Menu item focus.
    ALLEGRO_SAMPLE* sndMenuFocus = nullptr;
    
    //"Mission clear!" big message.
    ALLEGRO_SAMPLE* sndMissionClear = nullptr;
    
    //"Mission failed..." big message.
    ALLEGRO_SAMPLE* sndMissionFailed = nullptr;
    
    //"1 minute left!" big message.
    ALLEGRO_SAMPLE* sndOneMinuteLeft = nullptr;
    
    //"Ready..." big message.
    ALLEGRO_SAMPLE* sndReady = nullptr;
    
    //Switching standby Pikmin type.
    ALLEGRO_SAMPLE* sndSwitchPikmin = nullptr;
    
    //Global animations.
    
    //Leader damage spark.
    AnimationInstance anmSparks;
    
};


/**
 * @brief List of internal names of content that is needed by the system.
 */
struct SystemContentNames {

    //--- Members ---
    
    //Graphics.
    
    //Bright circle.
    string bmpBrightCircle = "effects/bright_circle";
    
    //Bright ring.
    string bmpBrightRing = "effects/bright_ring";
    
    //Bubble box.
    string bmpBubbleBox = "gui/bubble_box";
    
    //9-slice texture for player input buttons.
    string bmpButtonBox = "gui/button_box";
    
    //Checkbox with a checkmark.
    string bmpCheckboxCheck = "gui/checkbox_check";
    
    //Checkbox without a checkmark.
    string bmpCheckboxNoCheck = "gui/checkbox_no_check";
    
    //Mission difficulty.
    string bmpDifficulty = "gui/difficulty";
    
    //Discord icon.
    string bmpDiscordIcon = "gui/discord_icon";
    
    //Editor icons.
    string bmpEditorIcons = "gui/editor_icons";
    
    //Enemy soul.
    string bmpEnemySoul = "effects/enemy_soul";
    
    //GUI focus box.
    string bmpFocusBox = "gui/focus_box";
    
    //9-slice texture for GUI frames.
    string bmpFrameBox = "gui/frame_box";
    
    //GitHub icon.
    string bmpGithubIcon = "gui/github_icon";
    
    //A hard bubble.
    string bmpHardBubble = "gui/hard_bubble";
    
    //Pikifen icon.
    string bmpIcon = "gui/icon";
    
    //Idle glow.
    string bmpIdleGlow = "effects/idle_glow";
    
    //9-slice texture for player input keys.
    string bmpKeyBox = "gui/key_box";
    
    //Leader cursor.
    string bmpLeaderCursor = "gui/leader_cursor";
    
    //Leader silhouette from the side.
    string bmpLeaderSilhouetteSide = "gui/leader_silhouette_side";
    
    //Leader silhouette from the top.
    string bmpLeaderSilhouetteTop = "gui/leader_silhouette_top";
    
    //Leader low health ring.
    string bmpLowHealthRing = "gui/low_health_ring";
    
    //Bronze medal.
    string bmpMedalBronze = "gui/medal_bronze";
    
    //Gold medal.
    string bmpMedalGold = "gui/medal_gold";
    
    //"Got it!" blurb for mission medals.
    string bmpMedalGotIt = "gui/medal_got_it";
    
    //No medal.
    string bmpMedalNone = "gui/medal_none";
    
    //Platinum medal.
    string bmpMedalPlatinum = "gui/medal_platinum";
    
    //Silver medal.
    string bmpMedalSilver = "gui/medal_silver";
    
    //Icons for menu buttons.
    string bmpMenuIcons = "gui/menu_icons";
    
    //Mission clear stamp.
    string bmpMissionClear = "gui/mission_clear";
    
    //Mission fail stamp.
    string bmpMissionFail = "gui/mission_fail";
    
    //"More..." icon.
    string bmpMore = "gui/more";
    
    //Mouse cursor.
    string bmpMouseCursor = "gui/mouse_cursor";
    
    //Notification.
    string bmpNotification = "gui/notification";
    
    //Onion menu change 1 icon.
    string bmpOnionMenu1 = "gui/onion_menu_1";
    
    //Onion menu change 10 icon.
    string bmpOnionMenu10 = "gui/onion_menu_10";
    
    //Onion menu all types icon.
    string bmpOnionMenuAll = "gui/onion_menu_all";
    
    //Onion menu single type icon.
    string bmpOnionMenuSingle = "gui/onion_menu_single";
    
    //Pikmin soul.
    string bmpPikminSoul = "effects/pikmin_soul";
    
    //Misc. specific player input icons.
    string bmpPlayerInputIcons = "gui/player_input_icons";
    
    //Randomness symbol.
    string bmpRandom = "gui/random";
    
    //A rock.
    string bmpRock = "effects/rock";
    
    //Mob shadow.
    string bmpShadow = "effects/shadow";
    
    //Rectangular mob shadow.
    string bmpShadowSquare = "effects/shadow_square";
    
    //Smack effect.
    string bmpSmack = "effects/smack";
    
    //Smoke.
    string bmpSmoke = "effects/smoke";
    
    //Sparkle effect.
    string bmpSparkle = "effects/sparkle";
    
    //Spotlight for blackout.
    string bmpSpotlight = "effects/spotlight";
    
    //Swarm arrow.
    string bmpSwarmArrow = "gui/swarm_arrow";
    
    //Invalid throw marker.
    string bmpThrowInvalid = "gui/throw_invalid";
    
    //Throw preview texture.
    string bmpThrowPreview = "gui/throw_preview";
    
    //Dashed throw preview texture.
    string bmpThrowPreviewDashed = "gui/throw_preview_dashed";
    
    //Title screen background.
    string bmpTitleScreenBg = "gui/title_screen_bg";
    
    //Wave ring.
    string bmpWaveRing = "effects/wave_ring";
    
    //Fonts.
    
    //Font for area names.
    string fntAreaName = "fonts/area_name";
    
    //Font for HUD counters.
    string fntCounter = "fonts/counter";
    
    //Font displayed next to the leader cursor.
    string fntLeaderCursorCounter = "fonts/leader_cursor_counter";
    
    //TTF Dear ImGui header font for editors.
    string fntEditorHeader = "fonts/editor_header";
    
    //TTF Dear ImGui monospace font for editors.
    string fntEditorMonospace = "fonts/editor_monospace";
    
    //TTF Dear Imgui standard font for editors.
    string fntEditorStandard = "fonts/editor_standard";
    
    //Font for slim text.
    string fntSlim = "fonts/slim";
    
    //Font for standard text.
    string fntStandard = "fonts/standard";
    
    //Font for the carrying / money values.
    string fntValue = "fonts/value";
    
    //Sound effects.
    
    //Attack.
    string sndAttack = "effects/attack";
    
    //Camera zoom level.
    string sndCamera = "gui/camera";
    
    //10 second countdown second tick.
    string sndCountdownTick = "gui/countdown_tick";
    
    //Useless attack ding.
    string sndDing = "effects/ding";
    
    //Enemy soul.
    string sndEnemySoul = "effects/enemy_soul";
    
    //Gameplay message character being typed out.
    string sndGameplayMsgChar = "gui/gameplay_message_char";
    
    //"Go!" big message.
    string sndGo = "gui/go";
    
    //"Got it!" medal blurb.
    string sndMedalGotIt = "gui/medal_got_it";
    
    //Menu item activation.
    string sndMenuActivate = "gui/menu_activate";
    
    //Menu item back.
    string sndMenuBack = "gui/menu_back";
    
    //Menu item fail.
    string sndMenuFail = "gui/menu_fail";
    
    //Menu item focus.
    string sndMenuFocus = "gui/menu_focus";
    
    //"Mission clear!" big message.
    string sndMissionClear = "gui/mission_clear";
    
    //"Mission failed..." big message.
    string sndMissionFailed = "gui/mission_failed";
    
    //"1 minute left!" big message.
    string sndOneMinuteLeft = "gui/one_minute_left";
    
    //"Ready..." big message.
    string sndReady = "gui/ready";
    
    //Switching standby Pikmin type.
    string sndSwitchPikmin = "gui/switch_pikmin";
    
    //Songs.
    
    //Boss theme.
    string sngBoss = "others/boss";
    
    //Boss victory theme.
    string sngBossVictory = "others/boss_victory";
    
    //Editors.
    string sngEditors = "others/editors";
    
    //Menus.
    string sngMenus = "others/menus";
    
    //Results menu.
    string sngResults = "others/results";
    
    //Global animations.
    
    //Leader damage spark.
    string anmSparks = "sparks";
    
    //Particle generators.
    
    //Pikmin inserted in converter.
    string parConverterInsertion = "converter_insertion";
    
    //Useless attack ding.
    string parDing = "ding";
    
    //Enemy defeat sparkles.
    string parEnemyDefeat = "enemy_defeat";
    
    //Leader being healed at a ship.
    string parLeaderHeal = "leader_heal";
    
    //Leader landed on the floor after some height.
    string parLeaderLand = "leader_land";
    
    //Onion generating inside.
    string parOnionGenInside = "onion_generating_inside";
    
    //Onion object insertion.
    string parOnionInsertion = "onion_insertion";
    
    //Dirt that comes out of the floor when a Pikmin gets plucked.
    string parPikminPluckDirt = "pikmin_pluck_dirt";
    
    //Sparkles for a Pikmin seed in the air.
    string parPikminSeed = "pikmin_seed";
    
    //Dirt that comes out of the floor when a Pikmin seed lands.
    string parPikminSeedLanded = "pikmin_seed_landed";
    
    //Successful attack smack.
    string parSmack = "smack";
    
    //Leader spray.
    string parSpray = "spray";
    
    //Pikmin sprout maturity evolution.
    string parSproutEvolution = "sprout_evolution";
    
    //Pikmin sprout maturity regression.
    string parSproutRegression = "sprout_regression";
    
    //Throw trail.
    string parThrowTrail = "throw_trail";
    
    //Sparkles on top of treasures.
    string parTreasure = "treasure";
    
    //Wave ring as a ripple around mobs on water.
    string parWaveRing = "wave_ring";
    
    
    //--- Function declarations ---
    
    void load(DataNode* file);
    
};


/**
 * @brief Manages fade ins/outs for transitions.
 */
struct FadeManager {

    public:
    
    //--- Function declarations ---
    
    FadeManager(float duration);
    void setNextFadeDuration(float duration);
    void startFade(bool fadeIn, const std::function<void()>& onEnd);
    bool isFadeIn() const;
    bool isFading() const;
    float getPercLeft() const;
    void tick(float deltaT);
    void draw();
    
    private:
    
    //--- Members ---
    
    //Standard duration of a fade in/out.
    float duration = 0.0f;
    
    //Override the next fade in/out's duration with this. 0 for no override.
    float durationOverride = 0.0f;
    
    //Time left in the current fade in/out.
    float timeLeft = 0.0f;
    
    //True if fading in, false if fading out.
    bool fadeIn = false;
    
    //Code to run when the fade in/out finishes.
    std::function<void()> onEnd = nullptr;
    
};



/**
 * @brief Info about the currently visible notification during gameplay.
 * This is stuff like a note above the leader telling the player
 * what button to press to do something, like plucking.
 */
struct Notification {

    public:
    
    //--- Function declarations ---
    
    void draw(const Viewport& view) const;
    float getVisibility() const;
    void reset();
    void setContents(
        const Inpution::InputSource& inputSource,
        const string& text, const Point& pos
    );
    void setEnabled(bool enabled);
    void tick(float deltaT);
    
    private:
    
    //--- Members ---
    
    //Is it meant to exist?
    bool enabled = true;
    
    //What player input source icon to show.
    Inpution::InputSource inputSource;
    
    //What text to write.
    string text;
    
    //Coordinates of the focal point.
    Point pos;
    
    //Visibility. 0 is hidden, 1 is fully visible. Mid values for transitioning.
    float visibility = 0.0f;
    
};



/**
 * @brief Info on how a bitmap should be drawn, in regards to
 * translation, rotation, coloring, etc.
 */
struct BitmapEffect {

    //--- Members ---
    
    //Offset horizontally and vertically by this much.
    Point translation;
    
    //Rotate the bitmap by this angle, in radians.
    float rotation = 0.0f;
    
    //Scale horizontally and vertically. LARGE_FLOAT = use the other's scale.
    Point scale = Point(1.0f);
    
    //Tint the bitmap by this color. Also makes it transparent.
    ALLEGRO_COLOR tintColor = COLOR_WHITE;
    
    //Re-draws the bitmap on top, in additive blend, with this color.
    ALLEGRO_COLOR glowColor = COLOR_BLACK;
    
};


//Performance monitor states.
enum PERF_MON_STATE {

    //Measuring loading times.
    PERF_MON_STATE_LOADING,
    
    //Measuring gameplay frame performance.
    PERF_MON_STATE_FRAME,
    
};


/**
 * @brief Info about how long certain things took. Useful for makers
 * to monitor performance with.
 */
struct PerformanceMonitor {

    public:
    
    //--- Function declarations ---
    
    PerformanceMonitor();
    void setAreaName(const string& name);
    void setPaused(bool paused);
    void enterState(const PERF_MON_STATE mode);
    void leaveState();
    void startMeasurement(const string& name);
    void finishMeasurement();
    void saveLog();
    void reset();
    
    private:
    
    //--- Misc. declarations ---
    
    /**
     * @brief A page in the report.
     */
    struct Page {
    
        public:
        
        //--- Members ---
        
        //How long it lasted for in total.
        double duration = 0.0f;
        
        //Measurements took, and how long each one took.
        vector<std::pair<string, double> > measurements;
        
        
        //--- Function declarations ---
        
        void write(string& s);
        
        private:
        
        //--- Function declarations ---
        
        void writeMeasurement(
            string& str, const string& name,
            double time, float total
        );
    };
    
    
    //--- Members ---
    
    //Name of the area being monitored.
    string areaName;
    
    //Current state.
    PERF_MON_STATE curState = PERF_MON_STATE_LOADING;
    
    //Is the monitoring currently paused?
    bool paused = false;
    
    //When the current state began.
    double curStateStartTime = 0.0f;
    
    //When the current measurement began.
    double curMeasurementStartTime = 0.0f;
    
    //Name of the current measurement.
    string curMeasurementName;
    
    //Page of information about the current working info.
    PerformanceMonitor::Page curPage;
    
    //How many frames of gameplay have been sampled.
    size_t frameSamples = 0;
    
    //Page of information about the loading process.
    PerformanceMonitor::Page loadingPage;
    
    //Page of information about the average frame.
    PerformanceMonitor::Page frameAvgPage;
    
    //Page of information about the fastest frame.
    PerformanceMonitor::Page frameFastestPage;
    
    //Page of information about the slowest frame.
    PerformanceMonitor::Page frameSlowestPage;
    
};


struct SubgroupTypeManager;


/**
 * @brief Represents a leader subgroup type;
 * a Red Pikmin, a Yellow Pikmin, a leader, etc.
 */
struct SubgroupType {

    public:
    
    //--- Function declarations ---
    
    SUBGROUP_TYPE_CATEGORY getCategory() const { return category; }
    ALLEGRO_BITMAP* getIcon() const { return icon; }
    
private:

    friend SubgroupTypeManager;
    
    //--- Members ---
    
    //Category this subgroup type belongs to.
    SUBGROUP_TYPE_CATEGORY category = SUBGROUP_TYPE_CATEGORY_LEADER;
    
    //Specific mob type it refers to.
    MobType* specificType = nullptr;
    
    //Icon used to represent this subgroup type.
    ALLEGRO_BITMAP* icon = nullptr;
    
};


/**
 * @brief Manages what types of subgroups exist.
 */
struct SubgroupTypeManager {

    public:
    
    //--- Function declarations ---
    
    void registerType(
        const SUBGROUP_TYPE_CATEGORY category,
        MobType* specificType = nullptr,
        ALLEGRO_BITMAP* icon = nullptr
    );
    SubgroupType* getType(
        const SUBGROUP_TYPE_CATEGORY category,
        const MobType* specificType = nullptr
    ) const;
    SubgroupType* getFirstType() const;
    SubgroupType* getPrevType(const SubgroupType* sgt) const;
    SubgroupType* getNextType(const SubgroupType* sgt) const;
    void clear();
    
    private:
    
    //--- Members ---
    
    //Known types.
    vector<SubgroupType*> types;
    
};


/**
 * @brief Info about the engine's lifetime statistics.
 */
struct Statistics {

    //--- Members ---
    
    //Times Pikifen was started.
    uint32_t startups = 0;
    
    //Time Pikifen was running for, in seconds.
    double runtime = 0.0f;
    
    //Time gameplay happened for, in seconds.
    double gameplayTime = 0.0f;
    
    //Times areas were entered.
    uint32_t areaEntries = 0;
    
    //Times Pikmin were born from an Onion.
    uint64_t pikminBirths = 0;
    
    //Times Pikmin died for other reasons.
    uint64_t pikminDeaths = 0;
    
    //Times Pikmin died by being eaten.
    uint64_t pikminEaten = 0;
    
    //Times Pikmin died from a hazard.
    uint64_t pikminHazardDeaths = 0;
    
    //Times Pikmin bloomed (leaf to bud, leaf to flower, or bud to flower).
    uint64_t pikminBlooms = 0;
    
    //Times Pikmin were saved from a hazard by being whistled.
    uint64_t pikminSaved = 0;
    
    //Times enemies were defeated.
    uint64_t enemyDefeats = 0;
    
    //Times Pikmin were thrown. Leaders thrown don't count.
    uint64_t pikminThrown = 0;
    
    //Times the whistle was used.
    uint64_t whistleUses = 0;
    
    //Distance walked by an active leader, in pixels.
    double distanceWalked = 0.0f;
    
    //Damage suffered by leaders.
    double leaderDamageSuffered = 0.0f;
    
    //Damage caused by punches.
    double punchDamageCaused = 0.0f;
    
    //Times leaders were KO'd.
    uint64_t leaderKos = 0;
    
    //Times sprays were used.
    uint64_t spraysUsed = 0;
    
};


/**
 * @brief Cached information about how an edge should draw its offset effects.
 */
struct EdgeOffsetCache {

    //--- Members ---
    
    //Length of the effect's "rectangle", per end vertex. 0 for none.
    float lengths[2] = {0.0f, 0.0f};
    
    //Angle of the effect's "rectangle", per end vertex.
    float angles[2] = {0.0f, 0.0f};
    
    //Color of the effect, per end vertex.
    ALLEGRO_COLOR colors[2] = {COLOR_EMPTY, COLOR_EMPTY};
    
    //Length of the effect's "elbow", per end vertex. 0 for none.
    float elbowLengths[2] = {0.0f, 0.0f};
    
    //Angle of the effect's "elbow", per end vertex.
    float elbowAngles[2] = {0.0f, 0.0f};
    
    //Index of the vertex that should be processed first.
    unsigned char firstEndVertexIdx = 0;
    
};


/**
 * @brief Info about the current whistle usage.
 */
struct Whistle {

    //--- Members ---
    
    //Current center.
    Point center;
    
    //Current radius of the whistle.
    float radius = 0.0f;
    
    //Radius of every 6th dot.
    float dotRadius[6] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
    
    //Radius the whistle was at pre-fade.
    float fadeRadius = 0.0f;
    
    //Time left for the whistle's fading animations.
    Timer fadeTimer = Timer(WHISTLE::FADE_TIME);
    
    //Time left until the next series of dots begins.
    Timer nextDotTimer = Timer(WHISTLE::DOT_INTERVAL);
    
    //Time left until the next ring is spat out.
    Timer nextRingTimer = Timer(WHISTLE::RINGS_INTERVAL);
    
    //Color index of each ring.
    vector<unsigned char> ringColors;
    
    //Color index of the previous ring.
    unsigned char ringPrevColor = 0;
    
    //Distance of each ring.
    vector<float> rings;
    
    //Is the whistle currently being blown?
    bool whistling = false;
    
    
    //--- Function declarations ---
    
    Whistle();
    void startWhistling();
    void stopWhistling();
    void tick(
        float deltaT, const Point& center,
        float whistleRange, float leaderToCursorDist
    );
    
};


/**
 * @brief Asset manager.
 *
 * When you have the likes of an animation, every
 * frame in it is normally a sub-bitmap of the same
 * parent bitmap.
 * Naturally, loading from the disk and storing
 * in memory the same parent bitmap for every single
 * frame would be unbelievable catastrophical, so
 * that is why the asset manager was created.
 *
 * Whenever a frame of animation is being loaded,
 * it asks the asset manager to retrieve the
 * parent bitmap from memory. If the parent bitmap
 * has never been loaded, it gets loaded now.
 * When the next frame comes, and requests the
 * parent bitmap, the manager just returns the one already
 * loaded.
 * All the while, the manager is keeping track
 * of how many frames are referencing this parent bitmap.
 * When one of them doesn't need it any more, it sends
 * a free request (e.g.: when a frame is changed
 * in the animation editor, the entire bitmap
 * is destroyed and another is created).
 * This decreases the counter by one.
 * When the counter reaches 0, that means no frame
 * is needing the parent bitmap, so it gets destroyed.
 * If some other frame needs it, it'll be loaded from
 * the disk again.
 * This manager can also handle other types of asset, like audio samples.
 *
 * @tparam AssetT Asset type.
 */
template<typename AssetT>
class AssetManager {

public:

    //--- Function definitions ---
    
    virtual ~AssetManager() = default;
    
    /**
     * @brief Returns the specified asset, by name.
     *
     * @param name Name of the asset to get.
     * @param node If not nullptr, blame this data node if the file
     * doesn't exist.
     * @param reportErrors Only issues errors if this is true.
     * @return The asset
     */
    AssetT get(
        const string& name, DataNode* node = nullptr,
        bool reportErrors = true
    ) {
        if(name.empty()) return doLoad("", node, reportErrors);
        
        if(!isInMap(list, name)) {
            AssetT assetPtr =
                doLoad(name, node, reportErrors);
            list[name] = AssetUse(assetPtr);
            totalUses++;
            return assetPtr;
        } else {
            list[name].uses++;
            totalUses++;
            return list[name].ptr;
        }
    }
    
    /**
     * @brief Frees one use of the asset. If the asset has no more calls,
     * it's automatically cleared.
     *
     * @param ptr Asset to free.
     */
    void free(const AssetT ptr) {
        if(!ptr) return;
        auto it = list.begin();
        for(; it != list.end(); ++it) {
            if(it->second.ptr == ptr) break;
        }
        free(it);
    }
    
    /**
     * @brief Frees one use of the asset. If the asset has no more calls,
     * it's automatically cleared.
     *
     * @param ptr Name of the asset to free.
     */
    void free(const string& name) {
        if(name.empty()) return;
        free(list.find(name));
    }
    
    /**
     * @brief Unloads all assets loaded and clears the list.
     */
    void clear() {
        for(auto& asset : list) {
            doUnload(asset.second.ptr);
        }
        list.clear();
        totalUses = 0;
    }
    
    /**
     * @brief Returns the total number of uses. Used for debugging.
     *
     * @return The amount.
     */
    long getTotalUses() const {
        return totalUses;
    }
    
    /**
     * @brief Returns the size of the list. Used for debugging.
     *
     * @return The size.
     */
    size_t getListSize() const {
        return list.size();
    }
    
protected:

    //--- Misc. declarations ---
    
    virtual AssetT doLoad(
        const string& path, DataNode* node, bool reportErrors
    ) = 0;
    virtual void doUnload(AssetT asset) = 0;
    
    /**
     * @brief Info about an asset.
     */
    struct AssetUse {
    
        //--- Members ---
        
        //Asset pointer.
        AssetT ptr = nullptr;
        
        //How many uses it has.
        size_t uses = 1;
        
        
        //--- Function declarations ---
        
        explicit AssetUse(AssetT ptr = nullptr) : ptr(ptr) {}
    };
    
    
    //--- Members ---
    
    //List of loaded assets.
    map<string, AssetUse> list;
    
    //Total sum of uses. Useful for debugging.
    long totalUses = 0;
    
    
    //--- Function definitions ---
    
    /**
     * @brief Frees one use of the asset. If the asset has no more calls,
     * it's automatically cleared.
     *
     * @param it Iterator of the asset from the list.
     */
    void free(typename map<string, AssetUse>::iterator it) {
        if(it == list.end()) return;
        it->second.uses--;
        totalUses--;
        if(it->second.uses == 0) {
            doUnload(it->second.ptr);
            list.erase(it);
        }
    }
    
};


/**
 * @brief Audio stream manager. See AssetManager.
 */
class AudioStreamManager : public AssetManager<ALLEGRO_AUDIO_STREAM*> {

protected:

    //--- Function declarations ---
    
    ALLEGRO_AUDIO_STREAM* doLoad(
        const string& name, DataNode* node, bool reportErrors
    ) override;
    void doUnload(ALLEGRO_AUDIO_STREAM* asset) override;
    
};


/**
 * @brief Bitmap manager. See AssetManager.
 */
class BitmapManager : public AssetManager<ALLEGRO_BITMAP*> {

protected:

    //--- Function declarations ---
    
    ALLEGRO_BITMAP* doLoad(
        const string& name, DataNode* node, bool reportErrors
    ) override;
    void doUnload(ALLEGRO_BITMAP* asset) override;
    
};


/**
 * @brief Sound effect sample manager. See AssetManager.
 */
class SampleManager : public AssetManager<ALLEGRO_SAMPLE*> {

protected:

    //--- Function declarations ---
    
    ALLEGRO_SAMPLE* doLoad(
        const string& name, DataNode* node, bool reportErrors
    ) override;
    void doUnload(ALLEGRO_SAMPLE* asset) override;
    
};
