#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QCoreApplication>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class Logger
{
public:
    // 初始化日志器
    static void init(const QString& logFile = "logs/app.log", 
                     LogLevel level = LogLevel::INFO,
                     bool logToConsole = true,
                     bool logToFile = true);
    
    // 便捷初始化方法
    static void initialize(const QString& logDir, const QString& logFileName,
                          LogLevel level = LogLevel::INFO,
                          bool logToConsole = true,
                          bool logToFile = true);
    
    // 关闭日志器
    static void shutdown();
    
    // 日志方法
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);
    static void critical(const QString& message);
    
    // 格式化日志
    static void log(LogLevel level, const QString& message);
    
private:
    Logger() = delete;
    ~Logger() = delete;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    static QString logLevelToString(LogLevel level);
    static QString formatLogMessage(LogLevel level, const QString& message);
    static void writeToFile(const QString& message);
    static void writeToConsole(const QString& message);
    
    static QFile* logFile;
    static QTextStream* fileStream;
    static QMutex mutex;
    static LogLevel currentLevel;
    static bool consoleOutput;
    static bool fileOutput;
    static bool initialized;
};

// 宏定义，方便使用
#define LOG_DEBUG(msg) Logger::debug(msg)
#define LOG_INFO(msg) Logger::info(msg)
#define LOG_WARNING(msg) Logger::warning(msg)
#define LOG_ERROR(msg) Logger::error(msg)
#define LOG_CRITICAL(msg) Logger::critical(msg)

#endif // LOGGER_H