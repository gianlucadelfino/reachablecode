#pragma once

class Buffer
{
private:
    class BufferLine
    {
    public:
        using LineType = std::array<char, 80>;

        LineType::const_iterator cbegin() const { return _line.cbegin(); }
        LineType::iterator begin() { return _line.begin(); }

        LineType::const_iterator begin() const { return _line.begin(); }
        LineType::const_iterator end() const { return _line.end(); }

        LineType::const_iterator cend() const { return _line.cend(); }
        LineType::iterator end() { return _line.end(); }
        char& operator[](size_t i)
        {
            assert(i < _line.size());
            return _line[i];
        }

        char operator[](size_t i) const
        {
            assert(i < _line.size());
            return _line[i];
        }

        size_t size() const { return _line.size(); }

    private:
        LineType _line{};
    };
    using BufferType = std::array<BufferLine, 40>;
    BufferType _buffer{};

public:
    BufferLine& operator[](size_t i)
    {
        assert(i < _buffer.size());
        return _buffer.at(i);
    }

    const BufferLine& operator[](size_t i) const
    {
        assert(i < _buffer.size());
        return _buffer.at(i);
    }

    BufferType::const_iterator cbegin() const { return _buffer.cbegin(); }
    BufferType::iterator begin() { return _buffer.begin(); }

    BufferType::const_iterator begin() const { return _buffer.begin(); }
    BufferType::const_iterator end() const { return _buffer.end(); }

    BufferType::const_iterator cend() const { return _buffer.cend(); }
    BufferType::iterator end() { return _buffer.end(); }

    size_t size() const { return _buffer.size(); }
};
