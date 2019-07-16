#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "Pos.h"

class Buffer
{
public:
    enum class ElemType : uint8_t
    {
        Undefined = 0,
        Box,
        Arrow,
    };
    class BufferElem
    {
    public:
        BufferElem() : BufferElem(0) {}
        explicit BufferElem(char c) : _elem(c), _elemType(ElemType::Undefined)
        {
        }

        BufferElem(const BufferElem& other_) : _elem(other_._elem), _elemType(other_._elemType) {}
        void operator=(const BufferElem& other_) { _elem = other_._elem; _elemType = other_._elemType; }

        void operator=(char c) { _elem = c; }
        operator char&() { return _elem; }
        operator char() const { return _elem; }
        ElemType type() const { return _elemType; }
        void type(ElemType type_) { _elemType = type_; }

    private:
        char _elem;
        ElemType _elemType;
    };

private:
    class BufferLine
    {

    public:
        using LineType = std::array<BufferElem, 128>;

        LineType::const_iterator cbegin() const { return _line.cbegin(); }
        LineType::iterator begin() { return _line.begin(); }

        LineType::const_iterator begin() const { return _line.begin(); }
        LineType::const_iterator end() const { return _line.end(); }

        LineType::const_iterator cend() const { return _line.cend(); }
        LineType::iterator end() { return _line.end(); }
        BufferElem& operator[](int i)
        {
            assert(i >= 0);
            assert(i < static_cast<int>(_line.size()));
            return _line.at(i);
        }

        BufferElem operator[](int i) const
        {
            assert(i >= 0);
            assert(i < static_cast<int>(_line.size()));
            return _line.at(i);
        }

         void clear() { for(auto&& item : _line) { item = BufferElem(); } }

        size_t size() const { return _line.size(); }

    private:
        LineType _line{};
    };
    using BufferType = std::array<BufferLine, 40>;
    BufferType _buffer{};

public:
    Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

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

    BufferElem& at(const Pos& pos)
    {
        assert(isPosValid(pos));
        return this->operator[](pos.y)[pos.x];
    }

    BufferElem at(const Pos& pos) const
    {
        assert(isPosValid(pos));
        return this->operator[](pos.y)[pos.x];
    }

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

    BufferType::const_reverse_iterator crbegin() const
    {
        return _buffer.crbegin();
    }
    BufferType::reverse_iterator rbegin() { return _buffer.rbegin(); }

    BufferType::const_reverse_iterator rbegin() const
    {
        return _buffer.rbegin();
    }
    BufferType::const_reverse_iterator rend() const { return _buffer.rend(); }

    BufferType::const_reverse_iterator crend() const { return _buffer.crend(); }
    BufferType::reverse_iterator rend() { return _buffer.rend(); }

    size_t size() const { return _buffer.size(); }

    void clear()
    {
        for (size_t y = 0; y != _buffer.size(); ++y)
            _buffer[y].clear();
    }
};

