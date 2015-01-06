//ToDo automated guis

#ifndef LAFI_GUI_INCLUDED
#define LAFI_GUI_INCLUDED

#include <vector>

#include "widget.h"

using namespace std;

namespace lafi {

/* ----------------------------------------------------------------------------
 * Graphical User Interface. The wrapper that should contain
 * all widgets for a screen.
 */
class gui : public widget {
private:
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_THREAD* thread;
    
    static void* thread_code(ALLEGRO_THREAD* thread, void* gui);
public:
    bool autonomous;
    bool close_button_quits;
    
    gui(int w, int h, lafi::style* style = new lafi::style(), unsigned char flags = 0);
    //gui(unsigned int display_w, unsigned int display_h, bool close_button_quits = true, lafi::style* style = new style(), unsigned char flags = 0);
    ~gui();
    
    void stop();
    void wait();
    
    void draw_self();
};

}

#endif //ifndef LAFI_GUI_INCLUDED
