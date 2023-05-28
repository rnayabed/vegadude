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
template Logger& operator<<(Logger&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>);
template Logger& operator<<(Logger&, std::basic_string_view<char, std::char_traits<char>>);
template Logger& operator<<(Logger&, std::filesystem::__cxx11::path);
template Logger& operator<<(Logger&, unsigned short);
