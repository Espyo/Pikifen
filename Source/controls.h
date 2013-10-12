#ifndef	CONTROLS_INCLUDED
#define	CONTROLS_INCLUDED

void handle_controls(ALLEGRO_EVENT ev);
void handle_button_down(unsigned int button, int aux=0);
void handle_button_up(unsigned int button);
void handle_analog(unsigned int action, float pos, bool x_axis);
void handle_mouse(unsigned int action, float mx, float my);

#endif //ifndef CONTROLS_INCLUDED