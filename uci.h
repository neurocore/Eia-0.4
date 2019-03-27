#ifndef UCI_H
#define UCI_H

#include <iostream>
#include <iomanip>

using namespace std;

// Prototypes ///////////////

extern int  parse_input();
extern bool try_parse_int(const string & s, int& r);
extern bool input_available();

#endif // UCI_H
