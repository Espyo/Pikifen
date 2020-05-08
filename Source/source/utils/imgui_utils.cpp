/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Dear ImGui-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include "imgui_utils.h"

#include "../imgui/imgui_impl_allegro5.h"


/* ----------------------------------------------------------------------------
 * Helps creating an ImGui combo box, using a vector of strings for the
 * list of items.
 */
bool ImGui::Combo(
    const string &label, int* current_item, const vector<string> &items,
    const int popup_max_height_in_items
) {
    string items_str;
    for(size_t i = 0; i < items.size(); ++i) {
        items_str += items[i] + '\0';
    }
    
    return ImGui::Combo(
        label.c_str(), current_item, items_str.c_str(),
        popup_max_height_in_items
    );
}


/* ----------------------------------------------------------------------------
 * Helps creating an ImGui combo box, using a string to control the selection,
 * as well as a vector of strings for the list of items.
 */
bool ImGui::Combo(
    const string &label, string* current_item, const vector<string> &items,
    const int popup_max_height_in_items
) {

    string items_str;
    int item_nr = 0;
    for(size_t i = 0; i < items.size(); ++i) {
        items_str += items[i] + '\0';
        if(*current_item == items[i]) {
            item_nr = i;
        }
    }
    
    bool result = ImGui::Combo(
        label.c_str(), &item_nr, items_str.c_str(), popup_max_height_in_items
    );
    
    if(item_nr < items.size()) {
        *current_item = items[item_nr];
    } else {
        *current_item = "";
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Helps creating an ImGui list box, using a vector of strings for the
 * list of items.
 */
bool ImGui::ListBox(
    const string &label, int* current_item, const vector<string> &items,
    const int height_in_items
) {
    const char** array = new const char* [items.size()];
    
    for(size_t i = 0; i < items.size(); ++i) {
        array[i] = items[i].c_str();
    }
    
    return ImGui::ListBox(
        label.c_str(), current_item, array, items.size(), height_in_items
    );
}
