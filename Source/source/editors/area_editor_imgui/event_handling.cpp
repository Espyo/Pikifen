/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor event handler function.
 */

#include "editor.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_allegro5.h"


/* ----------------------------------------------------------------------------
 * Handles a raw Allegro event.
 */
void area_editor_imgui::handle_event(ALLEGRO_EVENT *ev) {
    ImGui_ImplAllegro5_ProcessEvent(ev);
}
