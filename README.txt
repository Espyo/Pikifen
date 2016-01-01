Pikmin fangame engine

An engine that allows users to create their Pikmin fangames easily.

This project is still under construction, so expect weird things to happen.
You can find more info here on Pikmin Fanon http://www.pikminfanon.com/Pikmin_fangame_engine or by contacting me via IRC, on DarkMyst's #pikipedia

===============
1) Default controls
2) FAQ
3) Compiling
4) Credits
5) Disclaimer
===============

===============
1) Default controls
===============
    These can be changed on the options menu.
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
    Testing stuff (beware!):   T

===============
2) FAQ
===============
    Why is Louie so fast?
        Because having to walk to a certain point every time I'm testing something wastes a lot of time. So I made Louie fast to get there faster.
    Why are some animations and physics so broken?
        Because their code is still under construction.
    Why can't I see the game window?
        Because windows sometimes break under Windows. Open Options.txt, go to the line with "window_pos_hack=false", and change it to "window_pos_hack=true".

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
