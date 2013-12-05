#include "weather.h"

weather::weather() {}

weather::weather(string n, map<unsigned, ALLEGRO_COLOR> l) {
    this->name = n;
    this->lighting = l;
}