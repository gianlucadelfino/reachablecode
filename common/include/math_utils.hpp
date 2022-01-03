#pragma once

#include <algorithm>
#include <numeric>
#include <iterator>
#include <vector>
#include <cmath>

namespace math
{
template <typename NumberIter, typename Pred>
auto average(
    const NumberIter beg,
    const NumberIter end,
    Pred pred)
{
    const double sum = static_cast<double>(std::accumulate(beg, end, 0.0, pred));
    const size_t size = std::distance(beg, end);
    if (!size)
    {
        return 0.0;
    }

    return sum / size;
}

template <typename NumberIter>
auto average(
    const NumberIter beg,
    const NumberIter end)
{
    return average(beg, end, [](double sum, auto& item) { return sum + item; });
}

template <typename NumberIter, typename BinSize>
auto bin(
    const NumberIter beg,
    const NumberIter end,
    BinSize bin_size)
{
    using value_type = std::decay_t<typename std::iterator_traits<NumberIter>::value_type>;
    std::vector<std::vector<value_type>> bins;

    if (end == beg)
    {
        return bins;
    }

    const auto min_max = std::minmax_element(beg, end);

    const size_t num_of_bins = static_cast<size_t>((*min_max.second - *min_max.first + bin_size)/bin_size);

    bins.resize(num_of_bins);

    for (auto it = beg; it != end; ++it)
    {
        const size_t dest_bin = static_cast<size_t>((*it - *min_max.first)/bin_size);
        bins[dest_bin].push_back(*it);
    }

    return bins;
}


} // namespace math
