/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area editor drawing function.
 */

#include <algorithm>

#include "editor.h"

#include "../../imgui/imgui_impl_allegro5.h"
#include "../../game.h"


/* ----------------------------------------------------------------------------
 * Handles the drawing part of the main loop of the area editor.
 */
void area_editor::do_drawing() {
    //Draw the GUI first.
    ImGui::Render();
    
    al_clear_to_color(al_map_rgb(0, 0, 0));
    ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
    
    //And now the canvas.
    al_use_transform(&game.world_to_screen_transform);
    al_set_clipping_rectangle(
        canvas_tl.x, canvas_tl.y,
        canvas_br.x - canvas_tl.x, canvas_br.y - canvas_tl.y
    );
    
    draw_canvas();
    
    al_reset_clipping_rectangle();
    al_use_transform(&game.identity_transform);
    
    //And the fade manager atop it all.
    game.fade_mgr.draw();
    
    //Finally, swap buffers.
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Draw the canvas.
 */
void area_editor::draw_canvas() {
}
