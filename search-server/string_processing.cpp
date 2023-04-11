#include "string_processing.h"
 
 
std::vector<std::string_view> SplitIntoWords( std::string_view text) {
    std::vector<std::string_view> words;
    size_t pos = 0;
    size_t end = text.npos;
    while(true) {
        size_t space = text.find(' ', pos);
        words.push_back(space == end ? text.substr(pos) : text.substr(pos, space - pos));
        if (space == end ) {
            break;
        }
        else {
            pos = space + 1;
        }
    }
    return words;
}