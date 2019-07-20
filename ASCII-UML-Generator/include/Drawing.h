#pragma once

#include <array>
#include <cassert>

#include "Buffer.h"
#include "ClassNode.h"

inline namespace drawing
{

enum class Relation
{
    Composition,
    Aggregation,
    Inheritance,
    Unset
};

void render(const Buffer& buf, std::ostream& out)
{
    for (size_t i = buf.size() - 1; i != 0; --i)
    {
        const auto& line = buf[i];
        out << i << "\t";
        for (auto c : line)
        {
            if (c)
                out << c;
            else
                out << '.';
        }
        out << "\n";
    }
}

void move_closer(int& i, int destination)
{
    if (i < destination)
        ++i;
    else if (i > destination)
        --i;

    // If equal do nothing
}

void drawHorizontalArrow(const Pos& start, const Pos& end, Buffer& buf)
{
    // Draw a straigth line that breaks in the middle point
    //    --------------,
    //                  |
    //                  |
    //                  '----------

    // Draw x
    const int midpoint = (start.x + end.x) / 2;
    for (int i = start.x; i != midpoint; move_closer(i, midpoint))
    {
        // first half
        buf[start.y][i] = '-';
    }

    // Second half
    for (int i = end.x; i != midpoint; move_closer(i, midpoint))
    {
        // first half
        buf[end.y][i] = '-';
    }

    // Draw the corners and the vertical line if the line isn't just a vertical
    if (start.x != end.x)
    {
        if (end.y > start.y)
        {
            // It goes up, so we start with a '
            buf[start.y][midpoint] = '\'';
            buf[end.y][midpoint] = ',';
        }
        else if (end.y < start.y)
        {
            // It goes down, so we add a comma
            buf[start.y][midpoint] = ',';
            buf[end.y][midpoint] = '\'';
        }
        else
        {
            // They are on the same level, just put a - in the middle
            buf[start.y][midpoint] = '-';
        }
    }
    else
    {
        // Just add another | in the "midpoint"
        buf[start.y][midpoint] = '|';
    }

    // Draw vertical line
    int j = start.y;
    // Move one step already as we have ' or , already
    move_closer(j, end.y);
    while (j != end.y)
    {
        // Not very cache friendly, but who cares
        buf[j][midpoint] = '|';

        move_closer(j, end.y);
    }
}

void drawArrowBegin(const Pos& pos, Relation r, Buffer& buffer)
{
    switch (r)
    {
    case Relation::Composition:
        // Fallthrough .. TODO: fix Composition
    case Relation::Aggregation:
        buffer[pos.y][pos.x] = '<';

        // Check it fits
        buffer[pos.y][pos.x + 1] = '>';
        break;

    case Relation::Inheritance:
        // Nothing to do here
        break;
    case Relation::Unset:
        assert(false);
        break;

    default:
        break;
    }
}

void drawArrowEnd(const Pos& pos, Relation r, Buffer& buffer)
{
    switch (r)
    {
    case Relation::Composition:
        // Fallthrough. Nothing to do here
    case Relation::Aggregation:
        break;

    case Relation::Inheritance:
        buffer[pos.y][pos.x] = '^';
        break;
    default:
        assert(false);
        break;
    }
}

void drawArrow(const Pos& start, const Pos& end, Relation relation, Buffer& buf)
{
    drawHorizontalArrow(start, end, buf);
    drawArrowBegin(start, relation, buf);
    drawArrowEnd(end, relation, buf);
}

} // namespace drawing
