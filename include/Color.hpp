#pragma once

#include <inttypes.h>

enum class Color : uint16_t {
    Default = 0xFFFF,
    White   = 0xFFFF,
    Black   = 0x0000,
    Red     = 0xfa8a,
    Green   = 0x07e0,
    Blue    = 0x631f,
    Yellow  = 0xffe0,
};

inline Color charToColor(char c) {
    switch (c) {
        case 'r': return Color::Red;
        case 'g': return Color::Green;
        case 'b': return Color::Blue;
        case 'y': return Color::Yellow;
        case 'w': return Color::White;
        case 'k': return Color::Black;
        default:  return Color::Default;
    }
}