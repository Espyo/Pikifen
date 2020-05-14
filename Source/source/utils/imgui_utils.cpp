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
 * Helps creating an ImGui ImageButton, followed by a centered Text.
 */
bool ImGui::ImageButtonAndText(
    ALLEGRO_BITMAP* icon, const ImVec2 &icon_size, const float button_padding,
    const string &text
) {
    bool result =
        ImGui::ImageButton(
            icon, icon_size,
            ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
            button_padding
        );
        
    float offset = (icon_size.y + button_padding * 2 - 16.0f) / 2.0f;
    
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
    ImGui::Text("%s", text.c_str());
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - offset);
    
    return result;
}


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
