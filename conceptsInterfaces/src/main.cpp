template <typename T>
concept Shape = requires(const T& t)
{
    { t.area() } ->float;
};

struct Rectangle
{
    Rectangle()
    {
        static_assert(Shape<decltype(*this)>);
    }
    float area() const;
};

template <typename T>
struct Square
{
    Square()
    {
        static_assert(Shape<decltype(*this)>);
    }
    float area() const;
    T edge;
};

template <typename T>
struct ShapeBase
{
    ShapeBase() { static_assert(Shape<T>); }
};

template <typename T>
struct Circle : ShapeBase<Circle<T>>
{
    float area() const;
    T radius;
};

int main()
{
    Rectangle r;
    Square<int> s;
    Circle<float> c;
    return 0;
}
