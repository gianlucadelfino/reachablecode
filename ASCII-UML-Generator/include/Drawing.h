#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <map>
#include <queue>
#include <unordered_map>

#include "Buffer.h"
#include "ClassNode.h"
#include "Pos.h"

namespace std
{

template <> struct hash<Pos>
{
    std::size_t operator()(const Pos& k) const
    {
        const size_t hash =
            static_cast<size_t>(k.x) << sizeof(int) | static_cast<size_t>(k.y);

        return hash;
    }
};

} // namespace std

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

int computeDistance(const Pos& start,
                    const Pos& end,
                    const Pos& cur_pos,
                    const Pos& next,
                    Pos::Coord prefCoord)
{
    assert(cur_pos != next);
    int dist{2};

    const Pos direction = next - cur_pos;

    // Make lines follow the preferred direction, unless they are in the middle
    // area, where we want the line to break
    // middle region
    const int third_dist = std::abs(end[prefCoord] - start[prefCoord]) / 3;
    const int middle_point = (end[prefCoord] + start[prefCoord]) / 2;

    const bool in_middle_range =
        cur_pos[prefCoord] < (middle_point + third_dist) and
        cur_pos[prefCoord] > (middle_point - third_dist);

    // Hard to follow logic below
    const bool vertical = static_cast<size_t>(prefCoord) & 1;
    const bool preferred_path_choser =
        (!vertical and in_middle_range) or (vertical and !in_middle_range);

    //                            in_middle_range
    //                               True    False
    //                              _______________
    //        vertical    true     | false | true
    //                    false    | true  | false

    if ((!preferred_path_choser and
         (direction == directions[LEFT] or direction == directions[RIGHT])) or
        (preferred_path_choser and
         (direction == directions[UP] or direction == directions[DOWN])))
    {
        dist -= 1;
    }

    return dist;
}

struct GraphNode
{
    GraphNode(const Pos& pos_) : pos(pos_) {}

    GraphNode* parent{};

    GraphNode& operator=(const GraphNode&) = default;

    Pos pos{};
    bool visited{};
    int cur_minimum_distance = std::numeric_limits<int>::max();

    bool operator<(const GraphNode& rhs) const
    {
        return cur_minimum_distance < rhs.cur_minimum_distance;
    }

    bool operator>(const GraphNode& rhs) const
    {
        return cur_minimum_distance > rhs.cur_minimum_distance;
    }
};

std::vector<Pos> findPath(const Pos& start,
                          const Pos& end,
                          Pos::Coord preferredCoord,
                          const Buffer& buffer)
{
    std::unordered_map<Pos, GraphNode> graph;

    std::vector<GraphNode*> heap;

    // Insert all nodes here
    for (int y = 0; y < static_cast<int>(buffer.size()); ++y)
    {
        for (int x = 0; x < static_cast<int>(buffer[0].size()); ++x)
        {
            auto iterPair =
                graph.insert(std::make_pair(Pos(x, y), GraphNode({x, y})));

            heap.push_back(std::addressof(iterPair.first->second));
        }
    }
    graph.find(start)->second.cur_minimum_distance = 0;

    auto compNodes = [](const GraphNode* lhs, const GraphNode* rhs) {
        return *lhs > *rhs; // > for min prioriry queue
    };

    while (!heap.empty())
    {
        std::make_heap(heap.begin(), heap.end(), compNodes);
        std::pop_heap(heap.begin(), heap.end());
        GraphNode* cur_node = heap.back();
        heap.pop_back();

        assert(!cur_node->visited);
        cur_node->visited = true;

        if (cur_node->cur_minimum_distance == std::numeric_limits<int>::max())
        {
            // We walked all the reachable nodes and the nodes that were not
            // occupied, not need to proceed
            break;
        }

        const Pos& cur_pos = cur_node->pos;

        for (int i = Direction::UP; i != Direction::END; ++i)
        {
            Direction d = static_cast<Direction>(i);
            const Pos new_pos = cur_pos + directions[d];

            // If you find an empty cell, then consider the branch
            if (buffer.isPosValid(new_pos) &&
                (!buffer.at(new_pos) or buffer.at(new_pos) == '@'))
            {
                GraphNode& adjacient_node = graph.find(new_pos)->second;

                if (adjacient_node.visited)
                    continue;

                const int delta_distance = computeDistance(
                    start, end, cur_pos, new_pos, preferredCoord);

                // Check for overflow
                assert(cur_node->cur_minimum_distance !=
                       std::numeric_limits<int>::max());
                const int cur_dist_to_node =
                    cur_node->cur_minimum_distance + delta_distance;

                if (cur_dist_to_node < adjacient_node.cur_minimum_distance)
                {
                    adjacient_node.parent = cur_node;
                    adjacient_node.cur_minimum_distance = cur_dist_to_node;
                }
            }
        }
    }

    // Build path
    GraphNode* end_node = &graph.find(end)->second;
    assert(end_node->cur_minimum_distance < std::numeric_limits<int>::max());

    GraphNode* start_node = &graph.find(start)->second;

    std::vector<Pos> path;
    for (GraphNode* cur_node = end_node;; cur_node = cur_node->parent)
    {
        const Pos& cur_pos = cur_node->pos;
        path.push_back(cur_pos);
        assert(cur_node == start_node or cur_node->parent);

        if (cur_node == start_node)
        {
            break;
        }
    }

    return path;
}

void drawHorizontalLine(const Pos& start, const Pos& end, Buffer& buffer)
{
    const std::vector<Pos> reverse_path =
        findPath(start, end, Pos::Coord::X, buffer);

    assert(reverse_path.size() > 1);
    Pos last_direction =
        *std::next(reverse_path.crbegin()) - *reverse_path.crbegin();
    for (auto iter = reverse_path.crbegin();
         iter != std::prev(reverse_path.crend());
         ++iter)
    {
        const Pos new_pos = *iter;
        const Pos direction = *std::next(iter) - new_pos;
        const bool directionChanging = direction != last_direction;

        if (direction == directions[UP])
        {
            if (directionChanging)
                buffer.at(new_pos) = '\'';
            else
                buffer.at(new_pos) = '|';
        }
        else if (direction == directions[DOWN])
        {
            if (directionChanging)
                buffer.at(new_pos) = ',';
            else
                buffer.at(new_pos) = '|';
        }
        else if (direction == directions[LEFT] or
                 direction == directions[RIGHT])
        {
            if (directionChanging)
            {
                if (last_direction == directions[UP])
                    buffer.at(new_pos) = ',';
                else if (last_direction == directions[DOWN])
                    buffer.at(new_pos) = '\'';
            }
            else
                buffer.at(new_pos) = '-';
        }

        last_direction = direction;
    }
}

void drawVerticalLine(const Pos& start, const Pos& end, Buffer& buffer)
{
    const std::vector<Pos> reverse_path =
        findPath(start, end, Pos::Coord::Y, buffer);

    assert(reverse_path.size() > 1);
    Pos last_direction =
        *std::next(reverse_path.crbegin()) - *reverse_path.crbegin();
    for (auto iter = reverse_path.crbegin();
         iter != std::prev(reverse_path.crend());
         ++iter)
    {
        const Pos new_pos = *iter;
        const Pos direction = *std::next(iter) - new_pos;
        const bool directionChanging = direction != last_direction;

        if (direction == directions[UP])
        {
            if (directionChanging)
                buffer.at(new_pos) = '\'';
            else
                buffer.at(new_pos) = '|';
        }
        else if (direction == directions[DOWN])
        {
            if (directionChanging)
                buffer.at(new_pos) = ',';
            else
                buffer.at(new_pos) = '|';
        }
        else if (direction == directions[LEFT] or
                 direction == directions[RIGHT])
        {
            if (directionChanging)
            {
                if (last_direction == directions[UP])
                    buffer.at(new_pos) = ',';
                else if (last_direction == directions[DOWN])
                    buffer.at(new_pos) = '\'';
            }
            else
                buffer.at(new_pos) = '-';
        }

        last_direction = direction;
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
