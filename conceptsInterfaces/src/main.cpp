template <typename T> concept Shape = requires(const T& t)
{
    {
        t.area()
    }
    ->float;
};

template <class T> struct ShapeBase
{
    ShapeBase() requires(Shape<T>) = default;
};

template <typename T> struct Circle : ShapeBase<Circle<T>>
{
    float area() const;
    T radius;
};

template <class T> struct Square
{
    Square() requires(Shape<Circle<T>>) = default;
    float area() const;
    int edge;
};

#include <type_traits>
static_assert(std::is_trivially_default_constructible_v<Circle<int>>);
static_assert(std::is_trivially_default_constructible_v<Square<int>>);

int main()
{
    [[maybe_unused]] Square<int> s;
    [[maybe_unused]] Circle<float> c;

    return 0;
}
