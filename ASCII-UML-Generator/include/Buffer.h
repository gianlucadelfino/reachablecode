#pragma once

#include <array>
#include <cassert>

#include "Pos.h"

class Buffer
{
private:
    class BufferLine
    {
    public:
        using LineType = std::array<char, 100>;

        LineType::const_iterator cbegin() const { return _line.cbegin(); }
        LineType::iterator begin() { return _line.begin(); }

        LineType::const_iterator begin() const { return _line.begin(); }
        LineType::const_iterator end() const { return _line.end(); }

        LineType::const_iterator cend() const { return _line.cend(); }
        LineType::iterator end() { return _line.end(); }
        char& operator[](int i)
        {
            assert(i >= 0);
            assert(i < static_cast<int>(_line.size()));
            return _line.at(i);
        }

        char operator[](int i) const
        {
            assert(i >= 0);
            assert(i < static_cast<int>(_line.size()));
            return _line.at(i);
        }

        size_t size() const { return _line.size(); }

    private:
        LineType _line{};
    };
    using BufferType = std::array<BufferLine, 40>;
    BufferType _buffer{};

public:
    BufferLine& operator[](int i)
    {
        assert(i >= 0);
        assert(i < static_cast<int>(_buffer.size()));
        return _buffer.at(i);
    }

    const BufferLine& operator[](int i) const
    {
        assert(i >= 0);
        assert(i < static_cast<int>(_buffer.size()));
        return _buffer.at(i);
    }

    char& at(const Pos& pos) { return this->operator[](pos.y)[pos.x]; }
    char at(const Pos& pos) const { return this->operator[](pos.y)[pos.x]; }

    bool isPosValid(const Pos& pos) const
    {
        return pos.x >= 0 &&
               static_cast<size_t>(pos.x) < _buffer.at(0).size() &&
               pos.y >= 0 && static_cast<size_t>(pos.y) < _buffer.size();
    }

    BufferType::const_iterator cbegin() const { return _buffer.cbegin(); }
    BufferType::iterator begin() { return _buffer.begin(); }

    BufferType::const_iterator begin() const { return _buffer.begin(); }
    BufferType::const_iterator end() const { return _buffer.end(); }

    BufferType::const_iterator cend() const { return _buffer.cend(); }
    BufferType::iterator end() { return _buffer.end(); }

    BufferType::const_reverse_iterator crbegin() const { return _buffer.crbegin(); }
    BufferType::reverse_iterator rbegin() { return _buffer.rbegin(); }

    BufferType::const_reverse_iterator rbegin() const { return _buffer.rbegin(); }
    BufferType::const_reverse_iterator rend() const { return _buffer.rend(); }

    BufferType::const_reverse_iterator crend() const { return _buffer.crend(); }
    BufferType::reverse_iterator rend() { return _buffer.rend(); }


    size_t size() const { return _buffer.size(); }
};
