/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the analog stick cleaner class and related functions.
 *
 * Given game controller analog stick position readings, this class
 * can clean up the values and output new values that much better match
 * with what the player intends.
 *
 * The behavior of this process can be configured.
 *
 * Special thanks to:
 * https://www.gamedeveloper.com/business/doing-thumbstick-dead-zones-right
 * https://www.gamedeveloper.com/design/interpreting-analog-sticks-in-inversus
 *
 * Future ideas:
 *   Snapback reduction
 *   Axis deadzones
 */

#pragma once


/**
 * @brief Static utility class that holds analog stick cleaning logic.
 */
class analog_stick_cleaner {

public:

    //--- Misc. declarations ---
    
    /**
     * @brief Settings for the cleaner.
     */
    struct settings_t {
    
        /**
         * @brief Values inside of a deadzone will all be considered the same
         * value. Useful to prevent situations where an analog stick wiggling
         * by itself being interpreted as purposeful player inputs.
         */
        struct deadzones_t {
        
            /**
             * @brief Deadzones related to the radius of the analog stick, i.e.
             * how far away from the center it is.
             */
            struct radial_t {
            
                //Inner radial deadzone size, in radius (0 to 1).
                //This is your typical analog stick deadzone value.
                //Something like 0.2 is recommended for most analog sticks.
                //Use 0 for no inner radial deadzone.
                float inner = 0.2f;
                
                //Outer radial deadzone size, in radius (0 to 1).
                //Like the inner radial deadzone, except this is for values near
                //the edges, since most analog sticks never physically reach the
                //exact edge of the input circle.
                //Something like 0.9 is recommended for most analog sticks.
                //Use 1 for no outer radial deadzone.
                float outer = 0.9f;
                
                //If true, the stick radius is interpolated between the
                //inner radial deadzone and the outer radial deadzone (if any).
                //If false, no interpolation is done, meaning once the player
                //leaves a deadzone the radius value will jump to whatever
                //the values map to in the raw unit circle.
                //Using this setting is recommended.
                bool interpolate = true;
                
            } radial;
            
            /**
             * @brief Deadzones related to the angle of the analog stick.
             */
            struct angular_t {
            
                //Deadzone size, in radians (0 to PI/4), for the left and right
                //inputs' angular deadzone.
                //If the player wants to hold directly left or directly right,
                //subtle movements up or down can veer the player off-course.
                //This deadzone keeps the player locked if the stick angle is
                //close enough to the left or right.
                //Mostly recommended for something like a 3D platformer game.
                //Use 0 for no horizontal angular deadzone.
                float horizontal = 0.0f;
                
                //Deadzone size, in radians (0 to PI/4), for the up and down
                //inputs' angular deadzone.
                //Same as angular_horizontal_deadzone, but for up and down.
                //Use 0 for no vertical angular deadzone.
                float vertical = 0.0f;
                
                //Deadzone size, in radians (0 to PI/4), for the four diagonal
                //inputs' angular deadzone.
                //Same as angular_horizontal_deadzone, but for up and down.
                //Use 0 for no diagonal angular deadzone.
                float diagonal = 0.0f;
                
                //If true, the stick angle is interpolated between the different
                //angular deadzones (if any).
                //If false, no interpolation is done, meaning once the player
                //leaves a deadzone the angle value will jump to whatever the
                //values map to in the raw unit circle.
                //Using this setting is recommended.
                bool interpolate = true;
                
            } angular;
            
        } deadzones;
        
        settings_t() {}
    };
    
    //--- Function declarations ---
    
    static void clean(
        float coords[2], const settings_t &settings = settings_t()
    );
    
    
private:

    //--- Function declarations ---
    
    static float clamp(float value, float lower_limit, float higher_limit);
    static float get_snap_dir_deadzone(
        int snap_dir_idx, const settings_t &settings
    );
    static float interpolate_and_clamp(
        float input, float input_start, float input_end,
        float output_start, float output_end
    );
    static void process_angular_deadzones(
        float coords[2], const settings_t &settings
    );
    static void process_radial_deadzones(
        float coords[2], const settings_t &settings
    );
    static void to_cartesian(float coords[2], float angle, float radius);
    static void to_polar(float coords[2], float &angle, float &radius);
};
