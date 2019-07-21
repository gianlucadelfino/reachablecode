#pragma once

#include <cassert>
#include <initializer_list>

struct Pos
{
    Pos() : x(0),y(0){}
    Pos(int x_, int y_) : x(x_), y(y_) {}
    Pos(std::initializer_list<int> coords) : Pos(*coords.begin(), *std::rbegin(coords))
    {
        assert(coords.size() == 2);
    }
    int x{};
    int y{};

    friend Pos operator+(const Pos& lhs, const Pos& rhs)
    {
        Pos res = lhs;
        res.x += rhs.x;
        res.y += rhs.y;
        return res;
    }

    friend Pos operator-(const Pos& lhs, const Pos& rhs)
    {
        Pos res = lhs;
        res.x -= rhs.x;
        res.y -= rhs.y;
        return res;
    }
};
