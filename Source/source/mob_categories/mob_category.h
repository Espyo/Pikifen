/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob category classes and mob category-related functions.
 */

#ifndef MOB_CATEGORY_INCLUDED
#define MOB_CATEGORY_INCLUDED

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../utils/geometry_utils.h"


using std::size_t;
using std::string;
using std::vector;


//Mob categories. Sorted by what types of mobs to load first.
enum MOB_CATEGORIES {
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


class mob;
class mob_type;

/* ----------------------------------------------------------------------------
 * A mob category. Pikmin, leader, enemy, etc.
 * Each category helps organize the types of mob and the mobs themselves.
 */
class mob_category {
public:
    string name;
    MOB_CATEGORIES id;
    
    string plural_name;
    string folder;
    ALLEGRO_COLOR editor_color;
    
    virtual void get_type_names(vector<string> &list) const = 0;
    virtual mob_type* get_type(const string &name) const = 0;
    virtual mob_type* create_type() = 0;
    virtual void register_type(mob_type* type) = 0;
    virtual mob* create_mob(
        const point &pos, mob_type* type, const float angle
    ) = 0;
    virtual void erase_mob(mob* m) = 0;
    virtual void clear_types() = 0;
    
    mob_category(
        const MOB_CATEGORIES id, const string &name, const string &plural_name,
        const string &folder, const ALLEGRO_COLOR editor_color
    );
    virtual ~mob_category() = default;
    
};



/* ----------------------------------------------------------------------------
 * A list of the different mob categories.
 * The MOB_CATEGORY_* constants are meant to be used here.
 * Read the sector type manager's comments for more info.
 */
struct mob_category_manager {
public:
    void register_category(MOB_CATEGORIES nr, mob_category* category);
    mob_type* find_mob_type(const string &name) const;
    mob_type* find_mob_type_from_folder_name(
        mob_category* cat, const string &name
    ) const;
    mob_category* get(const MOB_CATEGORIES id) const;
    mob_category* get_from_folder_name(const string &name) const;
    mob_category* get_from_name(const string &name) const;
    mob_category* get_from_pname(const string &pname) const;
    void clear();
    
private:
    vector<mob_category*> categories;
    
};



/* ----------------------------------------------------------------------------
 * "None" mob category. Used as a placeholder.
 */
class none_category : public mob_category {
public:
    virtual void get_type_names(vector<string> &list) const;
    virtual mob_type* get_type(const string &name) const;
    virtual mob_type* create_type();
    virtual void register_type(mob_type* type);
    virtual mob* create_mob(
        const point &pos, mob_type* type, const float angle
    );
    virtual void erase_mob(mob* m);
    virtual void clear_types();
    
    none_category();
};


#endif //ifndef MOB_CATEGORY_INCLUDED
