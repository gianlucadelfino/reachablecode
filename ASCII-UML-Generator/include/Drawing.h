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

void drawHorizontalLine(const Pos& start, const Pos& end, Buffer& buf)
{
    if (start.x > end.x)
    {
        drawHorizontalLine(end, start, buf);
        return;
    }
    // Draw a straigth line that breaks in the middle point
    //    --------------,
    //                  |
    //                  |
    //                  '----------

    // Draw the first half of the horizontal line
    const int midpoint = (start.x + end.x) / 2;
    for (int i = start.x; i != midpoint; ++i)
    {
        buf[start.y][i] = '-';
    }

    // Second half
    for (int i = midpoint; i != end.x; ++i)
    {
        buf[end.y][i] = '-';
    }

    // Draw the corners and the horizontal line if the line isn't just a
    // horizontal
    if (start.y < end.y)
    {
        // It goes up, so we start with a '
        buf[start.y][midpoint] = '\'';
        buf[end.y][midpoint] = ',';
    }
    else if (start.y > end.y)
    {
        // It goes down, so we add a comma
        buf[start.y][midpoint] = ',';
        buf[end.y][midpoint] = '\'';
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

void drawVerticalLine(const Pos& start, const Pos& end, Buffer& buf)
{
    if (start.y > end.y)
    {
        drawVerticalLine(end, start, buf);
        return;
    }
    // Draw a straigth line that breaks in the middle point
    //    |
    //    |
    //    '---------------,
    //                    |
    //                    |

    // Draw first half of the vertical
    const int midpoint = (start.y + end.y) / 2;
    for (int i = start.y; i != midpoint; ++i)
    {
        buf[i][start.x] = '|';
    }

    // Second half
    for (int i = midpoint; i != end.y; ++i)
    {
        buf[i][end.x] = '|';
    }

    // Draw the corners and the vertical line if the line isn't just a
    // vertical
    if (start.x != end.x)
    {
        buf[midpoint][start.x] = ',';
        buf[midpoint][end.x] = '\'';
    }

    // Draw horizontal line
    int j = start.x;
    // Move one step already as we have ' or , already
    move_closer(j, end.x);
    while (j != end.x)
    {
        buf[midpoint][j] = '-';

        move_closer(j, end.x);
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
    assert(relation != Relation::Unset);
    if (relation == Relation::Inheritance)
    {
        drawVerticalLine(start, end, buf);
    }
    else
    {
        drawHorizontalLine(start, end, buf);
    }
    drawArrowBegin(start, relation, buf);
    drawArrowEnd(end, relation, buf);
}

} // namespace drawing
