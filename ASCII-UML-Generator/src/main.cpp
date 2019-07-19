#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "Drawing.H"

//struct DiagramNode
//{
//    ClassNode classNode;
//    Pos pos;
//    std::vector<std::unique_ptr<DiagramNode>> diagramNode;
//};

struct ClassNode
{
    ClassNode(const std::string& name_) : name(name_) {}
    const std::string name;

    struct Link
    {
        Link(const std::string& name_, Relation relation_)
            : node(std::make_unique<ClassNode>(name_)), relation(relation_)
        {
        }
        Pos drawArrow(const Pos& start, Buffer& buf)
        {
            const size_t arrowLength = 15;
            // Parent classes go up, members go right
            const Pos end = [this, &start]() -> Pos {
                if (relation == Relation::Inheritance)
                {
                    assert(start.y > arrowLength);
                    return {start.x, start.y - arrowLength};
                }
                else
                {
                    return {start.x + arrowLength, start.y};
                }
            }();

            drawLine(start, end, buf);
            drawArrowBegin(start, relation, buf);
            drawArrowEnd(end, relation, buf);
            return end;
        }

        std::unique_ptr<ClassNode> node{};
        Relation relation{Relation::Unset};
    };

    Pos pos;

    Pos getRightAnchorPoint() const
    {
        return {pos.x + getBoxWidth(), pos.y + 1};
    }

    Pos getBottomAnchorPoint() const
    {
        return {pos.x + getBoxWidth() / 2, pos.y + 2};
    }

    Pos getBottomRightCorner() const
    {
        return {pos.x + getBoxWidth(), pos.y + 2};
    }

    std::vector<Link> children;

    void draw(Buffer& buf) const
    {
        // Draw sides
        buf[pos.y + 1][pos.x] = '|';
        buf[pos.y + 1][pos.x + getBoxWidth()] = '|';

        // Top and bottom
        for (size_t i = pos.x + 1; i < pos.x + getBoxWidth(); ++i)
        {
            buf[pos.y][i] = '_';
            buf[pos.y + 2][i] = '-';
        }

        // Now write the word
        for (size_t i = 0; i < name.size(); ++i)
        {
            buf[pos.y + 1][pos.x + padding + i + 1] = name[i];
        }
    }

private:
    size_t getBoxWidth() const { return name.size() + 2 * padding + 1; }
    const size_t padding = 1;
};

// Pos drawArrow(const Pos& start, Relation rel, Buffer& buf)
//{
//    const size_t arrowLength = 15;
//    // Parent classes go up, members go right
//    const Pos end = [rel, &start]() -> Pos {
//        if (rel == Relation::Inheritance)
//        {
//            assert(start.y > arrowLength);
//            return {start.x, start.y - arrowLength};
//        }
//        else
//        {
//            return {start.x + arrowLength, start.y};
//        }
//    }();

//    drawLine(start, end, buf);
//    drawArrowBegin(start, rel, buf);
//    drawArrowEnd(end, rel, buf);
//    return end;
//}

void drawDiagram(std::unique_ptr<ClassNode> head,
                 const Pos& start,
                 Buffer& buffer)
{
    head->pos = start;
    std::queue<std::unique_ptr<ClassNode>> q;
    q.push(std::move(head));
    while (!q.empty())
    {
        std::unique_ptr<ClassNode> cur_node = std::move(q.front());
        q.pop();
        cur_node->draw(buffer);
        for (auto& child : cur_node->children)
        {
            // Next child position is given by the end of the arrow
            child.node->pos = child.drawArrow(cur_node->pos, buffer);
            q.push(std::move(child.node));
        }
    }
}

int main()
{
    std::ostream& out = std::cout;

    //     _________           ______________
    //    | MyClass |<>-------| MyOtherClass |
    //     ---------           --------------

    Buffer buffer{};

    std::unique_ptr<ClassNode> head = std::make_unique<ClassNode>("MyClass");
    head->children.emplace_back("MyParent", Relation::Inheritance);

    head->children.emplace_back("OtherClass", Relation::Composition);
    drawDiagram(std::move(head), {10, 30}, buffer);

    // drawLine({3,3}, {10,15}, buffer);
    // drawLine({10,15}, {3,3}, buffer);
    // drawLine({3,15}, {15,3}, buffer);
    render(buffer, out);

    return 0;
}
