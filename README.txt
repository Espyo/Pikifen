Pikifen

A fan-made Pikmin engine that allows users to create and play their own Pikmin content.

This project, formerly called "Pikmin fangame engine", is still under construction, so expect weird things to happen.
You can find more info on Pikmin Fanon http://www.pikminfanon.com/Pikifen or by contacting me, Espyo. You can find contacts in the Pikmin Fanon page.

===============
1) Controls
2) How to create
3) FAQ and troubleshooting
4) Compiling
5) Changelog
6) Tips
7) Credits
8) Disclaimer
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
2) How to create
===============
    To create content for the engine, just edit the image, sound, or text files in the "Game_data" folder.
    Some things can also be edited using the built-in editors on the main menu.
    Everything should be intuitive, but you may want to check the tutorials here: http://www.pikminfanon.com/Pikifen#How_to_create

===============
3) FAQ and troubleshooting
===============
    Playing problem troubleshooting
        Why did it crash?
            If you made some content for it and you suspect that that is the problem, please check the "User_data/Error_log.txt" file, if it exists. It should tell you what went wrong, like a missing comma, or a misspelled name. Fix the problems and try again.
            If you think the crash is due to a problem with the engine itself, then it could be one of the following:
                1. You're not using the most recent version of the engine, which could have solved that problem. You should try upgrading if you're not up-to-date.
                2. The game is running from inside a zip file. Remember that when you download the engine's zip file, you need to unzip everything inside of it into a folder, before you're able to play.
                3. It's a problem that I need to fix. Please get in touch with me about it. I'll likely ask you to explain what you were doing when the crash happened, ask you to send me any errors in the "User_data/Error_log.txt" file, and ask you to send me the crash screenshots in the "User_data" folder. So you should try to get those ready.
        
        Why can't I see the game window?
            Because windows sometimes break under Windows.
            Open "User_data/Options.txt", go to the line with "window_pos_hack=false", and change it to "window_pos_hack=true".
        
        Why is it running slowly?
            Well, it IS a complex engine running the flexible logic behind several dozens of entities, so it's only natural.
            You can check the "User_data/Options.txt" file and use the Pikmin Fanon page for settings that you can change in order to sacrifice appearance for performance.
            If you have created a particularly large area or complex creatures, maybe consider toning them down, since they can have an impact on performance.
        
        Why are some parts of the logic and physics so broken?
            Because their code is still under construction, most likely.
        
        Why are some textures black, but not HUD elements?
            Are you running it under Wine? If so, update Wine. That fixed the problem for a friend that had it. If not, contact me, and I'll try to see what's up.
        
        Why does the engine think I'm touching the analog stick when I'm not?
            You can open "User_data/Options.txt" and set "joystick_min_deadzone" to a higher value. This way, a loose analog stick won't be accounted for if it is simply wiggling a bit.
        
    Content creation troubleshooting
        Why do I see everything in a single line when I open a text file with Notepad?
            Don't use Notepad. It has a hard time doing LF-only line breaks, which most of the engine's files use.
            Install Notepad++ and start using that instead, since it's better in every way.
        
        Why are my graphics showing up as black and violet patterns?
            That's what the engine shows when it can't open a texture. Make sure you're following these points:
            1. File names are case sensitive. Lowercase and uppercase matter.
            2. You need to include the extension. This is the ".jpg" or ".png" part of the file name.
            3. The engine only finds the image in one folder. General images go in "Game_data/Graphics", and textures go in "Game_data/Graphics/Textures".
        
        I'm having a hard time understanding X.
            Please let me know. I try to make the interface, mechanics, and tutorials be as easy to understand as possible.
            If you contact me, I can help you with your problem, as well as change things to make them easier to understand in the future.
        
    Engine development questions
        What is there left to do?
            It's hard to say, but the Pikmin Fanon page has a rough roadmap.
        
        If the engine is just meant to be the game's logic, why does it come with so many enemies, graphics, etc.?
            1. Having some content to play with means people can try the engine and its features right out of the box, without having to download or create content.
            2. The existing content serves as a basis for new content. If you're having trouble placing a bridge, just see how the included areas do it. If you want to create an Orange Bulborb, just copy the Red Bulborb and recolor it. And so on.
        
        Why do some enemies act a bit differently from how they do in the canon games?
            It could either be an engine limitation (since Pikifen isn't as complex as a Nintendo product), or it could've been a change I made on purpose. The engine's content isn't meant to mirror the canon content exactly, and sometimes, I make slight changes to features and content because it's needed in order to be more readable in a top-down format, because I think it will make for a funner, fairer experience, etc.
        
        Why are the animations so clunky and the graphics so simple?
            I'm not an artist! Since nobody else is doing the graphics, I did the best I could.
        
        Why are the textures and HUD all shiny and realistic, but the objects are simplistic?
            It's the style choice I went with for the packaged content, but anybody can style the textures, HUD, and objects in any way they want.
            In my opinion, Pikmin games have always looked fairly realistic. The engine can achieve pretty environments too, so making the textures simplistic would steal quite a lot of beauty.
            The objects are simple, and based off of vector graphics, so it's easier to edit their graphics. If they were screenshots of 3D models, that would require content creators to have or create models for their new enemies, just so they could take screenshots in order to follow the pre-packaged content's style.
            An alternative style that could maybe work all around is pixel art for everything, but I haven't tried it.
        
        Why didn't you use textures and sounds from the Pikmin games?
            1. The less copyrighted content I use, the safer I am, even if minimally. There's a difference between "code some fan made from scratch" and "a repository of Nintendo's copyrighted assets right there for the taking".
            2. Should something happen to the engine, I can rebrand it, and release the assets as they are, since they are my creations.
            3. If somebody wants to add a sound effect from the series, at best, it can be a bit cumbersome to obtain, at worst, it can be downright impossible (depends on the game and the sound). But if all sounds are custom-made, this isn't really a problem. This applies to textures and such as well, to a lesser degree.
            4. It doesn't really matter anyway because what is included with the engine is demo content; users are meant to use whatever assets they please.
        
        Why 2D?
            3D would be MUCH harder for me to develop, and for players to create content for. You can get something convincing with 2D alone.
        
        Will you ever add X?
            Depends. If it's an area or enemy or something, the content creators are meant to do that, not me. Though I might still add it for the sake of demonstration. But if it's something like a feature, ask me!
        
        Why do some updates make previously created content incompatible for such small reasons?
            I believe that making the way things work, are organized, are named, etc. consistent is the key to understanding the engine.
            If I give a feature a certain name, but later on, rework it and figure out a better name for it, I will rename it (and break some compatibilities in the process), because I want the feature to be intuitive. If it kept the old name, it would confuse newcomers.
            I don't like making content incompatible, but I think it's a small price to pay for the sake of making everyone able to juice the most out of the engine.
        
        Why are some features so customizable and others so strict?
            It's very hard to balance responsibility and power when creating a feature.
            I want to give the developer as much power as possible, but that also increases the developer's responsibility to learn how the feature works in detail, and how to use it properly. To try to make it easy for anyone to create content, I have to lower the responsibility and power of some features.
        
        Is it possible to add Winged Pikmin?
            Unlikely... First, their airborne state would be a nightmare to code in with the currently existing finite-state-machine logic.
            Secondly, it's very difficult to convey height in a top-down game as-is, so creating a Pikmin type that has height as its main mechanic wouldn't work well at all.

===============
4) Compiling
===============
    (You don't need to do this if you download the zip file with the engine already built!)
    First, download the source, of course. Then...
    
    Under Windows, with Visual Studio 2019:
        Open the VS2019 .sln file with Visual Studio 2019.
        Compile.
        If you get any errors, try the following troubleshooting tips:
            If you get "'min': identifier not found" (or 'max'):
                At the top of the file with the error add, near the other includes, "#include <algorithm>". (I try to fix these errors for Windows builds, but I don't do it often, so bear with me!)
            If you get tons of "expected an identifier" and "namespace __ has no member --" errors:
                Try compiling with the Release x64 target.
            If you get "Element <LanguageStandard> has an invalid value of "Default"":
                Project properties, C/C++, Language, C++ Language, C++ Language Standard: Set it to the highest available option, hopefully C++ 17.
            If you don't have the Windows 10 SDK:
                Right-click the solution, Retarget solution, and choose whatever the most recent option is.
            It might only work if, during Visual Studio's installation, you've included "MSVC v142 - VS 2019 C++ x64/x86 Build Tools" and "Windows 10 SDK".
    
    Under Windows, with Visual Studio 2010:
        Install Allegro 5 (5.0.10) by following the instructions here https://wiki.allegro.cc/index.php?title=Getting_Started
        Open the VS2010 .sln file with Visual Studio 2010.
        Go to the project's properties, have a look around, and see if any attribute needs to be changed to your needs, like the Allegro directories.
        Compile. Good luck...
    
    Under Windows, with another version of Visual Studio:
        Follow the instructions for Visual Studio 2019, but go to the project properties, General, Platform Toolset, and set it to whatever you have.
    
    Under Linux, with g++ or Clang:
        Install Allegro 5.2 on your system using the instructions here https://wiki.allegro.cc/index.php?title=Getting_Started
            If you are on Ubuntu, I recommend this link https://wiki.allegro.cc/index.php?title=Install_Allegro_from_Ubuntu_PPAs&oldid=6853
        On a terminal, go to the engine's root folder and write "make".
        If you get linker errors, edit the "makefile" file and follow the instructions. Write "make clean" and then "make" once more.
        If you prefer to use Clang, edit the "makefile" file and follow the instructions. Write "make clean" and then "make" once more.
        If you prefer to compile with Allegro 5.0.10 instead, edit the "makefile" file and add a ".0" after every "5" in the "LDFLAGS" line.

===============
5) Changelog
===============
    See http://www.pikminfanon.com/Pikifen/Changelog

===============
6) Tips
===============
    * If you dismiss while ordering the group to go in a certain direction, they will be dismissed in that direction.
    * To drop a held Pikmin without throwing it, keep holding the throw button, and press the whistle button.
    * If you're having trouble understanding why your custom mob is acting the way it is, try aiming your cursor at it
        and using the "mob info" creator tool (5 key, by default).

===============
7) Credits
===============
    The project's code is licensed under the MIT License (MIT), described in LICENSE.txt.
    Various assets may have their own licenses, which are detailed below.
    Assets whose license and credit are not listed are made by Espyo, and licensed under the MIT License.
    
    Engine
        Inspiration
            Nintendo
        Support and beta-testing
            My friends at Pikipedia and Pikmin Fanon
                Special shoutout to Neo, Kman, Scruffy
            My friends at Brawl Snapshots
                Special shoutout to Gaming98, Deku, FATBEN
            Everyone at the Pikifen Discord server
        Dedicated beta-testing
            Gaming98, Deku
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
            http://wyw.dcweb.cn/leakage.htm
            http://www.dummies.com/education/science/physics/how-to-calculate-the-maximum-height-of-a-projectile/
            http://www.melloland.com/scripts-and-tutos/collision-detection-between-circles-and-lines
            http://stackoverflow.com/questions/2049582/how-to-determine-a-point-in-a-triangle
            http://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
    
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
            Pikmin standby icons: CraftedPBody (for making the basis)
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
8) Disclaimer
===============
    Pikifen and any fan content run within it are NOT affiliated with Nintendo® and should NOT be sold.
    They are NON-PROFIT projects created by fans of the Pikmin® franchise, and do not intend to infringe on the copyrights or registered trademarks of Nintendo®.
    Pikmin® is a trademark of Nintendo®. May contain content taken directly from official sources; said content belongs to Nintendo®.
    
    That said, we highly recommend you buy the official games, as a fangame cannot possibly replace the experience the real games bring!
