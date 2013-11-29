#ifndef STATE_INCLUDED
#define STATE_INCLUDED

#include <vector>

using namespace std;

class state {
    vector<state*> next_states;     //If there's only one, it uses that one. If not, it picks at random.
    float state_time;
};

#endif //ifndef STATE_INCLUDED
