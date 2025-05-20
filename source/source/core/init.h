/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for program initializer and deinitializer functions.
 */

#pragma once

#include "../content/animation/animation.h"
#include "../lib/data_file/data_file.h"


void initAllegro();
void initControls();
void initDearImGui();
void initDearImGuiColors();
void initErrorBitmap();
void initEssentials();
void initEventThings(ALLEGRO_TIMER*& timer, ALLEGRO_EVENT_QUEUE*& queue);
void initMisc();
void initMiscDatabases();
void initMobActions();
void initMobCategories();

void destroyAllegro();
void destroyEventThings(ALLEGRO_TIMER*& timer, ALLEGRO_EVENT_QUEUE*& queue);
void destroyMisc();
void destroyMobCategories();
