#ifndef	CONTROLS_INCLUDED
#define	CONTROLS_INCLUDED

void handle_controls(ALLEGRO_EVENT ev);
void handle_button(unsigned int button, float pos);
void handle_mouse(unsigned int action, float mx, float my);

struct control_info{
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

#endif //ifndef CONTROLS_INCLUDED