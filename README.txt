Pikmin fangame engine

An engine that allows users to create their Pikmin fangames easily.

This project is still under construction, so expect weird things to happen.
You can find more info on Pikmin Fanon http://www.pikminfanon.com/Pikmin_fangame_engine or by contacting me via IRC, on DarkMyst's #pikipedia.

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
    Most of these can be changed on the options menu.
    Quit:                      Esc
    Move:                      WASD
    Move cursor:               Mouse
    Whistle:                   Right mouse button
    Punch/Pluck/Throw:         Left mouse button
    
    Move group towards cursor: Space
    Switch leader:             Tab
    Dismiss:                   Left Ctrl
    Switch zoom level:         C

    Zoom in:                   Mouse wheel up
    Zoom out:                  Mouse wheel down
    Switch held Pikmin type:   Right mouse button
    Use top spray:             R
    Use bottom spray:          F
    Lie down:                  Z
    
    Toggle framerate:          F1
    Take screenshot:           F12

===============
2) FAQ and troubleshooting
===============
    Why are some parts of the logic and physics so broken?
        Because their code is still under construction.
    Why are the animations so clunky and the graphics so simple?
        I'm not an artist. Since nobody else is doing the graphics, I did the best I could.
    Why can't I see the game window?
        Because windows sometimes break under Windows. Open Options.txt, go to the line with "window_pos_hack=false", and change it to "window_pos_hack=true".
    Why is it so slow?
        It IS a complex engine running the flexible logic behind several dozens of entities, so it's only natural. Check the Options.txt file and use the wiki page for settings that you can change in order to sacrifice appearance for performance.
    Why did it crash?
        Remember that when you download the engine's zip file, you need to unzip everything inside of it before you're able to play. Other than that, check the file Error_log.txt on the same folder as the game. It should tell you what went wrong. If you think the crash is not your fault, please let me know of the problem.

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
        ...But if you really must compile it, install Allegro 5 by following the instructions here https://wiki.allegro.cc/index.php?title=Getting_Started
        Open the .sln with Visual Studio.
        Compile. Good luck.

===============
4) Credits
===============
    Pretty much everything:
        André 'Espyo' Silva
    Special thanks:
        My friends at Pikipedia and Pikmin Fanon
        My friends at Brawl Snapshots
        id Software for Doom
        The developers of Doom Builder
        stackoverflow and other such websites

===============
5) Disclaimer
===============
    The Pikmin fangame engine and any fan content run within it are NOT affiliated with Nintendo® and should NOT be sold.
    They are NON-PROFIT projects created by fans of the Pikmin® franchise, and do not intend to infringe on the copyrights or registered trademarks of Nintendo®.
    Pikmin® is a trademark of Nintendo®. The copyrights of all associated characters, names, terms, art, music, and assets belong to Nintendo®.
    
    That said, we highly recommend you buy the official games, as a fangame cannot possibly replace the experience the real games bring!
