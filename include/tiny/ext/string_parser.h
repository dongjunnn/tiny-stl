#include <ios>
#include <iterator>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

// string substr (size_t pos = 0, size_t len = npos) const;
// The substring is the portion of the object that starts at character position pos and spans len characters (or until the end of the string, whichever comes first).

// s.find(what_to_find, where_to_start);
// find() returns:
// - Found: the index of the first match, starting from 0.
// - Not found: string::npos, a special value meaning “no position.”
// 
// The main way here is to do sliding window style instead of recreating a new string everytime

std::vector<std::string> split(const std::string& str, const std::string& deliminator) {
    if (deliminator.size() == 0) {
        throw std::invalid_argument("invalid argument");
    }
    std::vector<std::string> res;

    std::size_t start_pos{0};

    // while (start_pos < str.size()) { -> this misses the case that the string ends with deliminator
    while (true) {
        std::size_t end_pos = str.find(deliminator, start_pos);
        if (end_pos == std::string::npos) {
            res.push_back(str.substr(start_pos));
            break;
        }
        res.push_back(str.substr(start_pos, end_pos - start_pos));
        start_pos = end_pos + deliminator.size();
    }

    return res;
}

// getline version
