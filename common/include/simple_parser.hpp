#include <any>
#include <cassert>
#include <concepts>
#include <iostream>
#include <map>
#include <variant>

/**
 * @brief simple_parser generates an iterable container from main's input
 * paramemters
 *
 */
class simple_parser
{
  using container = std::map<std::string, std::variant<std::string, bool, int>>;

public:
  simple_parser() = default;

  void parse(int argc, const char* const argv[])
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
        const std::string key(token.substr(2));
        if (!_args.contains(key))
        {
          throw std::runtime_error("Unrecognized item passed via command line: " + key);
        }
        const std::string value(argv[i + 1]);
        if (value == "true" or value == "false")
        {
          _args[key] = value == "true" ? true : false;
        }
        else if (is_number(value))
        {
          _args[key] = std::stoi(value);
        }
        else
        {
          _args[key] = value;
        }

        i += 2;
      }
      else
      {
        throw std::runtime_error("Expected key starting with --, found a value.");
      }
    }
  }

  template <typename T = std::string>
  void add_default(const std::string& key_, T value_)
  {
    if (auto iter = _args.find(key_); iter == _args.cend())
    {
      _args.insert(std::make_pair(key_, value_));
    }
  }

  template <typename T = std::string>
  T get_value(const std::string& key_) const
  {
    return std::get<T>(_args.at(key_));
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
  static bool is_number(const std::string& s)
  {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
      ++it;
    return !s.empty() && it == s.end();
  }
  container _args{};
};