This file will explain my process behind creating spritesheets. It should also help you navigate my SVG files. Keep in mind that this might not be 100% accurate, especially for older files.

I use Inkscape for my development.
I have two layers, one for the green (0 128 0 ) "grid", one for the sprites themselves (and sometimes another one for extra things)
I get screenshots from games and trace on top of them.
    No outlines.
    Shading is minimal.
    Everything uses a gradient, which is always named.
For every gradient I use, I create a square with it outside of the spritesheet, to the left, and explain what color it is.
For every sprite, I align the grid to wrap it snuggly. I also make the top-left of the spritesheet be at 0, 0. The position and size of the grid's rectangles is what I use on the animation editor, like so:
    To get a frame's X, width, and height, I just click its rectangle on the grid and check the properties.
    To get a frame's Y, I know it's 0 if it's at the top row, but if not, I click the grid rectangle above it, and use that Y, but add one and flip from negative to positive (the + 1 is because of the grid spacing).
I try to create just enough frames to run animations at 10FPS.
When I want to export the final spritesheet:
    I lock the sprites layer.
    I select everything on the grid layer with a simple rectangle selection.
    I set the export section to the selection's span coordinates.
    I hide the grid layer and show the spritesheet layer, and export with those settings.

To recolor a spritesheet, simply select the squares that use the colors, and edit their gradient. It should edit the coloring of everything on the spritesheet.