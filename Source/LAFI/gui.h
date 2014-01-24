//ToDo automated guis

#ifndef LAFI_GUI_INCLUDED
#define LAFI_GUI_INCLUDED

#include <vector>

#include "widget.h"

using namespace std;

class lafi_gui : public lafi_widget {
private:
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_THREAD* thread;
    
    static void* thread_code(ALLEGRO_THREAD* thread, void* gui);
public:
    bool autonomous;
    bool close_button_quits;
    
    lafi_gui(int w, int h, lafi_style* style = new lafi_style(), unsigned char flags = 0);
    //lafi_gui(unsigned int display_w, unsigned int display_h, bool close_button_quits = true, lafi_style* style = new lafi_style(), unsigned char flags = 0);
    ~lafi_gui();
    
    void stop();
    void wait();
    
    void draw_self();
    
    void render();
};

#endif //ifndef LAFI_GUI_INCLUDED