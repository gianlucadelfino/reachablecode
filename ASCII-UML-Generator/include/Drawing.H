#pragma once

inline namespace drawing
{

enum class Relation
{
    Composition,
    Aggregation,
    Inheritance,
    Unset
};

using Buffer = std::array<std::array<char, 80>, 40>;

struct Pos
{
    size_t x{};
    size_t y{};
};

// struct BufferArea
//{
//    const Pos& leftTopCorner{};
//    const Pos& rightBottomCorner{};
//};

struct Box
{
    const size_t width{};
    const size_t height{};
};

void render(const Buffer& buf, std::ostream& out)
{
    for (size_t i = 0; i < buf.size(); ++i)
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
    //                  '----------

    // Draw x
    const size_t midpoint = (start.x + end.x) / 2;
    for (size_t i = start.x; i != midpoint; move_closer(i, midpoint))
    {
        // first half
        buf[start.y][i] = '-';
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
    else if (end.y > start.y)
    {
        // It goes down, so we add a comma
        buf[start.y][midpoint] = ',';
        buf[end.y][midpoint] = '\'';
    }
    else
    {
        // They are on the same level, just put a - in the middle
        buf[start.y][midpoint] = '-';
    }

    // Draw vertical line
    size_t j = start.y;
    // Move one step already as we have ' or , already
    move_closer(j, end.y);
    while (j != end.y)
    {
        // Not very cache friendly, but who cares
        buf[j][midpoint] = '|';

        move_closer(j, end.y);
    }
}

void drawClass_old(const std::string& className, const Pos& pos, Buffer& buf)
{
    // Drawing the box first
    const size_t padding = 1;

    // Draw sides
    buf[pos.y + 1][pos.x] = '|';
    buf[pos.y + 1][pos.x + className.size() + 2 * padding + 1] = '|';

    // Top and bottom
    for (size_t i = pos.x + 1; i < pos.x + className.size() + 2 * padding + 1;
         ++i)
    {
        buf[pos.y][i] = '_';
        buf[pos.y + 2][i] = '-';
    }

    // Now write the word
    for (size_t i = 0; i < className.size(); ++i)
    {
        buf[pos.y + 1][pos.x + padding + i + 1] = className[i];
    }
}


//void drawClass(const ClassNode& classNode, Buffer& buf)
//{
//    // Draw sides
//    buf[classNode.pos.y + 1][classNode.pos.x] = '|';
//    buf[classNode.pos.y + 1][pos.x + classNode.getBoxWidth()] = '|';

//    // Top and bottom
//    for (size_t i = pos.x + 1; i < pos.x + className.size() + 2 * padding + 1;
//         ++i)
//    {
//        buf[pos.y][i] = '_';
//        buf[pos.y + 2][i] = '-';
//    }

//    // Now write the word
//    for (size_t i = 0; i < className.size(); ++i)
//    {
//        buf[pos.y + 1][pos.x + padding + i + 1] = className[i];
//    }
//}

void drawArrowBegin(const Pos& pos, Relation r, Buffer& buffer)
{
    switch (r)
    {
    case Relation::Composition:
        // Fallthrough .. TODO: fix Composition
    case Relation::Aggregation:
        buffer[pos.y][pos.x] = '<';

        // Check it fits
        assert(pos.x + 1 < buffer[0].size());
        buffer[pos.y][pos.x + 1] = '>';
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

} // namespace drawing
