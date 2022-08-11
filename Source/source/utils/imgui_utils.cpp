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

#include <cmath>

#include "imgui_utils.h"

#include "../imgui/imgui_impl_allegro5.h"


/* ----------------------------------------------------------------------------
 * Helps creating an ImGui combo box, using a vector of strings for the
 * list of items.
 * label:
 *   Combo widget label.
 * current_item:
 *   Index number of the current selected item.
 * items:
 *   List of items.
 * popup_max_height_in_items:
 *   Maximum height of the popup, in number of items.
 */
bool ImGui::Combo(
    const string &label, int* current_item, const vector<string> &items,
    const int popup_max_height_in_items
) {
    string items_str;
    for(size_t i = 0; i < items.size(); ++i) {
        items_str += items[i] + '\0';
    }
    
    return
        ImGui::Combo(
            label.c_str(), current_item, items_str.c_str(),
            popup_max_height_in_items
        );
}


/* ----------------------------------------------------------------------------
 * Helps creating an ImGui combo box, using a string to control the selection,
 * as well as a vector of strings for the list of items.
 * label:
 *   Combo widget label.
 * current_item:
 *   Name of the current selected item.
 * items:
 *   List of items.
 * popup_max_height_in_items:
 *   Maximum height of the popup, in number of items.
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
            item_nr = (int) i;
        }
    }
    
    bool result =
        ImGui::Combo(
            label.c_str(), &item_nr, items_str.c_str(),
            popup_max_height_in_items
        );
        
    if(item_nr < (int) items.size()) {
        *current_item = items[item_nr];
    } else {
        *current_item = "";
    }
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Creates two ImGui drag ints, one that sets the number of minutes, one that
 * sets the number of seconds. Though with some arguments, this can be changed
 * to hours and minutes.
 * Returns true if either value was changed.
 * label:
 *   Widget label.
 * total_amount:
 *   Time in the total amount of seconds. Or minutes, or whatever the lowest
 *   unit represent is.
 * format1:
 *   String to write in front of the first component's value.
 * format2:
 *   String to write in front of the second component's value.
 * limit1:
 *   Maximum value for the first component.
 * limit2:
 *   Maximum value for the second component.
 */
bool ImGui::DragTime2(
    const string &label, int* total_amount,
    const string format1, const string format2,
    const int limit1, const int limit2
) {
    int part1 = std::floor(*total_amount / 60.0f);
    int part2 = *total_amount % 60;
    
    ImGui::BeginGroup();
    ImGui::PushID(label.c_str());
    
    //Part 1 (hours or minutes) value.
    ImGui::SetNextItemWidth(80);
    ImGui::PushID(1);
    string format = "%02d" + format1;
    bool result =
        ImGui::DragInt("", &part1, 0.1f, 0, limit1, format.c_str());
    part1 = std::max(0, part1);
    part1 = std::min(part1, limit1);
    ImGui::PopID();
    
    //Part 2 (seconds or minutes) value.
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::PushID(2);
    format = "%02d" + format2;
    result |=
        ImGui::DragInt(label.c_str(), &part2, 0.1f, 0, limit2, format.c_str());
    part2 = std::max(0, part2);
    part2 = std::min(part2, limit2);
    ImGui::PopID();
    
    ImGui::PopID();
    ImGui::EndGroup();
    
    *total_amount = part1 * 60 + part2;
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Helps creating an ImGui ImageButton, followed by a centered Text.
 * icon:
 *   Icon to show on the button.
 * icon_size:
 *   Width and height of the icon.
 * button_padding:
 *   Padding between the icon and button edges.
 * text:
 *   The button's text.
 */
bool ImGui::ImageButtonAndText(
    ALLEGRO_BITMAP* icon, const ImVec2 &icon_size, const float button_padding,
    const string &text
) {
    ImGui::BeginGroup();
    
    bool result =
        ImGui::ImageButton(
            icon, icon_size,
            ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
            button_padding
        );
        
    float offset = (icon_size.y + button_padding * 2 - 16.0f) / 2.0f;
    offset -= 3.0f; //It's 3.0 too far, with the group + dummy approach.
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(0.0f, offset));
    ImGui::Text("%s", text.c_str());
    ImGui::EndGroup();
    
    ImGui::EndGroup();
    
    return result;
}


/* ----------------------------------------------------------------------------
 * Helps creating an ImGui list box, using a vector of strings for the
 * list of items.
 * label:
 *   ListBox widget label.
 * current_item:
 *   Index number of the current selected item.
 * items:
 *   List of items.
 * height_in_items:
 *   Maximum height, in number of items.
 */
bool ImGui::ListBox(
    const string &label, int* current_item, const vector<string> &items,
    const int height_in_items
) {
    const char** array = new const char* [items.size()];
    
    for(size_t i = 0; i < items.size(); ++i) {
        array[i] = items[i].c_str();
    }
    
    return
        ImGui::ListBox(
            label.c_str(), current_item, array, (int) items.size(), height_in_items
        );
}


/* ----------------------------------------------------------------------------
 * Resets some variables inside the ImGui namespace.
 */
void ImGui::Reset() {
    ImGuiIO &io = ImGui::GetIO();
    
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    io.MouseWheel = 0.0f;
    io.MouseWheelH = 0.0f;
    for(size_t b = 0; b < IM_ARRAYSIZE(io.MouseDown); ++b) {
        io.MouseDown[b] = false;
    }
    
    io.KeyCtrl = false;
    io.KeyShift = false;
    io.KeyAlt = false;
    io.KeySuper = false;
    
    io.AddKeyEvent(ImGuiKey_Escape, false);
    io.AddKeyEvent(ImGuiKey_ModCtrl, false);
    io.AddKeyEvent(ImGuiKey_ModShift, false);
    io.AddKeyEvent(ImGuiKey_ModAlt, false);
    io.AddKeyEvent(ImGuiKey_ModSuper, false);
}


/* ----------------------------------------------------------------------------
 * Prepares the "cursor X" so that the next widgets will be centered.
 * upcoming_items_width:
 *   Width of the items that will belong to this line.
 */
void ImGui::SetupCentering(const int upcoming_items_width) {
    int window_width = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX((window_width - upcoming_items_width) * 0.5f);
}
