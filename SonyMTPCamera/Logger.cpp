#include "pch.h"
#include "Logger.h"

#define MAX_LOGFILE_NAME_LENGTH 1024

HANDLE Logger::s_logFileHandle = INVALID_HANDLE_VALUE;
std::wstring Logger::s_logfileName = L"";

Logger::Logger()
{
    m_serializationMutex = CreateMutex(nullptr, false, nullptr);
}

Logger::~Logger()
{
    CloseHandle(m_serializationMutex);
}

void
Logger::SetLogFilename(std::wstring filename)
{
    if (s_logFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(s_logFileHandle);
        s_logFileHandle = INVALID_HANDLE_VALUE;
    }

    s_logfileName = filename;

    if (!filename.empty())
    {
        // Deal with ENV expansion first
        WCHAR* buffer = new WCHAR[MAX_LOGFILE_NAME_LENGTH];

        if (ExpandEnvironmentStrings(filename.c_str(), buffer, MAX_LOGFILE_NAME_LENGTH))
        {
            s_logfileName = buffer;
        }

        HANDLE hResult = CreateFile(s_logfileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hResult == INVALID_HANDLE_VALUE)
        {
            if (GetLastError() == ERROR_SHARING_VIOLATION)
            {
                // Seems someone else may have the file locked, lets try adding pid to the end of the filename and try again
                std::wostringstream builder;

                builder << s_logfileName << L"." << GetCurrentProcessId() << L".log";
                s_logfileName = builder.str();

                hResult = CreateFile(s_logfileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            }
        }

        if (hResult != INVALID_HANDLE_VALUE)
        {
            s_logFileHandle = hResult;
            SetFilePointer(s_logFileHandle, 0L, nullptr, FILE_END);
        }

        delete[] buffer;
    }
}

void
Logger::Log(LogLevel level, const WCHAR* format, ...)
{
    if (level >= m_level)
    {
        // Ensure we don't make messy log
        if (WaitForSingleObject(m_serializationMutex, 100) == WAIT_OBJECT_0)
        {
            static size_t buffersize = 8192;
            WCHAR* buffer = new WCHAR[buffersize];
            va_list args;
            va_start(args, format);

            _vsnwprintf_s(buffer, buffersize, buffersize, format, args);

            if (s_logFileHandle != INVALID_HANDLE_VALUE)
            {
                std::wostringstream builder;
                SYSTEMTIME t;

                GetLocalTime(&t);

                builder << std::setw(4) << t.wYear << L"-" << std::setw(2) << std::setfill(L'0') << t.wMonth << L"-" << std::setw(2) << std::setfill(L'0') << t.wDay;
                builder << L" " << std::setw(2) << std::setfill(L'0') << t.wHour << L":" << std::setw(2) << std::setfill(L'0') << t.wMinute << L":" << std::setw(6) << std::setfill(L'0') << std::fixed << std::setprecision(3) << (float)(t.wSecond + t.wMilliseconds / 1000.0);
                builder << L" ";

                switch (level)
                {
                case LogLevel::Trace:
                    builder << L"TRACE ";
                    break;

                case LogLevel::Debug:
                    builder << L"DEBUG ";
                    break;

                case LogLevel::Info:
                    builder << L"INFO  ";
                    break;

                case LogLevel::Warn:
                    builder << L"WARN  ";
                    break;

                case LogLevel::Error:
                    builder << L"ERROR ";
                    break;

                default:
                    builder << L"WAT?? ";
                    break;
                }

                builder << L" " << GetCurrentThreadId() << L" ";

                std::wstring prefix = builder.str();

                WriteFile(s_logFileHandle, prefix.c_str(), prefix.size() * sizeof(WCHAR), nullptr, nullptr);
                WriteFile(s_logFileHandle, buffer, wcslen(buffer) * sizeof(WCHAR), nullptr, nullptr);
                WriteFile(s_logFileHandle, L"\n", sizeof(WCHAR), nullptr, nullptr);

                FlushFileBuffers(s_logFileHandle);
            }

            delete[] buffer;
            va_end(args);

            ReleaseMutex(m_serializationMutex);
        }
    }
}

void
Logger::SetLogLevel(LogLevel level)
{
    m_level = level;
}