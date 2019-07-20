#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Buffer.h"

struct Pos
{
    size_t x{};
    size_t y{};
};

struct ClassNode
{
    ClassNode(const std::string& name_) : name(name_) {}
    const std::string name;

    Pos pos;

    Pos getRightAnchorPoint() const
    {
        return {pos.x + getBoxWidth() + 1, pos.y - 1};
    }

    Pos getLeftAnchorPoint() const { return {pos.x, pos.y - 1}; }

    Pos getTopAnchorPoint() const
    {
        return {pos.x + getBoxWidth() / 2, pos.y - 1};
    }

    Pos getBottomAnchorPoint() const
    {
        return {pos.x + getBoxWidth() / 2, pos.y - 3};
    }

    Pos getBottomRightCorner() const
    {
        return {pos.x + getBoxWidth(), pos.y - 2};
    }

    std::vector<std::unique_ptr<ClassNode>> ownedMembers;
    std::vector<std::unique_ptr<ClassNode>> aggregates;
    std::vector<std::unique_ptr<ClassNode>> parents;

    void draw(Buffer& buf) const
    {
        // Draw sides
        buf[pos.y - 1][pos.x] = '|';
        buf[pos.y - 1][pos.x + getBoxWidth()] = '|';

        // Top and bottom
        for (size_t i = pos.x + 1; i < pos.x + getBoxWidth(); ++i)
        {
            buf[pos.y][i] = '_';
            buf[pos.y - 2][i] = '-';
        }

        // Now write the word
        for (size_t i = 0; i < name.size(); ++i)
        {
            buf[pos.y - 1][pos.x + padding + i + 1] = name[i];
        }
    }

    size_t getBoxWidth() const { return name.size() + 2 * padding + 1; }

private:
    const size_t padding = 1;
};
