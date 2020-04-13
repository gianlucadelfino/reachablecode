#pragma once

#include <algorithm>
#include <numeric>

namespace math
{
template <typename NumberIter, typename Pred>
float average(
    const NumberIter beg,
    const NumberIter end,
    Pred pred = [](float sum, auto& item) { return sum + item; })
{
    const float sum = std::accumulate(beg, end, 0.f, pred);
    const float size = std::distance(beg, end);
    if (!size)
    {
        return 0;
    }

    return sum / size;
}
} // namespace math
