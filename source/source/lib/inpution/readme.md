# Inpution

_From input to action._

> * [Overview](#overview)
> * [Quick example](#quick-example)
> * [Features](#features)
>   * [What it doesn't do](#what-it-doesnt-do)
> * [Other usage information](#other-usage-information)
>   * [Key terms](#key-terms)
>   * [FAQ and troubleshooting](#faq-and-troubleshooting)
> * [Inner workings notes](#inner-workings-notes)
> * [Future plans](#future-plans)
> * [Misc.](#misc)


## Overview

**Inpution** is a source-only C++ piece of middleware that abstracts and simplifies control binds in game development. The way it works is pretty simple:

* ⇥ The game must...
  * _At startup_: Teach Inpution what action types exist in this game.
  * _After loading_: Teach Inpution what control binds the player has registered.
  * _Every hardware event_: Inform Inpution of what hardware inputs happened.
  * _Every frame_: Inform Inpution that a game frame is over.
* ↤ Then, Inpution...
  * _Every frame_: Informs the game of what actions have happened.

On top of simply giving the game a list of actions, it also simplifies a lot of fluff and nuance in the input processing side of things. See [Features](#features) and [Troubleshooting](#faq-and-troubleshooting) for a general idea of the sorts of gotchas and troubles the library can help you with.


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

### Many settings

* Different input value types.
  * Actions can have an analog (range [0 - 1]), digital (just 0 or 1), or "down-only" (just 1) value type. Conversions between them happen automatically.
  * e.g. A digital action can be triggered by an analog button, like shooting bound to an analog trigger.
  * See `ActionType::valueType`.
* Auto-repeats.
  * If enabled, as long as the input is held, Inpution can repeatedly send multiple actions over time.
  * e.g. The player holds up on the D-pad, and several "menu up" actions are triggered so they can navigate through menus quickly.
  * See `ActionType::autoRepeat`.
* Direct action events.
  * If enabled, actions are generated directly from input events. Disabled by default, which instead uses the total combined internal hardware state at the end of a frame.
  * e.g. Pressing A triggers a punch action. Pressing B (also punch) when A is held does nothing if the feature is disabled, but triggers a new action if enabled.
  * See `ActionType::directEvents`.
* Queue reinsertion.
  * Reinserting actions into the queue for a buffer effect.
  * e.g. The player pressed the A button to jump while in mid-air. The action goes back into the queue for some frames. If the player lands a couple of frames after this, the action is still in the queue and can now make the player jump.
  * See `ActionType::reinsertionTTL`.
* Multiple players.
  * See `Action::playerNr`.
* Analog stick deadzone processing.
  * Comes with all sorts of interpolations.
  * See `ManagerOptions`.


### Varied support

* Support for multiple controllers at once.
* Support for free binds.
  * An input source can trigger multiple actions, and multiple input sources can trigger the same action.
* Support for stateful and stateless input sources.
  * Stateful sources include buttons, keys, analog sticks, etc., meaning they are always in a given state till the player physically changes them. Stateless sources include mouse wheel spins, typed characters, etc., and a player's physical movement is one-and-done.
* Support for binds with modifiers.
  * e.g. pressing Ctrl before pressing S, in order to save in a menu.
  * See `Manager::modifiers`.
* Support for analog buttons.
  * As long as you can identify an input source as an analog button, which are sometimes internally defined as analog sticks, then you can inform Inpution about it.
  * e.g. The X-Box 360 triggers.


### Specific features

* Game states logic.
  * e.g. If you have a state for the "Ready..." cutscene before the player has control, and have a state for normal gameplay, then holding right in the cutscene state lets the player walk right on frame 1 of the gameplay state.
  * See `Manager::setGameState()`.
* Temporarily ignoring given input sources.
  * e.g. You just captured an input in the options menu to assign to a bind, and you don't want that input to immediately trigger its action.
  * See `Manager::startIgnoringInputSource()`.


### About the library

* Fairly light, and fairly simple.
* Very agnostic, and with no external dependencies.


### What it doesn't do

* It does not obtain hardware information. It's not a hardware driver, it doesn't recognize keyboard, mouse, or controller hardware input events, it doesn't recognize controller or controller button names. Your program is the one that must obtain this information and feed it to Inpution.
* It does not understand game controller anatomy. That is, it doesn't know if a button is really a button or an analog trigger, it doesn't know if a stick is an analog stick or a D-pad, etc. It also doesn't understand stick axes, but it does assume them; see [Inner workings notes](#inner-workings-notes).
* It does not abstract mouse movement. If, for instance, you want camera control to be done via mouse movement and an analog stick, you can abstract the analog stick logic with Inpution, but you will have to add the mouse movement logic yourself separately.


## Other usage information

* Inpution reports all player actions that happened. For instance, it could report the "jump" action and the "menu confirm" action when the player presses A. It is up to you to figure out your game state and accept or ignore certain actions.
* For more information on how a part of the library works, read its comments in the header file.


### Key terms

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
  * e.g. Tilting the right analog stick upwards by 50%.
* **Input source**:
  * A specific button, key, analog stick, etc. on a specific device.
  * e.g. The left bumper button on the second controller.


### FAQ and troubleshooting

* The player is pressing B to cancel out of the pause menu and then immediately does an attack, which is also bound to B. How can I make the game unpause only?
  * When you receive the "unpause" action, call `Manager::startIgnoringInputSource()` with the B button, or `Manager::startIgnoringActionInputSources()` with the "unpause" action. In addition, make use of `Manager::setGameState()` to change from a "pause menu" to a "normal gameplay" state. With this in place, the "unpause" action will be processed, the input will be ignored until it's released, the game will leave the pause menu state to enter normal gameplay, and the "attack" action won't be processed. If you don't call `Manager::setGameState()` you'll probably still have the "attack" action in the action queue for this frame.
* How can I detect whether something is an analog stick or an analog button?
  * You have a few ways. Whatever library you're using to get input from might be able to give you more information about a stick; if it reports it only has one axis then it may be an analog button. In addition, you can check the controller's name and/or GUID and cross-reference a list of known brands, like the [SDL Game Controller DB](https://github.com/mdqinc/SDL_GameControllerDB).
* The player Alt+Tab'd and now some inputs are stuck. What can I do?
  * Whatever engine you're using probably has ways to detect the window is out of focus. When that happens, call `Manager::releaseEverything()`.
* I don't want to receive actions right now (e.g. I have a textbox focused and don't want the key presses to be turned to actions). What can I do?
  * Let Inpution work like normal, but simply discard the list of actions returned by `Manager::newFrame()`. This is preferred over not calling anything Inpution-related, since you still want the library to receive key-up events, do auto-repeat logic, etc.


## Inner workings notes

* Analog sticks can have any number of axes, and each axis could mean something different, so Inpution makes no attempt to understand them. However, for the sake of usability, it treats axis 0 of an analog stick as the horizontal axis and axis 1 as the vertical axis. And it treats 0,0 as the analog stick's neutral position, axis 0 negative and positive as left and right respectively, and axis 1 negative and positive as up and down respectively. This seems to be what almost all game controllers use in the real world.


## Future plans

* Implement sub-schemes, so a game can have a scheme for when the player is driving, another for when they're on foot, etc.


## Misc.

* Project license: MIT license.
* Project developer: Espyo.
* Project keywords for searches: abstraction, hardware, keyboard, mouse, controller, gamepad, button, analog stick, D-pad, action, event, control binds, mapping
