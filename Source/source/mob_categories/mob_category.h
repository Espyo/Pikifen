/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
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

using namespace std;

enum MOB_CATEGORIES {
    //Sorted by what types of mobs to load first.
    MOB_CATEGORY_NONE,
    MOB_CATEGORY_PIKMIN,
    MOB_CATEGORY_ONIONS,
    MOB_CATEGORY_LEADERS,
    MOB_CATEGORY_ENEMIES,
    MOB_CATEGORY_TREASURES,
    MOB_CATEGORY_PELLETS,
    MOB_CATEGORY_DROPS,
    MOB_CATEGORY_SPECIAL,
    MOB_CATEGORY_SHIPS,
    MOB_CATEGORY_GATES,
    MOB_CATEGORY_BRIDGES,
    MOB_CATEGORY_INTERACTABLES,
    MOB_CATEGORY_CUSTOM,
    
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
    size_t id;
    
    string plural_name;
    string folder;
    ALLEGRO_COLOR editor_color;
    
    virtual void get_type_names(vector<string> &list) = 0;
    virtual mob_type* get_type(const string &name) = 0;
    virtual mob_type* create_type() = 0;
    virtual void register_type(mob_type* type) = 0;
    virtual mob* create_mob(
        const point &pos, mob_type* type, const float angle
    ) = 0;
    virtual void erase_mob(mob* m) = 0;
    virtual void clear_types() = 0;
    
    mob_category(
        const size_t id, const string &name, const string &plural_name,
        const string &folder, const ALLEGRO_COLOR editor_color
    );
    virtual ~mob_category();
    
};



/* ----------------------------------------------------------------------------
 * A list of the different mob categories.
 * The MOB_CATEGORY_* constants are meant to be used here.
 * Read the sector type manager's comments for more info.
 */
struct mob_category_manager {
private:
    vector<mob_category*> categories;
    
public:
    void register_category(size_t nr, mob_category* category);
    mob_type* find_mob_type(const string &name);
    mob_type* find_mob_type_from_folder_name(
        mob_category* cat, const string &name
    );
    mob_category* get(const size_t id);
    mob_category* get_from_name(const string &name);
    mob_category* get_from_pname(const string &pname);
    void clear();
};



/* ----------------------------------------------------------------------------
 * "None" mob category. Used as a placeholder.
 */
class none_category : public mob_category {
public:
    virtual void get_type_names(vector<string> &list);
    virtual mob_type* get_type(const string &name);
    virtual mob_type* create_type();
    virtual void register_type(mob_type* type);
    virtual mob* create_mob(
        const point &pos, mob_type* type, const float angle
    );
    virtual void erase_mob(mob* m);
    virtual void clear_types();
    
    none_category();
    ~none_category();
};


#endif //ifndef MOB_CATEGORY_INCLUDED
