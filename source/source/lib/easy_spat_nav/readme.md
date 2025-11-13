# Easy Spatial Navigation

Espyo's Easy Spatial Navigation algorithm for GUIs.

> * [Overview](#overview)
> * [Quick example](#quick-example)
> * [Features](#features)


## Overview

This is a source-only C++ library that implements a spatial navigation algorithm for graphical user interfaces. The way it works is pretty simple:

* Every time the user performs a spatial navigation input (e.g. pressing the right arrow key)...
  * ⇥ The program informs the library...
    * What GUI items exist, and where, in an abstract way.
    * Any algorithm customizations.
  * ↤ Then, the library...
    * Returns what the next GUI item to focus should be.

Spatial navigation is surprisingly tricky. Guessing which GUI item the user wanted to focus is difficult, and depends on the program, the GUI, and more.  There are also a few things that take work to implement, like the ability to loop around once the edge of the GUI is reached, how to handle GUI items with more items inside, or tie-breaking. This library aims to ease those burdens.


## Quick example

```cpp
#include "easy_spat_nav.h"
#include "my_program_stuff.h"

// (...)

void myProgram::doSpatialNavigation(EGSpatNav::DIRECTION direction) {
    EasySpatNav::Interface esnInterface;

    for(const auto& item : myProgram.currentGUI.items) {
        if(!item.canBeFocused()) continue;
        esnInterface.addItem((EasySpatNav::ItemId) item, item.center.x, item.center.y, item.size.x, item.size.y);
    }

    MyProgram::GuiItem* targetItemPtr =
        (MyProgram::GuiItem*) esnInterface.navigate(direction, (EasySpatNav::ItemId) myProgram.currentGui.focusedItem);
}
```

## Features

* Support for looping around the edges of the GUI (see `Interface::settings`).
* Support for parent items that have children items that can overflow inside (see `Interface::setParentItem`).
* Support for tie-breaking, including respecting the user's navigation history (see `Interface::heuristics::historyScoreThreshold`).
* Customizable heuristics (see `Interface::heuristics`).
* Basic debugging logic (see `EASY_SPAT_NAV_DEBUG`).
* Fairly light, and fairly simple.
* Very agnostic, and with no external dependencies.
* Simple unit test coverage for the library itself.


## Important information

* For simplicity's sake, each item is identified with a `void *`. In your application you probably identify your widgets with an index number or a pointer, so just cast that to a `void *` freely. `nullptr` (zero) is reserved for "none", however.
* To work with parent and children items:
  * Note that parent items are never eligible for being focused.
  * For children items, specify their position in normal GUI coordinates, even if they overflow past the parent's borders.
* If you don't provide a valid starting point for navigation, it will use 0,0 for the focus position and size. Your users will probably prefer if you pick the first available widget, or the one closest to the mouse cursor.
* The starting point doesn't care about children overflow. In other words, if the currently focused item is a child item that is overflowing from the parent, you should probably change the parent's offset so that that child is in-view and not overflowing. Otherwise the algorithm will start navigating from weird places.


## Troubleshooting

* When looping through an edge of the GUI, it's skipping over an item.
  * Make sure there's at least a small gap between the items and the edges of the GUI. If it doesn't, you can manually grow the limits by 0.01 without any real drawback.
* The algorithm is picking the wrong item and I can't understand why.
  * Try defining `EASY_SPAT_NAV_DEBUG` (or uncomment its line near the top of the header file). Then on every frame draw onto the window the contents provided by `Interface::lastNavInfo` in whatever way you want. This should help you understand what the algorithm is doing and what you can customize to make it do what you want.
* There's an item a bit out of the way, but I can't seem to reach it.
  * Try `Interface::heuristics::singleLoopPass`.


## Inner workings notes

* For parent and children items:
  * Children items that are overflowing get flattened. In essence the algorithm pretends they exist at the border of the parent, just very very thin.
  * This approach allows the relative position between items to be kept, whilst also ensuring that navigating into the parent item correctly navigates to the most obvious child item.
  * Other approaches were possible, but much more complex to implement.
  * Because of this approach, you have to be wary that while hierarchies can be deeper than one level, the deeper you go the sooner you run into floating-point imprecisions. It should be fine for any normal application though.
* The algorithm rotates all widgets such that right (positive X) points to the direction of the navigation.
* Navigation only works in the four cardinal directions. Free angles were considered, but deemed to be way too unreasonable to implement.


## Research

* https://github.com/WICG/spatial-navigation
* https://github.com/NoriginMedia/Norigin-Spatial-Navigation
* https://gist.github.com/rygorous/6981057
