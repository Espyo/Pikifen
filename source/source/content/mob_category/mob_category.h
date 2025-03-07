/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob category classes and mob category-related functions.
 */

#pragma once

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../../core/const.h"
#include "../../util/drawing_utils.h"
#include "../../util/geometry_utils.h"


using std::size_t;
using std::string;
using std::vector;


//Mob categories. Sorted by what types of mobs to load first.
enum MOB_CATEGORY {

    //None.
    MOB_CATEGORY_NONE,
    
    //Pikmin.
    MOB_CATEGORY_PIKMIN,
    
    //Onions.
    MOB_CATEGORY_ONIONS,
    
    //Leaders.
    MOB_CATEGORY_LEADERS,
    
    //Enemies.
    MOB_CATEGORY_ENEMIES,
    
    //Treasures.
    MOB_CATEGORY_TREASURES,
    
    //Pellets.
    MOB_CATEGORY_PELLETS,
    
    //Converters.
    MOB_CATEGORY_CONVERTERS,
    
    //Drops.
    MOB_CATEGORY_DROPS,
    
    //Resources.
    MOB_CATEGORY_RESOURCES,
    
    //Piles.
    MOB_CATEGORY_PILES,
    
    //Tools.
    MOB_CATEGORY_TOOLS,
    
    //Ships.
    MOB_CATEGORY_SHIPS,
    
    //Bridges.
    MOB_CATEGORY_BRIDGES,
    
    //Group tasks.
    MOB_CATEGORY_GROUP_TASKS,
    
    //Scales.
    MOB_CATEGORY_SCALES,
    
    //Tracks.
    MOB_CATEGORY_TRACKS,
    
    //Bouncers.
    MOB_CATEGORY_BOUNCERS,
    
    //Decorations.
    MOB_CATEGORY_DECORATIONS,
    
    //Interactables.
    MOB_CATEGORY_INTERACTABLES,
    
    //Custom.
    MOB_CATEGORY_CUSTOM,
    
    //Total amount of mob categories.
    N_MOB_CATEGORIES,
    
};


class Mob;
class MobType;


/**
 * @brief A mob category. Pikmin, leader, enemy, etc.
 * Each category helps organize the types of mob and the mobs themselves.
 */
class MobCategory {

public:

    //--- Members ---
    
    //Internal name.
    string internal_name;
    
    //Name of the mob category.
    string name;
    
    //ID of the mob category.
    MOB_CATEGORY id = MOB_CATEGORY_NONE;
    
    //Name used when referring to objects of this category in plural.
    string plural_name;
    
    //Name of the folder for this category.
    string folder_name;
    
    //Color used to represent objects of this category in the area editor.
    ALLEGRO_COLOR editor_color = COLOR_WHITE;
    
    
    //--- Function declarations ---
    
    MobCategory(
        const MOB_CATEGORY id, const string &internal_name,
        const string &name, const string &plural_name,
        const string &folder_name, const ALLEGRO_COLOR editor_color
    );
    virtual ~MobCategory() = default;
    virtual void get_type_names(vector<string> &list) const = 0;
    virtual MobType* get_type(const string &internal_name) const = 0;
    virtual MobType* create_type() = 0;
    virtual void register_type(const string &internal_name, MobType* type) = 0;
    virtual Mob* create_mob(
        const Point &pos, MobType* type, float angle
    ) = 0;
    virtual void erase_mob(Mob* m) = 0;
    virtual void clear_types() = 0;
    
};


/**
 * @brief A list of the different mob categories.
 * The MOB_CATEGORY_* constants are meant to be used here.
 *
 * Read the sector type manager's comments for more info.
 */
struct CategoryManager {

    public:
    
    //--- Function declarations ---
    
    void register_category(MOB_CATEGORY id, MobCategory* category);
    MobType* find_mob_type(const string &name) const;
    MobCategory* get(const MOB_CATEGORY id) const;
    MobCategory* get_from_folder_name(const string &internal_name) const;
    MobCategory* get_from_internal_name(const string &name) const;
    MobCategory* get_from_name(const string &name) const;
    MobCategory* get_from_pname(const string &pname) const;
    void clear();
    
    private:
    
    //--- Members ---
    
    //List of known mob categories.
    vector<MobCategory*> categories;
    
};


/**
 * @brief "None" mob category. Used as a placeholder.
 */
class NoneCategory : public MobCategory {

public:

    //--- Function declarations ---
    
    NoneCategory();
    void get_type_names(vector<string> &list) const override;
    MobType* get_type(const string &internal_name) const override;
    MobType* create_type() override;
    void register_type(const string &internal_name, MobType* type) override;
    Mob* create_mob(
        const Point &pos, MobType* type, float angle
    ) override;
    void erase_mob(Mob* m) override;
    void clear_types() override;
    
};
