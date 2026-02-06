/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the animation-related classes and functions.
 */

#pragma once

#include <map>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "../../core/const.h"
#include "../../lib/data_file/data_file.h"
#include "../../util/drawing_utils.h"
#include "../../util/general_utils.h"
#include "../content.h"
#include "hitbox.h"


using std::size_t;
using std::string;
using std::vector;


class AnimationDatabase;
class MobType;


/*
 * Animations work as follows:
 * An animation is a set of frames.
 * A frame contains hitboxes.
 *
 * A hitbox (hitbox.h) is defined by a body part,
 * a type of hitbox (can be hurt, or hurts other mobs),
 * and some other properties, like position.
 *
 * A frame in an animation is defined by a sprite
 * in a spritesheet, as well as its duration.
 *
 * Finally, an animation contains a list of frames,
 * and the loop frame, which is the one the
 * animation goes back to when it reaches
 * the end.
 *
 * To get a mob to display an animation,
 * you need to create an animation INSTANCE.
 * This can be played, rewound, etc., and
 * every mob may have a different animation
 * instance, with a different progress time and such.
 *
 * In order for all the different classes
 * to connect, they're referred to using
 * pointers. The animation database holds all of
 * this data so they know where each other is.
 */


/**
 * @brief A sprite in a spritesheet.
 */
class Sprite {

public:

    //--- Public members ---
    
    //Name of the sprite.
    string name;
    
    //Parent bitmap, normally a spritesheet.
    ALLEGRO_BITMAP* parentBmp = nullptr;
    
    //Internal name of the parent bitmap it belongs to.
    string bmpName;
    
    //Top-left corner of the sprite inside the parent bitmap.
    Point bmpPos;
    
    //Size of the sprite inside the parent bitmap.
    Point bmpSize;
    
    //Transformation data.
    Transform2d tf;
    
    //Tint the image with this color.
    ALLEGRO_COLOR tint = COLOR_WHITE;
    
    //Positional data for the Pikmin top (left/bud/flower) or leader light.
    Pose2d topPose;
    
    //Does this sprite even have a visible Pikmin top or leader light?
    bool topVisible = false;
    
    //The sprite's actual bitmap. This is a sub-bitmap of parentBmp.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //List of hitboxes on this frame.
    vector<Hitbox> hitboxes;
    
    
    //--- Public function declarations ---
    
    explicit Sprite(
        const string& name = "", ALLEGRO_BITMAP* const b = nullptr,
        const vector<Hitbox>& h = vector<Hitbox>()
    );
    Sprite(
        const string& name, ALLEGRO_BITMAP* const b, const Point& bPos,
        const Point& bSize, const vector<Hitbox>& h
    );
    Sprite(const Sprite& s2);
    ~Sprite();
    Sprite& operator=(const Sprite& s2);
    void createHitboxes(
        AnimationDatabase* const adb,
        float height = 0, float radius = 0
    );
    void setBitmap(
        const string& newBmpName,
        const Point& newBmpPos, const Point& newBmpSize,
        DataNode* node = nullptr
    );
    
};


/**
 * @brief A frame inside an animation.
 * A single sprite can appear multiple times in the same animation
 * (imagine an enemy shaking back and forth).
 */
class Frame {

public:

    //--- Public members ---
    
    //Name of the sprite to use in this frame.
    string spriteName;
    
    //Index of the sprite. Cache for performance.
    size_t spriteIdx = INVALID;
    
    //Pointer to the sprite. Cache for performance.
    Sprite* spritePtr = nullptr;
    
    //How long this frame lasts for, in seconds.
    float duration = 0.0f;
    
    //Interpolate transformation data between this frame and the next.
    bool interpolate = false;
    
    //Sound to play, if any. This is a sound info block in the mob's data.
    string sound;
    
    //Index of the sound to play, or INVALID. Cache for performance.
    size_t soundIdx = INVALID;
    
    //Signal to send, if any. INVALID = none.
    size_t signal = INVALID;
    
    
    //--- Public function declarations ---
    
    explicit Frame(
        const string& sn = "", size_t si = INVALID,
        Sprite* sp = nullptr, float d = 0.1,
        bool in = false, const string& snd = "", size_t s = INVALID
    );
    
};


/**
 * @brief An animation. A list of frames, basically.
 */
class Animation {

public:

    //--- Public members ---
    
    //Name of the animation.
    string name;
    
    //List of frames.
    vector<Frame> frames;
    
    //The animation loops back to this frame index when it reaches the end.
    size_t loopFrame = 0;
    
    //If this animation represents an attack that can miss,
    //this represents the successful hit rate.
    //100 means it cannot miss and/or is a normal animation.
    unsigned char hitRate = 100;
    
    
    //--- Public function declarations ---
    
    explicit Animation(
        const string& name = "",
        const vector<Frame>& frames = vector<Frame>(),
        size_t loopFrame = 0, unsigned char hitRate = 100
    );
    Animation(const Animation& a2);
    Animation& operator=(const Animation& a2);
    void deleteFrame(size_t idx);
    float getDuration();
    float getLoopDuration();
    void getFrameAndTime(
        float t, size_t* frameIdx, float* frameTime
    );
    float getTime(size_t frameIdx, float frameTime);
    
};


/**
 * @brief A database of animations, sprites, and body parts.
 *
 * Basically, an animation file.
 */
class AnimationDatabase : public Content {

public:

    //--- Public members ---
    
    //List of known animations.
    vector<Animation*> animations;
    
    //List of known sprites.
    vector<Sprite*> sprites;
    
    //List of known body parts.
    vector<BodyPart*> bodyParts;
    
    //Conversion between pre-named animations and in-file animations.
    vector<size_t> preNamedConversions;
    
    //Maximum span of the hitboxes. Cache for performance.
    float hitboxSpan = 0.0f;
    
    
    //--- Public function declarations ---
    
    explicit AnimationDatabase(
        const vector<Animation*>& a = vector<Animation*>(),
        const vector<Sprite*>& s = vector<Sprite*>(),
        const vector<BodyPart*>& b = vector<BodyPart*>()
    );
    size_t findAnimation(const string& name) const;
    size_t findSprite(const string& name) const;
    size_t findBodyPart(const string& name) const;
    void calculateHitboxSpan();
    void createConversions(
        const vector<std::pair<size_t, string> >& conversions,
        const DataNode* file
    );
    void deleteSprite(size_t idx);
    void fillSoundIdxCaches(MobType* mtPtr);
    void fixBodyPartPointers();
    void loadFromDataNode(DataNode* node);
    void saveToDataNode(DataNode* node, bool saveTopData);
    void sortAlphabetically();
    void destroy();
    
};


/**
 * @brief Instance of a running animation. This can be played, rewound, etc.
 */
class AnimationInstance {

public:

    //--- Public members ---
    
    //The animation currently running.
    Animation* curAnim = nullptr;
    
    //The database this belongs to.
    AnimationDatabase* animDb = nullptr;
    
    //Time passed on the current frame.
    float curFrameTime = 0.0f;
    
    //Index of the current frame of animation, or INVALID for none.
    size_t curFrameIdx = INVALID;
    
    
    //--- Public function declarations ---
    
    explicit AnimationInstance(AnimationDatabase* animDb = nullptr);
    AnimationInstance(const AnimationInstance& ai2);
    AnimationInstance& operator=(const AnimationInstance& ai2);
    void clear();
    void toStart();
    void skipAheadRandomly();
    bool tick(
        float deltaT,
        vector<size_t>* signals = nullptr,
        vector<size_t>* sounds = nullptr
    );
    bool validFrame() const;
    void getSpriteData(
        Sprite** outCurSpritePtr, Sprite** outNextSpritePtr,
        float* outInterpolationFactor
    ) const;
    size_t getNextFrameIdx(bool* outReachedEnd = nullptr) const;
    void initToFirstAnim(AnimationDatabase* db);
    
};


void getSpriteBasicEffects(
    const Point& basePos, float baseAngle,
    float baseAngleCosCache, float baseAngleSinCache,
    Sprite* curSpritePtr, Sprite* nextSpritePtr, float interpolationFactor,
    Point* outEffTrans, float* outEffAngle,
    Point* outEffScale, ALLEGRO_COLOR* outEffTint
);
void getSpriteBasicTopEffects(
    Sprite* curSpritePtr, Sprite* nextSpritePtr, float interpolationFactor,
    Point* outEffTrans, float* outEffAngle,
    Point* outEffSize
);
