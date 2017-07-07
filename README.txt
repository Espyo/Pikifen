Pikmin fangame engine

An engine that allows users to create their Pikmin fangames easily.

This project is still under construction, so expect weird things to happen.
You can find more info on Pikmin Fanon http://www.pikminfanon.com/Pikmin_fangame_engine or by contacting me, Espyo, via IRC, on DarkMyst's #pikipedia.

===============
1) Controls
2) FAQ and troubleshooting
3) Compiling
4) Credits
5) Disclaimer
===============

===============
1) Controls
===============
    Most of these can be changed in the options menu.
    Quit:                      Esc
    Move:                      WASD
    Move cursor:               Mouse
    Whistle:                   Right mouse button
    Punch/Pluck/Throw:         Left mouse button
    
    Switch active Pikmin type: Mouse wheel
    Move group towards cursor: Space
    Switch leader:             Tab
    Dismiss:                   Left Ctrl
    Switch zoom level:         C

    Use top spray:             R
    Use bottom spray:          F
    Lie down:                  Z
    
    Toggle framerate:          F1
    Take screenshot:           F12

===============
2) FAQ and troubleshooting
===============
    Why did it crash?
        Remember that when you download the engine's zip file, you need to unzip everything inside of it before you're able to play.
        If you did and it keeps crashing, check the file Error_log.txt in the same folder as the game. It should tell you what went wrong. If you think the crash is not your fault, please let me know of the problem.
        Here are some possible solutions and workarounds:
            If you get a crash right after a large area finishes loading, and you have a weak machine, it is possible that the engine failed to render the area's geometry buffers. Please turn down the value of the "area_quality" option in Options.txt. 0.3 or lower might work.
    
    Why can't I see the game window?
        Because windows sometimes break under Windows.
        Open Options.txt, go to the line with "window_pos_hack=false", and change it to "window_pos_hack=true".
    
    Why are some textures black, but not HUD elements?
        I'm not quite sure, but I can tell that this only happens if a) you're using Wine, b) with JPG images.
        So you can either try compiling the engine for your native platform to get rid of the problem, or change the JPG images on the Game_data folder to PNG, but remember to update files for areas, enemies, etc. to point to the new files.
    
    Why are my textures showing up as red "ERROR" text?
        1. Remember that file names are case sensitive. Lowercase and uppercase matter.
        2. Remember that you need to include the extension. This is the ".jpg" or ".png" part of the file name.
        3. Remember that the engine only finds textures if they're in the Game_data/Graphics/Textures folder.
    
    Why is Olimar moving when I'm not touching the analog stick?
        You can open Options.txt and set joystick_min_deadzone to a higher value. This way, a loose analog stick won't be accounted for if you only wiggle it a bit.
    
    Why is it so slow?
        It IS a complex engine running the flexible logic behind several dozens of entities, so it's only natural.
        Check the Options.txt file and use the Pikmin Fanon page for settings that you can change in order to sacrifice appearance for performance.
    
    Why are some parts of the logic and physics so broken?
        Because their code is still under construction.
    
    What is there left to do?
        It's hard to say, but the Pikmin Fanon page has a rough roadmap.
    
    If the engine is just meant to be the game's logic, why does it come with so many enemies, graphics, etc.?
        1. Having some content to play with means people can try out the engine and its features without having to download or create content.
        2. The existing content serves as a basis for new content. If you're having trouble placing a bridge, just see how the included areas do it. If you want to create an Orange Bulborb, just copy the Red bulborb and recolor it.
    
    Why are the animations so clunky and the graphics so simple?
        I'm not an artist. Since nobody else is doing the graphics, I did the best I could.
    
    Why are the textures and HUD all shiny and realistic, but the objects are simplistic?
        Pikmin games have always looked fairly realistic. The engine can achieve pretty environments too, so making the textures simplistic would steal quite a lot of beauty.
        The objects are simple, and based off of vector graphics, so it's easier to edit their graphics. If they were screenshots of 3D models, that would require content creators to have or create models for their new enemies, just so they could take screenshots and put them in-game.
        An alternative style that could work all around is pixel art for everything. Game creators can create their graphics in this style if they so choose.
    
    Why didn't you use textures and sounds from the Pikmin games?
        1. The less copyrighted content I use, the safer I am, even if minimally. There's a difference between "code some fan made from scratch" and "a repository of Nintendo's copyrighted assets right there for the taking".
        2. Should something happen to the engine, I can rebrand it, and release the assets as they are.
        3. If somebody wants to add a sound effect from the series, depending on the game, at best, it can be a bit cumbersome to obtain, at worst, it can be downright impossible; if all sounds are custom-made, this isn't really a problem. This applies to a lesser degree to textures and such as well.
        4. It doesn't really matter anyway because what is included with the engine is demo content; users are meant to use whatever assets they please.
    
    Why 2D?
        3D would be MUCH harder for everyone. You can get something convincing with 2D alone.
        A 3D engine would be much harder to create, and the game developers would have a much harder time creating models, areas, etc. that look good in 3D.
    
    Will you ever add X?
        Depends. If it's an area or enemy or something, the game developers are meant to do that, not me. Though I might still add it for the sake of demonstration. Other than that, ask me!
    
    Is it possible to add Winged Pikmin?
        No. First, their airborne state would be a nightmare to code in with the currently existing finite-state-machine logic.
        Secondly, it's very difficult to convey height in a top-down game as-is, so creating a Pikmin type that has height as its main mechanic wouldn't work well at all.
    
    I'm having a hard time understanding X.
        Please let me know. I try to make the interface, mechanics, and tutorials be as easy to understand as possible.
        If you contact me, I can help you with your problem, as well as change stuff to make it easier to follow in the future.
    
===============
3) Compiling
===============
    (You likely don't need to do this.)
    First, download the source, of course.
    Then, under Linux...
        Install Allegro 5 on your system using the instructions here https://wiki.allegro.cc/index.php?title=Getting_Started
        On a terminal, go to the source's folder and write "make".
        If you get linker errors, edit the "makefile" file and follow the instructions. Write "make clean" and then "make" once more.
    Or under Windows...
        What are you doing? Just download a pre-built binary, please!
        ...But if you really must compile it, install Allegro 5 (5.0.10) by following the instructions here https://wiki.allegro.cc/index.php?title=Getting_Started
        Open the .sln with Visual Studio.
        Compile. Good luck.

===============
4) Credits
===============
    The project's code is licensed under the MIT License (MIT), described in LICENSE.txt.
    Various assets may have their own licenses, which are detailed below.
    Assets whose license and credit are not listed are made by Espyo, and licensed under the MIT License.
    
    Engine
        Inspiration
            Nintendo
        Support and beta-testing
            My friends at Pikipedia and Pikmin Fanon
            My friends at Brawl Snapshots
            Pretty much everyone over on the Pikmin subreddit
        Everything else
            André 'Espyo' Silva
        Tools used
            Allegro
            Codelite
            Visual Studio
            Notepad++
        Special thanks
            Pikipedia and Pikmin Fanon users interested in the project
            The developers of all the tools I used
            The Allegro forums and the users within
            id Software for Doom
            The developers of Doom Builder
            Stackoverflow and other such websites
            http://www.gamasutra.com/blogs/JoshSutphin/20130416/190541/Doing_Thumbstick_Dead_Zones_Right.php
            http://blog.hypersect.com/interpreting-analog-sticks/
            https://gamedev.stackexchange.com/questions/59491/gamepad-thumbsticks-active-range
    
    Demo content
        Graphics
            Bark texture: Daniel Smith - License: Public domain - Source: http://www.publicdomainpictures.net/view-image.php?image=70997&picture=tree-bark-texture-14
            Trunk texture: Caroline Steinhauer - License: Public domain - Source: http://www.publicdomainpictures.net/view-image.php?image=14748&picture=slice-of-oak-tree public domain
            Concrete texture: andreas160578 - License: Public domain - Source: https://pixabay.com/pt/steinplatte-preto-1331975/
            Light grass texture: SilasCarmago - License: Public domain - Source: https://pixabay.com/pt/grama-textura-fundo-grama-verde-1089984/
            Bridge texture: PublicDomainPictures - License: Public domain - Source: https://pixabay.com/pt/abstract-%C3%A1sia-fundo-bambu-cacho-71516/
            Bridge rail texture: Titus Tscharntke - License: Public domain - Source: https://commons.wikimedia.org/wiki/File:Sticks_wood.jpg
            Dirt texture: tatyana - License: CC BY-SA 4.0 - Source: https://sftextures.com/2014/08/15/dry-cracked-orange-desert-ground-with-yellow-sun-burnt-grass-texture/
            Dark grass texture: tatyana - License: CC BY-SA 4.0 - Source: https://sftextures.com/2015/09/04/grass-texture-seamless-green-fresh-summer-lawn/
            Metal texture: tatyana - License: CC BY-SA 4.0 - Source: https://sftextures.com/2014/08/19/rusty-grungy-metal-material-old-weathered-brown-white-grey-painted-surface-texture/
            Stone texture: Bart K. - License: CC BY-SA 3.0 - Source: http://opengameart.org/content/stone-texture
            Area name font
                Original font inspiration: Nintendo
                Fan clone: Adorable Oshawott - License: Fair use with permission - Source: http://theadorableoshawott.deviantart.com/art/Pikmin-font-526210650
                Bubble style: The Sneaky Spy
            Pikmin idle glow texture: hc - License: CC BY-SA 3.0 - Source: http://opengameart.org/node/8279
            Smoke texture: Crack.com - License: Public domain - Source: http://opengameart.org/node/7691
            Main menu background: Helen Marshall - License: Public domain - Source: http://www.publicdomainpictures.net/view-image.php?image=1255
            Leaf shadows: Titus Tscharntke - License: Public domain - Source: https://commons.wikimedia.org/wiki/File:Leaf_branch_texture.jpg https://commons.wikimedia.org/wiki/File:Leaf_leaves_white_background.jpg https://commons.wikimedia.org/wiki/File:Leaf_leaves_texture.jpg https://commons.wikimedia.org/wiki/File:Leaf_leaves_branch_birch_alpha.jpg
            Tools used
                Inkscape
                GIMP
                Paint.NET
        Sounds
            Design help: Scruffy
            Olimar's whistle: Pablo-F - License: CC BY 3.0 - Source: https://freesound.org/people/Pablo-F/sounds/90743/
            Louie's whistle: timgormly - License: Public domain - Source: https://freesound.org/people/timgormly/sounds/162803/
            President's whistle: han1 - License: CC BY 3.0 - Source: https://www.freesound.org/people/han1/sounds/19026/
            Throw: sophiehall3535 - License: Public domain - Source: https://freesound.org/people/sophiehall3535/sounds/245933/
            Pluck: dreeke - License: CC BY-NC 3.0 - Source: https://freesound.org/people/dreeke/sounds/140705/
            Attack: Scruffy
            Tools used
                Audacity
                LMMS
                Chaos Bank V2 soundfont
        Everything else
            André 'Espyo' Silva

===============
5) Disclaimer
===============
    The Pikmin fangame engine and any fan content run within it are NOT affiliated with Nintendo® and should NOT be sold.
    They are NON-PROFIT projects created by fans of the Pikmin® franchise, and do not intend to infringe on the copyrights or registered trademarks of Nintendo®.
    Pikmin® is a trademark of Nintendo®. May contain content taken directly from official sources; said content belongs to Nintendo®.
    
    That said, we highly recommend you buy the official games, as a fangame cannot possibly replace the experience the real games bring!
