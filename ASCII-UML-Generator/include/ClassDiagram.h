#pragma once

#include <list>
#include <map>
#include <thread>
#include <future>

#include "Buffer.h"
#include "ClassNode.h"
#include "Drawing.h"
#include "Pos.h"
#include "TimeLogger.h"

class ClassDiagram
{
public:
    explicit ClassDiagram(Buffer& buffer_)
    {
        // TODO: This is going to be generated
        _head = std::make_shared<ClassNode>("MyClass");
        _head->parents.emplace_back(std::make_shared<ClassNode>("MyParent"));
        _head->parents.emplace_back(
            std::make_shared<ClassNode>("MyOtherParent"));

        _head->ownedMembers.emplace_back(
            std::make_shared<ClassNode>("OtherClass"));
        _head->ownedMembers.front()->parents.push_back(
            std::make_shared<ClassNode>("Parent2"));
        _head->ownedMembers.front()->parents.push_back(_head->parents.front());
        _head->ownedMembers.emplace_back(
            std::make_shared<ClassNode>("OtherClass2"));

        // TODO: Head pos can be a parameter
        _head->pos = {10, 30};

        initialDrawPass(_head.get(), buffer_);
    }

    void draw(Buffer& buffer_)
    {
        buffer_.clear();
        drawImpl(buffer_);
    }

    void moveClass(const Pos& start_, const Pos& end_)
    {
        // Find class
        auto classIter = std::find_if(
            _drawedNodes.begin(),
            _drawedNodes.end(),
            [&start_](const std::pair<std::string, ClassNode*>& nameNodePair) {
                return nameNodePair.second->isHit(start_);
            });

        if (classIter == _drawedNodes.end())
        {
            return;
        }

        // update position
        const Pos delta = end_ - start_;
        classIter->second->pos += delta;
    }

private:
    // Return the x of the rightest parent diagram, and the y of the highest
    // parent or member diagram. These are used to draw the next element.
    Pos initialDrawPass(ClassNode* node, Buffer& buffer)
    {
        // We need something to check for cycles and not draw classes twice. So
        // we store the drawned nodes to check if we already drawed them.

        Pos maxXY(node->getTopRightCorner());
        node->draw(buffer);

        // Members go on the right hand side. We start drawing the first one at
        // the bottom right, then depth first into it keeping track of the
        // highest position reached

        if (!node->parents.empty())
        {
            const int leftMostParentX = [node] {
                const int numParents = node->parents.size();
                if (numParents <= 1)
                {
                    return node->pos.x;
                }
                else
                {
                    // Let's assume about 20 char for each parents,
                    return std::max(
                        0, node->getTopAnchorPoint().x - 20 * (numParents - 1));
                }
            }();

            int cur_parent_x = leftMostParentX;

            for (auto& parent : node->parents)
            {
                // If it's already there no need to recurse
                if (_drawedNodes.find(parent->name) == _drawedNodes.end())
                {
                    const int arrow_length_y = 10;
                    // Because why not make it look prettier
                    const int x_padding = 2;
                    cur_parent_x += x_padding;
                    parent->pos =
                        Pos(cur_parent_x,
                            node->getTopAnchorPoint().y - arrow_length_y);

                    // Before recursing we need to check if we already drawed it
                    const Pos parentMaxXY =
                        initialDrawPass(parent.get(), buffer);

                    maxXY.x = std::max(parentMaxXY.x, maxXY.x);
                    maxXY.y = std::min(parentMaxXY.y, maxXY.y);

                    cur_parent_x = parentMaxXY.x;
                }

                drawArrow(node->getTopAnchorPoint(),
                          parent->getBottomAnchorPoint(),
                          Relation::Inheritance,
                          buffer);
            }
        }

        if (!node->ownedMembers.empty())
        {
            // Let's estimate the amount of space we need to leave given the
            // total number of members
            const int lowestMemberY = [node, &buffer] {
                const int numOfMembers =
                    node->aggregates.size() + node->ownedMembers.size();
                if (numOfMembers <= 1)
                {
                    return node->pos.y;
                }
                else
                {
                    // Let's assume each member will need 10 characters total
                    // vertical space
                    const int approxVerticalSpacePerMember = 10;
                    return std::min(static_cast<int>(buffer.size()) -
                                        node->getBoxHeight(),
                                    node->pos.y + approxVerticalSpacePerMember *
                                                      numOfMembers / 2);
                }
            }();

            int cur_member_y = lowestMemberY;
            for (auto& member : node->ownedMembers)
            {
                // If it's already there no need to recurse
                if (_drawedNodes.find(member->name) == _drawedNodes.end())
                {
                    const int arrow_length_x = 45;
                    const int cur_member_x =
                        node->getRightAnchorPoint().x + arrow_length_x;

                    member->setLeftAnchorPoint(
                        {cur_member_x, cur_member_y - member->getBoxHeight()});

                    const Pos memberMaxXY =
                        initialDrawPass(member.get(), buffer);

                    maxXY.x = std::max(memberMaxXY.x, maxXY.x);
                    maxXY.y = std::min(memberMaxXY.y, maxXY.y);

                    cur_member_y = memberMaxXY.y;
                }

                drawArrow(node->getRightAnchorPoint(),
                          member->getLeftAnchorPoint(),
                          Relation::Composition,
                          buffer);
            }
        }
        // TODO: add print aggregates

        // Add to drawed nodes
        _drawedNodes.insert(std::make_pair(node->name, node));

        return maxXY;
    }

    // Return the x of the rightest parent diagram, and the y of the highest
    // parent or member diagram. These are used to draw the next element.
    void drawImpl(Buffer& buffer_)
    {
//        TimeLogger t("drawImpl", std::cout);

        std::list<std::thread> threads;

        for (auto& [name, node] : _drawedNodes)
        {
            threads.emplace_back(
                [&buffer_, this, id = threads.size(), &name, &node]() mutable{
                    node->draw(buffer_);

                    for (auto& parent : node->parents)
                    {
                        drawArrow(node->getTopAnchorPoint(),
                                  parent->getBottomAnchorPoint(),
                                  Relation::Inheritance,
                                  buffer_);
                    }

                    for (auto& member : node->ownedMembers)
                    {
                        drawArrow(node->getRightAnchorPoint(),
                                  member->getLeftAnchorPoint(),
                                  Relation::Composition,
                                  buffer_);
                    }

                    // TODO: add print aggregates
                });
        }

        for (auto&& t : threads)
        {
            t.join();
        }
    }

private:
    std::map<std::string, ClassNode*> _drawedNodes;
    std::shared_ptr<ClassNode> _head;
};
