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

#include <algorithm>
#include <cmath>

#include "imgui_utils.h"

#include "allegro_utils.h"
#include "math_utils.h"


/**
 * @brief Adjusts the hue, saturation, and value of a given Dear ImGui color.
 *
 * @param color Color to edit.
 * @param hDelta Hue amount [0 - 1] to add or subtract.
 * @param sDelta Saturation amount [0 - 1] to add or subtract.
 * @param vDelta Value amount [0 - 1] to add or subtract.
 */
void ImGui::AdjustColorHSV(
    ImVec4& color, float hDelta, float sDelta, float vDelta
) {
    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(color.x, color.y, color.z, h, s, v);
    h += hDelta;
    s += sDelta;
    v += vDelta;
    ImGui::ColorConvertHSVtoRGB(h, s, v, color.x, color.y, color.z);
}


/**
 * @brief Wrapper for creating a Dear ImGui combo box widget, but
 * using a vector of strings for the list of items.
 *
 * @param label Combo widget label.
 * @param currentItem Index number of the current selected item. -1 means none.
 * @param items List of items.
 * @param popupMaxHeightInItems Maximum height of the popup,
 * in number of items.
 * @return Whether the value was changed.
 */
bool ImGui::Combo(
    const string& label, int* currentItem, const vector<string>& items,
    int popupMaxHeightInItems
) {
    string itemsStr;
    for(size_t i = 0; i < items.size(); i++) {
        itemsStr += items[i] + '\0';
    }
    
    return
        ImGui::Combo(
            label.c_str(), currentItem, itemsStr.c_str(),
            popupMaxHeightInItems
        );
}


/**
 * @brief Wrapper for creating a Dear ImGui combo box widget, but
 * using a string to control the selection,
 * as well as a vector of strings for the list of items.
 *
 * @param label Combo widget label.
 * @param currentItem Name of the current selected item.
 * @param items List of items.
 * @param popupMaxHeightInItems Maximum height of the popup,
 * in number of items.
 * @return Whether the value was changed.
 */
bool ImGui::Combo(
    const string& label, string* currentItem, const vector<string>& items,
    int popupMaxHeightInItems
) {

    string itemsStr;
    int itemIdx = -1;
    for(size_t i = 0; i < items.size(); i++) {
        itemsStr += items[i] + '\0';
        if(*currentItem == items[i]) {
            itemIdx = (int) i;
        }
    }
    
    bool result =
        ImGui::Combo(
            label.c_str(), &itemIdx, itemsStr.c_str(),
            popupMaxHeightInItems
        );
        
    if(itemIdx >= 0 && itemIdx < (int) items.size()) {
        *currentItem = items[itemIdx];
    } else {
        *currentItem = "";
    }
    
    return result;
}


/**
 * @brief Wrapper for creating a Dear ImGui combo box widget, but
 * using a string to control the selection,
 * as well as two vector of strings for the list of items, one with
 * the internal values of each item, another with the names to display.
 *
 * @param label Combo widget label.
 * @param currentItem Internal value of the current selected item.
 * @param itemInternalValues List of internal values for each item.
 * @param itemDisplayNames List of names to show the user for each item.
 * @param popupMaxHeightInItems Maximum height of the popup,
 * in number of items.
 * @return Whether the value was changed.
 */
bool ImGui::Combo(
    const string& label, string* currentItem,
    const vector<string>& itemInternalValues,
    const vector<string>& itemDisplayNames,
    int popupMaxHeightInItems
) {
    int currentItemIdx = -1;
    for(size_t i = 0; i < itemInternalValues.size(); i++) {
        if(itemInternalValues[i] == *currentItem) {
            currentItemIdx = (int) i;
            break;
        }
    }
    
    bool result =
        ImGui::Combo(
            label, &currentItemIdx, itemDisplayNames,
            popupMaxHeightInItems
        );
        
    if(currentItemIdx == -1) {
        currentItem->clear();
    } else {
        *currentItem = itemInternalValues[currentItemIdx];
    }
    
    return result;
}


/**
 * @brief Creates two Dear ImGui drag int widgets, one that sets the
 * number of minutes, one that sets the number of seconds.
 * Though with some arguments, this can be changed to hours and minutes.
 *
 * @param label Widget label.
 * @param totalAmount Time in the total amount of seconds.
 * Or minutes, or whatever the lowest unit represent is.
 * @param format1 String to write in front of the first component's value.
 * @param format2 String to write in front of the second component's value.
 * @param limit1 Maximum value for the first component.
 * @param limit2 Maximum value for the second component.
 * @return Whether either value was changed.
 */
bool ImGui::DragTime2(
    const string& label, int* totalAmount,
    const string& format1, const string& format2,
    int limit1, int limit2
) {
    int part1 = floor(*totalAmount / 60.0f);
    int part2 = *totalAmount % 60;
    
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
    
    *totalAmount = part1 * 60 + part2;
    
    return result;
}


/**
 * @brief Makes it so Dear ImGui focuses on the next input text widget.
 *
 * @param condition Only focus if this boolean is true. After setting the focus,
 * this boolean is set to false. This is done so that Dear ImGui only focuses
 * when you want, like when the dialog is first shown, instead of doing it
 * every frame.
 */
void ImGui::FocusOnInputText(bool& condition) {
    if(!ImGui::IsAnyItemActive() && condition) {
        ImGui::SetKeyboardFocusHere();
        condition = false;
    }
}


/**
 * @brief Wrapper for creating a Dear ImGui combo image widget
 * (with background), but using Allegro bitmaps.
 *
 * @param bitmap Bitmap to show on the button.
 * @param bitmapSize Width and height of the bitmap.
 * @param uv0 UV coordinates of the top-left coordinate.
 * @param uv1 UV coordinates of the bottom-right coordinate.
 * @param tintCol Tint color.
 * @return Whether the button was pressed.
 */
void ImGui::Image(
    ALLEGRO_BITMAP* bitmap, const Point& bitmapSize,
    const Point& uv0, const Point& uv1,
    const ALLEGRO_COLOR& tintCol
) {
    ImGui::ImageWithBg(
        (ImTextureID) (intptr_t) bitmap,
        ImVec2(bitmapSize.x, bitmapSize.y),
        ImVec2(uv0.x, uv0.y),
        ImVec2(uv1.x, uv1.y),
        ImVec4(0, 0, 0, 0),
        ImVec4(tintCol.r, tintCol.g, tintCol.b, tintCol.a)
    );
}


/**
 * @brief Wrapper for creating a Dear ImGui image button widget, but
 * using Allegro bitmaps.
 *
 * @param strId Button widget ID.
 * @param bitmap Bitmap to show on the button.
 * @param bitmapSize Size to display the bitmap at in the GUI.
 * @param uv0 UV coordinates of the top-left coordinate.
 * @param uv1 UV coordinates of the bottom-right coordinate.
 * @param bgCol Bitmap background color.
 * @param tintCol Bitmap tint color.
 * @return Whether the button was pressed.
 */
bool ImGui::ImageButton(
    const string& strId, ALLEGRO_BITMAP* bitmap, const Point& bitmapSize,
    const Point& uv0, const Point& uv1,
    const ALLEGRO_COLOR& bgCol,
    const ALLEGRO_COLOR& tintCol
) {
    return
        ImGui::ImageButton(
            strId.c_str(), (ImTextureID) (intptr_t) bitmap,
            ImVec2(bitmapSize.x, bitmapSize.y),
            ImVec2(uv0.x, uv0.y),
            ImVec2(uv1.x, uv1.y),
            ImVec4(bgCol.r, bgCol.g, bgCol.b, bgCol.a),
            ImVec4(tintCol.r, tintCol.g, tintCol.b, tintCol.a)
        );
}


/**
 * @brief Wrapper for creating a Dear ImGui image button widget, followed
 * by a text widget.
 *
 * @param id Button widget ID.
 * @param icon Icon to show on the button.
 * @param iconSize Size to display the bitmap at in the GUI.
 * @param buttonPadding Padding between the icon and button edges.
 * @param text The button's text.
 * @return Whether the button was pressed.
 */
bool ImGui::ImageButtonAndText(
    const string& id, ALLEGRO_BITMAP* icon, const Point& iconSize,
    float buttonPadding, const string& text
) {
    ImGui::BeginGroup();
    
    ImGui::PushStyleVar(
        ImGuiStyleVar_FramePadding, ImVec2(buttonPadding, buttonPadding)
    );
    bool result = ImGui::ImageButton(id, icon, iconSize);
    ImGui::PopStyleVar();
    
    float offset = (iconSize.y + buttonPadding * 2 - 16.0f) / 2.0f;
    offset -= 3.0f; //It's 3.0 too far, with the group + dummy approach.
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(0.0f, offset));
    ImGui::Text("%s", text.c_str());
    ImGui::EndGroup();
    
    ImGui::EndGroup();
    
    return result;
}


/**
 * @brief Wrapper for creating a Dear ImGui image button widget, but
 * using Allegro bitmaps, and keeping the bitmap centered and in proportion,
 * while also allowing the button size to be specified.
 *
 * @param strId Button widget ID.
 * @param bitmap Bitmap to show on the button.
 * @param maxBitmapSize Maximum size to display the bitmap at in the GUI.
 * @param buttonSize Size of the button.
 * @param bgCol Bitmap background color.
 * @param tintCol Bitmap tint color.
 * @return Whether the button was pressed.
 */
bool ImGui::ImageButtonOrganized(
    const string& strId, ALLEGRO_BITMAP* bitmap,
    const Point& maxBitmapSize, const Point& buttonSize,
    const ALLEGRO_COLOR& bgCol, const ALLEGRO_COLOR& tintCol
) {
    Point finalBmpSize =
        resizeToBoxKeepingAspectRatio(
            getBitmapDimensions(bitmap), maxBitmapSize
        );
        
    Point padding = (buttonSize - finalBmpSize) / 2.0f;
    
    PushStyleVar(
        ImGuiStyleVar_FramePadding, ImVec2(padding.x, padding.y)
    );
    bool result = ImageButton(strId, bitmap, finalBmpSize);
    PopStyleVar();
    
    return result;
}


/**
 * @brief Wrapper for creating a Dear ImGui list box widget, but
 * using a vector of strings for the list of items.
 *
 * @param label ListBox widget label.
 * @param currentItem Index number of the current selected item.
 * @param items List of items.
 * @param heightInItems Maximum height, in number of items.
 * @return Whether the value was changed.
 */
bool ImGui::ListBox(
    const string& label, int* currentItem, const vector<string>& items,
    int heightInItems
) {
    //TODO check if items is empty
    const char** array = new const char* [items.size()];
    
    for(size_t i = 0; i < items.size(); i++) {
        array[i] = items[i].c_str();
    }
    
    return
        ImGui::ListBox(
            label.c_str(),
            currentItem, array, (int) items.size(),
            heightInItems
        );
    //TODO free "array"
}


/**
 * @brief Resets some variables inside the ImGui namespace.
 */
void ImGui::Reset() {
    ImGuiIO& io = ImGui::GetIO();
    
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    io.MouseWheel = 0.0f;
    io.MouseWheelH = 0.0f;
    for(size_t b = 0; b < IM_ARRAYSIZE(io.MouseDown); b++) {
        io.MouseDown[b] = false;
    }
    
    io.KeyCtrl = false;
    io.KeyShift = false;
    io.KeyAlt = false;
    io.KeySuper = false;
    
    io.AddKeyEvent(ImGuiKey_Escape, false);
    io.AddKeyEvent(ImGuiKey_LeftCtrl, false);
    io.AddKeyEvent(ImGuiKey_RightCtrl, false);
    io.AddKeyEvent(ImGuiKey_LeftShift, false);
    io.AddKeyEvent(ImGuiKey_RightShift, false);
    io.AddKeyEvent(ImGuiKey_LeftAlt, false);
    io.AddKeyEvent(ImGuiKey_RightAlt, false);
    io.AddKeyEvent(ImGuiKey_LeftSuper, false);
    io.AddKeyEvent(ImGuiKey_RightSuper, false);
    io.AddKeyEvent(ImGuiMod_Alt, false);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    io.AddKeyEvent(ImGuiMod_Shift, false);
    io.AddKeyEvent(ImGuiMod_Super, false);
}


/**
 * @brief Prepares the state of the GUI to either place the next button
 * on the same line, or to break to a new line if it wouldn't fit.
 *
 * @param nextButtonWidth Width of the next button.
 * @param nextButtonIdx Index of the next button.
 * @param totalNButtons Total amount of buttons.
 */
void ImGui::SetupButtonWrapping(
    int nextButtonWidth, int nextButtonIdx, int totalNButtons
) {
    float lastX2 =
        ImGui::GetItemRectMax().x;
    float nextX2 =
        lastX2 + ImGui::GetStyle().ItemSpacing.x + nextButtonWidth;
    float windowX2 =
        GetCursorScreenPos().x + GetContentRegionAvail().x;
    if(nextButtonIdx < totalNButtons && nextX2 < windowX2) {
        ImGui::SameLine();
    }
}


/**
 * @brief Prepares the "cursor X" so that the next widgets will be centered.
 *
 * @param upcomingItemsWidth Width of the items that will belong to this line.
 */
void ImGui::SetupCentering(int upcomingItemsWidth) {
    int windowWidth = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosX((windowWidth - upcomingItemsWidth) * 0.5f);
}


/**
 * @brief Processes a SliderAngle widget but also adds a context menu with
 * some helpful tools.
 *
 * @param label Widget label.
 * @param vRad Value, in radians.
 * @param vDegreesMin Minimum value.
 * @param vDegreesMax Maximum value.
 * @param format Text format.
 * @param flags Flags.
 * @return Whether the value was changed, be it from the widget or the
 * context menu tools.
 */
bool ImGui::SliderAngleWithContext(
    const char* label, float* vRad, float vDegreesMin,
    float vDegreesMax, const char* format, ImGuiSliderFlags flags
) {
    bool changed =
        ImGui::SliderAngle(
            label, vRad, vDegreesMin, vDegreesMax, format, flags
        );
    if(ImGui::BeginPopupContextItem()) {
        //East selectable.
        if(ImGui::Selectable("East (0)")) {
            *vRad = 0;
            changed = true;
        }
        
        //South selectable.
        if(ImGui::Selectable("South (90)")) {
            *vRad = TAU * 0.25f;
            changed = true;
        }
        
        //West selectable.
        if(ImGui::Selectable("West (180)")) {
            *vRad = TAU * 0.50f;
            changed = true;
        }
        
        //North selectable.
        if(ImGui::Selectable("North (270)")) {
            *vRad = TAU * 0.75f;
            changed = true;
        }
        
        //+90 selectable.
        if(ImGui::Selectable("Quarter clockwise (+90)")) {
            *vRad = normalizeAngle(*vRad + TAU * 0.25f);
            changed = true;
        }
        
        //+180 selectable.
        if(ImGui::Selectable("Turn around (+180)")) {
            *vRad = normalizeAngle(*vRad + TAU * 0.50f);
            changed = true;
        }
        
        //-90 selectable.
        if(ImGui::Selectable("Quarter counterclockwise (-90)")) {
            *vRad = normalizeAngle(*vRad - TAU * 0.25);
            changed = true;
        }
        
        ImGui::EndPopup();
    }
    return changed;
}


/**
 * @brief Places a dummy widget designed to space things vertically.
 */
void ImGui::Spacer() {
    ImGui::Dummy(ImVec2(0, 16));
}
