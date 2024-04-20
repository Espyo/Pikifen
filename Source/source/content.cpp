/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Class representing a piece of game content.
 */

#include "content.h"

#include "misc_structs.h"


/**
 * @brief Loads content metadata from a data node.
 * 
 * @param node Data node to load from.
 */
void content::load_metadata_from_data_node(data_node* node) {
    reader_setter rs(node);

    rs.set("name", name);
    rs.set("description", description);
    rs.set("tags", tags);
    rs.set("maker", maker);
    rs.set("version", version);
    rs.set("engine_version", engine_version);
    rs.set("maker_notes", maker_notes);
    rs.set("notes", notes);
}


/**
 * @brief Resets the metadata.
 */
void content::reset_metadata() {
    name.clear();
    description.clear();
    tags.clear();
    maker.clear();
    version.clear();
    engine_version.clear();
    maker_notes.clear();
    notes.clear();
}


/**
 * @brief Saves content metadata to a data node.
 * 
 * @param node Data node to save to.
 */
void content::save_metadata_to_data_node(data_node* node) const {
#define saver(n, v) node->add(new data_node((n), (v)))
#define saver_o(n, v) if(!v.empty()) node->add(new data_node((n), (v)))

    saver("name", name);
    saver_o("description", description);
    saver_o("tags", tags);
    saver_o("maker", maker);
    saver_o("version", version);
    saver_o("engine_version", engine_version);
    saver_o("maker_notes", maker_notes);
    saver_o("notes", notes);

#undef saver_o
#undef saver
}
