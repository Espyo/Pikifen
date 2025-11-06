# Spatial Navigation

Espyo's GUI spatial navigation algorithm.

> * [Overview](#overview)
> * [Quick example](#quick-example)
> * [Features](#features)

## Overview

This is a source-only C++ library that implements a spatial navigation algorithm for graphical user interfaces. The way it works is pretty simple:

* Every time the user performs a spatial navigation input...
  * ⇥ The program informs the library...
    * What GUI items exist, and where, in an abstract way.
    * Any algorithm customizations.
  * ↤ Then, the library...
    * Returns what the next GUI item to focus should be.

Spatial navigation is surprisingly tricky. Guessing which GUI item the user wanted to focus is difficult, and depends on the program, the GUI, and more.  There are also a few things that take work to implement, like the ability to loop around once the edge of the GUI is reached, or how to handle GUI items with more items inside. This library aims to ease those burdens.

## Quick example

```cpp
#include "spatial_navigation.h"
#include "my_program_stuff.h"

// (...)

void myProgram::doSpatialNavigation(SpatNav::DIRECTION direction) {
    SpatNav::Interface spatNavManager;

    for(const auto& item : myProgram.currentGUI.items) {
        if(!item.canBeFocused()) continue;
        spatNavManager.addItem((void*) item, item.center.x, item.center.y, item.size.x, item.size.y);
    }

    GuiItem* targetItemPtr = (MyProgram::GuiItem*) spatNavManager.navigate(direction, (void*) myProgram.currentGui.focusedItem);
}
```

## Features

* Support for looping around the edges of the GUI.
* Support for parent items that have children items inside.
* Customizable heuristics.
* Basic debugging logic.
* Fairly light, and fairly simple.
* Very agnostic, and with no external dependencies.
* Unit tests.

## Research

* https://github.com/WICG/spatial-navigation
* https://github.com/NoriginMedia/Norigin-Spatial-Navigation
* https://gist.github.com/rygorous/6981057
