#ifndef LAFI_LABEL_INCLUDED
#define LAFI_LABEL_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

namespace lafi {

/* ----------------------------------------------------------------------------
 * Label widgets contain text. Simple as that.
 */
class label : public widget {
public:
    int text_flags;
    
    string text;
    
    label(
        const int x1, const int y1, const int x2, const int y2,
        const string &text = "", const int text_flags = 0,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    label(const string &text = "", const int text_flags = 0);
    ~label();
    
    void draw_self();
};

}

#endif //ifndef LAFI_LABEL_INCLUDED
