/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Header for the Easy Analog Cleaner class and related functions.
 *
 * Given game controller analog stick position readings, this class
 * can clean up the values and output new values that much better match
 * with what the player intends.
 *
 * The behavior of this process can be configured. It can also similarly clean
 * analog button pressure values.
 *
 * Special thanks to:
 * https://www.gamedeveloper.com/business/doing-thumbstick-dead-zones-right
 * https://www.gamedeveloper.com/design/interpreting-analog-sticks-in-inversus
 *
 */

#pragma once


//define EASY_ANALOG_CLEANER_DEBUG


/**
 * @brief Static utility class that holds analog stick cleaning logic.
 */
class EasyAnalogCleaner {

public:

    //--- Misc. declarations ---
    
    /**
     * @brief Settings for the cleaner.
     */
    struct Settings {
    
        /**
         * @brief Values inside of a deadzone will all be considered the same
         * value. Useful to prevent situations where an analog stick wiggling
         * by itself is being interpreted as purposeful player inputs.
         */
        struct Deadzones {
        
            /**
             * @brief Deadzones related to the radius of an analog stick, i.e.
             * how far away from the center it is.
             */
            struct Radial {
            
                /**
                 * @brief This is your typical analog stick deadzone value.
                 * Something like 0.2 is recommended for most analog sticks.
                 * Use 0 for no inner radial deadzone.
                 */
                float inner = 0.2f;
                
                /**
                 * @brief Outer radial deadzone size, in radius [0 - 1].
                 * Like the inner radial deadzone, except this is for values
                 * near the edges, since most analog sticks never physically
                 * reach the exact edge of the input circle.
                 * Something like 0.9 is recommended for most analog sticks.
                 * Use 1 for no outer radial deadzone.
                 */
                float outer = 0.9f;
                
                /**
                 * @brief If true, the stick radius is interpolated between the
                 * inner radial deadzone and the outer radial deadzone (if any).
                 * If false, no interpolation is done, meaning once the player
                 * leaves a deadzone the radius value will jump to whatever
                 * the values map to in the raw unit circle.
                 * Using this setting is recommended.
                 */
                bool interpolate = true;
                
            } radial;
            
            /**
             * @brief Deadzones related to the angle of an analog stick.
             */
            struct Angular {
            
                /**
                 * @brief Deadzone size, in radians [0 - PI/4], for the left
                 * and right inputs' angular deadzone.
                 * If the player wants to hold directly left or directly right,
                 * subtle movements up or down can veer the player off-course.
                 * This deadzone keeps the player locked if the stick angle is
                 * close enough to the left or right.
                 * Mostly recommended for something like a 3D platformer game.
                 * Use 0 for no horizontal angular deadzone.
                 */
                float horizontal = 0.0f;
                
                /**
                 * @brief Deadzone size, in radians [0 - PI/4], for the up and
                 * down inputs' angular deadzone.
                 * Same as angularHorizontalDeadzone, but for up and down.
                 * Use 0 for no vertical angular deadzone.
                 */
                float vertical = 0.0f;
                
                /**
                 * @brief Deadzone size, in radians [0 - PI/4], for the four
                 * diagonal inputs' angular deadzone.
                 * Same as angularHorizontalDeadzone, but for up and down.
                 * Use 0 for no diagonal angular deadzone.
                 */
                float diagonal = 0.0f;
                
                /**
                 * @brief If true, the stick angle is interpolated between the
                 * different angular deadzones (if any).
                 * If false, no interpolation is done, meaning once the player
                 * leaves a deadzone the angle value will jump to whatever the
                 * values map to in the raw unit circle.
                 * Using this setting is recommended.
                 */
                bool interpolate = true;
                
            } angular;
            
            /**
             * @brief Deadzones related to analog buttons.
             */
            struct Button {
            
                /**
                 * @brief Unpressed deadzone size, in pressure ratio [0 - 1].
                 * Something like 0.1 is recommended for most analog buttons.
                 * Use 0 for no unpressed deadzone.
                 */
                float unpressed = 0.1f;
                
                /**
                 * @brief Pressed deadzone size, in pressure ratio [0 - 1].
                 * Like the unpressed deadzone, except this is for when the
                 * button is fully pressed down.
                 * Something like 0.9 is recommended for most analog buttons.
                 * Use 1 for no pressed deadzone.
                 */
                float pressed = 0.9f;
                
                /**
                 * @brief If true, the pressure amount is interpolated between
                 * the unpressed deadzone and the pressed deadzone (if any).
                 * If false, no interpolation is done, meaning once the player
                 * leaves a deadzone the pressure value will jump to the
                 * given value.
                 * Using this setting is recommended.
                 */
                bool interpolate = true;
                
            } button;
            
        } deadzones;
        
        /**
         * @brief Applies a low-pass filter to the coordinate values,
         * avoiding jitter caused by natural human imprecision. Helps to
         * prevent snapback as well.
         * In order for this to work, the cleaned values given the previous
         * frame need to be provided too.
         */
        struct LowPassFilter {
        
            /**
             * @brief Filter factor [0 - 1]. 0 to disable. This is how much
             * the current values factor into the final result, compared to
             * the previous frame's values. If you want this feature enabled,
             * a value of 0.9 or so is recommended.
             */
            float factor = 0.0f;
            
            /**
             * @brief Filter factor for buttons [0 - 1]. 0 to disable.
             * Same as the analog stick low pass filter factor property.
             */
            float factorButton = 0.0f;
            
        } lowPassFilter;
        
        /**
         * @brief Other miscellaneous settings.
         */
        struct Misc {
        
            /**
             * @brief Clamps the coordinates into a unit circle. This is useful
             * to stop analog sticks with a wrong (e.g. square) plastic frame
             * from specifying coordinates that are outside what a regular
             * analog stick circular frame should allow, like for instance
             * 0.9,0.9. Also useful if you're doing something funny like making
             * use of D-pad coordinates or W A S D coordinates to figure out
             * the final coordinates you want to clean, though you should
             * probably avoid that.
             * This essentially stops a common problem where the player can
             * move their character or their reticle faster than intended.
             * Using this setting is recommended.
             */
            bool unitCircleClamp = true;
            
        } misc;
    };
    
    
    //--- Constants ---
    
    //Settings for when you want no changes to be made to the values.
    static const Settings SETTINGS_NO_CHANGES;
    
    
    //--- Function declarations ---
    
    static void clean(float coords[2]);
    static void clean(
        float coords[2], const Settings& settings,
        float previousFrameCoords[2] = nullptr
    );
    static void cleanButton(float* pressure);
    static void cleanButton(
        float* pressure, const Settings& settings,
        float previousFramePressure = 0.0f
    );
    
    
protected:

    //--- Function declarations ---
    
    static float getSnapDirDeadzone(
        int snapDirIdx, const Settings& settings
    );
    static float interpolate(
        float input, float inputStart, float inputEnd,
        float outputStart, float outputEnd
    );
    static float interpolateAndClamp(
        float input, float inputStart, float inputEnd,
        float outputStart, float outputEnd
    );
    static void processAngularDeadzones(
        float coords[2], const Settings& settings
    );
    static void processButtonDeadzones(
        float* pressure, const Settings& settings
    );
    static void processLowPassFilter(
        float coords[2], float previousFrameCoords[2], const Settings& settings
    );
    static void processLowPassFilterButton(
        float* pressure, float previousFramePressure, const Settings& settings
    );
    static void processRadialDeadzones(
        float coords[2], const Settings& settings
    );
    static void processUnitCircle(
        float coords[2], const Settings& settings
    );
    static void toCartesian(float coords[2], float angle, float radius);
    static void toPolar(float coords[2], float& angle, float& radius);
    static void writeDebugStickValues(float coords[2], bool input);
};
