#ifndef WEATHER_INCLUDED
#define WEATHER_INCLUDED

#include <map>
#include <string>

#include <allegro5/allegro.h>

#include "interval.h"

using namespace std;

class weather {
public:
    string name;
    map<unsigned, ALLEGRO_COLOR> lighting;
    unsigned char percipitation_type;
    interval percipitation_frequency;
    interval percipitation_speed;
    interval percipitation_angle;
    
    weather();
    weather(string n, map<unsigned, ALLEGRO_COLOR> l, unsigned char pt, interval pf, interval ps, interval pa);
};

#endif //ifndef WEATHER_INCLUDED