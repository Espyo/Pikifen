#ifndef LAFI_LABEL_INCLUDED
#define LAFI_LABEL_INCLUDED

#include <string>

#include "widget.h"

using std::string;

namespace lafi {

/* ----------------------------------------------------------------------------
 * Label widgets contain text. Simple as that.
 */
class label : public widget {
private:
    string prev_text;
    float offset;
    float offset_start_time_left;
    float offset_reset_time_left;
    
    static const float OFFSET_START_DELAY;
    static const float OFFSET_RESET_DELAY;
    static const float OFFSET_SPEED;
    
public:
    string text;
    int text_flags;
    bool autoscroll;
    
    label(
        const int x1, const int y1, const int x2, const int y2,
        const string &text = "", const int text_flags = 0,
        const bool autoscroll = false,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    label(
        const string &text = "", const int text_flags = 0,
        const bool autoscroll = false
    );
    
    void widget_on_tick(const float time);
    void draw_self();
};

}

#endif //ifndef LAFI_LABEL_INCLUDED
