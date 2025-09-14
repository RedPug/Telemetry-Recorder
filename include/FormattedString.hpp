#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cstddef>
#include <regex>
#include "Color.hpp"

class FormattedString {
public:
    using Segment = std::pair<std::string, Color>;
    using SegmentList = std::vector<Segment>;
    using iterator = SegmentList::const_iterator;

    template<typename... Args>
    explicit FormattedString(const std::string& input, Args... args) {
        parse(input, args...);
        source = input;
    }

    template<typename... Args>
    explicit FormattedString(const char* input, Args... args) {
        parse(input, args...);
        source = input;
    }

    FormattedString& operator=(const std::string& other){
        parse(other);
        return *this;
    }

    FormattedString& operator=(const FormattedString& other){
        source = other.source;
        segments_ = other.segments_;
        return *this;
    }

    bool operator==(const FormattedString& other) const {
        return segments_ == other.segments_;
    }

    bool operator!=(const FormattedString& other) const {
        return segments_ != other.segments_;
    }

    operator std::string() const {
        return source;
    }
    

    iterator begin() const { return segments_.begin(); }
    iterator end() const { return segments_.end(); }

private:
    SegmentList segments_;
    std::string source;

    template<typename... Args>
    void parse(const std::string& input, Args... args) {
        
        static char buff[64];
        sprintf(buff, input.c_str(), args...);
        std::string str = std::string(buff);


        segments_ = SegmentList();

        //format $_{text}
        std::regex re(R"(\$(\w)\{([^}]*)\})");
        std::sregex_iterator it(str.begin(), str.end(), re);
        std::sregex_iterator end;

        size_t lastPos = 0;
        Color currentColor = Color::Default;

        for (; it != end; ++it) {
            size_t matchPos = it->position();
            // Add text before the match as default color
            if (matchPos > lastPos) {
                segments_.emplace_back(str.substr(lastPos, matchPos - lastPos), Color::Default);
            }
            // Add colored segment
            char colorChar = it->str(1)[0];
            std::string text = it->str(2);
            segments_.emplace_back(text, charToColor(colorChar));
            lastPos = matchPos + it->length();
        }
        // Add remaining text as default color
        if (lastPos < str.size()) {
            segments_.emplace_back(str.substr(lastPos), Color::Default);
        }
    }
};