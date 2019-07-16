#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Buffer.h"
#include <ncurses.h>

struct ClassNode
{
    ClassNode(const std::string& name_) : name(name_) {}
    const std::string name;

    Pos pos;

    Pos getRightAnchorPoint() const
    {
        return {pos.x + getBoxWidth() + 1, pos.y + 1};
    }

    Pos getLeftAnchorPoint() const { return {pos.x - 1, pos.y + 1}; }

    Pos getTopAnchorPoint() const
    {
        return {pos.x + getBoxWidth() / 2, pos.y - 1};
    }

    Pos getBottomAnchorPoint() const
    {
        return {pos.x + getBoxWidth() / 2, pos.y + getBoxHeight()};
    }

    Pos getBottomRightCorner() const
    {
        return {pos.x + getBoxWidth(), pos.y + 2};
    }

    void setBottomAnchorPoint(const Pos& pos_)
    {
        pos.x = pos_.x - getBoxWidth() / 2;
        pos.y = pos_.y - getBoxHeight();
        assert(pos.x >= 0);
        assert(pos.y >= 0);
    }

    void setLeftAnchorPoint(const Pos& pos_)
    {
        pos.x = pos_.x + 1;
        pos.y = pos_.y - getBoxHeight() / 2;
        assert(pos.x >= 0);
        assert(pos.y >= 0);
    }

    Pos getBottomLeftCorner() const { return {pos.x, pos.y + 2}; }

    Pos getTopLeftCorner() const { return pos; }

    Pos getTopRightCorner() const { return {pos.x + getBoxWidth(), pos.y}; }

    std::vector<std::shared_ptr<ClassNode>> ownedMembers;
    std::vector<std::shared_ptr<ClassNode>> aggregates;
    std::vector<std::shared_ptr<ClassNode>> parents;

    void draw(Buffer& buf) const
    {
        // Set buffer type
        for (int j = 0; j > getBoxHeight(); --j)
        {
            for (int i = 0; i <= getBoxWidth(); ++i)
            {
                const Pos cur_pos = pos + Pos(i, j);
                if (buf.isPosValid(cur_pos))
                    buf.at(cur_pos).type(Buffer::ElemType::Box);
            }
        }

        // Draw sides
        if (buf.isPosValid({pos.x, pos.y + 1}))
            buf[pos.y + 1][pos.x] = '|';

        if (buf.isPosValid({pos.x + getBoxWidth(), pos.y + 1}))
            buf[pos.y + 1][pos.x + getBoxWidth()] = '|';

        // Top and bottom
        for (int i = pos.x + 1; i < pos.x + getBoxWidth(); ++i)
        {
            if (buf.isPosValid({i, pos.y}))
                buf[pos.y][i] = '-';
            if (buf.isPosValid({i, pos.y + 2}))
                buf[pos.y + 2][i] = '-';
        }

        // Draw corners
        if (buf.isPosValid(getBottomLeftCorner()))
            buf.at(getBottomLeftCorner()) = '+';
        if (buf.isPosValid(getBottomRightCorner()))
            buf.at(getBottomRightCorner()) = '+';
        if (buf.isPosValid(getTopLeftCorner()))
            buf.at(getTopLeftCorner()) = '+';
        if (buf.isPosValid(getTopRightCorner()))
            buf.at(getTopRightCorner()) = '+';

        // Now write the word
        for (int i = 0; i < static_cast<int>(name.size()); ++i)
        {
            if (buf.isPosValid({pos.x + _padding + i + 1, pos.y + 1}))
                buf[pos.y + 1][pos.x + _padding + i + 1] = name[i];
        }
    }

    void eraseSelf(Buffer& buffer_)
    {
        for (int j = 0; j > -getBoxHeight(); --j)
        {
            for (int i = 0; i <= getBoxWidth(); ++i)
            {
                const Pos cur_pos = pos + Pos(i, j);
                if (buffer_.isPosValid(cur_pos))
                    buffer_.at(cur_pos) = Buffer::BufferElem();
            }
        }
    }

    int getBoxWidth() const { return name.size() + 2 * _padding + 1; }

    int getBoxHeight() const { return 3; }

    bool isHit(const Pos& pos_) const
    {
        const bool isHit = pos_.x >= pos.x and
                           pos_.x < pos.x + getBoxWidth() and
                           pos_.y >= pos.y and pos_.y < pos.y + getBoxHeight();
        return isHit;
    }

private:
    const int _padding = 1;
};
