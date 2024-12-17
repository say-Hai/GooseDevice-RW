
#ifndef TRANSDATATOSERVER_RFID_H
#define TRANSDATATOSERVER_RFID_H

#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
using namespace std;

class  RFID {
public:
    string user_id;
    string epc;
    string startTime;
    string bar_col;
    int ant;

    friend ostream &operator<<(ostream &os, const RFID &rfid) {
        os << "{" << rfid.user_id << "} {" << rfid.epc << "} {" << rfid.startTime << "} {"
           << rfid.bar_col << "} {" << rfid.ant << "}" << ";";
        return os;
    }

    string to_string() const {
        stringstream ss;
        ss << *this;
        return ss.str();
    }
};


struct pair_hash {
    template<class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ hash2; // Combine the two hash values
    }
};

#endif //TRANSDATATOSERVER_RFID_H
