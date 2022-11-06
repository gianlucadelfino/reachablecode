#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <vector>

namespace math
{
template <typename NumberIter, typename Pred>
auto average(const NumberIter beg, const NumberIter end, Pred pred)
{
  const double sum = std::accumulate(beg, end, 0.0, pred);
  const double size = std::distance(beg, end);
  if (!size)
  {
    return 0.0;
  }

  return sum / size;
}

template <typename NumberIter>
auto average(const NumberIter beg, const NumberIter end)
{
  return average(beg, end, [](float sum, auto& item) { return sum + item; });
}

template <typename NumberIter, typename BinSize>
auto bin(const NumberIter beg, const NumberIter end, BinSize bin_size)
{
  using value_type = std::decay_t<typename std::iterator_traits<NumberIter>::value_type>;
  std::vector<std::vector<value_type>> bins;

  if (end == beg)
  {
    return bins;
  }

  const auto min_max = std::minmax_element(beg, end);

  const size_t num_of_bins = (*min_max.second - *min_max.first + bin_size) / bin_size;

  bins.resize(num_of_bins);

  for (auto it = beg; it != end; ++it)
  {
    const size_t dest_bin = (*it - *min_max.first) / bin_size;
    bins[dest_bin].push_back(*it);
  }

  return bins;
}

template <typename NumberIter>
double standard_deviation(const NumberIter beg, const NumberIter end, const double average)
{
  const double sum = std::accumulate(beg,
                                     end,
                                     0.0,
                                     [average](double sum, const auto& item)
                                     { return sum + (item - average) * (item - average); });
  const double size = std::distance(beg, end);

  return std::sqrt(sum / size);
}

template <typename NumberIter>
double percentile(NumberIter beg, NumberIter end, const double percent)
{
  const double size = std::distance(beg, end);
  const auto nth = beg + (percent * size) / 100.0;
  std::nth_element(beg, nth, end);
  return *nth;
}

} // namespace math
