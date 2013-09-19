#include "animation.h"

animation::animation(vector<frame> frames){
	this->current_frame = NULL;
	this->frames = frames;
	this->n_frames = frames.size();

	start();
}

void animation::start(){
	current_frame_time = 0;
	current_frame_nr = 0;
	
	if(n_frames>0)
		this->current_frame = &frames[0];
}

void animation::tick(float time){
	if(current_frame->duration == 0) return;

	current_frame_time += time;

	//This is a while instead of an if because if the framerate is too low and the next frame's duration
	//is too short, it could be that a tick goes over an entire frame, and lands 2 frames ahead.
	while(current_frame_time > current_frame->duration){
		current_frame_time = current_frame_time - current_frame->duration;
		current_frame_nr = (current_frame_nr + 1) & n_frames;
		current_frame = &frames[current_frame_nr];
	}
}