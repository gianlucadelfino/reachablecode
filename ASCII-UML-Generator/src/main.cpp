#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "Drawing.H"

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
        Pos drawArrow(const ClassNode& originNode, Buffer& buf)
        {
            const Pos start = [this, &originNode]() {
                if (relation == Relation::Inheritance)
                {
                    return originNode.getTopAnchorPoint();
                }
                else
                {
                    return originNode.getRightAnchorPoint();
                }
            }();

            const Pos end = [this]() -> Pos {
                if (relation == Relation::Inheritance)
                {
                    return this->node->getBottomAnchorPoint();
                }
                else
                {
                    return this->node->getLeftAnchorPoint();
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
        return {pos.x + getBoxWidth() + 1, pos.y + 1};
    }

    Pos getLeftAnchorPoint() const { return {pos.x, pos.y + 1}; }

    Pos getTopAnchorPoint() const
    {
        return {pos.x + getBoxWidth() / 2, pos.y - 1};
    }

    Pos getBottomAnchorPoint() const
    {
        return {pos.x + getBoxWidth() / 2, pos.y + 3};
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

    size_t getBoxWidth() const { return name.size() + 2 * padding + 1; }
private:
    const size_t padding = 1;
};

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
            child.node->pos = [&cur_node, &child]() -> Pos {
                const size_t arrowLength = 10;
                const Pos& start = cur_node->pos;

                // Parent classes go up, members go right
                if (child.relation == Relation::Inheritance)
                {
                    assert(start.y > arrowLength);
                    return {start.x, start.y - arrowLength};
                }
                else
                {
                    return {start.x + cur_node->getBoxWidth() + arrowLength,
                            start.y};
                }
            }();

            child.drawArrow(*cur_node, buffer);
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
