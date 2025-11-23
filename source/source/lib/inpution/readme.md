# Inpution

_From input to action._

> * [Overview](#overview)
> * [Quick example](#quick-example)
> * [Features](#features)
>   * [What it doesn't do](#what-it-doesnt-do)
> * [Other usage information](#other-usage-information)
> * [Inner workings notes](#inner-workings-notes)


## Overview

**Inpution** is a source-only C++ piece of middleware that abstracts and simplifies control binds in game development. The way it works is pretty simple:

* ⇥ The game must...
  * _At startup_: Teach Inpution what action types exist in this game.
  * _After loading_: Teach Inpution what control binds the player has registered.
  * _Every hardware event_: Inform Inpution of what hardware inputs happened.
  * _Every frame_: Inform Inpution that a game frame is over.
* ↤ Then, Inpution...
  * _Every frame_: Informs the game of what actions have happened.

On top of simply giving the game a list of actions, it also simplifies a lot of fluff and nuance in the input processing side of things. See [Features](#features).


## Quick example

```cpp
#include "inpution.h"
#include "my_game_stuff.h"

enum PLAYER_ACTION {
    PLAYER_ACTION_MOVE_LEFT,
    PLAYER_ACTION_MOVE_RIGHT,
    PLAYER_ACTION_MOVE_JUMP,
}

int main() {
    Inpution::Manager inpution;
    myGame.setup();
    
    // 1. Teach Inpution what action types exist.
    inpution.actionTypes[MyGame::PLAYER_ACTION_MOVE_LEFT] = {
        .id = PLAYER_ACTION_MOVE_LEFT,
    };
    inpution.actionTypes[MyGame::PLAYER_ACTION_MOVE_RIGHT] = {
        .id = PLAYER_ACTION_MOVE_RIGHT,
    };
    inpution.actionTypes[MyGame::PLAYER_ACTION_JUMP] = {
        .id = PLAYER_ACTION_JUMP,
        .valueType = Inpution::ACTION_VALUE_TYPE_DIGITAL,
    };

    // 2. Teach Inpution what control binds the player has.
    inpution.binds.push_back({
        .actionTypeId = MyGame::PLAYER_ACTION_MOVE_LEFT,
        .inputSource = {
            .type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG,
            .stickNr = MyGame::LEFT_STICK
        }
    });
    inpution.binds.push_back({
        .actionTypeId = MyGame::PLAYER_ACTION_MOVE_RIGHT,
        .inputSource = {
            .type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS,
            .stickNr = MyGame::LEFT_STICK
        }
    });
    inpution.binds.push_back({
        .actionTypeId = MyGame::PLAYER_ACTION_JUMP,
        .inputSource = {
            .type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON,
            .buttonNr = MyGame::A_BUTTON
        }
    });

    while(myGame.running) {
        MyGame::Event event = myGame.getEvent();

        // 3. Inform Inpution of any hardware inputs.
        if(event.type == MyGame::EVENT_BUTTON_DOWN || event.type == MyGame::EVENT_BUTTON_UP) {
            inpution.handleInput({
                .source = {
                    .type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_BUTTON,
                    .buttonNr = event.keyButton
                },
                .value = (event.type == MyGame::EVENT_BUTTON_DOWN) ? 1 : 0;
            });
        } else if(event.type == MyGame::EVENT_ANALOG_STICK && event.value >= 0) {
            inpution.handleInput({
                .source = {
                    .type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS,
                    .axisNr = event.analogAxisNr
                },
                .value = event.value
            });
        } else if(event.type == MyGame::EVENT_ANALOG_STICK && event.value < 0) {
            inpution.handleInput({
                .source = {
                    .type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG,
                    .axisNr = event.analogAxisNr
                },
                .value = -event.value
            });
        }

        myGame.handleEvent(event);

        if(event.type == MyGame::EVENT_TIMER) {
            // 4. Every frame, inform Inpution and process the actions.
            std::vector<Inpution::Action> actions = inpution.newFrame(myGame.deltaT);
            for(const auto& a : actions) {
                switch(a.actionTypeId) {
                case MyGame::PLAYER_ACTION_MOVE_LEFT:
                    character.horizontalSpeed = a.value * 100;
                    break;
                case MyGame::PLAYER_ACTION_MOVE_RIGHT:
                    character.horizontalSpeed = -a.value * 100;
                    break;
                case MyGame::PLAYER_ACTION_JUMP:
                    character.verticalSpeed = 100;
                    break;
                }
            }

            myGame.tick(myGame.deltaT);
        }
    }
}
```


## Features

* Many settings:
  * Analog (range [0 - 1]), digital (just 0 or 1), and "down-only" (just 1) input values, and conversions between them (see `ActionType::valueType`).
  * Auto-repeated action events as long as the input is held (see `ActionType::autoRepeat`).
  * Actions can be generated directly from input events, or from the total combined internal hardware state (see `ActionType::directEvents`).
  * Reinserting actions into the queue for a buffer effect (see `ActionType::reinsertionTTL`).
  * Multiple players (see `Action::playerNr`).
  * Analog stick deadzone processing, with all sorts of interpolations (see `ManagerOptions`).
* Varied support:
  * Support for multiple controllers at once.
  * Support for the input source triggering multiple actions, and for multiple input sources triggering the same action.
  * Support for stateful input sources (buttons, keys, analog sticks, etc.) and stateless sources (mouse wheel spins, typed characters).
  * Support for binds with modifiers (e.g. pressing Ctrl before pressing S) (see `Manager::modifiers`).
* Specific features:
  * Game states logic (see `Manager::setGameState()`).
  * Temporarily ignoring given input sources (see `Manager::startIgnoringInputSource()`).
* About the library:
  * Fairly light, and fairly simple.
  * Very agnostic, and with no external dependencies.


### What it doesn't do

* It does not obtain hardware information. It's not a hardware driver, it doesn't recognize keyboard, mouse, or controller hardware input events, it doesn't recognize controller or controller button names. Your program is the one that must obtain this information and feed it to Inpution.
* It does not understand game controller anatomy. That is, it doesn't know if a button is really a button or a trigger, it doesn't know if a stick is an analog stick or a D-pad, etc. It also doesn't understand stick axes, but it does assume them; see [Inner workings notes](#inner-workings-notes).
* It does not abstract mouse movement. If, for instance, you want camera control to be done via mouse movement and an analog stick, you can abstract the analog stick logic with Inpution, but you will have to add the mouse movement logic yourself.


## Other usage information

* Inpution reports all player actions that happened. For instance, it could report the "jump" action and the "menu confirm" action when the player presses A. It is up to you to figure out your game state and accept or ignore certain actions.

Here are some important key terms:

* **Action**:
  * An abstract representation of a specific action in the game that the player performed. The exact real-life movement the player made for this is not relevant. This is essentially an event.
  * e.g. Move aiming reticle right, with a strength of 0.75.
* **Action type**:
  * A type of action the player can perform in the game.
  * e.g. Crouch (and un-crouch).
* **Bind**:
  * Information about a bind between an action type and an input source. Typically the player can customize these in the options menu.
  * e.g. The bind between the A button on the controller and the jump action.
* **Input**:
  * A specific movement made by a human being, on a specific source, on a specific hardware device.
  * Tilting the right analog stick upwards by 50%.
* **Input source**:
  * A specific button, key, analog stick, etc. on a specific device.
  * e.g. The left bumper button on the second controller.


## Inner workings notes

* Analog sticks can have any number of axes, and each axis could mean something different, so Inpution makes no attempt to understand them. However, for the sake of usability, it treats axis 0 of an analog stick as the horizontal axis and axis 1 as the vertical axis. And it treats 0,0 as the analog stick's neutral position, axis 0 negative and positive as left and right respectively, and axis 1 negative and positive as up and down respectively. This seems to be what almost all game controllers use in the real world.
