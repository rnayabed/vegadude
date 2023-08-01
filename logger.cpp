#include <iostream>

#include "logger.h"

#include <string>
#include <string_view>

Logger::Logger()
    : m_log{false}
{}

Logger &Logger::get()
{
    static Logger instance;
    return instance;
}

bool Logger::setup(std::filesystem::path& filePath)
{
    m_stream.open(filePath, std::ofstream::out | std::ofstream::trunc);

    m_log = !m_stream.fail();
    return m_log;
}

void Logger::showProgress(const std::string& message, const float &ratio)
{
    constexpr int32_t barSize = 50;
    int32_t completeSize = ratio * barSize;
    int32_t remainingSize = barSize - completeSize;

    std::cerr << "\rProgress [";
    for (int32_t i = 0; i < completeSize; i++)
        std::cerr << "#";
    for (int32_t i = 0; i < remainingSize; i++)
        std::cerr << "-";
    std::cerr << "] - "
              << static_cast<int>(ratio * 100)
              << "% - "
              << message;
}

void Logger::close()
{
    get() << NewLine;

    if (m_log)
        m_stream.close();
}

template<typename T>
Logger& operator<<(Logger& logger, T text)
{
    std::cerr << text;
    if (logger.m_log) logger.m_stream << text;
    return logger;
}

template Logger& operator<<(Logger&, size_t);
template Logger& operator<<(Logger&, int32_t);
template Logger& operator<<(Logger&, bool);
template Logger& operator<<(Logger&, double);
template Logger& operator<<(Logger&, char);
template Logger& operator<<(Logger&, char*);
template Logger& operator<<(Logger&, char const*);
template Logger& operator<<(Logger&, std::basic_string<char, std::char_traits<char>, std::allocator<char>>);
template Logger& operator<<(Logger&, std::basic_string_view<char, std::char_traits<char>>);
template Logger& operator<<(Logger&, std::filesystem::path);
template Logger& operator<<(Logger&, unsigned short);
