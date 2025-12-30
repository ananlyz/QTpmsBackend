#include "AppConfig.h"
#include <QDir>
#include <QFile>
#include <QDebug>

AppConfig& AppConfig::instance()
{
    static AppConfig instance;
    return instance;
}

AppConfig::AppConfig()
{
    setDefaultValues();
}

bool AppConfig::loadConfig(const QString& configFile)
{
    QString configPath = configFile;
    
    // 如果配置文件不存在，创建默认配置
    if (!QFile::exists(configPath)) {
        QDir().mkpath("config");
        
        // 创建默认配置文件
        QFile file(configPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "[Server]\n";
            stream << "host=127.0.0.1\n";
            stream << "port=8080\n";
            stream << "maxThreads=10\n";
            stream << "\n";
            stream << "[Database]\n";
            stream << "host=127.0.0.1\n";
            stream << "port=3306\n";
            stream << "name=parking\n";
            stream << "user=root\n";
            stream << "password=123456\n";
            stream << "poolSize=10\n";
            stream << "\n";
            stream << "[Logging]\n";
            stream << "level=INFO\n";
            stream << "file=logs/app.log\n";
            stream << "logToFile=true\n";
            stream << "logToConsole=true\n";
            file.close();
        }
    }
    
    settings = new QSettings(configPath, QSettings::IniFormat);
    return true;
}

QString AppConfig::getServerHost() const
{
    return getValue("Server/host", "127.0.0.1");
}

quint16 AppConfig::getServerPort() const
{
    return static_cast<quint16>(getIntValue("Server/port", 8080));
}

int AppConfig::getMaxThreads() const
{
    return getIntValue("Server/maxThreads", 10);
}

QString AppConfig::getDbHost() const
{
    return getValue("Database/host", "127.0.0.1");
}

int AppConfig::getDbPort() const
{
    return getIntValue("Database/port", 3306);
}

QString AppConfig::getDbName() const
{
    return getValue("Database/name", "parking");
}

QString AppConfig::getDbUser() const
{
    return getValue("Database/user", "root");
}

QString AppConfig::getDbPassword() const
{
    return getValue("Database/password", "");
}

int AppConfig::getDbPoolSize() const
{
    return getIntValue("Database/poolSize", 10);
}

QString AppConfig::getLogLevel() const
{
    return getValue("Logging/level", "INFO");
}

QString AppConfig::getLogFile() const
{
    return getValue("Logging/file", "logs/app.log");
}

bool AppConfig::isLogToFile() const
{
    return getBoolValue("Logging/logToFile", true);
}

bool AppConfig::isLogToConsole() const
{
    return getBoolValue("Logging/logToConsole", true);
}

QString AppConfig::getValue(const QString& key, const QString& defaultValue) const
{
    if (settings) {
        return settings->value(key, defaultValue).toString();
    }
    return defaultValue;
}

int AppConfig::getIntValue(const QString& key, int defaultValue) const
{
    if (settings) {
        return settings->value(key, defaultValue).toInt();
    }
    return defaultValue;
}

bool AppConfig::getBoolValue(const QString& key, bool defaultValue) const
{
    if (settings) {
        return settings->value(key, defaultValue).toBool();
    }
    return defaultValue;
}

void AppConfig::setDefaultValues()
{
    defaultValues["Server/host"] = "127.0.0.1";
    defaultValues["Server/port"] = 8080;
    defaultValues["Server/maxThreads"] = 10;
    defaultValues["Database/host"] = "127.0.0.1";
    defaultValues["Database/port"] = 3306;
    defaultValues["Database/name"] = "parking";
    defaultValues["Database/user"] = "root";
    defaultValues["Database/password"] = "";
    defaultValues["Database/poolSize"] = 10;
    defaultValues["Logging/level"] = "INFO";
    defaultValues["Logging/file"] = "logs/app.log";
    defaultValues["Logging/logToFile"] = true;
    defaultValues["Logging/logToConsole"] = true;
}