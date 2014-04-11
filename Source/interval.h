#ifndef INTERVAL_INCLUDED
#define INTERVAL_INCLUDED

#include <cfloat>
#include <vector>

using namespace std;

struct subinterval {
    float lower; //Lower boundary of the interval, inclusive. FLT_MIN for none.
    float upper; //Upper boundary of the interval, inclusive. FLT_MAX for none.
    float divisor; //Every X numbers, starting on the lower boundary (or on 0 if there is no lower boundary). 0 for no divisor.
    subinterval(float l = FLT_MIN, float u = FLT_MAX, float d = 0);
};

class interval {
private:
    vector<subinterval> subintervals;
    
public:
    interval(string s = "");
    float get_random_number();
    bool is_number_in_interval(float n);
};

#endif //ifndef INTERVAL_INCLUDED
