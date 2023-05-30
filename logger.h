#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <ostream>
#include <fstream>
#include <filesystem>

class Logger
{
public:
    Logger();

    static Logger& get();
    bool setup(std::filesystem::path& filePath);

    void showProgress(const std::string& message, const float& ratio);

    void close();

    constexpr static char NewLine {'\n'};

    template<typename T>
    friend Logger& operator<<(Logger& logger, T text);

private:
    bool m_log;
    std::ofstream m_stream;
};


#endif // LOGGER_H
