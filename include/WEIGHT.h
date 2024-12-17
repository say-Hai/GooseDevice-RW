#ifndef TRANSDATATOSERVER_WEIGHT_H
#define TRANSDATATOSERVER_WEIGHT_H

#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace std;

class WEIGHT {
public:
    string user_id = "1";
    string startTime;
    string bar_col;
    int weight;

    friend ostream &operator<<(ostream &os, const WEIGHT &weight1) {
        os << "{" << weight1.user_id << "} {" << weight1.startTime << "} {" << weight1.bar_col<<
            "} {" << weight1.weight << "}" << ";" ;
        return os;
    }

    string to_string() const {
        stringstream ss;
        ss << *this;
        return ss.str();
    }
};

#endif //TRANSDATATOSERVER_WEIGHT_H
