#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "Logger.h"

struct BookTitle
{
    explicit BookTitle(const std::string& title_, float average_score_)
        : _title(title_),
          _average_score(average_score_),
          _signature([&title_]() {
              std::map<char, int> signature;
              for (char c : title_)
              {
                  signature[c]++;
              }
              return signature;
          }())
    {
    }

    BookTitle(BookTitle&& other_) = default;

    BookTitle& operator=(BookTitle&&) = default;

    BookTitle& operator=(const BookTitle&) = delete;

    std::string getTitle() const { return _title; }

    bool matches(const BookTitle& other_) const
    {
        float score{};
        for (auto charCounter : other_._signature)
        {
            const char c = charCounter.first;
            const auto iter = _signature.find(c);

            if (iter != _signature.cend())
            {
                score += std::min(other_._signature.at(c), iter->second);
                Logger::Debug("Found ",
                              c,
                              "adding",
                              std::min(other_._signature.at(c), iter->second));
            }
        }

        const float final_score =
            2 * score /
            static_cast<float>(_title.size() + other_._title.size());
        Logger::Debug(
            "\tChecking ", other_._title, " vs ", _title, final_score);

        // let's say 70%
        return final_score > .7f;
    }

    float score() const { return _average_score; }

private:
    std::string _title;
    float _average_score;
    std::map<char, int> _signature;
};

class Library
{
public:
    void insert(BookTitle&& new_book_)
    {
        // Check if is there
        // Linear search here. We could do better but let's not bother for now
        for (BookTitle& book : _books)
        {
            if (book.matches(new_book_))
            {
                if (new_book_.score() > book.score())
                {
                    Logger::Debug("\tFound same book, replace",
                                  new_book_.getTitle());
                    book = std::move(new_book_);
                }
                else
                {
                    Logger::Debug("\tFound same book, don't replace",
                                  new_book_.getTitle());
                }
                return;
            }
            else
            {
                Logger::Debug("\tBook didn't match");
            }
        }

        Logger::Debug("\tBook not found in library, just add:",
                      new_book_.getTitle());
        // Not found. Just add
        _books.emplace_back(std::move(new_book_));
    }

    std::vector<std::string> getTitles() const
    {
        std::vector<std::string> titles;

        for (auto& book : _books)
        {
            titles.push_back(book.getTitle());
        }
        return titles;
    }

private:
    std::vector<BookTitle> _books;
};
