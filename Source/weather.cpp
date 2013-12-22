#include "weather.h"

weather::weather() {}

weather::weather(string n, map<unsigned, ALLEGRO_COLOR> l, unsigned char pt, interval pf, interval ps, interval pa) {
    this->name = n;
    this->lighting = l;
    this->percipitation_type = pt;
    this->percipitation_frequency = pf;
    this->percipitation_speed = ps;
    this->percipitation_angle = pa;
}