#ifndef LAFI_IMAGE_INCLUDED
#define LAFI_IMAGE_INCLUDED

#include <string>

#include "widget.h"


namespace lafi {

/* ----------------------------------------------------------------------------
 * An image widget. Basically, an Allegro bitmap.
 * The bitmap is NOT managed by the widget, so make sure it is not NULL.
 */
class image : public widget {
private:
    ALLEGRO_BITMAP* bmp;
    
public:
    image(
        const int x1, const int y1, const int x2, const int y2,
        ALLEGRO_BITMAP* bmp = NULL, lafi::style* style = NULL,
        const unsigned char flags = 0
    );
    image(ALLEGRO_BITMAP* bmp = NULL);
    image(image &i2);
    
    void draw_self();
};

}

#endif //ifndef LAFI_IMAGE_INCLUDED
