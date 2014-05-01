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
    weather(const string &n, const map<unsigned, ALLEGRO_COLOR> &l, const unsigned char pt, const interval pf, const interval ps, const interval pa);
};

enum PERCIPITATION_TYPES {
    PERCIPITATION_TYPE_NONE,
    PERCIPITATION_TYPE_RAIN,
    PERCIPITATION_TYPE_WIND,
};

#endif //ifndef WEATHER_INCLUDED