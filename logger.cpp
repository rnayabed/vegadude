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

bool Logger::setup(std::string& filePath)
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

template Logger& operator<<(Logger& logger, size_t);
template Logger& operator<<(Logger& logger, int32_t);
template Logger& operator<<(Logger& logger, bool);
template Logger& operator<<(Logger& logger, char);
template Logger& operator<<(Logger& logger, char*);
template Logger& operator<<(Logger& logger, char const*);
template Logger& operator<<(Logger& logger, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>);
template Logger& operator<<(Logger& logger, unsigned short);
