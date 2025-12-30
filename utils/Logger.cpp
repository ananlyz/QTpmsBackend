#include "Logger.h"
#include <QDir>
#include <QDebug>

// 静态成员初始化
QFile* Logger::logFile = nullptr;
QTextStream* Logger::fileStream = nullptr;
QMutex Logger::mutex;
LogLevel Logger::currentLevel = LogLevel::INFO;
bool Logger::consoleOutput = true;
bool Logger::fileOutput = true;
bool Logger::initialized = false;

void Logger::init(const QString& logFilePath, LogLevel level, bool logToConsole, bool logToFile)
{
    QMutexLocker locker(&mutex);
    
    currentLevel = level;
    consoleOutput = logToConsole;
    fileOutput = logToFile;
    
    if (fileOutput) {
        // 确保日志目录存在
        QFileInfo fileInfo(logFilePath);
        QDir().mkpath(fileInfo.absolutePath());
        
        logFile = new QFile(logFilePath);
        if (logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            fileStream = new QTextStream(logFile);
            fileStream->setCodec("UTF-8");
            info("Logger initialized successfully");
        } else {
            qWarning() << "Failed to open log file:" << logFilePath;
            logFile = nullptr;
            fileStream = nullptr;
        }
    }
    
    initialized = true;
}

void Logger::initialize(const QString& logDir, const QString& logFileName,
                       LogLevel level, bool logToConsole, bool logToFile)
{
    QString logFilePath = logDir + "/" + logFileName + ".log";
    init(logFilePath, level, logToConsole, logToFile);
}

void Logger::shutdown()
{
    QMutexLocker locker(&mutex);
    
    if (logFile && logFile->isOpen()) {
        logFile->close();
        delete fileStream;
        delete logFile;
        logFile = nullptr;
        fileStream = nullptr;
    }
    
    initialized = false;
}

void Logger::debug(const QString& message)
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const QString& message)
{
    log(LogLevel::INFO, message);
}

void Logger::warning(const QString& message)
{
    log(LogLevel::WARNING, message);
}

void Logger::error(const QString& message)
{
    log(LogLevel::ERROR, message);
}

void Logger::critical(const QString& message)
{
    log(LogLevel::CRITICAL, message);
}

void Logger::log(LogLevel level, const QString& message)
{
    if (!initialized || level < currentLevel) {
        return;
    }
    
    QMutexLocker locker(&mutex);
    
    QString formattedMessage = formatLogMessage(level, message);
    
    if (consoleOutput) {
        writeToConsole(formattedMessage);
    }
    
    if (fileOutput && logFile && logFile->isOpen()) {
        writeToFile(formattedMessage);
    }
}

QString Logger::logLevelToString(LogLevel level)
{
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

QString Logger::formatLogMessage(LogLevel level, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = logLevelToString(level);
    
    return QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
}

void Logger::writeToFile(const QString& message)
{
    if (fileStream) {
        *fileStream << message << "\n" << flush;
        fileStream->flush();
    }
}

void Logger::writeToConsole(const QString& message)
{
    qDebug().noquote() << message;
}