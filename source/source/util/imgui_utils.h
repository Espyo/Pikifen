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

bool Combo(
    const string &label, int* current_item, const vector<string> &items,
    int popup_max_height_in_items = -1
);
bool Combo(
    const string &label, string* current_item, const vector<string> &items,
    int popup_max_height_in_items = -1
);
bool Combo(
    const string &label, string* current_item,
    const vector<string> &item_internal_values,
    const vector<string> &item_display_names,
    int popup_max_height_in_items = -1
);
bool DragTime2(
    const string &label, int* total_amount,
    const string &format1 = "m", const string &format2 = "s",
    int limit1 = INT_MAX, int limit2 = 59
);
void FocusOnInputText(bool &condition);
void Image(
    ALLEGRO_BITMAP* bitmap, const point &bitmap_size,
    const point &uv0 = point(), const point &uv1 = point(1.0f),
    const ALLEGRO_COLOR &tint_col = al_map_rgb(255, 255, 255),
    const ALLEGRO_COLOR &border_col = al_map_rgba(0, 0, 0, 0)
);
bool ImageButton(
    const string &str_id, ALLEGRO_BITMAP* bitmap, const point &bitmap_size,
    const point &uv0 = point(), const point &uv1 = point(1.0f),
    const ALLEGRO_COLOR &bg_col = al_map_rgba(0, 0, 0, 0),
    const ALLEGRO_COLOR &tint_col = al_map_rgb(255, 255, 255)
);
bool ImageButtonAndText(
    const string &id, ALLEGRO_BITMAP* icon, const point &icon_size,
    float button_padding, const string &text
);
bool ListBox(
    const string &label, int* current_item, const vector<string> &items,
    int height_in_items = -1
);
void Reset();
void SetupCentering(int upcoming_items_width);
void SetupButtonWrapping(
    int next_button_width, int next_button_idx, int total_n_buttons
);
void Spacer();

};
