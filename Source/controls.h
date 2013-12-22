#ifndef CONTROLS_INCLUDED
#define CONTROLS_INCLUDED

struct control_info {
    unsigned char action;
    unsigned char player;
    unsigned char type;
    int device_nr;
    int button;
    int stick;
    int axis;
    
    control_info(unsigned char action, unsigned char player, string s);
    string stringify();
};

void handle_game_controls(ALLEGRO_EVENT ev);
void handle_button(unsigned int button, float pos);

#endif //ifndef CONTROLS_INCLUDED