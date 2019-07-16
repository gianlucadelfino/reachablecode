#include <iostream>
#include <array>
#include <utility>
#include <cassert>
//#include <unordered_map>

using Buffer = std::array<std::array<char, 80>, 20>;

struct Pos
{
    size_t x{};
    size_t y{};
};

struct Box
{
    Pos leftTopCorner;
    Pos rightBottomCorner;
};

void render(const Buffer& buf, std::ostream& out)
{
   for(size_t i = 0; i < buf.size(); ++i)
   {
       const auto& line = buf[i];
       out << i << "\t";
       for (auto c : line)
       {
           if (c)
            out << c;
           else
            out << '.';
       }
       out << "\n";
   }
}

void move_closer(size_t& i, size_t destination)
{
    if (i < destination)
        ++i;
    else if (i > destination)
        --i;

    // If equal do nothing
}

void drawLine(const Pos& start, const Pos& end, Buffer& buf)
{
    // Draw a straigth line that breaks in the middle point
    //    --------------,
    //                  |
    //                  |
    //                  '---------->

    // Draw x
    const size_t midpoint = (start.x + end.x) /2;
    for (size_t i = start.x; i != midpoint; move_closer(i, midpoint))
    {
        // first half
        std::cout << "i " << i << std::endl;
        buf[start.y][i] = '-';
//        const size_t oneIfOdd = (start.x - end.x) & 1;
//        if (start.x < end.x)
//        {
//            buf[end.y][end.x - i + oneIfOdd] = '-';
//        }
//        else if (start.x > end.x)
//        {
//            buf[end.y][i - end.x - oneIfOdd] = '-';
//        }
    }

    // Second half
    for (size_t i = end.x; i != midpoint; move_closer(i, midpoint))
    {
        // first half
        std::cout << "i " << i << std::endl;
        buf[end.y][i] = '-';
    }

    // Draw the corners and the vertical line
    if (end.y < start.y)
    {
        // It goes up, so we start with a '
        buf[start.y][midpoint] = '\'';
        buf[end.y][midpoint] = ',';
    }
    else
    {
        // It goes down, so we add a comma
        buf[start.y][midpoint] = ',';
        buf[end.y][midpoint] = '\'';
    }

    //
    size_t j = start.y;
    move_closer(j, end.y); // Move one step already

    while(j != end.y)
    {
        // Not very cache friendly, but who cares
        std::cout << "j " << j << std::endl;
        buf[j][midpoint] = '|';

        move_closer(j, end.y);
    }
}

void drawClass(const std::string& className, const Pos& pos, Buffer& buf)
{
    // Drawing the box first
    const size_t padding = 1;

    // Draw sides
    buf[pos.y + 1][pos.x] = '|';
    buf[pos.y + 1][pos.x + className.size() + 2* padding + 1] = '|';

    // Top and bottom
    for (size_t i = pos.x + 1; i < pos.x + className.size() + 2*padding + 1; ++i)
    {
        buf[pos.y][i] = '_';
        buf[pos.y + 2][i] = '-';
    }

    // Now write the word
    for (size_t i = 0; i < className.size(); ++i)
    {        
        buf[pos.y + 1][ pos.x + padding + i + 1] = className[i];
    }
}

int main()
{
    std::ostream& out = std::cout;

    //     _________           ______________
    //    | MyClass |<>-------| MyOtherClass |
    //     ---------           --------------

    Buffer buffer{};

    const std::string className = "MyClass";
    drawClass(className, {10, 10}, buffer);

    //drawLine({3,3}, {10,15}, buffer);
    //drawLine({10,15}, {3,3}, buffer);
    drawLine({3,15}, {15,3}, buffer);
    render(buffer, out);

    return 0;
}
