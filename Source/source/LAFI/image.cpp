#include "image.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates an image.
 */
image::image(
    const int x1, const int y1, const int x2, const int y2, ALLEGRO_BITMAP* bmp,
    lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    bmp(bmp) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an image.
 */
image::image(ALLEGRO_BITMAP* bmp) :
    widget(),
    bmp(bmp) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys an image.
 */
image::~image() {}


/* ----------------------------------------------------------------------------
 * Draws the actual image.
 */
void image::draw_self() {
    float bmp_w = al_get_bitmap_width(bmp);
    float bmp_h = al_get_bitmap_height(bmp);
    al_draw_scaled_bitmap(
        bmp, 0, 0, bmp_w, bmp_h,
        x1, y1,
        (x2 - x1), (y2 - y1),
        0
    );
}

}
