#ifndef ANIMATION_INCLUDED
#define	ANIMATION_INCLUDED

#include <vector>

#include "frame.h"

using namespace std;

class animation{
private:
	size_t current_frame_nr; //Just a helper, used when switching to the next frame, in order to save on CPU.
	size_t n_frames;

public:
	vector<frame> frames;
	frame* current_frame;
	float current_frame_time;

	animation(vector<frame> frames);

	void start();
	void tick(float time);
};

#endif //ifndef ANIMATION_INCLUDED