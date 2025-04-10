/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion menu classes and functions.
 */

#include <algorithm>

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"
#include "gameplay.h"


using DrawInfo = GuiItem::DrawInfo;


namespace ONION_MENU {

//Name of the GUI information file.
const string GUI_FILE_NAME = "onion_menu";

//How long to let text turn red for.
const float RED_TEXT_DURATION = 1.0f;

//The Onion menu can only show, at most, these many Pikmin types per page.
const size_t TYPES_PER_PAGE = 5;

}


/**
 * @brief Constructs a new Onion menu struct object.
 *
 * @param n_ptr Pointer to the nest information struct.
 * @param l_ptr Leader responsible.
 */
OnionMenu::OnionMenu(
    PikminNest* n_ptr, Leader* l_ptr
) :
    n_ptr(n_ptr),
    l_ptr(l_ptr) {
    
    for(size_t t = 0; t < n_ptr->nest_type->pik_types.size(); t++) {
        types.push_back(
            OnionMenuPikminType(t, n_ptr->nest_type->pik_types[t])
        );
    }
    
    nr_pages = ceil(types.size() / (float) ONION_MENU::TYPES_PER_PAGE);
    
    gui.registerCoords("instructions",     50,  7, 90, 20);
    gui.registerCoords("cancel",           16, 85, 18, 11);
    gui.registerCoords("cancel_input",      8, 89,  4,  4);
    gui.registerCoords("ok",               84, 85, 18, 11);
    gui.registerCoords("field",            50, 75, 18,  4);
    gui.registerCoords("select_all",       50, 87, 24,  6);
    gui.registerCoords("onion_1_button",   50, 20,  9, 12);
    gui.registerCoords("onion_2_button",   50, 20,  9, 12);
    gui.registerCoords("onion_3_button",   50, 20,  9, 12);
    gui.registerCoords("onion_4_button",   50, 20,  9, 12);
    gui.registerCoords("onion_5_button",   50, 20,  9, 12);
    gui.registerCoords("onion_1_amount",   50, 29, 12,  4);
    gui.registerCoords("onion_2_amount",   50, 29, 12,  4);
    gui.registerCoords("onion_3_amount",   50, 29, 12,  4);
    gui.registerCoords("onion_4_amount",   50, 29, 12,  4);
    gui.registerCoords("onion_5_amount",   50, 29, 12,  4);
    gui.registerCoords("group_1_button",   50, 60,  9, 12);
    gui.registerCoords("group_2_button",   50, 60,  9, 12);
    gui.registerCoords("group_3_button",   50, 60,  9, 12);
    gui.registerCoords("group_4_button",   50, 60,  9, 12);
    gui.registerCoords("group_5_button",   50, 60,  9, 12);
    gui.registerCoords("group_1_amount",   50, 51, 12,  4);
    gui.registerCoords("group_2_amount",   50, 51, 12,  4);
    gui.registerCoords("group_3_amount",   50, 51, 12,  4);
    gui.registerCoords("group_4_amount",   50, 51, 12,  4);
    gui.registerCoords("group_5_amount",   50, 51, 12,  4);
    gui.registerCoords("onion_all",        50, 20,  9, 12);
    gui.registerCoords("group_all",        50, 60,  9, 12);
    gui.registerCoords("prev_page",         5, 40,  8, 10);
    gui.registerCoords("next_page",        95, 40,  8, 11);
    gui.registerCoords("onion_left_more",   5, 20,  3,  4);
    gui.registerCoords("onion_right_more", 95, 20,  3,  4);
    gui.registerCoords("group_left_more",   5, 60,  3,  4);
    gui.registerCoords("group_right_more", 95, 60,  3,  4);
    gui.registerCoords("tooltip",          50, 95, 95,  8);
    gui.readCoords(
        game.content.gui_defs.list[ONION_MENU::GUI_FILE_NAME].getChildByName("positions")
    );
    
    //Instructions text.
    TextGuiItem* instructions_text =
        new TextGuiItem(
        "Call or store Pikmin", game.sys_content.fnt_standard, al_map_rgb(188, 230, 230)
    );
    gui.addItem(instructions_text, "instructions");
    
    //Cancel button.
    gui.back_item =
        new ButtonGuiItem(
        "Cancel", game.sys_content.fnt_standard, al_map_rgb(226, 112, 112)
    );
    gui.back_item->on_activate =
    [this] (const Point &) {
        start_closing();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Forget all changes and leave the Onion menu."; };
    gui.addItem(gui.back_item, "cancel");
    
    //Cancel input icon.
    guiAddBackInputIcon(&gui, "cancel_input");
    
    //Ok button.
    ButtonGuiItem* ok_button =
        new ButtonGuiItem(
        "Ok", game.sys_content.fnt_standard, al_map_rgb(96, 226, 80)
    );
    ok_button->on_activate =
    [this] (const Point &) {
        confirm();
        start_closing();
    };
    ok_button->on_get_tooltip =
    [] () { return "Confirm changes."; };
    gui.addItem(ok_button, "ok");
    
    //Field amount text.
    field_amount_text =
        new TextGuiItem("", game.sys_content.fnt_standard);
    field_amount_text->on_draw =
    [this] (const DrawInfo & draw) {
        int total_delta = 0;
        for(size_t t = 0; t < this->types.size(); t++) {
            total_delta += this->types[t].delta;
        }
        
        drawFilledRoundedRectangle(
            draw.center, draw.size, game.win_w * 0.01,
            al_map_rgba(188, 230, 230, 128)
        );
        
        ALLEGRO_COLOR color = al_map_rgb(188, 230, 230);
        const auto &red_it = this->red_items.find(field_amount_text);
        if(red_it != this->red_items.end()) {
            color =
                interpolateColor(
                    red_it->second,
                    0, ONION_MENU::RED_TEXT_DURATION,
                    color, al_map_rgb(224, 0, 0)
                );
        }
        
        float juicy_grow_amount = field_amount_text->getJuiceValue();
        drawText(
            "Field: " +
            i2s(game.states.gameplay->mobs.pikmin_list.size() + total_delta),
            game.sys_content.fnt_standard, draw.center,
            draw.size * GUI::STANDARD_CONTENT_SIZE, color,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
            Point(1.0f + juicy_grow_amount)
        );
    };
    gui.addItem(field_amount_text, "field");
    
    //Select all checkbox.
    CheckGuiItem* select_all_check =
        new CheckGuiItem(
        &select_all,
        "Select all", game.sys_content.fnt_standard, al_map_rgb(188, 230, 230)
    );
    select_all_check->on_activate =
    [this, select_all_check] (const Point &) {
        growButtons();
        select_all_check->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        toggleSelectAll();
    };
    select_all_check->visible = types.size() > 1;
    select_all_check->selectable = types.size() > 1;
    select_all_check->on_get_tooltip =
    [] () { return "Control all Pikmin numbers at once?"; };
    gui.addItem(select_all_check, "select_all");
    
    //Onion icons and buttons.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        string id = "onion_" + i2s(t + 1) + "_button";
        
        GuiItem* onion_icon = new GuiItem(false);
        onion_icon->on_draw =
        [this, t, onion_icon] (const DrawInfo & draw) {
            OnionMenuPikminType* t_ptr = this->on_screen_types[t];
            if(t_ptr->pik_type->bmp_onion_icon) {
                float juicy_grow_amount = onion_icon->getJuiceValue();
                drawBitmapInBox(
                    t_ptr->pik_type->bmp_onion_icon, draw.center,
                    (draw.size * 0.8f) + juicy_grow_amount, true
                );
            }
        };
        gui.addItem(onion_icon, id);
        onion_icon_items.push_back(onion_icon);
        
        ButtonGuiItem* onion_button =
            new ButtonGuiItem("", game.sys_content.fnt_standard);
        onion_button->on_draw =
        [this, onion_button] (const DrawInfo & draw) {
            float juicy_grow_amount = onion_button->getJuiceValue();
            drawButton(
                draw.center,
                draw.size + juicy_grow_amount,
                "", game.sys_content.fnt_standard, COLOR_WHITE,
                onion_button->selected
            );
        };
        onion_button->on_activate =
        [this, t] (const Point &) {
            addToOnion(on_screen_types[t]->type_idx);
        };
        onion_button->can_auto_repeat = true;
        onion_button->on_get_tooltip =
        [this, t] () {
            OnionMenuPikminType* t_ptr = this->on_screen_types[t];
            return "Store one " + t_ptr->pik_type->name + " inside.";
        };
        gui.addItem(onion_button, id);
        onion_button_items.push_back(onion_button);
    }
    
    //Onion's all button.
    onion_all_button =
        new ButtonGuiItem("", game.sys_content.fnt_standard);
    onion_all_button->on_draw =
    [this] (const DrawInfo & draw) {
        float juicy_grow_amount = onion_all_button->getJuiceValue();
        drawButton(
            draw.center,
            draw.size + juicy_grow_amount,
            "", game.sys_content.fnt_standard, COLOR_WHITE,
            onion_all_button->selected
        );
    };
    onion_all_button->on_activate =
    [this] (const Point &) {
        addAllToOnion();
    };
    onion_all_button->can_auto_repeat = true;
    onion_all_button->on_get_tooltip =
    [] () { return "Store one Pikmin of each type inside."; };
    gui.addItem(onion_all_button, "onion_all");
    
    //Onion amounts.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        GuiItem* onion_amount_text = new GuiItem(false);
        onion_amount_text->on_draw =
            [this, t, onion_amount_text]
        (const DrawInfo & draw) {
            OnionMenuPikminType* t_ptr = this->on_screen_types[t];
            
            size_t real_onion_amount =
                this->n_ptr->getAmountByType(t_ptr->pik_type);
                
            drawFilledRoundedRectangle(
                draw.center, draw.size, game.win_w * 0.01,
                al_map_rgba(188, 230, 230, 128)
            );
            
            ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
            const auto &red_it = this->red_items.find(onion_amount_text);
            if(red_it != this->red_items.end()) {
                color =
                    interpolateColor(
                        red_it->second,
                        0, ONION_MENU::RED_TEXT_DURATION,
                        color, al_map_rgb(224, 0, 0)
                    );
            }
            
            float juicy_grow_amount = onion_amount_text->getJuiceValue();
            drawText(
                i2s(real_onion_amount - t_ptr->delta),
                game.sys_content.fnt_area_name, draw.center,
                draw.size * GUI::STANDARD_CONTENT_SIZE, color,
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0f + juicy_grow_amount)
            );
        };
        gui.addItem(onion_amount_text, "onion_" + i2s(t + 1) + "_amount");
        onion_amount_items.push_back(onion_amount_text);
    }
    
    //Group icons and buttons.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        string id = "group_" + i2s(t + 1) + "_button";
        
        GuiItem* group_icon = new GuiItem(false);
        group_icon->on_draw =
        [this, t, group_icon] (const DrawInfo & draw) {
            OnionMenuPikminType* t_ptr = this->on_screen_types[t];
            float juicy_grow_amount = group_icon->getJuiceValue();
            if(t_ptr->pik_type->bmp_icon) {
                drawBitmapInBox(
                    t_ptr->pik_type->bmp_icon, draw.center,
                    (draw.size * 0.8f) + juicy_grow_amount, true
                );
            }
        };
        gui.addItem(group_icon, id);
        group_icon_items.push_back(group_icon);
        
        ButtonGuiItem* group_button =
            new ButtonGuiItem("", game.sys_content.fnt_standard);
        group_button->on_draw =
        [this, group_button] (const DrawInfo & draw) {
            float juicy_grow_amount = group_button->getJuiceValue();
            drawButton(
                draw.center,
                draw.size + juicy_grow_amount,
                "", game.sys_content.fnt_standard, COLOR_WHITE,
                group_button->selected
            );
        };
        group_button->on_activate =
        [this, t] (const Point &) {
            addToGroup(on_screen_types[t]->type_idx);
        };
        group_button->can_auto_repeat = true;
        group_button->on_get_tooltip =
        [this, t] () {
            OnionMenuPikminType* t_ptr = this->on_screen_types[t];
            return "Call one " + t_ptr->pik_type->name + " to the group.";
        };
        gui.addItem(group_button, id);
        group_button_items.push_back(group_button);
    }
    
    //Group's all button.
    group_all_button =
        new ButtonGuiItem("", game.sys_content.fnt_standard);
    group_all_button->on_draw =
    [this] (const DrawInfo & draw) {
        float juicy_grow_amount = group_all_button->getJuiceValue();
        drawButton(
            draw.center,
            draw.size + juicy_grow_amount,
            "", game.sys_content.fnt_standard, COLOR_WHITE,
            group_all_button->selected
        );
    };
    group_all_button->on_activate =
    [this] (const Point &) {
        addAllToGroup();
    };
    group_all_button->can_auto_repeat = true;
    group_all_button->on_get_tooltip =
    [] () { return "Call one Pikmin of each type to the group."; };
    gui.addItem(group_all_button, "group_all");
    
    //Group amounts.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        GuiItem* group_amount_text = new GuiItem(false);
        group_amount_text->on_draw =
            [this, t, group_amount_text]
        (const DrawInfo & draw) {
            OnionMenuPikminType* t_ptr = this->on_screen_types[t];
            
            size_t real_group_amount =
                this->l_ptr->group->getAmountByType(t_ptr->pik_type);
                
            drawFilledRoundedRectangle(
                draw.center, draw.size, game.win_w * 0.01,
                al_map_rgba(188, 230, 230, 128)
            );
            
            ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
            const auto &red_it = this->red_items.find(group_amount_text);
            if(red_it != this->red_items.end()) {
                color =
                    interpolateColor(
                        red_it->second,
                        0, ONION_MENU::RED_TEXT_DURATION,
                        color, al_map_rgb(224, 0, 0)
                    );
            }
            
            float juicy_grow_amount = group_amount_text->getJuiceValue();
            drawText(
                i2s(real_group_amount + t_ptr->delta),
                game.sys_content.fnt_area_name, draw.center,
                draw.size * GUI::STANDARD_CONTENT_SIZE, color,
                ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, 0,
                Point(1.0f + juicy_grow_amount)
            );
        };
        gui.addItem(group_amount_text, "group_" + i2s(t + 1) + "_amount");
        group_amount_items.push_back(group_amount_text);
    }
    
    //Onion left "more" indicator.
    onion_more_l_icon = new GuiItem(false);
    onion_more_l_icon->on_draw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sys_content.bmp_more,
            draw.center,
            Point(-draw.size.x, draw.size.y) * 0.8f,
            0, mapGray(128)
        );
    };
    gui.addItem(onion_more_l_icon, "onion_left_more");
    
    //Onion right "more" indicator.
    onion_more_r_icon = new GuiItem(false);
    onion_more_r_icon->on_draw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sys_content.bmp_more,
            draw.center,
            draw.size * 0.8f,
            0, mapGray(128)
        );
    };
    gui.addItem(onion_more_r_icon, "onion_right_more");
    
    //Group left "more" indicator.
    group_more_l_icon = new GuiItem(false);
    group_more_l_icon->on_draw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sys_content.bmp_more,
            draw.center,
            Point(-draw.size.x, draw.size.y) * 0.8f,
            0, mapGray(128)
        );
    };
    gui.addItem(group_more_l_icon, "group_left_more");
    
    //Group right "more" indicator.
    group_more_r_icon = new GuiItem(false);
    group_more_r_icon->on_draw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sys_content.bmp_more,
            draw.center,
            draw.size * 0.8f,
            0, mapGray(128)
        );
    };
    gui.addItem(group_more_r_icon, "group_right_more");
    
    //Previous page button.
    prev_page_button = new GuiItem(true);
    prev_page_button->on_draw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sys_content.bmp_more,
            draw.center, Point(-draw.size.x, draw.size.y) * 0.5f
        );
        
        drawButton(
            draw.center, draw.size, "", game.sys_content.fnt_standard, COLOR_WHITE,
            prev_page_button->selected,
            prev_page_button->getJuiceValue()
        );
    };
    prev_page_button->on_activate =
    [this] (const Point &) {
        goToPage(sumAndWrap((int) page, -1, (int) nr_pages));
    };
    prev_page_button->visible = nr_pages > 1;
    prev_page_button->selectable = nr_pages > 1;
    prev_page_button->on_get_tooltip =
    [] () { return "Go to the previous page of Pikmin types."; };
    gui.addItem(prev_page_button, "prev_page");
    
    //Next page button.
    next_page_button = new GuiItem(true);
    next_page_button->on_draw =
    [this] (const DrawInfo & draw) {
        drawBitmap(
            game.sys_content.bmp_more,
            draw.center, draw.size * 0.5f
        );
        
        drawButton(
            draw.center, draw.size, "", game.sys_content.fnt_standard, COLOR_WHITE,
            next_page_button->selected,
            next_page_button->getJuiceValue()
        );
    };
    next_page_button->on_activate =
    [this] (const Point &) {
        goToPage(sumAndWrap((int) page, 1, (int) nr_pages));
    };
    next_page_button->visible = nr_pages > 1;
    next_page_button->selectable = nr_pages > 1;
    next_page_button->on_get_tooltip =
    [] () { return "Go to the next page of Pikmin types."; };
    gui.addItem(next_page_button, "next_page");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&gui);
    gui.addItem(tooltip_text, "tooltip");
    
    //Finishing touches.
    update();
    gui.startAnimation(
        GUI_MANAGER_ANIM_UP_TO_CENTER,
        GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
}


/**
 * @brief Destroys the Onion menu struct object.
 */
OnionMenu::~OnionMenu() {
    gui.destroy();
}


/**
 * @brief Adds one Pikmin of each type from Onion to the group, if possible.
 */
void OnionMenu::addAllToGroup() {
    for(size_t t = 0; t < types.size(); t++) {
        addToGroup(t);
    }
}


/**
 * @brief Adds one Pikmin of each type from the group to the Onion, if possible.
 */
void OnionMenu::addAllToOnion() {
    for(size_t t = 0; t < types.size(); t++) {
        addToOnion(t);
    }
}


/**
 * @brief Adds one Pikmin from the Onion to the group, if possible.
 *
 * @param type_idx Index of the Onion's Pikmin type.
 */
void OnionMenu::addToGroup(size_t type_idx) {
    size_t real_onion_amount =
        n_ptr->getAmountByType(n_ptr->nest_type->pik_types[type_idx]);
        
    //First, check if there are enough in the Onion to take out.
    if((signed int) (real_onion_amount - types[type_idx].delta) <= 0) {
        size_t screen_idx = types[type_idx].on_screen_idx;
        if(screen_idx != INVALID) {
            makeGuiItemRed(onion_amount_items[screen_idx]);
        }
        return;
    }
    
    //Next, check if the addition won't make the field amount hit the limit.
    int total_delta = 0;
    for(size_t t = 0; t < types.size(); t++) {
        total_delta += types[t].delta;
    }
    if(
        game.states.gameplay->mobs.pikmin_list.size() + total_delta >=
        game.config.rules.max_pikmin_in_field
    ) {
        makeGuiItemRed(field_amount_text);
        return;
    }
    
    types[type_idx].delta++;
    
    size_t on_screen_idx = types[type_idx].on_screen_idx;
    if(on_screen_idx != INVALID) {
        onion_amount_items[on_screen_idx]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
        );
        group_amount_items[on_screen_idx]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
        );
    }
    field_amount_text->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
    );
    
}


/**
 * @brief Adds one Pikmin from the group to the Onion, if possible.
 *
 * @param type_idx Index of the Onion's Pikmin type.
 */
void OnionMenu::addToOnion(size_t type_idx) {
    size_t real_group_amount =
        l_ptr->group->getAmountByType(n_ptr->nest_type->pik_types[type_idx]);
        
    if((signed int) (real_group_amount + types[type_idx].delta) <= 0) {
        size_t screen_idx = types[type_idx].on_screen_idx;
        if(screen_idx != INVALID) {
            makeGuiItemRed(group_amount_items[screen_idx]);
        }
        return;
    }
    
    types[type_idx].delta--;
    
    size_t on_screen_idx = types[type_idx].on_screen_idx;
    if(on_screen_idx != INVALID) {
        onion_amount_items[on_screen_idx]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
        );
        group_amount_items[on_screen_idx]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
        );
    }
    field_amount_text->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_TEXT_MEDIUM
    );
}


/**
 * @brief Confirms the player's changes, and sets up the Pikmin to climb up the
 * Onion, if any, and sets up the Onion to spit out Pikmin, if any.
 */
void OnionMenu::confirm() {
    for(size_t t = 0; t < types.size(); t++) {
        if(types[t].delta > 0) {
            n_ptr->requestPikmin(t, types[t].delta, l_ptr);
        } else if(types[t].delta < 0) {
            l_ptr->orderPikminToOnion(
                types[t].pik_type, n_ptr, -types[t].delta
            );
        }
    }
}


/**
 * @brief Flips to the specified page of Pikmin types.
 *
 * @param page Index of the new page.
 */
void OnionMenu::goToPage(size_t page) {
    this->page = page;
    growButtons();
    update();
}


/**
 * @brief Makes the Onion and group buttons juicy grow.
 */
void OnionMenu::growButtons() {
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        onion_icon_items[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
        onion_button_items[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
        group_icon_items[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
        group_button_items[t]->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_ICON
        );
    }
    onion_all_button->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_ICON
    );
    group_all_button->startJuiceAnimation(
        GuiItem::JUICE_TYPE_GROW_ICON
    );
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void OnionMenu::handleAllegroEvent(const ALLEGRO_EVENT &ev) {
    if(!closing) gui.handleAllegroEvent(ev);
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void OnionMenu::handlePlayerAction(const PlayerAction &action) {
    gui.handlePlayerAction(action);
}


/**
 * @brief Makes a given GUI item turn red.
 *
 * @param item The item.
 */
void OnionMenu::makeGuiItemRed(GuiItem* item) {
    red_items[item] = ONION_MENU::RED_TEXT_DURATION;
}


/**
 * @brief Starts the closing process.
 */
void OnionMenu::start_closing() {
    closing = true;
    closing_timer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
    gui.startAnimation(
        GUI_MANAGER_ANIM_CENTER_TO_UP, GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
    game.states.gameplay->hud->gui.startAnimation(
        GUI_MANAGER_ANIM_OUT_TO_IN,
        GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void OnionMenu::tick(float delta_t) {

    //Correct the amount of wanted group members, if they are invalid.
    int total_delta = 0;
    
    for(size_t t = 0; t < n_ptr->nest_type->pik_types.size(); t++) {
        //Get how many the player really has with them.
        int real_group_amount =
            (int) l_ptr->group->getAmountByType(
                n_ptr->nest_type->pik_types[t]
            );
            
        //Make sure the player can't request to store more than what they have.
        types[t].delta = std::max(-real_group_amount, (int) types[t].delta);
        
        //Get how many are really in the Onion.
        int real_onion_amount =
            (int) n_ptr->getAmountByType(n_ptr->nest_type->pik_types[t]);
            
        //Make sure the player can't request to call more than the Onion has.
        types[t].delta = std::min(real_onion_amount, (int) types[t].delta);
        
        //Calculate the total delta.
        total_delta += types[t].delta;
    }
    
    //Make sure the player can't request to have more than the field limit.
    int delta_over_limit =
        (int) game.states.gameplay->mobs.pikmin_list.size() +
        total_delta -
        (int) game.config.rules.max_pikmin_in_field;
        
    while(delta_over_limit > 0) {
        vector<size_t> candidate_types;
        
        for(size_t t = 0; t < n_ptr->nest_type->pik_types.size(); t++) {
            int real_group_amount =
                (int) l_ptr->group->getAmountByType(
                    n_ptr->nest_type->pik_types[t]
                );
                
            if((-types[t].delta) < real_group_amount) {
                //It's possible to take away from this type's delta request.
                candidate_types.push_back(t);
            }
        }
        
        //Figure out the type with the largest delta.
        size_t best_type = 0;
        int best_type_delta = types[candidate_types[0]].delta;
        for(size_t t = 1; t < candidate_types.size(); t++) {
            if(types[candidate_types[t]].delta > best_type_delta) {
                best_type = candidate_types[t];
                best_type_delta = types[candidate_types[t]].delta;
            }
        }
        
        //Finally, remove one request from this type.
        types[candidate_types[best_type]].delta--;
        delta_over_limit--;
    }
    
    //Animate red text, if any.
    for(auto w = red_items.begin(); w != red_items.end();) {
        w->second -= delta_t;
        if(w->second <= 0.0f) {
            w = red_items.erase(w);
        } else {
            ++w;
        }
    }
    
    //Tick the GUI.
    gui.tick(delta_t);
    
    //Tick the background.
    const float bg_alpha_mult_speed =
        1.0f / GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME;
    const float diff =
        closing ? -bg_alpha_mult_speed : bg_alpha_mult_speed;
    bg_alpha_mult = std::clamp(bg_alpha_mult + diff * delta_t, 0.0f, 1.0f);
    
    //Tick the menu closing.
    if(closing) {
        closing_timer -= delta_t;
        if(closing_timer <= 0.0f) {
            to_delete = true;
        }
    }
}


/**
 * @brief Toggles the "select all" mode.
 */
void OnionMenu::toggleSelectAll() {
    select_all = !select_all;
    
    update();
}


/**
 * @brief Updates some things about the Onion's state, especially caches.
 */
void OnionMenu::update() {
    //Reset the on-screen types.
    on_screen_types.clear();
    
    for(size_t t = 0; t < types.size(); t++) {
        types[t].on_screen_idx = INVALID;
    }
    
    //Reset the button and amount states.
    for(size_t t = 0; t < ONION_MENU::TYPES_PER_PAGE; t++) {
        onion_icon_items[t]->visible = false;
        onion_button_items[t]->visible = false;
        onion_button_items[t]->selectable = false;
        onion_amount_items[t]->visible = false;
        group_icon_items[t]->visible = false;
        group_button_items[t]->visible = false;
        group_button_items[t]->selectable = false;
        group_amount_items[t]->visible = false;
    }
    
    //Assign the on-screen types.
    for(
        size_t t = page * ONION_MENU::TYPES_PER_PAGE;
        t < (page + 1) * ONION_MENU::TYPES_PER_PAGE &&
        t < n_ptr->nest_type->pik_types.size();
        t++
    ) {
        types[t].on_screen_idx = on_screen_types.size();
        on_screen_types.push_back(&types[t]);
    }
    
    //Assign the coordinates of the on-screen-type-related GUI items.
    float splits = on_screen_types.size() + 1;
    float leftmost = 0.50f;
    float rightmost = 0.50f;
    
    for(size_t t = 0; t < on_screen_types.size(); t++) {
        float x = 1.0f / splits * (t + 1);
        onion_icon_items[t]->ratio_center.x = x;
        onion_button_items[t]->ratio_center.x = x;
        onion_amount_items[t]->ratio_center.x = x;
        group_icon_items[t]->ratio_center.x = x;
        group_button_items[t]->ratio_center.x = x;
        group_amount_items[t]->ratio_center.x = x;
        
        leftmost =
            std::min(
                leftmost,
                x - onion_button_items[t]->ratio_size.x / 2.0f
            );
        rightmost =
            std::max(
                rightmost,
                x + onion_button_items[t]->ratio_size.x / 2.0f
            );
    }
    
    //Make all relevant GUI items active.
    for(size_t t = 0; t < on_screen_types.size(); t++) {
        onion_icon_items[t]->visible = true;
        onion_amount_items[t]->visible = true;
        group_icon_items[t]->visible = true;
        group_amount_items[t]->visible = true;
        if(!select_all) {
            onion_button_items[t]->visible = true;
            onion_button_items[t]->selectable = true;
            group_button_items[t]->visible = true;
            group_button_items[t]->selectable = true;
        }
    }
    
    if(nr_pages > 1) {
        leftmost =
            std::min(
                leftmost,
                onion_more_l_icon->ratio_center.x - onion_more_l_icon->ratio_size.x / 2.0f
            );
        rightmost =
            std::max(
                rightmost,
                onion_more_r_icon->ratio_center.x + onion_more_r_icon->ratio_size.x / 2.0f
            );
    }
    
    onion_more_l_icon->visible = nr_pages > 1 && select_all;
    onion_more_r_icon->visible = nr_pages > 1 && select_all;
    group_more_l_icon->visible = nr_pages > 1 && select_all;
    group_more_r_icon->visible = nr_pages > 1 && select_all;
    
    onion_all_button->ratio_size.x = rightmost - leftmost;
    group_all_button->ratio_size.x = rightmost - leftmost;
    
    onion_all_button->visible = select_all;
    onion_all_button->selectable = select_all;
    group_all_button->visible = select_all;
    group_all_button->selectable = select_all;
}


/**
 * @brief Constructs a new Onion menu type struct object.
 *
 * @param idx Index of the Pikmin type in the nest object.
 * @param pik_type The Pikmin type.
 */
OnionMenuPikminType::OnionMenuPikminType(
    size_t idx, PikminType* pik_type
) :
    type_idx(idx),
    pik_type(pik_type) {
    
}
