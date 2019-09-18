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
#include "TimeLogger.h"

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
const std::vector<Pos> directions{{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

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

// TODO: Refactor to use GraphNode instead of ClassNode
struct GraphNode
{
    GraphNode(const Pos& pos_) : pos(pos_) {}

    GraphNode* parent{};

    GraphNode& operator=(const GraphNode&) = default;

    void resetNode()
    {
        parent = nullptr;
        visited = false;
        cur_minimum_distance = std::numeric_limits<int>::max();
    }

    void swap(GraphNode& other_) noexcept
    {
        std::swap(parent, other_.parent);
        std::swap(pos, other_.pos);
        std::swap(visited, other_.visited);
        std::swap(cur_minimum_distance, other_.cur_minimum_distance);
    }

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

    std::vector<GraphNode*> heap;
    thread_local std::unordered_map<Pos, GraphNode> graph = [&heap, &buffer]() {
        std::unordered_map<Pos, GraphNode> graph;
        // Insert all nodes here
        for (int y = 0; y < static_cast<int>(buffer.size()); ++y)
        {
            for (int x = 0; x < static_cast<int>(buffer[0].size()); ++x)
            {
                graph.insert(std::make_pair(Pos(x, y), GraphNode({x, y})));
            }
        }
        return graph;
    }();

    // reinitialise the graph nodes
    for (auto&& posGraphNode : graph)
    {
        posGraphNode.second.resetNode();
        heap.push_back(std::addressof(posGraphNode.second));
    }

    graph.find(start)->second.cur_minimum_distance = 0;

    //        auto compNodes = [](const GraphNode* lhs, const GraphNode* rhs) {
    //            return *lhs > *rhs; // > for min prioriry queue
    //        };

    {
//        TimeLogger t("whileHeap", std::cout);

        int cur_min_end_distance = std::numeric_limits<int>::max();
        while (!heap.empty())
        {
            //            std::make_heap(heap.begin(), heap.end(), compNodes);
            //            std::pop_heap(heap.begin(), heap.end());
            auto min_node_iter = std::min_element(
                heap.begin(),
                heap.end(),
                [](const GraphNode* const lhs_, const GraphNode* const rhs_) {
                    return lhs_->cur_minimum_distance <
                           rhs_->cur_minimum_distance;
                });
            std::swap(*min_node_iter, *heap.rbegin());
            GraphNode* cur_node = heap.back();
            heap.pop_back();

            assert(!cur_node->visited);

            cur_node->visited = true;

            if (cur_node->cur_minimum_distance ==
                std::numeric_limits<int>::max())
            {
                // We walked all the reachable nodes and the nodes that were not
                // occupied, not need to proceed
                break;
            }

            if (cur_node->cur_minimum_distance >= cur_min_end_distance)
            {
                // No need to go down this path as it's already longer than
                // some other path to the end
                continue;
            }

            const Pos& cur_pos = cur_node->pos;

            //         Buffer& hackedBuf = const_cast<Buffer&>(buffer);
            //         hackedBuf.at(cur_pos) = '@';

            for (int i = Direction::UP; i != Direction::END; ++i)
            {
                const Direction d = static_cast<Direction>(i);
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
    }

    // DEBUG
    //     Buffer& hackedBuf = const_cast<Buffer&>(buffer);
    //        hackedBuf.at(end) = 'X';
    //       render(buffer);
    //       refresh();
    //       return {{1,2},{1,3}};

    // Build path
    GraphNode* end_node = &graph.find(end)->second;
    assert(end_node->cur_minimum_distance < std::numeric_limits<int>::max());

    GraphNode* start_node = &graph.find(start)->second;

    // Debug
    //    std::ofstream debugFile("deleteme");
    //        for (CoordMover y(end.y, start.y); !y.done(); y.move_closer())
    //        {
    //            debugFile << y << "\t";
    //            for (CoordMover x(start.x, end.x); !x.done(); x.move_closer())
    //            {
    //                const Pos pos(x, y);
    //                const auto& node = graph.at(pos);

    //                const int dist = node.cur_minimum_distance;
    //                if (dist < 10)
    //                {
    //                    debugFile << '0' << dist;
    //                }
    //                else if (dist < 100)
    //                {
    //                    debugFile << dist;
    //                }
    //                debugFile << '.';
    //            }
    //            debugFile << "\n";
    //        }
    //        debugFile << "\n";

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
            {
                buffer.at(new_pos) = '-';
            }
        }

        last_direction = direction;
    }

    // Set the last one in the direction of the previous one
    buffer.at(*reverse_path.begin()) =
        last_direction == directions[UP] or last_direction == directions[DOWN] ?
            '|' :
            '-';
}

void drawArrowBegin(const Pos& pos, Relation r, Buffer& buffer)
{
    switch (r)
    {
    case Relation::Aggregation:
        // Fallthrough .. TODO: fix
        // const std::string empty_diamond = "\u22C4";
        break;
    case Relation::Composition:
    {
        // The unicode version looks great, but does not work well as it "eats"
        // two extra characters from the outgoing line..
        // const std::string diamond = "\u25C6"; // ◆
        // Consider alternatives:
        //    <#>-----------
        //    <*>-----------
        //    <>-----------
        //    <@>-----------
        //    <_>-----------
        //    <.>-----------
        //    < >-----------
        const std::string diamond = "<>";

        for (size_t i = 0; i < diamond.size(); ++i)
        {
            buffer[pos.y][pos.x + i] = diamond[i];
        }
    }
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
