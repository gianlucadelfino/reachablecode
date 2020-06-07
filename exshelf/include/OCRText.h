#pragma once

#include "Math.h"
#include <memory>
#include <vector>

template <typename T> struct ArrayDeleter
{
    // Not explicit!
    ArrayDeleter(T* t) : _data(t) {}

    ~ArrayDeleter() { delete[] _data; }

    T* get() const { return _data; }

private:
    T* const _data;
};

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
    ArrayDeleter<char> UNLVText_, ArrayDeleter<int> confidences_)
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
    std::cout << "NEW TExt\n";

    while (true)
    {
        ss >> word;
        std::cout << "word " << word << " conf " << *confidence << std::endl;

        if (word.find("veraa") != std::string::npos)
        {
            std::cout << "vera" << std::endl;
        }

        if (*confidence > 100)
        {
            assert(false);
        }
        if (word != "~" and *confidence > 80)
        {
            if (is_word_valid(word))
            {
                word_scores.push_back(*confidence);
                line += " " + word;
            }
        }

        if (word == "~")
        {
            // New Line
            if (!word_scores.empty())
            {
                const double average =
                    math::average(word_scores.cbegin(), word_scores.cend());
                const auto min_score =
                    std::min_element(word_scores.cbegin(), word_scores.cend());
                if (*min_score > 80 and average > 90)
                {
                    titles.emplace_back(
                        std::make_pair(std::move(line), average));
                }
            }
            line.clear();
            word_scores.clear();
        }

        if (*confidence == -1)
        {
            break;
        }
        ++confidence;
    }

    return titles;
}

} // namespace ocrTextUtils
