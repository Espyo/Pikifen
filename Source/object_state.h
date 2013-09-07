#ifndef OBJECT_STATE_INCLUDED
#define OBJECT_STATE_INCLUDED

#include <vector>

#include "element.h"
#include "state.h"

using namespace std;

class object_state : public state{
public:
	vector<element*> elements;
};

#endif //ifndef OBJECT_STATE_INCLUDED
