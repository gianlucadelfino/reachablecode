#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include "opencv2/opencv.hpp"
#include <tesseract/baseapi.h>

#include "Math.h"
#include "VideoWindow.h"

struct BookTitle
{
    explicit BookTitle(
        const std::vector<std::pair<int, std::string>>& weightsAndWords_)
        : _weightsAndWords(weightsAndWords_),
          _wordScores([&weightsAndWords_]() {
              std::map<std::string, int> map;
              for (auto& scoreAndWord : weightsAndWords_)
              {
                  map.insert(
                      std::make_pair(scoreAndWord.second, scoreAndWord.first));
              }
              return map;
          }())
    {
    }

    BookTitle(BookTitle&& other_)
        : _weightsAndWords(std::move(other_._weightsAndWords)),
          _wordScores(std::move(other_._wordScores))
    {
    }

    BookTitle& operator=(BookTitle&& other_)
    {
        _weightsAndWords = std::move(other_._weightsAndWords);
        _wordScores = std::move(other_._wordScores);
        return *this;
    }

    BookTitle& operator=(const BookTitle&) = delete;

    std::string getTitle() const
    {
        std::string title;
        for (auto& weightAndWord : _weightsAndWords)
        {
            title += " " + weightAndWord.second;
        }
        return title;
    }

    bool matches(const BookTitle& other_) const
    {
        int common_words{};
        for (auto& wordScore : _wordScores)
        {
            const std::string& our_word = wordScore.first;
            const auto otherWordScoreIter = other_._wordScores.find(our_word);
            if (otherWordScoreIter != other_._wordScores.cend())
            {
                // Common word has been found
                common_words++;
            }
        }
        const size_t num_words_other = other_._weightsAndWords.size();
        const size_t num_words_this = _weightsAndWords.size();
        const float match_score =
            2 * common_words /
            static_cast<float>(num_words_other + num_words_this);

        // Let's say 80..
        // DEbUG
        std::cout << "\tComputing match " << other_.getTitle() << " vs "
                  << getTitle() << ", score: " << match_score
                  << std::endl;

        return match_score >= 0.4f;
    }

    float score() const
    {
        return std::accumulate(
            _weightsAndWords.cbegin(),
            _weightsAndWords.cend(),
            0.f,
            [](float sum, auto& item) { return sum + item.first; });
    }

private:
    std::vector<std::pair<int, std::string>> _weightsAndWords;
    std::map<std::string, int> _wordScores;
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
                    // DEBUG
                    std::cout << "\tFound same book, replace "
                              << new_book_.getTitle() << std::endl;
                    book = std::move(new_book_);
                }
                else
                {
                    std::cout << "\tFound same book, don't replace "
                              << new_book_.getTitle() << std::endl;
                }
                return;
            }
            else {
                std::cout << "DIDN NOT MATCH\n";
            }
        }

        // DEBUG
        std::cout << "\tBook not found in library, just add: "
                  << new_book_.getTitle() << std::endl;
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

class OCRText
{
public:
    explicit OCRText(const char* const TSVtext_) : _text(TSVtext_) {}

    ~OCRText() { delete[] _text; }

    char operator[](size_t i) { return _text[i]; }

    const char* getText() const { return _text; }

    std::vector<std::vector<std::pair<int, std::string>>> getBooks() const
    {
        if (!_text)
        {
            std::cerr << "No text found" << std::endl;
            return {};
        }

        std::vector<std::vector<std::pair<int, std::string>>> titles;
        // Each line is like.
        // 5       1       1       1       1       2       201     255     240
        // 77      96      Malavoglia
        int a, b, c, d, e, f, g, h, i, j, word_score{};

        // We only want the good words (say confidence > 90)
        std::stringstream ss(_text);
        std::vector<std::pair<int, std::string>> line;
        while (ss >> a >> b >> c >> d >> e >> f >> g >> h >> i >> j >>
               word_score)
        {
            if (word_score != -1)
            {
                std::string word;
                ss >> word;
                line.push_back(std::make_pair(word_score, word));
            }
            else
            {
                // New Line
                const float average = math::average(
                    line.cbegin(), line.cend(), [](float sum, auto& item) {
                        return sum + item.first;
                    });
                if (average > 85)
                {
                    titles.push_back(line);
                }
                line.clear();
            }
        }

        return titles;
    }

private:
    const char* const _text;
};

int main()
{
    VideoWindow win(0, "exShelf");

    // Tesseract setup
    tesseract::TessBaseAPI ocr;

    // Change is to the appropriate language
    ocr.Init(nullptr, "ita", tesseract::OEM_LSTM_ONLY);

    Library lib;

    while (true)
    {
        cv::Mat frame = win.getFrame();

        // If the frame is empty, continue
        if (frame.empty())
        {
            std::cerr << "Emtpy frame";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // Display the resulting frame
        cv::bitwise_not(frame, frame);
        cv::imshow(win.getWindowName(), frame);

        ocr.SetImage(frame.data, frame.cols, frame.rows, 3, frame.step);
        ocr.SetSourceResolution(300);

        OCRText outOCR(ocr.GetTSVText(0));

        const std::vector<std::vector<std::pair<int, std::string>>> books =
            outOCR.getBooks();
        //        std::cout << "books size " << books.size() << std::endl;
        for (auto& title : books)
        {
            lib.insert(BookTitle(title));
        }

        auto titles = lib.getTitles();
        std::cout << "\nNum titles " << titles.size() << std::endl;
        for (auto& title : titles)
        {
            std::cout << title << std::endl;
        }

        std::cout << std::endl << std::endl;

        // Press  ESC on keyboard to  exit
        const char c = static_cast<char>(cv::waitKey(1));
        if (c == 27)
        {
            break;
        }
    }

    return 0;
}
