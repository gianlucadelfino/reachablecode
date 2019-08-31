#pragma once
#include <string>
#include <chrono>

class TimeLogger
{
public:
    TimeLogger(const std::string& operation_name_, std::ostream& out_)
        : _operation_name(operation_name_),
          _start(std::chrono::high_resolution_clock::now()),
          _out(out_)
    {
    }

    ~TimeLogger()
    {
#ifndef DISABLE_TIME_LOGGER
        const std::chrono::high_resolution_clock::time_point end =
            std::chrono::high_resolution_clock::now();

        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - _start);
        _out << _operation_name << " took: " << elapsed.count() << "ms\n";
#endif
    }

private:
    const std::string _operation_name;
    const std::chrono::high_resolution_clock::time_point _start;
    std::ostream& _out;
};


