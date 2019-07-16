#pragma once

#include <cassert>
#include <initializer_list>

struct Pos
{
    Pos() : x(0), y(0) {}
    Pos(int x_, int y_) : x(x_), y(y_) {}
    Pos(std::initializer_list<int> coords)
        : Pos(*coords.begin(), *std::rbegin(coords))
    {
        assert(coords.size() == 2);
    }
    int x{};
    int y{};

    bool operator==(const Pos& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const Pos& rhs) const { return !operator==(rhs); }

    void swap(Pos& other_) noexcept
    {
        std::swap(x, other_.x);
        std::swap(y, other_.y);
    }

    enum class Coord
    {
        X = 0,
        Y = 1
    };
    int operator[](Coord c) const { return static_cast<size_t>(c) & 1 ? y : x; }
    int operator[](size_t i) const { return i & 1 ? x : y; }

    friend Pos operator+(const Pos& lhs, const Pos& rhs)
    {
        Pos res = lhs;
        res.x += rhs.x;
        res.y += rhs.y;
        return res;
    }

    Pos& operator+=(const Pos& rhs)
    {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }

    Pos& operator-=(const Pos& rhs)
    {
        this->x -= rhs.x;
        this->y -= rhs.y;
        return *this;
    }

    friend Pos operator-(const Pos& lhs, const Pos& rhs)
    {
        Pos res = lhs;
        res.x -= rhs.x;
        res.y -= rhs.y;
        return res;
    }
};
