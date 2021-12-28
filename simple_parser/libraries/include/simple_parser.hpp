#include <any>
#include <cassert>
#include <concepts>
#include <iostream>
#include <map>

/**
 * @brief simple_parser generates an iterable container from main's input
 * paramemters
 *
 */
class simple_parser
{
  using container = std::map<std::string, std::string>;

  public:
  simple_parser(int argc, const char* const argv[])
  {
    // Skip the first argument that is the name of the binary
    for (int i = 1; i < argc;)
    {
      const std::string_view token(argv[i]);
      if (token.starts_with("--"))
      {
        if (i + 1 == argc)
        {
          throw std::runtime_error("Missing value after key.");
        }
        const std::string_view key = token.substr(2);
        const std::string_view value(argv[i + 1]);
        _args.insert(std::make_pair(key, value));
        i += 2;
      }
      else
      {
        throw std::runtime_error(
            "Expected key starting with --, found a value.");
      }
    }
  }

  void add_default(const std::string& key_, const std::string& value_)
  {
    if (auto iter = _args.find(key_); iter == _args.cend())
    {
      _args.insert(std::make_pair(key_, value_));
    }
  }

  std::string get_value(const std::string& key_) const
  {
    return _args.at(key_);
  }

  using const_iterator = container::const_iterator;
  const_iterator cbegin() const { return _args.cbegin(); }
  const_iterator cend() const { return _args.cend(); }

  using iterator = container::iterator;
  iterator begin() { return _args.begin(); }
  iterator end() { return _args.end(); }

  simple_parser(const simple_parser&) = delete;
  simple_parser(simple_parser&&) = delete;
  simple_parser& operator=(const simple_parser&) = delete;
  simple_parser& operator=(simple_parser&&) = delete;

  private:
  container _args;
};