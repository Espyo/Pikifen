#ifndef LAFI_IMAGE_INCLUDED
#define LAFI_IMAGE_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

namespace lafi {

/* ----------------------------------------------------------------------------
 * An image widget. Basically, an Allegro bitmap.
 * The bitmap is NOT managed by the widget, so make sure it is not NULL.
 */
class image : public widget {
private:
    ALLEGRO_BITMAP* bmp;
    
public:
    image(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, ALLEGRO_BITMAP* bmp = NULL, lafi::style* style = NULL, unsigned char flags = 0);
    image(image &i2);
    ~image();
    
    void draw_self();
};

}

#endif //ifndef LAFI_IMAGE_INCLUDED
