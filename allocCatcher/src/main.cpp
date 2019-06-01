#include <iostream>
#include <vector>

void* operator new(size_t s)
{
    void* ptr = malloc(s);
    std::cout << "Allocated " << s << " bytes.\n";

    return ptr;
}

// These are just to make the compiler and sanitizer happy
void operator delete(void* ptr_) { free(ptr_); }
void operator delete(void* ptr_, size_t) { free(ptr_); }

int main()
{
    std::vector<int> vec;

    // Uncomment the following line to see the difference in output
    // vec.reserve(1'000'000);

    for (int i = 0; i < 1'000'000; ++i)
    {
        vec.push_back(i);
    }

    std::cout << "Set size " << vec.size() << "\n";
    return 0;
}
