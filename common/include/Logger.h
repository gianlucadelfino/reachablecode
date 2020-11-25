#pragma once

#include <iostream>
#include <string>

/**
 * @brief The lazier Logger class you can imagine.
 */
class Logger
{
public:
    enum Level
    {
        DEBUG = 0,
        INFO,
        WARNING,
        ERR
    };
    Level level{Level::INFO};

    static Logger& Instance()
    {
        static Logger l;
        return l;
    }

    static void SetLevel(Level l) { Instance().level = l; }

    template <typename First, typename... Args>
    static void Debug(First&& first, Args&&... args)
    {
        if (Level::DEBUG >= Instance().level)
        {
            std::cout << "DEBUG: ";
            DebugHelper(first, args...);
        }
    }

    template <typename First, typename... Args>
    static void Info(First&& first, Args&&... args)
    {
        if (Level::INFO >= Instance().level)
        {
            std::cout << "INFO: ";
            InfoHelper(first, args...);
        }
    }

    template <typename First, typename... Args>
    static void Warning(First&& first, Args&&... args)
    {
        if (Level::WARNING >= Instance().level)
        {
            std::cout << "WARNING: ";
            WarningHelper(first, args...);
        }
    }

    template <typename First, typename... Args>
    static void Error(First&& first, Args&&... args)
    {
        if (Level::ERR >= Instance().level)
        {
            std::cerr << "ERROR: ";
            ErrorHelper(first, args...);
        }
    }

private:
    template <typename First, typename... Args>
    static void DebugHelper(First& first, Args&&... args)
    {
        std::cout << first;

        if constexpr (sizeof...(Args) > 0)
        {
            std::cout << " ";
            Logger::DebugHelper(args...);
        }
        else
        {
            std::cout << std::endl;
        }
    }

    template <typename First, typename... Args>
    static void InfoHelper(First&& first, Args&&... args)
    {
        std::cout << first;

        if constexpr (sizeof...(Args) > 0)
        {
            std::cout << " ";
            Logger::InfoHelper(args...);
        }
        else
        {
            std::cout << std::endl;
        }
    }

    template <typename First, typename... Args>
    static void WarningHelper(First&& first, Args&&... args)
    {
        std::cout << first;

        if constexpr (sizeof...(Args) > 0)
        {
            std::cout << " ";
            Logger::WarningHelper(args...);
        }
        else
        {
            std::cout << std::endl;
        }
    }

    template <typename First, typename... Args>
    static void ErrorHelper(First&& first, Args&&... args)
    {
        std::cerr << first;

        if constexpr (sizeof...(Args) > 0)
        {
            std::cerr << " ";
            Logger::ErrorHelper(args...);
        }
        else
        {
            std::cerr << std::endl;
        }
    }
};