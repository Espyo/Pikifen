/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general editor-related functions.
 */

#ifndef EDITOR_INCLUDED
#define EDITOR_INCLUDED

#include <string>
#include <vector>

#include "../game_state.h"

using std::map;
using std::string;
using std::vector;

/*
 * A generic class for an editor.
 * It comes with some common stuff, mostly GUI stuff.
 */
class editor : public game_state {
public:

    editor();
    virtual ~editor() = default;
    
    virtual void do_drawing() = 0;
    virtual void do_logic() = 0;
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void load();
    virtual void unload();
    virtual void update_transformations();
    virtual string get_name() const = 0;
    
protected:

private:

};

#endif //ifndef EDITOR_INCLUDED
