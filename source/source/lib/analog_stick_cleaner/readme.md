# Analog Stick Cleaner

> * [Overview](#overview)
> * [Quick example](#quick-example)
> * [Features](#features)
> * [Other usage information](#other-usage-information)
> * [Research](#research)


## Overview

This is a source-only C++ library that performs some [analog stick](https://en.wikipedia.org/wiki/Analog_stick) input value cleanup. Analog sticks are finicky input sources by nature, whether that's because a human's finger isn't very precise, because the stick's looseness allows for wobbling, or because the plastic shell of the game pad doesn't allow the stick to reach the values it should. As such, with deadzones, value interpolation, and a few more tricks, it's possible to output values much closer to what the player actually wants. It also supports cleaning up analog button input values.

The library is entirely stateless: you feed it the X and Y coordinates of an analog stick, along with some configuration settings, and it spits out a clean value, all in a single function.


## Quick example

```cpp
#include "analog_stick_cleaner.h"
#include "my_game_stuff.h"

// (...)

while(myGame.running) {
    MyGame::Event event = myGame.getEvent();
    if(event.type == MyGame::ANALOG_STICK_MOVE) {
        float coords[2] = event.joystick.coords;

        AnalogStickCleaner::Settings cleanupSettings;
        cleanupSettings.deadzones.radial.inner = 0.2f;
        cleanupSettings.deadzones.radial.outer = 0.75f;
        AnalogStickCleaner::clean(coords, cleanupSettings);

        myGame.setPlayerMovement(coords);
    }
}
```


## Features

* Deadzone logic:
  * The standard deadzone, the inner radius deadzone, helps consider loose joystick wobbling as 0, so no input is read when the player isn't touching the stick.
  * The outer radius deadzone helps with sticks that aren't well calibrated or physically can't reach the highest tilt value.
  * See `AnalogStickCleaner::Settings::Deadzones::Radial`.
* Angular deadzones:
  * Makes it easier to snap to a horizontal, vertical, and/or diagonal angle. Good for 3D platformers!
  * See `AnalogStickCleaner::Settings::Deadzones::Angular`.
* Interpolation:
  * When using deadzones, for instance, a deadzone of 0.2, interpolation makes it so the value doesn't instantly jump from 0 to 0.2 once the player reaches the threshold.
  * See `AnalogStickCleaner::Settings::Deadzones::*::interpolate`.
* Low-pass filtering:
  * Reduces finger jittering by smoothening the values, though it can make controls feel a bit less responsive. Good for when careful aiming is required.
  * See `AnalogStickCleaner::Settings::LowPassFilter`.
* Analog button support:
  * Like analog sticks, the pressure value of analog buttons (or analog triggers) can make use of deadzones, interpolation, and low-pass filtering. Except instead of working in two dimensions, it works in one.
  * See `AnalogStickCleaner::Settings::Deadzones::Button` and `AnalogStickCleaner::Settings::LowPassFilter::factorButton`.


## Other usage information

* For more information on how a part of the library works, read its comments in the header file.


## Research

The library mostly implements the solutions encountered in these blog posts:

* [Doing Thumbstick Dead Zones Right, by Josh Sutphin - Game Developer](https://www.gamedeveloper.com/business/doing-thumbstick-dead-zones-right)
* [Interpreting Analog Sticks in INVERSUS, by Ryan Juckett - Game Developer](https://www.gamedeveloper.com/design/interpreting-analog-sticks-in-inversus)
  * [Interpreting Analog Sticks - Hypersect](https://blog.hypersect.com/interpreting-analog-sticks/) (Original article)
