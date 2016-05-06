#include "image.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates an image.
 */
image::image(
    int x1, int y1, int x2, int y2, ALLEGRO_BITMAP* bmp,
    lafi::style* style, unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
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
