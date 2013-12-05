#ifndef WEATHER_INCLUDED
#define WEATHER_INCLUDED

#include <map>
#include <string>

#include <allegro5/allegro.h>

using namespace std;

class weather {
public:
    string name;
    map<unsigned, ALLEGRO_COLOR> lighting;
    
    weather();
    weather(string n, map<unsigned, ALLEGRO_COLOR> l);
};

#endif //ifndef WEATHER_INCLUDED