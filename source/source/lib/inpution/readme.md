# Inpution

_From input to action._

> * [Overview](#overview)
> * [Quick example](#quick-example)
> * [Features](#features)
> * [Key terms](#key-terms)

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
    inpution.actionTypes[PLAYER_ACTION_MOVE_LEFT] = {
        .id = PLAYER_ACTION_MOVE_LEFT,
    };
    inpution.actionTypes[PLAYER_ACTION_MOVE_RIGHT] = {
        .id = PLAYER_ACTION_MOVE_RIGHT,
    };
    inpution.actionTypes[PLAYER_ACTION_JUMP] = {
        .id = PLAYER_ACTION_JUMP,
        .valueType = Inpution::ACTION_VALUE_TYPE_BOOLEAN,
    };

    // 2. Teach Inpution what control binds the player has.
    inpution.binds.push_back({
        .actionTypeId = PLAYER_ACTION_MOVE_LEFT,
        .inputSource = {
            .type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_NEG,
            .stickNr = MyGame::LEFT_STICK
        }
    });
    inpution.binds.push_back({
        .actionTypeId = PLAYER_ACTION_MOVE_RIGHT,
        .inputSource = {
            .type = Inpution::INPUT_SOURCE_TYPE_CONTROLLER_AXIS_POS,
            .stickNr = MyGame::LEFT_STICK
        }
    });
    inpution.binds.push_back({
        .actionTypeId = PLAYER_ACTION_JUMP,
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
                case PLAYER_ACTION_MOVE_LEFT:
                    character.horizontalSpeed = a.value * 100;
                    break;
                case PLAYER_ACTION_MOVE_RIGHT:
                    character.horizontalSpeed = -a.value * 100;
                    break;
                case PLAYER_ACTION_JUMP:
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

* Support for analog (0 to 1) and binary (just 0 or 1) input values.
* Support for multiple players.
* Support for multiple controllers at once.
* Support for the input source triggering multiple actions, and for multiple input sources triggering the same action.
* Analog stick deadzone processing, with all sorts of interpolations.
* Specific features, like temporarily ignoring given input sources.
* Very agnostic, and with no external dependencies.

## Key terms

* **Action**:
  * An abstract representation of a specific action in the game that the player performed. The exact real-life movement the player made for this is not relevant.
  * e.g. Move aiming reticle right, with a strength of 0.75.
* **Action type**:
  * A type of action the player can perform in the game.
  * e.g. Crouch (and un-crouch).
* **Bind**:
  * Information about a bind between an action type and an input source.
  * e.g. The bind between the A button on the controller and the jump action.
* **Input**:
  * A specific movement made by a human being, on a specific source, on a specific hardware device.
  * Tilting the right analog stick upwards by 50%.
* **Input source**:
  * A specific button, key, analog stick, etc. on a specific device.
  * e.g. The left bumper button on the second controller.
