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

class CoordMover
{
public:
    CoordMover(int initial_pos, int destination)
        : _coord(initial_pos), _destination(destination), _end(false)
    {
    }

    void move_closer()
    {
        if (_coord < _destination)
            ++_coord;
        else if (_coord > _destination)
            --_coord;
    }

    operator int() const { return _coord; }

    bool done()
    {
        if (_end)
        {
            return true;
        }

        if (_coord == _destination)
        {
            _end = true;
        }

        return false;
    }

    int value() const { return _coord; }

private:
    int _coord;
    const int _destination;
    bool _end;
};

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
    const int middle_range = std::abs(end[prefCoord] - start[prefCoord]) / 5;
    const int middle_point = (end[prefCoord] + start[prefCoord]) / 2;

    const bool in_middle_range =
        cur_pos[prefCoord] < (middle_point + middle_range) and
        cur_pos[prefCoord] >= (middle_point);

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
                buffer.at(new_pos).type() != Buffer::ElemType::Box)
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

    // Debug
    //    for (CoordMover y(end.y, start.y); !y.done(); y.move_closer())
    //    {
    //        std::cout << y << "\t";
    //        for (CoordMover x(start.x, end.x); !x.done(); x.move_closer())
    //        {
    //            const Pos pos(x, y);
    //            const auto& node = graph.at(pos);

    //            const int dist = node.cur_minimum_distance;
    //            if (dist < 10)
    //            {
    //                std::cout << '0' << dist;
    //            }
    //            else if (dist < 100)
    //            {
    //                std::cout << dist;
    //            }
    //            std::cout << '.';
    //        }
    //        std::cout << "\n";
    //    }
    //    std::cout << "\n";

    return path;
}

void drawLine(const Pos& start,
              const Pos& end,
              Pos::Coord preferredDirection,
              Buffer& buffer)
{
    const std::vector<Pos> reverse_path =
        findPath(start, end, preferredDirection, buffer);

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

        buffer.at(new_pos).type(Buffer::ElemType::Arrow);

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
        drawLine(start, end, Pos::Coord::Y, buf);
    }
    else
    {
        drawLine(start, end, Pos::Coord::X, buf);
    }
    drawArrowBegin(start, relation, buf);
    drawArrowEnd(end, relation, buf);
}

} // namespace drawing
