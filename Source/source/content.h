/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the class representing a piece of game content.
 */

#pragma once

#include <string>

#include "libs/data_file.h"


using std::string;


/**
 * @brief Represents any piece of game content that can be used in the engine,
 * shared around, belong as part of another piece of content, etc.
 */
class plain_content {
public:

    //--- Members ---

    //Relative path to the folder or file.
    string path;

};


/**
 * @brief Like the plain_content class, except this includes metadata
 * that can be loaded from and saved to a data file.
 */
class content : public plain_content {
public:

    //--- Members ---

    //Name.
    string name;

    //Optional description.
    string description;

    //Optional tags, separated by semicolon.
    string tags;

    //Optional person(s) who made it.
    string maker;

    //Optional version name or number.
    string version;

    //Optional version of the engine it was made for.
    string engine_version;

    //Optional notes for other makers to see.
    string maker_notes;

    //Optional notes of any kind.
    string notes;

    //Optional license.
    string license;


protected:

    //--- Function declarations ---

    void load_metadata_from_data_node(data_node* node);
    void reset_metadata();
    void save_metadata_to_data_node(data_node* node) const;

};
