#include "weather.h"

weather::weather() {}

weather::weather(const string &n, const map<unsigned, ALLEGRO_COLOR> &l, const unsigned char pt, const interval pf, const interval ps, const interval pa) {
    this->name = n;
    this->lighting = l;
    this->percipitation_type = pt;
    this->percipitation_frequency = pf;
    this->percipitation_speed = ps;
    this->percipitation_angle = pa;
}