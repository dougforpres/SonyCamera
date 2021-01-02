#pragma once
#include "pch.h"

#define LOGTRACE(format, ...) logger.Log(Logger::LogLevel::Trace, format, __VA_ARGS__)
#define LOGDEBUG(format, ...) logger.Log(Logger::LogLevel::Debug, format, __VA_ARGS__)
#define LOGINFO(format, ...) logger.Log(Logger::LogLevel::Info, format, __VA_ARGS__)
#define LOGWARN(format, ...) logger.Log(Logger::LogLevel::Warn, format, __VA_ARGS__)
#define LOGERROR(format, ...) logger.Log(Logger::LogLevel::Error, format, __VA_ARGS__)

class Logger
{
public:
    enum class LogLevel
    {
        Error = 5, Warn = 4, Info = 3, Debug = 2, Trace = 1
    };

    Logger();
    ~Logger();

    static void SetLogFilename(std::wstring filename);

    void SetLogLevel(LogLevel level);
    void Log(LogLevel level, const WCHAR* format, ...);

    static std::wstring s_logfileName;
    static HANDLE s_logFileHandle;

protected:

private:
    HANDLE m_serializationMutex = INVALID_HANDLE_VALUE;
    LogLevel m_level = LogLevel::Error;
};

extern Logger logger;
