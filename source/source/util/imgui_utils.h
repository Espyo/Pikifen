/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for Dear ImGui-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#pragma once

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../lib/imgui/imgui_impl_allegro5.h"
#include "geometry_utils.h"


using std::string;
using std::vector;


namespace ImGui {

void AdjustColorHSV(ImVec4& color, float hDelta, float sDelta, float vDelta);
bool Combo(
    const string& label, int* currentItem, const vector<string>& items,
    int popupMaxHeightInItems = -1
);
bool Combo(
    const string& label, string* currentItem, const vector<string>& items,
    int popupMaxHeightInItems = -1
);
bool Combo(
    const string& label, string* currentItem,
    const vector<string>& itemInternalValues,
    const vector<string>& itemDisplayNames,
    int popupMaxHeightInItems = -1
);
bool DragTime2(
    const string& label, int* totalAmount,
    const string& format1 = "m", const string& format2 = "s",
    int limit1 = INT_MAX, int limit2 = 59
);
void FocusOnInputText(bool& condition);
void Image(
    ALLEGRO_BITMAP* bitmap, const Point& bitmapSize,
    const Point& uv0 = Point(), const Point& uv1 = Point(1.0f),
    const ALLEGRO_COLOR& tintCol = al_map_rgb(255, 255, 255)
);
bool ImageButton(
    const string& strId, ALLEGRO_BITMAP* bitmap, const Point& bitmapSize,
    const Point& uv0 = Point(), const Point& uv1 = Point(1.0f),
    const ALLEGRO_COLOR& bgCol = al_map_rgba(0, 0, 0, 0),
    const ALLEGRO_COLOR& tintCol = al_map_rgb(255, 255, 255)
);
bool ImageButtonOrganized(
    const string& strId, ALLEGRO_BITMAP* bitmap,
    const Point& maxBitmapSize, const Point& buttonSize,
    const ALLEGRO_COLOR& bgCol = al_map_rgba(0, 0, 0, 0),
    const ALLEGRO_COLOR& tintCol = al_map_rgb(255, 255, 255)
);
bool ImageButtonAndText(
    const string& id, ALLEGRO_BITMAP* icon, const Point& iconSize,
    float buttonPadding, const string& text
);
bool ListBox(
    const string& label, int* currentItem, const vector<string>& items,
    int heightInItems = -1
);
void Reset();
void SetupCentering(int upcomingItemsWidth);
void SetupButtonWrapping(
    int nextButtonWidth, int nextButtonIdx, int totalNButtons
);
void Spacer();

};
