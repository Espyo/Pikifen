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

#ifndef IMGUI_UTILS_INCLUDED
#define IMGUI_UTILS_INCLUDED

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../imgui/imgui_impl_allegro5.h"


using std::string;
using std::vector;


namespace ImGui {

bool Combo(
    const string &label, int* current_item, const vector<string> &items,
    const int popup_max_height_in_items = -1
);
bool Combo(
    const string &label, string* current_item, const vector<string> &items,
    const int popup_max_height_in_items = -1
);
bool ImageButtonAndText(
    ALLEGRO_BITMAP* icon, const ImVec2 &icon_size, const float button_padding,
    const string &text
);
bool ListBox(
    const string &label, int* current_item, const vector<string> &items,
    const int height_in_items = -1
);
void Reset();
void SetupCentering(const int upcoming_items_width);

};


#endif //ifndef IMGUI_UTILS_INCLUDED
