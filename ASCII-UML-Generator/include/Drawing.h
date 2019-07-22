#pragma once

#include <array>
#include <cassert>
#include <limits>

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

enum Direction
{
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    END
};
const std::vector<Pos> directions{{0, 1}, {0, -1}, {-1, 0}, {1, 0}};

int computeDistance(const std::vector<Direction>& path)
{
//    static std::unordered_map<> memoized;
    int size{};
    for (auto iter = std::next(path.cbegin()); iter != path.cend(); ++iter)
    {
        if (*iter != *std::prev(iter))
            size += 2;
        else
            size += 1;
    }
    return size;
}

std::vector<Direction> findPath(const Pos& cur_pos,
                                const Pos& end,
                                const Buffer& buffer,
                                std::vector<Direction> path = {})
{
    // Find all possible paths recursively, returning the shorten one with
    // The least number of turns!
    if (cur_pos == end)
    {
        return path;
    }

    std::vector<Direction> chosenPath = path;
    int shortestPath = std::numeric_limits<int>::max();

    for (int i = Direction::UP; i != Direction::END; ++i)
    {
        Direction d = static_cast<Direction>(i);
        const Pos newPos = cur_pos + directions[d];

        // Don't go futher
        if (std::abs(newPos.x - end.x) > std::abs(cur_pos.x - end.x) or
            std::abs(newPos.y - end.y) > std::abs(cur_pos.y - end.y))
            continue;

        // If you find an empty cell, then consider the branch
        if (buffer.isPosValid(newPos) && !buffer.at(newPos))
        {
            // Debug
            //            Buffer& hackedBuffer = const_cast<Buffer&>(buffer);
            //            hackedBuffer.at(newPos) = '@';
            //            render(hackedBuffer, std::cout);

            std::vector<Direction> currentPath = path;
            currentPath.push_back(d);

            currentPath = std::move(
                findPath(newPos, end, buffer, std::move(currentPath)));

            const int branchDistance = computeDistance(currentPath);
            if (branchDistance <= shortestPath)
            {
                shortestPath = branchDistance;
                chosenPath = std::move(currentPath);
            }
        }
    }

    return chosenPath;
}

void drawHorizontalLine_v2(const Pos& start, const Pos& end, Buffer& buffer)
{
    std::vector<Direction> shortestPath = findPath(start, end, buffer);

    Pos cur_pos = start;
    //   for (auto iter = std::next(shortestPath.cbegin()); iter != path.cend();
    //   ++iter)
    for (auto d : shortestPath)
    {
        //        const Direction d = *iter;
        cur_pos += directions[d];
        assert(buffer.isPosValid(cur_pos));
        assert(d != Direction::END);

        buffer.at(cur_pos) = '*';
    }

    //        const bool directionChanged =  *std::prev(iter) != *iter;

    //        switch (d) {
    //        case Direction::UP:
    //            if (directionChanged)
    //                buffer.at(newPos)= '\'';
    //            else
    //                buffer.at(newPos) = '|';
    //            break;
    //        case Direction::DOWN:
    //            if (directionChanged)
    //                buffer.at(newPos) = ',';
    //            else
    //                buffer.at(newPos) = '|';
    //            break;
    //        case Direction::LEFT:
    //            if (std::prev(iter) == Direction::UP)
    //                buffer.at(newPos) = '';
    //            else

    //            break;
    //        case Direction::RIGHT:
    //            break;

    //        case Direction::END:
    //            assert(false);
    //            break;

    //        }

    //        if (directionChanged)
    //        {
    //            // If it goes up go ', otherwise ,
    //            if (d == Direction::UP)
    //            {
    //                buffer.at(newPos) = '\'';
    //            }
    //        }
    //        else
    //        {}

    //        if (d == Direction::UP or d == Direction::DOWN)
    //        {
    //            if ()
    //            buffer.at(newPos) = '|';
    //        }
    //        else
    //        {
    //            buffer.at(newPos) = '-';
    //        }
    //    }
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
        drawHorizontalLine_v2(start, end, buf);
        //        drawHorizontalLine(start, end, buf);
    }
    drawArrowBegin(start, relation, buf);
    drawArrowEnd(end, relation, buf);
}

} // namespace drawing
