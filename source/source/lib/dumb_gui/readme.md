# DumbGui

> * [Overview](#overview)
> * [Quick example](#quick-example)
> * [Features](#features)
> * [Creating derived classes](#creating-derived-classes)
> * [Tips](#tips)
> * [About the project](#about-the-project)
> * [Notes](#notes)


## Overview

**DumbGui** is a source-only C++ middleware library that simplifies the process of creating a GUI and HUD system for your game.

When you want to create a simple menu or a HUD, just do this:

* GUI initialization:
    1. [Create](#memory-management) `Item`s.
        1. Specify their [relative locations and sizes](#ratios) (`Item::raw*`).
        2. Specify what each item should do when it's time to be [drawn](#drawinfo) (`Item::onDraw`).
        3. For interactive items, specify what happens when they are interacted with (activated) (`Item::onActivate`).
    2. Create a `Manager`.
        1. Feed it the list of items (`Manager::addItem`).
        2. Tell it how to obtain the window's dimensions (`Manager::onGetWindowSizePx`).
* Every frame:
    1. Tell the manager about mouse events (`Manager::handleMouse*`).
    2. Tell the manager to process a frame (`Manager::tick`).
    3. Tell the manager to [draw](#drawinfo) all of the items on-screen (`Manager::draw`).

So why is it dumb? Because it doesn't know anything!

You need to teach it what drawing means, what mouse events mean, and so on. This makes it fairly basic on its own, but incredibly versatile.

In games, different HUD elements look and behave differently, and even different widgets in a menu have different ways of working. So instead of trying in vain to create a smart system that handles every possible variation, DumbGui sticks to the basics so you can feed all the behavior and look you need with lambdas!


## Quick example
```cpp
#include "dumb_gui.hpp"
#include "my_game_stuff.hpp"

int main() {
    myGame.setup();

    // 1. Create the items.
    DumbGui::Item* playButton = new DumbGui::Item(true);
    playButton->rawCenterRatioX = 0.50f;
    playButton->rawCenterRatioY = 0.33f;
    playButton->rawSizeRatioW = 0.33f;
    playButton->rawSizeRatioH = 0.20f;
    playButton->onDraw = [] (const DumbGui::DrawInfo& draw) { myGame.drawButton(draw, "Play!"); };
    playButton->onActivate = [] (float, float) { myGame.startPlaying(); };

    DumbGui::Item* optionsButton = new DumbGui::Item(true);
    optionsButton->rawCenterRatioX = 0.50f;
    optionsButton->rawCenterRatioY = 0.66f;
    optionsButton->rawSizeRatioW = 0.25f;
    optionsButton->rawSizeRatioH = 0.20f;
    optionsButton->onDraw = [] (const DumbGui::DrawInfo& draw) { myGame.drawButton(draw, "Options"); };
    optionsButton->onActivate = [] (float, float) { myGame.openOptions(); };

    // 2. Create the manager.
    DumbGui::Manager mainMenu;
    mainMenu.addItem(playButton);
    mainMenu.addItem(optionsButton);
    mainMenu.onGetWindowSizePx = [] (int* outWindowSizePxX, int* outWindowSizePxY) {
        *outWindowSizePxX = myGame.windowWidth;
        *outWindowSizePxY = myGame.windowHeight;
    };

    while(myGame.running) {
        MyGame::Event event = myGame.getEvent();

        // 3. Feed mouse events.
        if(event.type == MyGame::EVENT_MOUSE_MOVE) {
            mainMenu.handleMouseMovement(event.mouseX, event.mouseY);
        } else if(event.type == MyGame::EVENT_MOUSE_DOWN && event.mouseButton == 1) {
            mainMenu.handleMouseClick(event.mouseX, event.mouseY);
        }

        // 4. Tick and draw each frame.
        if(event.type == MyGame::EVENT_TIMER) {
            mainMenu.tick(myGame.deltaT);
            mainMenu.draw();
        }

        myGame.handleEvent(event);
    }
}
```


## Features

### Basics

* Setting which item is currently focused (including none).
  * [By itself](#creating-derived-classes), DumbGui enables the user to focus on items via the mouse cursor.
* Interactive item activation.
  * [By itself](#creating-derived-classes), DumbGui enables the user to activate an item via mouse button clicks.
* Setting item visibility.
  * Specific items (`Item::visible`), or all (`Manager::visible`). Invisible items can still be interacted with.
* Setting item responsiveness.
  * Specific items (`Item::responsive`), or all (`Manager::responsive`). Unresponsive items can still be visible.
* Setting whether an item can be focused.
  * Specific items (`Item::focusable`).

### Advanced
* Parent and child items.
  * Edge padding (`Item::edgePaddingPx`)
  * Offsets for scrolling (`Item::offsetRatio*`).
  * Clipping logic (`Manager::on*ClippingRectangle`).
* Item tooltips.
  * If you set the tooltips of items (`Item::onGetTooltip`), you can create something like a text GUI item that reads the focused item's tooltip (`Manager::getCurrentTooltip`) and displays it.
* `Item` interaction and behavior callbacks.
  * When a frame passes (`Item::onTick`), when the mouse cursor hovers over it (`Item::onMouseOver`), and more.
* Drawing by layers.
  * Manager item draw order depends on the order of the list (`Manager::addItem`), but can be tweaked with layers (`Item::drawingLayer`).


## Creating derived classes
While DumbGui works out of the box, you can create classes that derive from `Manager` and `Item` and make use of class extensions and polymorphism. These can make your life easier and allow you to customize your GUIs in more powerful ways. Here are some ideas:

* For easier development:
  * Create classes that extend `Manager` and `Item`, and make their constructors fill in lambdas that will always be the same (`Manager::onGetWindowSizePx`, `Manager::on*ClippingRectangle`).
  * Create classes that inherit from your extended `Item` class for different types of widgets, like buttons, sliders, labels, etc. Their class members can include the text to show, and their constructors can automatically fill in lambdas that draw those things.
  * Add wrappers for getting and setting the raw centers (`Item::rawCenterRatio*`), raw sizes (`Item::rawSizeRatio*`), and so on, so you can use your own higher-level structs like a `Point` or a `Rect`.
  * Create `Item` class members for drawing callbacks that instead use a custom [`DrawInfo`](#drawinfo) class with higher-level structs. Then, make the manager's drawing logic (`Item::draw`) convert the class and call those callbacks instead.
  * Have the `Manager` store a pointer to the "back button" or "confirm button", so users can hit their "menu back" or "menu OK" keys/buttons to instantly activate (`Item::activate`) that item.
* For more interesting GUIs:
  * Add a menu animation system to `Manager` that can make items slide into view, fade away, etc. When it's time to obtain an item's coordinates for drawing (`Manager::getItemDrawInfo`), the manager can check if it's mid-animation and return the coordinates offset by some amount.
  * Add a juice system that sets an `Item` class member that controls the juice timer, and has the drawing callbacks read that value to temporarily change the font size, make it glow white, etc.
  * Block input from being received (`Manager::shouldHandleEvents`) if your game has a pop-up in front, or is fading away to a different screen.
  * Add a [spatial navigation](https://en.wikipedia.org/wiki/Spatial_navigation) algorithm that receives each item's on-screen coordinates (`Item::getReferencePx*`) and finds a different item to focus on, as a response to the user hitting a directional key/button.
  * Add `Item` callbacks for things like a hardware event, then have the `Manager` iterate through all items and feed them the event, so you can have things like text input boxes that respond to keyboard presses.
  * When drawing an item, if it is the focused item (`Item::focused`), change its appearance to reflect that.


## Tips
* For more information on how a part of the library works, read its comments in the header file.
* Variables and functions that deal with coordinates will have `Ratio` or `Px` in their name so you always know if they use [ratio coordinates](#ratios) or pixel measurements.
* If you need to access a specific item frequently, keep its pointer when you create it instead of searching through the manager's item list each time.
* Split the drawing routines for different types of widgets into as many functions as you can. That way, any GUI item that needs to draw things slightly different, like the text in a different color or with a different background texture can make use of those functions to minimize repeated code. Try storing them in your `Button`, `Slider`, etc. classes as static public functions.
* See how _[Pikifen](https://github.com/Espyo/Pikifen)_ uses DumbGui!


## About the project
* Fairly light, and fairly simple.
* No external dependencies. Uses only primitives, `std::string`, `std::vector`, and `std::function`.
* Project license: MIT license.
* Project developer: Espyo.
* Project keywords for searches: abstraction, GUI, UI, UX, graphical user interface, HUD, heads-up display, keyboard, mouse, controller, navigation, position, hierarchy


## Notes

### Memory management
* When creating the items, you are responsible for allocating them in the heap with `new`. The Manager does not need this.
* Make sure the item objects stay alive, until the `Manager` is destroyed.
* Only use pointers to address the items.
* When destroying the GUI, you can use `Manager::destroy`, which frees the allocated memory for you.

### Ratios
* When some things use the "ratio" coordinate system, that means you specify a ratio of the parent's width and height, or the window's width and height if the item does not have a parent.
* (0,0) represents the top-left corner, and (1,1) represents the bottom-right corner. For instance, an item positioned at (0.1,0.5) means it's near the left edge horizontally and is centered vertically.
* You can use values beyond these ranges if it makes sense to do so. For instance, children items in a vertical list may go over 1 for the Y coordinate, which means the parent's `Item::offsetRatioY` needs to be increased in order to see those items.
* By using this, the same GUI looks more or less the same, and scales accordingly, regardless of the player's game resolution.

### DrawInfo
* When it's time to draw an item, the `Item::onDraw` callback receives a `DrawInfo` object with the final on-screen coordinates, in pixels.
* Items should try to be drawn with the given dimensions, coordinates, and transparency, but can deviate from them for juice, like a bounce.
