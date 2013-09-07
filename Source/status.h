#ifndef STATUS_INCLUDED
#define STATUS_INCLUDED

class status{
	float speed_multiplier;
	float attack_multiplier;
	float defense_multiplier;
	float state_speed_multiplier;
	bool freezes_everything;
	ALLEGRO_COLOR color;
	unsigned char affects;		//What kind of objects it affects.
};

enum{
	STATUS_AFFECTS_PIKMIN=1,
	STATUS_AFFECTS_ENEMIES=2,
	STATUS_AFFECTS_LEADERS=4,
	STATUS_AFFECTS_ENEMY_PIKMIN=8,
	STATUS_AFFECTS_HAZARDS=16,
};

#endif //ifndef STATUS_INCLUDED
