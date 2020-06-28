#pragma once

#include "Logger.h"
#include "Math.h"

#include <memory>
#include <vector>

namespace ocrTextUtils
{

inline bool is_word_valid(const std::string& word)
{
    for (char c : word)
    {
        const int lowered = std::tolower(static_cast<unsigned char>(c));
        if ((lowered > 'z' or lowered < 'a') and lowered != '-')
        {
            return false;
        }
    }
    return true;
}

inline std::vector<std::pair<std::string, float>> getBooks(
    std::unique_ptr<char[]> UNLVText_, std::unique_ptr<int[]> confidences_)
{
    // Parsing the text. Empty lines are ~
    std::stringstream ss(UNLVText_.get());
    const int* confidence = confidences_.get();
    std::string word;
    std::vector<std::pair<std::string, float>> titles;

    if (!UNLVText_.get())
    {
        return titles;
    }

    std::vector<int> word_scores;
    std::string line;

    const int conf_threshold = 20;

    while (confidence && *confidence && *confidence != -1)
    {
        ss >> word;
        assert(*confidence < 100);

        Logger::Debug("word", word, "conf", *confidence);

        if (word != "~" and *confidence > conf_threshold)
        {
            if (is_word_valid(word))
            {
                word_scores.push_back(*confidence);
                line += " " + word;
            }
        }

        if (word == "~" or *(confidence + 1) == -1)
        {
            // New Line
            if (!word_scores.empty())
            {
                const double average =
                    math::average(word_scores.cbegin(), word_scores.cend());
                const auto min_score =
                    std::min_element(word_scores.cbegin(), word_scores.cend());

                Logger::Debug(
                    "line", line, "average:", average, "min:", *min_score);
                if (*min_score > conf_threshold and average > 60)
                {
                    titles.emplace_back(
                        std::make_pair(std::move(line), average));
                }
            }
            line.clear();
            word_scores.clear();
        }

        ++confidence;
    }

    return titles;
}

} // namespace ocrTextUtils
